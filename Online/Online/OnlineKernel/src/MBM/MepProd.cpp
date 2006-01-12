#include "MBM/MepProducer.h"
#include "MBM/Producer.h"
#include "RTL/rtl.h"
#include "WT/wtdef.h"
#include "WT/wt_facilities.h"

#define PACKING_FACTOR 20
namespace {
  static void help()  {
    ::printf("mep_prod_a -opt [-opt]\n");
    ::printf("    -n=<name>      buffer member name\n");
    ::printf("    -a             Asynchonous mode (default is synchronous)\n");
  }
  struct Prod  : public MEP::Producer  {
    int mep_identifier;
    MBM::Producer* m_evtProd;
    Prod(const std::string& nam) : MEP::Producer(nam, 0x103), mep_identifier(0)   
    {
      m_flags = USE_EVT_BUFFER+USE_MEP_BUFFER;
      include();
      m_bmid = m_mepID->mepBuffer;
      m_evtProd = new MBM::Producer(m_mepID->evtBuffer, nam, partitionID());
      mbm_register_free_event(m_mepID->evtBuffer,  0, 0);
      mbm_register_alloc_event(m_mepID->evtBuffer, 0, 0);
      ::printf(" MEP    buffer start: %08X\n",m_mepID->mepStart);
      ::printf(" EVENT  buffer start: %08X\n",m_mepID->evtStart);
      ::printf(" RESULT buffer start: %08X\n",m_mepID->resStart);
    }
    ~Prod()  {
      delete m_evtProd;
    }
    int spaceRearm(int) {
      return Producer::spaceRearm(sizeof(MEPEVENT)+PACKING_FACTOR*sizeof(MEP_SINGLE_EVT));
    }
    int spaceAction() {
      MEPEVENT* ev = (MEPEVENT*)event().data;
      size_t sub_evt_len = sizeof(MEP_SINGLE_EVT);
      mep_identifier++;

      ev->refCount    = 1;
      ev->refCount   += PACKING_FACTOR; // saves mbm_register_alloc_event
      ev->mepBufferID = mep_identifier;
      ev->begin       = int(int(ev)-m_mepID->mepStart);
      ev->packing     = PACKING_FACTOR;
      ev->valid       = 1;
      ev->magic       = mep_magic_pattern();
      m_event.mask[0] = 0x103;
      m_event.mask[1] = 0;
      m_event.mask[2] = 0;
      m_event.mask[3] = 0;
      m_event.type    = EVENT_TYPE_MEP;
      for (int i = 0; i < PACKING_FACTOR; ++i )   {
        if ( m_evtProd->getSpace(sub_evt_len) == MBM_NORMAL ) {
          MBM::EventDesc& e = m_evtProd->event();
          MEP_SINGLE_EVT* sube = (MEP_SINGLE_EVT*)e.data;
          sube->begin = ev->begin;
          e.type    = EVENT_TYPE_EVENT;
          e.mask[0] = 0x103;
          e.mask[1] = 0;
          e.mask[2] = 0;
          e.mask[3] = 0;
          e.len     = sub_evt_len;
          m_evtProd->sendEvent();
        }
        else  {
          printf("Space error !\n");
        }
      }
      declareEvent();
      return sendSpace();
    }
    // Run the application in synchonous mode
    int runSynchronous() {
      spaceRearm(0);
      return spaceAction();
    }
  };
}

extern "C" int mep_prod_a(int argc,char **argv) {
  RTL::CLI cli(argc, argv, help);
  std::string name = "producer";
  bool async = cli.getopt("asynchronous",1) != 0;
  cli.getopt("name",1,name);
  ::printf("%synchronous MEP Producer \"%s\" (pid:%d) included in buffers.\n",
	   async ? "As" : "S", name.c_str(),Prod::pid());
  Prod p(name);
  if ( async ) p.setNonBlocking(WT_FACILITY_DAQ_SPACE, true);
  return p.run();
}
