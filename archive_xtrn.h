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
extern u8 archive_downsample(ULONG slice_count_idx_max, ULONG *slice_count_list_base, u8 *sort_status_base, u32 *u32_list_base, u32 **u32_list_base_list_base);
extern void *archive_free(void *base);
extern u8 archive_header_check(u64 archive_size, header_t *header_base, u64 *lmd2_iterand_base, u8 lmd2_iterand_check_status, u64 *lmd2_partial_sum_base, ULONG *pdf_count_base, ULONG *pdf_float_count_base, ULONG *pdf_slot_count_base);
extern ULONG archive_header_export(header_t *header_base, u32 *u32_list_base);
extern void archive_header_fill(u8 hash_reset_status, header_t *header_base, ULONG pdf_float_count, ULONG pdf_float_count_old, ULONG pdf_idx_max, ULONG pdf_slot_idx_max, u32 *u32_list_base);
extern header_t *archive_header_init(void);
extern u64 archive_header_lmd2_get(header_t *header_base);
extern void archive_header_import(header_t *header_base, u32 *u32_list_base);
extern u8 archive_init(u32 build_break_count, u32 build_feature_count);
extern u32 **archive_list_free(ULONG u32_count_idx_max, u32 **u32_list_base_list_base);
extern u32 **archive_list_malloc(ULONG pdf_count, ULONG u32_count_idx_max, ULONG *u32_count_list_base);
extern void archive_lmd2_u64_digest(u64 *lmd2, u32 *lmd2_c_base, u32 *lmd2_x_base, u64 *lmd2_iterand_base, u64 u64_0);
extern ULONG archive_size_check(u64 archive_size);
extern u8 archive_sort_compact(u8 compact_status, ULONG delete_u32_count, header_t *header_base, u8 sign_status, u32 *u32_list_base);
extern void archive_u32_pair_from_u64(ULONG u32_idx, u32 *u32_list_base, u64 u64_0);
extern u64 archive_u64_from_u32_pair(ULONG u32_idx, u32 *u32_list_base);
