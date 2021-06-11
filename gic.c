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
Gaussian Information Criterion Functions

See https://vixra.org/abs/2106.0036 .
*/
#include "flag.h"
#include "flag_gic.h"
#include <math.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "gic.h"
#include "gic_xtrn.h"

void *
gic_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which GIC calls free().

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
gic_gauss_list_fill(ULONG float_idx_min, float *float_list_base, gauss_t *gauss_list_base, gic_t *gic_base, u8 log_status, ULONG slot_count){
/*
Fill a list of means and variances which are implied by a list of lists of (float)s.

In:

  float_idx_min is the index of the first float in the first list at float_list_base.

  float_list_base contains lists of slot_count (float)s, only the first (gic_base->float_count) of which being defined.

  *gauss_list_base is the return value of gic_gauss_list_malloc(gic_base).

  *gic_base is the return value of gic_malloc().

  log_status is one to take the natural logarithm of each float before computing the mean and variance.

  slot_count is the number of defined, followed by undefined, (float)s per PDF at *float_list_base.

Out:

  Returns one if log_status was one and a nonpositive (float) was encountered, which is a fatal error. Else zero.

  *gauss_list_base contains (gic_base->pdf_count) (gauss_t)s derived from *float_list_base.
*/
  double delta;
  ULONG float_count;
  double float_count_recip;
  ULONG float_idx;
  ULONG float_idx_max;
  float float0;
  double mean;
  ULONG pdf_idx;
  ULONG pdf_count;
  u8 status;
  double variance;
  double zero;

  status=0;
  float_count=(ULONG)(gic_base->float_count);
  float_count_recip=1/(double)(float_count);
  float_idx_max=float_idx_min+float_count-1;
  pdf_idx=0;
  pdf_count=(ULONG)(gic_base->pdf_count);
  zero=0;
  do{
    mean=zero;
    for(float_idx=float_idx_min; float_idx<=float_idx_max; float_idx++){
      float0=float_list_base[float_idx];
      if(!log_status){
        mean+=float0;
      }else{
        if(float0<=zero){
          status=1;
          break;
        }
        mean+=log(float0);
      }
    }
    mean*=float_count_recip;
    variance=zero;
    for(float_idx=float_idx_min; float_idx<=float_idx_max; float_idx++){
      delta=float_list_base[float_idx];
      if(log_status){
        delta=log(delta);
      }
      delta-=mean;
      variance+=delta*delta;
    }
    variance*=float_count_recip;
    gauss_list_base[pdf_idx].mean=mean;
    gauss_list_base[pdf_idx].variance=variance;
    float_idx_min+=slot_count;
    float_idx_max+=slot_count;
  }while((++pdf_idx)!=pdf_count);
  return status;
}

gauss_t *
gic_gauss_list_malloc(gic_t *gic_base){
/*
Allocate a list of (gauss_t)s.

In:

  *gic_base is the return value of gic_malloc().

Out:

  Returns NULL on failure, else the base of a list of (gic_base->pdf_count) (gauss_t)s.
*/
  gauss_t *gauss_list_base;
  ULONG list_size;
  ULONG pdf_count;

  gauss_list_base=NULL;
  pdf_count=(ULONG)(gic_base->pdf_count);
  list_size=pdf_count*(ULONG)(sizeof(gauss_t));
  if((list_size/pdf_count)==(ULONG)(sizeof(gauss_t))){
    gauss_list_base=DEBUG_MALLOC_PARANOID(list_size); 
  }
  return gauss_list_base;
}

u8
gic_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of GIC_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of GIC_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=GIC_BUILD_BREAK_COUNT) or (build_feature_count>GIC_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=GIC_BUILD_BREAK_COUNT);
  status=(u8)(status|(GIC_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}

gic_t *
gic_malloc(ULONG float_count, ULONG pdf_count){
/*
Allocate private data structure.

In:

  float_count is the number of defined (float)s per PDF. This is K in the GIC.

  pdf_count is the number of PDFs.

Out:

  Returns the base of a private data structure for use with other functions.
*/
  gic_t *gic_base;

  gic_base=NULL;
  if(float_count&&pdf_count){
    gic_base=DEBUG_MALLOC_PARANOID(sizeof(gic_t));
    if(gic_base){
      gic_base->float_count=float_count;
      gic_base->pdf_count=pdf_count;
    }
  }
  return gic_base;
}

void
gic_ranker_list_copy(gic_t *gic_base, gic_ranker_t *gic_ranker_list_base0, gic_ranker_t *gic_ranker_list_base1){
/*
Copy one list of (gic_ranker_t)s to another.

In:

  *gic_base is the return value of gic_malloc().

  *gic_ranker_list_base0 is the return value of gic_ranker_list_malloc(gic_base) for the target list.

  *gic_ranker_list_base1 is the return value of gic_ranker_list_malloc(gic_base) for the source list.

Out:

  *gic_ranker_list_base0 equals *gic_ranker_list_base1.
*/
  ULONG list_size;

  list_size=(ULONG)(gic_base->pdf_count)*(ULONG)(sizeof(gic_ranker_t));
  memcpy(gic_ranker_list_base0, gic_ranker_list_base1, list_size);
  return;
}

void
gic_ranker_list_fill(gauss_t *gauss_list_base0, gauss_t *gauss_list_base1, gic_t *gic_base, gic_ranker_t *gic_ranker_list_base){
/*
Fill a list of PDF indexes and GIC rankers as which are implied by a list of means and variances.

In:

  *gauss_list_base0 has passed through gic_gauss_list_fill() for the source set of PDFs, which are presumed Gaussian.

  *gauss_list_base1 has passed through gic_gauss_list_fill() for the target set of PDFs, which are modeled as Gaussian but not actually presumed as such.

  *gic_base is the return value of gic_malloc().

  *gic_ranker_list_base is the return value of gic_ranker_list_malloc(gic_base).

Out:

  *gic_ranker_list_base contains (gic_base->pdf_count) (gauss_t)s derived from *gauss_list_base0 and *gauss_list_base1.
*/
  ULONG float_count;
  double float_count_double;
  ULONG gauss_idx;
  double mean0;
  double mean1;
  ULONG pdf_count;
  double ranker;
  double variance0;
  double variance1;

  float_count=(ULONG)(gic_base->float_count);
  float_count_double=(double)(float_count);
  pdf_count=(ULONG)(gic_base->pdf_count);
  for(gauss_idx=0; gauss_idx!=pdf_count; gauss_idx++){
/*
Compute the GIC ranker from gauss_list_base0[gauss_idx] to gauss_list_base1[gauss_idx].

r(μ1, σ1) = (K/[σ0^2])([σ1^2]+[μ1-μ0]^2)
*/
    mean0=gauss_list_base0[gauss_idx].mean;
    variance0=gauss_list_base0[gauss_idx].variance;
    mean1=gauss_list_base1[gauss_idx].mean;
    variance1=gauss_list_base1[gauss_idx].variance;
    ranker=mean1-mean0;
    ranker=float_count_double/variance0*(ranker*ranker+variance1);
    gic_ranker_list_base[gauss_idx].pdf_idx=gauss_idx;
    gic_ranker_list_base[gauss_idx].ranker=ranker;
  }
  return;
}

gic_ranker_t *
gic_ranker_list_malloc(gic_t *gic_base){
/*
Allocate a list of (gic_ranker_t)s.

In:

  *gic_base is the return value of gic_malloc().

Out:

  Returns NULL on failure, else the base of a list of (gic_base->pdf_count) (gic_ranker_t)s.
*/
  gic_ranker_t *gic_ranker_list_base;
  ULONG list_size;
  ULONG pdf_count;

  gic_ranker_list_base=NULL;
  pdf_count=(ULONG)(gic_base->pdf_count);
  list_size=pdf_count*(ULONG)(sizeof(gic_ranker_t));
  if((list_size/pdf_count)==(ULONG)(sizeof(gic_ranker_t))){
    gic_ranker_list_base=DEBUG_MALLOC_PARANOID(list_size); 
  }
  return gic_ranker_list_base;
}

void
gic_ranker_list_scale(gic_t *gic_base, gic_ranker_t *gic_ranker_list_base){
/*
Scale the GIC rankers in a list of (gic_ranker_t)s by the number of floats used to derive them (the GIC K value). The idea is to maximize precision by deferring this step this until after GIC ratios have been evaluated, in which the factors of K generally cancel.

In:

  *gic_base is the return value of gic_malloc().

  *gic_ranker_list_base has passed through gic_ranker_list_fill().

Out:

  All (ranker)s at *gic_ranker_list_base have been multiplied by (gic_base->float_count).
*/
  ULONG float_count;
  double float_count_double;
  ULONG gic_ranker_idx;
  ULONG pdf_count;

  float_count=(ULONG)(gic_base->float_count);
  float_count_double=(double)(float_count);
  pdf_count=(ULONG)(gic_base->pdf_count);
  for(gic_ranker_idx=0; gic_ranker_idx!=pdf_count; gic_ranker_idx++){
    gic_ranker_list_base[gic_ranker_idx].ranker*=float_count_double;
  }
  return;
}

u8
gic_ranker_list_sort(u8 ascending_status, gic_t *gic_base, gic_ranker_t *gic_ranker_list_base0, gic_ranker_t *gic_ranker_list_base1){
/*
Sort a list of (gic_ranker_t)s by their (ranker)s (which in practice might as well be GIC ratios as actual GIC rankers).

In:

  ascending_status is one to zero ascending, else zero.

  *gic_base is the return value of gic_malloc().

  *gic_ranker_list_base0 is the return value of gic_ranker_list_malloc(gic_base) and has passed through gic_ranker_list_fill() and optionally gic_ranker_list_to_ratio_list().

  *gic_ranker_list_base1 is the return value of gic_ranker_list_malloc(gic_base).

Out:

  Returns one on failure due to memory exhaustion, else zero.

  *gic_ranker_list_base0 is sorted as described in the summary in the direction instructed by ascending_status.

  *gic_ranker_list_base1 is undefined.
*/
  u8 bit_idx;
  u32 freq_idx;
  ULONG *freq_list_base;
  ULONG gic_ranker_idx;
  ULONG gic_ranker_idx_delta;
  ULONG gic_ranker_idx_max;
  ULONG gic_ranker_idx_new;
  gic_ranker_t* gic_ranker_list_base2;
  u64 pdf_idx;
  u8 status;
  u16 u16_0;
  u16 u16_xor_mask;
  u64 u64_0;
/*
Perform a base-U16_SPAN radix sort of *gic_ranker_list_base0, which requires just 4 passes because pdf_idx is ignored.
*/
  freq_list_base=DEBUG_MALLOC_PARANOID(U16_SPAN<<ULONG_SIZE_LOG2);
  status=1;
  if(freq_list_base){
    bit_idx=0;
    gic_ranker_idx_max=(ULONG)(gic_base->pdf_count)-1;
    u16_xor_mask=ascending_status?0:U16_MAX;
    do{
      memset(freq_list_base, 0, (size_t)(U16_SPAN<<ULONG_SIZE_LOG2));
/*
Count u16 frequencies and save them to *freq_list_base. Move backwards because the end of the list is more likely to be cached.
*/
      for(gic_ranker_idx=gic_ranker_idx_max; gic_ranker_idx<=gic_ranker_idx_max; gic_ranker_idx--){
        memcpy(&u64_0, &gic_ranker_list_base0[gic_ranker_idx].ranker, U64_SIZE);
        u16_0=(u16)(u64_0>>bit_idx)^u16_xor_mask;
        freq_list_base[u16_0]++;
      }
/*
Convert u16 symbol frequencies into u64 base indexes.
*/
      gic_ranker_idx=0;
      for(freq_idx=0; freq_idx<=U16_MAX; freq_idx++){
        gic_ranker_idx_delta=freq_list_base[freq_idx];
        freq_list_base[freq_idx]=gic_ranker_idx;
        gic_ranker_idx+=gic_ranker_idx_delta;
      }
/*
Rearrange the (gic_ranker_t)s according the base indexes just computed, moving forwards to maximize cache hits.
*/
      for(gic_ranker_idx=0; gic_ranker_idx<=gic_ranker_idx_max; gic_ranker_idx++){
        pdf_idx=gic_ranker_list_base0[gic_ranker_idx].pdf_idx;
        memcpy(&u64_0, &gic_ranker_list_base0[gic_ranker_idx].ranker, U64_SIZE);
        u16_0=(u16)(u64_0>>bit_idx)^u16_xor_mask;
        gic_ranker_idx_new=freq_list_base[u16_0];
        gic_ranker_list_base1[gic_ranker_idx_new].pdf_idx=pdf_idx;
        memcpy(&gic_ranker_list_base1[gic_ranker_idx_new].ranker, &u64_0, U64_SIZE);
        gic_ranker_idx_new++;
        freq_list_base[u16_0]=gic_ranker_idx_new;
      }
      bit_idx=(u8)(bit_idx+U16_BITS);
      gic_ranker_list_base2=gic_ranker_list_base0;
      gic_ranker_list_base0=gic_ranker_list_base1;
      gic_ranker_list_base1=gic_ranker_list_base2;
    }while(bit_idx!=U64_BITS);
    DEBUG_FREE_PARANOID(freq_list_base);
    status=0;
  }
  return status;
}

void
gic_ranker_list_to_ratio_list(gic_t *gic_base, gic_ranker_t *gic_ranker_list_base){
/*
Convert a list of (gic_ranker_t)s sorted descending by (ranker)s into a list of ratios of successive (ranker)s, such that all such ratios are on the unit interval.

In:

  *gic_base is the return value of gic_malloc().

  *gic_ranker_list_base has passed through gic_ranker_list_fill() and gic_ranker_list_sort() with ascending_status set to zero.

Out:

  *gic_ranker_list_base contains (gic_ranker_t)s, the (ranker)s of which having been converted to the ratios of respective successive input (ranker)s. The last (gic_ranker_t) has its ranker set to 1.0 (because there is no successive ranker from which to compute the ratio. Thus this list is now an unsorted list of GIC ratios.
*/
  double gic_ranker;
  double gic_ranker_old;
  ULONG gic_ranker_idx;
  ULONG gic_ranker_idx_max;
  double gic_ratio;

  gic_ranker_old=gic_ranker_list_base[0].ranker;
  gic_ranker_idx_max=(ULONG)(gic_base->pdf_count)-1;
  for(gic_ranker_idx=0; gic_ranker_idx!=gic_ranker_idx_max; gic_ranker_idx++){
    gic_ranker=gic_ranker_list_base[gic_ranker_idx+1].ranker;
    gic_ratio=gic_ranker/gic_ranker_old;
    gic_ranker_old=gic_ranker;
    gic_ranker_list_base[gic_ranker_idx].ranker=gic_ratio;
  }
  gic_ranker_list_base[gic_ranker_idx_max].ranker=1;
  return;
}
