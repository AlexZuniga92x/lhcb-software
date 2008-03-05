#include "rtl_internal.h"
#include <vector>
#include <map>
#include <ctime>
#include <cerrno>
#include <cstdarg>
#include <iostream>
#include <fcntl.h>
#ifdef _WIN32
#define vsnprintf _vsnprintf
#include <winsock.h>
#include <stdexcept>
namespace {
  struct __init__ {
    __init__()  {
      static WSADATA g_WSAData;
      memset(&g_WSAData, 0, sizeof(WSADATA));
      if (WSAStartup ( MAKEWORD(1,1), &g_WSAData) != 0)    {
        throw std::runtime_error("RTL: WSAStartup failed!");
      }
    }
  };
  static __init__ g_init;
}

#else
#include <cstdlib>
#include <unistd.h>
#include <netdb.h>
#include <arpa/inet.h>
#endif
#include "RTL/strdef.h"

namespace RTL  {
  struct EXHDEF   {
    int   flink;
    int  (*exit_handler)(void*);
    void *exit_param;
    int  *exit_status;
  };
  struct ExitHandler : public std::vector<EXHDEF>  {
    ExitHandler();
    ~ExitHandler();
    static void execute();
    static std::vector<EXHDEF>& exitHandlers();
  };
}

static int exit_status;

#ifdef USE_PTHREADS
#include <unistd.h>
#include <signal.h>
#define ERROR_SUCCESS 0
const char* RTL::errorString(int status)  {
  return strerror(status);
}

#define INSTALL_SIGNAL(x,y) install(x , #x , y);
namespace RTL {

  /**@class ExitSignalHandler
   *
   * Small class to manipulate default signal handling
   *
   * @author M.Frank
   */
  class ExitSignalHandler {
  protected:
    typedef std::map<int,std::pair<std::string, struct sigaction> > SigMap;
    SigMap m_map;
    ExitSignalHandler();
    ~ExitSignalHandler();
  public:
    static ExitSignalHandler& instance();
    void install(int num, const std::string& name, struct sigaction& action);
    static void handler(int signum, siginfo_t *info,void * );
  };

  static ExitSignalHandler& s_RTL_ExitSignalHandler = ExitSignalHandler::instance();
}
extern "C" void* lib_rtl_exithandler_instance() {
  return &RTL::s_RTL_ExitSignalHandler;
}


RTL::ExitSignalHandler::ExitSignalHandler() {
  struct sigaction new_action;
  sigemptyset(&new_action.sa_mask);
  new_action.sa_handler   = 0;
  new_action.sa_sigaction = handler;
  new_action.sa_flags     = SA_SIGINFO;
  INSTALL_SIGNAL(SIGABRT,  new_action);
  INSTALL_SIGNAL(SIGFPE,   new_action);
  INSTALL_SIGNAL(SIGILL,   new_action);
  INSTALL_SIGNAL(SIGINT,   new_action);
  INSTALL_SIGNAL(SIGSEGV,  new_action);
  INSTALL_SIGNAL(SIGTERM,  new_action);
  INSTALL_SIGNAL(SIGHUP,   new_action);
  // INSTALL_SIGNAL(SIGKILL,  new_action);
  INSTALL_SIGNAL(SIGQUIT,  new_action);
  INSTALL_SIGNAL(SIGBUS,   new_action);
  INSTALL_SIGNAL(SIGXCPU,  new_action);
}

RTL::ExitSignalHandler::~ExitSignalHandler() {
}

RTL::ExitSignalHandler& RTL::ExitSignalHandler::instance() {
  static ExitSignalHandler inst;
  return inst;
}

void RTL::ExitSignalHandler::install(int num, const std::string& name, struct sigaction& action) {
  std::pair<std::string, struct sigaction>& old_action = m_map[num];
  int res = sigaction (num, &action, &old_action.second);
  if ( res != 0 ) {
    std::cout << "Failed to install exit handler for " << name << std::endl;
    return;
  }
  //std::cout << "Successfully installed handler for " << name << std::endl;
  old_action.first = name;
}


void RTL::ExitSignalHandler::handler(int signum, siginfo_t *info,void * ) {
  RTL::ExitHandler::execute();
  SigMap& m = instance().m_map;
  SigMap::iterator i=m.find(signum);
  if ( i != m.end() ) {
    __sighandler_t old = (*i).second.second.sa_handler;
    std::cout << "Handled signal: " << info->si_signo;
    std::cout << " [" << (*i).second.first;
    std::cout << "] Old action:" << (void*)old << std::endl;
    if ( old && old != SIG_IGN ) {
      (*old)(signum);
    }
    else if ( old == SIG_DFL ) {
      ::_exit(0);
    }
  }
}

#elif _WIN32

#include <windows.h>
#include <process.h>

const char* RTL::errorString(int status)  {
  static char s[1024] = "No error reporting implemented";
  static int len = sizeof(s)-1;
  void* lpMessageBuffer;
  ::FormatMessage( 
    FORMAT_MESSAGE_ALLOCATE_BUFFER |  FORMAT_MESSAGE_FROM_SYSTEM,
    0,
    status,
    MAKELANGID(LANG_NEUTRAL, SUBLANG_DEFAULT), //The user default language
    (LPTSTR) &lpMessageBuffer,
    0,
    0 );
  if ( lpMessageBuffer )  {
    strncpy(s, (const char*)lpMessageBuffer, len);
    s[len] = 0;
    size_t l = strlen(s);
    if ( l > 1 )  {
      s[l-1] = 0;
      if ( !::isalnum(s[l-2]) ) s[l-2]=0;
    }
    ::LocalFree( lpMessageBuffer ); 
  }
  else {
    sprintf(s,"RTL Error: Unknown error code: %08X",status);
  }
  return s;
}
#endif

const char* RTL::errorString()  {
  return errorString(lib_rtl_get_error());
}

RTL::ExitHandler::ExitHandler() {
}

RTL::ExitHandler::~ExitHandler() {
  execute();
}

extern "C" int lib_rtl_event_exithandler();
extern "C" int lib_rtl_lock_exithandler();
extern "C" int lib_rtl_gbl_exithandler();
void RTL::ExitHandler::execute()  {
  static bool executing = false;
  if ( !executing )  {
    executing = true;
    const std::vector<EXHDEF>& v = exitHandlers();
    for (std::vector<EXHDEF>::const_reverse_iterator i=v.rbegin(); i != v.rend(); ++i)  {
      const EXHDEF& hdlr = *i;
      if ( hdlr.exit_handler )  {
        (*hdlr.exit_handler)(hdlr.exit_param);
      }
    }
    exitHandlers().clear();
    lib_rtl_event_exithandler();
    lib_rtl_lock_exithandler();
    lib_rtl_gbl_exithandler();
    executing = false;
  }
}

std::vector<RTL::EXHDEF>& RTL::ExitHandler::exitHandlers() {
  static ExitHandler s_exitHandlers;
  return s_exitHandlers;
}

/// Access to error code from socket library
int lib_rtl_socket_error()  {
#ifdef _WIN32
  return ::WSAGetLastError();
#else
  return errno;
#endif
}

int lib_rtl_get_error()   {
#ifdef USE_PTHREADS
  return errno;
#elif _WIN32
  return ::GetLastError();
#endif
}

int lib_rtl_remove_rundown(lib_rtl_rundown_handler_t,void*)    {
  return 1;
}

int lib_rtl_declare_rundown(lib_rtl_rundown_handler_t,void*)   {
  return 1;
}

int lib_rtl_declare_exit(int (*hdlr)(void*), void* param)  {
#if defined(_WIN32) || defined(__linux)
  static bool first = true;
  if ( first )  {
    first = false;
    ::atexit(RTL::ExitHandler::execute);
  }
  RTL::EXHDEF h;
  h.exit_handler = hdlr;
  h.exit_param   = param;
  h.exit_status  = &exit_status;
  RTL::ExitHandler::exitHandlers().push_back(h);
  return 1;
#endif
}

int lib_rtl_remove_exit(int (*hdlr)(void*), void* param) {
#if defined(_WIN32) || defined(__linux)
  RTL::ExitHandler::iterator i=RTL::ExitHandler::exitHandlers().begin();
  RTL::ExitHandler::iterator j=RTL::ExitHandler::exitHandlers().end();
  for(; i!=j; ++i)  {
    if ( (*i).exit_handler == hdlr && (*i).exit_param == param )  {
      RTL::ExitHandler::exitHandlers().erase(i);
      return 1;
    }
  }
  return 0;
#else
  return 1;
#endif
}

extern "C" void* lib_rtl_alloc_int_pointer_map()   {
  return new std::map<int,void*>;
}

int lib_rtl_run_ast (RTL_ast_t astadd, void* param, int)    {
#if defined(_WIN32) || defined(__linux)
  if ( astadd )  {
    return (*astadd)(param);
  }
#endif
  return 1;
}

int lib_rtl_pid()  {
  return getpid();
}

int lib_rtl_signal_message(int action, const char* fmt, ...)  {
  char buff[2048];
  va_list args;
  va_start( args, fmt );
  if ( fmt )  {
    int err;
    switch(action) {
    case LIB_RTL_ERRNO:
      err = errno;
      if ( err != 0 )  {
        ::lib_rtl_output(LIB_RTL_ERROR,"RTL: %8d : %s\n",err, RTL::errorString(err));
        ::vsnprintf(buff, sizeof(buff), fmt, args);
        ::lib_rtl_output(LIB_RTL_ERROR,"                %s\n",buff);
        return 0;
      }
      return 1;
    case LIB_RTL_DEFAULT:
        ::vsnprintf(buff, sizeof(buff), fmt, args);
        ::lib_rtl_output(LIB_RTL_ERROR,"RTL: %s\n",buff);
      break;
    case LIB_RTL_OS:
    default:
      err = lib_rtl_get_error();
      if ( err != ERROR_SUCCESS )   {
        ::lib_rtl_output(LIB_RTL_ERROR,"RTL: %8d : %s\n",err, RTL::errorString(err));
        ::vsnprintf(buff, sizeof(buff), fmt, args);
        ::lib_rtl_output(LIB_RTL_ERROR,"                %s\n",buff);
        return 0;
      }
      return 1;
    }
  }
  return 1;
}

int lib_rtl_signal_message(int,int,int) {
  return 1;
}

int lib_rtl_start_debugger()    {
#ifdef _WIN32
  _asm int 3
#else
  char txt[128];
  ::sprintf(txt,"ddd --pid=%d &",lib_rtl_pid()); 
  ::system(txt);
  ::lib_rtl_sleep(5000);  // Sleep a few seconds to allow 
                          // to attach the debugger
#endif
  return 1;
}

const char* lib_rtl_error_message(int status)  {
  return RTL::errorString(status);
}

int lib_rtl_default_return()  {
#if defined(_VMS) || defined(_WIN32)
  return 1;
#elif defined(__linux) || defined(_OSK)
  return 0;
#else
#endif
}

/// Disable intercepts
int lib_rtl_disable_intercept() {
  return lib_rtl_default_return();
}

/// Enable intercpets
int lib_rtl_enable_intercept()    {
  return lib_rtl_default_return();
}

int lib_rtl_get_process_name(char* process, size_t len)  {
  const char *tmp;
  char buff[32], buff2[64];
  size_t resultant_length = sizeof(buff2);
  tmp = ::getenv("UTGID");
  if ( !tmp ) tmp = ::getenv("PROCESSNAME");
  if ( !tmp ) tmp = ::getenv("PROCESS");
  if ( !tmp ) { sprintf(buff,"P%06d",lib_rtl_pid()); tmp=buff;}
  ::str_trim(tmp, buff2, &resultant_length);
  ::strncpy(process, buff2, len);
  return tmp ? strlen(tmp)+1>len ? 0 : 1 : 0;
}

int lib_rtl_get_node_name(char* node, size_t len)  {
  char n[64];
  const char *tmp = ::getenv("NODE");
#if defined(_WIN32)
  if ( !tmp ) tmp = ::getenv("COMPUTERNAME");
  if ( !tmp && 0 == ::gethostname (n,sizeof(n)) ) tmp = n;
#elif defined(_OSK)
#else
  if ( 0 == ::gethostname (n,sizeof(n)) ) tmp = n;
#endif
  ::strncpy(node,tmp != 0 ? tmp : "UNKNOWN", len);
  return 1;
}

int lib_rtl_get_datainterface_name(char* node, size_t len)  {
  const char *tmp = ::getenv("DATAINTERFACE");
  if ( !tmp )  {
    char n[64], nn[70];
    if ( 0 == ::gethostname (n,sizeof(n)) )  {
      if ( strchr(n,'.')>0 ) *strchr(n,'.') = 0;
      ::strncpy(nn,n,sizeof(n));
      ::strcat(nn,"-d");
      hostent* h = ::gethostbyname(nn);
      if ( h ) {
        tmp = inet_ntoa(*(in_addr*)h->h_addr);
      }
      else {
	::strcat(nn,"1");
	if ( (h=::gethostbyname(nn)) ) {
	  tmp = inet_ntoa(*(in_addr*)h->h_addr);
	}
	else if ( (h=::gethostbyname(n)) ) {
	  tmp = inet_ntoa(*(in_addr*)h->h_addr);
	}
      }
    }
  }
  ::strncpy(node,tmp != 0 ? tmp : "UNKNOWN", len);
  node[len-1] = 0;
  return 1;
}

static size_t (*s_rtl_printer)(void*, int, const char*, va_list args) = 0;
static void*    s_rtl_printer_arg = 0;

/// Printout redirection
size_t lib_rtl_output(int level, const char* format, ...)   {
  size_t result;
  va_list args;
  va_start( args, format );
  result = (s_rtl_printer != 0)
    ? (*s_rtl_printer)(s_rtl_printer_arg, level, format, args)
    : ::vfprintf(stdout, format, args);
  if ( !s_rtl_printer ) ::fflush(stdout);
  va_end(args);
  return result;
}

/// Printout redirection
size_t lib_rtl_printf(const char* format, ...)   {
  size_t result;
  va_list args;
  va_start( args, format );
  result = (s_rtl_printer != 0)
    ? (*s_rtl_printer)(s_rtl_printer_arg, LIB_RTL_ALWAYS, format, args)
    : ::vfprintf(stdout, format, args);
  if ( !s_rtl_printer ) ::fflush(stdout);
  va_end(args);
  return result;
}

/// Install RTL printer 
void lib_rtl_install_printer(size_t (*func)(void*, int, const char*, va_list args), void* param)  {
  s_rtl_printer = func;
  s_rtl_printer_arg = param;
}

extern "C" int rtl_test_main(int /* argc */, char** /* argv */)  {
  std::cout << "Executing empty test action ..... finished ......" << std::endl;
  return 1;
}

extern "C" const char* lib_rtl_timestr(const char* fmt, const time_t* tp)  {
  static char timestr[256];
  time_t t;
  if ( 0 == tp )  {
    tp = &t;
    ::time(&t);
  }
  struct tm *now = ::localtime(&t);
  ::strftime(timestr,sizeof(timestr),fmt,now);
  return timestr;
}

extern "C" const char* lib_rtl_gmtimestr(const char* fmt, const time_t* tp)  {
  static char timestr[256];
  time_t t;
  if ( 0 == tp )  {
    tp = &t;
    ::time(&t);
  }
  struct tm *now = ::gmtime(tp);
  ::strftime(timestr,sizeof(timestr),fmt,now);
  return timestr;
}

namespace RTL {
  const std::string& processName()  {
    static std::string s;
    if ( s.empty() )  {
      char txt[64];
      ::lib_rtl_get_process_name(txt, sizeof(txt));
      s = txt;
    }
    return s;
  }
  const std::string& dataInterfaceName()  {
    static std::string s;
    if ( s.empty() )  {
      char txt[64];
      ::lib_rtl_get_datainterface_name(txt, sizeof(txt));
      s = txt;
    }
    return s;
  }
  const std::string& nodeName()  {
    static std::string s;
    if ( s.empty() )  {
      char txt[64];
      ::lib_rtl_get_node_name(txt, sizeof(txt));
      s = txt;
    }
    return s;
  }
  const std::string& nodeNameShort()  {
    static std::string s;
    if ( s.empty() )  {
      s = nodeName();
      if ( s.find(".") != std::string::npos )  {
        s = s.substr(0,s.find("."));
      }
    }
    return s;
  }
}
