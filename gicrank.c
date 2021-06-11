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
#include "flag_ascii.h"
#include "flag_filesys.h"
#include "flag_gic.h"
#include <stdint.h>
#include <string.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "ascii_xtrn.h"
#include "emit.h"
#include "emit_xtrn.h"
#include "filesys.h"
#include "filesys_xtrn.h"
#include "gic.h"
#include "gic_xtrn.h"

void gicrank_error_print(u8 emit_mode, char *text_base);

void
gicrank_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("ERROR: ");
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

void
gicrank_out_of_memory_print(u8 emit_mode){
  gicrank_error_print(emit_mode, "Out of memory");
  return;
}

void
gicrank_parameter_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("Invalid parameter: (");
    EMIT_PRINT(text_base);
    EMIT_WRITE("). For help, run without parameters.");
  }
  return;
}

void
gicrank_progress_print(u8 emit_mode, char *text_base){
  if(EMIT3<=emit_mode){
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

int
main(int argc, char *argv[]){
  ULONG arg_idx;
  u8 emit_mode;
  ULONG file_size;
  u64 file_size_u64;
  u8 filesys_status;
  gauss_t *gauss_list_base0;
  gauss_t *gauss_list_base1;
  char *gauss_pathname_base0;
  char *gauss_pathname_base1;
  gic_t *gic_base;
  char *gic_rank_pathname_base;
  gic_ranker_t *gic_ranker_list_base0;
  gic_ranker_t *gic_ranker_list_base1;
  gic_ranker_t *gic_ranker_list_base2;
  char *gic_ratio_pathname_base;
  u8 gic_ratio_status;
  u64 parameter;
  char *parameter_text_base;
  ULONG transfer_size;
  u8 status;

  status=ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 5));
  status=(u8)(status|gic_init(GIC_BUILD_BREAK_COUNT_EXPECTED, 0));
  emit_mode=EMIT3;
  gauss_list_base0=NULL;
  gauss_list_base1=NULL;
  gic_base=NULL;
  gic_ranker_list_base0=NULL;
  gic_ranker_list_base1=NULL;
  gic_ranker_list_base2=NULL;
  do{
    if(status){
      gicrank_error_print(emit_mode, "Outdated source code");
      break;
    }
    status=1;
    if((argc!=5)&&(argc!=6)){
      EMIT_WRITE("GICRank\nCopyright 2021 Russell Leidich\nhttps://github.com/egione/Widebandit");
      EMIT_WRITE("Build 1");
      EMIT_WRITE("Rank the Gaussian information criterion delta across a spectrum.\n");
      EMIT_WRITE("Syntax:\n");
      EMIT_WRITE("  gicrank verbosity gauss_file0 gauss_file1 gic_rank_file [gic_ratio_file]\n");
      EMIT_WRITE("where:\n");
      EMIT_WRITE("  (gauss_file0) is the name of the source file produced by Gaussify, which");
      EMIT_WRITE("  contains the means and standard deviations of the background Gaussians.\n");
      EMIT_WRITE("  (gauss_file1) is the name of the target file produced by Gaussify, which");
      EMIT_WRITE("  contains the means and standard deviations of the experimental PDFs.\n");
      EMIT_WRITE("  (gic_rank_file) is the name of the file to overwrite with data of the");
      EMIT_WRITE("  following format, wherein each index contains 8 bytes:\n");
      EMIT_WRITE("    [0]: (pdf_count) This value is unchanged from the input files. It tells the");
      EMIT_WRITE("    number of 16-byte items starting at index 2.\n");
      EMIT_WRITE("    [1]: (float_count) This value was copied from (gauss_file1). It tells the");
      EMIT_WRITE("    number of 32-bit floats which were used to generate all of the following");
      EMIT_WRITE("    64-bit floats.\n");
      EMIT_WRITE("    [2+2N]: (pdf_idx) The index of the PDF (or channel), sorted descending by");
      EMIT_WRITE("    (gic_rank). Thus higher ranks represent higher likelihood of signals.\n");
      EMIT_WRITE("    [2+2N+1]: (gic_rank) The GIC delta from PDF number (pdf_idx) in");
      EMIT_WRITE("    (gauss_file0) to the same in (gauss_file1).\n");
      EMIT_WRITE("  (gic_rank_file) is, optionally, the name of the file to overwrite with data");
      EMIT_WRITE("  of the same format as (gic_rank_file), except that: (1) (gic_rank) is");
      EMIT_WRITE("  replaced with (gic_ratio), which is the ([N+1]/[N]) ratio of (gic_rank)s when");
      EMIT_WRITE("  sorted in descending order; (2) said (gic_ratio)s are sorting in ascending");
      EMIT_WRITE("  order; and (3) the greatst (and last) (gic_ratio) is forced to be 1.0.");
      EMIT_WRITE("  of the same format as (gic_rank_file), except that:\n");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        gicrank_error_print(emit_mode, "One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    gic_ratio_status=(argc==6);
    parameter_text_base=argv[1];
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, EMIT2);
    if(status){
      gicrank_parameter_error_print(emit_mode, "verbosity");
      break;
    }
    status=1;
    emit_mode=(u8)(parameter);
/*
Increment emit_mode because we provide 3 levels, whereas emit.h provides 4. The difference is that we don't have any need for priority zero (critical) messages.
*/
    emit_mode++;
    gauss_pathname_base0=argv[2];
    filesys_status=filesys_file_size_get(&file_size_u64, gauss_pathname_base0);
    if(filesys_status){
      gicrank_error_print(emit_mode, "(gauss_file0) not found");
      break;
    }
    file_size=(ULONG)(file_size_u64);
    if(file_size!=file_size_u64){
      gicrank_error_print(emit_mode, "(gauss_file0) too large. Try the 64-bit version");
      break;
    }
    if((file_size&U128_BYTE_MAX)||!file_size){
      gicrank_error_print(emit_mode, "(gauss_file0) must be a nonzero multiple of 16 bytes");
      break;
    }
    gauss_pathname_base1=argv[3];
    filesys_status=filesys_file_size_get(&file_size_u64, gauss_pathname_base1);
    if(filesys_status||(file_size!=file_size_u64)){
      gicrank_error_print(emit_mode, "(gauss_file1) must be the same size as (gauss_file0)");
      break;
    }
    gic_base=gic_malloc(1, 1);
    if(!gic_base){
      gicrank_out_of_memory_print(emit_mode);
      break;
    }
    filesys_status=filesys_subfile_read(0, gauss_pathname_base0, (ULONG)(sizeof(gic_t)), 0, gic_base);
    if(filesys_status){
      gicrank_error_print(emit_mode, "Could not read (gauss_file0)");
      break;
    }
    file_size_u64=(gic_base->pdf_count*(ULONG)(sizeof(gauss_t)))+(ULONG)(sizeof(gic_t));
    if(file_size!=file_size_u64){
      gicrank_error_print(emit_mode, "(gauss_file0) is corrupt");
      break;
    }
    filesys_status=filesys_subfile_read(0, gauss_pathname_base1, (ULONG)(sizeof(gic_t)), 0, gic_base);
    if(filesys_status){
      gicrank_error_print(emit_mode, "Could not read (gauss_file1)");
      break;
    }
    file_size_u64=(gic_base->pdf_count)*(ULONG)(sizeof(gauss_t))+(ULONG)(sizeof(gic_t));
    if(file_size!=file_size_u64){
      gicrank_error_print(emit_mode, "(gauss_file1) is corrupt");
      break;
    }
    gauss_list_base0=gic_gauss_list_malloc(gic_base);
    gauss_list_base1=gic_gauss_list_malloc(gic_base);
    gic_ranker_list_base0=gic_ranker_list_malloc(gic_base);
    gic_ranker_list_base1=gic_ranker_list_malloc(gic_base);
    gic_ranker_list_base2=gic_ranker_list_malloc(gic_base);
    if(!(gauss_list_base0&&gauss_list_base1&&gic_ranker_list_base0&&gic_ranker_list_base1&&gic_ranker_list_base2)){
      gicrank_out_of_memory_print(emit_mode);
      break;
    }
    transfer_size=file_size-(ULONG)(sizeof(gic_t));
    filesys_status=filesys_subfile_read(1, gauss_pathname_base0, transfer_size, 0, gauss_list_base0);
    if(filesys_status){
      gicrank_error_print(emit_mode, "Could not read (gauss_file0)");
      break;
    }
    filesys_status=filesys_subfile_read(1, gauss_pathname_base1, transfer_size, 0, gauss_list_base1);
    if(filesys_status){
      gicrank_error_print(emit_mode, "Could not read (gauss_file1)");
      break;
    }
    gic_ranker_list_fill(gauss_list_base0, gauss_list_base1, gic_base, gic_ranker_list_base0);
    status=gic_ranker_list_sort(0, gic_base, gic_ranker_list_base0, gic_ranker_list_base1);
    if(status){
      gicrank_out_of_memory_print(emit_mode);
      break;
    }
    status=1;
    transfer_size=(ULONG)(gic_base->pdf_count)*(ULONG)(sizeof(gic_ranker_t));
    if(gic_ratio_status){
      gic_ranker_list_copy(gic_base, gic_ranker_list_base1, gic_ranker_list_base0);
      gic_ranker_list_to_ratio_list(gic_base, gic_ranker_list_base1);
      status=gic_ranker_list_sort(1, gic_base, gic_ranker_list_base1, gic_ranker_list_base2);
      if(status){
        gicrank_out_of_memory_print(emit_mode);
        break;
      }
      status=1;
      gic_ratio_pathname_base=argv[5];
      filesys_status=filesys_file_write_obnoxious(0, (ULONG)(sizeof(gic_t)), gic_ratio_pathname_base, gic_base);
      if(filesys_status){
        gicrank_error_print(emit_mode, "Cannot write to (gic_ratio_file)");
        break;
      }
      filesys_status=filesys_file_write_obnoxious(1, transfer_size, gic_ratio_pathname_base, gic_ranker_list_base1);
      if(filesys_status){
        gicrank_error_print(emit_mode, "Cannot write to (gic_ratio_file)");
        break;
      }
    }
    gic_ranker_list_scale(gic_base, gic_ranker_list_base0);
    gic_rank_pathname_base=argv[4];
    filesys_status=filesys_file_write_obnoxious(0, (ULONG)(sizeof(gic_t)), gic_rank_pathname_base, gic_base);
    if(filesys_status){
      gicrank_error_print(emit_mode, "Cannot write to (gic_rank_file)");
      break;
    }
    filesys_status=filesys_file_write_obnoxious(1, transfer_size, gic_rank_pathname_base, gic_ranker_list_base0);
    if(filesys_status){
      gicrank_error_print(emit_mode, "Cannot write to (gic_rank_file)");
      break;
    }
    gicrank_progress_print(emit_mode, "Done");
    status=0;
  }while(0);
  gic_free(gic_ranker_list_base2);
  gic_free(gic_ranker_list_base1);
  gic_free(gic_ranker_list_base0);
  gic_free(gauss_list_base1);
  gic_free(gauss_list_base0);
  gic_free(gic_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
