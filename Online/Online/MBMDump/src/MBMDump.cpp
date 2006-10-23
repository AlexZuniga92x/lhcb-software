#include "MBMDump/MBMDump.h"
#include "UPI/upidef.h"
#include "RTL/rtl.h"
#include <cctype>

const char* MBMDump::procName()  {  
  return RTL::processName().c_str();    
}

void MBMDump::rconv(char c[], int word)   {
  for(int i=0;i<4;i++){
    c[3-i] = (word>>(i*8) & 0XFF);
    if(!isalnum(c[3-i]))c[3-i] = '.';
  }
}

static void help()  {
    ::lib_rtl_printf("mbmdump -opt [-opt]");
    ::lib_rtl_printf("    -d(ebug)               Invoke debugger.");
}

static size_t print_msg(void* /* env */, const char* fmt, va_list args)  {
  char buff[1024];
  size_t result = vsprintf(buff,fmt,args);
  ::upic_write_message(buff,"");
  return result;
}

extern "C" int mbmdump(int argc,char *argv[])   {
  using namespace MBMDump;
  RTL::CLI cli(argc, argv, help);
  bool debug = cli.getopt("debug",1) != 0;
  bool files = cli.getopt("files",1) != 0;
  if ( debug ) lib_rtl_start_debugger();
  int status = ::upic_attach_terminal();
  if(status != UPI_NORMAL)
    exit(EXIT_FAILURE);

  ::lib_rtl_install_printer(print_msg,0);
  ::upic_change_pasteboard(132,0);
  if ( files )  {
    FileMainMenu m;
    return m.run();
  }
  MBMMainMenu m;
  return m.run();
}
