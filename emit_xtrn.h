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
extern void emit_biguint(char *name_base, ULONG chunk_idx_max, ULONG *chunk_list_base);
extern void emit_bitmap(char *name_base, u64 bit_count, u64 bit_idx_min, ULONG *chunk_list_base);
extern void emit_double(char *name_base, double value);
extern void emit_f128(char *name_base, u128 value);
extern void emit_f128_pair(char *name_base, u128 value0, u128 value1);
extern void emit_f64(char *name_base, u64 value);
extern void emit_f64_pair(char *name_base, u64 value0, u64 value1);
extern void emit_float(char *name_base, float value);
extern void emit_list(char *name_base, ULONG chunk_count, u8 *chunk_list_base, u8 chunk_size_log2);
extern void emit_name(char *name_base);
extern void emit_print(char *string_base);
extern void emit_print_flush();
extern void emit_print_newline_if(char *string_base);
extern void emit_u128(char *name_base, u128 value);
extern void emit_u128_pair(char *name_base, u128 value0, u128 value1);
extern void emit_u16(char *name_base, u16 value);
extern void emit_u24(char *name_base, u32 value);
extern void emit_u32(char *name_base, u32 value);
extern void emit_u64(char *name_base, u64 value);
extern void emit_u64_decimal(char *name_base, u64 value);
extern void emit_u64_pair(char *name_base, u64 value0, u64 value1);
extern void emit_u8(char *name_base, u8 value);
extern void emit_write(char *string_base);
