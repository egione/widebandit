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

void spectrafy_error_print(u8 emit_mode, char *text_base);

u8
spectrafy_file_size_multiple_check(u8 emit_mode, ULONG file_size, ULONG spectrum_size){
  u8 status;

  status=0;
  if(!file_size){
    status=1;
    spectrafy_error_print(emit_mode, "All file sizes in (float_file_or_folder) must be nonzero");
  }else if(file_size%spectrum_size){
    status=1;
    spectrafy_error_print(emit_mode, "All file sizes in (float_file_or_folder) must be a multiple of ((floats_per_row)*4)");
  }
  return status;
}

void
spectrafy_float_report(u8 emit_mode, u8 emit_priority, ULONG float_idx, char *float_text_base){
  ULONG u8_idx;

  if(emit_priority<=emit_mode){
    if(emit_priority==EMIT1){
      EMIT_PRINT("ERROR: ");
    }else if(emit_priority==EMIT2){
      EMIT_PRINT("WARNING: ");
    }
    u8_idx=float_idx<<U32_SIZE_LOG2;
    EMIT_PRINT(float_text_base);
    EMIT_PRINT(" is present at (sfy_file) offset 0x");
    EMIT_U64("", u8_idx);
    EMIT_WRITE(".");
  }
  return;
}

void
spectrafy_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("ERROR: ");
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

void
spectrafy_not_found_print(u8 emit_mode, char *text_base){
  if(EMIT3<=emit_mode){
    EMIT_PRINT("No ");
    EMIT_PRINT(text_base);
    EMIT_WRITE(" found.");
  }
  return;
}

void
spectrafy_out_of_memory_print(u8 emit_mode){
  spectrafy_error_print(emit_mode, "Out of memory");
  return;
}

void
spectrafy_parameter_error_print(u8 emit_mode, char *text_base){
  if(EMIT1<=emit_mode){
    EMIT_PRINT("Invalid parameter: (");
    EMIT_PRINT(text_base);
    EMIT_WRITE("). For help, run without parameters.");
  }
  return;
}

void
spectrafy_progress_print(u8 emit_mode, char *text_base){
  if(EMIT3<=emit_mode){
    EMIT_PRINT(text_base);
    EMIT_WRITE(".");
  }
  return;
}

void
spectrafy_value_report(u8 emit_mode, u8 emit_priority, char *text_base, u64 value){
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
spectrafy_value_report_u32(u8 emit_mode, u8 emit_priority, char *text_base, u32 value){
  if(emit_priority<=emit_mode){
    if(emit_priority==EMIT1){
      EMIT_PRINT("ERROR: ");
    }else if(emit_priority==EMIT2){
      EMIT_PRINT("WARNING: ");
    }
    EMIT_PRINT(text_base);
    EMIT_PRINT(" ");
    EMIT_U32("", value);
    EMIT_WRITE(".");
  }
  return;
}

void
spectrafy_warning_print(u8 emit_mode, char *text_base){
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
  ULONG archive_u32_idx_max;
  ULONG archive_u32_idx_min;
  ULONG archive_u32_idx_post;
  u32 *archive_u32_list_base;
  u8 archive_status;
  ULONG arg_idx;
  u8 check_status;
  u32 checksum;
  u8 delete_status;
  u8 emit_mode;
  u8 expansion_status;
  u8 fatal_status;
  u8 file_status;
  u8 filesys_status;
  ULONG floats_per_row;
  ULONG *freq_list_base;
  u8 hash_reset_status;
  header_t *header_base;
  ULONG in_file_size;
  ULONG in_file_size_max;
  ULONG in_file_size_sum;
  ULONG in_file_u32_idx_max;
  ULONG in_filename_count;
  ULONG in_filename_idx;
  char *in_filename_list_base;
  ULONG in_filename_list_char_idx;
  ULONG in_filename_list_char_idx_max;
  ULONG in_filename_list_char_idx_new;
  ULONG in_filename_list_size;
  ULONG in_filename_list_size_new;
  char *in_pathname_base;
  ULONG list_size;
  u64 lmd2_iterand;
  u64 lmd2_partial;
  u64 lmd2_partial_sum;
  u64 parameter;
  char *parameter_text_base;
  ULONG pdf_count;
  ULONG pdf_float_count;
  ULONG pdf_float_count_new;
  ULONG pdf_free_count;
  ULONG pdf_free_count_new;
  ULONG pdf_idx;
  ULONG pdf_idx_max;
  ULONG pdf_slot_count;
  ULONG pdf_slot_count_new;
  ULONG pdf_slot_idx_max;
  u8 retry_status;
  char *sfy_pathname_base;
  u8 sort_status;
  ULONG spectrum_pdf_float_count;
  ULONG spectrum_pdf_float_idx_max;
  ULONG spectrum_pdf_float_idx_min;
  ULONG spectrum_size;
  ULONG spectrum_u32_idx_max;
  ULONG spectrum_u32_idx_min;
  u32 *spectrum_u32_list_base0;
  u32 *spectrum_u32_list_base1;
  u8 status;
  u8 tune_status;
  ULONG u32_idx_max;
  ULONG u32_idx_min;
  ULONG u32_idx_min_infinity;
  ULONG u32_idx_min_nan;
  ULONG u32_idx_min_negative;
  ULONG u32_idx_min_nonempty;
  ULONG u32_idx_min_subnormal;
  ULONG u32_idx_min_zero_minus;
  ULONG u32_idx_min_zero_plus;

  status=archive_init(ARCHIVE_BUILD_BREAK_COUNT_EXPECTED, 0);
  status=(u8)(status|ascii_init(ASCII_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|fakefloat_init(FAKEFLOAT_BUILD_BREAK_COUNT_EXPECTED, 0));
  status=(u8)(status|filesys_init(FILESYS_BUILD_BREAK_COUNT_EXPECTED, 5));
  archive_u32_idx_post=0;
  archive_u32_list_base=NULL;
  emit_mode=EMIT3;
  freq_list_base=NULL;
  header_base=NULL;
  in_filename_list_base=NULL;
  pdf_idx_max=0;
  pdf_slot_idx_max=0;
  spectrum_u32_list_base0=NULL;
  spectrum_u32_list_base1=NULL;
  do{
    if(status){
      spectrafy_error_print(emit_mode, "Outdated source code");
      break;
    }
    status=1;
    if((argc<3)||(5<argc)){
      EMIT_WRITE("Spectrafy\nCopyright 2021 Russell Leidich\nhttps://github.com/egione/Widebandit");
      EMIT_WRITE("Build 1");
      EMIT_WRITE("Construct probability distribution functions from 32-bit floats.\n");
      EMIT_WRITE("Syntax:\n");
      EMIT_WRITE("  spectrafy verbosity sfy_file [floats_per_row [float_file_or_folder]]\n");
      EMIT_WRITE("where:\n");
      EMIT_WRITE("  (verbosity) is one of:\n");
      EMIT_WRITE("    0: Report only errors.\n");
      EMIT_WRITE("    1: Report errors and warnings.\n");
      EMIT_WRITE("    2: Report errors, warnings, and progress information.\n");
      EMIT_WRITE("  (sfy_file) is the name of the archive containing probability distribution");
      EMIT_WRITE("  functions (PDFs) composed of 32-bit IEEE754 floating-point values (\"floats\")");
      EMIT_WRITE("  obtained from previous invokations of this utility. If it doesn't exist, then");
      EMIT_WRITE("  it will be created. The filename extension must be \".sfy\", or nothing, in");
      EMIT_WRITE("  which case it will automatically be appended.\n");
      EMIT_WRITE("  If none of the following parameters are present, then this command will");
      EMIT_WRITE("  produce a report on the data integrity of (sfy_file):\n");
      EMIT_WRITE("  (floats_per_row) has 2 possible meanings:\n");
      EMIT_WRITE("    If (float_file_or_folder) is specified, then (floats_per_row) is the number");
      EMIT_WRITE("    of floats in each row of each file designated by the former. Different files");
      EMIT_WRITE("    may contain different numbers of rows, but each of them must have size");
      EMIT_WRITE("    ((floats_per_row)*4). If preceded by a minus sign, then the meaning is to");
      EMIT_WRITE("    remove all such rows from the archive. This is useful in case previously");
      EMIT_WRITE("    integrated data is later found to be corrupt or duplicate.\n");
      EMIT_WRITE("    Otherwise, (floats_per_row) cannot be negative, as it specifies the number");
      EMIT_WRITE("    of free floats to reserve in each PDF in the archive. This number is then");
      EMIT_WRITE("    equal to the maximum number of new rows that may be integrated before more");
      EMIT_WRITE("    space is required. If not added by this direct method, space will be added");
      EMIT_WRITE("    automatically in a roughly exponential manner, which wastes time and space.");
      EMIT_WRITE("    If set to 0, then all free space will be removed and each PDF will be");
      EMIT_WRITE("    internally sorted so as to facilitate fast lookup.\n");
      EMIT_WRITE("  (float_file_or_folder) is a file or folder (or folder tree) containing");
      EMIT_WRITE("  floats as specified above.");
      break;
    }
    arg_idx=0;
    do{
      status=ascii_utf8_string_verify(argv[arg_idx]);
      if(status){
        spectrafy_error_print(emit_mode, "One or more parameters is encoded using invalid UTF8");
        break;
      }
    }while((++arg_idx)<(ULONG)(argc));
    if(status){
      break;
    }
    parameter_text_base=argv[1];
    status=ascii_decimal_to_u64_convert(parameter_text_base, &parameter, EMIT2);
    if(status){
      spectrafy_parameter_error_print(emit_mode, "verbosity");
      break;
    }
    status=1;
    check_status=(argc==3);
    delete_status=0;
    sort_status=0;
    tune_status=(argc==4);
    emit_mode=(u8)(parameter);
/*
Increment emit_mode because we provide 3 levels, whereas emit.h provides 4. The difference is that we don't have any need for priority zero (critical) messages.
*/
    emit_mode++;
    floats_per_row=0;
    hash_reset_status=0;
    pdf_count=0;
    if(!check_status){
      parameter_text_base=argv[3];
      status=ascii_decimal_to_u64_convert_negatable(parameter_text_base, &parameter, ULONG_MAX>>U32_SIZE_LOG2, &delete_status);
      if(status){
        spectrafy_parameter_error_print(emit_mode, "floats_per_row");
        break;
      }
      floats_per_row=(ULONG)(parameter);
/*
If we delete floats instead of append them, we'll need to reevaluate the archive hash from scratch. The reason is that deletion requires sorting the PDFs for fast lookup of floats to delete, so incremental hashing is not an option.
*/
      hash_reset_status=delete_status;
      status=1;
      if(!tune_status){
        if(!floats_per_row){
          spectrafy_error_print(emit_mode, "(floats_per_row) must be nonzero when (float_file_or_folder) is specified");
          break;
        }
        pdf_count=floats_per_row;
      }else{
        if(delete_status){
          spectrafy_error_print(emit_mode, "(floats_per_row) cannot be negative when (float_file_or_folder) isn't specified");
          break;
        }
      }
    }
    sfy_pathname_base=argv[2];
    header_base=archive_header_init();
    if(!header_base){
      spectrafy_out_of_memory_print(emit_mode);
      break;
    }
    filesys_status=filesys_file_size_get(&archive_size_u64, sfy_pathname_base);
    archive_status=!filesys_status;
    archive_size=archive_size_check(archive_size_u64);
    if(archive_status){
      if(!archive_size){
        spectrafy_error_print(emit_mode, "(sfy_file) size expected to be a nonzero multiple of 4");
        break;
      }
      archive_u32_idx_max=(archive_size>>U32_SIZE_LOG2)-1;
      if(!check_status){
        archive_u32_idx_max=ARCHIVE_HEADER_U32_COUNT-1;
      }
      archive_u32_list_base=fakefloat_u32_list_malloc(0, archive_u32_idx_max);
      if(!archive_u32_list_base){
        spectrafy_out_of_memory_print(emit_mode);
        break;
      }
      if(!check_status){
        filesys_status=filesys_subfile_read(0, sfy_pathname_base, ARCHIVE_HEADER_SIZE, 0, archive_u32_list_base);
        if(filesys_status){
          spectrafy_error_print(emit_mode, "Could not read (sfy_file)");
          break;
        }
      }else{
        spectrafy_progress_print(emit_mode, "Checking integrity of (sfy_file)..");
        filesys_status=filesys_file_read_exact(archive_size, sfy_pathname_base, archive_u32_list_base);
        if(filesys_status){
          spectrafy_error_print(emit_mode, "(sfy_file) changed size during execution");
          break;
        }
      }
      archive_header_import(header_base, archive_u32_list_base);
      status=archive_header_check(archive_size_u64, header_base, &lmd2_iterand, check_status, &lmd2_partial_sum, &pdf_count, &pdf_float_count, &pdf_slot_count);
      if(status){
        spectrafy_error_print(emit_mode, "(sfy_file) header is corrupt");
        break;
      }
      status=1;
      pdf_free_count=pdf_slot_count-pdf_float_count;
      if(!check_status){
        if(!tune_status){
          if(floats_per_row!=pdf_count){
            spectrafy_error_print(emit_mode, "(floats_per_row) differs from the corresponding value in the (sfy_file) header");
            break;
          }
          if(delete_status&&!pdf_float_count){
            spectrafy_error_print(emit_mode, "Nothing to delete because (sfy_file) contains only empty space");
            break;
          }
        }
        archive_u32_list_base=fakefloat_free(archive_u32_list_base);
      }else{
        sort_status=1;
        checksum=0;
        pdf_idx=0;
        u32_idx_min=ARCHIVE_HEADER_U32_COUNT;
        u32_idx_min_infinity=ULONG_MAX;
        u32_idx_min_nan=u32_idx_min_infinity;
        u32_idx_min_negative=u32_idx_min_infinity;
        u32_idx_min_nonempty=u32_idx_min_infinity;
        u32_idx_min_subnormal=u32_idx_min_infinity;
        u32_idx_min_zero_minus=u32_idx_min_infinity;
        u32_idx_min_zero_plus=u32_idx_min_infinity;
        u32_idx_max=u32_idx_min+pdf_float_count-1;
        do{
          lmd2_iterand=0;
          if(pdf_float_count){
            fakefloat_u32_list_check(&checksum, &lmd2_iterand, &lmd2_partial, &sort_status, u32_idx_max, u32_idx_min, &u32_idx_min_infinity, &u32_idx_min_nan, &u32_idx_min_negative, &u32_idx_min_subnormal, &u32_idx_min_zero_minus, &u32_idx_min_zero_plus, archive_u32_list_base);
            lmd2_partial_sum-=lmd2_partial;
          }
          u32_idx_max+=pdf_free_count;
          u32_idx_min+=pdf_float_count;
          if(pdf_free_count&&!~u32_idx_min_nonempty){
            u32_idx_min_nonempty=fakefloat_u32_list_empty_check(u32_idx_max, u32_idx_min, archive_u32_list_base);
          }
          u32_idx_max+=pdf_float_count;
          u32_idx_min+=pdf_free_count;
          pdf_idx++;
        }while(pdf_idx!=pdf_count);
        if(lmd2_partial_sum){
          spectrafy_error_print(emit_mode, "Bad LMD2 hash, indicating corruption");
          break;
        }else{
          spectrafy_progress_print(emit_mode, "Hash is correct");
        }
        spectrafy_value_report(emit_mode, EMIT3, "Number of PDFs is", pdf_count);
        spectrafy_value_report(emit_mode, EMIT3, "Slots per PDF is", pdf_slot_count);
        spectrafy_value_report(emit_mode, EMIT3, "Allocated slots per PDF is", pdf_float_count);
        spectrafy_value_report(emit_mode, EMIT3, "Free slots per PDF is", pdf_free_count);
        spectrafy_value_report_u32(emit_mode, EMIT3, "Checksum of allocated slots is", checksum);
        if(!sort_status){
          spectrafy_progress_print(emit_mode, "Not optimized for fast lookup");
        }else{
          spectrafy_progress_print(emit_mode, "Optimized for fast lookup");
        }
        if(~u32_idx_min_infinity){
          spectrafy_float_report(emit_mode, EMIT2, u32_idx_min_infinity, "An infinity");
        }else{
          spectrafy_not_found_print(emit_mode, "infinities");
        }
        if(~u32_idx_min_negative){
          spectrafy_float_report(emit_mode, EMIT2, u32_idx_min_negative, "A negative");
        }else{
          spectrafy_not_found_print(emit_mode, "negatives");
        }
        if(~u32_idx_min_subnormal){
          spectrafy_float_report(emit_mode, EMIT2, u32_idx_min_subnormal, "A subnormal");
        }else{
          spectrafy_not_found_print(emit_mode, "subnormals");
        }
        if(~u32_idx_min_zero_minus){
          spectrafy_float_report(emit_mode, EMIT2, u32_idx_min_zero_minus, "A signed zero");
        }else{
          spectrafy_not_found_print(emit_mode, "signed zeroes");
        }
        if(~u32_idx_min_zero_plus){
          spectrafy_float_report(emit_mode, EMIT2, u32_idx_min_zero_plus, "An unsigned zero");
        }else{
          spectrafy_not_found_print(emit_mode, "unsigned zeroes");
        }
        status=0;
        if(~u32_idx_min_nan){
          status=1;
          spectrafy_float_report(emit_mode, EMIT1, u32_idx_min_nan, "A NaN");
        }else{
          spectrafy_not_found_print(emit_mode, "NaNs");
        }
        if(~u32_idx_min_nonempty){
          status=1;
          spectrafy_float_report(emit_mode, EMIT1, u32_idx_min_nonempty, "Buffer overflow? Nonempty data in free space");
        }else{
          spectrafy_not_found_print(emit_mode, "corrupted free space");
        }
        spectrafy_progress_print(emit_mode, "Inspection complete");
        break;
      }
      status=1;
      if(tune_status){
        pdf_free_count_new=floats_per_row;
        pdf_float_count_new=pdf_float_count;
        pdf_slot_count_new=pdf_float_count+pdf_free_count_new;
        if(pdf_free_count_new&&(pdf_free_count==pdf_free_count_new)){
          spectrafy_progress_print(emit_mode, "Each PDF already contains exactly (floats_per_row) free slots. Nothing to do");
          status=0;
          break;
        }
        if(pdf_slot_count_new){
          spectrafy_value_report(emit_mode, EMIT3, "Free slots per PDF will be", pdf_free_count_new);
        }else{
          pdf_free_count_new++;
          pdf_slot_count_new++;
          if(pdf_slot_count!=pdf_slot_count_new){
            spectrafy_warning_print(emit_mode, "The archive is empty. Due to file format constraints, there will be one free slot per PDF even though you requested zero");
          }
        }
        archive_u32_idx_post=MAX(pdf_slot_count, pdf_slot_count_new);
        if((archive_u32_idx_post*pdf_count/pdf_count)!=archive_u32_idx_post){
          spectrafy_out_of_memory_print(emit_mode);
          break;
        }
        archive_u32_idx_post*=pdf_count;
        archive_u32_idx_post+=ARCHIVE_HEADER_U32_COUNT;
        if(archive_u32_idx_post<ARCHIVE_HEADER_U32_COUNT){
          spectrafy_out_of_memory_print(emit_mode);
          break;
        }
        archive_u32_idx_max=archive_u32_idx_post-1;
        archive_u32_list_base=fakefloat_u32_list_malloc(0, archive_u32_idx_max);
        if(!archive_u32_list_base){
          spectrafy_out_of_memory_print(emit_mode);
          break;
        }
        if(pdf_slot_count_new<pdf_slot_count){
          archive_u32_idx_post-=pdf_count*(pdf_slot_count-pdf_slot_count_new);
        }
        spectrafy_progress_print(emit_mode, "Reading (sfy_file)..");
        filesys_status=filesys_file_read_exact(archive_size, sfy_pathname_base, archive_u32_list_base);
        if(filesys_status){
          spectrafy_error_print(emit_mode, "(sfy_file) changed size during execution");
          break;
        }
        pdf_idx_max=pdf_count-1;
        pdf_slot_idx_max=pdf_slot_count_new-1;
        if((!pdf_free_count)&&pdf_float_count){
          spectrafy_progress_print(emit_mode, "Checking optimization state..");
          archive_u32_idx_max=pdf_float_count+ARCHIVE_HEADER_U32_COUNT-1;
          archive_u32_idx_min=ARCHIVE_HEADER_U32_COUNT;
          pdf_idx=0;
          do{
            sort_status=fakefloat_u32_list_sort_check(archive_u32_idx_max, archive_u32_idx_min, archive_u32_list_base);
            archive_u32_idx_max+=pdf_slot_count;
            archive_u32_idx_min+=pdf_slot_count;
            pdf_idx++;
          }while(sort_status&&(pdf_count!=pdf_idx));
        }
        if(pdf_slot_count!=pdf_slot_count_new){
          spectrafy_progress_print(emit_mode, "Adjusting free slots..");
          fakefloat_u32_list_interleave(pdf_idx_max, pdf_float_count, pdf_slot_count, pdf_slot_count_new, ARCHIVE_HEADER_U32_COUNT, archive_u32_list_base);
        }
        if(!pdf_free_count_new){
          if(!sort_status){
            spectrafy_progress_print(emit_mode, "Optimizing (sorting PDFs)..");
            freq_list_base=fakefloat_freq_list_malloc();
            status=!freq_list_base;
            spectrum_pdf_float_idx_max=pdf_float_count_new-1;
            spectrum_u32_list_base0=fakefloat_u32_list_malloc(0, spectrum_pdf_float_idx_max);
            status=(u8)(status|!spectrum_u32_list_base0);
            spectrum_u32_list_base1=fakefloat_u32_list_malloc(0, spectrum_pdf_float_idx_max);
            status=(u8)(status|!spectrum_u32_list_base1);
            if(status){
              spectrafy_out_of_memory_print(emit_mode);
              break;
            }
            status=1;
            archive_u32_idx_min=ARCHIVE_HEADER_U32_COUNT;
            list_size=pdf_slot_count_new<<U32_SIZE_LOG2;
            pdf_idx=0;
            do{
              memcpy(spectrum_u32_list_base0, &archive_u32_list_base[archive_u32_idx_min], (size_t)(list_size));
              fakefloat_u32_list_sort(freq_list_base, 1, spectrum_pdf_float_idx_max, spectrum_u32_list_base0, spectrum_u32_list_base1);
              memcpy(&archive_u32_list_base[archive_u32_idx_min], spectrum_u32_list_base0, (size_t)(list_size));
              archive_u32_idx_min+=pdf_slot_count_new;
              pdf_idx++;
            }while(pdf_idx!=pdf_count);
            hash_reset_status=1;
          }else{
            spectrafy_progress_print(emit_mode, "Already optimized");
            if(!pdf_free_count){
              break;
            }
          }
        }
      }
    }else if(check_status||delete_status||tune_status){
      spectrafy_error_print(emit_mode, "(sfy_file) not found");
      break;
    }
    status=1;
    if(!tune_status){
      in_pathname_base=argv[4];
      fatal_status=0;
      in_file_size_max=0;
      in_filename_count=0;
      in_filename_list_size=U16_MAX;
      retry_status=0;
      do{
        in_filename_list_char_idx_max=in_filename_list_size-1;
        in_filename_list_base=filesys_char_list_malloc(in_filename_list_char_idx_max);
        if(!in_filename_list_base){
          fatal_status=1;
          spectrafy_out_of_memory_print(emit_mode);
          break;
        }
        in_filename_list_size_new=in_filename_list_size;
        retry_status=filesys_filename_list_get(&fatal_status, &in_file_size_max, &in_file_size_sum, &file_status, &in_filename_count, in_filename_list_base, &in_filename_list_size_new, in_pathname_base);
        if(fatal_status){
          spectrafy_error_print(emit_mode, "(float_file_or_folder) not found or inaccessible");
          break;
        }
        if(retry_status){
          in_filename_list_base=filesys_free(in_filename_list_base);
          in_filename_list_size=in_filename_list_size_new;
        }
      }while(retry_status);
      if(fatal_status){
        break;
      }
      if(!~in_file_size_sum){
        spectrafy_out_of_memory_print(emit_mode);
        break;
      }
      if(!in_file_size_max){
        spectrafy_error_print(emit_mode, "All files have zero size");
        break;
      }
      spectrum_size=floats_per_row<<U32_SIZE_LOG2;
      status=spectrafy_file_size_multiple_check(emit_mode, in_file_size_sum, spectrum_size);
      if(status){
        spectrafy_error_print(emit_mode, "At least one file in (float_file_or_folder) has a size which is incompatible with the number of PDFs in (sfy_file)");
        break;
      }
      status=1;
      pdf_float_count_new=in_file_size_sum/spectrum_size;
      pdf_slot_count_new=pdf_slot_count;
      if(!delete_status){
        pdf_float_count_new+=pdf_float_count;
      }else{
        if(pdf_float_count<pdf_float_count_new){
          spectrafy_error_print(emit_mode, "You're asking to delete more data than (sfy_file) contains");
          break;
        }
        pdf_float_count_new=pdf_float_count-pdf_float_count_new;
      }
      expansion_status=(pdf_slot_count_new<pdf_float_count_new);
      if(expansion_status){
        pdf_slot_count_new=1;
        while(pdf_slot_count_new<pdf_float_count_new){
          pdf_slot_count_new<<=1;
          if(!pdf_slot_count_new){
            pdf_slot_count_new=~pdf_slot_count_new;
          }
        }
      }
      if(!archive_status){
        pdf_slot_count_new=pdf_float_count_new;
      }
      archive_u32_idx_post=pdf_count*pdf_slot_count_new;
      if((archive_u32_idx_post/pdf_count)!=pdf_slot_count_new){
        spectrafy_out_of_memory_print(emit_mode);
        break;      
      }
      archive_u32_idx_post+=ARCHIVE_HEADER_U32_COUNT;
      if(archive_u32_idx_post<ARCHIVE_HEADER_U32_COUNT){
        spectrafy_out_of_memory_print(emit_mode);
        break;      
      }
      archive_u32_idx_max=archive_u32_idx_post-1;
      archive_u32_list_base=fakefloat_u32_list_malloc(expansion_status, archive_u32_idx_max);
      if(!archive_u32_list_base){
        spectrafy_out_of_memory_print(emit_mode);
        break;
      }    
      if(archive_status){
        archive_size=(ULONG)(archive_size_u64);
        filesys_status=filesys_file_read_exact(archive_size, sfy_pathname_base, archive_u32_list_base);
        if(filesys_status){
          spectrafy_error_print(emit_mode, "Could not read (sfy_file)");
          break;
        }
      }
      if(delete_status){
        status=archive_sort_compact(0, 0, header_base, 0, archive_u32_list_base);
        freq_list_base=fakefloat_freq_list_malloc();
        status=(u8)(status|!freq_list_base);
        if(status){
          spectrafy_out_of_memory_print(emit_mode);
          break;
        }
      }
      status=filesys_filename_list_sort(in_filename_count, in_filename_list_base);
      in_file_u32_idx_max=(in_file_size_max-1)>>U32_SIZE_LOG2;
      spectrum_u32_list_base0=fakefloat_u32_list_malloc(0, in_file_u32_idx_max);
      status=(u8)(status|!spectrum_u32_list_base0);
      spectrum_u32_list_base1=fakefloat_u32_list_malloc(0, in_file_u32_idx_max);
      status=(u8)(status|!spectrum_u32_list_base1);
      if(status){
        spectrafy_out_of_memory_print(emit_mode);
        break;
      }
      status=1;
      archive_u32_idx_max=0;
      archive_u32_idx_min=ARCHIVE_HEADER_U32_COUNT+pdf_float_count;
      fatal_status=0;
      in_filename_idx=0;
      in_filename_list_char_idx=0;
      pdf_idx_max=pdf_count-1;
      pdf_slot_idx_max=pdf_slot_count_new-1;
      spectrum_pdf_float_count=0;
      do{
        in_file_size=in_file_size_max;
        in_filename_list_char_idx_new=in_filename_list_char_idx;
        filesys_status=filesys_file_read_next(&in_file_size, &in_filename_list_char_idx_new, in_filename_list_base, spectrum_u32_list_base0);
        if((filesys_status==FILESYS_STATUS_SIZE_CHANGED)||(filesys_status==FILESYS_STATUS_TOO_BIG)){
          fatal_status=1;
          spectrafy_error_print(emit_mode, "File in (float_file_or_folder) changed size during execution");
          break;
        }else if(filesys_status==FILESYS_STATUS_NOT_FOUND){
          fatal_status=1;
          spectrafy_error_print(emit_mode, "File in (float_file_or_folder) disappeared during execution");
          break;
        }
        fatal_status=spectrafy_file_size_multiple_check(emit_mode, in_file_size, spectrum_size);
        if(fatal_status){
          spectrafy_error_print(emit_mode, "At least one file in (float_file_or_folder) has a size which is incompatible with the number of PDFs in (sfy_file)");
          break;
        }
        spectrum_u32_idx_max=(in_file_size>>U32_SIZE_LOG2)-1;
        spectrum_u32_idx_min=0;
        fatal_status=fakefloat_u32_list_find(U32_MAX, spectrum_u32_idx_max, &spectrum_u32_idx_min, spectrum_u32_list_base0);
        if(fatal_status){
          spectrafy_error_print(emit_mode, "At least one file in (float_file_or_folder) contains FFFFFFFF hex, which cannot be imported");
          break;
        }
        spectrum_pdf_float_count=in_file_size/spectrum_size;
        spectrum_pdf_float_idx_max=spectrum_pdf_float_count-1;
        fakefloat_u32_list_transpose(spectrum_u32_list_base0, spectrum_u32_list_base1, pdf_idx_max, spectrum_pdf_float_idx_max);
        if(!delete_status){
          if(!expansion_status){
            fakefloat_u32_list_inject(pdf_idx_max, 0, spectrum_u32_list_base1, spectrum_pdf_float_count, pdf_slot_count_new, archive_u32_idx_min, archive_u32_list_base);
          }else{
            fakefloat_u32_list_interleave_inject(pdf_idx_max, 0, spectrum_u32_list_base1, spectrum_pdf_float_count, pdf_float_count, pdf_slot_count, pdf_slot_count_new, ARCHIVE_HEADER_U32_COUNT, archive_u32_list_base);
            expansion_status=0;
          }
          archive_u32_idx_min+=spectrum_pdf_float_count;
        }else{
          archive_u32_idx_max=ARCHIVE_HEADER_U32_COUNT+pdf_float_count-1;
          archive_u32_idx_min=ARCHIVE_HEADER_U32_COUNT;
          list_size=spectrum_pdf_float_count<<U32_SIZE_LOG2;
          pdf_idx=0;
          spectrum_pdf_float_idx_min=0;
          do{
            if(pdf_idx){
              memcpy(spectrum_u32_list_base1, &spectrum_u32_list_base1[spectrum_pdf_float_idx_min], list_size);
            }
            fakefloat_u32_list_sort(freq_list_base, 0, spectrum_pdf_float_idx_max, spectrum_u32_list_base1, spectrum_u32_list_base0);
            fakefloat_u32_sublist_redact(spectrum_pdf_float_idx_max, 0, spectrum_u32_list_base1, archive_u32_idx_max, archive_u32_idx_min, archive_u32_list_base);
            archive_u32_idx_max+=pdf_slot_count_new;
            archive_u32_idx_min+=pdf_slot_count_new;
            spectrum_pdf_float_idx_min+=spectrum_pdf_float_count;
          }while((pdf_idx++)!=pdf_idx_max);
        }
        in_filename_list_char_idx=in_filename_list_char_idx_new;
        in_filename_idx++;
      }while(in_filename_idx!=in_filename_count);
      if(fatal_status){
        break;
      }
      if(delete_status){
        status=archive_sort_compact(1, spectrum_pdf_float_count, header_base, 1, archive_u32_list_base);
        if(status){
          if(status==1){
            spectrafy_out_of_memory_print(emit_mode);
          }else{
            status=1;
            spectrafy_error_print(emit_mode, "Could not delete everything requested from (sfy_file), so no changes have been made");
          }
          break;
        }
        pdf_float_count=0;
      }
    }
    status=1;
    if(hash_reset_status){
      spectrafy_progress_print(emit_mode, "Reevaluating global hash from scratch..");
    }
    archive_header_fill(hash_reset_status, header_base, pdf_float_count_new, pdf_float_count, pdf_idx_max, pdf_slot_idx_max, archive_u32_list_base);
    archive_header_export(header_base, archive_u32_list_base);
    archive_size=archive_u32_idx_post<<U32_SIZE_LOG2;
    if(archive_status){
      spectrafy_progress_print(emit_mode, "Updating (sfy_file)..");
    }else{
      spectrafy_progress_print(emit_mode, "Creating (sfy_file)..");
    }
    filesys_status=filesys_file_write_obnoxious(0, archive_size, sfy_pathname_base, archive_u32_list_base);
    if(filesys_status){
      spectrafy_error_print(emit_mode, "Cannot write to (sfy_file)");
      break;
    }
    spectrafy_progress_print(emit_mode, "Done");
    status=0;
  }while(0);
  fakefloat_free(freq_list_base);
  fakefloat_free(spectrum_u32_list_base1);
  fakefloat_free(spectrum_u32_list_base0);
  fakefloat_free(archive_u32_list_base);
  filesys_free(in_filename_list_base);
  archive_free(header_base);
  DEBUG_ALLOCATION_CHECK();
  return status;
}
