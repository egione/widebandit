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
Spectrafy Archive Downsampler
*/
#include "flag.h"
#include "flag_archive.h"
#include "flag_ascii.h"
#include "flag_filesys.h"
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

void downsample_error_print(u8 emit_mode, char *text_base);

void
downsample_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("ERROR: ");
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

void
downsample_out_of_memory_print(u8 emit_mode){
  downsample_error_print(emit_mode, "Out of memory");
  return;
}

void
downsample_parameter_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("Invalid parameter: (");
    EMIT_PRINT(text_base);
    EMIT_WRITE("). For help, run without parameters.");
  }
  return;
}

void
downsample_progress_print(u8 emit_mode, char *text_base){
  if(EMIT3<=emit_mode){
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

void
downsample_warning_print(u8 emit_mode, char *text_base){
  if(EMIT2<=emit_mode){
    EMIT_PRINT("WARNING: ");
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

int
main(int argc, char *argv[]){
  ULONG archive_size;
  u64 archive_size_u64;
  u32 *archive_u32_list_base;
  ULONG arg_idx;
  int descriptor;
  u8 emit_mode;
  u8 filesys_status;
  header_t *header_base;
  ULONG list_size;
  u64 lmd2_iterand;
  u64 lmd2_partial_sum;
  char *out_pathname_base;
  u64 parameter;
  char *parameter_text_base;
  ULONG pdf_count;
  ULONG pdf_float_count;
  ULONG pdf_slot_count;
  char *sfy_pathname_base;
  u8 sort_status;
  u8 status;
  ULONG u32_count_idx;
  ULONG u32_count_idx_max;
  ULONG *u32_count_list_base;
  u32 *u32_list_base;
  u32 **u32_list_base_list_base;

  status=archive_init(ARCHIVE_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 5));
  archive_u32_list_base=NULL;
  emit_mode=EMIT3;
  header_base=NULL;
  u32_count_idx_max=0;
  u32_count_list_base=NULL;
  u32_list_base_list_base=NULL;
  do{
    if(status){
      downsample_error_print(emit_mode, "Outdated source code");
      break;
    }
    status=1;
    if((argc<5)||!(argc&1)){
      EMIT_WRITE("Downsample\nCopyright 2021 Russell Leidich\nhttps://github.com/egione/Widebandit");
      EMIT_WRITE("Build 1");
      EMIT_WRITE("Samples probability distribution functions at roughly regular intervals.");
      EMIT_WRITE("Syntax:\n");
      EMIT_WRITE("  downsample verbosity sfy_file index_file0 width0 [index_file1 width1]...\n");
      EMIT_WRITE("where:\n");
      EMIT_WRITE("  (verbosity) is one of:\n");
      EMIT_WRITE("    0: Report only errors.\n");
      EMIT_WRITE("    1: Report errors and warnings.\n");
      EMIT_WRITE("    2: Report errors, warnings, and progress information.\n");
      EMIT_WRITE("  (sfy_file) is an archive for use with Spectrafy.\n");
      EMIT_WRITE("  (index_file0) will be a downsampled summary of (sfy_file) containing sorted");
      EMIT_WRITE("  PDFs for fast lookup.\n");
      EMIT_WRITE("  (width0) is the downsample resolution, that is, the number of floats per new");
      EMIT_WRITE("  PDF. Note that N floats imply (N+1) regions (slices).\n");
      EMIT_WRITE("  ...and so on with [(index_file1) (width1)] etc.\n");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        downsample_error_print(emit_mode, "One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    parameter_text_base=argv[1];
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, EMIT2);
    if(status){
      downsample_parameter_error_print(emit_mode, "verbosity");
      break;
    }
    status=1;
    emit_mode=(u8)(parameter);
/*
Increment emit_mode because we provide 3 levels, whereas emit.h provides 4. The difference is that we don't have any need for priority zero (critical) messages.
*/
    emit_mode++;
    sfy_pathname_base=argv[2];
    filesys_status=filesys_file_mem_map(&descriptor, &archive_size_u64, sfy_pathname_base, (void **)(&archive_u32_list_base), 0);
    if(filesys_status){
      if(filesys_status==FILESYS_STATUS_MEM_MAP_FAIL){
        downsample_error_print(emit_mode, "Could not make a readonly memory map for (sfy_file)");
      }else{
        downsample_error_print(emit_mode, "Could not open (sfy_file) for reading");
      }
      break;
    }
    header_base=archive_header_init();
    if(!header_base){
      downsample_out_of_memory_print(emit_mode);
      break;
    }
    archive_header_import(header_base, archive_u32_list_base);
    status=archive_header_check(archive_size_u64, header_base, &lmd2_iterand, 0, &lmd2_partial_sum, &pdf_count, &pdf_float_count, &pdf_slot_count);
    if(status){
      downsample_error_print(emit_mode, "(sfy_file) header is corrupt");
      break;
    }
    status=1;
/*
Each width parameter is a count of (u32)s to appear in a corresponding downsampled archive. The maximum index of such widths is the number of them less one, which can be computed from the commandline argument count (argc) and saved into u32_count_idx_max.
*/
    u32_count_idx_max=(((ULONG)(argc)-3)>>1)-1;
    list_size=(u32_count_idx_max+1)<<ULONG_SIZE_LOG2;
    u32_count_list_base=DEBUG_MALLOC_PARANOID(list_size);
    if(!u32_count_list_base){
      downsample_out_of_memory_print(emit_mode);
      break;
    }
    arg_idx=4;
    u32_count_idx=0;
    do{
      parameter_text_base=argv[arg_idx];
      status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, ULONG_MAX);
      if(status){
        downsample_error_print(emit_mode, "Invalid syntax. Run without parameters for help.");
        break;
      }
      arg_idx+=2;
      u32_count_list_base[u32_count_idx]=(ULONG)(parameter);
    }while((u32_count_idx++)!=u32_count_idx_max);
    if(status){
      break;
    }
    status=1;
    u32_list_base_list_base=archive_list_malloc(pdf_count, u32_count_idx_max, u32_count_list_base);
    if(!u32_list_base_list_base){
      downsample_out_of_memory_print(emit_mode);
      break;
    }
    sort_status=1;
    status=archive_downsample(u32_count_idx_max, u32_count_list_base, &sort_status, archive_u32_list_base, u32_list_base_list_base);
    switch(status){
    case 1:
      downsample_out_of_memory_print(emit_mode);
      break;
    case 2:
      downsample_error_print(emit_mode, "Each (width) value must exceed 1");
      break;
    case 3:
      downsample_error_print(emit_mode, "You asked for a (width) which exceeds the value (floats_per_row) with which (sfy_file) was created");
      break;
    }
    if(status){
      status=1;
      break;
    }
    status=1;
    if(!sort_status){
      downsample_warning_print(emit_mode, "(sfy_file) is not optimized, so downsampled output file cannot be used for lookup");
    }
    filesys_status=filesys_file_mem_unmap(descriptor, archive_size_u64, archive_u32_list_base);
    if(filesys_status){
      downsample_error_print(emit_mode, "Could not close memory map for (sfy_file)");
      break;
    }
    downsample_progress_print(emit_mode, "Saving output files...");
    arg_idx=3;
    u32_count_idx=0;
    do{
      out_pathname_base=argv[arg_idx];
      downsample_progress_print(emit_mode, out_pathname_base);
      archive_size=u32_count_list_base[u32_count_idx];
      u32_list_base=u32_list_base_list_base[u32_count_idx];
      filesys_status=filesys_file_write_obnoxious(0, archive_size, out_pathname_base, u32_list_base);
      if(filesys_status){
        downsample_error_print(emit_mode, "Cannot write to output file(s)");
        break;
      }
      arg_idx+=2;
    }while((u32_count_idx++)!=u32_count_idx_max);
    if(filesys_status){
      break;
    }
    downsample_progress_print(emit_mode, "Done");
    status=0;
  }while(0);
  archive_list_free(u32_count_idx_max, u32_list_base_list_base);
  DEBUG_FREE_PARANOID(u32_count_list_base);
  archive_free(header_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
