#ifndef CHECKPOINTING_RESTORE_H
#define CHECKPOINTING_RESTORE_H

#include "Checkpointing/Namespace.h"
/*
 * Checkpointing namespace declaration
 */
namespace CHECKPOINTING_NAMESPACE  {

  struct FileDesc;

  /// Print file descriptor information
  STATIC(void) checkpoint_file_print(int lvl, const FileDesc* d);
  /// Reopen file descriptor for process restoration
  STATIC(int) checkpoint_file_reopen(FileDesc* d);
  /// Read file descriptor information from file
  STATIC(int) checkpoint_file_fread(FileDesc* d, int fd, bool restore);
  /// Read file descriptor information from memory block
  STATIC(int) checkpoint_file_read(FileDesc* d, const void* addr, bool restore);

  struct Process;
  /// Read process descriptor header from memory
  STATIC(int) checkpointing_process_read_header(Process*, const void* addr);
  /// Read process descriptor trailer from memory
  STATIC(int) checkpointing_process_read_trailer(Process*, const void* addr);
  /// Read process descriptor header from file
  STATIC(int) checkpointing_process_fread_header(Process*,int fd);
  /// Read process descriptor trailer from file
  STATIC(int) checkpointing_process_fread_trailer(Process*,int fd);
  STATIC(int) checkpointing_process_skip_sys(Process*,const void* addr);
  /// Read full file descriptor information from memory
  STATIC(int) checkpointing_process_read_files(Process*,const void* addr);

  STATIC(int) checkpointing_process_read_memory(Process*, const void* addr);
  STATIC(int) checkpointing_process_read(Process* p, const void* addr);

  STATIC(int) checkpointing_process_fskip_sys(Process*,int fd);
  STATIC(int) checkpointing_process_fread_files(Process*,int fd);
  STATIC(int) checkpointing_process_fread_memory(Process*,int fd);
  STATIC(int) checkpointing_process_fread(Process* p, int fd);

  struct AreaHandler;
  struct Area;
  STATIC(int) checkpointing_process_map_memory(const AreaHandler* ,const Area& a, const unsigned char* data, int data_len);
  STATIC(int) checkpointing_process_fmap_memory(const AreaHandler* ,const Area& a, int fd, int data_len);
  STATIC(int) checkpointing_area_datalength(const Area* a);
  STATIC(void) checkpointing_area_print(const Area* a,int lvl,const char* opt);
  STATIC(int) checkpointing_area_read(Area* a, const void* addr, int (*map)(const AreaHandler*, const Area& a,const unsigned char*,int), const AreaHandler* handler);

  STATIC(int) checkpointing_area_fread(Area* a,int fd, int (*map)(const AreaHandler*, const Area& a,int fd,int), const AreaHandler* handler);
  STATIC(int) checkpointing_area_protection(const Area* a);
  STATIC(int) checkpointing_area_mapFlags(const Area* a);

  STATIC(int) checkpointing_area_map(const Area& a,int fd_in,const unsigned char* in,int data_len);

  struct SysInfo;
  typedef void (*checkpointing_string_handler_t)(void* param, const char* s);

  /// Print data content of SysInfo structure
  STATIC(void) checkpointing_sys_print(const SysInfo& s);
  /// Main restart routine in checkpointing image
  STATIC(void) checkpointing_sys_restore_start(SysInfo* sys,int print_level, int optional_flags);
  /// Secondary restore routine. Execution starts once we jumped to the local stack.
  STATIC(void) checkpointing_sys_restore_process();
  /// Final restart routine. Execution starts once we are back on the stack of the restored process.
  STATIC(void) checkpointing_sys_restore_finish();
  /// Handle input file to set environment etc.
  STATIC(void) checkpointing_sys_process_file(int fd, void* par, checkpointing_string_handler_t handler);

  // CHECKPOINTING_NAMESPACE::
}
#endif  // CHECKPOINTING_RESTORE_H
