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
Whole Number Functions
*/
#include "flag.h"
#include "flag_fracterval_u128.h"
#include "flag_fracterval_u64.h"
#include "flag_transform.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "fracterval_u128.h"
#include "fracterval_u128_xtrn.h"
#include "fracterval_u64.h"
#include "fracterval_u64_xtrn.h"
#include "transform.h"
#include "transform_xtrn.h"

u8
transform_delta_delta_list_init(fru128 **entropy_delta_delta_fru128_list_base_base, fru64 **entropy_delta_delta_fru64_list_base_base, u8 precise_status){
/*
Initialize (allocate and populate) a lookup table of (changes in (changes in entropy)).

In:

  *entropy_delta_delta_fru128_list_base_base is undefined.

  *entropy_delta_delta_fru64_list_base_base is undefined.

  precise_status is zero for 32.32 or one for 64.64 fixed-points expressed as 64-bit or 128-bit fractervals, respectively.

Out:

  Returns one on failure, else zero.

  *entropy_delta_delta_fru(64/128)_list_base_base is the base of TRANSFORM_DELTA_DELTA_FREQ_POST (64/128)-bit fractervals as defined in the summary, depending on precise_status. The other is NULL.
*/
  fru128 entropy_delta_delta_delta;
  fru128 entropy_delta_delta_fru128;
  fru128 *entropy_delta_delta_fru128_list_base;
  fru64 entropy_delta_delta_fru64;
  fru64 *entropy_delta_delta_fru64_list_base;
  ULONG freq;
  u8 status;
  u64 term_u64;

  entropy_delta_delta_fru64_list_base=NULL;
  entropy_delta_delta_fru128_list_base=NULL;
  if(!precise_status){
    entropy_delta_delta_fru64_list_base=fracterval_u64_list_malloc(TRANSFORM_DELTA_DELTA_FREQ_MAX);
    status=!entropy_delta_delta_fru64_list_base;
  }else{
    entropy_delta_delta_fru128_list_base=fracterval_u128_list_malloc(TRANSFORM_DELTA_DELTA_FREQ_MAX);
    status=!entropy_delta_delta_fru128_list_base;
  }
  if(!status){
    FRU128_SET_ZERO(entropy_delta_delta_fru128);
    freq=0;
    do{
      if(freq){
        term_u64=freq;
        FRU128_LOG_DELTA_U64(entropy_delta_delta_delta, term_u64, status);
        FRU128_SHIFT_RIGHT_SELF(entropy_delta_delta_delta, U64_BITS-U64_BITS_LOG2);
        FRU128_MULTIPLY_U64(entropy_delta_delta_fru128, entropy_delta_delta_delta, term_u64, status);
        term_u64++;
        FRU128_LOG_U64(entropy_delta_delta_delta, term_u64, status);
        FRU128_SHIFT_RIGHT_SELF(entropy_delta_delta_delta, U64_BITS-U64_BITS_LOG2);
        FRU128_ADD_FRU128_SELF(entropy_delta_delta_fru128, entropy_delta_delta_delta, status);
      }
      if(!precise_status){
        FRU128_SHIFT_RIGHT_SELF(entropy_delta_delta_fru128, U32_BITS);
        U128_TO_U64_LO(entropy_delta_delta_fru64.a, entropy_delta_delta_fru128.a);
        U128_TO_U64_LO(entropy_delta_delta_fru64.b, entropy_delta_delta_fru128.b);
        entropy_delta_delta_fru64_list_base[freq]=entropy_delta_delta_fru64;
      }else{
        entropy_delta_delta_fru128_list_base[freq]=entropy_delta_delta_fru128;
      }
    }while((freq++)!=TRANSFORM_DELTA_DELTA_FREQ_MAX);
  }
  *entropy_delta_delta_fru128_list_base_base=entropy_delta_delta_fru128_list_base;
  *entropy_delta_delta_fru64_list_base_base=entropy_delta_delta_fru64_list_base;
  return status;
}

u8
transform_dispatch(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base){
/*
Excute a 2D channelized entropy transform by scrolling a window across all columns (channels), then across all rows (spectra).

In:

  entropy_base is transform_malloc():Out:*entropy_base_base.

  transform_base is transform_malloc():Out:*transform_base_base.

  whole_u8_list_base is the base of (entropy_base->whole_count) wholes, each of size (entropy_base->granularity+1) and not exceeding (entropy_base->whole_max_max).

Out:

  Returns one if fracterval underflow or overflow occured (which is usually benign) else zero.

  The transform has been done according to transform_malloc():In. If transform_malloc():In:fakefloat_list_base was not NULL, then an array of fakefloats corresponding to window entropy values, and indexed in the same way as *whole_u8_list_base, but having width (entropy_base->fakefloat_x_idx_max+1) and height (entropy_base->fakefloat_y_idx_max+1) has been created at (entropy_base->fakefloat_list_base).
*/
  u64 entropy_max_max_u64;
  u128 entropy_max_max_u128;
  ULONG entropy_max_max_x_idx;
  u64 entropy_min_min_u64;
  ULONG entropy_min_min_x_idx;
  u128 entropy_min_min_u128;
  ULONG entropy_y_idx;
  ULONG entropy_y_idx_max;
  u8 optimize_status;
  u8 precise_status;
  u8 status;
  u8 status_delta;

  optimize_status=transform_base->optimize_status;
  precise_status=entropy_base->precise_status;
  status=0;
  entropy_base->max_max_y_idx=0;
  entropy_base->min_min_y_idx=0;
  U128_SET_ZERO(entropy_max_max_u128);
  entropy_max_max_u64=0;
  entropy_max_max_x_idx=0;
  U128_NOT(entropy_min_min_u128, entropy_max_max_u128);
  entropy_min_min_u64=~entropy_max_max_u64;
  entropy_min_min_x_idx=0;
  entropy_y_idx=0;
  entropy_y_idx_max=entropy_base->fakefloat_y_idx_max;
  do{
    if(!precise_status){
      if(optimize_status&&entropy_y_idx){
        status_delta=transform_whole_list_line_entropy_delta_get_fru64(entropy_base, transform_base, whole_u8_list_base, entropy_y_idx);
      }else{
        status_delta=transform_whole_list_line_entropy_get_fru64(entropy_base, transform_base, whole_u8_list_base, entropy_y_idx);
      }
      if(entropy_max_max_u64<entropy_base->max_max_u64){
        entropy_base->max_max_y_idx=entropy_y_idx;
        entropy_max_max_u64=entropy_base->max_max_u64;
        entropy_max_max_x_idx=entropy_base->max_max_x_idx;
      }
      if(entropy_base->min_min_u64<entropy_min_min_u64){
        entropy_base->min_min_y_idx=entropy_y_idx;
        entropy_min_min_u64=entropy_base->min_min_u64;
        entropy_min_min_x_idx=entropy_base->min_min_x_idx;
      }
    }else{
      if(optimize_status&&entropy_y_idx){
        status_delta=transform_whole_list_line_entropy_delta_get_fru128(entropy_base, transform_base, whole_u8_list_base, entropy_y_idx);
      }else{
        status_delta=transform_whole_list_line_entropy_get_fru128(entropy_base, transform_base, whole_u8_list_base, entropy_y_idx);
      }
      if(U128_IS_LESS(entropy_max_max_u128, entropy_base->max_max_u128)){
        entropy_base->max_max_y_idx=entropy_y_idx;
        entropy_max_max_u128=entropy_base->max_max_u128;
        entropy_max_max_x_idx=entropy_base->max_max_x_idx;
      }
      if(U128_IS_LESS(entropy_base->min_min_u128, entropy_min_min_u128)){
        entropy_base->min_min_y_idx=entropy_y_idx;
        entropy_min_min_u128=entropy_base->min_min_u128;
        entropy_min_min_x_idx=entropy_base->min_min_x_idx;
      }
    }
    status=(u8)(status|status_delta);
  }while((entropy_y_idx++)!=entropy_y_idx_max);
  entropy_base->max_max_u128=entropy_max_max_u128;
  entropy_base->max_max_u64=entropy_max_max_u64;
  entropy_base->max_max_x_idx=entropy_max_max_x_idx;
  entropy_base->min_min_u128=entropy_min_min_u128;
  entropy_base->min_min_u64=entropy_min_min_u64;
  entropy_base->min_min_x_idx=entropy_min_min_x_idx;
  if(entropy_base->discount_status){
    status_delta=transform_entropy_to_discount(entropy_base);
    status=(u8)(status|status_delta);
  }
  return status;
}

u8
transform_entropy_to_discount(entropy_t *entropy_base){
/*
Subtract min and max entropy values from the theoretical maximum window entropy, then exchange the (x, y) coordinates of the corresponding windows (because discounting inverts min and max).

In:

  entropy_base is transform_malloc():Out:*entropy_base_base.

Out:

  Returns one if fracterval underflow or overflow occured else zero. This can only occur if either (1) the upper bound of a computed entropy value was at least the maximum estimated entropy or (2) said maximum is actually less than its true value. Either way, the result is saturation to zero discount nats, which is probably benign if not exactly correct.

  *entropy_base has been adjusted in the following items in order to reflect discount nats: max_max_u128, max_max_u64, max_max_x_idx, max_max_y_idx, min_min_u128, min_min_u64, min_min_x_idx, and min_min_y_idx.
*/
  fru128 entropy_fru128;
  fru64 entropy_fru64;
  ULONG entropy_idx;
  fru128 entropy_max_max_max_fru128;
  fru64 entropy_max_max_max_fru64;
  u8 status;
  u64 term_u64;

  status=0;
  term_u64=(u64)(entropy_base->window_x_idx_post);
  if(!entropy_base->precise_status){
    entropy_fru64.a=entropy_base->min_min_u64;
    entropy_fru64.b=entropy_base->max_max_u64;
    entropy_max_max_max_fru64=entropy_base->window_max_max_fru64;
    FRU64_SUBTRACT_FROM_FRU64_SELF(entropy_fru64, entropy_max_max_max_fru64, status);
    FRU64_DIVIDE_U64_SELF(entropy_fru64, term_u64, status);
    entropy_base->max_max_u64=entropy_fru64.b;
    entropy_base->min_min_u64=entropy_fru64.a;
  }else{
    entropy_fru128.a=entropy_base->min_min_u128;
    entropy_fru128.b=entropy_base->max_max_u128;
    entropy_max_max_max_fru128=entropy_base->window_max_max_fru128;
    FRU128_SUBTRACT_FROM_FRU128_SELF(entropy_fru128, entropy_max_max_max_fru128, status);
    FRU128_DIVIDE_U64_SELF(entropy_fru128, term_u64, status);
    entropy_base->max_max_u128=entropy_fru128.b;
    entropy_base->min_min_u128=entropy_fru128.a;
  }
  entropy_idx=entropy_base->max_max_x_idx;
  entropy_base->max_max_x_idx=entropy_base->min_min_x_idx;
  entropy_base->min_min_x_idx=entropy_idx;
  entropy_idx=entropy_base->max_max_y_idx;
  entropy_base->max_max_y_idx=entropy_base->min_min_y_idx;
  entropy_base->min_min_y_idx=entropy_idx;
  return status;
}

u32 *
transform_fakefloat_list_malloc(ULONG *fakefloat_list_size_base, ULONG whole_x_idx_max, ULONG whole_y_idx_max, ULONG window_x_idx_max, ULONG window_y_idx_max){
/*
Allocate an array of fakefloats ((u32)s which are bitwise equivalent to, but not identified as, floats).

In:

  *fakefloat_list_size_base is undefined.

  whole_x_idx_max is the number of columns (channels) per row (spectrum) in the array of wholes to scan, less one.

  whole_y_idx_max is the number of rows in the array of wholes to scan, less one.

  window_x_idx_max is the number of columns (channels) per row (spectrum) in the window of wholes to scan, less one. On [0, whole_x_idx_max].

  window_y_idx_max is the number of rows in the window of wholes to scan, less one. On [0, whole_y_idx_max].

Out:

  Returns NULL on failure, else the base of (whole_x_idx_max+1)*(whole_y_idx_max+1) fakefloat (u32)s.
*/
  ULONG fakefloat_idx_post;
  u32 *fakefloat_list_base;
  ULONG fakefloat_x_idx_post;
  ULONG fakefloat_y_idx_post;
  ULONG list_size;

  fakefloat_list_base=NULL;
  if((window_x_idx_max<=whole_x_idx_max)&&(window_y_idx_max<=whole_y_idx_max)){
    fakefloat_x_idx_post=whole_x_idx_max-window_x_idx_max+1;
    fakefloat_y_idx_post=whole_y_idx_max-window_y_idx_max+1;
    fakefloat_idx_post=fakefloat_x_idx_post*fakefloat_y_idx_post;
    if(fakefloat_idx_post&&((fakefloat_idx_post/fakefloat_x_idx_post)==fakefloat_y_idx_post)){
      list_size=fakefloat_idx_post<<U32_SIZE_LOG2;
      if((list_size>>U32_SIZE_LOG2)==fakefloat_idx_post){
        *fakefloat_list_size_base=list_size;
        fakefloat_list_base=DEBUG_MALLOC_PARANOID(list_size);
      }
    }
  }
  return fakefloat_list_base;
}

void *
transform_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Transform calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

void
transform_free_all(entropy_t **entropy_base_base, transform_t **transform_base_base){
/*
Free all transform data structures.

In:

  entropy_base is transform_malloc():Out:*entropy_base_base. May be NULL only if transform_base is NULL.

  transform_base is transform_malloc():Out:*transform_base_base. May be NULL.

Out:

  *entropy_base_base is NULL and all its children have been freed except for *fakefloat_list_base (because it was never the property of Transform).

  *transform_base_base is NULL and all its children have been freed.
*/
  entropy_t *entropy_base;
  ULONG *freq_list_base;
  ULONG **freq_list_base_list_base;
  ULONG freq_list_idx;
  ULONG *pop_list_base;
  ULONG **pop_list_base_list_base;
  transform_t *transform_base;

  entropy_base=*entropy_base_base;
  transform_base=*transform_base_base;
  if(transform_base){
    if(!entropy_base->precise_status){
      fracterval_u64_free(transform_base->entropy_delta_delta_fru64_list_base);
      transform_lookup_lists_free(transform_base);
      fracterval_u64_free(transform_base->entropy_delta_fru64_list_base);
    }else{
      fracterval_u128_free(transform_base->entropy_delta_delta_fru128_list_base);
      transform_lookup_lists_free(transform_base);
      fracterval_u128_free(transform_base->entropy_delta_fru128_list_base);
    }
    freq_list_base_list_base=transform_base->freq_list_base_list_base;
    pop_list_base_list_base=transform_base->pop_list_base_list_base;
    if(freq_list_base_list_base){
      freq_list_idx=transform_base->freq_list_idx_max;
      do{
        if(pop_list_base_list_base){
          pop_list_base=pop_list_base_list_base[freq_list_idx];
          DEBUG_FREE_PARANOID(pop_list_base);
        }
        freq_list_base=freq_list_base_list_base[freq_list_idx];
        DEBUG_FREE_PARANOID(freq_list_base);
      }while(freq_list_idx--);
    }
    DEBUG_FREE_PARANOID(transform_base->pop_list_best_best_base);
    DEBUG_FREE_PARANOID(transform_base->pop_list_best_base);
    DEBUG_FREE_PARANOID(pop_list_base_list_base);
    DEBUG_FREE_PARANOID(freq_list_base_list_base);
    DEBUG_FREE_PARANOID(transform_base->pop_list_base);
    DEBUG_FREE_PARANOID(transform_base);
    *transform_base_base=NULL;
  }
  entropy_base=*entropy_base_base;
  DEBUG_FREE_PARANOID(entropy_base);
  *entropy_base_base=NULL;
  return;
}

u8
transform_freq_list_entropy_get_fru128(u8 algo_status, fru128 *entropy_fru128_base, ULONG *freq_list_base, fru128 *lookup_list_base, ULONG *pop_list_base, ULONG whole_idx_max, u32 whole_max_max){
/*
Get the entropy delta implied by a frequency list, which the caller must then subtract from the entropy bias, for example, from (entropy_base->window_bias_fru128), in order to obtain entropy.

In:

  algo_status is one of the TRANSFORM_ALGO_STATUS constants, which implies which definition of entropy to use.

  *entropy_fru128_base is undefined.

  *freq_list_base contains the frequency (occurrences count) each corresponding whole on [0, whole_max_max].

  *lookup_list_base is (transform_base->log_fru128_list_base) for (algo_status==TRANSFORM_ALGO_STATUS_SHANNON) or (transform_base->log_factorial_fru128_list_base) otherwise, as initialized by transform_lookup_lists_init().

  *pop_list_base is writable on [0, whole_idx_max+1].

  whole_idx_max is sum of all of *freq_list_base, less one.

  whole_max_max is the maximum index of *freq_list_base.

Out:

  Returns one if fracterval underflow or overflow occured (which is usually benign) else zero.

  *entropy_fru128_list_base is the entropy delta described in the summary, as determined according to algo_status.
*/
  fru128 entropy;
  fru128 entropy_delta;
  ULONG freq;
  ULONG freq_x_pop;
  ULONG freq_x_pop_sum;
  ULONG pop;
  u8 status;
  u64 term_u64;

  transform_pop_list_fill(freq_list_base, pop_list_base, whole_idx_max, whole_max_max);
  status=0;
  FRU128_SET_ZERO(entropy);
  freq_x_pop_sum=pop_list_base[1];
  if(algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM){
/*
Maintain maximum precision by avoiding unnecessary arithmetic.
*/
    if(freq_x_pop_sum){
      entropy=lookup_list_base[freq_x_pop_sum];
      pop=pop_list_base[0];
      if(pop){
        entropy_delta=lookup_list_base[pop];
        FRU128_ADD_FRU128_SELF(entropy, entropy_delta, status);
      }
    }else{
      pop=pop_list_base[0];
      entropy=lookup_list_base[pop];
    }
  }
  if(freq_x_pop_sum<=whole_idx_max){
    freq=2;
    do{
      pop=pop_list_base[freq];
      if(pop){
        freq_x_pop=freq*pop;
        if(algo_status==TRANSFORM_ALGO_STATUS_AGNENTROPY){
          entropy_delta=lookup_list_base[freq];
          term_u64=pop;
          FRU128_MULTIPLY_U64_SELF(entropy_delta, term_u64, status);
          FRU128_ADD_FRU128_SELF(entropy, entropy_delta, status);
        }else if(algo_status==TRANSFORM_ALGO_STATUS_SHANNON){
          entropy_delta=lookup_list_base[freq];
          term_u64=freq_x_pop;
          FRU128_MULTIPLY_U64_SELF(entropy_delta, term_u64, status);
          FRU128_ADD_FRU128_SELF(entropy, entropy_delta, status);
        }else{
          entropy_delta=lookup_list_base[freq];
          if(pop!=1){
            term_u64=pop;
            FRU128_MULTIPLY_U64_SELF(entropy_delta, term_u64, status);
          }
          FRU128_ADD_FRU128_SELF(entropy, entropy_delta, status);
          if(pop!=1){
            entropy_delta=lookup_list_base[pop];
            FRU128_ADD_FRU128_SELF(entropy, entropy_delta, status);
          }
        }
        freq_x_pop_sum+=freq_x_pop;
      }
      freq++;
    }while(freq_x_pop_sum<=whole_idx_max);
  }
  *entropy_fru128_base=entropy;
  return status;
}

u8
transform_freq_list_entropy_get_fru64(u8 algo_status, fru64 *entropy_fru64_base, ULONG *freq_list_base, fru64 *lookup_list_base, ULONG *pop_list_base, ULONG whole_idx_max, u32 whole_max_max){
/*
This is the 64-bit fracterval equivalent of transform_freq_list_entropy_get_fru128(). Replace all occurrences of "128" with "64" in its definition.
*/
  fru64 entropy;
  fru64 entropy_delta;
  ULONG freq;
  ULONG freq_x_pop;
  ULONG freq_x_pop_sum;
  ULONG pop;
  u8 status;
  u64 term_u64;

  transform_pop_list_fill(freq_list_base, pop_list_base, whole_idx_max, whole_max_max);
  status=0;
  FRU64_SET_ZERO(entropy);
  if(algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM){
/*
Maintain maximum precision by avoiding unnecessary arithmetic.
*/
    pop=pop_list_base[0];
    if(pop){
      entropy=lookup_list_base[pop];
      pop=pop_list_base[1];
      if(pop){
        entropy_delta=lookup_list_base[pop];
        FRU64_ADD_FRU64_SELF(entropy, entropy_delta, status);
      }
    }else{
      pop=pop_list_base[1];
      entropy=lookup_list_base[pop];
    }
  }
  freq_x_pop_sum=pop_list_base[1];
  if(freq_x_pop_sum<=whole_idx_max){
    freq=2;
    do{
      pop=pop_list_base[freq];
      if(pop){
        freq_x_pop=freq*pop;
        if(algo_status==TRANSFORM_ALGO_STATUS_AGNENTROPY){
          entropy_delta=lookup_list_base[freq];
          term_u64=pop;
          FRU64_MULTIPLY_U64_SELF(entropy_delta, term_u64, status);
          FRU64_ADD_FRU64_SELF(entropy, entropy_delta, status);
        }else if(algo_status==TRANSFORM_ALGO_STATUS_SHANNON){
          entropy_delta=lookup_list_base[freq];
          term_u64=freq_x_pop;
          FRU64_MULTIPLY_U64_SELF(entropy_delta, term_u64, status);
          FRU64_ADD_FRU64_SELF(entropy, entropy_delta, status);
        }else{
          entropy_delta=lookup_list_base[freq];
          if(pop!=1){
            term_u64=pop;
            FRU64_MULTIPLY_U64_SELF(entropy_delta, term_u64, status);
          }
          FRU64_ADD_FRU64_SELF(entropy, entropy_delta, status);
          if(pop!=1){
            entropy_delta=lookup_list_base[pop];
            FRU64_ADD_FRU64_SELF(entropy, entropy_delta, status);
          }
        }
        freq_x_pop_sum+=freq_x_pop;
      }
      freq++;
    }while(freq_x_pop_sum<=whole_idx_max);
  }
  *entropy_fru64_base=entropy;
  return status;
}

u8
transform_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of TRANSFORM_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of TRANSFORM_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=TRANSFORM_BUILD_BREAK_COUNT) or (build_feature_count>TRANSFORM_BUILD_FEATURE_COUNT), or if fracterval_u128_init() or fracterval_u64_init() fails. Otherwise, returns zero.
*/
  u8 fracterval_u128_status;
  u8 fracterval_u64_status;
  u8 status;

  fracterval_u128_status=fracterval_u128_init(FRU128_BUILD_BREAK_COUNT_EXPECTED, 4);
  fracterval_u64_status=fracterval_u64_init(FRU64_BUILD_BREAK_COUNT_EXPECTED, 2);
  status=(u8)(fracterval_u128_status|fracterval_u64_status);
  status=(u8)(status|(build_break_count!=TRANSFORM_BUILD_BREAK_COUNT));
  status=(u8)(status|(TRANSFORM_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}

u8
transform_logfreedom_max_approximate(entropy_t *entropy_base, u32 iteration_max, fru128 *logfreedom_max_base, transform_t *transform_base){
/*
From its definition, approximate maximum possible logfreedom for a given whole_idx_max (Q-1) and whole_max_max (Z-1). Do this with a heuristic involving a iterations of gradient descent followed by population perturbation. This was adapted from existing debugged code in Dyspoissometer but has been improved to converge to the global maximum in all tested cases.

In:

  entropy_base is transform_malloc():Out:*entropy_base_base.

  iteration is the number of genetic iterations to perform, less one. The largest iteration_max observed in testing was less than 10, but could be somewhat larger in rare cases involving huge datasets. U64_BITS would seem to be a conservative upper bound over all values of Q and Z supported by Transform.

  *logfreedom_max_base is undefined.

  transform_base is transform_malloc():Out:*transform_base_base.

Out:

  Returns one if fracterval underflow or overflow occured (which should be regarded as a fatal error in this particular case) else zero.

  *logfreedom_max_base is a 64.64 fixed-point, expressed as a 128-bit fracterval, which is virtually certain to contain the actual maximum possible logfreedom. There is a theoretical chance that the latter lies above the upper limit, but no chance that it lies below the lower limit.
*/
  fru128 freq_expression_minus;
  fru128 freq_expression_plus;
  ULONG freq_delta;
  ULONG freq_down_right;
  ULONG freq_max;
  ULONG freq_max_max;
  ULONG freq_min;
  ULONG freq_min_min;
  ULONG freq_span;
  ULONG freq_up_left;
  u32 iteration;
  fru128 *log_factorial_list_base;
  fru128 *log_list_base;
  fru128 logfreedom_best;
  fru128 logfreedom_best_best;
  fru128 logfreedom_delta_delta_minus;
  fru128 logfreedom_delta_delta_plus;
  fru128 logfreedom_delta_minus;
  fru128 logfreedom_delta_plus;
  fru128 logfreedom_delta_sum_minus;
  fru128 logfreedom_delta_sum_plus;
  u32 marsaglia_c;
  u32 marsaglia_x;
  u128 mean_minus;
  u128 mean_plus;
  ULONG pop_delta;
  ULONG pop_delta_max;
  ULONG pop_delta_max_minus_1;
  ULONG pop_delta_min;
  ULONG pop_down_left;
  ULONG pop_down_right;
  ULONG pop_freq_max;
  ULONG pop_freq_min;
  ULONG pop_up_left;
  ULONG pop_up_right;
  ULONG *pop_list_base;
  ULONG *pop_list_best_base;
  ULONG *pop_list_best_best_base;
  ULONG pop_list_size;
  u8 progress_status;
  u64 random;
  u8 status;
  u8 status_delta;
  u64 term_u64;
  fru128 term_fru128;
  ULONG whole_count;
  ULONG whole_idx_max;
  u32 whole_max_max;
  ULONG whole_span_max;

  log_factorial_list_base=transform_base->log_factorial_fru128_list_base;
  log_list_base=transform_base->log_fru128_list_base;
  pop_list_base=transform_base->pop_list_base;
  pop_list_best_base=transform_base->pop_list_best_base;
  pop_list_best_best_base=transform_base->pop_list_best_best_base;
  iteration=0;
  random=0;
  random=~random;
  whole_idx_max=entropy_base->window_y_idx_max;
  whole_count=whole_idx_max+1;
  whole_max_max=entropy_base->whole_max_max;
  whole_span_max=(ULONG)(whole_max_max)+1;
  freq_max=whole_count/whole_span_max;
  freq_min=freq_max;
  pop_freq_max=whole_count%whole_span_max;
  pop_freq_min=whole_span_max;
  if(pop_freq_max){
    freq_max++;
    pop_freq_min-=pop_freq_max;
  }
  freq_max_max=freq_max;
  freq_min_min=freq_min;
  pop_list_size=(whole_count+1)<<ULONG_SIZE_LOG2;
  memset(pop_list_base, 0, (size_t)(pop_list_size));
  memset(pop_list_best_base, 0, (size_t)(pop_list_size));
  memset(pop_list_best_best_base, 0, (size_t)(pop_list_size));
  pop_list_base[freq_max]=pop_freq_max;
  pop_list_base[freq_min]=pop_freq_min;
  pop_list_best_best_base[freq_max]=pop_freq_max;
  pop_list_best_best_base[freq_min]=pop_freq_min;
  status=transform_pop_list_logfreedom_get(log_factorial_list_base, &logfreedom_best_best, pop_list_base, whole_idx_max, whole_max_max);
  do{
    do{
      progress_status=0;
      pop_list_size=(freq_max_max-freq_min_min+1)<<ULONG_SIZE_LOG2;
      memcpy(&pop_list_best_base[freq_min_min], &pop_list_base[freq_min_min], (size_t)(pop_list_size));
      freq_up_left=freq_min;
      do{
        freq_down_right=freq_min+!freq_min;
        do{
          pop_up_left=pop_list_base[freq_up_left];
          if(!pop_up_left){
            break;
          }
          pop_down_right=pop_list_base[freq_down_right];
          pop_down_left=pop_list_base[freq_down_right-1];
          pop_up_right=pop_list_base[freq_up_left+1];
          if(pop_down_right&&(((freq_down_right-1)!=freq_up_left)&&((pop_down_right!=1)||(freq_down_right!=freq_up_left)))){
/*
The following logfreedom gradient descent code was adapted from dyspoissometer_logfreedom_max_get() in Dyspoissometer, which explains all the transformations.
*/
            freq_delta=freq_down_right-freq_up_left;
            FRU128_SET_ZERO(logfreedom_delta_sum_minus);
            FRU128_SET_ZERO(logfreedom_delta_sum_plus);
            freq_expression_minus=log_list_base[freq_up_left+1];
            freq_expression_plus=log_list_base[freq_down_right];
            logfreedom_delta_minus=log_list_base[pop_down_left+1];
            FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, freq_expression_minus, status);
            logfreedom_delta_plus=freq_expression_plus;
            pop_delta_max=MIN(pop_down_right, pop_up_left);
            pop_delta_min=1;
            if(2<freq_delta){
              term_fru128=log_list_base[pop_down_right];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
              term_fru128=log_list_base[pop_up_left];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
              term_fru128=log_list_base[pop_up_right+1];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
            }else if(!freq_delta){
              pop_delta_max>>=1;
              term_fru128=log_list_base[pop_down_right];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
              term_fru128=log_list_base[pop_down_right-1];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
              term_fru128=log_list_base[pop_up_right+1];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
            }else{
              term_fru128=log_list_base[pop_down_left+2];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
              term_fru128=log_list_base[pop_down_right];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
              term_fru128=log_list_base[pop_up_left];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
            }
            pop_delta_max_minus_1=pop_delta_max-pop_delta_min;
            if(pop_delta_max_minus_1){
              logfreedom_delta_delta_minus=freq_expression_minus;
              logfreedom_delta_delta_plus=freq_expression_plus;
              if(2<freq_delta){
                term_fru128=log_list_base[pop_down_left+2];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                term_fru128=log_list_base[pop_down_right-1];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
                term_fru128=log_list_base[pop_up_right+2];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                term_fru128=log_list_base[pop_up_left-1];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
              }else if(!freq_delta){
                term_fru128=log_list_base[pop_down_left+2];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                term_fru128=log_list_base[pop_down_right-3];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
                term_fru128=log_list_base[pop_down_right-2];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
                term_fru128=log_list_base[pop_up_right+2];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
              }else{
                term_fru128=log_list_base[pop_down_left+3];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                term_fru128=log_list_base[pop_down_left+4];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                term_fru128=log_list_base[pop_down_right-1];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
                term_fru128=log_list_base[pop_up_left-1];
                FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
              }
              FRU128_MEAN_TO_FTD128(mean_minus, logfreedom_delta_delta_minus);
              FRU128_MEAN_TO_FTD128(mean_plus, logfreedom_delta_delta_plus);
              if(U128_IS_LESS(mean_minus, mean_plus)){
                while(pop_delta_max_minus_1!=pop_delta_min){
                  pop_delta=pop_delta_max_minus_1-((pop_delta_max_minus_1-pop_delta_min)>>1);
                  logfreedom_delta_delta_minus=freq_expression_minus;
                  logfreedom_delta_delta_plus=freq_expression_plus;
                  if(2<freq_delta){
                    term_fru128=log_list_base[pop_down_left+pop_delta+1];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                    term_fru128=log_list_base[pop_up_right+pop_delta+1];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                    term_fru128=log_list_base[pop_down_right-pop_delta];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
                    term_fru128=log_list_base[pop_up_left-pop_delta];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
                  }else if(!freq_delta){
                    term_fru128=log_list_base[pop_down_left+pop_delta+1];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                    term_fru128=log_list_base[pop_up_right+pop_delta+1];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                    term_fru128=log_list_base[pop_down_right-(pop_delta<<1)-1];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
                    term_fru128=log_list_base[pop_down_right-(pop_delta<<1)];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
                  }else{
                    term_fru128=log_list_base[pop_down_left+(pop_delta<<1)+1];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                    term_fru128=log_list_base[pop_down_left+(pop_delta<<1)+2];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_minus, term_fru128, status);
                    term_fru128=log_list_base[pop_down_right-pop_delta];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
                    term_fru128=log_list_base[pop_up_left-pop_delta];
                    FRU128_ADD_FRU128_SELF(logfreedom_delta_delta_plus, term_fru128, status);
                  }
                  FRU128_MEAN_TO_FTD128(mean_minus, logfreedom_delta_delta_minus);
                  FRU128_MEAN_TO_FTD128(mean_plus, logfreedom_delta_delta_plus);
                  if(U128_IS_LESS(mean_minus, mean_plus)){
                    pop_delta_min=pop_delta;
                  }else{
                    pop_delta_max_minus_1=pop_delta-1;
                  }
                }
                pop_delta_min++;
              }
            }
            pop_delta=pop_delta_min;
            term_u64=pop_delta;
            FRU128_MULTIPLY_U64(logfreedom_delta_minus, freq_expression_minus, term_u64, status);
            FRU128_MULTIPLY_U64(logfreedom_delta_plus, freq_expression_plus, term_u64, status);
            term_fru128=log_factorial_list_base[pop_down_left];
            FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
            term_fru128=log_factorial_list_base[pop_down_right];
            FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
            if(2<freq_delta){
              term_fru128=log_factorial_list_base[pop_up_left];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
              term_fru128=log_factorial_list_base[pop_up_right];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
              pop_down_left+=pop_delta;
              pop_down_right-=pop_delta;
              pop_up_left-=pop_delta;
              pop_up_right+=pop_delta;
              term_fru128=log_factorial_list_base[pop_down_left];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
              term_fru128=log_factorial_list_base[pop_down_right];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
              term_fru128=log_factorial_list_base[pop_up_left];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
              term_fru128=log_factorial_list_base[pop_up_right];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
            }else if(!freq_delta){
              term_fru128=log_factorial_list_base[pop_up_right];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
              pop_down_left+=pop_delta;
              pop_down_right-=pop_delta<<1;
              pop_up_left=pop_down_right;
              pop_up_right+=pop_delta;
              term_fru128=log_factorial_list_base[pop_down_left];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
              term_fru128=log_factorial_list_base[pop_down_right];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
              term_fru128=log_factorial_list_base[pop_up_right];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
            }else{
              term_fru128=log_factorial_list_base[pop_up_left];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_plus, term_fru128, status);
              pop_down_left+=pop_delta<<1;
              pop_down_right-=pop_delta;
              pop_up_left-=pop_delta;
              pop_up_right=pop_down_left;
              term_fru128=log_factorial_list_base[pop_down_left];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
              term_fru128=log_factorial_list_base[pop_down_right];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
              term_fru128=log_factorial_list_base[pop_up_left];
              FRU128_ADD_FRU128_SELF(logfreedom_delta_minus, term_fru128, status);
            }
/*
At this point, Dyspoissometer updates all the affected populations, regardless of whether the logfreedom increased or not. This is done to provide a degree of noise in order to avoid getting stuck in local minima. But here, we won't do so unless the logfreedom increases, simply because we have explicit randomization following each gradient descent.
*/
            FRU128_ADD_FRU128_SELF(logfreedom_delta_sum_minus, logfreedom_delta_minus, status);
            FRU128_ADD_FRU128_SELF(logfreedom_delta_sum_plus, logfreedom_delta_plus, status);
            FRU128_MEAN_TO_FTD128(mean_minus, logfreedom_delta_sum_minus);
            FRU128_MEAN_TO_FTD128(mean_plus, logfreedom_delta_sum_plus);
            if(U128_IS_LESS(mean_minus, mean_plus)){
              pop_list_base[freq_down_right-1]=pop_down_left;
              pop_list_base[freq_down_right]=pop_down_right;
              pop_list_base[freq_up_left]=pop_up_left;
              pop_list_base[freq_up_left+1]=pop_up_right;
/*
Decrement freq_min and/or increment freq_max if necessary, which in general will leave some possible values of (freq_down_right, freq_up_left) unexplored, but that's OK because this is just a heuristic. Then prepare to copy *pop_list_base to *pop_list_best_base as efficiently as possible (because it might be huge and full of zeroes).
*/
              freq_max+=(freq_up_left==freq_max);
              freq_max_max=MAX(freq_max, freq_max_max);
              freq_min-=(freq_down_right==freq_min);
              freq_min_min=MIN(freq_min, freq_min_min);              
              pop_list_size=(freq_max_max-freq_min_min+1)<<ULONG_SIZE_LOG2;
              memcpy(&pop_list_best_base[freq_min_min], &pop_list_base[freq_min_min], (size_t)(pop_list_size));
              FRU128_SET_ZERO(logfreedom_delta_sum_minus);
              FRU128_SET_ZERO(logfreedom_delta_sum_plus);
              progress_status=1;
            }
          }
        }while((freq_down_right++)!=freq_max);
      }while((freq_up_left!=freq_max)&&((++freq_up_left)!=whole_count));
    }while(progress_status);
    status_delta=transform_pop_list_logfreedom_get(log_factorial_list_base, &logfreedom_best, pop_list_best_base, whole_idx_max, whole_max_max);
    status=(u8)(status|status_delta);
    FRU128_MEAN_TO_FTD128(mean_minus, logfreedom_best_best);
    FRU128_MEAN_TO_FTD128(mean_plus, logfreedom_best);
    if(U128_IS_LESS(mean_minus, mean_plus)){
      logfreedom_best_best=logfreedom_best;
      pop_list_size=(freq_max_max-freq_min_min+1)<<ULONG_SIZE_LOG2;
      memcpy(&pop_list_best_best_base[freq_min_min], &pop_list_best_base[freq_min_min], (size_t)(pop_list_size));
    }
/*
Randomly perturb *pop_list_base so that we'll hopefully be able to settle into a deeper maximum on the next iteration. (We don't want to permute *pop_list_best_best_base because that's too narrow.)
*/
    freq_up_left=freq_min;
    do{
      pop_up_left=pop_list_base[freq_up_left];
      if(pop_up_left){
        freq_down_right=freq_min+!freq_min;
        freq_span=freq_max-freq_down_right;
        if(freq_span){
          MARSAGLIA_ITERATE(marsaglia_c, marsaglia_x, random);
          freq_down_right+=(ULONG)(random%freq_span);
        }
        if((freq_down_right-1)!=freq_up_left){
          pop_down_right=pop_list_base[freq_down_right];
          pop_delta_max=MIN(pop_down_right, pop_up_left);
          if(freq_down_right==freq_up_left){
            pop_delta_max>>=1;
          }
          if(pop_delta_max){
            MARSAGLIA_ITERATE(marsaglia_c, marsaglia_x, random);
            pop_delta=(ULONG)(random%pop_delta_max)+1;
/*
Move pop_delta population units from freq_down_right to (freq_down_right-1), and the same number from freq_up_left to (freq_up_left+1). Do the arithmetic one step at a time because it's possible that (freq_down_right==freq_up_left) or (freq_up_left==(freq_down_right-2)), resulting in a collision in the middle.
*/
            pop_list_base[freq_down_right-1]+=pop_delta;
            pop_list_base[freq_down_right]-=pop_delta;
            pop_list_base[freq_up_left]-=pop_delta;
            pop_list_base[freq_up_left+1]+=pop_delta;
            freq_max+=(freq_up_left==freq_max);
            freq_min-=(freq_down_right==freq_min);
          }
        }
      }
    }while((freq_up_left!=freq_max)&&((++freq_up_left)!=whole_count));
/*
While not strictly necessary, we should tighten up [freq_min, freq_max] for better performance.
*/
    while(!pop_list_base[freq_max]){
      freq_max--;
    }
    while(!pop_list_base[freq_min]){
      freq_min++;
    }
    freq_max_max=MAX(freq_max, freq_max_max);
    freq_min_min=MIN(freq_min, freq_min_min);
  }while((iteration++)!=iteration_max);
  *logfreedom_max_base=logfreedom_best_best;
  return status;
}

void
transform_lookup_lists_free(transform_t *transform_base){
/*
Free all 128-bit fracterval lookup lists for log, logdelta, and log-of-factorial.

In:

  transform_base is transform_malloc():Out:*transform_base_base.

Out:

  All lookup lists at transform_base have been freed and their bases in *transform_base set to NULL.
*/
  transform_base->log_fru64_list_base=fracterval_u64_free(transform_base->log_fru64_list_base);
  transform_base->log_factorial_fru64_list_base=fracterval_u64_free(transform_base->log_factorial_fru64_list_base);
  transform_base->log_delta_fru64_list_base=fracterval_u64_free(transform_base->log_delta_fru64_list_base);
  transform_base->log_fru128_list_base=fracterval_u128_free(transform_base->log_fru128_list_base);
  transform_base->log_factorial_fru128_list_base=fracterval_u128_free(transform_base->log_factorial_fru128_list_base);
  transform_base->log_delta_fru128_list_base=fracterval_u128_free(transform_base->log_delta_fru128_list_base);
  return;
}

u8
transform_lookup_lists_init(entropy_t *entropy_base, transform_t *transform_base){
/*
Intialize (64/128)-bit fracterval lists of logs, logdeltas (differences of successive logs), and logs of factorials for fast lookup.

In:

  entropy_base is transform_malloc():Out:*entropy_base_base.

  transform_base is transform_malloc():Out:*transform_base_base.

Out:

  Returns one if allocation failed, in which case nothing has been allocated, else zero. In the latter case, it is still possible that fracterval underflow or overflow occurred, in which case one or both saturated limits have been used to populate the lookup lists where necessary. Unfortunately, it's computationally intractable to discover the precise lookup list index values at which this would occur.

  The lookup lists themselves, the bases of which having been stored to *transform_base, have only been populated to the extent required in light of (entropy_base->algo_status). They must be freed via transform_lookup_lists_free() after all transforms using the same transform_malloc() parameters have completed.
*/
  u8 algo_status;
  ULONG freq_span_max;
  ULONG greater_span_max;
  u8 ignored_status;
  ULONG joint_span_max;
  fru128 log_delta_fru128;
  fru128 *log_delta_fru128_list_base;
  fru64 log_delta_fru64;
  fru64 *log_delta_fru64_list_base;
  ULONG log_delta_idx_max;
  fru128 log_factorial_fru128;
  fru128 *log_factorial_fru128_list_base;
  fru128 log_factorial_fru128_precise;
  fru64 log_factorial_fru64;
  fru64 *log_factorial_fru64_list_base;
  ULONG log_factorial_idx_max;
  fru128 log_fru128;
  fru128 *log_fru128_list_base;
  fru128 log_fru128_old;
  fru64 log_fru64;
  fru64 *log_fru64_list_base;
  ULONG log_idx_max;
  ULONG lookup_idx;
  ULONG lookup_idx_max;
  u8 optimize_status;
  u8 overflow_status;
  u8 precise_status;
  u8 shift;
  u8 shift_max;
  u8 status;
  u64 term_u64;
  u32 whole_max_max;
  ULONG whole_span_max;

  algo_status=transform_base->algo_status;
  freq_span_max=entropy_base->window_y_idx_post+1;
  status=!freq_span_max;
  log_delta_fru128_list_base=NULL;
  log_factorial_fru128_list_base=NULL;
  log_fru128_list_base=NULL;
  log_delta_fru64_list_base=NULL;
  log_factorial_fru64_list_base=NULL;
  log_fru64_list_base=NULL;
  whole_max_max=entropy_base->whole_max_max;
  whole_span_max=(ULONG)(whole_max_max)+1;
  greater_span_max=MAX(freq_span_max, whole_span_max);
  joint_span_max=freq_span_max+whole_span_max;
  status=(u8)(status|(joint_span_max<freq_span_max));
  if(!status){
    optimize_status=transform_base->optimize_status;
    log_delta_idx_max=0;
    log_factorial_idx_max=0;
    log_idx_max=0;
    if(algo_status==TRANSFORM_ALGO_STATUS_AGNENTROPY){
      log_factorial_idx_max=joint_span_max-1;
      if(optimize_status){
        log_idx_max=freq_span_max+1;
        status=(u8)(status|(!log_idx_max));
      }
    }else if(algo_status==TRANSFORM_ALGO_STATUS_SHANNON){
      log_idx_max=freq_span_max;
      if(optimize_status){
        log_delta_idx_max=freq_span_max;
        log_idx_max++;
        status=(u8)(status|!log_idx_max);
      }
    }else{
      log_factorial_idx_max=greater_span_max;
      log_idx_max=log_factorial_idx_max+1;
      status=(u8)(status|!log_idx_max);
    }
    precise_status=entropy_base->precise_status;
    if(!status){
      if(log_delta_idx_max){
        if(precise_status){
          log_delta_fru128_list_base=fracterval_u128_list_malloc(log_delta_idx_max);
          status=(u8)(status|!log_delta_fru128_list_base);
        }else{
          log_delta_fru64_list_base=fracterval_u64_list_malloc(log_delta_idx_max);
          status=(u8)(status|!log_delta_fru64_list_base);
        }
      }
      if(log_factorial_idx_max){
        if((algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM)||precise_status){
          log_factorial_fru128_list_base=fracterval_u128_list_malloc(log_factorial_idx_max);
          status=(u8)(status|!log_factorial_fru128_list_base);
        }
        if(!precise_status){
          log_factorial_fru64_list_base=fracterval_u64_list_malloc(log_factorial_idx_max);
          status=(u8)(status|!log_factorial_fru64_list_base);
        }
      }
      if(log_idx_max){
        if((algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM)||precise_status){
          log_fru128_list_base=fracterval_u128_list_malloc(log_idx_max);
          status=(u8)(status|!log_fru128_list_base);
        }
        if(!precise_status){
          log_fru64_list_base=fracterval_u64_list_malloc(log_idx_max);
          status=(u8)(status|!log_fru64_list_base);
        }
      }
      if(!status){
        lookup_idx_max=MAX(log_delta_idx_max, log_factorial_idx_max);
        lookup_idx_max=MAX(log_idx_max, lookup_idx_max);
        FRU128_SET_ZERO(log_factorial_fru128_precise);
        FRU128_SET_ZERO(log_fru128_old);
        ignored_status=0;
        lookup_idx=1;
        shift=0;
        shift_max=U64_BITS-U64_BITS_LOG2;
        do{
          term_u64=lookup_idx;
          FRU128_LOG_U64(log_fru128, term_u64, status);
          if(lookup_idx<=log_delta_idx_max){
            FRU128_SUBTRACT_FRU128(log_delta_fru128, log_fru128, log_fru128_old, ignored_status);
            FRU128_SHIFT_RIGHT_SELF(log_delta_fru128, shift_max);
            if(log_delta_fru64_list_base){
              FRU128_TO_FRU64_MID(log_delta_fru64, log_delta_fru128);
              log_delta_fru64_list_base[lookup_idx-1]=log_delta_fru64;
            }
            if(log_delta_fru128_list_base){
              log_delta_fru128_list_base[lookup_idx-1]=log_delta_fru128;
            }
          }
          if(lookup_idx<=log_factorial_idx_max){
            do{
              FRU128_SHIFT_RIGHT(log_factorial_fru128, shift, log_fru128);
              overflow_status=0;
              FRU128_ADD_FRU128_SELF(log_factorial_fru128, log_factorial_fru128_precise, overflow_status);
              if(!overflow_status){
                log_factorial_fru128_precise=log_factorial_fru128;
              }else{
                if(shift==shift_max){
  /*
  log_factorial_fru128_precise has exhausted the maximum available precision, but don't bother to report an error. Instead, just set the result to ambiguous, so that if it's ever used (which is unlikely in practice), it will induce a fracterval precision error upon any subsequent computation. (We can't saturate to ones because we don't know if that's actually valid for the lower bound.)
  */
                  break;
                }
                shift++;
                FRU128_SHIFT_RIGHT_SELF(log_factorial_fru128_precise, 1);
              }
            }while(overflow_status);
            if(!overflow_status){
              shift=(u8)(shift_max-shift);
              FRU128_SHIFT_RIGHT_SELF(log_factorial_fru128, shift);
              shift=(u8)(shift_max-shift);
            }else{
              FRU128_SET_AMBIGUOUS(log_factorial_fru128);
            }
            if(log_factorial_fru64_list_base){
              FRU128_TO_FRU64_MID(log_factorial_fru64, log_factorial_fru128);
              log_factorial_fru64_list_base[lookup_idx]=log_factorial_fru64;
            }
            if(log_factorial_fru128_list_base){
              log_factorial_fru128_list_base[lookup_idx]=log_factorial_fru128;
            }
          }
          log_fru128_old=log_fru128;
          if(lookup_idx<=log_idx_max){
            FRU128_SHIFT_RIGHT_SELF(log_fru128, shift_max);
            if(log_fru64_list_base){
              FRU128_TO_FRU64_MID(log_fru64, log_fru128);
              log_fru64_list_base[lookup_idx]=log_fru64;
            }
            if(log_fru128_list_base){
              log_fru128_list_base[lookup_idx]=log_fru128;
            }
          }
          lookup_idx++;
        }while(lookup_idx<=lookup_idx_max);
        if(log_delta_idx_max){
          term_u64=(u64)(lookup_idx_max)+1;
          FRU128_LOG_U64(log_fru128, term_u64, ignored_status);
          FRU128_SUBTRACT_FRU128(log_delta_fru128, log_fru128, log_fru128_old, ignored_status);
  /*
  Prevent the compiler from complaining about ignored_status.
  */
          status=(u8)(status|(ignored_status&0));  
          FRU128_SHIFT_RIGHT_SELF(log_delta_fru128, shift_max);
          if(log_delta_fru64_list_base){
            FRU128_TO_FRU64_MID(log_delta_fru64, log_delta_fru128);
            log_delta_fru64_list_base[log_delta_idx_max]=log_delta_fru64;
            FRU64_SET_ONES(log_delta_fru64);
            log_delta_fru64_list_base[0]=log_delta_fru64;
          }
          if(log_delta_fru128_list_base){
            log_delta_fru128_list_base[log_delta_idx_max]=log_delta_fru128;
            FRU128_SET_ONES(log_delta_fru128);
            log_delta_fru128_list_base[0]=log_delta_fru128;
          }
        }
        if(log_factorial_fru64_list_base){
          FRU64_SET_ZERO(log_factorial_fru64);
          log_factorial_fru64_list_base[0]=log_factorial_fru64;
          log_factorial_fru64_list_base[1]=log_factorial_fru64;
        }
        if(log_factorial_fru128_list_base){
          FRU128_SET_ZERO(log_factorial_fru128);
          log_factorial_fru128_list_base[0]=log_factorial_fru128;
          log_factorial_fru128_list_base[1]=log_factorial_fru128;
        }
        if(log_fru64_list_base){
          FRU64_SET_ZERO(log_fru64);
          log_fru64_list_base[0]=log_fru64;
          log_fru64_list_base[1]=log_fru64;
        }
        if(log_fru128_list_base){
          FRU128_SET_ZERO(log_fru128);
          log_fru128_list_base[0]=log_fru128;
          log_fru128_list_base[1]=log_fru128;
        }
      }
    }
    transform_base->log_delta_fru128_list_base=log_delta_fru128_list_base;
    transform_base->log_factorial_fru128_list_base=log_factorial_fru128_list_base;
    transform_base->log_fru128_list_base=log_fru128_list_base;
    transform_base->log_delta_fru64_list_base=log_delta_fru64_list_base;
    transform_base->log_factorial_fru64_list_base=log_factorial_fru64_list_base;
    transform_base->log_fru64_list_base=log_fru64_list_base;
    if(status){
      transform_lookup_lists_free(transform_base);
    }
  }
  return status;
}

void
transform_malloc(u8 algo_status, u8 discount_status, entropy_t **entropy_base_base, u32 *fakefloat_list_base, u8 granularity, u8 optimize_status, u8 precise_status, u8 rounding_status, transform_t **transform_base_base, u32 whole_max_max, ULONG whole_x_idx_max, ULONG whole_y_idx_max, ULONG window_x_idx_max, ULONG window_y_idx_max){
/*
Initialize data structures in preparation for one or more entropy scans with a given window size.

In:

  algo_status is one of the TRANSFORM_ALGO_STATUS constants, which implies which definition of entropy to use.

  discount_status is one to compute discount nats, else zero for total entropy. Discount nats will be computed on a per-channel basis, whereas total entropy will be per-window.

  *entropy_base_base is undefined.

  fakefloat_list_base is NULL to compute only the minimum and maximum entropy across all windows, and to provide the coordinates of their upper left corners. Else the output of transform_fakefloat_list_malloc() with the same values of whole_x_idx_max, whole_y_idx_max, window_x_idx_max, and window_y_idx_max.

  granularity is the size of a whole number, less one.

  optimize_status is one to optimize performance at the expense of precision. In practice, it should only be set to zero in order to confirm and further resolve previous scan results.

  precise_status is zero for 32.32 or one for 64.64 fixed-points expressed as 64-bit or 128-bit fractervals, respectively.

  rounding_status is one of the TRANSFORM_ROUNDING_STATUS constants, which instructs Transform as to how to round fractervals during conversion to fakefloats. Generally, one should use TRANSFORM_ROUNDING_STATUS_NEAREST_EVEN unless the intention is to place more rigorous bounds on entropy values. This value has no effect currently if fakefloat_list_base is NULL, although it could if Transform eventually supports integrated float-to-whole conversion.

  *transform_base_base is undefined.
  
  whole_max_max is the maximum possible whole number that _could_ ever be encountered during any scan using Out:*entropy_base_base and Out:*transform_base_base. Unless the caller has some prior knowledge of this value, it would typically be discovered via Whole:whole_max_get().

  whole_x_idx_max is the number of columns (channels) per row (spectrum) in the array of wholes to scan, less one.

  whole_y_idx_max is the number of rows in the array of wholes to scan, less one. At least one because the entropy of one whole is not meaningful.

  window_x_idx_max is the number of columns (channels) per row (spectrum) in the window of wholes to scan, less one. On [0, whole_x_idx_max].

  window_y_idx_max is the number of rows in the window of wholes to scan, less one. On [1, whole_y_idx_max].

Out:

  *entropy_base_base has been populated with scan parameters which the caller can safely read but not write. It's a readonly shared data structure in this sense. If it is NULL, then *transform_base_base is also NULL, and allocation has failed due to either memory exhaustion or invalid inputs.

  *transform_base_base is NULL iff *entropy_base_base is null. Otherwise, it's the base of a private data structure to be used for scanning.
*/
  entropy_t *entropy_base;
  fru128 *entropy_delta_fru128_list_base;
  fru64 *entropy_delta_fru64_list_base;
  ULONG *freq_list_base;
  ULONG **freq_list_base_list_base;
  ULONG freq_list_base_list_size;
  ULONG freq_list_idx;
  ULONG freq_list_idx_max;
  ULONG freq_list_idx_post;
  ULONG freq_list_size;
  ULONG pop_idx_post;
  ULONG *pop_list_base;
  ULONG **pop_list_base_list_base;
  ULONG pop_list_size;
  u8 status;
  transform_t *transform_base;
  ULONG whole_count;
  ULONG whole_span_max;
  ULONG whole_x_idx_post;
  ULONG whole_y_idx_post;
  ULONG window_x_idx_post;
  ULONG window_y_idx_post;

  freq_list_idx_max=(optimize_status?whole_x_idx_max:window_x_idx_max);
  freq_list_idx_post=freq_list_idx_max+1;
  status=!freq_list_idx_post;
  freq_list_base_list_size=freq_list_idx_post*(ULONG)(sizeof(u32 *));
  status=(u8)(status|((freq_list_base_list_size/(ULONG)(sizeof(u32 *)))!=freq_list_idx_post));
  status=(u8)(status|(whole_x_idx_max<window_x_idx_max));
  whole_x_idx_post=whole_x_idx_max+1;
  status=(u8)(status|!whole_x_idx_post);
  status=(u8)(status|(whole_y_idx_max<window_y_idx_max));
  whole_y_idx_post=whole_y_idx_max+1;
  status=(u8)(status|!whole_y_idx_post);
  whole_count=whole_x_idx_post*whole_y_idx_post;
  status=(u8)(status|((!whole_count)||((whole_count/whole_x_idx_post)!=whole_y_idx_post)));
  window_x_idx_post=window_x_idx_max+1;
  status=(u8)(status|!window_x_idx_post);
  window_y_idx_post=window_y_idx_max+1;
  status=(u8)(status|!window_y_idx_post);
  whole_span_max=(ULONG)(whole_max_max)+1;
  #ifdef _32_
    status=(u8)(status|!whole_span_max);
  #endif
  freq_list_size=whole_span_max<<ULONG_SIZE_LOG2;
  #ifdef _32_
    status=(u8)(status|((freq_list_size>>ULONG_SIZE_LOG2)!=whole_span_max));
  #endif
  pop_idx_post=window_y_idx_post+1;
  status=(u8)(status|!pop_idx_post);
  pop_list_size=pop_idx_post<<ULONG_SIZE_LOG2;
  status=(u8)(status|((pop_list_size>>ULONG_SIZE_LOG2)!=pop_idx_post));
  entropy_base=NULL;
  transform_base=NULL;
  if(!status){
    entropy_base=DEBUG_CALLOC_PARANOID((ULONG)(sizeof(entropy_t)));
    transform_base=DEBUG_CALLOC_PARANOID((ULONG)(sizeof(transform_t)));
    if(entropy_base&&transform_base){
      entropy_base->fakefloat_list_base=fakefloat_list_base;
      entropy_base->fakefloat_x_idx_max=whole_x_idx_max-window_x_idx_max;
      entropy_base->fakefloat_y_idx_max=whole_y_idx_max-window_y_idx_max;
      entropy_base->granularity=granularity;
      entropy_base->discount_status=discount_status;
      entropy_base->precise_status=precise_status;
      entropy_base->rounding_status=rounding_status;
      entropy_base->whole_count=whole_count;
      entropy_base->whole_max_max=whole_max_max;
      entropy_base->whole_x_idx_max=whole_x_idx_max;
      entropy_base->whole_x_idx_post=whole_x_idx_post;
      entropy_base->whole_y_idx_max=whole_y_idx_max;
      entropy_base->whole_y_idx_post=whole_y_idx_post;
      entropy_base->window_x_idx_max=window_x_idx_max;
      entropy_base->window_x_idx_post=window_x_idx_post;
      entropy_base->window_y_idx_max=window_y_idx_max;
      entropy_base->window_y_idx_post=window_y_idx_post;
      transform_base->algo_status=algo_status;
      transform_base->freq_list_idx_max=freq_list_idx_max;
      pop_list_base=DEBUG_MALLOC_PARANOID(pop_list_size);
      status=(u8)(status|!pop_list_base);
      transform_base->pop_list_base=pop_list_base;
      freq_list_base_list_base=DEBUG_CALLOC_PARANOID(freq_list_base_list_size);
      status=(u8)(status|!freq_list_base_list_base);
      transform_base->freq_list_base_list_base=freq_list_base_list_base;
      if(algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM){
        pop_list_base_list_base=DEBUG_CALLOC_PARANOID(freq_list_base_list_size);
        status=(u8)(status|!pop_list_base_list_base);
        transform_base->pop_list_base_list_base=pop_list_base_list_base;
        pop_list_base=DEBUG_MALLOC_PARANOID(pop_list_size);
        status=(u8)(status|!pop_list_base);
        transform_base->pop_list_best_base=pop_list_base;
        pop_list_base=DEBUG_MALLOC_PARANOID(pop_list_size);
        status=(u8)(status|!pop_list_base);
        transform_base->pop_list_best_best_base=pop_list_base;
      }
      transform_base->optimize_status=optimize_status;
      if(!status){
        freq_list_idx=0;
        do{
          freq_list_base=DEBUG_CALLOC_PARANOID(freq_list_size);
          if(!freq_list_base){
            status=1;
            break;
          }
          freq_list_base_list_base[freq_list_idx]=freq_list_base;
          if(algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM){
            pop_list_base=DEBUG_CALLOC_PARANOID(pop_list_size);
            if(!pop_list_base){
              status=1;
              break;
            }
            pop_list_base_list_base[freq_list_idx]=pop_list_base;
          }
        }while((freq_list_idx++)!=freq_list_idx_max);
        if(!precise_status){
          entropy_delta_fru64_list_base=fracterval_u64_list_malloc(freq_list_idx_max);
          status=!entropy_delta_fru64_list_base;
          transform_base->entropy_delta_fru64_list_base=entropy_delta_fru64_list_base;
        }else{
          entropy_delta_fru128_list_base=fracterval_u128_list_malloc(freq_list_idx_max);
          status=!entropy_delta_fru128_list_base;
          transform_base->entropy_delta_fru128_list_base=entropy_delta_fru128_list_base;
        }
        if(!status){
          status=transform_lookup_lists_init(entropy_base, transform_base);
          if(!status){
            if((algo_status==TRANSFORM_ALGO_STATUS_SHANNON)&&optimize_status){
              status=transform_delta_delta_list_init(&transform_base->entropy_delta_delta_fru128_list_base, &transform_base->entropy_delta_delta_fru64_list_base, precise_status);
            }
            if(!status){
              status=transform_window_entropy_offsets_get(entropy_base, transform_base);
            }
          }
        }
      }
    }
    if(status){
      transform_free_all(&entropy_base, &transform_base);
    }
  }
  *entropy_base_base=entropy_base;
  *transform_base_base=transform_base;
  return;
}

void
transform_pop_list_fill(ULONG *freq_list_base, ULONG *pop_list_base, ULONG whole_idx_max, u32 whole_max_max){
/*
Given a valid frequency list, fill a population (frequency-of-frequency) list.

In:

  *freq_list_base contains the frequency (occurrence count) each corresponding whole on [0, whole_max_max].

  *pop_list_base is writeable on [0, whole_idx_max+1].

  whole_idx_max is transform_pop_list_malloc():In:whole_idx_max.

  whole_max_max can be as low as the greatest whole with nonzero frequency at freq_list_base, but in general, will be the same value which was previously passed to transform_malloc().

Out:

  *pop_list_base contains the populations of all possible frequencies at freq_list_base. Index zero contains the number of zeroes at the latter. The last index, which is (whole_idx_max+1), is zero unless all wholes are identical, in which case it's one. A value of P at index F implies that that frequency F occurs P times at freq_list_base.
*/
  ULONG freq;
  ULONG freq_idx;
  ULONG freq_max_max;
  ULONG freq_sum;

  freq_max_max=whole_idx_max+1;
  transform_ulong_list_zero(freq_max_max, pop_list_base);
  freq_idx=0;
  freq_sum=0;
  do{
    freq=freq_list_base[freq_idx];
    freq_idx++;
    freq_sum+=freq;
    pop_list_base[freq]++;
  }while(freq_sum<=whole_idx_max);
  pop_list_base[0]+=(ULONG)(whole_max_max)+1-freq_idx;
  return;
}

u8
transform_pop_list_logfreedom_get(fru128 *log_factorial_list_base, fru128 *logfreedom_base, ULONG *pop_list_base, ULONG whole_idx_max, u32 whole_max_max){
/*
Calculate the logfreedom of a population list while attempting to maximize accuracy -- actual logfreedom, not a delta relative to a bias.

In:

  *log_factorial_list_base is (transform_base->log_factorial_fru128_list_base).

  *logfreedom_base is undefined.

  *pop_list_base is transform_pop_list_fill():Out:pop_list_base, defined on [0, whole_idx_max+1].

  whole_idx_max is the sum of all items at pop_list_base times their respective indexes, i.e. the sum of population times frequency, less one.

  whole_max_max is the sum of all items at pop_list_base, less one.

Out:

  Returns one if fracterval underflow or overflow occured (which is usually benign) else zero.

  *logfreedom_base is the logfreedom implied by the inputs.
*/
  ULONG freq;
  fru128 logfreedom;
  fru128 logfreedom_delta;
  ULONG pop;
  ULONG pop_sum;
  u8 status;
  u64 term_u64;

  status=0;
  logfreedom=log_factorial_list_base[whole_idx_max+1];
  logfreedom_delta=log_factorial_list_base[whole_max_max+1];
  FRU128_ADD_FRU128_SELF(logfreedom, logfreedom_delta, status);
  freq=0;
  pop_sum=0;
  do{
    pop=pop_list_base[freq];
    if(pop){
      pop_sum+=pop;
      if(pop!=1){
        logfreedom_delta=log_factorial_list_base[pop];
        FRU128_SUBTRACT_FRU128_SELF(logfreedom, logfreedom_delta, status);
      }
      if(1<freq){
        logfreedom_delta=log_factorial_list_base[freq];
        if(pop!=1){
          term_u64=pop;
          FRU128_MULTIPLY_U64_SELF(logfreedom_delta, term_u64, status);
        }
        FRU128_SUBTRACT_FRU128_SELF(logfreedom, logfreedom_delta, status);
      }
    }
    freq++;
  }while(pop_sum<=whole_max_max);
  *logfreedom_base=logfreedom;
  return status;
}

ULONG *
transform_pop_list_malloc(ULONG whole_idx_max){
/*
Allocate a list of populations of frequencies of whole numbers.

In:

  whole_idx_max is one less than the maximum number of wholes for which the population list might need to be evaluated.

Out:

  Returns NULL on failure, else the base of (whole_idx_max+1) undefined (ULONG)s.
*/
  ULONG list_size;
  ULONG *pop_list_base;
  ULONG pop_span;

  pop_list_base=NULL;
  pop_span=whole_idx_max+2;
  if(1U<pop_span){
    list_size=pop_span<<ULONG_SIZE_LOG2;
    if((list_size>>ULONG_SIZE_LOG2)==pop_span){
      pop_list_base=DEBUG_MALLOC_PARANOID(list_size);
    }
  }
  return pop_list_base;
}

void
transform_ulong_list_zero(ULONG ulong_idx_max, ULONG *ulong_list_base){
/*
Zero a list of (ULONG)s.

In:

  ulong_idx_max is the number of (ULONG)s at ulong_list_base, less one.

  *ulong_list_base is defined on [0, ulong_idx_max].

Out:

  *ulong_list_base is zero on [0, ulong_idx_max].
*/
  ULONG list_size;

  list_size=(ulong_idx_max+1)<<ULONG_SIZE_LOG2;
  memset(ulong_list_base, 0, list_size);
  return;
}

u8
transform_whole_list_line_entropy_delta_get_fru128(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_y_idx){
/*
Get the entropy delta (to be subtracted from (entropy_base->window_bias_fru128)) of each window position from left to right in a row of channels after data structures have been initialized by transform_whole_list_line_entropy_get_fru128(). Don't call here directly; use transform_dispatch() instead.

In:

  entropy_base is transform_malloc():Out:*entropy_base_base. (entropy_base->optimize_status) must be one.

  transform_base is transform_malloc():Out:*transform_base_base.

  whole_u8_list_base is the base of (entropy_base->whole_count wholes), each of size (entropy_base->granularity+1) and not exceeding (entropy_base->whole_max_max).

  whole_y_idx is the row index. On [1, (entropy_base->whole_y_idx_max)-(entropy_base->window_y_idx_max)].

Out:

  Returns one if fracterval underflow or overflow occured (which is usually benign) else zero.

  (entropy_base->max_max_u128) is the maximum upper limit of the entropy delta fracterval over all window positions in the row.

  (entropy_base->max_max_x_idx) is the least column index of the window in which said maximum occurred.

  (entropy_base->min_min_u128) is the minimum lower limit of the entropy delta fracterval over all window positions in the row.

  (entropy_base->min_min_x_idx) is the least column index of the window in which said minimum occurred.

  (*(entropy_base->fakefloat_list_base)), if not NULL, has been populated with fakefloat entropy values for the row with index whole_y_idx. (If (entropy_base->discount) status is one, then they have first been subtracted from (entropy_base->window_max_max_fru128)). Note that these are actual entropies -- not entropy deltas. The final result has been rounded according to (entropy_base->rounding_status):

    TRANSFORM_ROUNDING_STATUS_NEAREST_EVEN: Each entropy value is the mean of its lower and upper limits (accounting for the understatement of the latter by one ULP) converted to a fakefloat via nearest-or-even rounding.

    TRANSFORM_ROUNDING_STATUS_NEGATIVE: Each entropy value is its lower limit converted to a fakefloat via rounding toward negative infinity.

    TRANSFORM_ROUNDING_STATUS_POSITIVE: Each entropy value is its upper limit (accounting its understatement by one ULP) converted to a fakefloat via rounding toward positive infinity.
*/
  u8 algo_status;
  u32 digit;
  u8 discount_status;
  fru128 entropy;
  fru128 entropy_copy;
  fru128 entropy_delta;
  fru128 entropy_delta_delta;
  fru128 *entropy_delta_delta_fru128_list_base;
  fru128 *entropy_delta_list_base;
  u128 entropy_max_max;
  fru128 entropy_max_max_max;
  ULONG entropy_max_max_x_idx;
  u128 entropy_min_min;
  ULONG entropy_min_min_x_idx;
  u32 entropy_u32;
  ULONG entropy_u32_idx;
  u32 *entropy_u32_list_base;
  ULONG entropy_x_idx;
  ULONG entropy_x_idx_max;
  ULONG freq;
  ULONG *freq_list_base;
  ULONG **freq_list_base_list_base;
  ULONG freq_old;
  u8 granularity;
  fru128 *log_delta_list_base;
  fru128 *log_list_base;
  ULONG pop;
  ULONG pop_freq_old;
  ULONG *pop_list_base;
  ULONG **pop_list_base_list_base;
  u8 rounding_status;
  u8 status;
  u64 term_u64;
  u32 whole;
  u32 whole_old;
  u8 whole_size;
  ULONG whole_u8_idx;
  ULONG whole_u8_idx_delta;
  ULONG whole_x_idx;
  ULONG whole_x_idx_max;
  ULONG whole_x_idx_post;
  ULONG window_x_idx_max;
  ULONG window_y_idx_max;
  ULONG window_y_idx_post;

  algo_status=transform_base->algo_status;
  discount_status=entropy_base->discount_status;
  rounding_status=entropy_base->rounding_status;
  status=0;
  entropy=entropy_base->window_bias_fru128;
  entropy_delta_delta_fru128_list_base=transform_base->entropy_delta_delta_fru128_list_base;
  entropy_delta_list_base=transform_base->entropy_delta_fru128_list_base;
  U128_SET_ZERO(entropy_max_max);
  entropy_max_max_max=entropy_base->window_max_max_fru128;
  entropy_max_max_x_idx=0;
  U128_NOT(entropy_min_min, entropy_max_max);
  entropy_min_min_x_idx=0;
  entropy_u32_list_base=entropy_base->fakefloat_list_base;
  entropy_x_idx=0;
  entropy_x_idx_max=entropy_base->fakefloat_x_idx_max;
  entropy_u32_idx=(entropy_x_idx_max+1)*whole_y_idx;
  freq_list_base_list_base=transform_base->freq_list_base_list_base;
  granularity=entropy_base->granularity;
  log_delta_list_base=transform_base->log_delta_fru128_list_base;
  log_list_base=transform_base->log_fru128_list_base;
  pop_list_base=NULL;
  pop_list_base_list_base=transform_base->pop_list_base_list_base;
  whole_size=(u8)(granularity+1);
  whole_x_idx_max=entropy_base->whole_x_idx_max;
  whole_x_idx_post=whole_x_idx_max+1;
  window_x_idx_max=entropy_base->window_x_idx_max;
  window_y_idx_max=entropy_base->window_y_idx_max;
  window_y_idx_post=window_y_idx_max+1;
  whole_u8_idx_delta=whole_size*whole_x_idx_post;
  whole_u8_idx=whole_u8_idx_delta*(whole_y_idx-1);
  whole_u8_idx_delta*=window_y_idx_post;
  whole_x_idx=0;
  do{
    if(entropy_x_idx){
      entropy_delta=entropy_delta_list_base[entropy_x_idx-1];
      FRU128_ADD_FRU128_SELF(entropy, entropy_delta, status);
    }
    entropy_delta=entropy_delta_list_base[whole_x_idx];
    freq_list_base=freq_list_base_list_base[whole_x_idx];
    if(algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM){
      pop_list_base=pop_list_base_list_base[whole_x_idx];
    }
    whole_old=whole_u8_list_base[whole_u8_idx];
    whole=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta];
    whole_u8_idx++;
    if(granularity){
      digit=whole_u8_list_base[whole_u8_idx];
      whole_old|=digit<<U8_BITS;
      digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta];
      whole|=digit<<U8_BITS;
      whole_u8_idx++;
      if(U16_BYTE_MAX<granularity){
        digit=whole_u8_list_base[whole_u8_idx];
        whole_old|=digit<<U8_BITS;
        digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta];
        whole|=digit<<U8_BITS;
        whole_u8_idx++;
        if(U24_BYTE_MAX<granularity){
          digit=whole_u8_list_base[whole_u8_idx];
          whole_old|=digit<<U8_BITS;
          digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta];
          whole|=digit<<U8_BITS;
          whole_u8_idx++;
        }
      }
    }
    if(whole!=whole_old){
      freq=freq_list_base[whole];
      freq_old=freq_list_base[whole_old];
      if(algo_status==TRANSFORM_ALGO_STATUS_AGNENTROPY){
/*
The negative of the agnentropy difference, dA, is given by:

  -dA=log(freq+1)-log(freq_old)
*/
        freq_list_base[whole]=freq+1;
        if(freq){
          entropy_delta_delta=log_list_base[freq+1];
          FRU128_ADD_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
        }
        freq_list_base[whole_old]=freq_old-1;
        if(freq_old!=1){
          entropy_delta_delta=log_list_base[freq_old];
          FRU128_SUBTRACT_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
        }
      }else if(algo_status==TRANSFORM_ALGO_STATUS_SHANNON){
/*
The negative of the Shannon entropy difference, dS, is given by:

  -dS=freq*log_delta(freq)+log(freq+1)-(freq_old-1)*log_delta(freq_old-1)-log(freq_old)
*/
        freq_list_base[whole]=freq+1;
        if(freq){
          if(freq<=TRANSFORM_DELTA_DELTA_FREQ_MAX){
            entropy_delta_delta=entropy_delta_delta_fru128_list_base[freq];
          }else{
            entropy_delta_delta=log_delta_list_base[freq];
            term_u64=freq;
            FRU128_MULTIPLY_U64_SELF(entropy_delta_delta, term_u64, status);
            FRU128_ADD_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
            entropy_delta_delta=log_list_base[freq+1];
          }
          FRU128_ADD_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
        }
        freq_list_base[whole_old]=freq_old-1;
        if(freq_old!=1){
          if(freq_old<=(TRANSFORM_DELTA_DELTA_FREQ_MAX+1)){
            entropy_delta_delta=entropy_delta_delta_fru128_list_base[freq_old-1];
          }else{
            entropy_delta_delta=log_delta_list_base[freq_old-1];
            term_u64=freq_old-1;
            FRU128_MULTIPLY_U64_SELF(entropy_delta_delta, term_u64, status);
            FRU128_SUBTRACT_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
            entropy_delta_delta=log_list_base[freq_old];
          }
          FRU128_SUBTRACT_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
        }
      }else{
/*
The negative of the logfreedom difference in terms the frequencies in question and their respective populations, dL, is:

  -dL=+log(freq+1)-log(freq_old)+log(pop(freq_old-1)+1)-log(pop(freq_old))+log(pop(freq+1)+1)-log(pop(freq))

where we need to serialize updates to the populations of the old and new masks, hence the processing all freq_old terms before all freq terms.
*/
        freq_list_base[whole]=freq+1;
        freq_list_base[whole_old]=freq_old-1;
        if(freq_old!=(freq+1)){
          entropy_delta_delta=log_list_base[freq+1];
          FRU128_ADD_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
          pop=pop_list_base[freq_old-1];
          pop++;
          pop_list_base[freq_old-1]=pop;
          if(pop!=1){
            entropy_delta_delta=log_list_base[pop];
            FRU128_ADD_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
          }
          pop_freq_old=pop_list_base[freq_old];
          pop_list_base[freq_old]=pop_freq_old-1;
          pop=pop_list_base[freq+1];
          pop++;
          pop_list_base[freq+1]=pop;
          if(pop!=1){
            entropy_delta_delta=log_list_base[pop];
            FRU128_ADD_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
          }
          pop=pop_list_base[freq];
          pop_list_base[freq]=pop-1;
          if(pop!=1){
            entropy_delta_delta=log_list_base[pop];
            FRU128_SUBTRACT_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
          }
          if(pop_freq_old!=1){
            entropy_delta_delta=log_list_base[pop_freq_old];
            FRU128_SUBTRACT_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
          }
          if(freq_old){
            entropy_delta_delta=log_list_base[freq_old];
            FRU128_SUBTRACT_FRU128_SELF(entropy_delta, entropy_delta_delta, status);
          }
        }
      }
    }
    entropy_delta_list_base[whole_x_idx]=entropy_delta;
    FRU128_SUBTRACT_FRU128_SELF(entropy, entropy_delta, status);
    if(window_x_idx_max<=whole_x_idx){
      if(U128_IS_LESS(entropy_max_max, entropy.b)){
        entropy_max_max=entropy.b;
        entropy_max_max_x_idx=entropy_x_idx;
      }
      if(U128_IS_LESS(entropy.a, entropy_min_min)){
        entropy_min_min=entropy.a;
        entropy_min_min_x_idx=entropy_x_idx;
      }
      if(entropy_u32_list_base){
        entropy_copy=entropy;
        if(discount_status){
          FRU128_SUBTRACT_FRU128(entropy_copy, entropy_max_max_max, entropy, status);
        }
        if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEAREST_EVEN){
          entropy_u32=fracterval_u128_to_fakefloat(entropy_copy, U64_BITS);
        }else if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEGATIVE){
          entropy_u32=fractoid_u128_to_fakefloat_floor(entropy_copy.a, U64_BITS);
        }else{
          entropy_u32=fractoid_u128_to_fakefloat_ceil(entropy_copy.b, U64_BITS);
        }
        entropy_u32_list_base[entropy_u32_idx]=entropy_u32;
        entropy_u32_idx++;
      }
      entropy_x_idx++;
    }
  }while((whole_x_idx++)!=whole_x_idx_max);
  entropy_base->max_max_u128=entropy_max_max;
  entropy_base->max_max_x_idx=entropy_max_max_x_idx;
  entropy_base->min_min_u128=entropy_min_min;
  entropy_base->min_min_x_idx=entropy_min_min_x_idx;
  return status;
}

u8
transform_whole_list_line_entropy_delta_get_fru64(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_y_idx){
/*
This is the 64-bit fracterval equivalent of transform_whole_list_line_entropy_delta_get_fru128(). Replace all occurrences of "128" with "64" in its definition.
*/
  u8 algo_status;
  u32 digit;
  u8 discount_status;
  fru64 entropy;
  fru64 entropy_copy;
  fru64 entropy_delta;
  fru64 entropy_delta_delta;
  fru64 *entropy_delta_delta_fru64_list_base;
  fru64 *entropy_delta_list_base;
  u64 entropy_max_max;
  fru64 entropy_max_max_max;
  ULONG entropy_max_max_x_idx;
  u64 entropy_min_min;
  ULONG entropy_min_min_x_idx;
  u32 entropy_u32;
  ULONG entropy_u32_idx;
  u32 *entropy_u32_list_base;
  ULONG entropy_x_idx;
  ULONG entropy_x_idx_max;
  ULONG freq;
  ULONG *freq_list_base;
  ULONG **freq_list_base_list_base;
  ULONG freq_old;
  u8 granularity;
  fru64 *log_delta_list_base;
  fru64 *log_list_base;
  ULONG pop;
  ULONG pop_freq_old;
  ULONG *pop_list_base;
  ULONG **pop_list_base_list_base;
  u8 rounding_status;
  u8 status;
  u64 term_u64;
  u32 whole;
  u32 whole_old;
  u8 whole_size;
  ULONG whole_u8_idx;
  ULONG whole_u8_idx_delta;
  ULONG whole_x_idx;
  ULONG whole_x_idx_max;
  ULONG whole_x_idx_post;
  ULONG window_x_idx_max;
  ULONG window_y_idx_max;
  ULONG window_y_idx_post;

  algo_status=transform_base->algo_status;
  discount_status=entropy_base->discount_status;
  rounding_status=entropy_base->rounding_status;
  status=0;
  entropy=entropy_base->window_bias_fru64;
  entropy_delta_delta_fru64_list_base=transform_base->entropy_delta_delta_fru64_list_base;
  entropy_delta_list_base=transform_base->entropy_delta_fru64_list_base;
  entropy_max_max=0;
  entropy_max_max_max=entropy_base->window_max_max_fru64;
  entropy_max_max_x_idx=0;
  entropy_min_min=~entropy_max_max;
  entropy_min_min_x_idx=0;
  entropy_u32_list_base=entropy_base->fakefloat_list_base;
  entropy_x_idx=0;
  entropy_x_idx_max=entropy_base->fakefloat_x_idx_max;
  entropy_u32_idx=(entropy_x_idx_max+1)*whole_y_idx;
  freq_list_base_list_base=transform_base->freq_list_base_list_base;
  granularity=entropy_base->granularity;
  log_delta_list_base=transform_base->log_delta_fru64_list_base;
  log_list_base=transform_base->log_fru64_list_base;
  pop_list_base=NULL;
  pop_list_base_list_base=transform_base->pop_list_base_list_base;
  whole_size=(u8)(granularity+1);
  whole_x_idx_max=entropy_base->whole_x_idx_max;
  whole_x_idx_post=whole_x_idx_max+1;
  window_x_idx_max=entropy_base->window_x_idx_max;
  window_y_idx_max=entropy_base->window_y_idx_max;
  window_y_idx_post=window_y_idx_max+1;
  whole_u8_idx_delta=whole_size*whole_x_idx_post;
  whole_u8_idx=whole_u8_idx_delta*(whole_y_idx-1);
  whole_u8_idx_delta*=window_y_idx_post;
  whole_x_idx=0;
  do{
    if(entropy_x_idx){
      entropy_delta=entropy_delta_list_base[entropy_x_idx-1];
      FRU64_ADD_FRU64_SELF(entropy, entropy_delta, status);
    }
    entropy_delta=entropy_delta_list_base[whole_x_idx];
    freq_list_base=freq_list_base_list_base[whole_x_idx];
    if(algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM){
      pop_list_base=pop_list_base_list_base[whole_x_idx];
    }
    whole_old=whole_u8_list_base[whole_u8_idx];
    whole=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta];
    whole_u8_idx++;
    if(granularity){
      digit=whole_u8_list_base[whole_u8_idx];
      whole_old|=digit<<U8_BITS;
      digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta];
      whole|=digit<<U8_BITS;
      whole_u8_idx++;
      if(U16_BYTE_MAX<granularity){
        digit=whole_u8_list_base[whole_u8_idx];
        whole_old|=digit<<U8_BITS;
        digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta];
        whole|=digit<<U8_BITS;
        whole_u8_idx++;
        if(U24_BYTE_MAX<granularity){
          digit=whole_u8_list_base[whole_u8_idx];
          whole_old|=digit<<U8_BITS;
          digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta];
          whole|=digit<<U8_BITS;
          whole_u8_idx++;
        }
      }
    }
    if(whole!=whole_old){
      freq=freq_list_base[whole];
      freq_old=freq_list_base[whole_old];
      if(algo_status==TRANSFORM_ALGO_STATUS_AGNENTROPY){
/*
The negative of the agnentropy difference, dA, is given by:

  -dA=log(freq+1)-log(freq_old)
*/
        freq_list_base[whole]=freq+1;
        if(freq){
          entropy_delta_delta=log_list_base[freq+1];
          FRU64_ADD_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
        }
        freq_list_base[whole_old]=freq_old-1;
        if(freq_old!=1){
          entropy_delta_delta=log_list_base[freq_old];
          FRU64_SUBTRACT_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
        }
      }else if(algo_status==TRANSFORM_ALGO_STATUS_SHANNON){
/*
The negative of the Shannon entropy difference, dS, is given by:

  -dS=freq*log_delta(freq)+log(freq+1)-(freq_old-1)*log_delta(freq_old-1)-log(freq_old)
*/
        freq_list_base[whole]=freq+1;
        if(freq){
          if(freq<=TRANSFORM_DELTA_DELTA_FREQ_MAX){
            entropy_delta_delta=entropy_delta_delta_fru64_list_base[freq];
          }else{
            entropy_delta_delta=log_delta_list_base[freq];
            term_u64=freq;
            FRU64_MULTIPLY_U64_SELF(entropy_delta_delta, term_u64, status);
            FRU64_ADD_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
            entropy_delta_delta=log_list_base[freq+1];
          }
          FRU64_ADD_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
        }
        freq_list_base[whole_old]=freq_old-1;
        if(freq_old!=1){
          if(freq_old<=(TRANSFORM_DELTA_DELTA_FREQ_MAX+1)){
            entropy_delta_delta=entropy_delta_delta_fru64_list_base[freq_old-1];
          }else{
            entropy_delta_delta=log_delta_list_base[freq_old-1];
            term_u64=freq_old-1;
            FRU64_MULTIPLY_U64_SELF(entropy_delta_delta, term_u64, status);
            FRU64_SUBTRACT_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
            entropy_delta_delta=log_list_base[freq_old];
          }
          FRU64_SUBTRACT_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
        }
      }else{
/*
The negative of the logfreedom difference in terms the frequencies in question and their respective populations, dL, is:

  -dL=+log(freq+1)-log(freq_old)+log(pop(freq_old-1)+1)-log(pop(freq_old))+log(pop(freq+1)+1)-log(pop(freq))

where we need to serialize updates to the populations of the old and new masks, hence the processing all freq_old terms before all freq terms.
*/
        freq_list_base[whole]=freq+1;
        freq_list_base[whole_old]=freq_old-1;
        if(freq_old!=(freq+1)){
          entropy_delta_delta=log_list_base[freq+1];
          FRU64_ADD_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
          pop=pop_list_base[freq_old-1];
          pop++;
          pop_list_base[freq_old-1]=pop;
          if(pop!=1){
            entropy_delta_delta=log_list_base[pop];
            FRU64_ADD_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
          }
          pop_freq_old=pop_list_base[freq_old];
          pop_list_base[freq_old]=pop_freq_old-1;
          pop=pop_list_base[freq+1];
          pop++;
          pop_list_base[freq+1]=pop;
          if(pop!=1){
            entropy_delta_delta=log_list_base[pop];
            FRU64_ADD_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
          }
          pop=pop_list_base[freq];
          pop_list_base[freq]=pop-1;
          if(pop!=1){
            entropy_delta_delta=log_list_base[pop];
            FRU64_SUBTRACT_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
          }
          if(pop_freq_old!=1){
            entropy_delta_delta=log_list_base[pop_freq_old];
            FRU64_SUBTRACT_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
          }
          if(freq_old){
            entropy_delta_delta=log_list_base[freq_old];
            FRU64_SUBTRACT_FRU64_SELF(entropy_delta, entropy_delta_delta, status);
          }
        }
      }
    }
    entropy_delta_list_base[whole_x_idx]=entropy_delta;
    FRU64_SUBTRACT_FRU64_SELF(entropy, entropy_delta, status);
    if(window_x_idx_max<=whole_x_idx){
      if(entropy_max_max<entropy.b){
        entropy_max_max=entropy.b;
        entropy_max_max_x_idx=entropy_x_idx;
      }
      if(entropy.a<entropy_min_min){
        entropy_min_min=entropy.a;
        entropy_min_min_x_idx=entropy_x_idx;
      }
      if(entropy_u32_list_base){
        entropy_copy=entropy;
        if(discount_status){
          FRU64_SUBTRACT_FRU64(entropy_copy, entropy_max_max_max, entropy, status);
        }
        if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEAREST_EVEN){
          entropy_u32=fracterval_u64_to_fakefloat(entropy_copy, U32_BITS);
        }else if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEGATIVE){
          entropy_u32=fractoid_u64_to_fakefloat_floor(entropy_copy.a, U32_BITS);
        }else{
          entropy_u32=fractoid_u64_to_fakefloat_ceil(entropy_copy.b, U32_BITS);
        }
        entropy_u32_list_base[entropy_u32_idx]=entropy_u32;
        entropy_u32_idx++;
      }
      entropy_x_idx++;
    }
  }while((whole_x_idx++)!=whole_x_idx_max);
  entropy_base->max_max_u64=entropy_max_max;
  entropy_base->max_max_x_idx=entropy_max_max_x_idx;
  entropy_base->min_min_u64=entropy_min_min;
  entropy_base->min_min_x_idx=entropy_min_min_x_idx;
  return status;
}

u8
transform_whole_list_line_entropy_get_fru128(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_y_idx){
/*
This is the functional equivalent of transform_whole_list_line_entropy_delta_get_fru128(), but for the fact that whole_y_idx can be zero because it computes the entropies in each row from scratch.
*/
  u8 algo_status;
  u32 digit;
  u8 discount_status;
  fru128 entropy;
  fru128 entropy_copy;
  fru128 entropy_delta;
  fru128 *entropy_delta_list_base;
  u128 entropy_max_max;
  fru128 entropy_max_max_max;
  ULONG entropy_max_max_x_idx;
  u128 entropy_min_min;
  ULONG entropy_min_min_x_idx;
  u32 entropy_u32;
  ULONG entropy_u32_idx;
  u32 *entropy_u32_list_base;
  ULONG entropy_x_idx;
  ULONG entropy_x_idx_max;
  ULONG *freq_list_base;
  ULONG freq_list_idx;
  ULONG freq_list_idx_delta;
  ULONG **freq_list_base_list_base;
  u8 granularity;
  fru128 *lookup_list_base;
  u8 optimize_status;
  ULONG *pop_list_base;
  ULONG **pop_list_base_list_base;
  u8 rounding_status;
  u8 status;
  u8 status_delta;
  u32 whole;
  u32 whole_max_max;
  u32 whole_old;
  u8 whole_size;
  ULONG whole_u8_idx;
  ULONG whole_u8_idx_delta0;
  ULONG whole_u8_idx_delta1;
  ULONG whole_x_idx_max;
  ULONG whole_x_idx_post;
  ULONG window_x_idx_max;
  ULONG window_x_idx_post;
  ULONG window_y_idx;
  ULONG window_y_idx_max;

  discount_status=entropy_base->discount_status;
  entropy_base->discount_status=0;
  status=transform_whole_list_window_entropy_get_fru128(entropy_base, transform_base, whole_u8_list_base, 0, whole_y_idx);
  entropy_base->discount_status=discount_status;
  rounding_status=entropy_base->rounding_status;
  entropy_max_max=entropy_base->max_max_u128;
  entropy.b=entropy_max_max;
  entropy_max_max_max=entropy_base->window_max_max_fru128;
  entropy_max_max_x_idx=0;
  entropy_min_min=entropy_base->min_min_u128;
  entropy.a=entropy_min_min;
  entropy_min_min_x_idx=0;
  entropy_u32_idx=0;
  entropy_u32_list_base=entropy_base->fakefloat_list_base;
  entropy_x_idx_max=entropy_base->fakefloat_x_idx_max;
  if(entropy_u32_list_base){
    entropy_copy=entropy;
    if(discount_status){
      FRU128_SUBTRACT_FRU128(entropy_copy, entropy_max_max_max, entropy, status);
    }
    entropy_u32_idx=(entropy_x_idx_max+1)*whole_y_idx;
     if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEAREST_EVEN){
      entropy_u32=fracterval_u128_to_fakefloat(entropy_copy, U64_BITS);
    }else if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEGATIVE){
      entropy_u32=fractoid_u128_to_fakefloat_floor(entropy_copy.a, U64_BITS);
    }else{
      entropy_u32=fractoid_u128_to_fakefloat_ceil(entropy_copy.b, U64_BITS);
    }
    entropy_u32_list_base[entropy_u32_idx]=entropy_u32;
    entropy_u32_idx++;
  }
  algo_status=transform_base->algo_status;
  entropy_delta_list_base=transform_base->entropy_delta_fru128_list_base;
  entropy_x_idx=1;
  freq_list_idx=0;
  freq_list_base_list_base=transform_base->freq_list_base_list_base;
  granularity=entropy_base->granularity;
  lookup_list_base=transform_base->log_factorial_fru128_list_base;
  if(algo_status==TRANSFORM_ALGO_STATUS_SHANNON){
    lookup_list_base=transform_base->log_fru128_list_base;
  }
  optimize_status=transform_base->optimize_status;
  pop_list_base=transform_base->pop_list_base;
  pop_list_base_list_base=transform_base->pop_list_base_list_base;
  whole_size=(u8)(granularity+1);
  window_x_idx_max=entropy_base->window_x_idx_max;
  window_x_idx_post=window_x_idx_max+1;
  freq_list_idx_delta=(optimize_status?window_x_idx_post:freq_list_idx);
  whole_max_max=entropy_base->whole_max_max;
  whole_u8_idx_delta0=whole_size*window_x_idx_post;
  whole_x_idx_max=entropy_base->whole_x_idx_max;
  whole_u8_idx_delta1=whole_size*whole_x_idx_max;
  whole_x_idx_post=whole_x_idx_max+1;
  window_y_idx_max=entropy_base->window_y_idx_max;
  while(entropy_x_idx<=entropy_x_idx_max){
    entropy_delta=entropy_delta_list_base[freq_list_idx];
    FRU128_ADD_FRU128_SELF(entropy, entropy_delta, status);
    freq_list_base=freq_list_base_list_base[freq_list_idx+freq_list_idx_delta];
    if(algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM){
      pop_list_base=pop_list_base_list_base[freq_list_idx+freq_list_idx_delta];
    }
    whole_u8_idx=whole_size*(entropy_x_idx-1+whole_x_idx_post*whole_y_idx);
    window_y_idx=0;
    do{
      whole_old=whole_u8_list_base[whole_u8_idx];
      whole=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta0];
      whole_u8_idx++;
      if(granularity){
        digit=whole_u8_list_base[whole_u8_idx];
        whole_old|=digit<<U8_BITS;
        digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta0];
        whole|=digit<<U8_BITS;
        whole_u8_idx++;
        if(U16_BYTE_MAX<granularity){
          digit=whole_u8_list_base[whole_u8_idx];
          whole_old|=digit<<U8_BITS;
          digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta0];
          whole|=digit<<U8_BITS;
          whole_u8_idx++;
          if(U24_BYTE_MAX<granularity){
            digit=whole_u8_list_base[whole_u8_idx];
            whole_old|=digit<<U8_BITS;
            digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta0];
            whole|=digit<<U8_BITS;
            whole_u8_idx++;
          }
        }
      }
      whole_u8_idx+=whole_u8_idx_delta1;
      freq_list_base[whole]++;
      if(!optimize_status){
        freq_list_base[whole_old]--;
      }
    }while((window_y_idx++)!=window_y_idx_max);
    status_delta=transform_freq_list_entropy_get_fru128(algo_status, &entropy_delta, freq_list_base, lookup_list_base, pop_list_base, window_y_idx_max, whole_max_max);
    status=(u8)(status|status_delta);
    entropy_delta_list_base[freq_list_idx+freq_list_idx_delta]=entropy_delta;
    FRU128_SUBTRACT_FRU128_SELF(entropy, entropy_delta, status);
    if(U128_IS_LESS(entropy_max_max, entropy.b)){
      entropy_max_max=entropy.b;
      entropy_max_max_x_idx=entropy_x_idx;
    }
    if(U128_IS_LESS(entropy.a, entropy_min_min)){
      entropy_min_min=entropy.a;
      entropy_min_min_x_idx=entropy_x_idx;
    }
    if(entropy_u32_list_base){
      entropy_copy=entropy;
      if(discount_status){
        FRU128_SUBTRACT_FRU128(entropy_copy, entropy_max_max_max, entropy, status);
      }
      if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEAREST_EVEN){
        entropy_u32=fracterval_u128_to_fakefloat(entropy_copy, U64_BITS);
      }else if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEGATIVE){
        entropy_u32=fractoid_u128_to_fakefloat_floor(entropy_copy.a, U64_BITS);
      }else{
        entropy_u32=fractoid_u128_to_fakefloat_ceil(entropy_copy.b, U64_BITS);
      }
      entropy_u32_list_base[entropy_u32_idx]=entropy_u32;
      entropy_u32_idx++;
    }
    freq_list_idx++;
    if((freq_list_idx==window_x_idx_post)&&!optimize_status){
      freq_list_idx=0;
    }
    entropy_x_idx++;
  }
  entropy_base->max_max_u128=entropy_max_max;
  entropy_base->max_max_x_idx=entropy_max_max_x_idx;
  entropy_base->min_min_u128=entropy_min_min;
  entropy_base->min_min_x_idx=entropy_min_min_x_idx;
  return status;
}

u8
transform_whole_list_line_entropy_get_fru64(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_y_idx){
/*
This is the functional equivalent of transform_whole_list_line_entropy_delta_get_fru64(), but for the fact that whole_y_idx can be zero because it computes the entropies in each row from scratch.
*/
  u8 algo_status;
  u32 digit;
  u8 discount_status;
  fru64 entropy;
  fru64 entropy_copy;
  fru64 entropy_delta;
  fru64 *entropy_delta_list_base;
  u64 entropy_max_max;
  fru64 entropy_max_max_max;
  ULONG entropy_max_max_x_idx;
  u64 entropy_min_min;
  ULONG entropy_min_min_x_idx;
  u32 entropy_u32;
  ULONG entropy_u32_idx;
  u32 *entropy_u32_list_base;
  ULONG entropy_x_idx;
  ULONG entropy_x_idx_max;
  ULONG *freq_list_base;
  ULONG **freq_list_base_list_base;
  ULONG freq_list_idx;
  ULONG freq_list_idx_delta;
  u8 granularity;
  fru64 *lookup_list_base;
  u8 optimize_status;
  ULONG *pop_list_base;
  ULONG **pop_list_base_list_base;
  u8 rounding_status;
  u8 status;
  u8 status_delta;
  u32 whole;
  u32 whole_max_max;
  u32 whole_old;
  u8 whole_size;
  ULONG whole_u8_idx;
  ULONG whole_u8_idx_delta0;
  ULONG whole_u8_idx_delta1;
  ULONG whole_x_idx_max;
  ULONG whole_x_idx_post;
  ULONG window_x_idx_max;
  ULONG window_x_idx_post;
  ULONG window_y_idx;
  ULONG window_y_idx_max;

  discount_status=entropy_base->discount_status;
  entropy_base->discount_status=0;
  status=transform_whole_list_window_entropy_get_fru64(entropy_base, transform_base, whole_u8_list_base, 0, whole_y_idx);
  entropy_base->discount_status=discount_status;
  rounding_status=entropy_base->rounding_status;
  entropy_max_max=entropy_base->max_max_u64;
  entropy.b=entropy_max_max;
  entropy_max_max_max=entropy_base->window_max_max_fru64;
  entropy_max_max_x_idx=0;
  entropy_min_min=entropy_base->min_min_u64;
  entropy.a=entropy_min_min;
  entropy_min_min_x_idx=0;
  entropy_u32_idx=0;
  entropy_u32_list_base=entropy_base->fakefloat_list_base;
  entropy_x_idx_max=entropy_base->fakefloat_x_idx_max;
  if(entropy_u32_list_base){
    entropy_copy=entropy;
    if(discount_status){
      FRU64_SUBTRACT_FRU64(entropy_copy, entropy_max_max_max, entropy, status);
    }
    entropy_u32_idx=(entropy_x_idx_max+1)*whole_y_idx;
    if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEAREST_EVEN){
      entropy_u32=fracterval_u64_to_fakefloat(entropy, U32_BITS);
    }else if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEGATIVE){
      entropy_u32=fractoid_u64_to_fakefloat_floor(entropy.a, U32_BITS);
    }else{
      entropy_u32=fractoid_u64_to_fakefloat_ceil(entropy.b, U32_BITS);
    }
    entropy_u32_list_base[entropy_u32_idx]=entropy_u32;
    entropy_u32_idx++;
  }
  algo_status=transform_base->algo_status;
  entropy_delta_list_base=transform_base->entropy_delta_fru64_list_base;
  entropy_x_idx=1;
  freq_list_idx=0;
  freq_list_base_list_base=transform_base->freq_list_base_list_base;
  granularity=entropy_base->granularity;
  lookup_list_base=transform_base->log_factorial_fru64_list_base;
  if(algo_status==TRANSFORM_ALGO_STATUS_SHANNON){
    lookup_list_base=transform_base->log_fru64_list_base;
  }
  optimize_status=transform_base->optimize_status;
  pop_list_base=transform_base->pop_list_base;
  pop_list_base_list_base=transform_base->pop_list_base_list_base;
  whole_size=(u8)(granularity+1);
  window_x_idx_max=entropy_base->window_x_idx_max;
  window_x_idx_post=window_x_idx_max+1;
  freq_list_idx_delta=(optimize_status?window_x_idx_post:freq_list_idx);
  whole_max_max=entropy_base->whole_max_max;
  whole_u8_idx_delta0=whole_size*window_x_idx_post;
  whole_x_idx_max=entropy_base->whole_x_idx_max;
  whole_u8_idx_delta1=whole_size*whole_x_idx_max;
  whole_x_idx_post=whole_x_idx_max+1;
  window_y_idx_max=entropy_base->window_y_idx_max;
  while(entropy_x_idx<=entropy_x_idx_max){
    entropy_delta=entropy_delta_list_base[freq_list_idx];
    FRU64_ADD_FRU64_SELF(entropy, entropy_delta, status);
    freq_list_base=freq_list_base_list_base[freq_list_idx+freq_list_idx_delta];
    if(algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM){
      pop_list_base=pop_list_base_list_base[freq_list_idx+freq_list_idx_delta];
    }
    whole_u8_idx=whole_size*(entropy_x_idx-1+whole_x_idx_post*whole_y_idx);
    window_y_idx=0;
    do{
      whole_old=whole_u8_list_base[whole_u8_idx];
      whole=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta0];
      whole_u8_idx++;
      if(granularity){
        digit=whole_u8_list_base[whole_u8_idx];
        whole_old|=digit<<U8_BITS;
        digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta0];
        whole|=digit<<U8_BITS;
        whole_u8_idx++;
        if(U16_BYTE_MAX<granularity){
          digit=whole_u8_list_base[whole_u8_idx];
          whole_old|=digit<<U8_BITS;
          digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta0];
          whole|=digit<<U8_BITS;
          whole_u8_idx++;
          if(U24_BYTE_MAX<granularity){
            digit=whole_u8_list_base[whole_u8_idx];
            whole_old|=digit<<U8_BITS;
            digit=whole_u8_list_base[whole_u8_idx+whole_u8_idx_delta0];
            whole|=digit<<U8_BITS;
            whole_u8_idx++;
          }
        }
      }
      whole_u8_idx+=whole_u8_idx_delta1;
      freq_list_base[whole]++;
      if(!optimize_status){
        freq_list_base[whole_old]--;
      }
    }while((window_y_idx++)!=window_y_idx_max);
    status_delta=transform_freq_list_entropy_get_fru64(algo_status, &entropy_delta, freq_list_base, lookup_list_base, pop_list_base, window_y_idx_max, whole_max_max);
    status=(u8)(status|status_delta);
    entropy_delta_list_base[freq_list_idx+freq_list_idx_delta]=entropy_delta;
    FRU64_SUBTRACT_FRU64_SELF(entropy, entropy_delta, status);
    if(entropy_max_max<entropy.b){
      entropy_max_max=entropy.b;
      entropy_max_max_x_idx=entropy_x_idx;
    }
    if(entropy.a<entropy_min_min){
      entropy_min_min=entropy.a;
      entropy_min_min_x_idx=entropy_x_idx;
    }
    if(entropy_u32_list_base){
      entropy_copy=entropy;
      if(discount_status){
        FRU64_SUBTRACT_FRU64(entropy_copy, entropy_max_max_max, entropy, status);
      }
      if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEAREST_EVEN){
        entropy_u32=fracterval_u64_to_fakefloat(entropy_copy, U32_BITS);
      }else if(rounding_status==TRANSFORM_ROUNDING_STATUS_NEGATIVE){
        entropy_u32=fractoid_u64_to_fakefloat_floor(entropy_copy.a, U32_BITS);
      }else{
        entropy_u32=fractoid_u64_to_fakefloat_ceil(entropy_copy.b, U32_BITS);
      }
      entropy_u32_list_base[entropy_u32_idx]=entropy_u32;
      entropy_u32_idx++;
    }
    freq_list_idx++;
    if((freq_list_idx==window_x_idx_post)&&!optimize_status){
      freq_list_idx=0;
    }
    entropy_x_idx++;
  }
  entropy_base->max_max_u64=entropy_max_max;
  entropy_base->max_max_x_idx=entropy_max_max_x_idx;
  entropy_base->min_min_u64=entropy_min_min;
  entropy_base->min_min_x_idx=entropy_min_min_x_idx;
  return status;
}

u8
transform_whole_list_window_entropy_get_fru128(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_x_idx_min, ULONG whole_y_idx_min){
/*
Compute the sum of channel entropy deltas (to be subtracted from (entropy_base->window_bias_fru128)) for a given window position.

In:

  entropy_base is transform_malloc():Out:*entropy_base_base.

  transform_base is transform_malloc():Out:*transform_base_base.

  whole_u8_list_base is the base of (entropy_base->whole_count) wholes, each of size (entropy_base->granularity+1) and not exceeding (entropy_base->whole_max_max).

  whole_x_idx_min is the the least column index of the window in *whole_u8_list_base.

  whole_x_idx_min is the the least row index of the window in *whole_u8_list_base.

Out:

  Returns one if fracterval underflow or overflow occured (which is usually benign) else zero.

  *entropy_base is updated to reflect that both the minimum and maximum entropy occur in the window with upper left corner (whole_x_idx_min, whole_y_idx_min). The entropy of the window is also stored as both the minimum and maximum entropy. Thus the following items are updated: max_max_u128, max_max_x_idx, max_max_y_idx, min_min_u128, min_min_x_idx, and min_min_y_idx.
*/
  u8 algo_status;
  u32 digit;
  fru128 entropy;
  fru128 entropy_delta;
  fru128 *entropy_delta_list_base;
  ULONG *freq_list_base;
  ULONG **freq_list_base_list_base;
  u8 granularity;
  fru128 *lookup_list_base;
  ULONG *pop_list_base;
  ULONG **pop_list_base_list_base;
  u8 status;
  u8 status_delta;
  u32 whole;
  u32 whole_max_max;
  u8 whole_size;
  ULONG whole_u8_idx;
  ULONG whole_u8_idx_delta;
  ULONG whole_x_idx_max;
  ULONG whole_x_idx_post;
  ULONG window_x_idx;
  ULONG window_x_idx_max;
  ULONG window_y_idx;
  ULONG window_y_idx_max;

  freq_list_base_list_base=transform_base->freq_list_base_list_base;
  whole_max_max=entropy_base->whole_max_max;
  window_x_idx=0;
  window_x_idx_max=entropy_base->window_x_idx_max;
  do{
    freq_list_base=freq_list_base_list_base[window_x_idx];
    transform_ulong_list_zero(whole_max_max, freq_list_base);
  }while((window_x_idx++)!=window_x_idx_max);
  granularity=entropy_base->granularity;
  whole_size=(u8)(granularity+1);
  whole_x_idx_max=entropy_base->whole_x_idx_max;
  whole_x_idx_post=whole_x_idx_max+1;
  whole_u8_idx=whole_size*(whole_x_idx_min+whole_x_idx_post*whole_y_idx_min);
  whole_u8_idx_delta=whole_size*(whole_x_idx_max-window_x_idx_max);
  window_y_idx=0;
  window_y_idx_max=entropy_base->window_y_idx_max;
  do{
    for(window_x_idx=0; window_x_idx<=window_x_idx_max; window_x_idx++){
      freq_list_base=freq_list_base_list_base[window_x_idx];
      whole=whole_u8_list_base[whole_u8_idx];
      whole_u8_idx++;
      if(granularity){
        digit=whole_u8_list_base[whole_u8_idx];
        whole|=digit<<U8_BITS;
        whole_u8_idx++;
        if(U16_BYTE_MAX<granularity){
          digit=whole_u8_list_base[whole_u8_idx];
          whole|=digit<<U16_BITS;
          whole_u8_idx++;
          if(U24_BYTE_MAX<granularity){
            digit=whole_u8_list_base[whole_u8_idx];
            whole|=digit<<U24_BITS;
            whole_u8_idx++;
          }
        }
      }
      freq_list_base[whole]++;
    }
    whole_u8_idx+=whole_u8_idx_delta;
  }while((window_y_idx++)!=window_y_idx_max);
  algo_status=transform_base->algo_status;
  status=0;
  entropy=entropy_base->window_bias_fru128;
  entropy_delta_list_base=transform_base->entropy_delta_fru128_list_base;
  lookup_list_base=transform_base->log_factorial_fru128_list_base;
  if(algo_status==TRANSFORM_ALGO_STATUS_SHANNON){
    lookup_list_base=transform_base->log_fru128_list_base;
  }
  pop_list_base=transform_base->pop_list_base;
  pop_list_base_list_base=transform_base->pop_list_base_list_base;
  window_x_idx=0;
  do{
    freq_list_base=freq_list_base_list_base[window_x_idx];
    if(algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM){
      pop_list_base=pop_list_base_list_base[window_x_idx];
    }
    status_delta=transform_freq_list_entropy_get_fru128(algo_status, &entropy_delta, freq_list_base, lookup_list_base, pop_list_base, window_y_idx_max, whole_max_max);
    status=(u8)(status|status_delta);
    entropy_delta_list_base[window_x_idx]=entropy_delta;
    FRU128_SUBTRACT_FRU128_SELF(entropy, entropy_delta, status);
  }while((window_x_idx++)!=window_x_idx_max);
  entropy_base->max_max_u128=entropy.b;
  entropy_base->max_max_x_idx=whole_x_idx_min;
  entropy_base->max_max_y_idx=whole_y_idx_min;
  entropy_base->min_min_u128=entropy.a;
  entropy_base->min_min_x_idx=whole_x_idx_min;
  entropy_base->min_min_y_idx=whole_y_idx_min;
  if(entropy_base->discount_status){
    status_delta=transform_entropy_to_discount(entropy_base);
    status=(u8)(status|status_delta);
  }
  return status;
}

u8
transform_whole_list_window_entropy_get_fru64(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_x_idx_min, ULONG whole_y_idx_min){
/*
This is the 64-bit fracterval equivalent of transform_whole_list_window_entropy_get_fru128(). Replace all occurrences of "128" with "64" in its definition.
*/
  u8 algo_status;
  u32 digit;
  fru64 entropy;
  fru64 entropy_delta;
  fru64 *entropy_delta_list_base;
  ULONG *freq_list_base;
  ULONG **freq_list_base_list_base;
  u8 granularity;
  fru64 *lookup_list_base;
  ULONG *pop_list_base;
  ULONG **pop_list_base_list_base;
  u8 status;
  u8 status_delta;
  u32 whole;
  u32 whole_max_max;
  u8 whole_size;
  ULONG whole_u8_idx;
  ULONG whole_u8_idx_delta;
  ULONG whole_x_idx_max;
  ULONG whole_x_idx_post;
  ULONG window_x_idx;
  ULONG window_x_idx_max;
  ULONG window_y_idx;
  ULONG window_y_idx_max;

  freq_list_base_list_base=transform_base->freq_list_base_list_base;
  whole_max_max=entropy_base->whole_max_max;
  window_x_idx=0;
  window_x_idx_max=entropy_base->window_x_idx_max;
  do{
    freq_list_base=freq_list_base_list_base[window_x_idx];
    transform_ulong_list_zero(whole_max_max, freq_list_base);
  }while((window_x_idx++)!=window_x_idx_max);
  granularity=entropy_base->granularity;
  whole_size=(u8)(granularity+1);
  whole_x_idx_max=entropy_base->whole_x_idx_max;
  whole_x_idx_post=whole_x_idx_max+1;
  whole_u8_idx=whole_size*(whole_x_idx_min+whole_x_idx_post*whole_y_idx_min);
  whole_u8_idx_delta=whole_size*(whole_x_idx_max-window_x_idx_max);
  window_y_idx=0;
  window_y_idx_max=entropy_base->window_y_idx_max;
  do{
    for(window_x_idx=0; window_x_idx<=window_x_idx_max; window_x_idx++){
      freq_list_base=freq_list_base_list_base[window_x_idx];
      whole=whole_u8_list_base[whole_u8_idx];
      whole_u8_idx++;
      if(granularity){
        digit=whole_u8_list_base[whole_u8_idx];
        whole|=digit<<U8_BITS;
        whole_u8_idx++;
        if(U16_BYTE_MAX<granularity){
          digit=whole_u8_list_base[whole_u8_idx];
          whole|=digit<<U16_BITS;
          whole_u8_idx++;
          if(U24_BYTE_MAX<granularity){
            digit=whole_u8_list_base[whole_u8_idx];
            whole|=digit<<U24_BITS;
            whole_u8_idx++;
          }
        }
      }
      freq_list_base[whole]++;
    }
    whole_u8_idx+=whole_u8_idx_delta;
  }while((window_y_idx++)!=window_y_idx_max);
  algo_status=transform_base->algo_status;
  status=0;
  entropy=entropy_base->window_bias_fru64;
  entropy_delta_list_base=transform_base->entropy_delta_fru64_list_base;
  lookup_list_base=transform_base->log_factorial_fru64_list_base;
  if(algo_status==TRANSFORM_ALGO_STATUS_SHANNON){
    lookup_list_base=transform_base->log_fru64_list_base;
  }
  pop_list_base=transform_base->pop_list_base;
  pop_list_base_list_base=transform_base->pop_list_base_list_base;
  window_x_idx=0;
  do{
    freq_list_base=freq_list_base_list_base[window_x_idx];
    if(algo_status==TRANSFORM_ALGO_STATUS_LOGFREEDOM){
      pop_list_base=pop_list_base_list_base[window_x_idx];
    }
    status_delta=transform_freq_list_entropy_get_fru64(algo_status, &entropy_delta, freq_list_base, lookup_list_base, pop_list_base, window_y_idx_max, whole_max_max);
    status=(u8)(status|status_delta);
    entropy_delta_list_base[window_x_idx]=entropy_delta;
    FRU64_SUBTRACT_FRU64_SELF(entropy, entropy_delta, status);
  }while((window_x_idx++)!=window_x_idx_max);
  entropy_base->max_max_u64=entropy.b;
  entropy_base->max_max_x_idx=whole_x_idx_min;
  entropy_base->max_max_y_idx=whole_y_idx_min;
  entropy_base->min_min_u64=entropy.a;
  entropy_base->min_min_x_idx=whole_x_idx_min;
  entropy_base->min_min_y_idx=whole_y_idx_min;
  if(entropy_base->discount_status){
    status_delta=transform_entropy_to_discount(entropy_base);
    status=(u8)(status|status_delta);
  }
  return status;
}

u8
transform_window_entropy_offsets_get(entropy_t *entropy_base, transform_t *transform_base){
/*
Get the bias and maximum possible entropy for a single column within a window, then multiply it by the number of columns therein. The "bias" refers to the sum of terms in an entropy formula which depend only on the number of wholes in question (Q) and the number of unique wholes permitted (Z). The point is that it's better to compute these values once, then only worry about them when we need to issue an output, relying instead on entropy deltas to determine whether or not a given entropy value is even worth saving.

In:

  entropy_base is transform_malloc():Out:*entropy_base_base.

  transform_base is transform_malloc():Out:*transform_base_base.

Out:

  Returns one if fracterval underflow or overflow occured (which should be regarded as a fatal error in this particular case) else zero. Return values are defined, or not, depending upon (entropy_base->precise_status). This is true even though computing logfreedom requires lookup lists of both precisions; this was already handled by transform_malloc().

  entropy_base->window_bias_fru(64/128) is the entropy bias per column times the number of columns. Each column contains (entropy_base->window_y_idx_post) wholes. Each window contains (entropy_base->window_x_idx_post) columns.

  entropy_base->window_max_max_fru(64/128) is the maximum possible entropy per column times the number of columns in such a window.
*/
  u8 algo_status;
  ULONG freq;
  ULONG pop;
  fru128 *lookup_fru128_list_base;
  fru64 *lookup_fru64_list_base;
  u8 precise_status;
  u8 status;
  u8 status_delta;
  u64 term_u64;
  u32 whole_max_max;
  ULONG whole_span_max;
  fru128 window_offset_fru128_0;
  fru128 window_offset_fru128_1;
  fru64 window_offset_fru64_0;
  fru64 window_offset_fru64_1;
  ULONG window_x_idx_post;
  ULONG window_y_idx_post;

  algo_status=transform_base->algo_status;
  precise_status=entropy_base->precise_status;
  status=0;
  whole_max_max=entropy_base->whole_max_max;
  whole_span_max=(ULONG)(whole_max_max)+1;
  window_x_idx_post=entropy_base->window_x_idx_post;
  window_y_idx_post=entropy_base->window_y_idx_post;
  if(algo_status==TRANSFORM_ALGO_STATUS_AGNENTROPY){
/*
Maximum agnentropy occurs when all wholes have as close to equal frequency as possible.
*/
    freq=window_y_idx_post/whole_span_max;
    pop=window_y_idx_post%whole_span_max;
    term_u64=window_x_idx_post;
    if(!precise_status){
      lookup_fru64_list_base=transform_base->log_factorial_fru64_list_base;
      window_offset_fru64_0=lookup_fru64_list_base[whole_max_max+window_y_idx_post];
      window_offset_fru64_1=lookup_fru64_list_base[whole_max_max];
      FRU64_SUBTRACT_FRU64_SELF(window_offset_fru64_0, window_offset_fru64_1, status);
      FRU64_MULTIPLY_U64(window_offset_fru64_1, window_offset_fru64_0, term_u64, status);
      entropy_base->window_bias_fru64=window_offset_fru64_1;
      if(freq&&pop){
        window_offset_fru64_1=lookup_fru64_list_base[freq+1];
        term_u64=pop;
        FRU64_MULTIPLY_U64_SELF(window_offset_fru64_1, term_u64, status);
        FRU64_SUBTRACT_FRU64_SELF(window_offset_fru64_0, window_offset_fru64_1, status);
      }
      pop=whole_span_max-pop;
      if((1<freq)&&pop){
        window_offset_fru64_1=lookup_fru64_list_base[freq];
        term_u64=pop;
        FRU64_MULTIPLY_U64_SELF(window_offset_fru64_1, term_u64, status);
        FRU64_SUBTRACT_FRU64_SELF(window_offset_fru64_0, window_offset_fru64_1, status);
      }
      term_u64=window_x_idx_post;
      FRU64_MULTIPLY_U64_SELF(window_offset_fru64_0, term_u64, status);
      entropy_base->window_max_max_fru64=window_offset_fru64_0;
    }else{
      lookup_fru128_list_base=transform_base->log_factorial_fru128_list_base;
      window_offset_fru128_0=lookup_fru128_list_base[whole_max_max+window_y_idx_post];
      window_offset_fru128_1=lookup_fru128_list_base[whole_max_max];
      FRU128_SUBTRACT_FRU128_SELF(window_offset_fru128_0, window_offset_fru128_1, status);
      FRU128_MULTIPLY_U64(window_offset_fru128_1, window_offset_fru128_0, term_u64, status);
      entropy_base->window_bias_fru128=window_offset_fru128_1;
      if(freq&&pop){
        window_offset_fru128_1=lookup_fru128_list_base[freq+1];
        term_u64=pop;
        FRU128_MULTIPLY_U64_SELF(window_offset_fru128_1, term_u64, status);
        FRU128_SUBTRACT_FRU128_SELF(window_offset_fru128_0, window_offset_fru128_1, status);
      }
      pop=whole_span_max-pop;
      if((1<freq)&&pop){
        window_offset_fru128_1=lookup_fru128_list_base[freq];
        term_u64=pop;
        FRU128_MULTIPLY_U64_SELF(window_offset_fru128_1, term_u64, status);
        FRU128_SUBTRACT_FRU128_SELF(window_offset_fru128_0, window_offset_fru128_1, status);
      }
      term_u64=window_x_idx_post;
      FRU128_MULTIPLY_U64_SELF(window_offset_fru128_0, term_u64, status);
      entropy_base->window_max_max_fru128=window_offset_fru128_0;
    }
  }else if(algo_status==TRANSFORM_ALGO_STATUS_SHANNON){
/*
Maximum Shannon entropy occurs when all wholes have as close to equal frequency as possible.
*/
    freq=window_y_idx_post/whole_span_max;
    pop=window_y_idx_post%whole_span_max;
    term_u64=window_y_idx_post;
    if(!precise_status){
      lookup_fru64_list_base=transform_base->log_fru64_list_base;
      window_offset_fru64_1=lookup_fru64_list_base[window_y_idx_post];
      FRU64_MULTIPLY_U64(window_offset_fru64_0, window_offset_fru64_1, term_u64, status);
      term_u64=window_x_idx_post;
      FRU64_MULTIPLY_U64(window_offset_fru64_1, window_offset_fru64_0, term_u64, status);
      entropy_base->window_bias_fru64=window_offset_fru64_1;
      if(freq&&pop){
        window_offset_fru64_1=lookup_fru64_list_base[freq+1];
        term_u64=freq+1;
        FRU64_MULTIPLY_U64_SELF(window_offset_fru64_1, term_u64, status);
        term_u64=pop;
        FRU64_MULTIPLY_U64_SELF(window_offset_fru64_1, term_u64, status);
        FRU64_SUBTRACT_FRU64_SELF(window_offset_fru64_0, window_offset_fru64_1, status);
      }
      pop=whole_span_max-pop;
      if((1<freq)&&pop){
        window_offset_fru64_1=lookup_fru64_list_base[freq];
        term_u64=freq;
        FRU64_MULTIPLY_U64_SELF(window_offset_fru64_1, term_u64, status);
        term_u64=pop;
        FRU64_MULTIPLY_U64_SELF(window_offset_fru64_1, term_u64, status);
        FRU64_SUBTRACT_FRU64_SELF(window_offset_fru64_0, window_offset_fru64_1, status);
      }
      term_u64=window_x_idx_post;
      FRU64_MULTIPLY_U64_SELF(window_offset_fru64_0, term_u64, status);
      entropy_base->window_max_max_fru64=window_offset_fru64_0;
    }else{
      lookup_fru128_list_base=transform_base->log_fru128_list_base;
      window_offset_fru128_1=lookup_fru128_list_base[window_y_idx_post];
      FRU128_MULTIPLY_U64(window_offset_fru128_0, window_offset_fru128_1, term_u64, status);
      term_u64=window_x_idx_post;
      FRU128_MULTIPLY_U64(window_offset_fru128_1, window_offset_fru128_0, term_u64, status);
      entropy_base->window_bias_fru128=window_offset_fru128_1;
      if(freq&&pop){
        window_offset_fru128_1=lookup_fru128_list_base[freq+1];
        term_u64=freq+1;
        FRU128_MULTIPLY_U64_SELF(window_offset_fru128_1, term_u64, status);
        term_u64=pop;
        FRU128_MULTIPLY_U64_SELF(window_offset_fru128_1, term_u64, status);
        FRU128_SUBTRACT_FRU128_SELF(window_offset_fru128_0, window_offset_fru128_1, status);
      }
      pop=whole_span_max-pop;
      if((1<freq)&&pop){
        window_offset_fru128_1=lookup_fru128_list_base[freq];
        term_u64=freq;
        FRU128_MULTIPLY_U64_SELF(window_offset_fru128_1, term_u64, status);
        term_u64=pop;
        FRU128_MULTIPLY_U64_SELF(window_offset_fru128_1, term_u64, status);
        FRU128_SUBTRACT_FRU128_SELF(window_offset_fru128_0, window_offset_fru128_1, status);
      }
      term_u64=window_x_idx_post;
      FRU128_MULTIPLY_U64_SELF(window_offset_fru128_0, term_u64, status);
      entropy_base->window_max_max_fru128=window_offset_fru128_0;
    }
  }else{
    lookup_fru128_list_base=transform_base->log_factorial_fru128_list_base;
    window_offset_fru128_0=lookup_fru128_list_base[window_y_idx_post];
    window_offset_fru128_1=lookup_fru128_list_base[whole_span_max];
    FRU128_ADD_FRU128_SELF(window_offset_fru128_1, window_offset_fru128_0, status);
/*
We don't actually have a generic method for computing maximum logfreedom, although there are good heuristics. (Neither gradient descent nor greedy optimization works all the time.) At slight risk of fracterval overflow, we can approximate it accurately. The iteration_max parameter is a gross overestimate even in the limits of maximal parameters, just to be safe.
*/
    status_delta=transform_logfreedom_max_approximate(entropy_base, U64_BITS, &window_offset_fru128_0, transform_base);
    status=(u8)(status|status_delta);
    term_u64=window_x_idx_post;
    FRU128_MULTIPLY_U64_SELF(window_offset_fru128_0, term_u64, status);
    FRU128_MULTIPLY_U64_SELF(window_offset_fru128_1, term_u64, status);
    if(!precise_status){
      FRU128_TO_FRU64_MID(window_offset_fru64_0, window_offset_fru128_0);
      FRU128_TO_FRU64_MID(window_offset_fru64_1, window_offset_fru128_1);
      entropy_base->window_bias_fru64=window_offset_fru64_1;
      entropy_base->window_max_max_fru64=window_offset_fru64_0;
    }else{
      entropy_base->window_bias_fru128=window_offset_fru128_1;
      entropy_base->window_max_max_fru128=window_offset_fru128_0;
    }
  }
  return status;
}
