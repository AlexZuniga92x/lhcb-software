// $Id: NodeStatsPublisher.cpp,v 1.7 2010-10-14 06:44:04 frankb Exp $
//====================================================================
//  ROMon
//--------------------------------------------------------------------
//
//  Package    : ROMon
//
//  Description: Readout monitoring in the LHCb experiment
//
//  Author     : M.Frank
//  Created    : 29/1/2008
//
//====================================================================
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/ROMon/src/NodeStatsPublisher.cpp,v 1.7 2010-10-14 06:44:04 frankb Exp $

// C++ include files
#include <iostream>
#include <stdexcept>

// Framework includes
#include "dic.hxx"
#include "dis.hxx"
#include "RTL/rtl.h"
#include "RTL/Lock.h"
#define MBM_IMPLEMENTATION
#include "ROMon/ROMon.h"
#include "ROMon/ROMonOstream.h"
#include "ROMon/CPUMon.h"
#include "ROMon/CPUMonOstream.h"
#include "ROMon/NodeStatsPublisher.h"
#include "ROMonDefs.h"

using namespace ROMon;
using namespace std;
typedef FMCMonListener::Descriptor DSC;
typedef RODimListener::Clients Clients;
static std::string PUBLISHING_NODE = "ECS03";

namespace {
  static const char* s_empty = "";
  template <class T> struct _Svc : public NodeStatsPublisher::_BaseSvc {
    int id, flag;
    size_t buffLen;
    char* buff;
    std::string name;
    FMCMonListener& info;
    _Svc(FMCMonListener& i, size_t l, const std::string& nam, int flg=0)
      : id(0), flag(flg), buffLen(l), buff(0), name(nam), info(i) {
      buffLen *= (1024*8); // *8 for 64 node farms
      buff = new char[buffLen];
    }
    virtual ~_Svc() {
      ::dis_remove_service(id);
      ::free(buff);
      buff = 0;
    }
    virtual int get_data() {  return 0; }
    virtual void start() {
      ((T*)buff)->reset();
      id = ::dis_add_service((char*)name.c_str(),(char*)"C",0,0,feed,(long)this);
    }
    virtual void print() const {
      log() << "========================" << ::lib_rtl_timestr() 
            << "========================" << endl
            << *(T*)buff << endl;
    }
    virtual void update() {
      ::dis_update_service(id);
    }
    virtual void load() {
      for(int res=get_data(); res==2; ) {
        if ( buff ) ::free(buff);
        buffLen += buffLen/2;
        buff = (char*)::malloc(buffLen);
        res = get_data();
      }
    }
    /// Feed data to DIS when updating data
    static void feed(void* tag, void** buf, int* size, int* first) {
      _Svc<T>* h = *(_Svc<T>**)tag;
      if ( !(*first) ) {
        *buf = h->buff;
        *size  = ((T*)h->buff)->length();
        return;
      }
      *size = 0;
      *buf = (void*)s_empty;
    }
  };

  /// Update buffer with CPU information
  template <> int _Svc<CPUfarm>::get_data() {
    CPUfarm& f = *(CPUfarm*)buff;
    CPUfarm::Nodes& nodes = f.reset()->nodes;
    ro_get_node_name(f.name,sizeof(f.name));
    f.type = CPUfarm::TYPE;
    f.time = ::time(0);
    CPUset* set_out = nodes.reset();
    for(Clients::const_iterator ic=info.clients().begin(); ic != info.clients().end(); ++ic) {
      NodeStats* ni = (NodeStats*)(*ic).second->data<DSC>()->data;
      if ( ni ) {
        if ( ((char*)set_out+ni->cpu()->length()) > buff+buffLen ) return 2;
        ::memcpy(set_out,ni->cpu(),ni->cpu()->length());
        set_out = nodes.add(set_out);
      }
    }
    f.fixup();
    return 1;
  }

  /// Update buffer with process information
  template <> int _Svc<ProcFarm>::get_data() {
    ProcFarm& f = *(ProcFarm*)buff;
    ProcFarm::Nodes& nodes = f.reset()->nodes;
    const Clients& cl_info = info.clients();
    ro_get_node_name(f.name,sizeof(f.name));
    f.type = ProcFarm::TYPE;
    f.time = ::time(0);
    Procset* set_out = nodes.reset();
    for(Clients::const_iterator ic = cl_info.begin(); ic != cl_info.end(); ++ic) {
      NodeStats* ni = (NodeStats*)(*ic).second->data<DSC>()->data;
      if ( ni ) {
	Procset*   set_in = ni->procs();
	if ( flag ) {
	  Procset::Processes::iterator p_in=(*set_in).processes.begin();
	  Procset::Processes::iterator p_out=(*set_out).processes.reset();
	
	  if ( ((char*)set_out + sizeof(Procset)) > buff+buffLen ) return 2;
	  ::strncpy((*set_out).name,(*set_in).name,sizeof((*set_out).name));
	  (*set_out).name[sizeof((*set_out).name)-1] = 0;
	  (*set_out).time    = (*set_in).time;
	  (*set_out).millitm = (*set_in).millitm;
	  for( ; p_in != (*set_in).processes.end(); p_in=(*set_in).processes.next(p_in)) {
	    if ( ((char*)p_out +sizeof(Process)) > (buff+buffLen) ) return 2;
	    //if ( ::strncmp((*p_in).utgid,"N/A",3)!=0 || ::strncmp((*p_in).cmd,"PVSS00",6)==0 ) {
	    if ( ::strncmp((*p_in).utgid,"N/A",3)!=0 ) {
	      ::memcpy(&(*p_out),&(*p_in),sizeof(Process));
	      p_out = (*set_out).processes.add(p_out);
	    }
	  }
	}
	else {  // Copy all processes in one big block....
	  if ( ((char*)set_out + set_in->length()) > buff+buffLen ) return 2;
	  ::memcpy(&(*set_out),&(*set_in),set_in->length());
	}
	set_out = nodes.add(set_out);
      }
    }
    f.fixup();
    return 1;
  }

  /// Update buffer with MBM information
  template <> int _Svc<Nodeset>::get_data() {
    Nodeset* n = (Nodeset*)buff;
    Nodeset::Nodes& nodes = n->reset()->nodes;
    Nodeset::Nodes::iterator it = nodes.reset();
    const Clients& cl = info.clients();
    ro_get_node_name(n->name,sizeof(n->name));
    n->type = Nodeset::TYPE;
    for(Clients::const_iterator ic = cl.begin(); ic != cl.end(); ++ic) {
      DSC* d = (*ic).second->data<DSC>();
      if ( d->data ) {
        if ( ((char*)it) > buff+buffLen ) return 2;
        ::memcpy(it,d->data,d->actual);
        it = nodes.add(it);
      }
    }
    return 1;
  }
}


/// Standard constructor
NodeStatsPublisher::NodeStatsPublisher(int argc, char** argv) 
: m_needUpdate(true)
{
  string match = "*", svc, nam;
  RTL::CLI cli(argc, argv, NodeStatsPublisher::help);
  cli.getopt("match",3,match);
  cli.getopt("publish",   2, svc);
  m_print   = cli.getopt("print",2) != 0;
  m_verbose = cli.getopt("verbose",1) != 0;
  cli.getopt("statDelay", 5, m_statDelay);
  cli.getopt("mbmDelay",  4, m_mbmDelay);

  ::dic_set_dns_node((char*)RTL::nodeNameShort().c_str());
  ::dis_set_dns_node((char*)PUBLISHING_NODE.c_str());
  m_service[0] = new _Svc<Nodeset> (m_mbm,  64,svc);
  m_service[1] = new _Svc<CPUfarm> (m_stat, 10,svc + "/CPU");
  m_service[2] = new _Svc<ProcFarm>(m_stat,512,svc + "/Tasks");
  m_service[3] = new _Svc<ProcFarm>(m_stat, 64,svc + "/ROTasks",1);

  if ( svc.empty() )  {
    log() << "Unknown data type -- cannot be published." << std::endl;
    throw std::runtime_error("Unknown data type and unknwon service name -- cannot be published.");
  }

  m_stat.setMatch(match);
  m_stat.setItem("Statistics");
  m_stat.setVerbose(m_verbose);
  m_stat.setUpdateHandler(this);
  m_mbm.setMatch(match);
  m_mbm.setItem("Readout");
  m_mbm.setVerbose(m_verbose);
  m_mbm.setUpdateHandler(this);
  m_mbm.start();
  m_stat.start();
  for(size_t i=0; i<sizeof(m_service)/sizeof(m_service[0]); ++i)
    m_service[i]->start();
  DimServer::start(svc.c_str());
}

/// Default destructor
NodeStatsPublisher::~NodeStatsPublisher() {
  for(size_t i=0; i<sizeof(m_service)/sizeof(m_service[0]); ++i)
    delete m_service[i];
}

/// Help printout in case of -h /? or wrong arguments
void NodeStatsPublisher::help() {
  ::lib_rtl_output(LIB_RTL_ALWAYS,"cpumon_collect -opt [-opt]\n");
}

/// Start monitoring activity
int NodeStatsPublisher::monitor() {
  bool exec = true;
  int stat_delay = m_statDelay;
  while(exec)    {
    stat_delay -= m_mbmDelay;
    dim_lock();
    m_service[0]->load();
    for(size_t i=1; stat_delay < 0 && i<sizeof(m_service)/sizeof(m_service[0]); ++i)
      m_service[i]->load();
    dim_unlock();
    m_service[0]->update();
    for(size_t i=1; stat_delay < 0 && i<sizeof(m_service)/sizeof(m_service[0]); ++i)
      m_service[i]->update();
    if ( m_print ) m_service[0]->print();
    for(size_t i=1; m_print && stat_delay < 0 && i<sizeof(m_service)/sizeof(m_service[0]); ++i)
      m_service[i]->print();
    if ( stat_delay<=0 ) stat_delay = m_statDelay;
    ::lib_rtl_sleep(m_mbmDelay);
  }
  return 1;
}

/// Update handler
void NodeStatsPublisher::update(void* /* param */)   {
  m_needUpdate = true;
}

extern "C" int romon_syspublish(int argc, char** argv) {
  NodeStatsPublisher mon(argc,argv);
  print_startup("Process information COLLECTOR");
  return mon.monitor();
}

