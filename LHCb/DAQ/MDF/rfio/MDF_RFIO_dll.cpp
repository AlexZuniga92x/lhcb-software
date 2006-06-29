// $Header: /afs/cern.ch/project/cvs/reps/lhcb/DAQ/MDF/rfio/MDF_RFIO_dll.cpp,v 1.1 2006-06-29 15:58:35 frankb Exp $
//	====================================================================
//  MDFIO.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#include "MDF/PosixIO.h"

#ifdef _WIN32
#define EXPORT __declspec(dllexport)
#else
#define EXPORT
#endif

extern "C" {
  int   rfio_open(const char *filepath, int flags, int mode);
  int   rfio_close(int s);
  int   rfio_read(int s, void *ptr, int size);
  int   rfio_write(int s, const void *ptr, int size);
  int   rfio_lseek(int s, int offset, int how);
  int   rfio_lseek64(int s, long long int offset, int how);
  int   rfio_access(const char *filepath, int mode);
  int   rfio_unlink(const char *filepath);
  int   rfio_parse(const char *name, char **host, char **path);
  int   rfio_stat(const char *path, struct stat *statbuf);
  int   rfio_stat64(const char *path, struct stat64 *statbuf);
  int   rfio_fstat(int s, struct stat *statbuf);
  int   rfio_fstat64(int s, struct stat64 *statbuf);
  void  rfio_perror(const char *msg);
  char *rfio_serror();
  int   rfiosetopt(int opt, int *pval, int len);
  int   rfio_mkdir(const char *path, int mode);
  void *rfio_opendir(const char *dirpath);
  int   rfio_closedir(void *dirp);
  void *rfio_readdir(void *dirp);
  long  rfio_ftell(void*);
  // long long int rfio_ftell64(void*);
  // long  rfio_tell(int);
  // long long int rfio_tell64(int);
#ifdef _WIN32
  int  *C__serrno(void);
  int  *C__rfio_errno (void);
#endif
};

extern "C" EXPORT LHCb::PosixIO* MDF_RFIO()  {
  static LHCb::PosixIO p;
  if ( 0 == p.open )  {
    p.open      = rfio_open;
    p.close     = rfio_close;
    p.read      = rfio_read;
    p.write     = rfio_write;
    p.lseek     = rfio_lseek;
    p.lseek64   = rfio_lseek64;
    p.access    = rfio_access;
    p.unlink    = rfio_unlink;
    p.stat      = rfio_stat;
    p.stat64    = rfio_stat64;
    p.fstat     = rfio_fstat;
    p.fstat64   = rfio_fstat64;
    p.ftell     = rfio_ftell;
    p.ftell64   = 0; //rfio_ftell64;
    p.setopt    = rfiosetopt;
    p.mkdir     = rfio_mkdir;
    p.opendir   = rfio_opendir;
    p.readdir   = rfio_readdir;
    p.closedir  = rfio_closedir;
    p.serror    = rfio_serror;
  #ifdef _WIN32
    p.serrno    = C__serrno;
    p.ioerrno   = C__rfio_errno;
  #endif
  }
  return &p;
}
