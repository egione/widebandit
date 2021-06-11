/*
Widebandit
Copyright 2021 Russell Leidich

This collection of files constitutes the Widebandit Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Widebandit Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Widebandit Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Widebandit Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
Probability Disitribution Function Archive Manager
*/
#include "flag.h"
#include "flag_archive.h"
#include "flag_fakefloat.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "archive.h"
#include "archive_xtrn.h"
#include "fakefloat.h"
#include "fakefloat_xtrn.h"
#include "lmd2.h"

u8
archive_downsample(ULONG slice_idx_max_idx_max, ULONG *slice_idx_max_list_base, u8 *sort_status_base, u32 *u32_list_base, u32 **u32_list_base_list_base){
/*
Convert each PDF in a Spectrafy archive to an approximately uniform subsampling of the same without using interpolation.

In:

  slice_idx_max_idx_max is the maximum index of both *slice_idx_max_list_base and *u32_list_base_list_base.

  *slice_idx_max_list_base contains maximum slice indexes from which to downsample all PDFs. The maximum slice index is equal to the number of fakefloats which will be preserved in each downsampled PDF. N fakefloats serve as the walls dividing (N+1) slices of the real line, the first and last of which being half-infinite in size.

  *sort_status_base is undefined.

  *u32_list_base is the entire archive to downsample.

  u32_list_base_list_base is the return value of archive_list_malloc().

Out:

  Returns one of the following, which, if nonzero, implies that no memory has been allocated and other outputs are unchanged:

    0: Success.

    1: Out of memory.

    2: At least one of the items at *slice_idx_max_list_base is zero, which would imply that the caller wishes to downsample a PDF to a single slice, which would be meaningless.

    3: At least one of the items at *slice_idx_max_list_base exceeds the number of floats defined (not merely allocated) per PDF, so the PDF lacks sufficient information to create the requested number of slices.

  The archive at *u32_list_base has been downsampled to archives pointed to by bases at *u32_list_base_list_base. Each archive contains the corresponding slice_idx_max fakefloats which imply (slice_idx_max+1) slices.

  *sort_status_base is one if all of the downsampled archives have sorted PDFs (which could happen even if the same was not true of the source archive), else zero. This is important to know because unsorted PDFs cannot be consumed by Slice.
*/
  ULONG archive_size;
  u8 exp;
  header_t *header_base;
  ULONG list_size;
  u32 mantissa;
  ULONG pdf_count;
  ULONG pdf_float_count;
  ULONG pdf_idx;
  ULONG pdf_idx_max;
  ULONG pdf_slot_count;
  ULONG quotient;
  ULONG *quotient_list_base;
  ULONG remainder;
  ULONG *remainder_list_base;
  u8 sign_status;
  ULONG slice_count;
  ULONG slice_idx_max;
  ULONG slice_idx_max_idx;
  ULONG slice_idx_max_max;
  ULONG slice_idx_max_min;
  ULONG slot_count;
  ULONG *slot_count_list_base;
  ULONG slot_idx_max;
  u8 sort_status;
  u8 status;
  u32 u32_0;
  u32 u32_1;
  ULONG u32_frac;
  ULONG *u32_frac_list_base;
  ULONG u32_frac_new;
  ULONG u32_idx0;
  ULONG u32_idx1;
  ULONG *u32_idx_list_base0;
  ULONG *u32_idx_list_base1;
  ULONG u32_idx_min;
  ULONG u32_idx_new;
  u32 *u32_list_base1;

  sort_status=1;
  slice_idx_max_idx=0;
  slice_idx_max_max=0;
  slice_idx_max_min=~slice_idx_max_max;
  do{
    slice_idx_max=slice_idx_max_list_base[slice_idx_max_idx];
    slice_idx_max_max=MAX(slice_idx_max, slice_idx_max_max);
    slice_idx_max_min=MIN(slice_idx_max, slice_idx_max_min);
  }while((slice_idx_max_idx++)!=slice_idx_max_idx_max);
  header_base=NULL;
  slot_count_list_base=NULL;
  quotient_list_base=NULL;
  remainder_list_base=NULL;
  u32_frac_list_base=NULL;
  u32_idx_list_base0=NULL;
  u32_idx_list_base1=NULL;
  do{
    status=2;
    if(!slice_idx_max_min){
      break;
    }
    header_base=archive_header_init();
    status=!header_base;
    if(status){
      break;
    }
    archive_header_import(header_base, u32_list_base);
    status=3;
    pdf_count=(ULONG)(header_base->pdf_count);
    pdf_float_count=(ULONG)(header_base->pdf_float_count);
    pdf_slot_count=(ULONG)(header_base->pdf_slot_count);
    if(pdf_float_count<slice_idx_max){
      break;
    }
    list_size=(slice_idx_max_idx_max+1)<<ULONG_SIZE_LOG2;
    slot_count_list_base=DEBUG_MALLOC_PARANOID(list_size);
    status=!slot_count_list_base;
    quotient_list_base=DEBUG_MALLOC_PARANOID(list_size);
    status=(u8)(status|!quotient_list_base);
    remainder_list_base=DEBUG_MALLOC_PARANOID(list_size);
    status=(u8)(status|!remainder_list_base);
    u32_frac_list_base=DEBUG_MALLOC_PARANOID(list_size);
    status=(u8)(status|!u32_frac_list_base);
    u32_idx_list_base0=DEBUG_MALLOC_PARANOID(list_size);
    status=(u8)(status|!u32_idx_list_base0);
    u32_idx_list_base1=DEBUG_MALLOC_PARANOID(list_size);
    status=(u8)(status|!u32_idx_list_base1);
    if(status){
      break;
    }
    slice_idx_max_idx=0;
    do{
      slice_idx_max=slice_idx_max_list_base[slice_idx_max_idx];
      slice_count=slice_idx_max+1;
      quotient=pdf_float_count/slice_count;
      remainder=pdf_float_count%slice_count;
      quotient_list_base[slice_idx_max_idx]=quotient;
      remainder_list_base[slice_idx_max_idx]=remainder;
/*
If we want to divide a PDF in N more-or-less equal slices, then we need only (N-1) floats for boundary demarkation.
*/
      slot_count=slice_idx_max;
      slot_count_list_base[slice_idx_max_idx]=slot_count;
      u32_idx_list_base1[slice_idx_max_idx]=ARCHIVE_HEADER_U32_COUNT;
    }while((slice_idx_max_idx++)!=slice_idx_max_idx_max);
    pdf_idx=0;
    u32_idx_min=ARCHIVE_HEADER_U32_COUNT;
    do{
      slice_idx_max_idx=0;
      do{
        quotient=quotient_list_base[slice_idx_max_idx];
        remainder=remainder_list_base[slice_idx_max_idx];
        u32_frac_list_base[slice_idx_max_idx]=remainder;
        u32_idx_list_base0[slice_idx_max_idx]=quotient;
      }while((slice_idx_max_idx++)!=slice_idx_max_idx_max);
      u32_1=U32_MAX;
      do{
        u32_frac=ULONG_MAX;
        u32_idx0=u32_frac;
        for(slice_idx_max_idx=0; slice_idx_max_idx<=slice_idx_max_idx_max; slice_idx_max_idx++){
          u32_idx_new=u32_idx_list_base0[slice_idx_max_idx];
          if(u32_idx_new<=u32_idx0){
            u32_frac_new=u32_frac_list_base[slice_idx_max_idx];
            if((u32_idx_new<u32_idx0)||(u32_frac_new<u32_frac)){
              u32_frac=u32_frac_new;
              u32_idx0=u32_idx_new;
            }
          }
        }
        if(pdf_float_count==u32_idx0){
          break;
        }
        for(slice_idx_max_idx=slice_idx_max_idx_max; slice_idx_max_idx<=slice_idx_max_idx_max; slice_idx_max_idx--){
          u32_idx_new=u32_idx_list_base0[slice_idx_max_idx];
          if(u32_idx_new==u32_idx0){
            u32_frac_new=u32_frac_list_base[slice_idx_max_idx];
            if(u32_frac==u32_frac_new){
              u32_idx1=u32_idx_list_base1[slice_idx_max_idx];
              u32_list_base1=u32_list_base_list_base[slice_idx_max_idx];
              u32_0=u32_list_base[u32_idx_min+u32_idx_new];
              u32_list_base1[u32_idx1]=u32_0;
              u32_idx1++;
              u32_idx_list_base1[slice_idx_max_idx]=u32_idx1;
              if(sort_status){
                exp=FLOAT_U32_EXP_GET(u32_1);
                mantissa=FLOAT_U32_MANTISSA_GET(u32_1);
                sign_status=FLOAT_U32_IS_SIGNED(u32_1);
                u32_1=u32_0;
                FLOAT_U32_IS_LESS_EQUAL(exp, mantissa, sign_status, u32_0, sort_status);
              }
              remainder=remainder_list_base[slice_idx_max_idx];
              slice_idx_max=slice_idx_max_list_base[slice_idx_max_idx];
              u32_idx_new+=quotient_list_base[slice_idx_max_idx];
              u32_frac_new+=remainder;
              if(slice_idx_max<u32_frac_new){
                u32_frac_new-=slice_idx_max+1;
                u32_idx_new++;
              }
              u32_frac_list_base[slice_idx_max_idx]=u32_frac_new;
              u32_idx_list_base0[slice_idx_max_idx]=u32_idx_new;
            }
          }
        }
      }while(1);
      slice_idx_max_idx=0;
      do{
        u32_idx1=u32_idx_list_base1[slice_idx_max_idx];
        u32_list_base1=u32_list_base_list_base[slice_idx_max_idx];
        u32_idx_list_base1[slice_idx_max_idx]=u32_idx1;
      }while((slice_idx_max_idx++)!=slice_idx_max_idx_max);
      pdf_idx++;
      u32_idx_min+=pdf_slot_count;
    }while(pdf_idx!=pdf_count);
    pdf_idx_max=pdf_count-1;
    slice_idx_max_idx=0;
    do{
      slice_idx_max=slice_idx_max_list_base[slice_idx_max_idx];
      slot_count=slot_count_list_base[slice_idx_max_idx];
      slot_idx_max=slot_count-1;
      u32_list_base1=u32_list_base_list_base[slice_idx_max_idx];
      archive_header_fill(1, header_base, slice_idx_max, slice_idx_max, pdf_idx_max, slot_idx_max, u32_list_base1);
      archive_size=archive_header_export(header_base, u32_list_base1);
      slice_idx_max_list_base[slice_idx_max_idx]=archive_size;
    }while((slice_idx_max_idx++)!=slice_idx_max_idx_max);
    status=0;
  }while(0);
  DEBUG_FREE_PARANOID(u32_idx_list_base1);
  DEBUG_FREE_PARANOID(u32_idx_list_base0);
  DEBUG_FREE_PARANOID(u32_frac_list_base);
  DEBUG_FREE_PARANOID(remainder_list_base);
  DEBUG_FREE_PARANOID(quotient_list_base);
  DEBUG_FREE_PARANOID(slot_count_list_base);
  archive_free(header_base);
  *sort_status_base=sort_status;
  return status;
}

void *
archive_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Archive calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

u8
archive_header_check(u64 archive_size, header_t *header_base, u64 *lmd2_iterand_base, u8 lmd2_iterand_check_status, u64 *lmd2_partial_sum_base, ULONG *pdf_count_base, ULONG *pdf_float_count_base, ULONG *pdf_slot_count_base){
/*
Verify the integrity of an archive header, optionally including its cached LMD2 hash iterator, then extract its contents to the output variables.

In:

  archive_size is the size of the entire archive including its header.

  header_base is the base of the archive header.

  *lmd2_iterand_base is undefined.

  lmd2_iterand_check_status is one to verify that header_base->lmd2_iterand_cache is in sync with other related header parameters (which might take a long time), else zero.

  *lmd2_partial_sum_base is undefined.

  *pdf_count_base is undefined.

  *pdf_float_count_base is undefined.

  *pdf_slot_count_base is undefined.

Out:

  Returns one if the header is corrupt, in which the return values are unchanged, else zero.

  *lmd2_iterand_base is header_base->lmd2_iterand_cache.

  *lmd2_partial_sum_base is header_base->lmd2_partial_sum.

  *pdf_count_base is header_base->pdf_count, which is the number of PDFs.

  *pdf_float_count_base is header_base->pdf_float_count, which is the number of floats allocated and defined per PDF.

  *pdf_slot_count_base is header_base->pdf_slot_count, which is the number of floats allocated per PDF, which is at least *pdf_float_count_base.
*/
  u64 i;
  u64 lmd2;
  u32 lmd2_c;
  u64 lmd2_iterand;
  u64 lmd2_iterand_cache;
  u32 lmd2_x;
  u64 pdf_count;
  u64 pdf_float_count;
  u64 pdf_slot_count;
  u64 size_following;
  u8 status;
  u64 u32_count;

  status=1;
  do{
    if(header_base->signature!=ARCHIVE_SIGNATURE){
      break;
    }
    pdf_float_count=header_base->pdf_float_count;
    pdf_slot_count=header_base->pdf_slot_count;
    if(pdf_slot_count<pdf_float_count){
      break;
    }
    pdf_count=header_base->pdf_count;
    u32_count=pdf_count*pdf_slot_count;
    #ifdef _32_
      if(u32_count!=(ULONG)(u32_count)){
        break;
      }
    #endif
    if(!u32_count){
      break;
    }
    if((u32_count/pdf_count)!=pdf_slot_count){
      break;
    }
/*
The high 2 bits of u32_count need to be clear so we can left-shift it safely.
*/
    if(u32_count>>(ULONG_BITS-U32_SIZE_LOG2)){
      break;
    }
    size_following=(u32_count+(ARCHIVE_HEADER_U32_COUNT-4))<<U32_SIZE_LOG2;
    if(size_following!=header_base->size_following){
      break;
    }
    if(archive_size<size_following){
      break;
    }
    if(size_following!=(archive_size-(2U<<U64_SIZE_LOG2))){
      break;
    }
    lmd2=archive_header_lmd2_get(header_base);
    if(lmd2!=header_base->lmd2_local){
      break;
    }
    lmd2_iterand_cache=header_base->lmd2_iterand_cache;
/*
We want to verify that the LMD2 iterand cached at (header_base->lmd2_iterand_cache) is in fact consistent with the official iterator. In order to do this, we need to iterate pdf_float_count times because one iteration protects one float. If pdf_float_count is less than the iteration at which lmd2_x (the data multiplier) first becomes zero (as mentioned in lmd2.h), then we don't need to check for zeroes (which must be skipped). When iterating such a large number of times, this can make a significant performance difference. We cache the iterand in order to avoid the need to recompute it from intitial conditions every time data gets appended to a PDF. But verifying it can take seconds, which is why we provide lmd2_iterand_check_status.
*/
    if(lmd2_iterand_check_status){
      LMD_SEED_INIT(LMD2_C0, lmd2_c, LMD2_X0, lmd2_x)
      lmd2_iterand=0;
      if(pdf_float_count<11460787448ULL){
        for(i=0; i<pdf_float_count; i++){
          LMD_ITERATE_NO_ZERO_CHECK(LMD2_A, lmd2_c, lmd2_x, lmd2_iterand)
        }
      }else{
        for(i=0; i<pdf_float_count; i++){
          LMD_ITERATE_WITH_ZERO_CHECK(LMD2_A, lmd2_c, lmd2_x, lmd2_iterand)
        }
      }
      if(lmd2_iterand!=lmd2_iterand_cache){
        status=1;
        break;
      }
    }
    *lmd2_iterand_base=lmd2_iterand_cache;
    *lmd2_partial_sum_base=header_base->lmd2_partial_sum;
    *pdf_count_base=(ULONG)(pdf_count);
    *pdf_float_count_base=(ULONG)(pdf_float_count);
    *pdf_slot_count_base=(ULONG)(pdf_slot_count);
    status=0;
  }while(0);
  return status;
}

ULONG
archive_header_export(header_t *header_base, u32 *u32_list_base){
/*
Export an archive header as a list of (u32)s.

In:

  *header_base contains an archive header, which is defined but may or may not be valid.

  *u32_list_base is the base of ARCHIVE_HEADER_U32_COUNT undefined (u32)s.

Out:

  Returns the archive size implied by the header, which is valid only if the header itself is valid. (See also archive_header_check().)

  *u32_list_base contains a copy of the header.
*/
  ULONG archive_size;
  ULONG pdf_count;
  ULONG pdf_slot_count;
  u64 u64_0;

  u64_0=header_base->signature;
  archive_u32_pair_from_u64(0, u32_list_base, u64_0);
  u64_0=header_base->size_following;
  archive_u32_pair_from_u64(2, u32_list_base, u64_0);
  u64_0=header_base->lmd2_local;
  archive_u32_pair_from_u64(4, u32_list_base, u64_0);
  u64_0=header_base->lmd2_iterand_cache;
  archive_u32_pair_from_u64(6, u32_list_base, u64_0);
  u64_0=header_base->lmd2_partial_sum;
  archive_u32_pair_from_u64(8, u32_list_base, u64_0);
  u64_0=header_base->pdf_float_count;
  archive_u32_pair_from_u64(0xA, u32_list_base, u64_0);
  u64_0=header_base->pdf_slot_count;
  pdf_slot_count=(ULONG)(u64_0);
  archive_u32_pair_from_u64(0xC, u32_list_base, u64_0);
  u64_0=header_base->pdf_count;
  pdf_count=(ULONG)(u64_0);
  archive_u32_pair_from_u64(0xE, u32_list_base, u64_0);
  archive_size=((pdf_count*pdf_slot_count)<<U32_SIZE_LOG2)+ARCHIVE_HEADER_SIZE;
  return archive_size;
}

void
archive_header_fill(u8 hash_reset_status, header_t *header_base, ULONG pdf_float_count, ULONG pdf_float_count_old, ULONG pdf_idx_max, ULONG pdf_slot_idx_max, u32 *u32_list_base){
/*
Populate the fields of an archive header and optionally recompute all the hashes rather than incrementally updating them.

In:

  hash_reset_status is one to recompute the sum of the partial LMD2 hashes of the allocated floats in each PDF, else zero. Must be one if *header_base is undefined.

  *header_base is either a valid but stale archive header (because more floats have since been allocated) or undefined because this is a new archive.

  pdf_float_count is the number of allocated and defined floats per PDF. They must be contiguous, as they would be if they had passed through archive_sort_compact().

  pdf_float_count old is ignored if hash_reset_status is one. Otherwise, it's the number of contiguously allocated and defined floats per PDF which are already reflected in the (stale) header.

  pdf_idx_max is the number of PDFs in the archive, less one.

  pdf_slot_idx_max is the sum of allocated and free float slots in each PDF, less one.

  *u32_list_base is the base of the archive from which to fill or update *header_base.

Out:

  *header_base is now consistent with the archive image at *u32_list_base, but the header at the base of the latter has not been changed.
*/
  u64 lmd2;
  u64 lmd2_iterand;
  u64 lmd2_partial_sum;
  ULONG pdf_count;
  ULONG pdf_hash_idx_max;
  ULONG pdf_hash_idx_min;
  ULONG pdf_slot_count;
  u64 size_following;
  u64 u32_count;

  pdf_count=pdf_idx_max+1;
  pdf_slot_count=pdf_slot_idx_max+1;
  u32_count=(u64)(pdf_count)*pdf_slot_count;
  size_following=u32_count<<U32_SIZE_LOG2;
  size_following+=ARCHIVE_HEADER_SIZE-(2U<<U64_SIZE_LOG2);
  header_base->pdf_count=pdf_count;
  header_base->pdf_float_count=pdf_float_count;
  header_base->pdf_slot_count=pdf_slot_count;
  header_base->signature=ARCHIVE_SIGNATURE;
  header_base->size_following=size_following;
  if(!hash_reset_status){
    lmd2_iterand=header_base->lmd2_iterand_cache;
    lmd2_partial_sum=header_base->lmd2_partial_sum;
  }else{
    lmd2_iterand=0;
    header_base->lmd2_iterand_cache=lmd2_iterand;
    lmd2_partial_sum=0;
  }
  pdf_hash_idx_max=pdf_float_count+ARCHIVE_HEADER_U32_COUNT-1;
  pdf_hash_idx_min=ARCHIVE_HEADER_U32_COUNT;
  if(!hash_reset_status){
    pdf_hash_idx_min+=pdf_float_count_old;
  }
  if(pdf_hash_idx_min<=pdf_hash_idx_max){
    while(pdf_count--){
      lmd2_iterand=header_base->lmd2_iterand_cache;
      lmd2_partial_sum+=fakefloat_u32_list_hash(&lmd2_iterand, pdf_hash_idx_max, pdf_hash_idx_min, u32_list_base);
      pdf_hash_idx_max+=pdf_slot_count;
      pdf_hash_idx_min+=pdf_slot_count;
    }
  }
  header_base->lmd2_iterand_cache=lmd2_iterand;
  header_base->lmd2_partial_sum=lmd2_partial_sum;
  lmd2=archive_header_lmd2_get(header_base);
  header_base->lmd2_local=lmd2;
  return;
}

void
archive_header_import(header_t *header_base, u32 *u32_list_base){
/*
Extract an archive header, whether or not valid, as a list of (u32)s.

In:

  *header_base is an alleged valid archive header.

  *u32_list_base is undefined but writable for ARCHIVE_HEADER_U32_COUNT (u32)s.

Out:

  *u32_list_base is a copy of *header_base.
*/
  u64 u64_0;

  u64_0=archive_u64_from_u32_pair(0, u32_list_base);
  header_base->signature=u64_0;
  u64_0=archive_u64_from_u32_pair(2, u32_list_base);
  header_base->size_following=u64_0;
  u64_0=archive_u64_from_u32_pair(4, u32_list_base);
  header_base->lmd2_local=u64_0;
  u64_0=archive_u64_from_u32_pair(6, u32_list_base);
  header_base->lmd2_iterand_cache=u64_0;
  u64_0=archive_u64_from_u32_pair(8, u32_list_base);
  header_base->lmd2_partial_sum=u64_0;
  u64_0=archive_u64_from_u32_pair(0xA, u32_list_base);
  header_base->pdf_float_count=u64_0;
  u64_0=archive_u64_from_u32_pair(0xC, u32_list_base);
  header_base->pdf_slot_count=u64_0;
  u64_0=archive_u64_from_u32_pair(0xE, u32_list_base);
  header_base->pdf_count=u64_0;
  return;
}

header_t *
archive_header_init(void){
/*
Allocate a header full of zeros, then initialize its file type signature.

Out:

  Returns a header as described in the summary.
*/
  header_t *header_base;

  header_base=NULL;
  do{
    header_base=DEBUG_CALLOC_PARANOID(ARCHIVE_HEADER_SIZE);
    if(header_base){
      header_base->signature=ARCHIVE_SIGNATURE;
    }
  }while(0);
  return header_base;
}

u64
archive_header_lmd2_get(header_t *header_base){
/*
Allocate a header full of zeros, then initialize its file type signature.

In:

  *header_base is a valid archive header.

Out:

  Returns the LMD2 hash of all fields of *header_base which are amenable to protection via error detection code.
*/
  u64 lmd2;
  u32 lmd2_c;
  u64 lmd2_iterand;
  u32 lmd2_x;

  lmd2_c=LMD2_C0;
  lmd2_x=LMD2_X0;
  LMD_ITERAND_INIT(lmd2_c, lmd2_x, lmd2_iterand)
  LMD_ACCUMULATOR_INIT(lmd2)
  archive_lmd2_u64_digest(&lmd2, &lmd2_c, &lmd2_x, &lmd2_iterand, header_base->lmd2_iterand_cache);
  archive_lmd2_u64_digest(&lmd2, &lmd2_c, &lmd2_x, &lmd2_iterand, header_base->lmd2_partial_sum);
  archive_lmd2_u64_digest(&lmd2, &lmd2_c, &lmd2_x, &lmd2_iterand, header_base->pdf_float_count);
  archive_lmd2_u64_digest(&lmd2, &lmd2_c, &lmd2_x, &lmd2_iterand, header_base->pdf_slot_count);
  archive_lmd2_u64_digest(&lmd2, &lmd2_c, &lmd2_x, &lmd2_iterand, header_base->pdf_count);
  LMD_FINALIZE(LMD2_A, lmd2_c, lmd2_x, lmd2_iterand, lmd2)
  return lmd2;
}

u32 **
archive_list_free(ULONG u32_count_idx_max, u32 **u32_list_base_list_base){
/*
Free a list of archives.

In:

  u32_count_idx_max is the maximum index of *u32_list_base_list_base.

  u32_list_base_list_base is the return value of archive_list_malloc().

Out:

  Returns NULL for convenience.

  **u32_list_base_list_base and all of its children have been freed.
*/
  ULONG u32_count_idx;
  u32 *u32_list_base;

  if(u32_list_base_list_base){
    u32_count_idx=u32_count_idx_max;
    do{
      u32_list_base=u32_list_base_list_base[u32_count_idx];
      fakefloat_free(u32_list_base);
    }while(u32_count_idx--);
    DEBUG_FREE_PARANOID(u32_list_base_list_base);
  }
  return NULL;
}

u32 **
archive_list_malloc(ULONG pdf_count, ULONG u32_count_idx_max, ULONG *u32_count_list_base){
/*
Allocate a set of undefined (u32) lists for storing archives, along with a list of their bases.

In:

  pdf_count is the number of PDFs to allocate in each new archive.

  u32_count_idx_max is the maximum index of *u32_count_list_base.

  *u32_count_list_base contains the numbers of fakefloat slots to allocate in each PDF of the corresponding archive.

Out:

  Returns NULL on failure, else the base of a list of (u32_count_idx_max+1) bases, each of which pointing to space for an archive containing PDFs with the respective fakefloat counts specified on input.
*/
  ULONG list_size;
  ULONG slot_count;
  u8 status;
  ULONG u32_count;
  ULONG u32_count_idx;
  ULONG u32_count_idx_post;
  ULONG u32_idx_max;
  u32 *u32_list_base;
  u32 **u32_list_base_list_base;

  u32_count_idx_post=u32_count_idx_max+1;
  list_size=u32_count_idx_post*(ULONG)(sizeof(u32 *));
  u32_list_base_list_base=NULL;
  if((list_size/(ULONG)(sizeof(u32 *)))==u32_count_idx_post){
    u32_list_base_list_base=DEBUG_CALLOC_PARANOID(list_size);
    if(u32_list_base_list_base){
      u32_count_idx=0;
      do{
        status=1;
        slot_count=u32_count_list_base[u32_count_idx];
        u32_count=pdf_count*slot_count;
        if((u32_count/pdf_count)==slot_count){
          u32_idx_max=u32_count+ARCHIVE_HEADER_U32_COUNT-1;
          if(u32_count<u32_idx_max){
            u32_list_base=fakefloat_u32_list_malloc(0, u32_idx_max);
            if(u32_list_base){
              status=0;
              u32_list_base_list_base[u32_count_idx]=u32_list_base;
            }
          }
        }
        if(status){
          break;
        }
      }while((u32_count_idx++)!=u32_count_idx_max);
      if(status){
        u32_list_base_list_base=archive_list_free(u32_count_idx_max, u32_list_base_list_base);
      }
    }
  }
  return u32_list_base_list_base;
}

u8
archive_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of ARCHIVE_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of ARCHIVE_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=ARCHIVE_BUILD_BREAK_COUNT) or (build_feature_count>ARCHIVE_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=ARCHIVE_BUILD_BREAK_COUNT);
  status=(u8)(status|(ARCHIVE_BUILD_FEATURE_COUNT<build_feature_count));
  status=(u8)(status|fakefloat_init(FAKEFLOAT_BUILD_BREAK_COUNT_EXPECTED, 0));
  return status;
}

void
archive_lmd2_u64_digest(u64 *lmd2_base, u32 *lmd2_c_base, u32 *lmd2_x_base, u64 *lmd2_iterand_base, u64 u64_0){
/*
Integrate a u64 into an existing partial LMD2.

In:

  *lmd2_base is the output of LMD_ACCUMULATOR_INIT() on the first call then looped back thereafter.

  *lmd2_c_base is LMD2_C0 on the first call then looped back thereafter.

  *lmd2_x_base is LMD2_X0 on the first call then looped back thereafter.

  *lmd2_iterand_base is the output of LMD_ITERAND_INIT() on the first call, then looped back thereafter.

  u64_0 is the u64 to integrate into *lmd2_base.

Out:

  *lmd2_base is a partial LMD2 which has now integrated u64_0.

  *lmd2_c_base has been updated.

  *lmd2_x_base has been updated.

  *lmd2_iterand_base has been updated.
*/
  u64 lmd2;
  u32 lmd2_c;
  u64 lmd2_iterand;
  u32 lmd2_x;
  u32 u32_0;

  lmd2=*lmd2_base;
  lmd2_c=*lmd2_c_base;
  lmd2_x=*lmd2_x_base;
  lmd2_iterand=*lmd2_iterand_base;
  LMD_ITERATE_WITH_ZERO_CHECK(LMD2_A, lmd2_c, lmd2_x, lmd2_iterand)
  u32_0=(u32)(u64_0);
  LMD_ACCUMULATE(u32_0, lmd2_x, lmd2)
  LMD_ITERATE_WITH_ZERO_CHECK(LMD2_A, lmd2_c, lmd2_x, lmd2_iterand)
  u32_0=(u32)(u64_0>>U32_BITS);
  LMD_ACCUMULATE(u32_0, lmd2_x, lmd2)
  *lmd2_base=lmd2;
  *lmd2_c_base=lmd2_c;
  *lmd2_x_base=lmd2_x;
  *lmd2_iterand_base=lmd2_iterand;
  return;
}

ULONG
archive_size_check(u64 archive_size){
/*
Determine whether the size of an alleged archive is (1) large enough to contain a header and (2) small enough to fit in a 32-bit address space (in the case of 32-bit builds).

In:

  archive_size is the size of an alleged archive, which typically would be a file size.

Out:

  Returns zero if either of the conditions mentioned in the summary are false, else the input value contained in a ULONG.
*/
  ULONG archive_size_ulong;

  archive_size_ulong=(ULONG)(archive_size);
  if(archive_size_ulong<ARCHIVE_HEADER_SIZE){
    archive_size_ulong=0;
  }
  #ifdef _32_
    if(archive_size!=(ULONG)(archive_size)){
      archive_size_ulong=0;
    }
  #endif
  return archive_size_ulong;
}

u8
archive_sort_compact(u8 compact_status, ULONG delete_u32_count, header_t *header_base, u8 sign_status, u32 *u32_list_base){
/*
Sort, then optionally compact to low indexes, all the fakefloats in each PDF in an archive. This leaves any allocated but undefined space at the end of each PDF.

In:

  compact_status is one to rearrange all the fakefloats in each PDF to be contiguous (as well as sorted) starting with the first slot.

  delete_u32_count is the number of (U32_MAX)s (indicating free slots) which should be removed from what would otherwise be a contiguous block of defined fakefloats. Must be zero if compact_status is zero.

  *header_base is the base of an archive header which is valid but the following fields: lmd2_local, lmd2_iterand_cache, and lmd2_partial_sum.

  sign_status is one iff the fakefloats _might_ contain a negative value, else zero.

  *u32_list_base is a archive which, but for its header, is valid.

Out:

  Returns one of the following:

  0: Success.

  1: Out of memory.

  2: compact_status was one and a PDF was discovered in which the number of free slots did not equal delete_u32_count. *u32_list_base is undefined.

  *u32_list_base is the archive updated so as to comply with the summary.
*/
  ULONG *freq_list_base;
  ULONG list_size;
  ULONG pdf_count;
  ULONG preserved_u32_count;
  u8 status;
  ULONG sublist_u32_count;
  ULONG u32_count;
  ULONG u32_idx;
  ULONG u32_idx_delta;
  ULONG u32_idx_max;
  u32 *u32_list_base0;
  u32 *u32_list_base1;

  freq_list_base=fakefloat_freq_list_malloc();
  status=!freq_list_base;
  u32_count=(ULONG)(header_base->pdf_float_count);
  u32_idx_max=u32_count-1;
  u32_list_base0=fakefloat_u32_list_malloc(0, u32_idx_max);
  status=(u8)(status|!u32_list_base0);
  u32_list_base1=fakefloat_u32_list_malloc(0, u32_idx_max);
  status=(u8)(status|!u32_list_base0);
  pdf_count=(ULONG)(header_base->pdf_count);
  if((!status)&&pdf_count&&u32_count){
    list_size=u32_count<<U32_SIZE_LOG2;
    preserved_u32_count=u32_count-delete_u32_count;
    sublist_u32_count=0;
    u32_idx=ARCHIVE_HEADER_U32_COUNT;
    u32_idx_delta=(ULONG)(header_base->pdf_slot_count);
    while(pdf_count--){
      memcpy(u32_list_base0, &u32_list_base[u32_idx], (size_t)(list_size));
      if(compact_status){
        sublist_u32_count=fakefloat_u32_sublist_compact(u32_idx_max, 0, u32_list_base0);
        if(sublist_u32_count!=preserved_u32_count){
          status=2;
          break;
        }
        list_size=preserved_u32_count<<U32_SIZE_LOG2;
        u32_idx_max=preserved_u32_count-1;
      }
      if(list_size){
        fakefloat_u32_list_sort(freq_list_base, sign_status, u32_idx_max, u32_list_base0, u32_list_base1);
        memcpy(&u32_list_base[u32_idx], u32_list_base0, (size_t)(list_size));
      }
      if(compact_status){
        list_size=delete_u32_count<<U32_SIZE_LOG2;
        u32_idx+=sublist_u32_count;
        memset(&u32_list_base[u32_idx], U8_MAX, (size_t)(list_size));
        list_size=u32_count<<U32_SIZE_LOG2;
        u32_idx-=sublist_u32_count;
        u32_idx_max=u32_count-1;
      }
      u32_idx+=u32_idx_delta;
    }
  }
  fakefloat_free(u32_list_base1);
  fakefloat_free(u32_list_base0);
  fakefloat_free(freq_list_base);
  return status;
}

void
archive_u32_pair_from_u64(ULONG u32_idx, u32 *u32_list_base, u64 u64_0){
/*
Store a u64 into a pair of successive (u32)s in a list.

In:

  u32_idx is the index at which to write the low 32 bits of u64_0. Its high 32 bits will be written at the next index.

  *u32_list_base is a list of (u32)s to modify.

  u64_0 is the u64 to split and write at the foregoing indexes.

Out:

  *u32_list_base has been modified as described.
*/
  u32 u32_0;
  u32 u32_1;

  u32_0=(u32)(u64_0);
  u32_1=(u32)(u64_0>>U32_BITS);
  u32_list_base[u32_idx]=u32_0;
  u32_list_base[u32_idx+1]=u32_1;
  return;
}

u64
archive_u64_from_u32_pair(ULONG u32_idx, u32 *u32_list_base){
/*
Extract a u64 from a pair of successive (u32)s in a list.

In:

  u32_idx is the index at which to read the low 32 bits of u64_0. Its high 32 bits will be read at the next index.

  *u32_list_base is a list of (u32)s from which to extract.

Out:

  Returns the u64 merged from reads at the foregoing indexes.
*/
  u32 u32_0;
  u32 u32_1;
  u64 u64_0;

  u32_0=u32_list_base[u32_idx];
  u32_1=u32_list_base[u32_idx+1];
  u64_0=((u64)(u32_1)<<U32_BITS)+u32_0;
  return u64_0;
}
