#include <cstdarg>

#ifndef __GCCXML
  #define __GCCXML
  #define __REAL_COMP
  #ifdef _WIN32
  #else
    typedef void __va_list_tag;
  #endif
#else
  #ifdef _WIN32
    #pragma warning("excluding _LDT_ENTRY_DEFINED")
    #define _LDT_ENTRY_DEFINED
    typedef struct _LDT_ENTRY {
    unsigned short LimitLow;
    unsigned short BaseLow;
    union {
        struct {
            unsigned char  BaseMid;
            unsigned char  Flags1;     // Declare as bytes to avoid alignment
            unsigned char  Flags2;     // Problems.
            unsigned char  BaseHi;
        } Bytes;
    } HighWord;
} LDT_ENTRY, *PLDT_ENTRY;
    #include "windows.h"
  #endif
#endif

#include <iostream>
#include <ctime>
#include <map>
#include <vector>
#include "WT/wtdef.h"
#include "MBM/bmstruct.h"
#include "MBM/bmdef.h"
#include "RTL/rtl.h"
#include "RTL/Lock.h"
#include "RTL/Thread.h"
#include "RTL/GlobalSection.h"
#include "AMS/amsdef.h"
#include "NET/DataTransfer.h"
#include "NET/IOPortManager.h"
#include "NET/NetworkChannel.h"
#include "NET/NetworkConnection.h"
#include "NET/TcpNetworkChannel.h"
#include "NET/TcpConnection.h"
#include "NET/UdpNetworkChannel.h"
#include "NET/UdpConnection.h"
#include "SCR/scr.h"
#include "CPP/IocSensor.h"
#include "CPP/AmsSensor.h"
#include "CPP/TimeSensor.h"
#include "CPP/Interactor.h"
#include "CPP/Event.h"

namespace  {
  class Dictionary  {
  protected:
    std::vector<int> m_intV;
    Dictionary()  {
    }
    ~Dictionary() {}
  };
}
#include "dict/PyInteractor.cpp"
