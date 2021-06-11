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
32-Bit Floating-Point Probability Distribution Function Constructor
*/
#include "flag.h"
#include "flag_archive.h"
#include "flag_ascii.h"
#include "flag_fakefloat.h"
#include "flag_filesys.h"
#include "flag_gic.h"
#include <stdint.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "archive.h"
#include "archive_xtrn.h"
#include "ascii_xtrn.h"
#include "emit.h"
#include "emit_xtrn.h"
#include "fakefloat.h"
#include "fakefloat_xtrn.h"
#include "filesys.h"
#include "filesys_xtrn.h"
#include "gic.h"
#include "gic_xtrn.h"

void gaussify_error_print(u8 emit_mode, char *text_base);

void
gaussify_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("ERROR: ");
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

void
gaussify_out_of_memory_print(u8 emit_mode){
  gaussify_error_print(emit_mode, "Out of memory");
  return;
}

void
gaussify_parameter_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("Invalid parameter: (");
    EMIT_PRINT(text_base);
    EMIT_WRITE("). For help, run without parameters.");
  }
  return;
}

void
gaussify_progress_print(u8 emit_mode, char *text_base){
  if(EMIT3<=emit_mode){
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

int
main(int argc, char *argv[]){
  ULONG archive_size;
  u64 archive_size_u64;
  ULONG archive_u32_idx_max;
  ULONG archive_u32_idx_post;
  u32 *archive_u32_list_base;
  ULONG arg_idx;
  u8 emit_mode;
  ULONG file_size;
  u8 filesys_status;
  float *float_list_base;
  gauss_t *gauss_list_base;
  char *gauss_pathname_base;
  gic_t *gic_base;
  header_t *header_base;
  u64 lmd2_iterand;
  u64 lmd2_partial_sum;
  u8 log_status;
  u64 parameter;
  ULONG pdf_count;
  ULONG pdf_float_count;
  ULONG pdf_slot_count;
  char *parameter_text_base;
  char *sfy_pathname_base;
  u8 status;

  status=archive_init(ARCHIVE_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|fakefloat_init(FAKEFLOAT_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 5));
  status=(u8)(status|gic_init(GIC_BUILD_BREAK_COUNT_EXPECTED, 0));
  archive_u32_list_base=NULL;
  emit_mode=EMIT3;
  float_list_base=NULL;
  gauss_list_base=NULL;
  gic_base=NULL;
  header_base=NULL;
  do{
    if(status){
      gaussify_error_print(emit_mode, "Outdated source code");
      break;
    }
    status=1;
    if(argc!=5){
      EMIT_WRITE("Gaussify\nCopyright 2021 Russell Leidich\nhttps://github.com/egione/Widebandit");
      EMIT_WRITE("Build 1");
      EMIT_WRITE("Derive Gaussian parameters for each channel in a Spectrafy archive.\n");
      EMIT_WRITE("Syntax:\n");
      EMIT_WRITE("  gaussify verbosity log sfy_file gauss_file\n");
      EMIT_WRITE("where:\n");
      EMIT_WRITE("  (verbosity) is one of:\n");
      EMIT_WRITE("    0: Report only errors.\n");
      EMIT_WRITE("    1: Report errors and warnings.\n");
      EMIT_WRITE("    2: Report errors, warnings, and progress information.\n");
      EMIT_WRITE("  (log) is 1 to take logs of all samples in the archive before computing");
      EMIT_WRITE("  their means and standard deviations. Else 0.\n");
      EMIT_WRITE("  (sfy_file) the filename of a Spectrafy archive.\n");
      EMIT_WRITE("  (gauss_file) is the filename to overwrite with a data structure of the");
      EMIT_WRITE("  following format, wherein each index contains 8 bytes:\n");
      EMIT_WRITE("    [0]: (pdf_count) The number of 16-byte items starting at index 2.\n");
      EMIT_WRITE("    [1]: (float_count) The number of 32-bit floats which were used to generate");
      EMIT_WRITE("    all of the following 64-bit floats.\n");
      EMIT_WRITE("    [2+2N]: (mean) The mean of the (float_count) floats for channel N.\n");
      EMIT_WRITE("    [2+2N+1]: (variance) The variance of the same.\n");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        gaussify_error_print(emit_mode, "One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    parameter_text_base=argv[1];
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, EMIT2);
    if(status){
      gaussify_parameter_error_print(emit_mode, "verbosity");
      break;
    }
    status=1;
    emit_mode=(u8)(parameter);
/*
Increment emit_mode because we provide 3 levels, whereas emit.h provides 4. The difference is that we don't have any need for priority zero (critical) messages.
*/
    emit_mode++;
    parameter_text_base=argv[2];
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, 1);
    if(status){
      gaussify_parameter_error_print(emit_mode, "log");
      break;
    }
    status=1;
    log_status=(u8)(parameter);
    header_base=archive_header_init();
    if(!header_base){
      gaussify_out_of_memory_print(emit_mode);
      break;
    }
    sfy_pathname_base=argv[3];
    filesys_status=filesys_file_size_get(&archive_size_u64, sfy_pathname_base);
    archive_size=archive_size_check(archive_size_u64);
    if(!archive_size){
      gaussify_error_print(emit_mode, "(sfy_file) size expected to be a nonzero multiple of 4");
      break;
    }
    archive_u32_idx_max=ARCHIVE_HEADER_U32_COUNT-1;
    archive_u32_list_base=fakefloat_u32_list_malloc(0, archive_u32_idx_max);
    if(!archive_u32_list_base){
      gaussify_out_of_memory_print(emit_mode);
      break;
    }
    filesys_status=filesys_subfile_read(0, sfy_pathname_base, ARCHIVE_HEADER_SIZE, 0, archive_u32_list_base);
    if(filesys_status){
      gaussify_error_print(emit_mode, "Could not read (sfy_file)");
      break;
    }
    archive_header_import(header_base, archive_u32_list_base);
    status=archive_header_check(archive_size_u64, header_base, &lmd2_iterand, 0, &lmd2_partial_sum, &pdf_count, &pdf_float_count, &pdf_slot_count);
    if(status){
      gaussify_error_print(emit_mode, "(sfy_file) header is corrupt");
      break;
    }
    status=1;
    gic_base=gic_malloc(pdf_float_count, pdf_count);
    if(!gic_base){
      gaussify_out_of_memory_print(emit_mode);
      break;
    }
    gauss_list_base=gic_gauss_list_malloc(gic_base);
    if(!gauss_list_base){
      gaussify_out_of_memory_print(emit_mode);
      break;
    }
    archive_u32_idx_post=pdf_count;
    if((archive_u32_idx_post*pdf_slot_count/pdf_slot_count)!=archive_u32_idx_post){
      gaussify_out_of_memory_print(emit_mode);
      break;
    }
    archive_u32_idx_post*=pdf_slot_count;
    archive_u32_idx_post+=ARCHIVE_HEADER_U32_COUNT;
    if(archive_u32_idx_post<ARCHIVE_HEADER_U32_COUNT){
      gaussify_out_of_memory_print(emit_mode);
      break;
    }
    archive_u32_idx_max=archive_u32_idx_post-1;
    float_list_base=(float *)(fakefloat_u32_list_malloc(0, archive_u32_idx_max));
    if(!archive_u32_list_base){
      gaussify_out_of_memory_print(emit_mode);
      break;
    }
    filesys_status=filesys_file_read_exact(archive_size, sfy_pathname_base, float_list_base);
    if(filesys_status){
      gaussify_error_print(emit_mode, "(sfy_file) changed size during execution");
      break;
    }
    status=gic_gauss_list_fill(ARCHIVE_HEADER_U32_COUNT, float_list_base, gauss_list_base, gic_base, log_status, pdf_slot_count);
    if(status){
      gaussify_error_print(emit_mode, "(log) was 1 but at least one sample was nonpositive");
      break;
    }
    status=1;
    gauss_pathname_base=argv[4];
    filesys_status=filesys_file_write_obnoxious(0, (ULONG)(sizeof(gic_t)), gauss_pathname_base, gic_base);
    if(filesys_status){
      gaussify_error_print(emit_mode, "Cannot write to (gauss_file)");
      break;
    }
    file_size=pdf_count*(ULONG)(sizeof(gauss_t));
    filesys_status=filesys_file_write_obnoxious(1, file_size, gauss_pathname_base, gauss_list_base);
    if(filesys_status){
      gaussify_error_print(emit_mode, "Cannot write to (gauss_file)");
      break;
    }
    gaussify_progress_print(emit_mode, "Done");
    status=0;
  }while(0);
  fakefloat_free(float_list_base);
  fakefloat_free(archive_u32_list_base);
  gic_free(gauss_list_base);
  gic_free(gic_base);
  archive_free(header_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
