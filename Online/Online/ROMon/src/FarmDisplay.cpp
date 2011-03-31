// $Id: FarmDisplay.cpp,v 1.51 2010-10-21 06:04:22 frankb Exp $
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
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/ROMon/src/FarmDisplay.cpp,v 1.51 2010-10-21 06:04:22 frankb Exp $

// Framework include files
#include "ROMon/ClusterDisplay.h"
#include "ROMon/FarmDisplay.h"
#include "ROMon/CPUMon.h"
#include "SCR/MouseSensor.h"
#include "CPP/TimeSensor.h"
#include "CPP/IocSensor.h"
#include "CPP/Event.h"
#include "RTL/Lock.h"
#include "RTL/strdef.h"
#include "SCR/scr.h"
#include "WT/wtdef.h"
#include "ROMonDefs.h"
extern "C" {
#include "dic.h"
}

// C++ include files
#include <set>
#include <limits>
#include <cstdio>
#include <cstdlib>
#include <cstdarg>
#include <fstream>
#include <iostream>
#include <algorithm>

using namespace ROMon;
using namespace SCR;
using namespace std;

typedef Nodeset::Nodes               Nodes;
typedef Node::Buffers                Buffers;
typedef MBMBuffer::Clients           Clients;
typedef Node::Tasks                  Tasks;
typedef vector<string>               StringV;

// Max. 15 seconds without update allowed
#define UPDATE_TIME_MAX 15

static FarmDisplay* s_fd = 0;

static void help() {
  cout << "  romon_farm -option [-option]" << endl
            << "       -all                         Show all subfarms." << endl
            << "       -p[artition]=<name>          Partition name providing monitoring information." << endl
            << "       -an[chor]=+<x-pos>+<ypos>    Set anchor for sub displays" << endl
            << endl;
}

namespace ROMon {
  InternalDisplay* createFarmSubDisplay(FarmDisplay* parent, const string& title);
  InternalDisplay* createRecFarmSubDisplay(FarmDisplay* parent, const string& title);
  InternalDisplay* createCtrlFarmSubDisplay(FarmDisplay* parent, const string& title);
  InternalDisplay* createMonitoringSubDisplay(FarmDisplay* parent, const string& title);
  InternalDisplay* createStorageSubDisplay(FarmDisplay* parent, const string& title);
  InternalDisplay* createBootDisplay(InternalDisplay* parent, const string& title);
  InternalDisplay* createBenchmarkDisplay(InternalDisplay* parent,int mode, const string& title);
  InternalDisplay* createFarmStatsDisplay(InternalDisplay* parent, const string& title);

  ClusterDisplay*  createSubfarmDisplay(int width, int height, int posx, int posy, int argc, char** argv);
  ClusterDisplay*  createRecSubfarmDisplay(int width, int height, int posx, int posy, int argc, char** argv);
  ClusterDisplay*  createCtrlSubfarmDisplay(int width, int height, int posx, int posy, int argc, char** argv);
  ClusterDisplay*  createStorageDisplay(int width, int height, int posx, int posy, int argc, char** argv);
  ClusterDisplay*  createMonitoringDisplay(int width, int height, int posx, int posy, int argc, char** argv);
}


namespace {
  ScrDisplay* swapMouseSelector(Interactor* ia, ClusterDisplay* from, ClusterDisplay* to) {
    if ( from ) {
      MouseSensor::instance().remove(from->display());
      ScrDisplay* d = dynamic_cast<ScrDisplay*>(from->nodeDisplay());
      if ( d ) MouseSensor::instance().remove(d->display());
    }
    if ( to ) {
      ScrDisplay* d = dynamic_cast<ScrDisplay*>(to->nodeDisplay());
      MouseSensor::instance().add(ia,to->display());
      if ( d )  MouseSensor::instance().add(ia,d->display());
      return d;
    }
    return 0;
  }

  struct DisplayUpdate {
    Pasteboard* m_pb;
    bool m_flush;
    DisplayUpdate(InternalDisplay* d, bool flush=false) : m_pb(d->pasteboard()), m_flush(flush) {
      ::scrc_begin_pasteboard_update(m_pb);
    }
    ~DisplayUpdate() {
      ::scrc_end_pasteboard_update (m_pb);
      if ( m_flush ) ::scrc_fflush(m_pb);
    }
  };
  /// Extract node/service name from DNS info
  void getServiceNode(char* s, string& svc, string& node) {
    char* at = strchr(s,'@');
    *at = 0;
    svc = s;
    node = at+1;
  }
}

/// Standard constructor
FarmDisplay::FarmDisplay(int argc, char** argv)
  : InternalDisplay(0,""), m_subfarmDisplay(0), m_nodeSelector(0), 
    m_posCursor(0), m_subPosCursor(0), m_anchorX(10), m_anchorY(5), m_mode(HLT_MODE)
{
  char txt[128];
  string anchor, prefix;
  RTL::CLI cli(argc,argv,help);
  bool all = 0 != cli.getopt("all",2);
  bool xml = 0 != cli.getopt("xml",2);
  cli.getopt("partition",   2, m_name = "ALL");
  cli.getopt("match",       2, m_match = "*");
  cli.getopt("prefix",      2, prefix);
  m_dense = 0 != cli.getopt("dense",2);
  m_mode  = cli.getopt("reconstruction",2) == 0 ? HLT_MODE : RECO_MODE;
  if ( cli.getopt("taskmonitor",2) != 0 ) m_mode = CTRL_MODE;
  if ( cli.getopt("anchor",2,anchor) != 0 ) {
    int x, y;
    if ( 2 == ::sscanf(anchor.c_str(),"+%d+%d",&x,&y) ) {
      m_anchorX = x;
      m_anchorY = y;
    }
    else if ( 2 == ::sscanf(anchor.c_str(),"%dx%d",&x,&y) ) {
      m_anchorX = x;
      m_anchorY = y;
    }
    else {
      ::printf("No valid anchor position given.\n");
    }
  }
  if ( !prefix.empty() ) InternalDisplay::setSvcPrefix(prefix);
  s_fd = this;
  if ( m_mode == RECO_MODE && all && m_match=="*" )
    ::sprintf(txt," Reconstruction farm display of all known subfarms ");
  else if ( m_mode == RECO_MODE && all )
    ::sprintf(txt," Reconstruction farm display of all known subfarms with name '%s'",m_match.c_str());
  else if ( m_mode == RECO_MODE )
    ::sprintf(txt," Reconstruction farm display of partition %s ",m_name.c_str());
  else if ( m_mode == CTRL_MODE && all && m_match=="*" )
    ::sprintf(txt," Task Control farm display of all known subfarms ");
  else if ( m_mode == CTRL_MODE && all )
    ::sprintf(txt," Task Control farm display of all known subfarms with name '%s'",m_match.c_str());
  else if ( m_mode == CTRL_MODE )
    ::sprintf(txt," Task Control farm display of partition %s ",m_name.c_str());
  else if ( m_match == "*" && all )
    ::sprintf(txt," HLT Farm display of all known subfarms ");
  else if ( all )
    ::sprintf(txt," HLT Farm display of all known subfarms with the name '%s'",m_match.c_str());
  else
    ::sprintf(txt," HLT Farm display of partition %s ",m_name.c_str());
  m_title = txt;
  ::scrc_create_pasteboard (&m_pasteboard, 0, &m_height, &m_width);
  ScrDisplay::setPasteboard(m_pasteboard);
  ScrDisplay::setBorder(BLUE|INVERSE);
  m_width -= 2;
  m_height -= 2;
  ::scrc_create_display (&m_display, m_height, m_width,NORMAL, ON, m_title.c_str());
  show(2,2);
  if ( m_mode == CTRL_MODE ) {
    ::scrc_put_chars(m_display,txt,NORMAL|BOLD,1,2,0);
    ::scrc_put_chars(m_display,"<CTRL-H for Help>, <CTRL-E to exit>",NORMAL|BOLD,1,40,0);
    ::scrc_put_chars(m_display,"nn",GREEN|INVERSE,1,80,0);
    ::scrc_put_chars(m_display,": OK",NORMAL,1,82,0);
    ::scrc_put_chars(m_display,"nn",RED|INVERSE,1,90,0);
    ::scrc_put_chars(m_display,": Not OK",NORMAL,1,92,0);
    ::scrc_put_chars(m_display,"nn",BLUE|INVERSE,1,110,0);
    ::scrc_put_chars(m_display,": OK/Excluded",NORMAL,1,112,0);
    ::scrc_put_chars(m_display,"nn",MAGENTA|INVERSE,1,130,0);
    ::scrc_put_chars(m_display,": Not OK/Excluded",NORMAL,1,132,1);
  }
  ::scrc_end_pasteboard_update (m_pasteboard);
  ::scrc_fflush(m_pasteboard);
  ::scrc_set_cursor(m_display, 2, 10);
  ::scrc_cursor_off(m_pasteboard);
  ::wtc_remove(WT_FACILITY_SCR);
  ::wtc_subscribe(WT_FACILITY_SCR, key_rearm, key_action, m_pasteboard);
  MouseSensor::instance().start(pasteboard());
  if ( xml ) {
    m_listener = auto_ptr<PartitionListener>(new PartitionListener(this,m_name,xml));
  }
  else if ( all ) {
    m_svc = ::dic_info_service((char*)"DIS_DNS/SERVER_LIST",MONITORED,0,0,0,dnsDataHandler,(long)this,0,0);
  }
  else {
    m_listener = auto_ptr<PartitionListener>(new PartitionListener(this,m_name));
  }
}

/// Standard destructor
FarmDisplay::~FarmDisplay()  {  
  MouseSensor::instance().stop();
  ::wtc_remove(WT_FACILITY_SCR);
  disconnect();
  m_listener = auto_ptr<PartitionListener>(0);
  ::scrc_begin_pasteboard_update(m_pasteboard);
  m_ctrlDisplay = auto_ptr<CtrlNodeDisplay>(0);
  m_mbmDisplay = auto_ptr<BufferDisplay>(0);
  if ( m_nodeSelector ) {
    MouseSensor::instance().remove(m_nodeSelector->display());
    m_nodeSelector = 0;
  }
  if ( m_subfarmDisplay ) {
    MouseSensor::instance().remove(m_subfarmDisplay->display());
    m_subfarmDisplay->finalize();
    delete m_subfarmDisplay;
    m_subfarmDisplay = 0;
  }
  subDisplays().clear();
  close();
  ::scrc_end_pasteboard_update (m_pasteboard);
  ::scrc_delete_pasteboard(m_pasteboard);
  m_pasteboard = 0;
  ::scrc_resetANSI();
  ::printf("Farm display deleted and resources freed......\n");
}

/// DIM command service callback
void FarmDisplay::dnsDataHandler(void* tag, void* address, int* size) {
  if ( address && tag && *size > 0 ) {
    FarmDisplay* disp = *(FarmDisplay**)tag;
    disp->update(address);
  }
}

/// Get the name of the currently selected cluster
string FarmDisplay::selectedCluster() const {
  if ( m_sysDisplay.get() )
    return m_sysDisplay->clusterName();
  else if ( m_subfarmDisplay )
    return m_subfarmDisplay->clusterName();
  else if ( currentDisplay() )
    return currentDisplay()->name();
  return "";
}

/// Get the name of the currently selected cluster and node
pair<string,string> FarmDisplay::selectedNode() const {
  string node_name, cl = selectedCluster();
  if ( !cl.empty() ) {
    if ( m_sysDisplay.get() )
      node_name = m_sysDisplay->nodeName(m_subPosCursor-SUBFARM_NODE_OFFSET);
    else if ( m_subfarmDisplay )
      node_name = m_subfarmDisplay->nodeName(m_subPosCursor-SUBFARM_NODE_OFFSET);
  }
  return make_pair(cl,node_name);
}

/// Number of sub-nodes in a cluster
size_t FarmDisplay::selectedClusterSize() const {
  if ( m_sysDisplay.get() )
    return m_sysDisplay->numNodes();
  else if ( m_subfarmDisplay )
    return m_subfarmDisplay->numNodes();
  return 0;
}

/// Keyboard rearm action
int FarmDisplay::key_rearm (unsigned int /* fac */, void* param)  {
  Pasteboard* pb = (Pasteboard*)param;
  return ::scrc_fflush(pb);
}

/// Keyboard action
int FarmDisplay::key_action(unsigned int /* fac */, void* /* param */)  {
  int key = ::scrc_read_keyboard(0,0);
  if (!key) return WT_SUCCESS;
  return s_fd->handleKeyboard(key);
}

/// Set cursor to position
void FarmDisplay::set_cursor(InternalDisplay* /* updater */) {
  set_cursor();
}

/// Set cursor to position
void FarmDisplay::set_cursor() {
  if ( 0 != m_sysDisplay.get() ) {
    Display* d1 = m_sysDisplay->display();
    if ( d1 ) ::scrc_set_cursor(d1, m_subPosCursor, 2);
  }
  else if ( 0 != m_subfarmDisplay ) {
    Display* d1 = m_subfarmDisplay->display();
    if ( d1 ) ::scrc_set_cursor(d1, m_subPosCursor, 2);
  }
  else {
    InternalDisplay* d = currentDisplay();
    if ( d ) ::scrc_set_cursor(d->display(),2, 2);
  }
}

/// Get farm display from cursor position
InternalDisplay* FarmDisplay::currentDisplay()  const {
  size_t cnt;
  SubDisplays::const_iterator k;
  for(k=m_farmDisplays.begin(), cnt=0; k != m_farmDisplays.end(); ++k, ++cnt) {
    if ( cnt == m_posCursor ) {
      return (*k).second;
    }
  }
  return 0;
}

/// Show subfarm display
int FarmDisplay::showSubfarm()    {
  InternalDisplay* d = 0;
  RTL::Lock unlock(screenLock(),true);
  if ( m_subfarmDisplay ) {
    DisplayUpdate update(this,true);
    m_nodeSelector = swapMouseSelector(this,m_subfarmDisplay,m_sysDisplay.get());
    m_subfarmDisplay->finalize();
    delete m_subfarmDisplay;
    m_subfarmDisplay = 0;
    m_sysDisplay   = auto_ptr<ClusterDisplay>(0);
    m_cpuDisplay   = auto_ptr<CPUDisplay>(0);
    m_mbmDisplay   = auto_ptr<BufferDisplay>(0);
    m_procDisplay  = auto_ptr<ProcessDisplay>(0);
    m_bootDisplay  = auto_ptr<InternalDisplay>(0);
    m_statsDisplay = auto_ptr<InternalDisplay>(0);
    m_ctrlDisplay  = auto_ptr<CtrlNodeDisplay>(0);
    m_benchDisplay = auto_ptr<InternalDisplay>(0);
    m_subPosCursor = 8;
  }
  else if ( (d=currentDisplay()) != 0 ) {
    string dnam = d->name();
    string svc = "-servicename="+svcPrefix()+dnam+"/ROpublish";
    string part= "-partition="+m_name;
    if ( m_mode == CTRL_MODE ) {
      string node = "-node="+strupper(dnam);
      svc = "-servicename="+svcPrefix()+strupper(dnam)+"/TaskSupervisor/Status";
      const char* argv[] = {"",svc.c_str(), node.c_str(), "-delay=300"};
      m_subfarmDisplay = createCtrlSubfarmDisplay(SUBFARM_WIDTH,SUBFARM_HEIGHT,m_anchorX,m_anchorY,4,(char**)argv);
    }
    else if ( strncasecmp(dnam.c_str(),"storectl01",10)==0 && m_name != "ALL" ) {
      const char* argv[] = {"", svc.c_str(), part.c_str(), "-delay=300"};
      m_subfarmDisplay = createStorageDisplay(SUBFARM_WIDTH,SUBFARM_HEIGHT,m_anchorX,m_anchorY,4,(char**)argv);
    }
    else if ( strncasecmp(dnam.c_str(),"mona08",6)==0 && m_name != "ALL" ) {
      string relay = "-namerelay="+dnam+"01";
      const char* argv[] = {"", svc.c_str(), part.c_str(), "-delay=300", "-relayheight=12", "-nodeheight=12", relay.c_str()};
      m_subfarmDisplay = createMonitoringDisplay(SUBFARM_WIDTH,SUBFARM_HEIGHT,m_anchorX,m_anchorY,7,(char**)argv);
    }
    else if ( strncasecmp(dnam.c_str(),"mona09",6)==0 && m_name != "ALL" ) {
      string relay = "-namerelay="+dnam+"01";
      const char* argv[] = {"", svc.c_str(), part.c_str(), "-delay=300", "-relayheight=14", "-nodeheight=22", relay.c_str()};
      m_subfarmDisplay = createMonitoringDisplay(SUBFARM_WIDTH,SUBFARM_HEIGHT,m_anchorX,m_anchorY,7,(char**)argv);
    }
    else if ( m_mode == RECO_MODE ) {
      const char* argv[] = {"", svc.c_str(), "-delay=300"};
      m_subfarmDisplay = createRecSubfarmDisplay(SUBFARM_WIDTH,SUBFARM_HEIGHT,m_anchorX,m_anchorY,3,(char**)argv);
    }
    else if ( m_mode == HLT_MODE ) {
      const char* argv[] = {"", svc.c_str(), "-delay=300", "-mooresheight=-1", "-nodesheight=31"};
      m_subfarmDisplay = createSubfarmDisplay(SUBFARM_WIDTH,SUBFARM_HEIGHT,m_anchorX,m_anchorY,5,(char**)argv);
    }
    else {
      m_nodeSelector = 0;
      m_subfarmDisplay = 0;
      return WT_SUCCESS;
    }
    m_subfarmDisplay->initialize();
    ::lib_rtl_sleep(200);
    m_nodeSelector = swapMouseSelector(this,m_sysDisplay.get(),m_subfarmDisplay);
    IocSensor::instance().send(this,CMD_UPDATE,m_subfarmDisplay);
    m_subPosCursor = SUBFARM_NODE_OFFSET;
  }
  return WT_SUCCESS;
}

/// Show window to run Moore bench marks
int FarmDisplay::showBenchmarkWindow() {
  DisplayUpdate update(this,true);
  if ( m_benchDisplay.get() ) {
    MouseSensor::instance().remove(this,m_benchDisplay->display());
    m_benchDisplay = auto_ptr<InternalDisplay>(0);
  }
  else if ( m_sysDisplay.get() || m_mbmDisplay.get() || m_ctrlDisplay.get() || m_procDisplay.get() ) {
    pair<string,string> node = selectedNode();
    if ( !node.second.empty() ) {
      m_benchDisplay = auto_ptr<InternalDisplay>(createBenchmarkDisplay(this,1,node.second));
      m_benchDisplay->show(m_anchorY+5,m_anchorX+12);
      m_benchDisplay->connect();
      MouseSensor::instance().add(this,m_benchDisplay->display());
    }
  }
  else if ( m_subfarmDisplay ) {
    string cluster_name = selectedCluster();
    if ( !cluster_name.empty() ) {
      m_benchDisplay = auto_ptr<InternalDisplay>(createBenchmarkDisplay(this,2,cluster_name));
      m_benchDisplay->show(m_anchorY+5,m_anchorX+12);
      m_benchDisplay->connect();
      MouseSensor::instance().add(this,m_benchDisplay->display());
    }
  }
  else {
    m_benchDisplay = auto_ptr<InternalDisplay>(createBenchmarkDisplay(this,3,name()));
    m_benchDisplay->show(m_anchorY+5,m_anchorX+12);
    m_benchDisplay->connect();
    MouseSensor::instance().add(this,m_benchDisplay->display());
  }
  return WT_SUCCESS;
}

/// Show context dependent help window
int FarmDisplay::showHelpWindow() {
  DisplayUpdate update(this,true);
  if ( m_helpDisplay.get() ) {
    MouseSensor::instance().remove(this,m_helpDisplay->display());
    m_helpDisplay = auto_ptr<HelpDisplay>(0);
  }
  else if ( m_bootDisplay.get() ) {
    string input = ::getenv("ROMONROOT") != 0 ? ::getenv("ROMONROOT") : "..";
    string fin = input+"/doc/bootMon.hlp";
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","boot-subfarm",fin));
  }
  else if ( m_statsDisplay.get() ) {
    string input = ::getenv("ROMONROOT") != 0 ? ::getenv("ROMONROOT") : "..";
    string fin = input+"/doc/farmStats.hlp";
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","boot-subfarm",fin));
  }
  else if ( m_sysDisplay.get() ) 
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","subfarm_ctrl"));
  else if ( m_mbmDisplay.get() ) 
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","mbm"));
  else if ( m_ctrlDisplay.get() ) 
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","ctrl"));
  else if ( m_procDisplay.get() ) 
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","procs"));
  else if ( m_cpuDisplay.get() ) 
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","cpu"));
  else if ( m_subfarmDisplay && m_mode == CTRL_MODE )
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","subfarm_ctrl"));
  else if ( m_subfarmDisplay && m_mode == RECO_MODE )
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","subfarm_reco"));
  else if ( m_subfarmDisplay )
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","subfarm"));
  else
    m_helpDisplay = auto_ptr<HelpDisplay>(new HelpDisplay(this,"Help window","farm"));
  if ( m_helpDisplay.get() ) {
    m_helpDisplay->show(m_anchorY,m_anchorX);
    MouseSensor::instance().add(this,m_helpDisplay->display());
  }
  return WT_SUCCESS;
}

/// Show window with buffer information of a given node
int FarmDisplay::showMbmWindow() {
  DisplayUpdate update(this,true);
  if ( m_mbmDisplay.get() ) {
    if ( m_helpDisplay.get() ) showHelpWindow();
    MouseSensor::instance().remove(this,m_mbmDisplay->display());
    m_mbmDisplay = auto_ptr<BufferDisplay>(0);
  }
  else if ( m_subfarmDisplay ) {
    m_mbmDisplay = auto_ptr<BufferDisplay>(new BufferDisplay(this,"MBM Monitor display"));
    m_mbmDisplay->setNode(m_subPosCursor-SUBFARM_NODE_OFFSET);
    m_mbmDisplay->update(m_subfarmDisplay->data().pointer);
    m_mbmDisplay->show(m_anchorY+5,m_anchorX+12);
    MouseSensor::instance().add(this,m_mbmDisplay->display());
  }
  return WT_SUCCESS;
}

/// Show window with buffer information of a given node
int FarmDisplay::showCtrlWindow() {
  DisplayUpdate update(this,true);
  if ( m_ctrlDisplay.get() ) {
    if ( m_helpDisplay.get() ) showHelpWindow();
    MouseSensor::instance().remove(this,m_ctrlDisplay->display());
    m_ctrlDisplay = auto_ptr<CtrlNodeDisplay>(0);
  }
  else if ( m_subfarmDisplay ) {
    m_ctrlDisplay = auto_ptr<CtrlNodeDisplay>(new CtrlNodeDisplay(this,"CTRL Monitor display"));
    m_ctrlDisplay->setNode(m_subPosCursor-SUBFARM_NODE_OFFSET);
    if ( m_mode == CTRL_MODE ) {
      m_ctrlDisplay->update(m_subfarmDisplay->data().pointer);
    }
    else {
      string node_name = m_subfarmDisplay->nodeName(0);
      node_name = node_name.substr(0,node_name.length()-2);
      m_ctrlDisplay->connect(node_name);
    }
    m_ctrlDisplay->show(m_anchorY+5,m_anchorX+12);
    MouseSensor::instance().add(this,m_ctrlDisplay->display());
  }
  return WT_SUCCESS;
}

/// Show window with processes on a given node
int FarmDisplay::showProcessWindow(int flag) {
  if ( m_procDisplay.get() ) {
    if ( m_helpDisplay.get() ) showHelpWindow();
    MouseSensor::instance().remove(this,m_procDisplay->display());
    m_procDisplay = auto_ptr<ProcessDisplay>(0);
    return WT_SUCCESS;
  }
  pair<string,string> node = selectedNode();
  if ( !node.second.empty() ) {
    m_procDisplay = auto_ptr<ProcessDisplay>(new ProcessDisplay(this,node.second,node.first,flag));
    m_procDisplay->show(m_anchorY+5,m_anchorX+12);
    MouseSensor::instance().add(this,m_procDisplay->display());
  }
  return WT_SUCCESS;
}

/// Show window with CPU information of a given subfarm
int FarmDisplay::showCpuWindow() {
  DisplayUpdate update(this,true);
  if ( m_cpuDisplay.get() ) {
    if ( m_helpDisplay.get() ) showHelpWindow();
    MouseSensor::instance().remove(this,m_cpuDisplay->display());
    m_cpuDisplay = auto_ptr<CPUDisplay>(0);
    return WT_SUCCESS;
  }
  pair<string,string> node = selectedNode();
  if ( !node.second.empty() ) {
    m_cpuDisplay = auto_ptr<CPUDisplay>(new CPUDisplay(this,node.first,node.second));
    m_cpuDisplay->show(m_anchorY+5,m_anchorX+12);
    MouseSensor::instance().add(this,m_cpuDisplay->display());
  }
  return WT_SUCCESS;
}

/// Show window with CPU information of a given subfarm
int FarmDisplay::showBootWindow() {
  DisplayUpdate update(this,true);
  if ( m_bootDisplay.get() ) {
    if ( m_helpDisplay.get() ) showHelpWindow();
    MouseSensor::instance().remove(this,m_bootDisplay->display());
    m_bootDisplay = auto_ptr<InternalDisplay>(0);
    return WT_SUCCESS;
  }
  string cluster_name = selectedCluster();
  if ( !cluster_name.empty() ) {
    m_bootDisplay = auto_ptr<InternalDisplay>(createBootDisplay(this,cluster_name));
    m_bootDisplay->show(m_anchorY+5,m_anchorX+12);
    m_bootDisplay->connect();
    MouseSensor::instance().add(this,m_bootDisplay->display());
  }
  return WT_SUCCESS;
}

/// Show window with CPU information of a given subfarm
int FarmDisplay::showStatsWindow() {
  DisplayUpdate update(this,true);
  string cluster_name = selectedCluster();
  if ( m_statsDisplay.get() ) {
    if ( m_helpDisplay.get() ) showHelpWindow();
    MouseSensor::instance().remove(this,m_statsDisplay->display());
    m_statsDisplay = auto_ptr<InternalDisplay>(0);
    return WT_SUCCESS;
  }
  if ( !cluster_name.empty() ) {
    m_statsDisplay = auto_ptr<InternalDisplay>(createFarmStatsDisplay(this,cluster_name));
    m_statsDisplay->show(m_anchorY+5,m_anchorX+12);
    m_statsDisplay->connect();
    MouseSensor::instance().add(this,m_statsDisplay->display());
  }
  return WT_SUCCESS;
}

/// Show window with SYSTEM information of a given subfarm
int FarmDisplay::showSysWindow() {
  DisplayUpdate update(this,true);
  if ( m_sysDisplay.get() ) {
    if ( m_helpDisplay.get() ) showHelpWindow();
    MouseSensor::instance().remove(this,m_sysDisplay->display());
    m_sysDisplay->finalize();
    m_nodeSelector = swapMouseSelector(this,m_sysDisplay.get(),m_subfarmDisplay);
    m_sysDisplay = auto_ptr<ClusterDisplay>(0);
    return WT_SUCCESS;
  }
  string dnam = selectedCluster();
  if ( !dnam.empty() ) {
    string node = "-node="+strupper(dnam);
    string svc = "-servicename="+svcPrefix()+strupper(dnam)+"/TaskSupervisor/Status";
    const char* argv[] = {"", svc.c_str(), node.c_str(), "-delay=300"};
    ClusterDisplay* disp = createCtrlSubfarmDisplay(SUBFARM_WIDTH,SUBFARM_HEIGHT,m_anchorX+3,m_anchorY,3,(char**)argv);
    m_sysDisplay = auto_ptr<ClusterDisplay>(disp);
    m_sysDisplay->initialize();
    m_nodeSelector = swapMouseSelector(this,m_subfarmDisplay,m_sysDisplay.get());
  }
  return WT_SUCCESS;
}

/// DIM command service callback
void FarmDisplay::update(const void* address) {
  char *msg = (char*)address;
  string svc, node;
  size_t idx, idq;
  switch(msg[0]) {
  case '+':
    getServiceNode(++msg,svc,node);
    idx = svc.find("/ROpublish");
    idq = svc.find("/hlt");
    if ( idq == string::npos ) idq = svc.find("/mona");
    if ( idq == string::npos ) idq = svc.find("/store");
    if ( idx != string::npos && idq == 0 ) {
      string f = svc.substr(1,idx-1);
      if ( ::strcase_match_wild(f.c_str(),m_match.c_str()) ) {
        IocSensor::instance().send(this,CMD_ADD,new string(f));
      }
    }
    break;
  case '-':
    break;
  case '!':
    //getServiceNode(++msg,svc,node);
    //log() << "Service " << msg << " in ERROR." << endl;
    break;
  default:
    if ( *(int*)msg != *(int*)"DEAD" )  {
      char *at, *p = msg, *last = msg;
      auto_ptr<Farms> farms(new Farms);
      while ( last != 0 && (at=strchr(p,'@')) != 0 )  {
        last = strchr(at,'|');
        if ( last ) *last = 0;
        getServiceNode(p,svc,node);
        idx = svc.find("/ROpublish");
        idq = svc.find("/hlt");
        if ( idq == string::npos ) idq = svc.find("/mona");
        if ( idq == string::npos ) idq = svc.find("/store");
        if ( idx != string::npos && idq == 0 ) {
          string f = svc.substr(1,idx-1);
          if ( ::strcase_match_wild(f.c_str(),m_match.c_str()) ) {
            farms->push_back(f);
          }
        }
        p = last+1;
      }
      if ( !farms->empty() )
        IocSensor::instance().send(this,CMD_CONNECT,farms.release());
    }
    break;
  }
}

/// Handle keyboard interrupts
int FarmDisplay::handleKeyboard(int key)    {
  int cnt, col, row;
  InternalDisplay* d = 0;
  Display* d1;
  SubDisplays& sd = subDisplays();
  RTL::Lock lock(screenLock());
  try {
    switch (key)    {
    case CTRL_W:
      ::scrc_repaint_screen (m_pasteboard);
      break;
    case 'E':
    case CTRL_E:
      delete this;
      ::exit(0);
      break;
    case 'h':
    case 'H':
    case CTRL_H:
      return showHelpWindow();
    case 'b':
    case 'B':
      return showBootWindow();
    case 'c':
    case 'C':
      return showCpuWindow();
    case 'k':
    case 'K':
      IocSensor::instance().send(this,CMD_SHOWCTRL,this);
      break;
    case 'l':
    case 'L':
      IocSensor::instance().send(this,CMD_SHOWSYS,this);
      break;
    case 'm':
    case 'M':
    case '.':
    case KPD_PERIOD:
      IocSensor::instance().send(this,m_sysDisplay.get()?CMD_SHOWCTRL:CMD_SHOWMBM,this);
      break;
    case CTRL_L:
      IocSensor::instance().send(this,CMD_SHOWBENCHMARK,this);
      break;
    case 'p':
    case 'P':
      return showProcessWindow(0);
    case CTRL_B:
      return showProcessWindow(1);
    case CTRL_V:
      return showProcessWindow(2);
    case CTRL_P:
      return showProcessWindow(3);
    case CTRL_N:
      return showProcessWindow(4);
    case CTRL_M:
      break;      
    case 's':
    case 'S':
      return showStatsWindow();
    case RETURN_KEY:
    case ENTER:
      IocSensor::instance().send(this,CMD_SHOWSUBFARM,this);
      break;
    case MOVE_UP:
      if( 0 == m_nodeSelector && m_posCursor > 0 )
        --m_posCursor;
      else if( m_nodeSelector && m_subPosCursor > SUBFARM_NODE_OFFSET )
        --m_subPosCursor;
      break;
    case MOVE_DOWN:
      if( 0 == m_nodeSelector && m_posCursor < subDisplays().size()-1 )
        ++m_posCursor;
      else if( m_nodeSelector && selectedClusterSize() > (m_subPosCursor-SUBFARM_NODE_OFFSET) )
        ++m_subPosCursor;
      break;
    case MOVE_LEFT:
      if( 0 == m_subfarmDisplay && 0 == m_sysDisplay.get() && (d=currentDisplay()) ) {
        SubDisplays::reverse_iterator k;
        col = d ? d->display()->col : 2;
        row = d ? d->display()->row : 2;
        for(k=sd.rbegin(), cnt=sd.size(); k != sd.rend(); ++k, --cnt) {
          d1 = (*k).second->display();
          if ( d1->col < col ) {
            if ( d1->row == row ) {
              m_posCursor = --cnt;
              set_cursor();
              return WT_SUCCESS;
            }
          }
        }
      }
      break;
    case MOVE_RIGHT:
      if( 0 == m_subfarmDisplay && 0 == m_sysDisplay.get() && (d=currentDisplay()) ) {
        SubDisplays::iterator k;
        col = d ? d->display()->col : 2;
        row = d ? d->display()->row : 2;
        for(k=sd.begin(), cnt=0; k != sd.end(); ++k, ++cnt) {
          d1 = (*k).second->display();
          if ( d1->col > col ) {
            if ( d1->row == row ) {
              m_posCursor = cnt;
              set_cursor();
              return WT_SUCCESS;
            }
          }
        }
      }
      break;
    case PAGE_UP:
      if ( 0 == m_nodeSelector && (d=currentDisplay()) ) {
        SubDisplays::reverse_iterator k;
        col = d ? d->display()->col : 2;
        row = d ? d->display()->row : 2;
        for(k=sd.rbegin(), cnt=sd.size(); k != sd.rend(); ++k, --cnt) {
          d1 = (*k).second->display();
          if ( d1->col == col ) {
            if ( d1->row == min(d1->row,row) ) {
              row = d1->row;
              m_posCursor = cnt;
              return WT_SUCCESS;
            }
          }
        }
      }
      break;
    case PAGE_DOWN:
      if ( 0 == m_nodeSelector && (d=currentDisplay()) ) {
        SubDisplays::iterator k;
        col = d ? d->display()->col : 2;
        row = d ? d->display()->row : 0;
        for(k=sd.begin(), cnt=0; k != sd.end(); ++k, ++cnt) {
          d1 = (*k).second->display();
          if ( d1->col == col ) {
            if ( d1->row == max(d1->row,row) ) {
              row = d1->row;
              m_posCursor = cnt;
              return WT_SUCCESS;
            }
          }
        }
      }
      break;
    default:
      return WT_SUCCESS;
    }
  }
  catch(...) {
  }
  set_cursor();
  return WT_SUCCESS;
}

/// Interactor overload: Display callback handler
void FarmDisplay::handle(const Event& ev) {
  int cnt = 0;
  time_t now = time(0);
  InternalDisplay* d = 0;
  Farms::iterator i;
  SubDisplays::iterator k;
  const MouseEvent* m = 0;

  RTL::Lock lock(screenLock());
  switch(ev.eventtype) {
  case ScrMouseEvent:
    m = ev.get<MouseEvent>();
    if ( m->button == 2 ) {
      IocSensor::instance().send(this,CMD_SHOWHELP,this);
    }
    else if ( m->msec != (unsigned int)-1 ) {
      if ( m_helpDisplay.get() )
        IocSensor::instance().send(this,CMD_SHOWHELP,this);
      else if ( m_ctrlDisplay.get() )
        IocSensor::instance().send(this,CMD_SHOWCTRL,this);
      else if ( m_mbmDisplay.get() )
        IocSensor::instance().send(this,CMD_SHOWMBM,this);
      else if ( m_procDisplay.get() )
        IocSensor::instance().send(this,CMD_SHOWPROCS,this);
      else if ( m_cpuDisplay.get() )
        IocSensor::instance().send(this,CMD_SHOWCPU,this);
      else if ( m_bootDisplay.get() )
        IocSensor::instance().send(this,CMD_SHOWBOOT,this);
      else if ( m_statsDisplay.get() )
        IocSensor::instance().send(this,CMD_SHOWSTATS,this);
      else if ( m_sysDisplay.get() )
        IocSensor::instance().send(this,CMD_SHOWSYS,this);
      else
        IocSensor::instance().send(this,CMD_SHOWSUBFARM,this);
    }
    else if ( m_nodeSelector && m->msec == (unsigned int)-1 ) {
      Display* disp = m_nodeSelector->display();
      if ( m->display == disp ) {
        size_t pos = m->y - disp->row - 2;
        if ( selectedClusterSize()>pos ) {
          int cmd = m->button==0 ? m_sysDisplay.get() ? CMD_SHOWCTRL : CMD_SHOWMBM : CMD_SHOWPROCS;
          m_subPosCursor = pos + SUBFARM_NODE_OFFSET;
          IocSensor::instance().send(this,cmd,this);
        }
      }
    }
    return;
  case TimeEvent:
    if (ev.timer_data == m_subfarmDisplay ) {
      IocSensor::instance().send(this,CMD_UPDATE,this);
    }
    return;
  case IocEvent:
    switch(ev.type) {
    case CMD_SETCURSOR:
      set_cursor();
      break;
    case CMD_HANDLE_KEY:
      handleKeyboard(int((long)ev.data));
      break;
    case CMD_SHOWSUBFARM:
      showSubfarm();
      return;
    case CMD_SHOWBENCHMARK:
      showBenchmarkWindow();
      return;
    case CMD_SHOWBOOT:
      showBootWindow();
      return;
    case CMD_SHOWSTATS:
      showStatsWindow();
      return;
    case CMD_SHOWSYS:
      showSysWindow();
      return;
    case CMD_SHOWCPU:
      showCpuWindow();
      return;
    case CMD_SHOWCTRL:
      showCtrlWindow();
      return;
    case CMD_SHOWMBM:
      (m_mode == CTRL_MODE) ? showCtrlWindow() : showMbmWindow();
      return;
    case CMD_SHOWPROCS:
      showProcessWindow(0);
      return;
    case CMD_SHOWHELP:
      showHelpWindow();
      return;
    case CMD_SHOW:
      for(k=m_farmDisplays.begin(); k != m_farmDisplays.end(); ++k, ++cnt) {
        if ( (d=(*k).second) == ev.data )  {
          m_posCursor = cnt;
          IocSensor::instance().send(this,CMD_SHOWSUBFARM,this);
          return;
        }
      }
      break;
    case CMD_POSCURSOR:
      for(k=m_farmDisplays.begin(); k != m_farmDisplays.end(); ++k, ++cnt) {
        if ( (d=(*k).second) == ev.data )  {
          m_posCursor = cnt;
          set_cursor();
          return;
        }
      }
      break;
    case CMD_UPDATE:
      if ( m_subfarmDisplay )   {
        IocSensor::instance().send(m_subfarmDisplay,  ROMonDisplay::CMD_UPDATEDISPLAY,this);
      }
      if ( m_sysDisplay.get() )   {
        IocSensor::instance().send(m_sysDisplay.get(),ROMonDisplay::CMD_UPDATEDISPLAY,this);
      }
      if ( m_mbmDisplay.get() )  {
        const void* data = m_subfarmDisplay->data().pointer;
        m_mbmDisplay->setNode(m_subPosCursor-SUBFARM_NODE_OFFSET);
        m_mbmDisplay->update(data);
      }
      else if ( m_mode == CTRL_MODE && m_ctrlDisplay.get() ) {
        const void* data = m_subfarmDisplay->data().pointer;
        m_ctrlDisplay->setNode(m_subPosCursor-SUBFARM_NODE_OFFSET);
        m_ctrlDisplay->update(data);
      }
      TimeSensor::instance().add(this,1,m_subfarmDisplay);
      break;
    case CMD_ADD:
      if ( (i=find(m_farms.begin(),m_farms.end(),*ev.iocPtr<string>())) == m_farms.end() ) {
        m_farms.push_back(*ev.iocPtr<string>());
        connect(m_farms);
      }
      delete ev.iocPtr<string>();
      return;
    case CMD_CONNECT:
      m_farms = *ev.iocPtr<StringV>();
      connect(m_farms);
      delete ev.iocPtr<StringV>();
      return;
    case CMD_CHECK: {
      DisplayUpdate update(this,true);
      for(k=m_farmDisplays.begin(); k != m_farmDisplays.end(); ++k)
        if ( (d=(*k).second) != ev.data ) d->check(now);
      if ( m_sysDisplay.get() ) m_sysDisplay->update();
      break;
    }
    case CMD_NOTIFY: {
      unsigned char* ptr = (unsigned char*)ev.data;
      if ( ptr ) {
	if ( m_benchDisplay.get() ) m_benchDisplay->update(ptr + sizeof(int), *(int*)ptr);
	delete [] ptr;
      }
      return;
    }
    case CMD_DELETE:
      delete this;
      ::lib_rtl_sleep(200);
      ::exit(0);
      return;
    default:
      break;
    }
    break;
  default:
    break;
  }
}

void FarmDisplay::connect(const vector<string>& farms) {
  SubDisplays::iterator k;
  SubDisplays copy;
  char txt[128];
  DisplayUpdate update(this,false);

  ::sprintf(txt,"Total number of subfarms:%d    ",int(farms.size()));
  ::scrc_put_chars(m_display,txt,NORMAL|BOLD,1,2,0);

  for( k=m_farmDisplays.begin(); k != m_farmDisplays.end(); ++k)
    (*k).second->hide();
  for (Farms::const_iterator i=farms.begin(); i != farms.end(); ++i) {
    k = m_farmDisplays.find(*i);
    if ( k == m_farmDisplays.end() ) {
      if ( m_mode == RECO_MODE )
        copy.insert(make_pair(*i,createRecFarmSubDisplay(this,*i)));
      else if ( m_mode == CTRL_MODE )
        copy.insert(make_pair(*i,createCtrlFarmSubDisplay(this,*i)));
      else if ( ::strncasecmp((*i).c_str(),"mona0",5)==0 )
        copy.insert(make_pair(*i,createMonitoringSubDisplay(this,*i)));
      else if ( ::strncasecmp((*i).c_str(),"storectl",8)==0 )
        copy.insert(make_pair(*i,createStorageSubDisplay(this,*i)));
      else
        copy.insert(make_pair(*i,createFarmSubDisplay(this,*i)));
    }
    else {
      copy.insert(*k);
      m_farmDisplays.erase(k);
    }
  }
  for (k=m_farmDisplays.begin(); k != m_farmDisplays.end(); ++k)
    delete (*k).second;
  m_farmDisplays = copy;

  int subdisp_height = m_dense ? SUBFARM_DISP_HEIGHT_DENSE : SUBFARM_DISP_HEIGHT_NORMAL;
  int subdisp_width  = SUBFARM_DISP_WIDTH;
  int total_height   = SUBFARM_DISP_TOP;
  int total_width    = SUBFARM_DISP_LEFT;
  for (k=m_farmDisplays.begin(); k != m_farmDisplays.end(); ++k) {
    int h = total_height+subdisp_height;
    int w = total_width+subdisp_width+1;
    if ( h > m_height+2 ) {
      total_height = SUBFARM_DISP_TOP;
      total_width  = w+1;
      if ( w+subdisp_width > m_width+2 ) break;
    }
    (*k).second->show(total_height, total_width);
    total_height += subdisp_height+1;
  }
}

static size_t do_output(void*,int,const char* fmt, va_list args) {
  char buffer[1024];
  size_t len = ::vsnprintf(buffer, sizeof(buffer), fmt, args);
  //::upic_write_message(buffer,"");
  return len;
}

extern "C" int romon_farm(int argc,char** argv) {
  FarmDisplay* disp = new FarmDisplay(argc,argv);
  ::lib_rtl_install_printer(do_output,0);
  IocSensor::instance().run();
  delete disp;
  return 1;
}
