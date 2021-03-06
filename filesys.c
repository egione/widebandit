/*
Filesys
Copyright 2017 Russell Leidich

This collection of files constitutes the Filesys Library. (This is a
library in the abstact sense; it's not intended to compile to a ".lib"
file.)

The Filesys Library is free software: you can redistribute it and/or
modify it under the terms of the GNU Limited General Public License as
published by the Free Software Foundation, version 3.

The Filesys Library is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the GNU
Limited General Public License version 3 for more details.

You should have received a copy of the GNU Limited General Public
License version 3 along with the Filesys Library (filename
"COPYING"). If not, see http://www.gnu.org/licenses/ .
*/
/*
File IO Kernel
*/
#include "flag.h"
#include "flag_filesys.h"
#include <dirent.h>
#include <fcntl.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <unistd.h>
#include "constant.h"
#include "debug.h"
#include "debug_xtrn.h"
#include "filesys.h"
#include "filesys_xtrn.h"

char *
filesys_char_list_malloc(ULONG char_idx_max){
/*
Allocate a list of undefined (char)s.

To maximize portability and debuggability, this is the only function in which Filesys calls malloc().

In:

  char_idx_max is the number of (char)s to allocate, less one.

Out:

  Returns NULL on failure, else the base of (char_idx_max+1) undefined items, which should eventually be freed via filesys_free().
*/
  char *list_base;
  ULONG list_size;

  list_base=NULL;
  list_size=char_idx_max+1;
  if(list_size){
    list_base=DEBUG_MALLOC_PARANOID(list_size);
  }
  return list_base;
}

u8
filesys_file_mem_map(int *descriptor_base, u64 *file_size_base, char *filename_base, void **mem_map_base_base, u8 write_status){
/*
Map a file into memory at an indeterminate address.

In:

  *descriptor_base is undefined.

  *file_size_base is undefined.

  *filename_base is the pathname or filename of the file to map.

  *mem_map_base_base is undefined.

  write_status is one to make the map writeable, or zero to make it readonly.

Out:

  Returns zero on success, else FILESYS_STATUS_NOT_FOUND if *filename_base was not found, FILESYS_STATUS_READ_FAIL if the file could not be opened for reading (and writing, if requested), or 
FILESYS_STATUS_MEM_MAP_FAIL is the OS refused to map it to memory. On failure, the file is neither open nor mapped.

  *descriptor_base is the value to pass as filesys_file_mem_unmap:In:descriptor in order to undo the map.

  *file_size_base is the size of the file, which of course may exceed the address space size of a 32-bit environment.

  *mem_map_base_base is the base at which the file has been mapped in memory. Generally, the map will only appear lazily, as accesses result in page faults. Therefore, depending on the projected access pattern, it might be better to read the entire file preemptively.
*/
  int descriptor;
  u64 file_size;
  void *mem_map_base;
  mode_t mode;
  int prot;
  u8 status;

  file_size=0;
  status=filesys_file_size_get(&file_size, filename_base);
  descriptor=-1;
  mem_map_base=NULL;
  if(!status){
    mode=O_RDONLY;
    prot=PROT_READ;
    if(write_status){
      mode=O_RDWR;
      prot=PROT_READ|PROT_WRITE;
    }
    descriptor=open((const char *)(filename_base), 0, mode);
    *descriptor_base=descriptor;
    status=0;
    if(descriptor==(-1)){
      status=FILESYS_STATUS_READ_FAIL;
    }else{
      mem_map_base=mmap(NULL, (size_t)(file_size), prot, MAP_SHARED, descriptor, 0);
      if(mem_map_base==(MAP_FAILED)){
        close(descriptor);
        status=FILESYS_STATUS_MEM_MAP_FAIL;
        mem_map_base=NULL;
      }
    }
  }
  if(!status){
    *descriptor_base=descriptor;
    *file_size_base=file_size;
    *mem_map_base_base=mem_map_base;
  }
  return status;
}

u8
filesys_file_mem_unmap(int descriptor, u64 file_size, void *mem_map_base){
/*
Undo a previous filesys_file_mem_map() call (but preserve any writes done if writing had been enabled).

In:

  descriptor is filesys_file_mem_map:Out:*decriptor_base.

  file_size is filesys_file_mem_map:Out:*file_size_base.

  mem_map_base is filesys_file_mem_map:Out:*mem_map_base_base.

Out:

  Returns zero on success, else FILESYS_STATUS_MEM_MAP_FAIL if either the unmap or the file close operation failed, both of which should be consider fatal errors.
*/
  u8 status;

  status=!!munmap(mem_map_base, (size_t)(file_size));
  status=(u8)(status|!!close(descriptor));
  if(status){
    status=FILESYS_STATUS_MEM_MAP_FAIL;
  }
  return status;
}

u8
filesys_file_read(ULONG *file_size_max_base, char *filename_base, void *void_list_base){
/*
Read a file into a buffer, subject to a size limit.

In:

  *file_size_max_base is the maximum size to read, such that if the file size exceeds this value, then FILESYS_STATUS_TOO_BIG will be returned.

  *filename_base is the null-terminated path and filename.

  *void_list_base is undefined and writable for *file_size_max_base bytes.

Out:

  Returns zero on success, else: (1) FILESYS_STATUS_SIZE_CHANGED if the file size was less when read than when its size had been read beforehand, (2) FILESYS_STATUS_NOT_FOUND if the file could not be opened, or (3) FILESYS_STATUS_TOO_BIG if the file size was greater than (In:*file_size_max_base).

  *file_size_max_base is the size actually read, which is (1) the size of the entire file if the return value is zero, (2) the size read if the return value is FILESYS_STATUS_SIZE_CHANGED, or (3) zero in all other cases. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.

  *void_list_base contains the first *file_size_max_base bytes of the file. All other bytes are undefined and may have changed. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.
*/
  ULONG filename_idx;
  u8 status;

  filename_idx=0;
  status=filesys_file_read_next(file_size_max_base, &filename_idx, filename_base, void_list_base);
  return status;
}

u8
filesys_file_read_exact(ULONG file_size, char *filename_base, void *void_list_base){
/*
Read a file into a buffer, so long as its size is exactly as expected.

In:

  file_size is the size to read, such that if the file size exceeds this value, then FILESYS_STATUS_TOO_BIG will be returned.

  *filename_base is the null-terminated path and filename.

  *void_list_base is undefined and writable for *file_size_max_base bytes.

Out:

  Returns zero on success, else: (1) FILESYS_STATUS_SIZE_CHANGED if the size of the file was less than file_size, (2) FILESYS_STATUS_NOT_FOUND if the file could not be opened, or (3) FILESYS_STATUS_TOO_BIG if the file size was greater than file_size.

  *void_list_base contains the first file_size bytes of the file. All other bytes are undefined and may have changed. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.
*/
  ULONG filename_idx;
  u8 status;
  ULONG file_size_read;

  filename_idx=0;
  file_size_read=file_size;
  status=filesys_file_read_next(&file_size_read, &filename_idx, filename_base, void_list_base);
  if(((!status)&&(file_size!=file_size_read))||(status==FILESYS_STATUS_TOO_BIG)){
    status=FILESYS_STATUS_SIZE_CHANGED;
  }
  return status;
}

u8
filesys_file_read_next(ULONG *file_size_max_base, ULONG *filename_idx_base, char *filename_list_base, void *void_list_base){
/*
Read a file specified in a filename list, into a buffer, subject to a size limit.

In:

  *file_size_max_base is the maximum size to read, such that if the file size exceeds this value, then FILESYS_STATUS_TOO_BIG will be returned.

  *filename_idx_base is the base index of the filename at filename_list_base.

  *filename_list_base is a concatenation of null-terminated filenames.

  *void_list_base is undefined and writable for *file_size_max_base bytes.

Out:

  Returns zero on success, else: (1) FILESYS_STATUS_SIZE_CHANGED if the file size was less when read than when its size had been read beforehand, (2) FILESYS_STATUS_NOT_FOUND if the file could not be opened, or (3) FILESYS_STATUS_TOO_BIG if the file size was greater than (In:*file_size_max_base).

  *file_size_max_base is the size actually read, which is (1) the size of the entire file if the return value is zero, (2) the size read if the return value is FILESYS_STATUS_SIZE_CHANGED, or (3) zero in all other cases. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.

  *filename_idx_base is the base index of the next filename at filename_list_base, or else the postterminal index of filename_list_base. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.

  *void_list_base contains the first *file_size_max_base bytes of the file. All other bytes are undefined and may have changed. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.
*/
  ULONG file_size_read;
  u64 file_size_u64;
  ULONG filename_idx;
  ULONG filename_size;
  FILE *handle;
  u8 status;

  filename_idx=*filename_idx_base;
  filename_size=(ULONG)(strlen(&filename_list_base[filename_idx]));
  file_size_read=0;
  *filename_idx_base=filename_idx+filename_size+1;
  status=filesys_file_size_get(&file_size_u64, &filename_list_base[filename_idx]);
  if(!status){
    status=FILESYS_STATUS_TOO_BIG;
    if(file_size_u64<=*file_size_max_base){
      handle=fopen(&filename_list_base[filename_idx], "rb");
      status=FILESYS_STATUS_NOT_FOUND;
      if(handle){
        file_size_read=(ULONG)(fread(void_list_base, (size_t)(U8_SIZE), (size_t)(file_size_u64), handle));
        status=FILESYS_STATUS_SIZE_CHANGED;
        if(file_size_read==file_size_u64){
          status=0;
        }
        fclose(handle);
      }
    }
  }
  *file_size_max_base=file_size_read;
  return status;
}

u8
filesys_file_size_get(u64 *file_size_base, char *filename_base){
/*
Get the size of a file via (1) fseeko() on MinGW for Windows because lstat() doesn't work or (2) lstat() otherwise.

In:

  *file_size_base is undefined.

  *filename_base is the null-terminated path and filename.

Out:

  Returns zero on success, else FILESYS_STATUS_NOT_FOUND.

  *file_size base is the size of the file, which is set to zero on failure.
*/
  u64 file_size;
  #ifndef WINDOWS
    struct stat file_stat;
  #else
    FILE *handle;
  #endif
  u8 status;

  file_size=0;
  status=FILESYS_STATUS_NOT_FOUND;
  #ifndef WINDOWS
    if(!lstat(filename_base, &file_stat)){
      file_size=(u64)(file_stat.st_size);
      status=0;
    }
  #else
    handle=fopen(filename_base, "rb");
    if(handle){
      if(!FILESYS_FSEEKO(handle, 0, SEEK_END)){
        file_size=(u64)(FILESYS_FTELLO(handle));
        if(~file_size){
          status=0;
        }
      }
      fclose(handle);
    }
  #endif
  *file_size_base=file_size;
  return status;
}

#ifdef _32_
  u8
  filesys_file_size_ulong_get(ULONG *file_size_base, char *filename_base){
/*
Filter filesys_file_size_get() output to ensure address space compliance for 32-bit systems.

In:

  *file_size_base is undefined.

  *filename_base is the null-terminated path and filename.

Out:

  Returns zero on success, else FILESYS_STATUS_NOT_FOUND or FILESYS_STATUS_TOO_BIG.

  *file_size base is the size of the file, which is set to zero on failure.
*/
    ULONG file_size;
    u64 file_size_u64;
    u8 status;

    status=filesys_file_size_get(&file_size_u64, filename_base);
    file_size=0;
    if(!status){
      file_size=(ULONG)(file_size_u64);
      status=FILESYS_STATUS_TOO_BIG;
      if(file_size==file_size_u64){
        status=0;
      }
    }
    *file_size_base=file_size;
    return status;
  }
#endif

u8
filesys_file_write(u8 append_status, ULONG file_size, char *filename_base, void *void_list_base){
/*
Write a buffer to a file, potentially overwriting any existing file of the same name.

In:

  append_status is one to append to the end of the file, else zero to write from scratch.

  file_size is the size to write.

  *filename_base is the null-terminated path and filename.

  *void_list_base contains the file_size bytes to write.

Out:

  Returns zero on success else FILESYS_STATUS_WRITE_FAIL. In the latter case, the file has been closed but its contents are undefined.
*/
  ULONG file_size_written;
  FILE *handle;
  u8 status;

  if(!append_status){
    handle=fopen(filename_base, "wb");
  }else{
    handle=fopen(filename_base, "ab");
  }
  status=FILESYS_STATUS_WRITE_FAIL;
  if(handle){
    status=0;
    if(file_size){
      file_size_written=(ULONG)(fwrite(void_list_base, (size_t)(U8_SIZE), (size_t)(file_size), handle));
      if(file_size!=file_size_written){
        status=FILESYS_STATUS_WRITE_FAIL;
      }
    }
    if(fclose(handle)){
      status=FILESYS_STATUS_WRITE_FAIL;
    }
  }
  return status;
}

u8
filesys_file_write_next_obnoxious(u8 append_status, ULONG file_size, ULONG *filename_idx_base, char *filename_list_base, void *void_list_base){
/*
Write or append to a file specified in a filename list, into a buffer, subject to a size limit.

In:

  append_status is one to append to the end of the file, else zero to write from scratch.

  file_size is the size to write.

  *filename_idx_base is the base index of the filename at filename_list_base.

  *filename_list_base is a writable concatenation of null-terminated filenames. It will be unchanged upon return.

  *void_list_base contains the file_size bytes to write.

Out:

  Returns zero on success, else FILESYS_STATUS_WRITE_FAIL.

  *filename_idx_base is the base index of the next filename at filename_list_base, or else the postterminal index of filename_list_base. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.
*/
  ULONG filename_idx;
  ULONG filename_size;
  u8 status;

  filename_idx=*filename_idx_base;
  filename_size=(ULONG)(strlen(&filename_list_base[filename_idx]));
  *filename_idx_base=filename_idx+filename_size+1;
  status=filesys_file_write_obnoxious(append_status, file_size, &filename_list_base[filename_idx], void_list_base);
  return status;
}

u8
filesys_file_write_obnoxious(u8 append_status, ULONG file_size, char *filename_base, void *void_list_base){
/*
Write a buffer to a file, overwriting any existing file of the same name. Create folders as needed in order to make the write succeed. All new folders created will have read, write, and execute privileges for the owner but no privileges otherwise.

In:

  append_status is one to append to the end of the file, else zero to write from scratch.

  file_size is the size to write.

  *filename_base is the writable null-terminated path and filename. It will be unchanged upon return.

  *void_list_base contains the file_size bytes to write.

Out:

  Returns zero on success else FILESYS_STATUS_WRITE_FAIL. In the latter case, the file has been closed but its contents are undefined.
*/
  ULONG file_size_written;
  ULONG filename_idx;
  ULONG filename_idx_first;
  ULONG filename_size;
  FILE *handle;
  u8 status;

  filename_size=(ULONG)(strlen(filename_base));
  filename_idx=filename_size;
  filename_idx_first=ULONG_MAX;
  status=1;
  if(filename_idx&&(filename_base[filename_idx-1]!=FILESYS_PATH_SEPARATOR)){
    filename_base[filename_idx]=0;
    if(!append_status){
      handle=fopen(filename_base, "wb");
    }else{
      handle=fopen(filename_base, "ab");
    }
    if(!handle){
      do{
        while(filename_idx&&(filename_base[filename_idx-1]!=FILESYS_PATH_SEPARATOR)){
          filename_idx--;
        }
        if(!filename_idx){
          break;
        }
        while(filename_idx&&(filename_base[filename_idx-1]==FILESYS_PATH_SEPARATOR)){
          filename_idx--;
          if(filename_idx_first==ULONG_MAX){
            filename_idx_first=filename_idx;
          }
          filename_base[filename_idx]=0;
        }
        if(!filename_idx){
          break;
        }
        #ifndef WINDOWS
          status=!!mkdir(filename_base, S_IRWXU);
        #else
          status=!!mkdir(filename_base);
        #endif
      }while(status);
      if(!status){
/*
Restore the (FILESYS_PATH_SEPARATOR)s one by one, attempting to create a new folder each time. Ignore failures because success is going to be determined by whether or not the fopen() below succeeds.
*/
        while(filename_idx!=filename_idx_first){
          if(!filename_base[filename_idx]){
            filename_base[filename_idx]=FILESYS_PATH_SEPARATOR;
            #ifndef WINDOWS
              mkdir(filename_base, S_IRWXU);
            #else
              mkdir(filename_base);
            #endif
          }
          filename_idx++;
        }
        filename_base[filename_idx_first]=FILESYS_PATH_SEPARATOR;
        if(!append_status){
          handle=fopen(filename_base, "wb");
        }else{
          handle=fopen(filename_base, "ab");
        }
      }	
    }
    if(handle){
      status=0;
      if(file_size){
        file_size_written=(ULONG)(fwrite(void_list_base, (size_t)(U8_SIZE), (size_t)(file_size), handle));
        if(file_size!=file_size_written){
          status=1;
        }
      }
      if(fclose(handle)){
        status=1;
      }
    }
  }
/*
Restore *filename_base to its input state.
*/
  while(filename_idx!=filename_size){
    if(!filename_base[filename_idx]){
      filename_base[filename_idx]=FILESYS_PATH_SEPARATOR;
    }
    filename_idx++;
  }
  if(status){
    status=FILESYS_STATUS_WRITE_FAIL;
  }
  return status;
}

ULONG
filesys_filename_isolate(ULONG *filename_idx_min_base, char *filename_list_base){
/*
Return the base index and size of a filename at the end of a full path.

In:

  *filename_idx_min_base is the base index of the full path to a file, within filename_list_base.

  *filename_list_base is a concatenation of null-terminated filenames.

Out:

  Returns the size of the filename alone, excluding its terminating null.

  *filename_idx_min_base is the base of the filename. If the alleged filename ends in FILESYS_PATH_SEPARATOR, or is null, then zero will be returned. In any event, null will be the last character, at index (return value).
*/
  ULONG filename_idx;
  ULONG filename_idx_min;
  ULONG filename_size;
  u8 status;

  filename_idx_min=*filename_idx_min_base;
  filename_size=(ULONG)(strlen(&filename_list_base[filename_idx_min]));
  if(filename_size){
    filename_idx=filename_idx_min+filename_size;
    do{
      filename_idx--;
      status=(filename_list_base[filename_idx]!=FILESYS_PATH_SEPARATOR);
    }while((filename_idx!=filename_idx_min)&&status);
    filename_idx+=!status;
    filename_size-=filename_idx-filename_idx_min;
    filename_idx_min=filename_idx;
  }
  *filename_idx_min_base=filename_idx_min;
  return filename_size;
}

u8
filesys_filename_list_get(u8 *fatal_status_base, ULONG *file_size_max_base, ULONG *file_size_sum_base, u8 *file_status_base, ULONG *filename_count_base, char *filename_list_base, ULONG *filename_list_size_max_base, char *target_base){
/*
Given a file, return the name of that file if it exists. Given a folder, return the relative path and name of every file which it contains, ignoring links, block devices, character devices, and sockets.

In:

  *fatal_status_base is undefined.

  *file_size_max_base is undefined.

  *file_size_sum_base is undefined.

  *file_status_base is undefined.

  *filename_count_base is undefined.

  *filename_list_base is undefined and writable for (*filename_list_size_max_base+1) bytes.

  *filename_list_size_max_base is the maximum size which may be written to *filename_list_base.

  *target_base is the file or folder to find, and in the latter case, spider. Both local and global paths are allowed.

Out:

  Returns zero on success, else one. Zero merely means that the process completed without encountering an error return from memory allocation or OS function calls; *filename_count_list_base may still be zero. This value shall be one if *fatal_status_base is one.

  *fatal_status_base is zero if the return value is zero, or if retrying this function with a larger *filename_list_size_max_base might result in a return value of zero. Otherwise a fatal error occurred, which includes the following cases without limitation: (1) In:*filename_list_size_max_base was ULONG_MAX but was nevertheless found to be insufficient; (2) the deepest nested folder exceeds lexical level FILESYS_DIRECTORY_DEPTH_IDX_MAX; and (3) if any discovered full pathname, including *target_base itself, exceeds size (FILESYS_PATHNAME_CHAR_IDX_MAX+1).

  *file_size_max_base is the maximum file size encountered, which is valid even if the return value is one.

  *file_size_sum_base is the sum of all file sizes, which is valid even if the return value is one. ULONG_MAX indicates that the result is at least that much.

  *file_status_base is one if *target_base is a file or the return value is zero, else zero if it's a folder; and likewise for equivalent links.

  *filename_count_base is the number of items at *filename_list_base, all of which being null-terminated. This value may be may be zero even if the return value is zero, indicating that *target_base was not found; or nonzero even if the return value is one, indicating partial but valid results at *filename_list_base.

  *filename_list_base is just a copy of *target_base if it's a file that exists, else a concatenation of (return value) null-terminated relative paths and filenames subordinate to *target_base. Points to a null character if the return value is zero.

  *filename_list_size_max_base is at least the defined size at filename_list_base. It has been inflated by filesys_hull_size_get() in order to prevent trivial changes to the folder tree from indefinitely delaying the successful completion of this process; it should complete successfully within O(log(N)) attempts -- and usually just one -- given N files. This value will be nonzero even if the return value is zero.
*/
  u8 continue_status;
  DIR *dir_handle;
  DIR *dir_handle_new;
  DIR **dir_handle_list_base;
  struct dirent *dirent_base;
  ULONG dirent_idx;
  char *dirname_base;
  ULONG dirname_size;
  u8 fatal_status;
  ULONG file_size;
  ULONG file_size_max;
  ULONG file_size_sum;
  u64 file_size_u64;
  char *filename_base;
  ULONG filename_count;
  ULONG filename_list_size;
  ULONG filename_list_size_max;
  ULONG filename_list_size_new;
  #ifndef WINDOWS
    mode_t filename_mode;
  #endif
  ULONG filename_size;
  ULONG filename_size_old;
  ULONG *filename_size_list_base;
  #ifndef WINDOWS
    struct stat filename_stat;
  #endif
  u8 rollback_status;
  u8 status;

  filename_list_size_max=*filename_list_size_max_base;
  file_size=0;
  file_size_max=0;
  file_size_sum=0;
  file_size_u64=0;
  filename_count=0;
  filename_list_size=0;
  *file_status_base=1;
  filename_size=(ULONG)(strlen(target_base));
  while((1<filename_size)&&(target_base[filename_size-1]==FILESYS_PATH_SEPARATOR)){
    filename_size--;
  }
  fatal_status=0;
  status=0;
  filename_list_base[0]=0;
  target_base[filename_size]=0;
  if(filename_size&&(filename_size<=(FILESYS_PATHNAME_CHAR_IDX_MAX+1))){
    filename_size++;
    dir_handle_list_base=DEBUG_MALLOC_PARANOID((FILESYS_DIRECTORY_DEPTH_IDX_MAX+1)*(ULONG)(sizeof(DIR *)));
    filename_base=filesys_char_list_malloc(FILESYS_PATHNAME_CHAR_IDX_MAX+1);
    filename_size_list_base=DEBUG_MALLOC_PARANOID((FILESYS_DIRECTORY_DEPTH_IDX_MAX+1)<<ULONG_SIZE_LOG2);
    if(dir_handle_list_base&&filename_base&&filename_size_list_base){
      strcpy(filename_base, target_base);
      continue_status=1;
      dir_handle=NULL;
      dirent_idx=0;
      dirname_size=0;
      filename_size_old=filename_size;
      rollback_status=0;
      do{
        if(!rollback_status){
/*
Ignore everything but files and folders -- even links to them -- because we don't want duplicate analysis and definitely don't want infinite recursion. We have to use lstat() instead of just inspecting dirent_base->d_type because DT_LNK is not consistently reported by Ubuntu. But lstat() doesn't work in Windows under MinGW; fortunately, in that case, links (shortcuts) appear to operate like files rather than folders, and therefore carry no risk of self-referential loops, even though they could result in unavoidable duplicate analysis.
*/
          #ifndef WINDOWS
            if(!lstat(filename_base, &filename_stat)){
              filename_mode=filename_stat.st_mode;
              if(!(S_ISDIR(filename_mode)||S_ISREG(filename_mode))){
                rollback_status=1;
              }
              if(S_ISBLK(filename_mode)||S_ISCHR(filename_mode)||S_ISFIFO(filename_mode)||S_ISLNK(filename_mode)||S_ISSOCK(filename_mode)){
                rollback_status=1;
              }
            }else{
              fatal_status=1;
              rollback_status=fatal_status;
              status=fatal_status;
            }
          #endif
        }
        if(!rollback_status){
          dir_handle_new=opendir(filename_base);
          if(!dir_handle_new){
            #ifndef WINDOWS
              file_size_u64=(u64)(filename_stat.st_size);
            #else
              rollback_status=!!filesys_file_size_get(&file_size_u64, filename_base);
              if(rollback_status){
                fatal_status=rollback_status;
                status=rollback_status;
              }
            #endif
            file_size=(ULONG)(file_size_u64);
            #ifdef _32_
              if(file_size!=file_size_u64){
                fatal_status=1;
                rollback_status=fatal_status;
                status=fatal_status;
              }
            #endif
            if(!rollback_status){
              filename_list_size_new=filename_list_size+filename_size;
              if(filename_list_size<filename_list_size_new){
                if(filename_list_size_new<=filename_list_size_max){
                  filename_count++;
                  file_size_max=MAX(file_size, file_size_max);
                  if(~file_size_sum){
                    file_size_sum+=file_size;
                    if(file_size_sum<file_size){
                      file_size_sum=ULONG_MAX;
                    }
                  }
                  memcpy(&filename_list_base[filename_list_size], filename_base, (size_t)(filename_size));
                  filename_list_size=filename_list_size_new;
                }else{
                  fatal_status=!~filename_list_size_max;
                  status=1;
                }
              }else{
                fatal_status=1;
                status=fatal_status;
              }
              rollback_status=1;
            }
          }else{
            if(dirent_idx<=FILESYS_DIRECTORY_DEPTH_IDX_MAX){
              dir_handle_list_base[dirent_idx]=dir_handle;
              dir_handle=dir_handle_new;
              filename_size_list_base[dirent_idx]=filename_size_old;
              dirent_idx++;
              filename_size_old=filename_size;
              rollback_status=0;
            }else{
              closedir(dir_handle_new);
              fatal_status=1;
              rollback_status=fatal_status;
              status=fatal_status;
            }
          }
        }
        if(rollback_status){
          continue_status=!!dirent_idx;
          filename_size=filename_size_old;
          filename_base[filename_size_old-1]=0;
        }
        if(continue_status){
          do{
            continue_status=0;
            dirent_base=readdir(dir_handle);
            if(!dirent_base){
              break;
            }
            continue_status=1;
            dirname_base=&dirent_base->d_name[0];
            dirname_size=(ULONG)(strlen(dirname_base)+1);
            rollback_status=0;
            filename_size=filename_size_old+dirname_size;
            if((FILESYS_PATHNAME_CHAR_IDX_MAX+1)<filename_size){
              fatal_status=1;
              rollback_status=fatal_status;
              status=fatal_status;
            }else if(dirname_size<=3){
              if(1<dirname_size){
                if(dirname_base[0]=='.'){
                  if((dirname_size==2)||(dirname_base[1]=='.')){
                    rollback_status=1;
                  }
                }
              }else{
                rollback_status=1;
              }
            }
          }while(rollback_status);
        }
        if(continue_status){
          filename_size=filename_size_old;
          if(filename_base[filename_size_old-2]==FILESYS_PATH_SEPARATOR){
            filename_size--;
          }
          filename_base[filename_size-1]=FILESYS_PATH_SEPARATOR;
          memcpy(&filename_base[filename_size], dirname_base, (size_t)(dirname_size));
          filename_size+=dirname_size;
        }else{
          if(dir_handle){
            closedir(dir_handle);
          }
          if(dirent_idx){
            dirent_idx--;
            dir_handle=dir_handle_list_base[dirent_idx];
            filename_size_old=filename_size_list_base[dirent_idx];
            rollback_status=1;
            if(dirent_idx){
              continue_status=1;
            }else{
              *file_status_base=0;
            }
          }
        }
      }while(continue_status);
    }
    DEBUG_FREE_PARANOID(filename_size_list_base);
    filesys_free(filename_base);
    DEBUG_FREE_PARANOID(dir_handle_list_base);
  }
  *fatal_status_base=fatal_status;
  *file_size_max_base=file_size_max;
  *file_size_sum_base=file_size_sum;
  *filename_count_base=filename_count;
  *filename_list_size_max_base=filesys_hull_size_get(filename_list_size_max);
  return status;
}

void
filesys_filename_list_morph(ULONG filename_count, char *source_base, char *source_filename_list_base, char *target_base, char *target_filename_list_base){
/*
Morph all the paths in filename list, but keep the isolated filenames identical.

In:

  filename_count is the return value of filesys_filename_list_get().

  *source_base is filesys_filename_list_get():In:*target_base.

  *source_filename_list_base is filesys_filename_list_get():Out:*filename_list_base.

  *target_base is analogous to *source_base, and is also null-terminated. It indicates the target folder which is to replace the source folder in all pathnames within *source_filename_list_base.

  *target_filename_list_base is undefined and writable for the size given by the return value of filesys_filename_list_morph_size_get().

Out:

  *target_filename_list_base mirrors *source_filename_list_base, except that path prefixes equivalent to *source_base have been replaced by *target_base.
*/
  ULONG source_filename_idx_min;
  ULONG source_filename_size;
  ULONG source_size;
  ULONG target_filename_idx_min;
  ULONG target_size;

  source_size=(ULONG)(strlen(source_base));
  while((1<source_size)&&(source_base[source_size-1]==FILESYS_PATH_SEPARATOR)){
    source_size--;
  }
  target_size=(ULONG)(strlen(target_base));
  while((1<target_size)&&(target_base[target_size-1]==FILESYS_PATH_SEPARATOR)){
    target_size--;
  }
  source_filename_idx_min=0;
  target_filename_idx_min=0;
  while(filename_count--){
    source_filename_idx_min+=source_size;
    while(source_filename_list_base[source_filename_idx_min]==FILESYS_PATH_SEPARATOR){
      source_filename_idx_min++;
    }    
    source_filename_size=(ULONG)(strlen(&source_filename_list_base[source_filename_idx_min]));
    memcpy(&target_filename_list_base[target_filename_idx_min], target_base, (size_t)(target_size));
    target_filename_idx_min+=target_size;
    if(source_filename_size&&target_size){
      target_filename_list_base[target_filename_idx_min]=FILESYS_PATH_SEPARATOR;
      target_filename_idx_min++;
    }
    source_filename_size++;
    memcpy(&target_filename_list_base[target_filename_idx_min], &source_filename_list_base[source_filename_idx_min], (size_t)(source_filename_size));
    source_filename_idx_min+=source_filename_size;
    target_filename_idx_min+=source_filename_size;
  }
  return;
}

ULONG
filesys_filename_list_morph_size_get(ULONG filename_count, char *source_base, char *source_filename_list_base, char *target_base){
/*
Get the size required by *target_filename_list_base in filesys_filename_list_morph().

In:

  filename_count is the return value of filesys_filename_list_get().

  *source_base is filesys_filename_list_get():In:*target_base.

  *source_filename_list_base is filesys_filename_list_get():Out:*filename_list_base.

  *target_base is analogous to *source_base, and is also null-terminated. It indicates the target folder which is to replace the source folder in all pathnames within *source_filename_list_base.

Out:

  Returns the maximum size required to store filesys_filename_list_morph():Out:*target_filename_list_base.
*/
  ULONG filename_size;
  ULONG filename_list_size;
  ULONG filename_list_size_delta;
  ULONG source_size;
  ULONG source_target_size_delta;
  ULONG target_size;
  u8 status;

  filename_list_size=0;
  if(filename_count){
/*
For the sake of consistency, filesys_filename_list_morph() only replaces text up to but excluding any tail (FILESYS_PATH_SEPARATOR)s, so ignore any such tail in the source and the target for the sake of size computation.
*/
    source_size=(ULONG)(strlen(source_base));
    while((1<source_size)&&(source_base[source_size-1]==FILESYS_PATH_SEPARATOR)){
      source_size--;
    }
    target_size=(ULONG)(strlen(target_base));
    while((1<target_size)&&(target_base[target_size-1]==FILESYS_PATH_SEPARATOR)){
      target_size--;
    }
    status=1;
/*
Account for the case in which we need an extra FILESYS_PATH_SEPARATOR for each target pathname but not the corresponding source pathname. (This might actually be impossible, but better safe than sorry.)
*/
    target_size++;
    if(target_size){
      source_target_size_delta=source_size-target_size;
      if(source_size<=target_size){
        source_target_size_delta=0U-source_target_size_delta;
      }
      filename_list_size_delta=filename_count*source_target_size_delta;
      if((filename_list_size_delta/filename_count)==source_target_size_delta){
        do{
          filename_size=(ULONG)(strlen(&source_filename_list_base[filename_list_size]));
          filename_list_size+=filename_size+1;
        }while(--filename_count);
        if(source_size<=target_size){
          filename_list_size+=filename_list_size_delta;
          status=(filename_list_size<filename_list_size_delta);
        }else{
          status=(filename_list_size<filename_list_size_delta);
          filename_list_size-=filename_list_size_delta;
        }
      }
    }
    if(status){
      filename_list_size=0;
    }
  }
  return filename_list_size;
}

u8
filesys_filename_list_sort(ULONG filename_count, char *filename_list_base){
/*
Sort a list of ilenames.

In:

  filename_count is number of filenames at filename_list_base.

  filename_list_base contains a concatendated list of null-terminated filenames or pathnames.

Out:

  Returns zero on success, else one.

  *filename_list_base contains the same concatendated strings, but sorted ascending.
*/
  ULONG byte_idx;
  ULONG file_idx_idx_idx;
  ULONG file_idx_idx_list_base[U8_SPAN+1U];
  ULONG file_idx_idx0;
  ULONG file_idx_idx1;
  ULONG *file_idx_list_base0;
  ULONG *file_idx_list_base1;
  ULONG *file_idx_list_base2;
  ULONG file_idx;
  ULONG *filename_char_idx_list_base;
  char *filename_list_base0;
  char *filename_list_base1;
  ULONG filename_list_char_idx0;
  ULONG filename_list_char_idx1;
  ULONG filename_size;
  ULONG *filename_size_list_base;
  ULONG filename_size_max;
  ULONG filename_size_min;
  ULONG list_size;
  u8 status;

  status=!filename_count;
  if(1U<filename_count){
    list_size=filename_count<<ULONG_SIZE_LOG2;
    file_idx_list_base0=DEBUG_MALLOC_PARANOID(list_size);
    status=!file_idx_list_base0;
    file_idx_list_base1=DEBUG_MALLOC_PARANOID(list_size);
    status=(u8)(status|!file_idx_list_base1);
    filename_char_idx_list_base=DEBUG_MALLOC_PARANOID(list_size);
    status=(u8)(status|!filename_char_idx_list_base);
    filename_size_list_base=DEBUG_MALLOC_PARANOID(list_size);
    status=(u8)(status|!filename_size_list_base);
    if(!status){
      file_idx=0;
      filename_list_base0=filename_list_base;
      filename_list_char_idx0=0;
      filename_size_max=0;
      filename_size_min=ULONG_MAX;
      status=1;
      do{
        filename_char_idx_list_base[file_idx]=filename_list_char_idx0;
        filename_size=(ULONG)(strlen(&filename_list_base0[filename_list_char_idx0]));
        filename_list_char_idx0+=filename_size+1U;
        filename_size_max=MAX(filename_size, filename_size_max);
        filename_size_min=MIN(filename_size, filename_size_min);
        filename_size_list_base[file_idx]=filename_size;
        file_idx++;
      }while(file_idx!=filename_count);
      filename_list_base1=DEBUG_MALLOC_PARANOID(filename_list_char_idx0);
      status=(!filename_list_base1)|!filename_size_min;
      if(!status){
        file_idx_idx0=0;
        list_size=(U8_SPAN+1U)<<ULONG_SIZE_LOG2;
        do{
          file_idx_list_base0[file_idx_idx0]=file_idx_idx0;
          file_idx_idx0++;
        }while(file_idx_idx0!=filename_count);
        byte_idx=filename_size_max-1U;
        do{
          memset(file_idx_idx_list_base, 0, (size_t)(list_size));
          file_idx=0;
          do{
            filename_size=filename_size_list_base[file_idx];
            file_idx_idx_idx=0;
            if(byte_idx<filename_size){
              filename_list_char_idx0=filename_char_idx_list_base[file_idx];
              file_idx_idx_idx=(u8)(filename_list_base0[filename_list_char_idx0+byte_idx]);
              file_idx_idx_idx++;
            }
            file_idx_idx_list_base[file_idx_idx_idx]++;
            file_idx++;
          }while(file_idx!=filename_count);
          file_idx_idx_idx=0;
          file_idx_idx0=0;
          do{
            file_idx_idx1=file_idx_idx_list_base[file_idx_idx_idx];
            file_idx_idx_list_base[file_idx_idx_idx]=file_idx_idx0;
            file_idx_idx0+=file_idx_idx1;
          }while((file_idx_idx_idx++)<=U8_MAX);
          file_idx_idx0=0;
          do{
            file_idx=file_idx_list_base0[file_idx_idx0];
            filename_size=filename_size_list_base[file_idx];
            file_idx_idx_idx=0;
            if(byte_idx<filename_size){
              filename_list_char_idx0=filename_char_idx_list_base[file_idx];
              file_idx_idx_idx=(u8)(filename_list_base0[filename_list_char_idx0+byte_idx]);
              file_idx_idx_idx++;
            }
            file_idx_idx1=file_idx_idx_list_base[file_idx_idx_idx];
            file_idx_list_base1[file_idx_idx1]=file_idx;
            file_idx_idx1++;
            file_idx_idx_list_base[file_idx_idx_idx]=file_idx_idx1;
            file_idx_idx0++;
          }while(file_idx_idx0!=filename_count);
          file_idx_list_base2=file_idx_list_base0;
          file_idx_list_base0=file_idx_list_base1;
          file_idx_list_base1=file_idx_list_base2;
        }while(byte_idx--);
        file_idx_idx0=0;
        filename_list_char_idx1=0;
        do{
          file_idx=file_idx_list_base0[file_idx_idx0];
          filename_list_char_idx0=filename_char_idx_list_base[file_idx];
          filename_size=filename_size_list_base[file_idx];
          filename_size++;
          memcpy(&filename_list_base1[filename_list_char_idx1], &filename_list_base0[filename_list_char_idx0], (size_t)(filename_size));
          filename_list_char_idx1+=filename_size;
          file_idx_idx0++;
        }while(file_idx_idx0!=filename_count);
        memcpy(filename_list_base0, filename_list_base1, (size_t)(filename_list_char_idx1));
      }
      DEBUG_FREE_PARANOID(filename_list_base1);
    }
    DEBUG_FREE_PARANOID(filename_size_list_base);
    DEBUG_FREE_PARANOID(filename_char_idx_list_base);
    DEBUG_FREE_PARANOID(file_idx_list_base1);
    DEBUG_FREE_PARANOID(file_idx_list_base0);
  }
  return status;
}

void *
filesys_free(void *base){
/*
To maximize portability and debuggability, this is the only function in which Filesys calls free().

In:

  base is the base of a memory region to free. May be NULL.

Out:

  Returns NULL so that the caller can easily maintain the good practice of NULLing out invalid pointers.

  *base is freed.
*/
  DEBUG_FREE_PARANOID(base);
  return NULL;
}

ULONG
filesys_hull_size_get(ULONG size_projected){
/*
Compute MAX(MIN(4, (2^ceil(log2(size_projected+1))), ULONG_MAX). This enables exponential backoff of projected allocation needs, which will terminate in tractable time because going slightly above a power of 2 will result in approximately double the allocation size required. The lower bound of 4 isn't magical; it's just convenient and not worth optimizing down.

In:

  size_projected is the size anticipated to be needed in a future allocation, with no added margin.

Out:

  Returns the size described in the summary, in order to minimize the incidence of insufficient allocation, yet without wasting too much memory.
*/
  ULONG hull_size;

  hull_size=1;
  while(size_projected){
    size_projected>>=1;
    hull_size<<=1;
  }
  if(hull_size<4){
    if(hull_size){
      hull_size=4;
    }else{
      hull_size=~hull_size;
    }
  }
  return hull_size;
}

u8
filesys_init(u32 build_break_count, u32 build_feature_count){
/*
Verify that the source code is sufficiently updated.

In:

  build_break_count is the caller's most recent knowledge of FILESYS_BUILD_BREAK_COUNT, which will fail if the caller is unaware of all critical updates.

  build_feature_count is the caller's most recent knowledge of FILESYS_BUILD_FEATURE_COUNT, which will fail if this library is not up to date with the caller's expectations.

Out:

  Returns one if (build_break_count!=FILESYS_BUILD_BREAK_COUNT) or (build_feature_count>FILESYS_BUILD_FEATURE_COUNT). Otherwise, returns zero.
*/
  u8 status;

  status=(u8)(build_break_count!=FILESYS_BUILD_BREAK_COUNT);
  status=(u8)(status|(FILESYS_BUILD_FEATURE_COUNT<build_feature_count));
  return status;
}

u8
filesys_subfile_read(u8 direction_status, char *filename_base, ULONG read_size, u64 read_u8_idx_min, void *void_list_base){
/*
Read a contiguous subset of a file into a buffer.

In:

  direction_status is zero if read_u8_idx_min is relative to the base of the file, else one if read_u8_idx_min is relative to the base of the bytewise reverse of the file.

  read_size is the size to read. For the sake of consistent behavior, a value of zero will still hit the OS in the same manner as with a nonzero value.

  read_u8_idx_min is the zero-based byte offset at which to begin reading, subject to the reversal imposed by direction_status. So for example if this value is 5, read_size is 7, and direction_status is one, then the first 7 of the last 12 bytes of the file will be returned in the same order in which they appear in the file.

  *void_list_base is undefined and writable for read_size bytes.

Out:

  Returns zero on success, else (1) FILESYS_STATUS_NOT_FOUND if the file could not be opened, or (2) FILESYS_STATUS_READ_FAIL otherwise.

  *void_list_base contains the read_size of the file, based at byte offset read_u8_idx_min. On failure, the same region may have changed. This is a noncanonical output, in the sense that it may have been updated regardless of whether or not the return value indicates an error condition.
*/
  ULONG filename_idx;
  u8 status;

  filename_idx=0;
  status=filesys_subfile_read_next(direction_status, &filename_idx, filename_base, read_size, read_u8_idx_min, void_list_base);
  return status;
}

u8
filesys_subfile_read_next(u8 direction_status, ULONG *filename_idx_base, char *filename_list_base, ULONG read_size, u64 read_u8_idx_min, void *void_list_base){
/*
Read a contiguous subset of a file specified in a filename list, into a buffer.

In:

  direction_status is as defined in filesys_subfile_read():In.

  *filename_idx_base is the base index of the filename at filename_list_base.

  *filename_list_base is a concatenation of null-terminated filenames.

  read_size is as defined in filesys_subfile_read():In.

  read_u8_idx_min is as defined in filesys_subfile_read():In.

  *void_list_base is as defined in filesys_subfile_read():In.

Out:

  Returns as defined in filesys_subfile_read().

  *filename_idx_base is the base index of the next filename at filename_list_base, or else the postterminal index of filename_list_base. This is a noncanonical output, in the sense that it's valid regardless of whether or not the return value indicates an error condition.

  *void_list_base is as defined in filesys_subfile_read():Out.
*/
  ULONG file_size_read;
  ULONG filename_idx;
  ULONG filename_size;
  FILE *handle;
  i64 read_u8_idx_min_i64;
  u8 status;
  int whence;

  filename_idx=*filename_idx_base;
  filename_size=(ULONG)(strlen(&filename_list_base[filename_idx]));
  *filename_idx_base=filename_idx+filename_size+1;
  handle=fopen(&filename_list_base[filename_idx], "rb");
  status=FILESYS_STATUS_NOT_FOUND;
  if(handle){
    status=FILESYS_STATUS_READ_FAIL;
    if(!((read_size>>ULONG_BIT_MAX)|(read_u8_idx_min>>U64_BIT_MAX)|((read_u8_idx_min+read_size)>>U64_BIT_MAX))){
      if(direction_status){
        read_u8_idx_min_i64=0L-(i64)(read_u8_idx_min+read_size);
        whence=SEEK_END;
      }else{
        read_u8_idx_min_i64=(i64)(read_u8_idx_min);
        whence=SEEK_SET;
      }
      if((!read_u8_idx_min_i64)&&(whence==SEEK_SET)){
        status=0;
      }else if(!FILESYS_FSEEKO(handle, (off_t)(read_u8_idx_min_i64), whence)){
        status=0;
      }
    }
    if(!status){
      status=FILESYS_STATUS_READ_FAIL;
      file_size_read=(ULONG)(fread(void_list_base, (size_t)(U8_SIZE), (size_t)(read_size), handle));
      if(file_size_read==read_size){
        status=0;
      }
    }
    fclose(handle);
  }
  return status;
}

u8
filesys_subfile_write(u8 direction_status, char *filename_list_base, ULONG write_size, u64 write_u8_idx_min, void *void_list_base){
/*
Write a buffer to a contiguous subset of a file.

In:

  direction_status is zero if write_u8_idx_min is relative to the base of the file, else one if write_u8_idx_min is relative to the base of the bytewise reverse of the file.

  *filename_list_base is a null-terminated filename.

  write_size is the size to write. For the sake of consistent behavior, a value of zero will still hit the OS in the same manner as with a nonzero value.

  write_u8_idx_min is the zero-based byte offset at which to begin writing, subject to the reversal imposed by direction_status. So for example if this value is 5, write_size is 7, and direction_status is one, then the first 7 of the last 12 bytes of the file will be written in the same order in which they appear in memory.

  *void_list_base contains write_size bytes to write.

Out:

  Returns zero on success, else (1) FILESYS_STATUS_NOT_FOUND if the file could not be opened, or (2) FILESYS_STATUS_WRITE_FAIL otherwise.
*/
  ULONG file_size_written;
  FILE *handle;
  u8 status;
  int whence;
  i64 write_u8_idx_min_i64;

  handle=fopen(filename_list_base, "r+b");
  status=FILESYS_STATUS_NOT_FOUND;
  if(handle){
    status=FILESYS_STATUS_WRITE_FAIL;
    if(!((write_size>>ULONG_BIT_MAX)|(write_u8_idx_min>>U64_BIT_MAX)|((write_u8_idx_min+write_size)>>U64_BIT_MAX))){
      if(direction_status){
        write_u8_idx_min_i64=0L-(i64)(write_u8_idx_min+write_size);
        whence=SEEK_END;
      }else{
        write_u8_idx_min_i64=(i64)(write_u8_idx_min);
        whence=SEEK_SET;
      }
      if(!FILESYS_FSEEKO(handle, (off_t)(write_u8_idx_min_i64), whence)){
        status=0;
      }
    }
    if(!status){
      status=FILESYS_STATUS_WRITE_FAIL;
      file_size_written=(ULONG)(fwrite(void_list_base, (size_t)(U8_SIZE), (size_t)(write_size), handle));
      if(file_size_written==write_size){
        status=0;
      }
    }
    fclose(handle);
  }
  return status;
}
