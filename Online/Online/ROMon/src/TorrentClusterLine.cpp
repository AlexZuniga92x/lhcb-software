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
namespace BitTorrent  {

  /**@class TorrentClusterLine ROMon.h GaudiOnline/FarmDisplay.h
   *
   *   Display summarizing one single subfarm. Showed as an array on the
   *   main display.
   *
   *   @author M.Frank
   */
  class TorrentClusterLine : public ROMon::ClusterLine, public Interactor  {
    typedef std::set<std::string> StrSet;
    /// Set of excluded nodes
    StrSet              m_excluded;
    /// Service ID for excluded nodes
    int                 m_exclID;
    int               m_numUpdate;
    bool              m_hasProblems;
    bool              m_inUse;
    /// Timestamp with last update
    time_t            m_lastUpdate;
    /// DIM command service callback
    static void excludedHandler(void* tag, void* address, int* size);
  public:
    /// Initializing constructor
    TorrentClusterLine(ROMon::FarmLineDisplay* p, int pos, const std::string& n);
    /// Default destructor
    virtual ~TorrentClusterLine();
    /// Interactor overload: Display callback handler
    void handle(const Event& ev);
    /// Display function drawing on pasteboard of current display
    void display();
  };
}

#include "ROMon/TorrentMon.h"
#include "ROMon/FarmLineDisplay.h"
#include "CPP/IocSensor.h"
#include "CPP/Event.h"
#include "RTL/Lock.h"
#include "WT/wtdef.h"
#include "SCR/scr.h"

#include <limits>
extern "C" {
#include "dic.h"
}


using namespace BitTorrent;
using namespace ROMon;
using namespace SCR;
using namespace std;
typedef vector<string>               StringV;

#define UPDATE_TIME_MAX        15
#define CLUSTERLINE_START      2
#define COL_WARNING         (RED|BOLD)
#define COL_ALARM           (RED|BOLD|INVERSE)
#define INT_max  numeric_limits<int>::max()
#define FLT_max  numeric_limits<float>::max()
namespace ROMon {
  std::string strupper(const std::string& n);
  std::string strlower(const std::string& n);
}

namespace {
  ClusterLineFactory<TorrentClusterLine>       s_farmLineFactory("TorrentFarm");
}

TorrentClusterLine::TorrentClusterLine(FarmLineDisplay* p, int pos, const std::string& n)
: ClusterLine(p,pos,n)
{
  string info = "/"+strupper(m_name)+"/TorrentInfo";
  m_numUpdate = 0;
  m_lastUpdate = time(0);
  m_hasProblems = false;
  //connect(info);
  m_svc        = ::dic_info_service((char*)info.c_str(),MONITORED,0,0,0,dataHandler,(long)this,0,0);
  string svc   = "HLT/ExcludedNodes/"+strupper(m_name);
  m_exclID     = ::dic_info_service((char*)svc.c_str(),MONITORED,0,0,0,excludedHandler,(long)this,0,0);
  Display* dis = m_parent->display();
  begin_update();
  ::scrc_put_chars(dis," --------------------------------------------------------------------",BOLD,pos,12,0);
  ::scrc_put_chars(dis,("---- No torrent information availible:"+info+
			" ------------------------------------------------------------------------").c_str(),
		   RED|BOLD,pos,85+CLUSTERLINE_START,1);
  end_update();
}

/// Default destructor
TorrentClusterLine::~TorrentClusterLine() {
  if ( m_exclID ) ::dic_release_service(m_exclID);
  m_exclID = 0;
}

/// DIM command service callback
void TorrentClusterLine::excludedHandler(void* tag, void* address, int* size) {
  if ( tag ) {
    TorrentClusterLine* l = *(TorrentClusterLine**)tag;
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

/// Interactor overload: Display callback handler
void TorrentClusterLine::handle(const Event& ev) {
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

void TorrentClusterLine::display() {
  typedef SessionStatus::Torrents Torrents;
  typedef SubfarmTorrentStatus::Sessions Sessions;

  char txt[256];
  string nam, val, torrent;
  size_t pos, line = position();
  Display*     dis = m_parent->display();
  const SubfarmTorrentStatus* sf = data<SubfarmTorrentStatus>();
  TorrentStatus sum;

  size_t num_session = sf->sessions.size();
  time_t now = ::time(0), t1 = num_session == 0 ? now : sf->firstUpdate().first;
  ::strftime(txt,sizeof(txt)," %H:%M:%S ",::localtime(&t1));
  ::sprintf(txt+strlen(txt)," %6d ",int(num_session));
  begin_update(txt);

  ::memset(&sum,0,sizeof(sum));
  pos = 85+CLUSTERLINE_START;
  for(Sessions::const_iterator i=sf->sessions.begin(); i!=sf->sessions.end(); i=sf->sessions.next(i)) {
    int col = COL_ALARM;
    const SessionStatus& s = *i;
    for(Torrents::const_iterator j=s.torrents.begin(); j!=s.torrents.end();j=s.torrents.next(j))   {
      const TorrentStatus& t = *j;
      if ( torrent.empty() ) torrent = t.name;
      switch(t.state) {
      case TorrentStatus::queued_for_checking:
	col = COL_WARNING;
	break;
      case TorrentStatus::allocating:
      case TorrentStatus::checking_resume_data:
      case TorrentStatus::checking_files:
      case TorrentStatus::downloading_metadata:
      case TorrentStatus::downloading:
	col = FG_YELLOW|BOLD|BG_BLACK;
	break;
      case TorrentStatus::finished:
      case TorrentStatus::seeding:
	col = GREEN|INVERSE;
	break;
      default:
	col = COL_ALARM;
	break;
      }
      if ( t.name != torrent ) col = COL_ALARM;
      sum.state            += 1;
      sum.num_peers        += t.num_peers;
      sum.num_uploads      += t.num_uploads;
      sum.num_pieces_total += t.num_pieces_total;
      sum.num_pieces_done  += t.num_pieces_done;
      sum.total_done       += t.total_done;
      sum.total_wanted     += t.total_wanted;
      sum.total_upload     += t.total_upload;
      sum.total_download   += t.total_download;
      sum.progress         += t.progress;
      break;
    }
    bool excl = m_excluded.find(s.name) != m_excluded.end();
    if ( excl ) col = INVERSE|BLUE;
    nam = s.name;
    val = (nam == m_name ? " "+nam : " "+nam.substr(nam.length()-2));
    ::scrc_put_chars(dis,val.c_str(),col,line,pos,0);
    pos += val.length();
  }
  ::scrc_put_chars(dis," ",GREEN|INVERSE,line,pos,1);

  ::sprintf(txt,"%6d",int(num_session));
  ::scrc_put_chars(dis,txt,NORMAL,line,33,0);

  sum.progress /= float(num_session);
  ::sprintf(txt,"%5.1f",sum.progress*100.);
  ::scrc_put_chars(dis,txt,sum.progress>0.99999 ? GREEN|INVERSE : RED,line,31,0);

  ::sprintf(txt,"%6d",sum.num_uploads);
  ::scrc_put_chars(dis,txt,NORMAL,line,37,0);

  ::sprintf(txt,"%4d",sum.num_peers);
  ::scrc_put_chars(dis,txt,NORMAL,line,45,0);

  ::sprintf(txt,"%6d/%-6d",sum.num_pieces_done,sum.num_pieces_total);
  ::scrc_put_chars(dis,txt,sum.num_pieces_done==sum.num_pieces_total ? NORMAL : RED,line,50,0);

  ::sprintf(txt,"%8ld",sum.total_download/1024/1024);
  ::scrc_put_chars(dis,txt,NORMAL,line,64,0);
  ::sprintf(txt,"%8ld",sum.total_upload/1024/1024);
  ::scrc_put_chars(dis,txt,NORMAL,line,73,0);

  end_update();
}
