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
#define ARCHIVE_HEADER_SIZE 0x40U
#define ARCHIVE_HEADER_U32_COUNT 0x10U
#define ARCHIVE_SIGNATURE 0x9A4AF8F2A5D4295DULL
/*
The size of header_t must be a multiple of U32_SIZE in order to fulfill alignment guarantees. Update ARCHIVE_HEADER_U32_COUNT if it changes.
*/
TYPEDEF_START
  u64 signature;
  u64 size_following;
  u64 lmd2_local;
  u64 lmd2_iterand_cache;
  u64 lmd2_partial_sum;
  u64 pdf_float_count;
  u64 pdf_slot_count;
  u64 pdf_count;
TYPEDEF_END(header_t)
