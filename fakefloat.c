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
32-Bit Float Management via Unsigned Integers
*/
#include "flag.h"
#include "flag_fakefloat.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "fakefloat.h"
#include "fakefloat_xtrn.h"
#include "lmd2.h"

void *
fakefloat_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Fakefloat calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

ULONG *
fakefloat_freq_list_malloc(void){
/*
Allocate *freq_list_base for use with fakefloat_u32_list_sort().

Out:

  Returns NULL on failure, else freq_list_base.
*/
  ULONG *freq_list_base;

  freq_list_base=DEBUG_MALLOC_PARANOID(U16_SPAN<<ULONG_SIZE_LOG2);
  return freq_list_base;
}

u8
fakefloat_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of FAKEFLOAT_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of FAKEFLOAT_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=FAKEFLOAT_BUILD_BREAK_COUNT) or (build_feature_count>FAKEFLOAT_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=FAKEFLOAT_BUILD_BREAK_COUNT);
  status=(u8)(status|(FAKEFLOAT_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}

u32
fakefloat_to_slice_idx(ULONG slice_idx_max, ULONG slice_idx_min, u32 u32_0, u32 *u32_list_base){
/*
Given a sorted list of fakefloats, try to find the index of the most negative one which is at least as positive as a given fakefloat. If none are, then return the number of fakefloats in said list, plus one.

In:

  slice_idx_max is the index of the most positive fakefloat in a particular PDF at *slice_list_base. (slice_idx_max-slice_idx_min) must not exceed (U32_MAX-1) because if it did then the return value could wrap 32 bits. That would be bad because Whole assumes that whole numbers always fit in 32 bits.

  slice_idx_min is the index of the most negative fakefloat in the same PDF.

  u32_0 is the fakefloat for which to return the slice index.

  *u32_list_base is a list of fakefloats which are defined on the index range [slice_idx_min, slice_idx_max].

Out:

  Returns the slice index of u32_0 according to the rules stated in the summary. On [0, (slice_idx_max+1)].
*/
  u8 exp;
  u32 mantissa;
  u8 sign_status;
  ULONG slice_idx;
  u32 slice_idx_local;
  u8 status;
  u32 u32_1;

  exp=FLOAT_U32_EXP_GET(u32_0);
  mantissa=FLOAT_U32_MANTISSA_GET(u32_0);
  sign_status=FLOAT_U32_IS_SIGNED(u32_0);
  u32_1=u32_list_base[slice_idx_max];
  FLOAT_U32_IS_LESS_EQUAL(exp, mantissa, sign_status, u32_1, status);
  slice_idx_local=(u32)(slice_idx_min);
  if(status){  
    slice_idx_local=(u32)(slice_idx_min);
    do{
      slice_idx=slice_idx_min+((slice_idx_max-slice_idx_min)>>1);
      u32_1=u32_list_base[slice_idx];
      FLOAT_U32_IS_LESS_EQUAL(exp, mantissa, sign_status, u32_1, status);
      if(status){
        slice_idx_max=slice_idx;
      }else{
        slice_idx_min=slice_idx+1;
      }
    }while(slice_idx_min<slice_idx_max);
    slice_idx_local=(u32)(slice_idx_min)-slice_idx_local;
  }else{
    slice_idx_local=(u32)(slice_idx_max)-slice_idx_local+1;
  }
  return slice_idx_local;
}

u8
fakefloat_u32_list_check(u32 *checksum_base, u64 *lmd2_iterand_base, u64 *lmd2_partial_base, u8 *sort_status_base, ULONG u32_idx_max, ULONG u32_idx_min, ULONG *u32_idx_min_infinity_base, ULONG *u32_idx_min_nan_base, ULONG *u32_idx_min_negative_base, ULONG *u32_idx_min_subnormal_base, ULONG *u32_idx_min_zero_minus_base, ULONG *u32_idx_min_zero_plus_base, u32 *u32_list_base){
/*
Given a list of (u32)s intended to be interpreted as floats, compute its partial LMD2 hash while checking for NaNs, infinities, subnormals, negatives, and positive or negative zeroes.

In:

  *checksum_base is zero on the first call and fed back thereafter.

  *lmd2_iterand_base is zero on the first call and fed back thereafter, provided that subsequent calls have u32_idx_min as greater than the previous u32_idx_max.

  *lmd2_partial_base is undefined.

  sort_status_base is one on the first call, or otherwise its previous output value if the purpose is to AND together many such instances.

  u32_idx_max is the index of the last (u32) to check at u32_list_base.

  u32_idx_min is the index of the first (u32) to check at u32_list_base.

  *u32_idx_min_infinity_base is ULONG_MAX to update itself on return, or anything else not to.

  *u32_idx_min_nan_base is ULONG_MAX to update itself on return, or anything else not to.

  *u32_idx_min_negative_base is ULONG_MAX to update itself on return, or anything else not to.

  *u32_idx_min_subnormal_base is ULONG_MAX to update itself on return, or anything else not to.

  *u32_idx_zero_min_minus_base is ULONG_MAX to update itself on return, or anything else not to.

  *u32_idx_zero_min_plus_base is ULONG_MAX to update itself on return, or anything else not to.

  *u32_list_base contains (u32_idx_max-u32_idx_min+1) (u32)s to inspect as though they were floats.

Out:

  Returns one if any of the following values is were changed from ULONG_MAX during this call, else zero: *u32_idx_min_infinity_base, *u32_idx_min_nan_base, *u32_idx_min_negative_base, *u32_idx_min_subnormal_base, *u32_idx_min_zero_minus_base, or *u32_idx_min_zero_plus_base.

  *checksum_base is the sum of all the (u32)s at u32_list_base with indexes in the range from u32_idx_min through u32_idx_max.

  *lmd2_iterand_base has been updated for feedback.

  *lmd2_partial_base is the partial (nonfinalized) LMD2 hash of *u32_list_base over the specified index range.

  sort_status_base is zero if *u32_list_base is not sorted in a manner consistent with fakefloat_u32_list_sort() with sign_status set to one, else unchanged from its input value.

  *u32_idx_min_infinity_base is the index into *u32_list_base of the first instance of infinity of either sign, or ULONG_MAX if none.

  *u32_idx_min_nan_base is the index into *u32_list_base of the first instance of a not-a-number (NaN) of either sign, or ULONG_MAX if none.

  *u32_idx_min_negative_base is the index into *u32_list_base of the first instance of a signed nornmal, or ULONG_MAX if none.

  *u32_idx_min_subnormal_base is the index into *u32_list_base of the first instance of a subnormal of either sign, or ULONG_MAX if none.

  *u32_idx_min_zero_minus_base the index into *u32_list_base of the first instance of negative zero, or ULONG_MAX if none.

  *u32_idx_min_zero_plus_base is the index into *u32_list_base of the first instance of positive zero, or ULONG_MAX if none.
*/
  u32 checksum;
  u8 exp;
  u64 lmd2;
  u32 lmd2_c;
  u64 lmd2_iterand;
  u32 lmd2_x;
  u32 mantissa;
  u8 sign_status;
  u8 sort_status;
  u8 status;
  u32 u32_0;
  u32 u32_1;
  ULONG u32_idx;
  ULONG u32_idx_min_infinity;
  ULONG u32_idx_min_nan;
  ULONG u32_idx_min_negative;
  ULONG u32_idx_min_subnormal;
  ULONG u32_idx_min_zero_minus;
  ULONG u32_idx_min_zero_plus;

  sort_status=1;
  status=0;
  lmd2_iterand=*lmd2_iterand_base;
  if(lmd2_iterand){
    lmd2_c=(u32)(lmd2_iterand>>U32_BITS);
    lmd2_x=(u32)(lmd2_iterand);
  }else{
    LMD_SEED_INIT(LMD2_C0, lmd2_c, LMD2_X0, lmd2_x)
  }
  LMD_ACCUMULATOR_INIT(lmd2)
  checksum=*checksum_base;
  u32_1=U32_MAX;
  u32_idx=u32_idx_min;
  u32_idx_min_infinity=*u32_idx_min_infinity_base;
  u32_idx_min_nan=*u32_idx_min_nan_base;
  u32_idx_min_negative=*u32_idx_min_negative_base;
  u32_idx_min_subnormal=*u32_idx_min_subnormal_base;
  u32_idx_min_zero_minus=*u32_idx_min_zero_minus_base;
  u32_idx_min_zero_plus=*u32_idx_min_zero_plus_base;
  do{
    u32_0=u32_list_base[u32_idx];
    LMD_ITERATE_WITH_ZERO_CHECK(LMD2_A, lmd2_c, lmd2_x, lmd2_iterand)
    if(sort_status){
      exp=FLOAT_U32_EXP_GET(u32_1);
      mantissa=FLOAT_U32_MANTISSA_GET(u32_1);
      sign_status=FLOAT_U32_IS_SIGNED(u32_1);
      u32_1=u32_0;
      FLOAT_U32_IS_LESS_EQUAL(exp, mantissa, sign_status, u32_0, sort_status);
    }
    checksum+=u32_0;
    if((!~u32_idx_min_nan)&&FLOAT_U32_IS_NAN(u32_0)){
      status=1;
      u32_idx_min_nan=u32_idx;
    }else if((!~u32_idx_min_infinity)&&FLOAT_U32_IS_INFINITY(u32_0)){
      status=1;
      u32_idx_min_infinity=u32_idx;
    }else if((!~u32_idx_min_subnormal)&&FLOAT_U32_IS_SUBNORMAL(u32_0)){
      status=1;
      u32_idx_min_subnormal=u32_idx;
    }else if(FLOAT_U32_IS_ZERO(u32_0)){
      if(FLOAT_U32_IS_SIGNED(u32_0)){
        if(!~u32_idx_min_zero_minus){
          status=1;
          u32_idx_min_zero_minus=u32_idx;
        }
      }else if(!~u32_idx_min_zero_plus){
        status=1;
        u32_idx_min_zero_plus=u32_idx;
      }
    }else if(FLOAT_U32_IS_SIGNED(u32_0)){
      if(!~u32_idx_min_negative){
        status=1;
        u32_idx_min_negative=u32_idx;
      }
    }
    LMD_ACCUMULATE(u32_0, lmd2_x, lmd2)
  }while((u32_idx++)!=u32_idx_max);
  if(!sort_status){
    *sort_status_base=sort_status;
  }
  *checksum_base=checksum;
  *lmd2_iterand_base=lmd2_iterand;
  *lmd2_partial_base=lmd2;
  *u32_idx_min_infinity_base=u32_idx_min_infinity;
  *u32_idx_min_nan_base=u32_idx_min_nan;
  *u32_idx_min_negative_base=u32_idx_min_negative;
  *u32_idx_min_subnormal_base=u32_idx_min_subnormal;
  *u32_idx_min_zero_minus_base=u32_idx_min_zero_minus;
  *u32_idx_min_zero_plus_base=u32_idx_min_zero_plus;
  return status;
}

ULONG
fakefloat_u32_list_empty_check(ULONG u32_idx_max, ULONG u32_idx_min, u32 *u32_list_base){
/*
Verify that a list of (u32)s contains only (U32_MAX)s.

In:

  u32_idx_max is the index of the last (u32) to check at u32_list_base.

  u32_idx_min is the index of the first (u32) to check at u32_list_base.

  *u32_idx_min_nonzero_base is undefined.

  *u32_list_base contains (u32_idx_max+1) (u32)s, expected to be all (U32_MAX)s.

Out:

  Returns the index into *u32_list_base of the first u32 which isn't U32_MAX, or ULONG_MAX if none.
*/
  ULONG u32_idx;
  ULONG u32_idx_min_nonzero;

  u32_idx=u32_idx_min;
  u32_idx_min_nonzero=ULONG_MAX;
  do{
    if((u32)(u32_idx_min_nonzero)!=u32_list_base[u32_idx]){
      u32_idx_min_nonzero=u32_idx;
      break;
    }
  }while((u32_idx++)!=u32_idx_max);
  return u32_idx_min_nonzero;
}

u8
fakefloat_u32_list_find(u32 u32_0, ULONG u32_idx_max, ULONG *u32_idx_min_base, u32 *u32_list_base){
/*
Find the first occurrence of a given u32 in a list.

In:

  u32_0 is the u32 to find.

  u32_idx_max is the index of the last (u32) to check at u32_list_base.

  *u32_idx_min_base is the index of the first (u32) to check at u32_list_base.

  *u32_list_base contains (u32_idx_max+1) (u32)s.

Out:

  Returns one if a match was found, else zero. Always zero if u32_idx_max was less than *u32_idx_min_base.

  *u32_idx_min_base is the least index at which u32_0 was found if the return value is one, else unchanged.
*/
  u8 status;
  ULONG u32_idx;

  status=0;
  u32_idx=*u32_idx_min_base;
  while(u32_idx<=u32_idx_max){
    if(u32_0==u32_list_base[u32_idx]){
      status=1;
      *u32_idx_min_base=u32_idx;
      break;
    }
    u32_idx++;
  }
  return status;
}

u64
fakefloat_u32_list_hash(u64 *lmd2_iterand_base, ULONG u32_idx_max, ULONG u32_idx_min, u32 *u32_list_base){
/*
Given a list of (u32)s intended to be interpreted as floats, compute its partial LMD2 hash and the postterminal state of its associated iterator.

In:

  *lmd2_iterand_base is zero on the first call and fed back thereafter, provided that subsequent calls have u32_idx_min as one more than the previous u32_idx_max.

  u32_idx_max is the index of the last (u32) to check at u32_list_base.

  u32_idx_min is the index of the first (u32) to check at u32_list_base.

  *u32_list_base contains (u32_idx_max-u32_idx_min+1) (u32)s to inspect as though they were floats.

Out:

  Returns the partial (nonfinalized) LMD2 of *u32_list_base over the specified index range.

  *lmd2_iterand_base has been updated for feedback.
*/
  u64 lmd2;
  u32 lmd2_c;
  u64 lmd2_iterand;
  u32 lmd2_x;
  u32 u32_0;
  ULONG u32_idx;

  lmd2_iterand=*lmd2_iterand_base;
  if(lmd2_iterand){
    lmd2_c=(u32)(lmd2_iterand>>U32_BITS);
    lmd2_x=(u32)(lmd2_iterand);
  }else{
    LMD_SEED_INIT(LMD2_C0, lmd2_c, LMD2_X0, lmd2_x)
  }
  LMD_ACCUMULATOR_INIT(lmd2)
  u32_idx=u32_idx_min;
  do{
    u32_0=u32_list_base[u32_idx];
    LMD_ITERATE_WITH_ZERO_CHECK(LMD2_A, lmd2_c, lmd2_x, lmd2_iterand)
    LMD_ACCUMULATE(u32_0, lmd2_x, lmd2)
  }while((u32_idx++)!=u32_idx_max);
  *lmd2_iterand_base=lmd2_iterand;
  return lmd2;
}

void
fakefloat_u32_list_inject(ULONG block_idx_max, ULONG from_u32_idx_min, u32 *from_u32_list_base, ULONG inject_u32_count, ULONG to_block_u32_count, ULONG to_u32_idx_min, u32 *to_u32_list_base){
/*
Copy a u32 sublist into another list, skip over a constant number of (u32)s in the latter, and repeat as required.

In:

  block_idx_max is the number of blocks, less one. Each block contains a sequence of (u32)s to read or write followed by another such sequence to skip. The read and write block sizes are generally different.

  from_u32_idx_min is the index into *from_u32_list_base at which to read the first u32.

  *from_u32_list_base is the u32 list to read, which must be defined on [from_u32_idx_min, from_u32_idx_min+inject_u32_count*(block_idx_max+1)-1].

  inject_u32_count is the number of successive (u32)s to copy before each skip. It's also the number of (u32)s per block at from_u32_list_base.

  to_block_u32_count is the number of (u32)s per block at to_u32_list_base.

  to_u32_idx_min is the index into *to_u32_list_base at which to write the first u32.

  *to_u32_list_base is the u32 list to write, which must be allocated through index (to_u32_idx_min+to_block_u32_count*block_idx_max+inject_u32_count-1). The write region must not overlap the read region at *from_u32_list_base.

Out:

  The interleaved copy has been performed as described in the summary.

  *to_u32_list_base is modified accordingly.
*/
  ULONG from_u32_idx;
  ULONG inject_size;
  ULONG to_u32_idx;

  from_u32_idx=from_u32_idx_min;
  inject_size=inject_u32_count<<U32_SIZE_LOG2;
  to_u32_idx=to_u32_idx_min;
  do{
    memcpy(&to_u32_list_base[to_u32_idx], &from_u32_list_base[from_u32_idx], (size_t)(inject_size));
    from_u32_idx+=inject_u32_count;
    to_u32_idx+=to_block_u32_count;
  }while(block_idx_max--);
  return;
}

void
fakefloat_u32_list_interleave(ULONG block_idx_max, ULONG interleave_u32_count, ULONG to_block_u32_count, ULONG to_block_u32_count_new, ULONG to_u32_idx_min, u32 *to_u32_list_base){
/*
Change the number of free (u32)s (denoted by U32_MAX) between existing blocks of defined (u32)s.

In:

  block_idx_max is the number of blocks, less one. Each block contains a sequence of (u32)s to read or write followed by another such sequence to skip. The read and write block sizes are generally different.

  interleave_u32_count is the number of successive (u32)s to copy before each skip.

  to_block_u32_count is the existing number of (u32)s per block at *to_u32_list_base.

  to_block_u32_count_new is desired the number of (u32)s per block at *to_u32_list_base.

  to_u32_idx_min is the index into *to_u32_list_base at which to read and write the first u32. (The first block does not actually move.)

  *to_u32_list_base is the u32 list to read and write, which must be allocated on [to_i32_idx_min, to_u32_idx_min+MAX(to_block_u32_count, to_block_u32_count_new)*block_idx_max+interleave_u32_count-1] and defined on [to_i32_idx_min, to_u32_idx_min+to_block_u32_count*block_idx_max+interleave_u32_count-1].

Out:

  The interleaved copy has been performed as described in the summary.

  *to_u32_list_base is modified accordingly.
*/
  ULONG empty_size;
  ULONG empty_u32_count;
  ULONG interleave_size;
  ULONG to_u32_idx;
  ULONG to_u32_idx_new;
  ULONG to_u32_idx_new_delta;

  empty_u32_count=to_block_u32_count_new-interleave_u32_count;
  empty_size=empty_u32_count<<U32_SIZE_LOG2;
  interleave_size=interleave_u32_count<<U32_SIZE_LOG2;
  if(to_block_u32_count<=to_block_u32_count_new){
    to_u32_idx=block_idx_max*to_block_u32_count+to_u32_idx_min;
    to_u32_idx_new=block_idx_max*to_block_u32_count_new+to_u32_idx_min;
    to_u32_idx_new_delta=empty_u32_count-(to_block_u32_count_new<<1);
    do{
      if(to_block_u32_count<=(to_u32_idx_new-to_u32_idx)){
        memcpy(&to_u32_list_base[to_u32_idx_new], &to_u32_list_base[to_u32_idx], (size_t)(interleave_size));
      }else{
        memmove(&to_u32_list_base[to_u32_idx_new], &to_u32_list_base[to_u32_idx], (size_t)(interleave_size));
      }
      to_u32_idx-=to_block_u32_count;
      to_u32_idx_new+=interleave_u32_count;
      memset(&to_u32_list_base[to_u32_idx_new], U8_MAX, (size_t)(empty_size));
      to_u32_idx_new+=to_u32_idx_new_delta;
    }while(block_idx_max--);
  }else{
    to_u32_idx=to_block_u32_count+to_u32_idx_min;
    to_u32_idx_new=to_block_u32_count_new+to_u32_idx_min;
    while(block_idx_max--){
      if(to_block_u32_count_new<=(to_u32_idx-to_u32_idx_new)){
        memcpy(&to_u32_list_base[to_u32_idx_new], &to_u32_list_base[to_u32_idx], (size_t)(interleave_size));
      }else{
        memmove(&to_u32_list_base[to_u32_idx_new], &to_u32_list_base[to_u32_idx], (size_t)(interleave_size));
      }
      to_u32_idx+=to_block_u32_count;
      to_u32_idx_new+=interleave_u32_count;
      memset(&to_u32_list_base[to_u32_idx_new], U8_MAX, (size_t)(empty_size));
      to_u32_idx_new+=empty_u32_count;
    }
  }
  return;
}

void
fakefloat_u32_list_interleave_inject(ULONG block_idx_max, ULONG from_u32_idx_min, u32 *from_u32_list_base, ULONG inject_u32_count, ULONG interleave_u32_count, ULONG to_block_u32_count, ULONG to_block_u32_count_new, ULONG to_u32_idx_min, u32 *to_u32_list_base){
/*
Change the number of free (u32)s (denoted by U32_MAX) between existing blocks of defined (u32)s, then inject a sublists of (u32)s from another list into those slots.

In:

  block_idx_max is the number of blocks, less one. Each block contains a sequence of (u32)s to read or write followed by another such sequence to skip. The read and write block sizes are generally different. In this case, there are 2 read block sizes and one write block size.

  from_u32_idx_min is the index into *from_u32_list_base at which to read the first u32.

  *from_u32_list_base is the u32 list to read, which must be defined on [from_u32_idx_min, from_u32_idx_min+inject_u32_count*(block_idx_max+1)-1].

  inject_u32_count is the number of successive (u32)s to copy from *from_u32_list_base to *to_u32_list_base before each skip. It's also the number of (u32)s per block at from_u32_list_base.

  interleave_u32_count is the number of successive (u32)s to copy from one region of *to_u32_list_base to another before copying inject_u32_count (u32)s from *from_list_base.

  to_block_u32_count is the existing number of (u32)s per block at *to_u32_list_base.

  to_block_u32_count_new is desired the number of (u32)s per block at *to_u32_list_base.

  to_u32_idx_min is the index into *to_u32_list_base at which to write the first u32.

  *to_u32_list_base is the u32 list to read and write, which must be allocated on [to_u32_idx_min, to_u32_idx_min+MAX(to_block_u32_count, to_block_u32_count_new)*block_idx_max+interleave_u32_count+inject_u32_count-1] and defined on [to_u32_idx_min, to_u32_idx_min+to_block_u32_count*block_idx_max+interleave_u32_count+inject_u32_count-1].

Out:

  The interleaved copy from 2 different source regions has been performed as described in the summary.

  *to_u32_list_base is modified accordingly.
*/
  ULONG empty_size;
  ULONG empty_u32_count;
  ULONG from_u32_idx;
  ULONG inject_size;
  ULONG interleave_size;
  ULONG to_u32_idx;
  ULONG to_u32_idx_new;
  ULONG to_u32_idx_new_delta;

  empty_u32_count=to_block_u32_count_new-inject_u32_count-interleave_u32_count;
  to_u32_idx_new_delta=empty_u32_count-(to_block_u32_count_new<<1);
  empty_size=empty_u32_count<<U32_SIZE_LOG2;
  from_u32_idx=block_idx_max*inject_u32_count+from_u32_idx_min;
  inject_size=inject_u32_count<<U32_SIZE_LOG2;
  interleave_size=interleave_u32_count<<U32_SIZE_LOG2;
  to_u32_idx=block_idx_max*to_block_u32_count+to_u32_idx_min;
  to_u32_idx_new=block_idx_max*to_block_u32_count_new+to_u32_idx_min;
  do{
    if(to_block_u32_count<=(to_u32_idx_new-to_u32_idx)){
      memcpy(&to_u32_list_base[to_u32_idx_new], &to_u32_list_base[to_u32_idx], (size_t)(interleave_size));
    }else{
      memmove(&to_u32_list_base[to_u32_idx_new], &to_u32_list_base[to_u32_idx], (size_t)(interleave_size));
    }
    to_u32_idx-=to_block_u32_count;
    to_u32_idx_new+=interleave_u32_count;
    memcpy(&to_u32_list_base[to_u32_idx_new], &from_u32_list_base[from_u32_idx], (size_t)(inject_size));
    from_u32_idx-=inject_u32_count;
    to_u32_idx_new+=inject_u32_count;
    memset(&to_u32_list_base[to_u32_idx_new], U8_MAX, (size_t)(empty_size));
    to_u32_idx_new+=to_u32_idx_new_delta;
  }while(block_idx_max--);
  return;
}

u32 *
fakefloat_u32_list_malloc(u8 empty_status, ULONG u32_idx_max){
/*
Allocate a list of (u32)s to be interpreted as fake floats. (We don't allocate actual floats because they suffer from a host of ill-defined behavior.)

In:

  empty_status is one iff all allocated (u32)s should be set to U32_MAX.

  u32_idx_max is one less than the number of (u32)s to allocate.

Out:

  Returns NULL on failure, else the base of (u32_idx_max+1) undefined (if empty_status is zero) or (U32_MAX)ed (if empty_status is one) (u32)s.
*/
  u32 *u32_list_base;
  ULONG list_size;

  u32_list_base=NULL;
  list_size=(u32_idx_max+1)<<U32_SIZE_LOG2;
  if(u32_idx_max<(list_size>>U32_SIZE_LOG2)){
    u32_list_base=DEBUG_MALLOC_PARANOID(list_size);
    if(empty_status&&u32_list_base){
      memset(u32_list_base, U8_MAX, (size_t)(list_size));
    }
  }
  return u32_list_base;
}

u8
fakefloat_u32_list_realloc(ULONG u32_idx_max, u32 **u32_list_base_base){
/*
Alter the size of a list of (u32)s.

In:

  u32_idx_max is one less than the desired number of (u32)s.

  **u32_list_base_base is the u32 list the size of which to change.

Out:

  Returns zero on failure, else one, in which case *u32_list_base_base now points to (u32_idx_max+1) (u32)s, in which case all newly allocated (u32)s will be undefined.
*/
  u32 *u32_list_base;
  u32 *u32_list_base_new;
  ULONG list_size;
  u8 status;

  u32_list_base=*u32_list_base_base;
  u32_list_base_new=NULL;
  list_size=(u32_idx_max+1)<<U32_SIZE_LOG2;
  if(u32_idx_max<(list_size>>U32_SIZE_LOG2)){
    u32_list_base_new=DEBUG_REALLOC_PARANOID(u32_list_base, list_size);
  }
  status=1;
  if(u32_list_base_new){
    status=0;
    *u32_list_base_base=u32_list_base_new;
  }
  return status;
}

void
fakefloat_u32_list_sort(ULONG *freq_list_base, u8 sign_status, ULONG u32_idx_max, u32 *u32_list_base0, u32 *u32_list_base1){
/*
Sort a list of (u32)s ascending as though they were (un)signed integers, using double buffering for speed.

In:

  *freq_list_base is a list of U16_SPAN undefined (ULONG)s.

  sign_status is zero if sorting unsigned integers (or signed integers which all have the same sign), else one to sort signed integers or IEEE754 floats. Sort order will be from most negative to most positive, with (NaN)s further from positive and negative zero than infinities.

  u32_idx_max is one less than the number of (u32)s in each list.

  *u32_list_base0 contains (u32_idx_max+1) (u32)s to sort.

  *u32_list_base1 is undefined and writable for (u32_idx_max+1) (u32)s.

Out:

  *u32_list_base0 is sorted according to sign_status.

  *u32_list_base1 is undefined.
*/
  u32 freq_idx;
  u16 u16_0;
  u32 u32_0;
  ULONG u32_idx;
  ULONG u32_idx_delta;
  ULONG u32_idx_new;

  memset(freq_list_base, 0, (size_t)(U16_SPAN<<ULONG_SIZE_LOG2));
/*
Perform a base-U16_SPAN radix sort of *u32_list_base0, which requires just 2 passes.

After accounting for sign_status, count lane zero u16 frequencies and save them to *freq_list_base. Move backwards because the end of the list is more likely to be cached.
*/
  for(u32_idx=u32_idx_max; u32_idx<=u32_idx_max; u32_idx--){
    u32_0=u32_list_base0[u32_idx];
    u16_0=(u16)(u32_0);
    if((u32_0>>U32_BIT_MAX)&&sign_status){
      u16_0=(u16)(~u16_0);
    }
    freq_list_base[u16_0]++;
  }
/*
Convert lane zero u16 symbol frequencies into u32 base indexes.
*/
  u32_idx=0;
  for(freq_idx=0; freq_idx<=U16_MAX; freq_idx++){
    u32_idx_delta=freq_list_base[freq_idx];
    freq_list_base[freq_idx]=u32_idx;
    u32_idx+=u32_idx_delta;
  }
/*
Rearrange the (u32)s according the base indexes just computed, moving forwards to maximize cache hits.
*/
  for(u32_idx=0; u32_idx<=u32_idx_max; u32_idx++){
    u32_0=u32_list_base0[u32_idx];
    u16_0=(u16)(u32_0);
    if((u32_0>>U32_BIT_MAX)&&sign_status){
      u16_0=(u16)(~u16_0);
    }
    u32_idx_new=freq_list_base[u16_0];
    u32_list_base1[u32_idx_new]=u32_0;
    u32_idx_new++;
    freq_list_base[u16_0]=u32_idx_new;
  }
/*
Do the same for lane one u16 symbols, this time rearranging *u32_list_base1 back to *u32_list_base0.
*/
  memset(freq_list_base, 0, (size_t)(U16_SPAN<<ULONG_SIZE_LOG2));
  for(u32_idx=u32_idx_max; u32_idx<=u32_idx_max; u32_idx--){
    u32_0=u32_list_base1[u32_idx];
    u16_0=(u16)(u32_0>>U16_BITS);
    if(sign_status){
      if(u32_0>>U32_BIT_MAX){
        u16_0=(u16)(~u16_0);
      }else{
        u16_0=(u16)(u16_0+U16_SPAN_HALF);
      }
    }
    freq_list_base[u16_0]++;
  }
  u32_idx=0;
  for(freq_idx=0; freq_idx<=U16_MAX; freq_idx++){
    u32_idx_delta=freq_list_base[freq_idx];
    freq_list_base[freq_idx]=u32_idx;
    u32_idx+=u32_idx_delta;
  }
  for(u32_idx=0; u32_idx<=u32_idx_max; u32_idx++){
    u32_0=u32_list_base1[u32_idx];
    u16_0=(u16)(u32_0>>U16_BITS);
    if(sign_status){
      if(u32_0>>U32_BIT_MAX){
        u16_0=(u16)(~u16_0);
      }else{
        u16_0=(u16)(u16_0+U16_SPAN_HALF);
      }
    }
    u32_idx_new=freq_list_base[u16_0];
    u32_list_base0[u32_idx_new]=u32_0;
    u32_idx_new++;
    freq_list_base[u16_0]=u32_idx_new;
  }
  return;
}

u8
fakefloat_u32_list_sort_check(ULONG u32_idx_max, ULONG u32_idx_min, u32 *u32_list_base){
/*
Given a list of (u32)s intended to be interpreted as floats, determine if it's sorted consistent with the output of fakefloat_u32_list_sort().

In:

  u32_idx_max is the index of the last (u32) to check at u32_list_base.

  u32_idx_min is the index of the first (u32) to check at u32_list_base.

  *u32_list_base contains (u32_idx_max-u32_idx_min+1) (u32)s to inspect as though they were floats.

Out:

  Returns one if the values are consistent with a possible output of fake_float_u32_list_sort(), else zero.
*/
  u8 exp;
  u32 mantissa;
  u8 sign_status;
  u8 status;
  u32 u32_0;
  u32 u32_1;
  ULONG u32_idx;

  status=1;
  u32_1=U32_MAX;
  u32_idx=u32_idx_min;
  do{
    u32_0=u32_list_base[u32_idx];
    exp=FLOAT_U32_EXP_GET(u32_1);
    mantissa=FLOAT_U32_MANTISSA_GET(u32_1);
    sign_status=FLOAT_U32_IS_SIGNED(u32_1);
    u32_1=u32_0;
    FLOAT_U32_IS_LESS_EQUAL(exp, mantissa, sign_status, u32_0, status);
  }while(status&&((u32_idx++)!=u32_idx_max));
  return status;
}

void
fakefloat_u32_list_transpose(u32 *u32_list_base0, u32 *u32_list_base1, ULONG u32_x_idx_max, ULONG u32_y_idx_max){
/*
Exchange the indexes of a 2D array of (u32)s.

In:

  u32_list_base0 is the base of the array to transpose.

  u32_list_base1 is the base of an array which is allocated for (u32_x_idx_max+1)*(u32_y_idx_max+1) (u32)s.

  u32_x_idx_max is the number of (u32)s in each row of the array, less one.

  u32_y_idx_max is the number of rows in the array, less one.

Out:

  *u32_list_base contains the transposed array, which thus has (u32_y_idx_max+1) (u32)s in each row and (u32_x_idx_max+1) rows.
*/
  u32 u32_0;
  ULONG u32_idx_max;
  ULONG u32_y_idx_post;
  ULONG u32_idx0;
  ULONG u32_idx1;

  u32_idx0=0;
  u32_idx1=0;
  u32_y_idx_post=u32_y_idx_max+1;
  u32_idx_max=u32_x_idx_max*u32_y_idx_max+u32_x_idx_max+u32_y_idx_max;
  do{
    do{
      u32_0=u32_list_base0[u32_idx0];
      u32_idx0++;
      u32_list_base1[u32_idx1]=u32_0;
      u32_idx1+=u32_y_idx_post;
    }while(u32_idx1<=u32_idx_max);
    u32_idx1-=u32_idx_max;
  }while(u32_idx1!=u32_y_idx_post);
  return;
}

ULONG
fakefloat_u32_sublist_compact(ULONG u32_idx_max, ULONG u32_idx_min, u32 *u32_list_base){
/*
Remove all free (u32)s (denoted by U32_MAX) from a sublist of a list, potentially leaving undefined (u32)s at the end of said sublist.

In:

  u32_idx_max is the last index of the sublist.
  
  u32_idx_min is the first index of the sublist.

  u32_list_base is the base of the list of (u32)s to compact.

Out:

  Returns the postterminal index of the sublist after scrolling over all of its (U32_MAX)s.

  All free (u32)s in the indicated region have been scrolled over by values other than (U32_MAX) read from successively higher indexes. If N is the number of (U32_MAX)s scrolled over, then the last N (u32)s of the sublist are undefined.
*/
  u32 u32_0;
  ULONG u32_idx_read;
  ULONG u32_idx_write;

  u32_idx_read=u32_idx_min;
  u32_idx_write=u32_idx_min;  
  do{
    u32_0=u32_list_base[u32_idx_read];
    if(~u32_0){
      u32_list_base[u32_idx_write]=u32_0;
      u32_idx_write++;
    }
  }while((u32_idx_read++)!=u32_idx_max);
  return u32_idx_write;
}

u8
fakefloat_u32_sublist_redact(ULONG from_u32_idx_max, ULONG from_u32_idx_min, u32 *from_u32_list_base, ULONG to_u32_idx_max, ULONG to_u32_idx_min, u32 *to_u32_list_base){
/*
Locate the first occurrence of each u32 from a source list in a sorted target list. Replace each such occurrence in the target list with U32_MAX, such that the set of (u32)s thus replaced matches the source list in both length and multiplicity.

In:

  from_u32_idx_max is the last index of the sublist at from_u32_list_base.

  from_u32_idx_min is the first index of the sublist at from_u32_list_base.

  *from_u32_list_base is the source list of (u32)s.

  to_u32_idx_max is the last index of the sublist at to_u32_list_base.

  to_u32_idx_min is the first index of the sublist at to_u32_list_base.

  *to_u32_list_base is the target list of (u32)s. It must be sorted as though the (u32)s are just that -- not (signed) fakefloats.

Out:

  Returns one if at least one of the requested (u32)s could not be found in the target sublist, in which case the latter should be regarded as corrupt.

  The seletive redaction has been performed as described in the summary.

  *to_u32_list_base is modified accordingly.
*/
  u32 from_u32;
  ULONG from_u32_idx;
  u8 status;
  u32 to_u32;
  ULONG to_u32_idx;

  status=0;
  from_u32_idx=from_u32_idx_min;
  to_u32_idx=to_u32_idx_min;
  do{
    from_u32=from_u32_list_base[from_u32_idx];
    do{
      to_u32=to_u32_list_base[to_u32_idx];
      if((from_u32<=to_u32)&&(~to_u32)){
        break;
      }
      to_u32_idx++;
    }while(to_u32_idx<=to_u32_idx_max);
    if((to_u32_idx_max<to_u32_idx)||(from_u32!=to_u32)){
      status=1;
      break;
    }
    to_u32_list_base[to_u32_idx]=U32_MAX;
    if((to_u32_idx==to_u32_idx_max)&&(from_u32_idx!=from_u32_idx_max)){
      status=1;
      break;
    }
    from_u32_idx++;
    to_u32_idx++;
  }while(from_u32_idx<=from_u32_idx_max);
  return status;
}
