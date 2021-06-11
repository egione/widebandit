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
Spectrum Discretizer
*/
#include "flag.h"
#include "flag_archive.h"
#include "flag_ascii.h"
#include "flag_fakefloat.h"
#include "flag_filesys.h"
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
#include "archive.h"
#include "archive_xtrn.h"
#include "fakefloat.h"
#include "fakefloat_xtrn.h"
#include "whole_xtrn.h"

void
slice_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("ERROR: ");
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

void
slice_out_of_memory_print(u8 emit_mode){
  slice_error_print(emit_mode, "Out of memory");
  return;
}

void
slice_parameter_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("Invalid parameter: (");
    EMIT_PRINT(text_base);
    EMIT_WRITE("). For help, run without parameters.");
  }
  return;
}

void
slice_progress_print(u8 emit_mode, char *text_base){
  if(EMIT3<=emit_mode){
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

void
slice_value_report(u8 emit_mode, u8 emit_priority, char *text_base, u64 value){
  if(emit_priority<=emit_mode){
    if(emit_priority==EMIT1){
      EMIT_PRINT("ERROR: ");
    }else if(emit_priority==EMIT2){
      EMIT_PRINT("WARNING: ");
    }
    EMIT_PRINT(text_base);
    EMIT_PRINT(" ");
    EMIT_U64_DECIMAL("", value);
    EMIT_WRITE(".");
  }
  return;
}

void
slice_warning_print(u8 emit_mode, char *text_base){
  if(EMIT2<=emit_mode){
    EMIT_PRINT("WARNING: ");
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

int
main(int argc, char *argv[]){
  u8 append_status;
  u64 archive_size_u64;
  ULONG archive_u32_idx_max;
  ULONG archive_u32_idx_min;
  u32 *archive_u32_list_base;
  ULONG arg_idx;
  int descriptor;
  u8 emit_mode;
  u8 filesys_status;
  u32 float_0;
  ULONG float_count;
  ULONG float_file_size;
  ULONG float_idx;
  ULONG float_idx_max;
  u32 *float_list_base;
  char *float_pathname_base;
  u8 granularity;
  header_t *header_base;
  char *index_pathname_base;
  u64 lmd2_iterand;
  u64 lmd2_partial_sum;
  u64 parameter;
  char *parameter_text_base;
  ULONG pdf_count;
  ULONG pdf_count_copy;
  ULONG pdf_float_count;
  ULONG pdf_slot_count;
  ULONG row_idx;
  ULONG row_count;
  ULONG row_size;
  ULONG row_u8_idx;
  ULONG row_u8_idx_post;
  u32 slice_count;
  u32 slice_idx;
  u8 status;
  ULONG whole_file_size;
  char *whole_pathname_base;
  u8 whole_size;
  ULONG whole_u8_idx;
  u8 *whole_u8_list_base;
  ULONG whole_u8_list_size;

  status=archive_init(ARCHIVE_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|fakefloat_init(FAKEFLOAT_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 5));
  emit_mode=EMIT3;
  header_base=NULL;
  float_list_base=NULL;
  whole_u8_list_base=NULL;
  do{
    if(status){
      slice_error_print(emit_mode, "Outdated source code");
      break;
    }
    status=1;
    if((argc!=5)&&(argc!=6)){
      EMIT_WRITE("Slice\nCopyright 2021 Russell Leidich\nhttps://github.com/egione/Widebandit");
      EMIT_WRITE("Build 1");
      EMIT_WRITE("Convert floats into wholes.\n");
      EMIT_WRITE("Syntax:\n");
      EMIT_WRITE("  slice verbosity index_file float_file whole_file [row_index]\n");
      EMIT_WRITE("where:\n");
      EMIT_WRITE("  (verbosity) is one of:\n");
      EMIT_WRITE("    0: Report only errors.\n");
      EMIT_WRITE("    1: Report errors and warnings.\n");
      EMIT_WRITE("    2: Report errors, warnings, and progress information.\n");
      EMIT_WRITE("  (index_file) is a Spectrafy archive which has been optimized for lookup.\n");
      EMIT_WRITE("  (float_file) is the name of a file containing a list of 32-bit IEEE754");
      EMIT_WRITE("  floating-point values (\"floats\"). All numeric types other than NaN are");
      EMIT_WRITE("  acceptable.\n");
      EMIT_WRITE("  (whole_file) is the name of the file to be appended with a list of whole");
      EMIT_WRITE("  numbers. If each PDF in (index_file) consists of N floats, then slice indexes");
      EMIT_WRITE("  may vary from zero through N, so each whole number will consist of the");
      EMIT_WRITE("  minimum possible number of bytes sufficient to represent N.\n");
      EMIT_WRITE("  (row_index), if present, is the zero-based row number to overwrite with the");
      EMIT_WRITE("  newly computed row(s) of wholes. (whole_file) must have sufficient size such");
      EMIT_WRITE("  that no data would need to be appended. If (row_index) is unspecified, then");
      EMIT_WRITE("  the row(s) will be appended.");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        slice_error_print(emit_mode, "One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    parameter_text_base=argv[1];
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, EMIT2);
    if(status){
      slice_parameter_error_print(emit_mode, "verbosity");
      break;
    }
    status=1;
    emit_mode=(u8)(parameter);
/*
Increment emit_mode because we provide 3 levels, whereas emit.h provides 4. The difference is that we don't have any need for priority zero (critical) messages.
*/
    emit_mode++;
    append_status=1;
    row_idx=0;
    row_u8_idx=0;
    if(argc==6){
      append_status=0;
      parameter_text_base=argv[5];
      status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, ULONG_MAX);
      if(status){
        slice_parameter_error_print(emit_mode, "row_idx");
        break;
      }
      status=1;
      row_idx=(ULONG)(parameter);
    }
    index_pathname_base=argv[2];
    filesys_status=filesys_file_mem_map(&descriptor, &archive_size_u64, index_pathname_base, (void **)(&archive_u32_list_base), 0);
    if(filesys_status){
      if(filesys_status==FILESYS_STATUS_MEM_MAP_FAIL){
        slice_error_print(emit_mode, "Could not make a readonly memory map for (index_file)");
      }else{
        slice_error_print(emit_mode, "Could not open (index_file) for reading");
      }
      break;
    }
    header_base=archive_header_init();
    if(!header_base){
      slice_out_of_memory_print(emit_mode);
      break;
    }
    archive_header_import(header_base, archive_u32_list_base);
    status=archive_header_check(archive_size_u64, header_base, &lmd2_iterand, 0, &lmd2_partial_sum, &pdf_count, &pdf_float_count, &pdf_slot_count);
    if(status){
      slice_error_print(emit_mode, "(index_file) header is corrupt");
      break;
    }
    status=1;
    slice_value_report(emit_mode, EMIT3, "Floats per PDF is", pdf_float_count);
    slice_count=(u32)(pdf_float_count+1);
    if(slice_count!=(pdf_float_count+1)){
      slice_error_print(emit_mode, "The number of floats per PDF in (index_file) exceeds (2^32-1). Downsample can fix this");
      break;
    }
    slice_value_report(emit_mode, EMIT3, "Slice count is", slice_count);
    float_pathname_base=argv[3];
    filesys_status=filesys_file_size_ulong_get(&float_file_size, float_pathname_base);
    if(filesys_status){
      if(filesys_status==FILESYS_STATUS_TOO_BIG){
        slice_error_print(emit_mode, "(float_file) too big. You need the 64-bit version of this utility");
      }else{
        slice_error_print(emit_mode, "(float_file) not found");
      }
      break;
    }
    if((float_file_size&U32_BYTE_MAX)||!float_file_size){
      slice_error_print(emit_mode, "(float_file) size must be a nonzero multiple of 4");
      break;
    }
    float_count=float_file_size>>U32_SIZE_LOG2;
    row_count=float_count/pdf_count;
    slice_value_report(emit_mode, EMIT3, "PDF count is", pdf_count);
    slice_value_report(emit_mode, EMIT3, "Float count is", float_count);
    if(float_count%pdf_count){
      slice_error_print(emit_mode, "The number of floats in (float_file) must be a multiple of the number of PDFs in (index_file)");
      break;
    }
    float_idx_max=float_count-1;
    float_list_base=fakefloat_u32_list_malloc(0, float_idx_max);
    filesys_status=filesys_file_read_exact(float_file_size, float_pathname_base, float_list_base);
    if(filesys_status){
      if(filesys_status==FILESYS_STATUS_NOT_FOUND){
        slice_error_print(emit_mode, "(float_file) disappeared during execution");
      }else{
        slice_error_print(emit_mode, "(float_file) changed size during execution");
      }
      break;
    }
    granularity=U8_BYTE_MAX;
    if(pdf_float_count>>U8_BITS){
      granularity=U16_BYTE_MAX;
      if(pdf_float_count>>U16_BITS){
        granularity=U24_BYTE_MAX;
        if(pdf_float_count>>U24_BITS){
          granularity=U32_BYTE_MAX;
          #ifdef _64_
            if(pdf_float_count>>U32_BITS){
              slice_error_print(emit_mode, "(index_file) contains too many floats per PDF, so slice indexes can't fit into 32 bits");
              break;
            }
          #endif
        }
      }
    }
    whole_u8_list_base=whole_list_malloc(granularity, float_idx_max);
    if(!whole_u8_list_base){
      slice_out_of_memory_print(emit_mode);
      break;
    }
    whole_size=(u8)(granularity+1);
    slice_value_report(emit_mode, EMIT3, "Bytes per whole is", whole_size);
    whole_u8_list_size=float_count*whole_size;
    row_size=pdf_count*whole_size;
    row_u8_idx=0;
    whole_pathname_base=argv[4];
    if(!append_status){
      filesys_status=filesys_file_size_ulong_get(&whole_file_size, whole_pathname_base);
      if(filesys_status==FILESYS_STATUS_NOT_FOUND){
        slice_error_print(emit_mode, "(whole_file) not found");
        break;
      }else if(filesys_status){
        slice_error_print(emit_mode, "(whole_file) too big to handle");
        break;
      }
      if(whole_file_size%row_size){
        slice_value_report(emit_mode, EMIT3, "Size of (whole_file) is", whole_file_size);
        slice_value_report(emit_mode, EMIT3, "Size of each row, given (whole_size), is", row_size);
        slice_error_print(emit_mode, "(whole_file) size is not a multiple of the implied row size");
        break;
      }
      row_u8_idx=row_idx*row_size;
      row_u8_idx_post=row_u8_idx+whole_u8_list_size;
      if((row_u8_idx_post<row_u8_idx)||(whole_file_size<row_u8_idx_post)||((row_u8_idx/row_size)!=row_idx)){
        slice_error_print(emit_mode, "(row_idx) refers to a row past the end of (whole_file). Leave it unspecified for append mode.");
        break;
      }
    }
    float_idx=0;
    whole_u8_idx=0;
    do{
      archive_u32_idx_max=pdf_float_count+ARCHIVE_HEADER_U32_COUNT-1;
      archive_u32_idx_min=ARCHIVE_HEADER_U32_COUNT;
      pdf_count_copy=pdf_count;
      do{
        float_0=float_list_base[float_idx];
        slice_idx=fakefloat_to_slice_idx(archive_u32_idx_max, archive_u32_idx_min, float_0, archive_u32_list_base);
        whole_u8_list_base[whole_u8_idx]=(u8)(slice_idx);
        whole_u8_idx++;
        if(granularity){
          whole_u8_list_base[whole_u8_idx]=(u8)(slice_idx>>U8_BITS);
          whole_u8_idx++;
          if(U16_BYTE_MAX<granularity){
            whole_u8_list_base[whole_u8_idx]=(u8)(slice_idx>>U16_BITS);
            whole_u8_idx++;
            if(U24_BYTE_MAX<granularity){
              whole_u8_list_base[whole_u8_idx]=(u8)(slice_idx>>U24_BITS);
              whole_u8_idx++;
            }
          }
        }
        archive_u32_idx_max+=pdf_slot_count;
        archive_u32_idx_min+=pdf_slot_count;
        float_idx++;
      }while(--pdf_count_copy);
    }while(--row_count);
    filesys_status=filesys_file_mem_unmap(descriptor, archive_size_u64, archive_u32_list_base);
    if(filesys_status){
      slice_error_print(emit_mode, "Could not close memory map for (index_file)");
      break;
    }
    if(!append_status){
      filesys_status=filesys_subfile_write(0, whole_pathname_base, whole_u8_idx, row_u8_idx, whole_u8_list_base);
      if(filesys_status){
        if(filesys_status==FILESYS_STATUS_NOT_FOUND){
          slice_error_print(emit_mode, "(whole_file) disappeared during execution");
          break;
        }else{
          slice_error_print(emit_mode, "Cannot update (whole_file)");
          break;
        }
      }
    }else{
      filesys_status=filesys_file_write_obnoxious(1, whole_u8_idx, whole_pathname_base, whole_u8_list_base);
      if(filesys_status){
        slice_error_print(emit_mode, "Cannot append to (whole_file)");
        break;
      }
    }
    slice_progress_print(emit_mode, "Done");
    status=0;
  }while(0);
  whole_free(whole_u8_list_base);
  fakefloat_free(float_list_base);
  archive_free(header_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
