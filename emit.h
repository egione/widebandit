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
These are predefined text emission priority levels, where 0 is critical, 1 is error, 2 is warning, and 3 is progress updates.
*/
#define EMIT0 0U
#define EMIT1 1U
#define EMIT2 2U
#define EMIT3 3U
/*
These macros are redundant, but they provide an easy way to redirect text emissions if necessary.
*/
#define EMIT_BIGUINT(name_base, chunk_idx_max, chunk_list_base) emit_biguint(name_base, chunk_idx_max, chunk_list_base)
#define EMIT_BITMAP(name_base, bit_count, bit_idx_min, chunk_list_base) emit_bitmap(name_base, bit_count, bit_idx_min, chunk_list_base)
#define EMIT_DOUBLE(name_base, value) emit_double(name_base, value)
#define EMIT_F128(name_base, value) emit_f128(name_base, value)
#define EMIT_F128_PAIR(name_base, value0, value1) emit_f128_pair(name_base, value0, value1)
#define EMIT_F64(name_base, value) emit_f64(name_base, value)
#define EMIT_F64_PAIR(name_base, value0, value1) emit_f64_pair(name_base, value0, value1)
#define EMIT_FLOAT(name_base, value) debug_float(name_base, value)
#define EMIT_LIST(name_base, chunk_count, list_base, chunk_size_log2) emit_list(name_base, chunk_count, list_base, chunk_size_log2)
#define EMIT_NAME(name_base) emit_name(name_base)
#define EMIT_PRINT(string_base) emit_print(string_base)
#define EMIT_PRINT_FLUSH(emit_priority, emit_verbosity) emit_print_flush()
#define EMIT_PRINT_NEWLINE_IF(string_base) emit_print_newline_if(char *name_base)
#define EMIT_U128(name_base, value) emit_u128(name_base, value)
#define EMIT_U128_PAIR(name_base, value0, value1) emit_u128_pair(name_base, value0, value1)
#define EMIT_U16(name_base, value) emit_u16(name_base, value)
#define EMIT_U24(name_base, value) emit_u24(name_base, value)
#define EMIT_U32(name_base, value) emit_u32(name_base, value)
#define EMIT_U64(name_base, value) emit_u64(name_base, value)
#define EMIT_U64_DECIMAL(name_base, value) emit_u64_decimal(name_base, value)
#define EMIT_U64_PAIR(name_base, value0, value1) emit_u64_pair(name_base, value0, value1)
#define EMIT_U8(name_base, value) emit_u8(name_base, value)
#define EMIT_WRITE(string_base) emit_write(string_base)
