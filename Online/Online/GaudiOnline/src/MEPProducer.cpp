// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/GaudiOnline/src/MEPProducer.cpp,v 1.8 2006-12-14 18:59:20 frankb Exp $
//  ====================================================================
//  RawBufferCreator.cpp
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
#include "MDF/RawEventHelpers.h"
#include "MDF/MEPEvent.h"
#include "MBM/MepProducer.h"
#include "WT/wt_facilities.h"

#include <fcntl.h>
#ifdef _WIN32
#include <io.h>
#else
#include <unistd.h>
#define O_BINARY 0
#endif

namespace {
  int __dummyReadEvent(void* data, size_t bufLen, size_t& evtLen)  {
    static int nrewind = 0;
    static int file = open("../cmt/mepData_0.dat", O_RDONLY|O_BINARY);
    LHCb::MEPEvent* me = (LHCb::MEPEvent*)data;
again:
    int status1 = ::read(file, me, me->sizeOf());
    if ( status1 < (int)me->sizeOf() )  {
      ::lseek(file, 0, SEEK_SET);
      ::printf("[0] Rewind # %d: End-of-file.\n", ++nrewind);
      goto again;
    }
    if ( status1 <= (int)bufLen )  {
      int status2 = ::read(file, ((char*)data)+me->sizeOf(), me->size());
      if ( status2 < (int)me->size() )  {
        ::lseek(file, 0, SEEK_SET);
        ::printf("[1] Rewind # %d: End-of-file.\n", ++nrewind);
        goto again;
      }
      evtLen = me->size()+me->sizeOf();
      // printf("MEP size: %d \n",evtLen);
      return 1;
    }
    return 0;
  }
}

namespace {
  static void help()  {
    ::printf("mep_prod_a -opt [-opt]\n");
    ::printf("    -n(ame)=<name>         buffer member name\n");
    ::printf("    -a(synchronous)        Asynchonous mode (default is synchronous)\n");
    ::printf("    -s(pace)=<number>      Default space allocation in kBytes\n");
    ::printf("    -p(artition)=<number>  Partition ID\n");
    ::printf("    -r(efcount)=<number>   Initial MEP reference count\n");
    ::printf("    -d(ebug)               Invoke debugger\n");
  }
  struct MEPProducer  : public MEP::Producer  {
    int m_spaceSize, m_refCount;
    MEPProducer(const std::string& nam, int partitionID, int refcnt, size_t siz) 
    : MEP::Producer(nam, partitionID), m_spaceSize(siz), m_refCount(refcnt)
    {
      m_spaceSize *= 1024;  // Space size is in kBytes
      m_flags = USE_MEP_BUFFER;
      include();
      m_bmid = m_mepID->mepBuffer;
      ::printf(" Buffer space: %d bytes\n",m_spaceSize);
      ::printf(" MEP    buffer start: %08X\n",m_mepID->mepStart);
      ::printf(" EVENT  buffer start: %08X\n",m_mepID->evtStart);
      ::printf(" RESULT buffer start: %08X\n",m_mepID->resStart);
    }
    ~MEPProducer()  {
    }
    int spaceRearm(int) {
      return MEP::Producer::spaceRearm(m_spaceSize);
    }
    int receiveEvent(void* data, size_t bufLen, size_t& evtLen)  {
      int status = __dummyReadEvent(data, bufLen, evtLen);
      return status==1 ? MBM_NORMAL : MBM_ERROR;
    }
    int spaceAction() {
      static int id = -1;
      size_t evtLen = 0;
      MBM::EventDesc& dsc = event();
      MEPEVENT* e = (MEPEVENT*)dsc.data;
      e->refCount    = m_refCount;
      e->evID        = ++id;
      e->begin       = int(int(e)-m_mepID->mepStart);
      e->packing     = -1;
      e->valid       = 1;
      e->magic       = mep_magic_pattern();
      //::printf("MEP Buffer: [%d] Event at address %08X MEP:%p [%d] Pattern:%08X [Release MEP]\n",
      //  e->refCount, m_mepID->mepStart+e->begin, (void*)e, e->evID, e->magic);
      int status = receiveEvent(e->data, m_spaceSize-sizeof(MEPEVENT), evtLen);
      if ( status == MBM_NORMAL )  {
        m_event.len = evtLen+sizeof(MEPEVENT)-sizeof(e->data);
        m_event.mask[0] = partitionID();
        m_event.mask[1] = 0;
        m_event.mask[2] = 0;
        m_event.mask[3] = 0;
        m_event.type    = EVENT_TYPE_MEP;
        declareEvent();
        status = sendSpace();
      }
      return status;
    }
    // Run the application in synchonous mode
    int runSynchronous() {
      int status = spaceRearm(0);
      if ( status == MBM_NORMAL )  {
        status = spaceAction();
      }
      return status;
    }
  };
}

extern "C" int mep_producer(int argc,char **argv) {
  RTL::CLI cli(argc, argv, help);
  int space = 64*1024;             // default 64 kB
  int partID = 0x103;              // default is LHCb partition id
  int refCount = 1;
  std::string name = "producer";
  bool async = cli.getopt("asynchronous",1) != 0;
  bool debug = cli.getopt("debug",1) != 0;
  cli.getopt("name",1,name);
  cli.getopt("space",1,space);
  cli.getopt("partitionid",1,partID);
  cli.getopt("refcount",1,refCount);
  if ( debug ) ::lib_rtl_start_debugger();
  ::printf("%synchronous MEP Producer \"%s\" Partition:%d (pid:%d) included in buffers.\n",
     async ? "As" : "S", name.c_str(), partID, MEPProducer::pid());
  MEPProducer p(name, partID, refCount, space);
  if ( async ) p.setNonBlocking(WT_FACILITY_DAQ_SPACE, true);
  return p.run();
}
