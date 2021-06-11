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
Multispectral Entropy Transform Utility
*/
#include "flag.h"
#include "flag_ascii.h"
#include "flag_filesys.h"
#include "flag_fracterval_u128.h"
#include "flag_fracterval_u64.h"
#include "flag_transform.h"
#include "flag_whole.h"
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "emit.h"
#include "emit_xtrn.h"
#include "ascii_xtrn.h"
#include "filesys.h"
#include "filesys_xtrn.h"
#include "fracterval_u128.h"
#include "fracterval_u128_xtrn.h"
#include "fracterval_u64.h"
#include "fracterval_u64_xtrn.h"
#include "transform.h"
#include "transform_xtrn.h"
#include "whole_xtrn.h"

#define SKAN_FLAGS_ALGO_LSB 6U
#define SKAN_FLAGS_ALGO_MASK 3U
#define SKAN_FLAGS_DISCOUNT_LSB 4U
#define SKAN_FLAGS_DISCOUNT_MASK 1U
#define SKAN_FLAGS_HEADER_LSB 1U
#define SKAN_FLAGS_HEADER_MASK 1U
#define SKAN_FLAGS_OPTIMIZE_LSB 5U
#define SKAN_FLAGS_OPTIMIZE_MASK 1U
#define SKAN_FLAGS_PRECISE_LSB 0U
#define SKAN_FLAGS_PRECISE_MASK 1U
#define SKAN_FLAGS_ROUNDING_LSB 2U
#define SKAN_FLAGS_ROUNDING_MASK 3U

void
skan_comma_print(void){
  EMIT_PRINT(",");
  return;
}

void
skan_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("ERROR: ");
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

void
skan_out_of_memory_print(u8 emit_mode){
  skan_error_print(emit_mode, "Out of memory");
  return;
}

void
skan_parameter_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("Invalid parameter: (");
    EMIT_PRINT(text_base);
    EMIT_WRITE("). For help, run without parameters.");
  }
  return;
}

void
skan_progress_print(u8 emit_mode, char *text_base){
  if(EMIT3<=emit_mode){
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

void
skan_too_big_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("(");
    EMIT_PRINT(text_base);
    EMIT_PRINT(") too big");
    #ifndef _64_
      EMIT_PRINT(". If you think the parameter was valid, then try the 64-bit version of this utility");
    #endif
    EMIT_WRITE(".");
  }
  return;
}

void
skan_warning_print(u8 emit_mode, char *text_base){
  if(EMIT2<=emit_mode){
    EMIT_PRINT("WARNING: ");
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

int
main(int argc, char *argv[]){
  u8 algo_status;
  ULONG arg_idx;
  u8 discount_status;
  u8 emit_mode;
  entropy_t *entropy_base;
  ULONG fakefloat_list_size;
  u32 *fakefloat_list_base;
  u8 filesys_status;
  u8 flags;
  u8 float_file_status;
  char *float_pathname_base;
  u8 granularity;
  u8 header_status;
  u8 optimize_status;
  u8 overflow_status;
  u64 parameter;
  char *parameter_text_base;
  u8 precise_status;
  u8 rounding_status;
  u8 status;
  transform_t *transform_base;
  ULONG whole_file_size;
  ULONG whole_idx_max;
  u32 whole_max;
  char *whole_pathname_base;
  u8 whole_size;
  ULONG whole_u8_idx_post;
  u8 *whole_u8_list_base;
  ULONG whole_x_idx_max;
  ULONG whole_x_idx_min;
  ULONG whole_x_idx_post;
  ULONG whole_y_idx_max;
  ULONG whole_y_idx_min;
  ULONG whole_y_idx_post;
  u8 window_status;
  ULONG window_x_idx_max;
  ULONG window_x_idx_post;
  ULONG window_y_idx_max;
  ULONG window_y_idx_post;

  status=ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 5));
  status=(u8)(status|transform_init(TRANSFORM_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|whole_init(WHOLE_BUILD_BREAK_COUNT_EXPECTED, 0));
  emit_mode=EMIT3;
  entropy_base=NULL;
  overflow_status=0;
  fakefloat_list_base=NULL;
  transform_base=NULL;
  whole_u8_list_base=NULL;
  do{
    if(status){
      skan_error_print(emit_mode, "Outdated source code");
      break;
    }
    status=1;
    if((argc!=9)&&(argc!=11)){
      EMIT_WRITE("Skan\nCopyright 2021 Russell Leidich\nhttps://github.com/egione/Widebandit");
      EMIT_WRITE("Build 1");
      EMIT_WRITE("Compute a 2D rolling window entropy transform of discretized power amplitudes");
      EMIT_WRITE("(whole numbers) across frequencies (columns) and spectra (rows).\n");
      EMIT_WRITE("Syntax:\n");
      EMIT_WRITE("  skan verbosity whole_size row_width whole_file float_file flags window_width");
      EMIT_WRITE("  window_height [x_min y_min]\n");
      EMIT_WRITE("where all values are decimal unless otherwise stated and:\n");
      EMIT_WRITE("  (verbosity) is one of:\n");
      EMIT_WRITE("    0: Report only errors.\n");
      EMIT_WRITE("    1: Report errors and warnings.\n");
      EMIT_WRITE("    2: Report errors, warnings, and progress information.\n");
      EMIT_WRITE("  (whole_size) is the number of bytes per whole in (whole_file), up to 4.\n");
      EMIT_WRITE("  (row_width) is the number of wholes per spectrum.\n");
      EMIT_WRITE("  (whole_file) is the name of the file containing spectra of (row_width) wholes");
      EMIT_WRITE("  of size (whole_size).\n");
      EMIT_WRITE("  (float_file) is the file to which to dump 32-bit floating-point entropy values");
      EMIT_WRITE("  encoded as discount nats below maximum window entropy. Enter \"-\" to save");
      EMIT_WRITE("  time by not writing this file.\n");
      EMIT_WRITE("  (flags) is a hex value which contains various control bits:\n");
      EMIT_WRITE("    [0](precise) tells internal fracterval (fractional interval) precision.\n");
      EMIT_WRITE("      0: Use 64-bit fractervals.\n");
      EMIT_WRITE("      1: Use 128-bit fractervals.\n");
      EMIT_WRITE("    [1](header) tells whether or not to print CSV column headers.\n");
      EMIT_WRITE("      0: Display column headers for output CSV (spreadsheet).\n");
      EMIT_WRITE("      1: Display only rows to append to an existing CSV (via \">>\" in UNIX).\n");
      EMIT_WRITE("    [2-3](rounding) tells how to round fractervals to IEEE754 32-bit floats,");
      EMIT_WRITE("    which only applies if (float_file) is not \"-\":\n");
      EMIT_WRITE("      00: Round nearest-or-even.\n");
      EMIT_WRITE("      01: Round toward negative infinity.\n");
      EMIT_WRITE("      10: Round toward positive infinity.\n");
      EMIT_WRITE("    [4](discount) tells how to display window information content.\n");
      EMIT_WRITE("      0: Display raw window nats.\n");
      EMIT_WRITE("      1: Display window nats as discount from theoretical maximum.\n");
      EMIT_WRITE("    [5](optimize) tells how to optimize the transform:\n");
      EMIT_WRITE("      0: More accurate, less memory, and slow.\n");
      EMIT_WRITE("      1: Less accurate, more memory, and fast.\n");
      EMIT_WRITE("    [6-7](algo) tells which entropy method to use:\n");
      EMIT_WRITE("      00: Shannon (fast, least sensitive).\n");
      EMIT_WRITE("      01: Agnentropy (faster, more sensitive).\n");
      EMIT_WRITE("      10: Logfreedom (slow, most sensitive).\n");
      EMIT_WRITE("  (window_width) is the number of columns in the rolling window.\n");
      EMIT_WRITE("  (window_height) is the number of rows in the rolling window. 0 for all rows.\n");
      EMIT_WRITE("  (x_min) is optional. If specified, it is the zero-based index of the leftmost");
      EMIT_WRITE("  column of the window in which to compute entropy. This is mostly useful for");
      EMIT_WRITE("  verifying the output of a previous full scan, or obtaining better precision."); 
      EMIT_WRITE("  (float_file) must be \"-\".\n");
      EMIT_WRITE("  (y_min) must be specified if and only if (x_min) is specified. It is the");
      EMIT_WRITE("  corresponding zero-based row number.\n");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        skan_error_print(emit_mode, "One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    window_status=(argc==11);
    parameter_text_base=argv[1];
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, EMIT2);
    if(status){
      skan_parameter_error_print(emit_mode, "verbosity");
      break;
    }
    emit_mode=(u8)(parameter);
/*
Increment emit_mode because we provide 3 levels, whereas emit.h provides 4. The difference is that we don't have any need for priority zero (critical) messages.
*/
    emit_mode++;
    parameter_text_base=argv[2];
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, U32_SIZE);
    status=(u8)(status|!parameter);
    if(status){
      skan_parameter_error_print(emit_mode, "whole_size");
      break;
    }
    parameter_text_base=argv[3];
    whole_size=(u8)(parameter);
    granularity=(u8)(whole_size-1);
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, ULONG_MAX);
    status=(u8)(status|!parameter);
    if(status){
      skan_parameter_error_print(emit_mode, "row_width");
      break;
    }
    parameter_text_base=argv[6];
    whole_x_idx_post=(ULONG)(parameter);
    status=ascii_hex_to_u64_convert(parameter_text_base, &parameter, (SKAN_FLAGS_ALGO_MASK<<SKAN_FLAGS_ALGO_LSB)+(SKAN_FLAGS_DISCOUNT_MASK<<SKAN_FLAGS_DISCOUNT_LSB)+(SKAN_FLAGS_HEADER_MASK<<SKAN_FLAGS_HEADER_LSB)+(SKAN_FLAGS_OPTIMIZE_MASK<<SKAN_FLAGS_OPTIMIZE_LSB)+(SKAN_FLAGS_PRECISE_MASK<<SKAN_FLAGS_PRECISE_LSB)+(SKAN_FLAGS_ROUNDING_MASK<<SKAN_FLAGS_ROUNDING_LSB));
    if(status){
      skan_parameter_error_print(emit_mode, "flags");
      break;
    }
    flags=(u8)(parameter);
    algo_status=(u8)(flags>>SKAN_FLAGS_ALGO_LSB)&SKAN_FLAGS_ALGO_MASK;
    discount_status=(u8)(flags>>SKAN_FLAGS_DISCOUNT_LSB)&SKAN_FLAGS_DISCOUNT_MASK;
    header_status=(u8)(flags>>SKAN_FLAGS_HEADER_LSB)&SKAN_FLAGS_HEADER_MASK;
    optimize_status=(u8)(flags>>SKAN_FLAGS_OPTIMIZE_LSB)&SKAN_FLAGS_OPTIMIZE_MASK;
    precise_status=(u8)(flags>>SKAN_FLAGS_PRECISE_LSB)&SKAN_FLAGS_PRECISE_MASK;
    rounding_status=(u8)(flags>>SKAN_FLAGS_ROUNDING_LSB)&SKAN_FLAGS_ROUNDING_MASK;
    status=1;
    if(TRANSFORM_ALGO_STATUS_MAX<algo_status){
      skan_parameter_error_print(emit_mode, "flags.algo");
      break;
    }
    if(TRANSFORM_ROUNDING_STATUS_MAX<rounding_status){
      skan_parameter_error_print(emit_mode, "flags.rounding");
      break;
    }
    parameter_text_base=argv[7];
    flags=(u8)(parameter);
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, ULONG_MAX);
    status=(u8)(status|!parameter);
    if(status){
      skan_parameter_error_print(emit_mode, "window_width");
      break;
    }
    parameter_text_base=argv[8];
    window_x_idx_post=(ULONG)(parameter);
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, ULONG_MAX);
    status=(u8)(status|(parameter<=1));
    if(status){
      skan_parameter_error_print(emit_mode, "window_height");
      break;
    }
    whole_x_idx_min=0;
    whole_y_idx_min=0;
    window_y_idx_post=(ULONG)(parameter);
    if(window_status){
      parameter_text_base=argv[9];
      status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, ULONG_MAX);
      if(status){
        skan_parameter_error_print(emit_mode, "x_min");
        break;
      }
      status=1;
      parameter_text_base=argv[10];
      whole_x_idx_min=(ULONG)(parameter);
      status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, ULONG_MAX);
      if(status){
        skan_parameter_error_print(emit_mode, "y_min");
        break;
      }
      whole_y_idx_min=(ULONG)(parameter);
    }
    status=1;
    whole_pathname_base=argv[4];
    filesys_status=filesys_file_size_ulong_get(&whole_file_size, whole_pathname_base);
    if(filesys_status){
      if(filesys_status==FILESYS_STATUS_TOO_BIG){
        skan_too_big_print(emit_mode, "whole_file");
      }else{
        skan_error_print(emit_mode, "(whole_file) not found");
      }
      break;
    }
    whole_u8_idx_post=whole_x_idx_post*whole_size;
    if((whole_u8_idx_post/whole_size)!=whole_x_idx_post){
      skan_too_big_print(emit_mode, "row_width");
      break;
    }
    whole_y_idx_post=whole_file_size/whole_u8_idx_post;
    if(!window_y_idx_post){
      window_y_idx_post=whole_y_idx_post;
    }
    if(whole_y_idx_post==1){
      skan_error_print(emit_mode, "(window_height) is 1. It must be at least 2.");
      break;
    }
    if((!whole_file_size)||(whole_file_size%whole_u8_idx_post)){
      skan_error_print(emit_mode, "(whole_file) size must be a nonzero multiple of ((wholes_per_row)*(whole_size))");
      break;
    }
    if(whole_x_idx_post<window_x_idx_post){
      skan_too_big_print(emit_mode, "window_width");
      break;
    }
    if(whole_y_idx_post<window_y_idx_post){
      skan_too_big_print(emit_mode, "window_height");
      break;
    }
    if((whole_x_idx_post-window_x_idx_post)<whole_x_idx_min){
      skan_too_big_print(emit_mode, "whole_x_idx_min");
      break;
    }
    if((whole_y_idx_post-window_y_idx_post)<whole_y_idx_min){
      skan_too_big_print(emit_mode, "whole_y_idx_min");
      break;
    }
    float_pathname_base=argv[5];
    float_file_status=(float_pathname_base[0]&&((float_pathname_base[0]!='-')||float_pathname_base[1]));
    if(EMIT3<=emit_mode){
      EMIT_U64_DECIMAL("flags.algo", algo_status);
      EMIT_U64_DECIMAL("flags.discount", discount_status);
      EMIT_U64_DECIMAL("flags.header", header_status);
      EMIT_U64_DECIMAL("flags.optimize", optimize_status);
      EMIT_U64_DECIMAL("flags.precise", precise_status);
      EMIT_U64_DECIMAL("flags.rounding", rounding_status);
      if(float_file_status){
        EMIT_PRINT("float_file=");
        EMIT_WRITE(float_pathname_base);
      }
      EMIT_U64_DECIMAL("row_count", whole_y_idx_post);
      EMIT_U64_DECIMAL("row_width", whole_x_idx_post);
      EMIT_PRINT("whole_file=");
      EMIT_WRITE(whole_pathname_base);
      EMIT_U64_DECIMAL("whole_size", whole_size);
      EMIT_U64_DECIMAL("window_height", window_y_idx_post);
      EMIT_U64_DECIMAL("window_width", window_x_idx_post);
      if(window_status){
        EMIT_U64_DECIMAL("x_min", whole_x_idx_min);
        EMIT_U64_DECIMAL("y_min", whole_y_idx_min);
      }
    }
    whole_idx_max=whole_file_size/whole_size-1;
    whole_u8_list_base=whole_list_malloc(granularity, whole_idx_max);
    if(!whole_u8_list_base){
      skan_out_of_memory_print(emit_mode);
      break;
    }
    fakefloat_list_size=0;
    whole_x_idx_max=whole_x_idx_post-1;
    whole_y_idx_max=whole_y_idx_post-1;
    window_x_idx_max=window_x_idx_post-1;
    window_y_idx_max=window_y_idx_post-1;
    if(float_file_status){
      if(window_status){
        skan_error_print(emit_mode, "(float_file) must be \"-\" when (x_min) is specified. (x_min) and (y_min) are mainly just intended for verification. Setting (flags.precise) is the best place to ensure that (float_file) is accurate");
        break;
      }
      filesys_status=filesys_file_write_obnoxious(0, 0, float_pathname_base, float_pathname_base);
      if(filesys_status){
        skan_error_print(emit_mode, "Could not create (float_file), perhaps due to a bad path");
        break;
      }
      fakefloat_list_base=transform_fakefloat_list_malloc(&fakefloat_list_size, whole_x_idx_max, whole_y_idx_max, window_x_idx_max, window_y_idx_max);
      if(!fakefloat_list_base){
        skan_out_of_memory_print(emit_mode);
        break;
      }
    }
    filesys_status=filesys_file_read_exact(whole_file_size, whole_pathname_base, whole_u8_list_base);
    if((filesys_status==FILESYS_STATUS_SIZE_CHANGED)||(filesys_status==FILESYS_STATUS_TOO_BIG)){
      skan_error_print(emit_mode, "(whole_file) changed during execution");
      break;
    }else if(filesys_status){
      skan_error_print(emit_mode, "Could not read (whole_file), perhaps due to a bad path");
      break;
    }
    whole_max=whole_max_get(granularity, whole_idx_max, whole_u8_list_base);
    if(window_status){
      optimize_status=0;
    }
    transform_malloc(algo_status, discount_status, &entropy_base, fakefloat_list_base, granularity, optimize_status, precise_status, rounding_status, &transform_base, whole_max, whole_x_idx_max, whole_y_idx_max, window_x_idx_max, window_y_idx_max);
    if(!entropy_base){
      skan_out_of_memory_print(emit_mode);
      break;
    }
    if(!window_status){
      overflow_status=transform_dispatch(entropy_base, transform_base, whole_u8_list_base);
    }else{
      if(!precise_status){
        overflow_status=transform_whole_list_window_entropy_get_fru64(entropy_base, transform_base, whole_u8_list_base, whole_x_idx_min, whole_y_idx_min);
      }else{
        overflow_status=transform_whole_list_window_entropy_get_fru128(entropy_base, transform_base, whole_u8_list_base, whole_x_idx_min, whole_y_idx_min);
      }
    }
    if(float_file_status){
      filesys_status=filesys_file_write_obnoxious(0, fakefloat_list_size, float_pathname_base, fakefloat_list_base);
      if(filesys_status){
        skan_error_print(emit_mode, "Could not write (float_file)");
        break;
      }
    }
    if(header_status){
      if(discount_status){
        EMIT_PRINT("DISCOUNT_NATS_MIN,DISCOUNT_");
      }else{
        EMIT_PRINT("NATS_MIN,");
      }
      EMIT_WRITE("NATS_MAX,X_SPAN,Y_SPAN,MIN_X_MIN,MIN_Y_MIN,MAX_X_MIN,MAX_Y_MIN");
    }
    if(!precise_status){
      EMIT_F64("", entropy_base->min_min_u64);
      skan_comma_print();
      EMIT_F64("", entropy_base->max_max_u64);
    }else{
      EMIT_F128("", entropy_base->min_min_u128);
      skan_comma_print();
      EMIT_F128("", entropy_base->max_max_u128);
    }
    skan_comma_print();
    EMIT_U64_DECIMAL("", window_x_idx_post);
    skan_comma_print();
    EMIT_U64_DECIMAL("", window_y_idx_post);
    skan_comma_print();
    EMIT_U64_DECIMAL("", entropy_base->min_min_x_idx);
    skan_comma_print();
    EMIT_U64_DECIMAL("", entropy_base->min_min_y_idx);
    skan_comma_print();
    EMIT_U64_DECIMAL("", entropy_base->max_max_x_idx);
    skan_comma_print();
    EMIT_U64_DECIMAL("", entropy_base->max_max_y_idx);
    EMIT_WRITE("");
    skan_progress_print(emit_mode, "Done");
    status=0;
  }while(0);
  if(overflow_status){
    if(!precise_status){
      skan_warning_print(emit_mode, "Fracterval saturation occurred, which is probably inconsequential. Try 128-bit precision");
    }else{
      skan_warning_print(emit_mode, "Fracterval saturation occurred, which is probably inconsequential");
    }
  }
  transform_free_all(&entropy_base, &transform_base);
  transform_free(fakefloat_list_base);
  whole_free(whole_u8_list_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
