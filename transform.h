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
#define TRANSFORM_ALGO_STATUS_MAX 2U
#define TRANSFORM_ALGO_STATUS_SHANNON 0U
#define TRANSFORM_ALGO_STATUS_AGNENTROPY 1U
#define TRANSFORM_ALGO_STATUS_LOGFREEDOM 2U
#define TRANSFORM_DELTA_DELTA_FREQ_MAX (TRANSFORM_DELTA_DELTA_FREQ_POST-1U)
#define TRANSFORM_DELTA_DELTA_FREQ_POST (1U<<TRANSFORM_DELTA_DELTA_IDX_BITS)
#define TRANSFORM_DELTA_DELTA_IDX_BITS 4U
#define TRANSFORM_E_FRACTION 0xB7E151628AED2A6AULL
#define TRANSFORM_LOG_CACHE_IDX_MAX U8_MAX
#define TRANSFORM_ROUNDING_STATUS_MAX 2U
#define TRANSFORM_ROUNDING_STATUS_NEAREST_EVEN 0U
#define TRANSFORM_ROUNDING_STATUS_NEGATIVE 1U
#define TRANSFORM_ROUNDING_STATUS_POSITIVE 2U

TYPEDEF_START
  fru128 *entropy_delta_delta_fru128_list_base;
  fru64 *entropy_delta_delta_fru64_list_base;
  fru128 *entropy_delta_fru128_list_base;
  fru64 *entropy_delta_fru64_list_base;
  ULONG **freq_list_base_list_base;  
  fru128 *log_delta_fru128_list_base;
  fru64 *log_delta_fru64_list_base;
  fru128 *log_factorial_fru128_list_base;
  fru64 *log_factorial_fru64_list_base;
  fru128 *log_fru128_list_base;
  fru64 *log_fru64_list_base;
  ULONG *pop_list_base;
  ULONG **pop_list_base_list_base;
  ULONG *pop_list_best_base;
  ULONG *pop_list_best_best_base;
  ULONG freq_list_idx_max;
  u8 algo_status;
  u8 optimize_status;
TYPEDEF_END(transform_t)

TYPEDEF_START
  fru128 window_bias_fru128;
  fru128 window_max_max_fru128;
  u128 max_max_u128;
  u128 min_min_u128;
  fru64 window_bias_fru64;
  fru64 window_max_max_fru64;
  u64 max_max_u64;
  u64 min_min_u64;
  u32 *fakefloat_list_base;
  ULONG fakefloat_x_idx_max;
  ULONG fakefloat_y_idx_max;
  ULONG max_max_x_idx;
  ULONG max_max_y_idx;
  ULONG min_min_x_idx;
  ULONG min_min_y_idx;
  ULONG whole_count;
  ULONG whole_x_idx_max;
  ULONG whole_x_idx_post;
  ULONG whole_y_idx_max;
  ULONG whole_y_idx_post;
  ULONG window_x_idx_max;
  ULONG window_x_idx_post;
  ULONG window_y_idx_max;
  ULONG window_y_idx_post;
  u32 whole_max_max;
  u8 granularity;
  u8 discount_status;
  u8 precise_status;
  u8 rounding_status;
TYPEDEF_END(entropy_t)
