#include "RTL/rtl.h"
#include "RTL/Lock.h"
#include "NET/IOPortManager.h"
#include <map>
#include <vector>
#include <stdexcept>

#ifdef __linux
#include <sys/poll.h>
#include <sys/ioctl.h>
#define ioctlsocket ioctl

#elif _WIN32
#include <conio.h>
#include "RTL/conioex.h"
#include <windows.h>
#include <io.h>
#include <fcntl.h>
#include <process.h>

extern "C" int console_read_test(int, char**)  {
  HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
  DWORD cNumRead, fdwMode, fdwSaveOldMode; 
  INPUT_RECORD irInBuf; 
  if (! GetConsoleMode(hStdin, &fdwSaveOldMode) ) 
    lib_rtl_printf("GetConsoleMode"); 

  // Enable the window and mouse input events. 
  fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT; 
  if (! SetConsoleMode(hStdin, fdwMode) ) 
    lib_rtl_printf("SetConsoleMode"); 
  while (1) {
    // Wait for the events. 
    if (! ReadConsoleInput( 
      hStdin,      // input buffer handle 
      &irInBuf,     // buffer to read into 
      1,         // size of read buffer 
      &cNumRead) ) // number of records read 
      lib_rtl_printf("ReadConsoleInput"); 
    switch(irInBuf.EventType) 
    { 
    case KEY_EVENT: // keyboard input 
      lib_rtl_printf("KEY_EVENT\n");
      break; 

    case MOUSE_EVENT: // mouse input 
      lib_rtl_printf("MOUSE_EVENT\n");
      break; 

    case WINDOW_BUFFER_SIZE_EVENT: // scrn buf. resizing 
      lib_rtl_printf("WINDOW_BUFFER_SIZE_EVENT\n");
      break; 

    case FOCUS_EVENT:  // disregard focus events 
      lib_rtl_printf("FOCUS_EVENT\n");

    case MENU_EVENT:   // disregard menu events 
      lib_rtl_printf("MENU_EVENT\n");
      break; 

    default: 
      lib_rtl_printf("unknown event type"); 
      break; 
    } 
  }
}

extern "C" int console_read_test2(int, char**)  {
  char c=0;
  lib_rtl_printf( "do not forget to execute: 'stty -icanon -echo'" );
  fflush(stdout);
  int fd = fileno(stdin);
  lib_rtl_printf("Get console IO...type q to quit\n\n\n");
  fflush(stdout);
  while(c!='q') {
    read(fd,&c,1);
    lib_rtl_printf("Got char:%02X\n",c);
    fflush(stdout);
  }
  lib_rtl_printf( "do not forget to execute: 'stty icanon echo'" );
  return 1;
}
#endif

namespace {
  struct PortEntry  {
    int type;
    int (*callback)(void*);
    void* param;
    int armed;
  };
  static bool s_unloading = false;
  int ioman_exithandler(void*) {
    s_unloading = true;
    return 1;
  }
  
  class EntryMap : public std::map<__NetworkChannel__,PortEntry*> {
  protected:
    friend class IOPortManager;
    lib_rtl_thread_t m_thread;
    __NetworkPort__  m_port;
    bool             m_dirty;
    void*            m_mutex_id;
  public:
    static int threadCall(void* param);
    static int consoleCall(void* param);
    EntryMap(__NetworkPort__ p);
    ~EntryMap();
    void setDirty() { m_dirty = true; }
    int handle();
    int run();
  };

  int EntryMap::threadCall(void* param)  {
    return ((EntryMap*)param)->handle();
  }
  int s_fdPipe[2], s_fdPipeBytes=0;
  int EntryMap::consoleCall(void* param)  {
    EntryMap* m = (EntryMap*)param;
    char ch;
    bool xterm = ::getenv("TERM") != 0;
#ifdef _WIN32
    int status = _pipe(s_fdPipe,1024,O_BINARY);
    HANDLE hStdin = GetStdHandle(STD_INPUT_HANDLE); 
    DWORD cNumRead, fdwSaveOldMode;
    INPUT_RECORD irInBuf; 
    if ( !xterm )  {
      if (! GetConsoleMode(hStdin, &fdwSaveOldMode) ) 
        lib_rtl_printf("GetConsoleMode"); 

      // Enable the window and mouse input events. 
      DWORD fdwMode = ENABLE_WINDOW_INPUT | ENABLE_MOUSE_INPUT; 
      if (! SetConsoleMode(hStdin, fdwMode) ) 
        lib_rtl_printf("SetConsoleMode"); 
    }
#endif

    while(1)  {
      if ( xterm )  {
        ::read(fileno(stdin),&ch,1);
        write(s_fdPipe[1],&ch,1);
      }
#ifdef _WIN32
      else {
        if (! ReadConsoleInput( 
              hStdin,         // input buffer handle 
              &irInBuf,       // buffer to read into 
              1,              // size of read buffer 
              &cNumRead) )    // number of records read 
        {
          lib_rtl_printf("ReadConsoleInput"); 
        }
        if ( irInBuf.EventType != KEY_EVENT ) continue;
        if ( irInBuf.Event.KeyEvent.bKeyDown == 0 ) continue;
        ch = char(irInBuf.Event.KeyEvent.wVirtualKeyCode);
        for(int i=0;i<irInBuf.Event.KeyEvent.wRepeatCount;++i)  {
          write(s_fdPipe[1],&ch,1);
        }
      }
#endif
      s_fdPipeBytes++;
      //lib_rtl_printf("Wait for hit...\n");
      if ( ch == -1 ) continue;
      // lib_rtl_printf("Got hit:%02X !!\n",ch);
      for(iterator i=m->begin(); i != m->end(); ++i)  {
        PortEntry* e = (*i).second;
        if ( e && e->armed )  {
          e->armed = 0;
          if ( e->callback ) (*e->callback)(e->param);
        }
      }
    }
  }
  int EntryMap::handle()  {
    std::vector<__NetworkChannel__> channels;
    while(1)  {
      int nsock = 0, mxsock = 0;
      size_t len = size();
      if(channels.size()<len) channels.resize(len+32);
      fd_set read_fds, exc_fds;
      FD_ZERO(&exc_fds);
      FD_ZERO(&read_fds);
      m_dirty = false;
      {
        RTL::Lock lock(m_mutex_id);
        for(iterator i=begin(); i != end(); ++i)  {
          if ( (*i).second->armed )  {
            __NetworkChannel__ fd = (*i).first;
            if ( fd > mxsock ) mxsock = fd;
            FD_SET(fd, &read_fds);
            FD_SET(fd, &exc_fds);
            channels[nsock] = fd;
            nsock++;
          }
        }
      }
      if ( nsock > 0 )  {
        timeval tv = { 0, 10 };
        int res = 0;
        res = select(mxsock+1, &read_fds, 0, &exc_fds, &tv);
        if ( res == 0 )  {
          continue;
        }
        else if (res < 0)  {
          return res;
        }
      }
      else  {        
        timeval tv = { 0, 10 };
        ::select(nsock, 0, 0, 0, &tv);
        continue;
      }
      for ( int j=0; j<nsock; ++j )  {
        __NetworkChannel__ fd = channels[j];      
        if (FD_ISSET(fd, &read_fds))  {
          RTL::Lock lock(m_mutex_id);
          iterator k=find(fd);
          if ( k != end() )  {
            PortEntry* e = (*k).second;
            if ( e )  {
              int t = e->type, nb = IOPortManager(m_port).getAvailBytes(fd);
              //::lib_rtl_printf("got read request: %d bytes!\n",nb);
              if ( e->callback )   {
                if ( !(nb==0 && fd == fileno(stdin)) )
                  e->armed = 0;
                int (*callback)(void*) = e->callback;
                void* param = e->param;
                RTL::Lock lock(m_mutex_id, true);
                try  {
                  (*callback)(param);
                }
                catch(...)  {
                  ::lib_rtl_printf("EntryMap::handle> Exception!\n");
                }
              }
              if ( t == 1 && nb <= 0 )  {
                k = find(fd);
                if ( k != end() )  {
                  if ( (*k).second ) delete (*k).second;
                  erase(k);
                }
              }
            }
          }
        }
      }
    }
  }

  EntryMap::EntryMap(__NetworkPort__ p) : m_thread(0), m_port(p), m_dirty(false), m_mutex_id(0) {
    lib_rtl_create_lock(0, &m_mutex_id);
  }
  EntryMap::~EntryMap() {
    lib_rtl_delete_lock(m_mutex_id);
  }
  int EntryMap::run()  {
    static bool first = true;
    if ( first )  {
      first = false;
      lib_rtl_declare_exit(ioman_exithandler,0);
    }
    if ( !m_thread )  {
      int (*call)(void*);
      switch(m_port)  {
        case 0:
#ifdef _WIN32
          call = consoleCall;
          break;
#endif
        default: 
          //lib_rtl_printf("Installing thread call!\n");
          call = threadCall;
          break;
      }
      int sc = lib_rtl_start_thread(call, this, &m_thread);
      if ( !lib_rtl_is_success(sc) )  {
        ::lib_rtl_printf("Failed to create port-thread\n");
        throw std::runtime_error("Failed to create port-thread");
      }
    }
    return 1;
  }
}

typedef std::map<int,EntryMap* > PortMap;

static inline PortMap& portMap()  {
  static PortMap* s_map = (PortMap*)lib_rtl_alloc_int_pointer_map();
  return *s_map;
}
int IOPortManager::getChar(int fd, char* c)  {
#ifdef _WIN32
  if ( fd == fileno(stdin) )  {
    fd = s_fdPipe[0];
    if ( 1 == ::read(fd,c,1) ) s_fdPipeBytes--;
    return 1;
  }
#endif
  return ::read(fd,c,1);
}

int IOPortManager::getAvailBytes(int fd)  {
#ifdef _WIN32
  if ( fd == fileno(stdin) ) return s_fdPipeBytes;
#endif
  unsigned long ret;
  if (ioctlsocket(fd, FIONREAD, &ret) != -1)
    return int(ret);
  else    {
    if (errno == EINVAL) // Server socket
      return -1;
    return 0;
  }
}

int IOPortManager::add(int typ, NetworkChannel::Channel c, int (*callback)(void*), void* param)  {
  EntryMap* em = portMap()[m_port];
  if ( !em ) {
    //lib_rtl_printf("Install port watcher for %d\n",m_port);
    portMap()[m_port] = em = new EntryMap(m_port);
  }
  em->setDirty();
  RTL::Lock lock(em->m_mutex_id);
  {
    PortEntry* e = (*em)[c];
    if ( !e ) {
      //lib_rtl_printf("Install channel watcher for %d\n",c);
      (*em)[c] = e = new PortEntry;
    }
    e->callback = callback;
    e->param = param;
    e->armed = 1;
    e->type = typ;
  }
  return em->run();
}

int IOPortManager::remove(NetworkChannel::Channel c)  {
  EntryMap* em = portMap()[m_port];
  if ( em ) {
    EntryMap::iterator i = em->find(c);
    if ( i != em->end() )  {
      RTL::Lock lock(em->m_mutex_id);
      i = em->find(c);
      if ( i != em->end() )  {
        if ( (*i).second ) delete (*i).second;
        em->erase(i);
        em->setDirty();
      }
    }
  }
  return 1;
}

