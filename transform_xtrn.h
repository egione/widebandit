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
extern u8 transform_delta_delta_list_init(fru128 **entropy_delta_delta_fru128_list_base_base, fru64 **entropy_delta_delta_fru64_list_base_base, u8 precise_status);
extern u8 transform_dispatch(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base);
extern u8 transform_entropy_to_discount(entropy_t *entropy_base);
extern u32 *transform_fakefloat_list_malloc(ULONG *fakefloat_list_size_base, ULONG whole_x_idx_max, ULONG whole_y_idx_max, ULONG window_x_idx_max, ULONG window_y_idx_max);
extern void *transform_free(void *base);
extern void transform_free_all(entropy_t **entropy_base_base, transform_t **transform_base_base);
extern u8 transform_freq_list_entropy_get_fru128(u8 algo_status, fru128 *entropy_fru128_base, ULONG *freq_list_base, fru128 *lookup_list_base, ULONG *pop_list_base, ULONG whole_idx_max, u32 whole_max_max);
extern u8 transform_freq_list_entropy_get_fru64(u8 algo_status, fru64 *entropy_fru64_base, ULONG *freq_list_base, fru64 *lookup_list_base, ULONG *pop_list_base, ULONG whole_idx_max, u32 whole_max_max);
extern u8 transform_init(u32 build_break_count, u32 build_feature_count);
extern u8 transform_logfreedom_max_approximate(entropy_t *entropy_base, u32 iteration_max, fru128 *logfreedom_max_base, transform_t *transform_base);
extern void transform_lookup_lists_free(transform_t *transform_base);
extern u8 transform_lookup_lists_init(entropy_t *entropy_base, transform_t *transform_base);
extern void transform_malloc(u8 algo_status, u8 discount_status, entropy_t **entropy_base_base, u32 *fakefloat_list_base, u8 granularity, u8 optimize_status, u8 precise_status, u8 rounding_status, transform_t **transform_base_base, u32 whole_max_max, ULONG whole_x_idx_max, ULONG whole_y_idx_max, ULONG window_x_idx_max, ULONG window_y_idx_max);
extern void transform_pop_list_fill(ULONG *freq_list_base, ULONG *pop_list_base, ULONG whole_idx_max, u32 whole_max_max);
extern u8 transform_pop_list_logfreedom_get(fru128 *log_factorial_list_base, fru128 *logfreedom_base, ULONG *pop_list_base, ULONG whole_idx_max, u32 whole_max_max);
extern ULONG *transform_pop_list_malloc(ULONG whole_idx_max);
extern void transform_ulong_list_zero(ULONG ulong_idx_max, ULONG *ulong_list_base);
extern u8 transform_whole_list_line_entropy_delta_get_fru128(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_y_idx);
extern u8 transform_whole_list_line_entropy_delta_get_fru64(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_y_idx);
extern u8 transform_whole_list_line_entropy_get_fru128(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_y_idx);
extern u8 transform_whole_list_line_entropy_get_fru64(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_y_idx);
extern u8 transform_whole_list_window_entropy_get_fru128(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_x_idx_min, ULONG whole_y_idx_min);
extern u8 transform_whole_list_window_entropy_get_fru64(entropy_t *entropy_base, transform_t *transform_base, u8 *whole_u8_list_base, ULONG whole_x_idx_min, ULONG whole_y_idx_min);
extern u8 transform_window_entropy_offsets_get(entropy_t *entropy_base, transform_t *transform_base);
