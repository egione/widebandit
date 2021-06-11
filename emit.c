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
Text Emitter
*/
#include "flag.h"
#include <stdint.h>
#include <stdio.h>
#include <string.h>
#include "constant.h"
#include "emit_xtrn.h"

void
emit_biguint(char *name_base, ULONG chunk_idx_max, ULONG *chunk_list_base){
/*
Print a hexadecimal big whole number starting with its most significant ULONG.

In:

  *name_base is a text identifier for the output.

  chunk_idx_max is the index containing the MSB of *chunk_list_base.

  *chunk_list_base is a list of chunks of a biguint.
*/
  ULONG chunk;
  u64 chunk_count;
  ULONG chunk_idx;
  u8 column_idx;

  chunk_count=(u64)(chunk_idx_max)+1;
  printf("%s[%08X%08X]={\n", name_base, (u32)(chunk_count>>U32_BITS), (u32)(chunk_count));
  chunk_idx=chunk_idx_max;
  column_idx=0;
  do{
    if(!column_idx){
      printf("  ");
    }
    chunk=chunk_list_base[chunk_idx];
    #ifdef _64_
      printf("%08X%08X", (u32)(chunk>>U32_BITS), (u32)(chunk));
    #else
      printf("%08X", (u32)(chunk));
    #endif
    column_idx++;
    column_idx&=(u8)(0x3F>>(ULONG_SIZE_LOG2+1));
    if(!column_idx){
      printf("\n");
    }
  }while(chunk_idx--);
  if(column_idx){
    printf("\n");
  }
  printf("}\n");
  emit_print_flush();
  return;
}

void
emit_bitmap(char *name_base, u64 bit_count, u64 bit_idx_min, ULONG *chunk_list_base){
/*
Print a bitmap starting with its least significant bit.

In:

  *name_base is a text identifier for the output.

  bit_count is the number of bits to print.

  bit_idx_min is the bit index at which to begin print.

  *chunk_list_base is a list of ULONG chunks of bits containing the bitmap.
*/
  u8 bit;
  u8 row_bit_idx;

  printf("%s[%08X%08X]={\n", name_base, (u32)(bit_count>>U32_BITS), (u32)(bit_count));
  row_bit_idx=0;
  if(bit_count){
    do{
      if(!row_bit_idx){
        printf("  ");
      }
      bit=(u8)(BIT_GET(chunk_list_base, bit_idx_min));
      bit_idx_min++;
      printf("%d", bit);
      if(row_bit_idx!=U64_BIT_MAX){
        row_bit_idx++;
      }else{
        row_bit_idx=0;
        printf("\n");
      }
      bit_count--;
    }while(bit_count);
    if(row_bit_idx){
      printf("\n");
    }
  }
  printf("}\n");
  emit_print_flush();
  return;
}

void
emit_double(char *name_base, double value){
/*
Print a double-precision floating-point value.

In:

  *name_base is a text identifier for the output.

  value is the double to print.
*/ 
  printf("%s=%+-1.15E\n", name_base, value);
  return;
}

void
emit_f128(char *name_base, u128 value){
/*
Print a 64.64 fixed-point value.

In:

  *name_base is a text identifier for the output.

  value is the 64.64 to print.
*/
  emit_name(name_base);
  #ifdef _64_
    printf("%08X%08X.%08X%08X", (u32)(value>>(U32_BITS*3)), (u32)(value>>(U32_BITS<<1)), (u32)(value>>U32_BITS), (u32)(value));
  #else
    printf("%08X%08X.%08X%08X", (u32)(value.b>>U32_BITS), (u32)(value.b), (u32)(value.a>>U32_BITS), (u32)(value.a));
  #endif
  emit_print_newline_if(name_base);
  return;
}

void
emit_f128_pair(char *name_base, u128 value0, u128 value1){
/*
Print a pair of 64.64 fixed-point values.

In:

  *name_base is a text identifier for the output.

  value0 is the first 64.64 to print.

  value1 is the second 64.64 to print.
*/
  emit_name(name_base);
  #ifdef _64_
    printf("{%08X%08X.%08X%08X, %08X%08X.%08X%08X}", (u32)(value0>>(U32_BITS*3)), (u32)(value0>>(U32_BITS<<1)), (u32)(value0>>U32_BITS), (u32)(value0), (u32)(value1>>(U32_BITS*3)), (u32)(value1>>(U32_BITS<<1)), (u32)(value1>>U32_BITS), (u32)(value1));
  #else
    printf("{%08X%08X.%08X%08X, %08X%08X.%08X%08X}", (u32)(value0.b>>U32_BITS), (u32)(value0.b), (u32)(value0.a>>U32_BITS), (u32)(value0.a), (u32)(value1.b>>U32_BITS), (u32)(value1.b), (u32)(value1.a>>U32_BITS), (u32)(value1.a));
  #endif
  emit_print_newline_if(name_base);
  return;
}

void
emit_f64(char *name_base, u64 value){
/*
Print a 32.32 fixed-point value.

In:

  *name_base is a text identifier for the output.

  value is the 32.32 to print.
*/
  emit_name(name_base);
  printf("%08X.%08X", (u32)(value>>U32_BITS), (u32)(value));
  emit_print_newline_if(name_base);
  return;
}

void
emit_f64_pair(char *name_base, u64 value0, u64 value1){
/*
Print a pair of 32.32 fixed-point values.

In:

  *name_base is a text identifier for the output.

  value0 is the first 32.32 to print.

  value1 is the second 32.32 to print.
*/
  emit_name(name_base);
  printf("{%08X.%08X, %08X.%08X}", (u32)(value0>>U32_BITS), (u32)(value0), (u32)(value1>>U32_BITS), (u32)(value1));
  emit_print_newline_if(name_base);
  return;
}

void
emit_float(char *name_base, float value){
/*
Print a single-precision floating-point value.

In:

  *name_base is a text identifier for the output.

  value is the float to print.
*/
  printf("%s=%+-1.7E\n", name_base, value);
  return;
}

void
emit_list(char *name_base, ULONG chunk_count, u8 *chunk_list_base, u8 chunk_size_log2){
/*
Print a list of chunks of 8/16/32/64/128-bit size.

In:

  *name_base is a text identifier for the output.

  chunk_count is the number of chunk to print.

  chunk_list_base is base of the list of chunks casted to (u8 *). This is bad coding practice, but it's a cheat for simplicty that seems to work.

  chunk_size_log2 is 0/1/2/3/4 for 8/16/32/64/128-bit chunks, respectively.
*/
  u8 byte;
  u8 byte_count;
  u8 char_idx;
  char char_list_base[0x20*3];
  u8 chunk_size;
  u8 column_count;
  ULONG i;
  u8 j;
  u8 nybble;
  ULONG row_count;
  ULONG u8_idx;

  if(chunk_size_log2<=U128_SIZE_LOG2){
    if(*name_base){
      #ifdef _64_
        printf("%s[%08X%08X]={\n", name_base, (u32)(chunk_count>>U32_BITS), (u32)(chunk_count));
      #else
        printf("%s[%08X%08X]={\n", name_base, 0, chunk_count);
      #endif
    }
    if(chunk_list_base){
      column_count=8;
      if(chunk_size_log2==U8_SIZE_LOG2){
        column_count=16;
      }else if(chunk_size_log2==U64_SIZE_LOG2){
        column_count=4;
      }else if(chunk_size_log2==U128_SIZE_LOG2){
        column_count=2;
      }
      row_count=(chunk_count/column_count)+!!(chunk_count%column_count);
      chunk_size=(u8)(1U<<chunk_size_log2);
      u8_idx=0;
      for(i=row_count-1; i<row_count; i--){
        if(!i){
          chunk_count%=column_count;
          if(chunk_count){
            column_count=(u8)(chunk_count);
          }
        }
        char_idx=0;
        for(j=(u8)(column_count-1); j<column_count; j--){
          byte_count=chunk_size;
          u8_idx+=chunk_size;
          do{
            u8_idx--;
            byte=chunk_list_base[u8_idx];
            nybble=(u8)(byte>>4);
            if(nybble<=9){
              nybble=(u8)(nybble+'0');
            }else{
              nybble=(u8)(nybble+'A'-0xA);
            }
            char_list_base[char_idx]=(char)(nybble);
            char_idx++;
            nybble=(u8)(byte&0xF);
            if(nybble<=9){
              nybble=(u8)(nybble+'0');
            }else{
              nybble=(u8)(nybble+'A'-0xA);
            }
            char_list_base[char_idx]=(char)(nybble);
            char_idx++;
            byte_count--;
          }while(byte_count);
          u8_idx+=chunk_size;
          if(j){
            char_list_base[char_idx]=',';
            char_idx++;
          }else{
            if(i){
              char_list_base[char_idx]=',';
              char_idx++;
            }
            char_list_base[char_idx]=0;
          }
        }
        printf("  %s\n", char_list_base);
      }
      if(*name_base){
        printf("}\n");
      }
    }
  }
  emit_print_flush();
  return;
}

void
emit_name(char *name_base){
/*
Print a field name followed by "=", or nothing if *name_base is null.

In:

  *name_base is a text identifier for the output.

  *string_base is the text to print.
*/
  if(*name_base){
    printf("%s=", name_base);
  }
  return;
}

void
emit_print(char *string_base){
/*
Print a string then flush the print buffer.

In:

  *string_base is the text to print.
*/
  printf("%s", string_base);
  emit_print_flush();
  return;
}

void
emit_print_flush(void){
/*
Flush the OS print queue in order to ensure that text display occurs before the calling program can crash.
*/
  fflush(stdout);
  return;
}

void
emit_print_newline_if(char *string_base){
/*
Print a newline character if and only if *string_base is nonnull. Flush the print buffer in either case.

In:

  *string_base is the text to test for nullness.
*/
  if(*string_base){
    printf("\n");
  }
  emit_print_flush();
  return;
}

void
emit_u128(char *name_base, u128 value){
/*
Print a u128 value.

In:

  *name_base is a text identifier for the output.

  value is the u128 to print.
*/
  emit_name(name_base);
  #ifdef _64_
    printf("%08X%08X%08X%08X", (u32)(value>>(U32_BITS*3)), (u32)(value>>(U32_BITS<<1)), (u32)(value>>U32_BITS), (u32)(value));
  #else
    printf("%08X%08X%08X%08X", (u32)(value.b>>U32_BITS), (u32)(value.b), (u32)(value.a>>U32_BITS), (u32)(value.a));
  #endif
  emit_print_newline_if(name_base);
  return;
}

void
emit_u128_pair(char *name_base, u128 value0, u128 value1){
/*
Print a pair of u128 values.

In:

  *name_base is a text identifier for the output.

  value0 is the first u128 to print.

  value1 is the second u128 to print.
*/
  emit_name(name_base);
  #ifdef _64_
     printf("{%08X%08X%08X%08X, %08X%08X%08X%08X}", (u32)(value0>>(U32_BITS*3)), (u32)(value0>>(U32_BITS<<1)), (u32)(value0>>U32_BITS), (u32)(value0), (u32)(value1>>(U32_BITS*3)), (u32)(value1>>(U32_BITS<<1)), (u32)(value1>>U32_BITS), (u32)(value1));
  #else
    printf("{%08X%08X%08X%08X, %08X%08X%08X%08X}", (u32)(value0.b>>U32_BITS), (u32)(value0.b), (u32)(value0.a>>U32_BITS), (u32)(value0.a), (u32)(value1.b>>U32_BITS), (u32)(value1.b), (u32)(value1.a>>U32_BITS), (u32)(value1.a));
  #endif
  emit_print_newline_if(name_base);
  return;
}

void
emit_u16(char *name_base, u16 value){
/*
Print a u16 value.

In:

  *name_base is a text identifier for the output.

  value is the u16 to print.
*/
  emit_name(name_base);
  printf("%04X", value);
  emit_print_newline_if(name_base);
  return;
}

void
emit_u24(char *name_base, u32 value){
/*
Print a u24 value.

In:

  *name_base is a text identifier for the output.

  value is the u32 to print as a u24.
*/
  emit_name(name_base);
  printf("%02X%04X", (u8)(value>>U16_BITS), (u16)(value));
  emit_print_newline_if(name_base);
  return;
}

void
emit_u32(char *name_base, u32 value){
/*
Print a u32 value.

In:

  *name_base is a text identifier for the output.

  value is the u32 to print.
*/
  emit_name(name_base);
  printf("%08X", value);
  emit_print_newline_if(name_base);
  return;
}

void
emit_u64(char *name_base, u64 value){
/*
Print a u64 value.

In:

  *name_base is a text identifier for the output.

  value is the u64 to print.
*/
  emit_name(name_base);
  printf("%08X%08X", (u32)(value>>U32_BITS), (u32)(value));
  emit_print_newline_if(name_base);
  return;
}

void
emit_u64_decimal(char *name_base, u64 value){
/*
Print a u64 value.

In:

  *name_base is a text identifier for the output.

  value is the u64 to print.
*/
  unsigned char decimal_base[24];
  u8 digit;
  u8 digit_count;
  u8 digit_idx;

  emit_name(name_base);
  decimal_base[0]='0';
  digit_count=1;
  if(value){
    digit_idx=23;
    while(value){
      digit=(u8)(value%10);
      digit=(u8)(digit+'0');
      decimal_base[digit_idx]=digit;
      digit_idx--;
      value/=10;
    }
    digit_count=(u8)(23-digit_idx);
    digit_idx++;
    memmove(decimal_base, &decimal_base[digit_idx], (size_t)(digit_count));
  }
  decimal_base[digit_count]=0;
  printf("%s", decimal_base);
  emit_print_newline_if(name_base);
  return;
}

void
emit_u64_pair(char *name_base, u64 value0, u64 value1){
/*
Print a pair of u64 values.

In:

  *name_base is a text identifier for the output.

  value0 is the first u64 to print.

  value1 is the second u64 to print.
*/
  emit_name(name_base);
  printf("{%08X%08X, %08X%08X}", (u32)(value0>>U32_BITS), (u32)(value0), (u32)(value1>>U32_BITS), (u32)(value1));
  emit_print_newline_if(name_base);
  return;
}

void
emit_u8(char *name_base, u8 value){
/*
Print a u8 value.

In:

  *name_base is a text identifier for the output.

  value is the u8 to print.
*/
  emit_name(name_base);
  printf("%02X", value);
  emit_print_newline_if(name_base);
  return;
}

void
emit_write(char *string_base){
/*
Print a string followed by a newline then flush the print buffer.

In:

  *string_base is the text to print.
*/
  printf("%s\n", string_base);
  emit_print_flush();
  return;
}
