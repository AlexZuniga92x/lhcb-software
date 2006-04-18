#include "MBM/MepConsumer.h"
#include "MBM/Producer.h"
#include "RTL/rtl.h"
#include "WT/wtdef.h"

namespace {
  static void help()  {
    ::lib_rtl_printf("mep_holder_a -opt [-opt]\n");
    ::lib_rtl_printf("    -n=<name>              Buffer member name\n");
    ::lib_rtl_printf("    -a                     Asynchonous mode (default is synchronous)\n");
    ::lib_rtl_printf("    -p(artition)=<number>  Partition ID\n");
  }
  struct Holder  : public MEP::Consumer  {
    Holder(const std::string& nam, int partID) : MEP::Consumer(nam, partID)  {
      m_flags = USE_MEP_BUFFER;
      include();
      m_bmid = m_mepID->mepBuffer;
      unsigned int vetomask[4] = {0,0,0,0};
      unsigned int trmask[4]   = {~0x0,~0x0,~0x0,~0x0};
      addRequest(EVENT_TYPE_MEP,trmask,vetomask,BM_MASK_ANY,BM_REQ_ALL,BM_FREQ_PERC,100.);
      ::lib_rtl_printf(" MEP    buffer start: %08X\n",m_mepID->mepStart);
      ::lib_rtl_printf(" EVENT  buffer start: %08X\n",m_mepID->evtStart);
      ::lib_rtl_printf(" RESULT buffer start: %08X\n",m_mepID->resStart);
      // mep_set_watch(m_mepID);
    }
    virtual int run() {
      int sc;
      for (sc=getEvent(); sc == MBM_NORMAL; sc=getEvent() )  {
        const MBM::EventDesc& evt = event();
        MEPEVENT* e = (MEPEVENT*)(int*)evt.data;
        if ( e->magic != mep_magic_pattern() )  {
          lib_rtl_printf("Bad magic MEP pattern !!!!\n");
        }
        //int cnt = 0;
        while ( 1 )  {
          //cnt++;
          //if ( (cnt%3)==0 ) lib_rtl_printf(".");
          //if ( (cnt%50)==0 )  {
          //  lib_rtl_printf("WAIT MEP release [%d] MEP @ %08X MEP:%p [%d] Pattern:%08X\n",
          //    e->refCount, m_mepID->mepStart+e->begin, (void*)e, e->evID, e->magic);
          //}
          if ( e->refCount <= 1 )    {
            if ( e->refCount != 1 )    {
              lib_rtl_printf("MEP release [%d] Event at address %08X MEP:%p [%d] Pattern:%08X\n",
                e->refCount, m_mepID->mepStart+e->begin, (void*)e, e->evID, e->magic);
            }
            break;
          }
#ifdef _WIN32
          lib_rtl_sleep(1);
#else
          lib_rtl_usleep(10);
#endif
        }
        //lib_rtl_printf("MEP release [%d] MEP @ %08X MEP:%p [%d] Pattern:%08X\n",
        //  e->refCount, m_mepID->mepStart+e->begin, (void*)e, e->evID, e->magic);
        freeEvent();

        //e->valid = 0;
      }
      return sc;
    }
  };
}

extern "C" int mep_holder_a(int argc,char **argv) {
  RTL::CLI cli(argc, argv, help);
  std::string name = "holder";
  int partID = 0x103;
  bool async = cli.getopt("asynchronous",1) != 0;
  cli.getopt("name",1,name);
  cli.getopt("partitionid",1,partID);
  ::lib_rtl_printf("%synchronous MEP Holder \"%s\" (pid:%d) included in buffers.\n",
     async ? "As" : "S", name.c_str(),Holder::pid());
  Holder c(name, partID);
  if ( async ) c.setNonBlocking(WT_FACILITY_DAQ_EVENT, true);
  return c.run();
}
