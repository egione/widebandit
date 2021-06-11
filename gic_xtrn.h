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
extern void *gic_free(void *base);
extern u8 gic_gauss_list_fill(ULONG float_idx_min, float *float_list_base, gauss_t *gauss_list_base, gic_t *gic_base, u8 log_status, ULONG slot_count);
extern gauss_t *gic_gauss_list_malloc(gic_t *gic_base);
extern u8 gic_init(u32 build_break_count, u32 build_feature_count);
extern gic_t *gic_malloc(ULONG float_count, ULONG pdf_count);
extern void gic_ranker_list_copy(gic_t *gic_base, gic_ranker_t *gic_ranker_list_base0, gic_ranker_t *gic_ranker_list_base1);
extern void gic_ranker_list_fill(gauss_t *gauss_list_base0, gauss_t *gauss_list_base1, gic_t *gic_base, gic_ranker_t *gic_ranker_list_base);
extern gic_ranker_t *gic_ranker_list_malloc(gic_t *gic_base);
extern void gic_ranker_list_scale(gic_t *gic_base, gic_ranker_t *gic_ranker_list_base);
extern u8 gic_ranker_list_sort(u8 ascending_status, gic_t *gic_base, gic_ranker_t *gic_ranker_list_base0, gic_ranker_t *gic_ranker_list_base1);
extern void gic_ranker_list_to_ratio_list(gic_t *gic_base, gic_ranker_t *gic_ranker_list_base);
