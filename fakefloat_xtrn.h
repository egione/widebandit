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
extern void *fakefloat_free(void *base);
extern ULONG *fakefloat_freq_list_malloc(void);
extern u8 fakefloat_init(u32 build_break_count, u32 build_feature_count);
extern u32 fakefloat_to_slice_idx(ULONG slice_idx_max, ULONG slice_idx_min, u32 u32_0, u32 *slice_list_base);
extern u8 fakefloat_u32_list_check(u32 *checksum_base, u64 *lmd2_iterand_base, u64 *lmd2_partial_base, u8 *sort_status_base, ULONG u32_idx_max, ULONG u32_idx_min, ULONG *u32_idx_min_infinity_base, ULONG *u32_idx_min_nan_base, ULONG *u32_idx_min_negative_base, ULONG *u32_idx_min_subnormal_base, ULONG *u32_idx_min_zero_minus_base, ULONG *u32_idx_min_zero_plus_base, u32 *u32_list_base);
extern ULONG fakefloat_u32_list_empty_check(ULONG u32_idx_max, ULONG u32_idx_min, u32 *u32_list_base);
extern u8 fakefloat_u32_list_find(u32 u32_0, ULONG u32_idx_max, ULONG *u32_idx_min_base, u32 *u32_list_base);
extern u64 fakefloat_u32_list_hash(u64 *lmd2_iterand_base, ULONG u32_idx_max, ULONG u32_idx_min, u32 *u32_list_base);
extern void fakefloat_u32_list_inject(ULONG block_idx_max, ULONG from_u32_idx_min, u32 *from_u32_list_base, ULONG inject_u32_count, ULONG to_block_u32_count, ULONG to_u32_idx_min, u32 *to_u32_list_base);
extern void fakefloat_u32_list_interleave(ULONG block_idx_max, ULONG interleave_u32_count, ULONG to_block_u32_count, ULONG to_block_u32_count_new, ULONG to_u32_idx_min, u32 *to_u32_list_base);
extern void fakefloat_u32_list_interleave_inject(ULONG block_idx_max, ULONG from_u32_idx_min, u32 *from_u32_list_base, ULONG inject_u32_count, ULONG interleave_u32_count, ULONG to_block_u32_count, ULONG to_block_u32_count_new, ULONG to_u32_idx_min, u32 *to_u32_list_base);
extern u32 *fakefloat_u32_list_malloc(u8 empty_status, ULONG u32_idx_max);
extern u8 fakefloat_u32_list_realloc(ULONG u32_idx_max, u32 **u32_list_base_base);
extern void fakefloat_u32_list_sort(ULONG *freq_list_base, u8 sign_status, ULONG u32_idx_max, u32 *u32_list_base0, u32 *u32_list_base1);
extern u8 fakefloat_u32_list_sort_check(ULONG u32_idx_max, ULONG u32_idx_min, u32 *u32_list_base);
extern void fakefloat_u32_list_transpose(u32 *u32_list_base0, u32 *u32_list_base1, ULONG u32_x_idx_max, ULONG u32_y_idx_max);
extern ULONG fakefloat_u32_sublist_compact(ULONG u32_idx_max, ULONG u32_idx_min, u32 *u32_list_base);
extern u8 fakefloat_u32_sublist_redact(ULONG from_u32_idx_max, ULONG from_u32_idx_min, u32 *from_u32_list_base, ULONG to_u32_idx_max, ULONG to_u32_idx_min, u32 *to_u32_list_base);
