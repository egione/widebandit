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
extern void *whole_free(void *base);
extern u8 whole_init(u32 build_break_count, u32 build_feature_count);
extern u8 *whole_list_malloc(u8 granularity, ULONG whole_idx_max);
extern u32 whole_max_get(u8 granularity, ULONG whole_idx_max, u8 *whole_u8_list_base);
