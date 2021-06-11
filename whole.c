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
#include "flag_whole.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "whole_xtrn.h"

void *
whole_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Whole calls free().

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
whole_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of WHOLE_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of WHOLE_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=WHOLE_BUILD_BREAK_COUNT) or (build_feature_count>WHOLE_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=WHOLE_BUILD_BREAK_COUNT);
  status=(u8)(status|(WHOLE_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}

u8 *
whole_list_malloc(u8 granularity, ULONG whole_idx_max){
/*
Allocate a list of whole numbers.

In:

  granularity is the size of a whole number, less one.

  whole_idx_max is the number of wholes to allocate, less one.

Out:

  Returns NULL on failure, else the base of (whole_idx_max+1) undefined wholes, each of size (granularity+1).
*/
  ULONG list_size;
  ULONG whole_count;
  u8 *whole_list_base;
  u8 whole_size;

  whole_count=whole_idx_max+1;
  whole_list_base=NULL;
  if(whole_count){
    whole_size=(u8)(granularity+1);
    list_size=whole_count*whole_size;
    if((list_size/whole_count)==whole_size){
      whole_list_base=DEBUG_MALLOC_PARANOID(list_size);
    }
  }
  return whole_list_base;
}

u32
whole_max_get(u8 granularity, ULONG whole_idx_max, u8 *whole_u8_list_base){
/*
Get the maximum whole in a list.

In:

  granularity is the size of each whole, less one.

  whole_idx_max is the maximum index of *whole_u8_list_base.

  whole_u8_list_base is defined on indexes [0, whole_idx_max] and contains wholes of size (granularity+1).

Out:

  Returns the maximum whole at *whole_u8_list_base. If (granularity==U32_BYTE_MAX), then this value could be U32_MAX, in which case the whole span (the maximum plus one) would wrap u32. Downstream processes need to be aware of this issue.
*/
  u32 digit;
  u32 whole;
  u32 whole_max;
  u8 whole_size;
  ULONG whole_u8_idx;
  ULONG whole_u8_idx_post;

  whole_max=0;
  whole_size=(u8)(granularity+1U);
  whole_u8_idx=0;
  whole_u8_idx_post=(whole_idx_max+1U)*whole_size;
  do{
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
    whole_max=MAX(whole, whole_max);
  }while(whole_u8_idx!=whole_u8_idx_post);
  return whole_max;
}
