#include "ROMon/FarmDisplay.h"

namespace ROMon {

  /**@class FarmSubDisplay ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Display summarizing one single subfarm. Showed as an array on the
   *   main display.
   *
   *   @author M.Frank
   */
  class FarmSubDisplay : public InternalDisplay  {
    int               m_evtBuilt;
    int               m_evtMoore;
    int               m_evtSent;
    int               m_totBuilt;
    int               m_totMoore;
    int               m_totSent;
    int               m_numUpdate;
    bool              m_hasProblems;
    bool              m_inUse;

  public:
    /// Access to problem flag
    bool hasProblems() const { return m_hasProblems; }
    /// Set timeout error
    void setTimeoutError();
    /// Initializing constructor
    FarmSubDisplay(FarmDisplay* parent, const std::string& title, bool bad=false);
    /// Standard destructor
    virtual ~FarmSubDisplay();
    /// Initialize default display text
    void init(bool bad);
    /// Update display content
    virtual void update(const void* data);
    /// Update display content
    virtual void update(const void* data, size_t len)
    { this->InternalDisplay::update(data,len); }
    /// Check display for errors
    virtual void check(time_t stamp);
    /// Set the focus to this display
    virtual void setFocus();
    /// Release the focus of this display
    virtual void releaseFocus();
    /// Interactor overload: Display callback handler
    virtual void handle(const Event& ev);
    /// Update display content
    virtual void updateContent(const Nodeset& ns);
  };

  InternalDisplay* createFarmSubDisplay(FarmDisplay* parent, const std::string& title) {
    return new FarmSubDisplay(parent,title);
  }
}
#include <set>
#include <limits>
#include "CPP/IocSensor.h"
#include "SCR/MouseSensor.h"
#include "SCR/scr.h"
extern "C" {
#include "dic.h"
}

using namespace ROMon;
using namespace SCR;
using namespace std;
// Max. 15 seconds without update allowed
#define UPDATE_TIME_MAX 15


namespace {
  const char* _procNam(const char* nam) {
    char* p;
    if (0 != ::strstr(nam,"MEPRx") ) return nam;
    p = ::strchr(nam,'_');
    if ( 0 != p ) return ++p;
    return "Unknown";
  }
}

/// Initializing constructor
FarmSubDisplay::FarmSubDisplay(FarmDisplay* parent, const string& title, bool bad) 
: InternalDisplay(parent, title)
{
  m_numUpdate = 0;
  m_evtSent  = m_totSent = 0;
  m_evtMoore = m_totMoore = 0;
  m_evtBuilt = m_totBuilt = 0;
  m_lastUpdate = time(0);
  ::scrc_create_display(&m_display,4,48,NORMAL,ON,m_title.c_str());
  init(bad);
  string svc = "/";
  for(size_t i=0; i<title.length();++i) svc += ::tolower(title[i]);
  svc += "/ROpublish";
  m_svc = ::dic_info_service((char*)svc.c_str(),MONITORED,0,0,0,dataHandler,(long)this,0,0);
  m_hasProblems = false;
  MouseSensor::instance().add(this,m_display);
}

/// Standard destructor
FarmSubDisplay::~FarmSubDisplay() {
  MouseSensor::instance().remove(m_display);
}

/// Initialize default display text
void FarmSubDisplay::init(bool bad) {
  int col = bad ? INVERSE|RED : NORMAL;
  char txt[128];
  ::sprintf(txt,"%-4s%9s %3s%10s %2s%4s%9s%5s",
            "","MEP","Sl","EVENT","Cl","Sl","SEND","Sl");
  ::scrc_put_chars(m_display,txt,col|INVERSE,1,1,1);
  //::scrc_put_chars(m_display,txt,col|BOLD,1,1,1);
  ::scrc_put_chars(m_display," ",col,2,1,1);
  ::scrc_put_chars(m_display," ",col,3,1,1);
  ::scrc_put_chars(m_display," ",col,4,1,1);
  ::scrc_put_chars(m_display,"Tot:",BOLD,2,1,1);
  ::scrc_put_chars(m_display,"Min:",BOLD,3,1,1);
  ::scrc_put_chars(m_display,"Max:",BOLD,4,1,1);
  ::scrc_put_chars(m_display,"  ",col,2,46,0);
  ::scrc_put_chars(m_display,"  ",col,3,45,0);
  ::scrc_put_chars(m_display,"  ",col,4,44,0);
  ::scrc_set_border(m_display,m_title.c_str(),col|BOLD);
}

/// DIM command service callback
void FarmSubDisplay::update(const void* address) {
  const Nodeset* ns = (const Nodeset*)address;
  if ( ns->type == Nodeset::TYPE ) {
    updateContent(*ns);
  }
}

/// Set timeout error
void FarmSubDisplay::setTimeoutError() {
  ::scrc_set_border(m_display,m_title.c_str(),INVERSE|RED);
  ::scrc_put_chars(m_display," No update information available",BOLD|RED|INVERSE,4,1,1);
}

/// Check display for errors
void FarmSubDisplay::check(time_t now) {
  if ( hasProblems() ) {
    if ( now - lastUpdate() > UPDATE_TIME_MAX ) {
      setTimeoutError();
    }
  }
}

/// Set the focus to this display
void FarmSubDisplay::setFocus() {
  ::scrc_set_border(m_display,m_title.c_str(),INVERSE|BLUE);
}

/// Release the focus of this display
void FarmSubDisplay::releaseFocus() {
}

/// Interactor overload: Display callback handler
void FarmSubDisplay::handle(const Event& ev) {
  switch(ev.eventtype) {
  case ScrMouseEvent: {
    const MouseEvent* m = ev.get<MouseEvent>();
    if ( m->button == 2 ) {
      IocSensor::instance().send(parent(),CMD_SHOWHELP,this);
      return;
    }
    setFocus();
    IocSensor::instance().send(parent(),m->msec == (unsigned int)-1 ? CMD_POSCURSOR : CMD_SHOW,this);
    return;
  }
  default:
    break;
  }
  InternalDisplay::handle(ev);
}

/// Update display content
void FarmSubDisplay::updateContent(const Nodeset& ns) {
  typedef MBMBuffer::Clients           Clients;
  typedef Node::Buffers                Buffers;
  typedef Nodeset::Nodes               Nodes;
  static const int   INT_max = numeric_limits<int>::max();
  static const float FLT_max = numeric_limits<float>::max();

  char txt[128], text[128];
  int evt_prod[3]    = {0,0,0}, min_prod[3]  = {INT_max,INT_max,INT_max};
  int free_space[3]  = {0,0,0}, min_space[3] = {INT_max,INT_max,INT_max};
  int used_slots[3]  = {0,0,0}, min_slots[3] = {INT_max,INT_max,INT_max};
  int buf_clients[3] = {0,0,0};
  float fspace[3]    = {FLT_max,FLT_max,FLT_max};
  float fslots[3]    = {FLT_max,FLT_max,FLT_max};
  int evt_sent       = INT_max;
  int evt_moore      = INT_max;
  int evt_built      = INT_max;
  bool inuse         = false;
  int numNodes       = 0;
  int numBuffs       = 0;
  int numClients     = 0;
  set<string> bad_nodes;

  for (Nodes::const_iterator n=ns.nodes.begin(); n!=ns.nodes.end(); n=ns.nodes.next(n))  {
    const Buffers& buffs = *(*n).buffers();
    numNodes++;
    int node_evt_mep = 0;
    int node_evt_sent = INT_max;
    int node_evt_moore = INT_max;
    for(Buffers::const_iterator ib=buffs.begin(); ib!=buffs.end(); ib=buffs.next(ib))  {
      int idx = 0;
      char b = (*ib).name[0];
      const MBMBuffer::Control& ctrl = (*ib).ctrl;
      ++numBuffs;
      switch(b) {
      case MEP_BUFFER:        idx = 0; break;
      case EVT_BUFFER:        idx = 1; break;
      case RES_BUFFER:
      case SND_BUFFER:        idx = 2; break;
      default:                   continue;
      }
      inuse = true;
      fspace[idx]       = min(fspace[idx],float(ctrl.i_space)/float(ctrl.bm_size)); 
      fslots[idx]       = min(fslots[idx],float(ctrl.p_emax-ctrl.i_events)/float(ctrl.p_emax));
      min_space[idx]    = min(min_space[idx],(ctrl.i_space*ctrl.bytes_p_Bit)/1024/1024);
      min_slots[idx]    = min(min_slots[idx],ctrl.p_emax-ctrl.i_events);
      min_prod[idx]     = min(min_prod[idx],ctrl.tot_produced);
      evt_prod[idx]    += ctrl.tot_produced;
      free_space[idx]  += (ctrl.i_space*ctrl.bytes_p_Bit)/1024/1024;
      used_slots[idx]  += (ctrl.p_emax-ctrl.i_events);
      buf_clients[idx] += ctrl.i_users;
      if ( fslots[idx] < SLOTS_MIN || fspace[idx] < SPACE_MIN ) {
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
  char b1[64];
  Nodeset::TimeStamp frst=ns.firstUpdate();
  time_t t1 = numNodes == 0 ? time(0) : frst.first, now = time(0);
  ::strftime(b1,sizeof(b1),"%H:%M:%S",::localtime(&t1));
  ::sprintf(text," %s %s [%d nodes %d buffers %d clients] ",
            m_name.c_str(),b1,numNodes,numBuffs,numClients);
  m_title = text;
  m_inUse = inuse;

  if ( numNodes != 0 ) {
    m_lastUpdate = t1;
  }
  m_hasProblems = true;
  if ( now-m_lastUpdate > UPDATE_TIME_MAX ) {
    setTimeoutError();
  }
  else if ( numNodes == 0 ) {
    ::scrc_set_border(m_display,m_title.c_str(),INVERSE|RED);
    ::scrc_put_chars(m_display," No nodes found in this subfarm!",BOLD|RED|INVERSE,4,1,1);
  }
  else if ( !inuse ) {
    ::scrc_set_border(m_display,m_title.c_str(),NORMAL);
    ::scrc_put_chars(m_display," Subfarm not used by any partition....",NORMAL|INVERSE|GREEN,4,1,1);
  }
  else if ( fslots[0] < SLOTS_MIN || fslots[1] < SLOTS_MIN || fslots[2] < SLOTS_MIN ) {
    ::scrc_set_border(m_display,m_title.c_str(),INVERSE|RED);
    ::sprintf(txt," SLOTS at limit:");
    if ( fslots[0] < SLOTS_MIN ) ::strcat(txt,"MEP ");
    if ( fslots[1] < SLOTS_MIN ) ::strcat(txt,"EVENT ");
    if ( fslots[2] < SLOTS_MIN ) ::strcat(txt,"RES/SEND ");
    ::sprintf(text,"[%d nodes]",int(bad_nodes.size()));
    ::strcat(txt,text);
    ::scrc_put_chars(m_display,txt,BOLD|RED|INVERSE,4,1,1);
  }
  else if ( fspace[0] < SPACE_MIN || fspace[1] < SPACE_MIN || fspace[2] < SPACE_MIN  ) {
    ::scrc_set_border(m_display,m_title.c_str(),INVERSE|RED);
    ::sprintf(txt," SPACE at limit:");
    if ( fspace[0] < SPACE_MIN ) ::strcat(txt,"MEP ");
    if ( fspace[1] < SPACE_MIN ) ::strcat(txt,"EVENT ");
    if ( fspace[2] < SPACE_MIN ) ::strcat(txt,"RES/SEND ");
    ::sprintf(text,"[%d nodes]",int(bad_nodes.size()));
    ::strcat(txt,text);
    ::scrc_put_chars(m_display,txt,BOLD|RED|INVERSE,4,1,1);
  }
  else if ( evt_built <= m_evtBuilt && evt_prod[0]<m_totBuilt ) {
    ::scrc_set_border(m_display,m_title.c_str(),INVERSE|RED);
    ::sprintf(txt," Some MEPRx(s) stuck.");
    ::scrc_put_chars(m_display,txt,BOLD|RED|INVERSE,4,1,1);
  }
  else if ( evt_built <= m_evtBuilt && evt_prod[0] == m_totBuilt ) {
    ::scrc_set_border(m_display,m_title.c_str(),NORMAL);
    ::scrc_put_chars(m_display," No DAQ activity visible.",BOLD|RED,4,1,1);
  }
  else if ( evt_moore+2 <= m_evtMoore && evt_prod[1] > m_totMoore ) {
    ::scrc_set_border(m_display,m_title.c_str(),INVERSE|RED);
    ::scrc_put_chars(m_display," Some MOORE(s) stuck.",BOLD|RED|INVERSE,4,1,1);
  }
  else if ( evt_moore <= m_evtMoore && evt_prod[1] == m_totMoore ) {
    ::scrc_set_border(m_display,m_title.c_str(),NORMAL);
    ::scrc_put_chars(m_display," No HLT activity visible.",BOLD|RED,4,1,1);
  }
  else if ( evt_sent+2 <= m_evtSent && evt_prod[2] > m_totSent ) {
    ::scrc_set_border(m_display,m_title.c_str(),INVERSE|RED);
    ::scrc_put_chars(m_display," Some Sender(s) stuck.",BOLD|RED|INVERSE,4,1,1);
  }
  else if ( evt_sent <= m_evtSent && evt_prod[0] == m_totSent ) {
    ::scrc_set_border(m_display,m_title.c_str(),NORMAL);
    ::scrc_put_chars(m_display," No STORAGE activity visible.",BOLD|RED,4,1,1);
  }
  else {
    ::scrc_set_border(m_display,m_title.c_str(),NORMAL);
    ::scrc_put_chars(m_display," No obvious Errors detected....",NORMAL|INVERSE|GREEN,4,1,1);
    m_hasProblems = false;
  }
  m_evtBuilt  = evt_built;
  m_evtMoore  = evt_moore;
  m_evtSent   = evt_sent;
  m_totBuilt  = evt_prod[0];
  m_totMoore  = evt_prod[1];
  m_totSent   = evt_prod[2];

  if ( evt_prod[0] != 0 )
    ::sprintf(txt,"%9d%4d%10d%3d%4d%9d%5d",
              evt_prod[0],used_slots[0],
              evt_prod[1],buf_clients[1],used_slots[1],
              evt_prod[2],used_slots[2]);
  else
    ::sprintf(txt,"%9s%4s%10s%7s%9s%5s","--","--","--","--","--","--");
  ::scrc_put_chars(m_display,txt,NORMAL,2,5,1);
  if ( min_prod[0] != INT_max )
    ::sprintf(txt,"%9d%4d%10d%7d%9d%5d",
              min_prod[0],min_slots[0],
              min_prod[1],min_slots[1],
              min_prod[2],min_slots[2]);
  else
    ::sprintf(txt,"%9s%4s%10s%7s%9s%5s","--","--","--","--","--","--");
  ::scrc_put_chars(m_display,txt,NORMAL,3,5,1);
  IocSensor::instance().send(m_parent,CMD_CHECK,this);
}
