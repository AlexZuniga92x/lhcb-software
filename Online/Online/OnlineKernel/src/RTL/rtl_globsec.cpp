#define RTL_IMPLEMENTATION
#include <map>
#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <string>
#include "rtl_internal.h"
#include "fcntl.h"

#if defined(linux)
#include "unistd.h"
#include <sys/mman.h>
#elif defined(_WIN32)
#include <windows.h>
#endif

namespace {
  struct GblLock  {
    GblLock() {}
    int status() const { return 1; }
    operator bool()  const { return true; }
  };
}


/// Create named global section
int lib_rtl_create_section(const char* sec_name, size_t size, lib_rtl_gbl_t* address) {
  long siz  = (size/4096)*4096 + (((size%4096)==0) ? 0 : 4096);  //  multiple of page size
  std::auto_ptr<lib_rtl_gbl_desc> h(new lib_rtl_gbl_desc);
  sprintf(h->name,"/%s",sec_name);
  h->addaux = h.get();
  h->size = siz;
  //::printf("Create global section %s of size:%d\n",h->name, h->size);
#if defined(linux)
  int sysprot  = PROT_READ+PROT_WRITE;
  int sysflags = MAP_SHARED;
  h->fd = ::shm_open(h->name,O_RDWR|O_CREAT|O_EXCL,0644);
  if ( h->fd ) {
    ::ftruncate(h->fd, h->size);
    h->address = ::mmap (0, h->size, sysprot, sysflags, h->fd, 0);
    if ( h->address != 0 )  {
      *address = h.release();
      return 1;
    }
  }
  ::shm_unlink(h->name);
#elif defined(_WIN32)
  // Setup inherited security attributes (FIXME: merge somewhere else)
  SECURITY_ATTRIBUTES   sa = {sizeof(SECURITY_ATTRIBUTES), 0, true};
  h->addaux = ::CreateFileMapping(INVALID_HANDLE_VALUE,&sa,PAGE_READWRITE,0,siz,h->name);
  if (h->addaux != 0 && ::GetLastError() == ERROR_ALREADY_EXISTS)   { 
    ::CloseHandle(h->addaux); 
    h->addaux = ::OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,h->name);
  }
  if ( h->addaux )  {
    h->address = ::MapViewOfFile(h->addaux,FILE_MAP_ALL_ACCESS,0,0,siz);
    if ( h->address != 0 )  {
      h->fd = 0;
      *address = h.release();
      return 1;
    }
  }
#endif
  int err = lib_rtl_get_error();
  ::printf("error mapping section [%s]. Status %d [%s]\n",h->name,err,errorString(err));
  return 0;
}

/// Delete named global section
int lib_rtl_delete_section(lib_rtl_gbl_t h)    {
  lib_rtl_gbl_desc* dsc = (lib_rtl_gbl_desc*)h;
  int sc = 0;
  if ( dsc )  {
#ifdef linux
    sc = ::shm_unlink(dsc->name)==0 ? 1 : 0;
    delete dsc;
    if ( sc == 0 ) sc = 1;
#else
    sc = lib_rtl_unmap_section(h);
    delete dsc;
#endif
  }
  return sc;
}

/// Map global section a a specific address
int lib_rtl_map_section(const char* sec_name, size_t size, lib_rtl_gbl_t* address)   {
  long siz  = (size/4096)*4096 + (((size%4096)==0) ? 0 : 4096);  //  multiple of page size
  std::auto_ptr<lib_rtl_gbl_desc> h(new lib_rtl_gbl_desc);
  sprintf(h->name,"/%s",sec_name);
  h->addaux = h.get();
  h->size = siz;
  //::printf("Map global section %s of size:%d\n",h->name, h->size);
#if defined(linux)
  int sysprot  = PROT_READ+PROT_WRITE;
  int sysflags = MAP_SHARED;
  h->fd = ::shm_open(h->name,O_RDWR|O_CREAT|O_EXCL,0644);
  if ( 0 == h->fd )  {
    ::close(h->fd);
    ::shm_unlink(h->name);
    return 0;
  }
  h->fd = ::shm_open(h->name,O_RDWR|O_CREAT,0644);
  if ( h->fd ) {
    ::ftruncate(h->fd, h->size);
  }
  h->address = ::mmap (0, h->size, sysprot, sysflags, h->fd, 0);
  if ( h->address != 0 )  {
    *address = h.release();
    return 1;
  }
  int err = lib_rtl_get_error();
  ::shm_unlink(h->name);
  ::printf("error mapping section [%s]. Status %d [%s]\n",h->name,err,errorString(err));
  return 0;
#elif defined(_WIN32)
  h->addaux = ::OpenFileMapping(FILE_MAP_ALL_ACCESS,FALSE,h->name);
  if ( h->addaux )  {
    h->address = ::MapViewOfFile(h->addaux,FILE_MAP_ALL_ACCESS,0,0,siz);
    if ( h->address != 0 )  {
      h->fd = 0;
      *address = h.release();
      return 1;
    }
    int err = lib_rtl_get_error();
    ::printf("error mapping section [%s]. Status %d [%s]\n",h->name,err,errorString(err));
  }
  return 0;
#endif
}

/// Unmap global section: address is quadword: void*[2]
int lib_rtl_unmap_section(lib_rtl_gbl_t handle)   {
  if ( handle )  {
    lib_rtl_gbl_desc* h = (lib_rtl_gbl_desc*)handle;
#if defined(linux)
    int sc = ::munmap(h->address,h->size)==0 ? 1 : 0;
#elif defined(_WIN32)
    int sc = (::UnmapViewOfFile(h->address) == 0) ? 0 : 1;
    if ( 0 != sc ) ::CloseHandle(h->addaux);
#endif
    return sc;
  }
  return 0;
}

/// Flush global section to disk file
int lib_rtl_flush_section(lib_rtl_gbl_t handle)   {
  if ( handle )  {
    lib_rtl_gbl_desc* h = (lib_rtl_gbl_desc*)handle;
#if defined(_WIN32)
    DWORD sc = ::FlushViewOfFile(h->addaux,h->size);
    if ( sc == 0 )  {
      return 0;
    }
#elif defined(linux)
    ::msync(h->address, h->size, MS_INVALIDATE|MS_SYNC);
#endif
  }
  return 1;
}
