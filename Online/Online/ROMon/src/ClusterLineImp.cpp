// $Id: FarmDisplayBase.cpp,v 1.51 2010-10-21 06:04:22 frankb Exp $
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
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/ROMon/src/FarmDisplayBase.cpp,v 1.51 2010-10-21 06:04:22 frankb Exp $

// Framework include files
#include "ROMon/ClusterLine.h"
#include "CPP/Interactor.h"

// C++ include files
#include <set>

/*
 *   ROMon namespace declaration
 */
namespace ROMon {


  /**@class FarmClusterLine ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Display summarizing one single subfarm. Showed as an array on the
   *   main display.
   *
   *   @author M.Frank
   */
  class FarmClusterLine : public ClusterLine  {
    int               m_evtBuilt;
    int               m_evtMoore;
    int               m_evtSent;
    int               m_totBuilt;
    int               m_totMoore;
    int               m_totSent;
    int               m_numUpdate;
    /// Flag to indicate probles with entity
    bool              m_hasProblems;
    /// Flag to indicate if the cluster is in use
    bool              m_inUse;
    /// Timestamp with last update
    time_t            m_lastUpdate;
  public:
    /// Initializing constructor
    FarmClusterLine(FarmLineDisplay* p, int pos, const std::string& n);
    /// Default destructor
    virtual ~FarmClusterLine() {}
    /// Display function drawing on pasteboard of current display
    void display();
  };

  /**@class HltDeferLine ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Display summarizing one single subfarm. Showed as an array on the
   *   main display.
   *
   *   @author M.Frank
   */
  class HltDeferLine : public ClusterLine, public Interactor  {
    typedef std::set<std::string> StrSet;
    int                 m_numFiles;
    int                 m_numRuns;
    int                 m_numUpdate;
    /// Flag to indicate probles with entity
    bool                m_hasProblems;
    /// Timestamp with last update
    time_t              m_lastUpdate;
    /// Service ID for excluded nodes
    int                 m_exclID;
    /// Set of excluded nodes
    StrSet              m_excluded;

    /// DIM command service callback
    static void excludedHandler(void* tag, void* address, int* size);
  public:
    /// Initializing constructor
    HltDeferLine(FarmLineDisplay* p, int pos, const std::string& n);
    /// Default destructor
    virtual ~HltDeferLine();
    /// Display function drawing on pasteboard of current display
    void display();
    /// Interactor overload: Display callback handler
    void handle(const Event& ev);
  };

  /**@class MonitoringClusterLine ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   @author M.Frank
   */
  class MonitoringClusterLine : public ClusterLine  {
    int               m_evtRelay;
    int               m_evtWorker;
    int               m_totRelay;
    int               m_totWorker;
    int               m_numUpdate;
    /// Flag to indicate probles with entity
    bool              m_hasProblems;
    /// Flag to indicate if the cluster is in use
    bool              m_inUse;
    /// Partition name
    std::string       m_partition;
    /// Name of the relay node withing storage cluster
    std::string       m_relayNode;
    /// Timestamp with last update
    time_t            m_lastUpdate;
  public:
    /// Initializing constructor
    MonitoringClusterLine(FarmLineDisplay* p, int pos, const std::string& n);
    /// Default destructor
    virtual ~MonitoringClusterLine() {}
    /// Display function drawing on pasteboard of current display
    void display();
  };

  /**@class StorageClusterLine ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   @author M.Frank
   */
  class StorageClusterLine : public ClusterLine   {
    int               m_evtRecv;
    int               m_evtSent;
    int               m_totRecv;
    int               m_totSent;
    int               m_numUpdate;
    /// Flag to indicate probles with entity
    bool              m_hasProblems;
    /// Flag to indicate if the cluster is in use
    bool              m_inUse;
    /// Partition name
    std::string       m_partition;
    /// Timestamp with last update
    time_t            m_lastUpdate;
  public:
    /// Initializing constructor
    StorageClusterLine(FarmLineDisplay* p, int pos, const std::string& n);
    /// Default destructor
    virtual ~StorageClusterLine() {}
    /// Display function drawing on pasteboard of current display
    void display();
  };
}

#include "ROMon/TaskSupervisor.h"
#include "TaskSupervisorParser.h"

namespace ROMon {

  /**@class CtrlFarmClusterLine ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   @author M.Frank
   */
  class CtrlFarmClusterLine : public ClusterLine, public Interactor   {
    typedef std::set<std::string> StrSet;
    /// Flag to indicate probles with entity
    bool                m_hasProblems;
    /// Service ID for excluded nodes
    int                 m_exclID;
    /// Extracted cluster information for all contained nodes
    Cluster             m_cluster;
    /// Set of excluded nodes
    StrSet              m_excluded;
    /// Timestamp with last update
    time_t              m_lastUpdate;

    /// DIM command service callback
    void updateContent(XML::TaskSupervisorParser& ts);
    /// DIM command service callback
    static void excludedHandler(void* tag, void* address, int* size);

  public:
    /// Initializing constructor
    CtrlFarmClusterLine(FarmLineDisplay* p, int pos, const std::string& n);
    /// Default destructor
    virtual ~CtrlFarmClusterLine();
    /// Display function drawing on pasteboard of current display
    void display();
    /// Interactor overload: Display callback handler
    void handle(const Event& ev);
  };
}

#include "ROMon/FarmLineDisplay.h"
#include "CPP/IocSensor.h"
#include "RTL/Lock.h"
#include "WT/wtdef.h"
#include "SCR/scr.h"
#include "ROMonDefs.h"
#include <limits>
extern "C" {
#include "dic.h"
}


using namespace ROMon;
using namespace SCR;
using namespace std;
typedef vector<string>               StringV;

#define UPDATE_TIME_MAX        15
#define CLUSTERLINE_START      2
#define COL_WARNING         (RED|BOLD)
#define COL_ALARM           (RED|BOLD|INVERSE)
#define COL_OK              (GREEN|INVERSE)
#define INT_max  numeric_limits<int>::max()
#define FLT_max  numeric_limits<float>::max()
namespace ROMon {
  typedef MBMBuffer::Clients           Clients;
  typedef Node::Buffers                Buffers;
  typedef Nodeset::Nodes               Nodes;
}

namespace {
  ClusterLineFactory<MonitoringClusterLine> s_monitoringLineFactory("Monitoring");
  ClusterLineFactory<StorageClusterLine>    s_storageLineFactory("Storage");
  ClusterLineFactory<CtrlFarmClusterLine>   s_ctrlFarmLineFactory("CtrlFarm");
  ClusterLineFactory<FarmClusterLine>       s_farmLineFactory("Farm");
  ClusterLineFactory<HltDeferLine>          s_hltDeferFactory("HLT");

  const char* _procNam(const char* nam) {
    const char* p;
    if (0 != ::strstr(nam,"MEPRx") ) return nam;
    p = ::strchr(nam,'_');
    if ( 0 != p ) return ++p;
    return "Unknown";
  }
}

/// Standard constructor
CtrlFarmClusterLine::CtrlFarmClusterLine(FarmLineDisplay* p, int pos, const std::string& n)
  : ClusterLine(p,pos,n), m_exclID(0)
{
  m_lastUpdate = time(0);
  m_hasProblems = false;
  connect(strupper(m_name)+"/TaskSupervisor/Status");
  string svc   = "HLT/ExcludedNodes/"+strupper(m_name);
  m_exclID = ::dic_info_service((char*)svc.c_str(),MONITORED,0,0,0,excludedHandler,(long)this,0,0);
}

/// Standard destructor
CtrlFarmClusterLine::~CtrlFarmClusterLine() {
  if ( m_exclID ) ::dic_release_service(m_exclID);
  m_exclID = 0;
}

void CtrlFarmClusterLine::display() {
  const char* p = data<char>();
  try {
    m_lastUpdate = time(0);
    XML::TaskSupervisorParser ts;
    if ( ts.parseBuffer(m_name, p,::strlen(p)+1) ) {
      updateContent(ts);
      return;
    }
  }
  catch(...) {
  }
  size_t         pos = position();
  Pasteboard*     pb = m_parent->pasteboard();
  Display*       dis = m_parent->display();
  RTL::Lock lock(InternalDisplay::screenLock());
  ::scrc_begin_pasteboard_update (pb);
  ::scrc_put_chars(dis," Received invalid XML information",COL_ALARM,pos,1,1);    
  ::scrc_end_pasteboard_update(pb);
}

/// DIM command service callback
void CtrlFarmClusterLine::updateContent(XML::TaskSupervisorParser& ts) {
  char txt[256];
  string val, err;
  bool cl_good = true;
  Cluster& c = m_cluster;
  Display*       dis = m_parent->display();
  int col = NORMAL, pos = 0, line=position();
  size_t taskCount=0, missTaskCount=0;
  size_t connCount=0, missConnCount=0;
  int pvss_status=0;
  c.nodes.clear();
  ts.getClusterNodes(c);

  RTL::Lock lock(InternalDisplay::screenLock());
  ::sprintf(txt,"%8s %6zu ",c.time.c_str()+11,c.nodes.size());
  begin_update(txt);
  pos = 87+CLUSTERLINE_START;
  for(Cluster::Nodes::const_iterator i=c.nodes.begin(), e=c.nodes.end(); i!=e;++i) {
    const Cluster::Node& n = (*i).second;
    bool excl = m_excluded.find(n.name) != m_excluded.end();
    bool good = (n.status == "ALIVE");
    for(Cluster::Projects::const_iterator q=n.projects.begin(); q != n.projects.end(); ++q) {
      bool pvss_ok = (*q).eventMgr && (*q).dataMgr && (*q).distMgr;
      if ( pvss_ok && pvss_status<2 ) pvss_status=1;
      else if ( !pvss_ok ) pvss_status = 2;
      good = good && pvss_ok;
    }
    col = good && n.missTaskCount==0 && n.missConnCount==0 ? GREEN|INVERSE : COL_ALARM;
    if ( excl )  {
      col = INVERSE|(col==COL_ALARM ? MAGENTA : BLUE);
    }
    else {
      taskCount     += n.taskCount;
      missTaskCount += n.missTaskCount;
      connCount     += n.connCount;
      missConnCount += n.missConnCount;
    }
    val = (n.name == m_name ? " "+n.name : " "+n.name.substr(n.name.length()-2));
    if ( ::strncasecmp(n.name.c_str(),"store",4)==0 ) val = " " + n.name;
    ::scrc_put_chars(dis,val.c_str(),col,line,pos,0);
    pos += val.length();
    cl_good |= (good || excl);
  }
  ::scrc_put_chars(dis," ",GREEN|INVERSE,line,pos,1);

  col = (c.status=="ALIVE" || cl_good) ? NORMAL : (c.status=="MIXED") ? COL_WARNING : COL_ALARM;
  if ( missTaskCount>0 ) {
    ::sprintf(txt,"%4zu/BAD ",missTaskCount);
    ::scrc_put_chars(dis,txt,COL_ALARM,line,28,0);
    ::scrc_put_chars(dis," ",NORMAL,line,37,0);
  }
  else {
    ::sprintf(txt,"%4zu ",taskCount);
    ::scrc_put_chars(dis,txt,GREEN|INVERSE,line,28,0);
    ::scrc_put_chars(dis,"     ",NORMAL,line,33,0);
  }
  if ( missConnCount>0 ) {
    ::sprintf(txt,"%4zu/BAD ",missConnCount);
    ::scrc_put_chars(dis,txt,COL_ALARM,line,37,0);  
    ::scrc_put_chars(dis," ",NORMAL,line,46,0);
  }
  else {
    ::sprintf(txt,"%4zu ",connCount);
    ::scrc_put_chars(dis,txt,GREEN|INVERSE,line,37,0);  
    ::scrc_put_chars(dis,"     ",NORMAL,line,42,0);
  }
  ::sprintf(txt,"  %-6s  ",c.status.c_str());
  ::scrc_put_chars(dis,txt,col|BOLD,line,45,0);
  //bg_black(pb);
  ::scrc_put_chars(dis,pvss_status>1?"ERROR":"   OK",pvss_status>1?COL_ALARM:GREEN,line,53,0);    

  col = NORMAL|BOLD;

  if ( pvss_status>1 )
    err = " PVSS environment looks funny.", col = COL_ALARM;    
  else if ( !cl_good && c.status == "DEAD" )
    err = " Nodes down - Check.", col = COL_WARNING;    
  else if ( c.status == "MIXED" )
    err = " Some nodes down - Check.", col = COL_WARNING;
  else if ( missTaskCount>0 )
    err = " Tasks missing - Check.", col = COL_WARNING;
  else if ( missConnCount>0 )
    err = " Connectivity bad - Check.", col = COL_WARNING;
  else
    err = " No obvious error detected.", col = NORMAL|BOLD|GREEN;
  err = err + "                                ";
  ::scrc_put_chars(dis,err.substr(0,30).c_str(),col,line,57+CLUSTERLINE_START,0);

  end_update();
}

/// Interactor overload: Display callback handler
void CtrlFarmClusterLine::handle(const Event& ev) {
  switch(ev.eventtype) {
  case IocEvent:
    switch(ev.type) {
    case CMD_EXCLUDE:
      m_excluded = *auto_ptr<StrSet>(ev.iocPtr<StrSet>()).get();
      return;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

/// DIM command service callback
void CtrlFarmClusterLine::excludedHandler(void* tag, void* address, int* size) {
  if ( tag ) {
    CtrlFarmClusterLine* l = *(CtrlFarmClusterLine**)tag;
    set<string> nodes;
    if ( address && *size > 0 ) {
      char *p = (char*)address, *end = p+*size;
      while(p<end) {
	nodes.insert(strlower(p));
	p += (::strlen(p)+1);
      }
    }
    IocSensor::instance().send(l,CMD_EXCLUDE,new set<string>(nodes));
  }
}

/// Standard constructor
StorageClusterLine::StorageClusterLine(FarmLineDisplay* p, int pos, const std::string& n)
: ClusterLine(p,pos,n)
{
  m_lastUpdate = time(0);
  m_hasProblems = false;
  m_partition = p->name();
  connect(strlower(m_name)+"/ROpublish");
}

void StorageClusterLine::display() {
  char txt[256];
  string err = "";
  int col = NORMAL;
  set<string> bad_nodes;
  size_t         pos = position();
  Display*       dis = m_parent->display();
  const Nodeset*   c = data<Nodeset>();

  string evt_buff = std::string("Events_"+m_partition);
  int numNodes = 0, numBuffs = 0, numClients = 0;
  float fsp, fsl, fspace[3] = {FLT_max,FLT_max,FLT_max}, fslots[3] = {FLT_max,FLT_max,FLT_max};
  int tot_prod[3] = {0,0,0}, min_prod[3] = {INT_max,INT_max,INT_max};
  int   num_cl[3] = {0,0,0}, num_sl[3] = {0,0,0};

  m_inUse = false;
  for (Nodes::const_iterator n=c->nodes.begin(); n!=c->nodes.end(); n=c->nodes.next(n))  {
    bool recv_node = ::strncasecmp((*n).name,"storerecv",8) == 0;
    const Buffers& buffs = *(*n).buffers();
    ++numNodes;
    for(Buffers::const_iterator ib=buffs.begin(); ib!=buffs.end(); ib=buffs.next(ib))  {
      const char* bnam = (*ib).name;
      if ( ::strstr(bnam,m_partition.c_str()) != 0 ) {
        int idx = (recv_node && evt_buff == bnam) ? 0 : (evt_buff == bnam) ? 1 : -1;
        if ( idx >= 0 )   {
          const MBMBuffer::Control& ctrl = (*ib).ctrl;
          ++numBuffs;
          tot_prod[idx] += ctrl.tot_produced;
          numClients    += (*ib).clients.size();
          num_cl[idx]   += (*ib).clients.size();
          num_sl[idx]   += ctrl.p_emax - ctrl.i_events;
          min_prod[idx]  = min(min_prod[idx],ctrl.tot_produced);
	  fsp = float(ctrl.i_space)/float(ctrl.bm_size);
	  fsl = float(ctrl.p_emax-ctrl.i_events)/float(ctrl.p_emax);
          fspace[idx]    = min(fspace[idx],fsp);
          fslots[idx]    = min(fslots[idx],fsl);
          if ( fsl < SLOTS_MIN || fsp < SPACE_MIN ) bad_nodes.insert((*n).name);
          m_inUse = true;
        }
      }
    }
  }

  RTL::Lock lock(InternalDisplay::screenLock());
  time_t now = ::time(0), t1 = numNodes == 0 ? now : c->firstUpdate().first;
  ::strftime(txt,sizeof(txt)," %H:%M:%S ",::localtime(&t1));
  ::sprintf(txt+strlen(txt),"%6d %6d %6d   ",numNodes,numBuffs,numClients);
  begin_update(txt);

  if ( numNodes != 0 ) {
    m_lastUpdate = t1;
  }

  if ( tot_prod[0] != 0 )
    ::sprintf(txt,"Evt:%9d Cl:%4d Sl:%5d%17s",tot_prod[0],num_cl[0],num_sl[0],"");
  else
    ::sprintf(txt,"%13s%8s%9s%64s","--","--","--","");
  ::scrc_put_chars(dis," Receive:",BOLD,pos,77+CLUSTERLINE_START,0);
  ::scrc_put_chars(dis,txt,NORMAL,pos,77+9+CLUSTERLINE_START,0);

  if ( tot_prod[1] != 0 )
    ::sprintf(txt,"Evt:%9d Cl:%4d Sl:%5d%17s",tot_prod[1],num_cl[1],num_sl[1],"");
  else
    ::sprintf(txt,"%13s%8s%9s%64s","--","--","--","");
  ::scrc_put_chars(dis,"  Stream:",BOLD,pos,77+37+CLUSTERLINE_START,0);
  ::scrc_put_chars(dis,txt,NORMAL,pos,77+46+CLUSTERLINE_START,0);

  m_hasProblems = true;
  if ( now-m_lastUpdate > UPDATE_TIME_MAX ) {
    err = " No update information available", col = RED|INVERSE|BOLD;
  }
  else if ( numNodes == 0 ) {
    err = " No nodes found!", col = BOLD|RED|INVERSE;
  }
  else if ( !m_inUse ) {
    err = " Storage not used yet....", col = NORMAL|INVERSE|GREEN;
  }
  else if ( fslots[0] < SLOTS_MIN || fslots[1] < SLOTS_MIN ) {
    ::sprintf(txt," SLOTS at limit:");
    if ( fslots[0] < SLOTS_MIN ) ::strcat(txt,"Recv ");
    if ( fslots[1] < SLOTS_MIN ) ::strcat(txt,"Stream ");
    ::sprintf(txt+strlen(txt),"[%d nodes]",int(bad_nodes.size()));
    err = txt, col = BOLD|RED|INVERSE;
  }
  else if ( fspace[0] < SPACE_MIN || fspace[1] < SPACE_MIN ) {
    ::sprintf(txt," SPACE at limit:");
    if ( fspace[0] < SPACE_MIN ) ::strcat(txt,"Recv ");
    if ( fspace[1] < SPACE_MIN ) ::strcat(txt,"Stream ");
    ::sprintf(txt+strlen(txt),"[%d nodes]",int(bad_nodes.size()));
    err = txt, col = BOLD|RED|INVERSE;
  }
  else if ( min_prod[0] != INT_max && min_prod[0]>0 && min_prod[0] <= m_evtRecv ) {
    err = " Receiving layer idle.", col = RED|BOLD;
  }
  else if ( min_prod[1] != INT_max && min_prod[1]>0 && min_prod[1] <= m_evtSent ) {
    err = " Streaming layer idle.", col = RED|BOLD;
  }
  else {
    err = " No obvious Errors detected....", col = NORMAL|INVERSE|GREEN;
  }

  err = err + "                                                                 ";
  ::scrc_put_chars(dis,err.substr(0,35).c_str(),col,pos,42+CLUSTERLINE_START,0);
  m_evtRecv = min_prod[0];
  m_evtSent = min_prod[1];
  m_totRecv = tot_prod[0];
  m_totSent = tot_prod[1];
  end_update();
}


/// Standard constructor
MonitoringClusterLine::MonitoringClusterLine(FarmLineDisplay* p, int pos, const std::string& n)
: ClusterLine(p,pos,n)
{
  m_numUpdate = 0;
  m_evtRelay  = m_totRelay = 0;
  m_evtWorker = m_totWorker = 0;
  m_partition = p->name();
  m_relayNode = m_name+"01";
  m_lastUpdate = time(0);
  m_hasProblems = false;
  connect(strlower(m_name)+"/ROpublish");
}


void MonitoringClusterLine::display() {
  char txt[256];
  string err = "";
  set<string> bad_nodes;
  size_t         pos = position();
  Display*       dis = m_parent->display();
  const Nodeset*   c = data<Nodeset>();
  const Nodes& nodes = c->nodes;
  string evt_buff = "Events_"+m_partition;
  string out_buff = "Output_"+m_partition;
  int numNodes = 0, numBuffs = 0, numClients = 0;
  int tot_prod[3] = {0,0,0}, num_cl[3] = {0,0,0}, num_sl[3] = {0,0,0};
  int min_prod[3] = {INT_max,INT_max,INT_max};
  float fsp, fspace[3] = {FLT_max,FLT_max,FLT_max};
  float fsl, fslots[3] = {FLT_max,FLT_max,FLT_max};

  m_inUse = false;
  for (Nodes::const_iterator n=nodes.begin(); n!=nodes.end(); n=nodes.next(n))  {
    bool relay = ::strncasecmp((*n).name,m_relayNode.c_str(),m_relayNode.length()) == 0;
    const Buffers& buffs = *(*n).buffers();
    ++numNodes;
    for(Buffers::const_iterator ib=buffs.begin(); ib!=buffs.end(); ib=buffs.next(ib))  {
      const char* bnam = (*ib).name;
      if ( ::strstr(bnam,m_partition.c_str()) != 0 ) {
        int idx = -1;
        if ( relay && evt_buff == bnam )
          idx = 0;
        else if ( !relay && evt_buff == bnam )
          idx = 1;
        else if ( !relay && out_buff == bnam )
          idx = 2;
        if ( idx >= 0 ) {
          const MBMBuffer::Control& ctrl = (*ib).ctrl;
          int ncl = (*ib).clients.size();
          numClients += ncl;
          ++numBuffs;
	  tot_prod[idx] += ctrl.tot_produced;
	  num_cl[idx]   += ncl;
	  num_sl[idx]   += ctrl.p_emax - ctrl.i_events;
          min_prod[idx]  = min(min_prod[idx],ctrl.tot_produced);
	  fsl            = float(ctrl.p_emax-ctrl.i_events)/float(ctrl.p_emax);
	  fsp            = float(ctrl.i_space)/float(ctrl.bm_size);
          fspace[idx]    = min(fspace[idx],fsp);
          fslots[idx]    = min(fslots[idx],fsl);
          if ( fsl < SLOTS_MIN || fsp < SPACE_MIN ) {
            bad_nodes.insert((*n).name);
          }
          m_inUse = true;
        }
      }
    }
  }

  RTL::Lock lock(InternalDisplay::screenLock());
  time_t now = time(0), t1 = numNodes == 0 ? time(0) : c->firstUpdate().first;
  ::strftime(txt,sizeof(txt)," %H:%M:%S ",::localtime(&t1));
  ::sprintf(txt+::strlen(txt),"%6d %6d %6d   ",numNodes,numBuffs,numClients);
  begin_update(txt);

  if ( numNodes != 0 ) {
    m_lastUpdate = t1;
  }
  m_hasProblems = true;

  if ( tot_prod[0] != 0 )
    ::sprintf(txt,"Evt:%9d Cl:%4d Sl:%5d%64s",tot_prod[0],num_cl[0],num_sl[0],"");
  else
    ::sprintf(txt,"%13s%8s%9s%64s","--","--","--","");
  ::scrc_put_chars(dis,"   Relay:",BOLD,pos,77+CLUSTERLINE_START,0);
  ::scrc_put_chars(dis,txt,NORMAL,pos,77+9+CLUSTERLINE_START,0);

  if ( tot_prod[1] != 0 && tot_prod[2] != 0 )
    ::sprintf(txt,"Evt:%9d Cl:%4d Sl:%5d Evt:%9d Cl:%4d Sl:%4d%64s",
	      tot_prod[1],num_cl[1],num_sl[1],tot_prod[2],num_cl[2],num_sl[2],"");
  else if ( tot_prod[1] != 0 )
    ::sprintf(txt,"Evt:%9d Cl:%4d Sl:%5d  %-62s",tot_prod[1],num_cl[1],num_sl[1],"No Output streams");
  else
    ::sprintf(txt,"%13s%8s%9s%64s","--","--","--","");
  ::scrc_put_chars(dis,"  Worker:",BOLD,pos,77+37+CLUSTERLINE_START,0);
  ::scrc_put_chars(dis,txt,NORMAL,pos,77+46+CLUSTERLINE_START,0);
  
  int col = NORMAL;
  if ( now-m_lastUpdate > UPDATE_TIME_MAX ) {
    err = " No update information available", col = RED|INVERSE|BOLD;
  }
  else if ( numNodes == 0 ) {
    err = " No nodes found in this cluster!", col = BOLD|RED|INVERSE;
  }
  else if ( !m_inUse ) {
    err = " Monitoring cluster not used yet....", col = NORMAL|INVERSE|GREEN;
  }
  else if ( fslots[0] < SLOTS_MIN || fslots[1] < SLOTS_MIN || fslots[2] < SLOTS_MIN ) {
    ::sprintf(txt," SLOTS at limit:");
    if ( fslots[0] < SLOTS_MIN ) ::strcat(txt,"Relay ");
    if ( fslots[1] < SLOTS_MIN ) ::strcat(txt,"Events ");
    if ( fslots[2] < SLOTS_MIN ) ::strcat(txt,"Output ");
    ::sprintf(txt+strlen(txt),"[%d nodes]",int(bad_nodes.size()));
    err = txt, col = BOLD|RED|INVERSE;
  }
  else if ( fspace[0] < SPACE_MIN || fspace[1] < SPACE_MIN || fspace[2] < SPACE_MIN  ) {
    ::sprintf(txt," SPACE at limit:");
    if ( fspace[0] < SPACE_MIN ) ::strcat(txt,"Relay ");
    if ( fspace[1] < SPACE_MIN ) ::strcat(txt,"Events ");
    if ( fspace[2] < SPACE_MIN ) ::strcat(txt,"Output ");
    ::sprintf(txt+strlen(txt),"[%d nodes]",int(bad_nodes.size()));
    err = txt, col = BOLD|RED|INVERSE;
  }
  else if ( tot_prod[0]>0 && tot_prod[0] <= m_totRelay ) {
    err = " No RELAY activity visible.", col = BOLD|RED;
  }
  else if ( tot_prod[1]>0 && tot_prod[1] <= m_totWorker ) {
    err = " No Worker activity visible.", col = BOLD|RED;
  }
  else if ( min_prod[0] != INT_max && min_prod[0]>0 && min_prod[0] <= m_evtRelay ) {
    err = " Relay node looks idle.", col = BOLD|RED;
  }
  else if ( min_prod[1] != INT_max && min_prod[1]>0 && min_prod[1] <= m_evtWorker ) {
    err = " Some workers idle around.", col = BOLD|RED;
  }
  else  {
    err = " No obvious Errors detected....", col = NORMAL|INVERSE|GREEN;
  }
  err = err + "                                                                 ";
  ::scrc_put_chars(dis,err.substr(0,35).c_str(),col,pos,42+CLUSTERLINE_START,0);

  m_evtRelay  = min_prod[0];
  m_totRelay  = tot_prod[0];
  m_evtWorker = min_prod[1];
  m_totWorker = tot_prod[1];

  end_update();
}


FarmClusterLine::FarmClusterLine(FarmLineDisplay* p, int pos, const std::string& n)
: ClusterLine(p,pos,n)
{
  m_numUpdate = 0;
  m_evtSent  = m_totSent = 0;
  m_evtMoore = m_totMoore = 0;
  m_evtBuilt = m_totBuilt = 0;
  m_lastUpdate = time(0);
  m_hasProblems = false;
  connect(strlower(m_name)+"/ROpublish");
}

void FarmClusterLine::display() {
  char txt[256];
  size_t pos = position();
  Display*       dis = m_parent->display();
  const Nodeset*   c = data<Nodeset>();
  const Nodes& nodes = c->nodes;
  int evt_prod[4]    = {0,0,0,0}, min_prod[4]  = {INT_max,INT_max,INT_max,INT_max};
  int free_space[4]  = {0,0,0,0}, min_space[4] = {INT_max,INT_max,INT_max,INT_max};
  int free_slots[4]  = {0,0,0,0}, min_slots[4] = {INT_max,INT_max,INT_max,INT_max};
  int buf_clients[4] = {0,0,0,0};
  float fspace[4]    = {FLT_max,FLT_max,FLT_max,FLT_max};
  float fslots[4]    = {FLT_max,FLT_max,FLT_max,FLT_max};
  float fsl, fsp;
  int evt_sent       = INT_max;
  int evt_moore      = INT_max;
  int evt_built      = INT_max;
  int numNodes       = 0;
  int numBuffs       = 0;
  int numClients     = 0;
  set<string> bad_nodes;
  int col = NORMAL;

  m_inUse = false;
  for (Nodes::const_iterator n=nodes.begin(); n!=nodes.end(); n=nodes.next(n))  {
    const Buffers& buffs = *(*n).buffers();
    numNodes++;
    int node_evt_mep = 0;
    int node_evt_sent = INT_max;
    int node_evt_moore = INT_max;

    const char* nn = (*n).name;
    txt[1] = nn[0];
    txt[2] = nn[1];
    col = NORMAL;
    ::sprintf(txt," %s ",nn+((::strncmp(nn,c->name,::strlen(c->name)+2) == 0) ? 0 : ::strlen(nn)-2));

    for(Buffers::const_iterator ib=buffs.begin(); ib!=buffs.end(); ib=buffs.next(ib))  {
      int idx = 0;
      char b = (*ib).name[0];
      const MBMBuffer::Control& ctrl = (*ib).ctrl;
      ++numBuffs;
      switch(b) {
      case MEP_BUFFER:        idx = 0; break;
      case EVT_BUFFER:        idx = 1; break;
      case RES_BUFFER:        idx = 2; break;
      case SND_BUFFER:        idx = 3; break;
      default:                continue;
      }
      m_inUse = true;
      fsp               = float(ctrl.i_space)/float(ctrl.bm_size);
      fsl               = float(ctrl.p_emax-ctrl.i_events)/float(ctrl.p_emax);
      fspace[idx]       = min(fspace[idx],fsp); 
      fslots[idx]       = min(fslots[idx],fsl);
      min_space[idx]    = min(min_space[idx],(ctrl.i_space*ctrl.bytes_p_Bit)/1024/1024);
      min_slots[idx]    = min(min_slots[idx],ctrl.p_emax-ctrl.i_events);
      min_prod[idx]     = min(min_prod[idx],ctrl.tot_produced);
      evt_prod[idx]    += ctrl.tot_produced;
      free_space[idx]  += (ctrl.i_space*ctrl.bytes_p_Bit)/1024/1024;
      free_slots[idx]  += (ctrl.p_emax-ctrl.i_events);
      buf_clients[idx] += ctrl.i_users;
      if ( fsl < SLOTS_MIN || fsp < SPACE_MIN ) {
        bad_nodes.insert((*n).name);
      }
      const Clients& clients = (*ib).clients;
      for (Clients::const_iterator ic=clients.begin(); ic!=clients.end(); ic=clients.next(ic))  {
        ++numClients;
        const char* p = _procNam((*ic).name);
        switch(*p) {
        case BUILDER_TASK:
          if( b == MEP_BUFFER ) {
            node_evt_mep += (*ic).events;
          }
          break;
        case SENDER_TASK:
          if( b == RES_BUFFER || b == SND_BUFFER )  {
            node_evt_sent = min(node_evt_sent,(*ic).events);
          }
          break;
        case MOORE_TASK:
          //  Normal  and        TAE event processing
          if( b == EVT_BUFFER || b == MEP_BUFFER )  {
            node_evt_moore = min(node_evt_moore,(*ic).events);
          }
          break;
        default:
          break;
        }
      }
    }
    evt_moore = min(evt_moore,node_evt_moore);
    evt_built = min(evt_built,node_evt_mep);
    evt_sent  = min(evt_sent,node_evt_sent);
  }

  RTL::Lock lock(InternalDisplay::screenLock());
  time_t t1 = numNodes == 0 ? time(0) : c->firstUpdate().first, prev_update=m_lastUpdate;
  ::strftime(txt,sizeof(txt)," %H:%M:%S ",::localtime(&t1));
  ::sprintf(txt+::strlen(txt),"%6d %6d %6d   ",numNodes,numBuffs,numClients);
  begin_update(txt);

  if ( numNodes != 0 ) {
    m_lastUpdate = t1;
  }
  m_hasProblems = true;
  // If Result buffer is not in use
  if ( buf_clients[3] != 0 && buf_clients[2] == 0 )  {
    buf_clients[2] = buf_clients[3];
    fspace[2]      = fspace[3];
    fslots[2]      = fslots[3];
    min_space[2]   = min_space[3];
    min_slots[2]   = min_slots[3];
    min_prod[2]    = min_prod[3];
    evt_prod[2]    = evt_prod[3];
    free_space[2]  = free_space[3];
    free_slots[2]  = free_slots[3];
  }

  bool slots_min = fslots[0] < SLOTS_MIN || fslots[1] < SLOTS_MIN || fslots[2] < SLOTS_MIN;
  bool space_min = fspace[0] < SPACE_MIN || fspace[1] < SPACE_MIN || fspace[2] < SPACE_MIN;

  col = NORMAL;
  string err = "";
  if ( prev_update-m_lastUpdate > UPDATE_TIME_MAX ) {
    err = " No update information available", col = RED|INVERSE|BOLD;
  }
  else if ( numNodes == 0 ) {
    err = " No nodes found in this subfarm!", col =  RED|INVERSE|BOLD;
  }
  else if ( !m_inUse ) {
    err = " Subfarm not used by any partition....", col = NORMAL|INVERSE|GREEN;
  }
  else if ( evt_built <= m_evtBuilt && evt_prod[0]<m_totBuilt ) {
    err = " Some MEPRx(s) stuck.", col = BOLD|RED|INVERSE;
  }
  else if ( evt_built <= m_evtBuilt && evt_prod[0] == m_totBuilt ) {
    err = " No DAQ activity visible.", col = BOLD|RED;
  }
  else if ( evt_moore+2 <= m_evtMoore && evt_prod[1] > m_totMoore ) {
    err = " Some MOORE(s) stuck.", col = BOLD|RED|INVERSE;
  }
  else if ( evt_moore <= m_evtMoore && evt_prod[1] == m_totMoore ) {
    err = " No HLT activity visible.", col = BOLD|RED;
  }
  else if ( evt_sent+2 <= m_evtSent && evt_prod[2] > m_totSent ) {
    err = " Some Sender(s) stuck.", col = BOLD|RED|INVERSE;
  }
  else if ( evt_sent <= m_evtSent && evt_prod[0] == m_totSent ) {
    err = " No STORAGE activity visible.", col = BOLD|RED;
  }
  else if ( slots_min  ) {
    int nbad = int(bad_nodes.size());
    ::sprintf(txt,"SLOTS at limit:");
    if ( fslots[0] < SLOTS_MIN ) ::strcat(txt,"MEP ");
    if ( fslots[1] < SLOTS_MIN ) ::strcat(txt,"EVENT ");
    if ( fslots[2] < SLOTS_MIN ) ::strcat(txt,"RES/SEND ");
    ::sprintf(txt+strlen(txt),"[%d nodes]",nbad);
    // We have 11 slow nodes in a farm: if these are full, this is no error
    err = txt, col = INVERSE|(nbad>0 ? GREEN : RED);
  }
  else if ( space_min  ) {
    int nbad = int(bad_nodes.size());
    ::sprintf(txt,"SPACE at limit:");
    if ( fspace[0] < SPACE_MIN ) ::strcat(txt,"MEP ");
    if ( fspace[1] < SPACE_MIN ) ::strcat(txt,"EVENT ");
    if ( fspace[2] < SPACE_MIN ) ::strcat(txt,"RES/SEND ");
    ::sprintf(txt+strlen(txt),"[%d nodes]",nbad);
    // We have 11 slow nodes in a farm: if these are full, this is no error
    err = txt, col = INVERSE|(nbad>0 ? GREEN : RED);
  }
  else  {
    err = " No obvious Errors detected....", col=NORMAL|INVERSE|GREEN;
    m_hasProblems = false;
  }
  m_evtBuilt  = evt_built;
  m_evtMoore  = evt_moore;
  m_evtSent   = evt_sent;
  m_totBuilt  = evt_prod[0];
  m_totMoore  = evt_prod[1];
  m_totSent   = evt_prod[2];

  err = err + "                                                                 ";
  ::scrc_put_chars(dis,err.substr(0,35).c_str(),col,pos,42+CLUSTERLINE_START,0);
  if ( evt_prod[0] != 0 )
    ::sprintf(txt,"%9d%5d%11d%6d%9d%5d",
              evt_prod[0],free_slots[0],
              evt_prod[1],free_slots[1],
              evt_prod[2],free_slots[2]);
  else
    ::sprintf(txt,"%9s%5s%10s%7s%9s%5s","--","--","--","--","--","--");
  ::scrc_put_chars(dis,txt,NORMAL,pos,77+CLUSTERLINE_START,0);
  if ( min_prod[0] != INT_max )
    ::sprintf(txt,"%9d%5d%11d%6d%9d%5d",
              min_prod[0],min_slots[0],
              min_prod[1],min_slots[1],
              min_prod[2],min_slots[2]);
  else
    ::sprintf(txt,"%9s%5s%10s%7s%9s%5s","--","--","--","--","--","--");
  ::scrc_put_chars(dis,txt,NORMAL,pos,77+47+CLUSTERLINE_START,0);
  end_update();
}

#include "ROMon/CPUMon.h"

HltDeferLine::HltDeferLine(FarmLineDisplay* p, int pos, const std::string& n)
: ClusterLine(p,pos,n)
{
  m_numUpdate   = 0;
  m_numFiles    = 0;
  m_numRuns     = 0;
  m_lastUpdate  = time(0);
  m_hasProblems = false;
  connect(strlower(m_name)+"/ROpublish/HLTDefer");
  string svc = "HLT/ExcludedNodes/"+strupper(m_name);
  m_exclID   = ::dic_info_service((char*)svc.c_str(),MONITORED,0,0,0,excludedHandler,(long)this,0,0);
}

HltDeferLine::~HltDeferLine() {
  if ( m_exclID ) ::dic_release_service(m_exclID);
  m_exclID = 0;
}

/// Interactor overload: Display callback handler
void HltDeferLine::handle(const Event& ev) {
  switch(ev.eventtype) {
  case IocEvent:
    switch(ev.type) {
    case CMD_EXCLUDE:
      m_excluded = *auto_ptr<StrSet>(ev.iocPtr<StrSet>()).get();
      if ( 0 == m_numUpdate ) {
	RTL::Lock lock(InternalDisplay::screenLock());
	begin_update("          -- No Subfarm information retrieved yet --");
	end_update();
      }
      return;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

/// DIM command service callback
void HltDeferLine::excludedHandler(void* tag, void* address, int* size) {
  if ( tag ) {
    HltDeferLine* l = *(HltDeferLine**)tag;
    set<string> nodes;
    if ( address && *size > 0 ) {
      char *p = (char*)address, *end = p+*size;
      while(p<end) {
	nodes.insert(strlower(p));
	p += (::strlen(p)+1);
      }
    }
    IocSensor::instance().send(l,CMD_EXCLUDE,new set<string>(nodes));
  }
}

void HltDeferLine::display() {
  typedef DeferredHLTSubfarmStats HLTStats;
  typedef HLTStats::Nodes Nodes;
  typedef HLTStats::Runs Runs;
  char txt[256];
  size_t pos = 66, numNodes = 0, numRuns = 0, numFiles = 0;
  Display*       dis = m_parent->display();
  const HLTStats*  s = data<HLTStats>();
  const Runs&   runs = s->runs;
  const Nodes* nodes = s->nodes();
  int col = NORMAL, line = position();
  string val;

  ++m_numUpdate;
  for (Runs::const_iterator ri=runs.begin(); ri!=runs.end(); ri=runs.next(ri))  {
    numFiles += (*ri).second;
    ++numRuns;
  }
  for (Nodes::const_iterator ni=nodes->begin(); ni!=nodes->end(); ni=nodes->next(ni))  {
    ++numNodes;
  }

  RTL::Lock lock(InternalDisplay::screenLock());
  time_t t1 = numNodes == 0 ? time(0) : s->firstUpdate().first;
  ::strftime(txt,sizeof(txt)," %H:%M:%S ",::localtime(&t1));
  ::sprintf(txt+strlen(txt),"%6ld %6ld %6ld ",numNodes, numRuns, numFiles);
  begin_update(txt);

  if ( numNodes != 0 ) m_lastUpdate = t1;

  if ( numRuns>0 )
    ::scrc_put_chars(dis,"  Processing HLT  ",COL_WARNING,line,46,0);
  else
    ::scrc_put_chars(dis,"       DONE       ",COL_OK,line,46,0);
  val = " "+string(s->name)+" ";
  ::scrc_put_chars(dis,val.c_str(),numRuns>0 ? COL_WARNING : COL_OK,line,pos,0);
  pos += val.length();
  ::scrc_put_chars(dis,"  ",NORMAL,line,pos,0);
  pos += 2;
  if ( nodes->size() > 0 ) {
    for (Nodes::const_iterator ni=nodes->begin(); ni!=nodes->end(); ni=nodes->next(ni))  {
      const DeferredHLTStats& n = *ni;
      string nn = n.name;
      if ( nn.length() > 0 && nn != s->name ) {
	const Runs& nr = n.runs;
	bool excl = m_excluded.find(nn) != m_excluded.end();
	col = nr.size()==0 ? COL_OK : COL_WARNING;
	if ( excl )  {
	  col = INVERSE|(col==COL_WARNING ? MAGENTA : BLUE);
	}
	val = " "+nn.substr(nn.length()-2);
	::scrc_put_chars(dis,val.c_str(),col,line,pos,0);
	pos += val.length();
      }
    }
  }
  ::scrc_put_chars(dis," ",COL_OK,line,pos,1);
  end_update();
  IocSensor::instance().send(m_parent,CMD_SHOWDEFERREDRUNS,this);
}

