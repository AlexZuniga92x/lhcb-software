// $Id: FarmMonitor.cpp,v 1.7 2009-05-05 18:35:31 frankb Exp $
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
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/ROMon/src/FarmMonitor.cpp,v 1.7 2009-05-05 18:35:31 frankb Exp $

#define MBM_IMPLEMENTATION
#include "ROMon/ROMon.h"
#include "ROMon/FarmMonitor.h"
#include "CPP/IocSensor.h"
#include "CPP/Event.h"
#include "RTL/rtl.h"
#include "RTL/Lock.h"
#include "RTL/strdef.h"
#include "SCR/scr.h"
extern "C" {
#include "dic.h"
#include "dis.h"
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

// Max. 15 seconds without update allowed
static int UPDATE_TIME_MAX = 15;

using namespace ROMon;
using namespace SCR;
using namespace std;

typedef vector<string> StringV;

static lib_rtl_lock_t    s_lock;

namespace ROMon {
  class Alarm;

  struct NullAlarm  {
    bool operator()(const Alarm* a) const { return a == 0;                 }  
  };
  struct DeleteAlarm   {
    void operator()(pair<const int,vector<Alarm*> >& a)  const
    {  for_each(a.second.begin(),a.second.end(),DeleteAlarm());            }
    bool operator()(Alarm* a) const       { if ( a ) delete a; return true;} 
  };
  struct FindAlarm {
    const Alarm&  m_a;
    FindAlarm(const Alarm& a) : m_a(a) {}
    void operator()(const pair<const int,vector<Alarm*> >& a)  const
    {  for_each(a.second.begin(),a.second.end(),FindAlarm(m_a));    }
    bool operator()(const Alarm& a) const {  return this->operator()(&a);  }
    bool operator()(const Alarm* a) const  
    {  return 0 != a && a->code == m_a.code && a->node == m_a.node && a->description == m_a.description;        }
  };
  struct PrintAlarm {
    const string& tag;
    PrintAlarm(const string& t) : tag(t) {}
    void operator()(pair<const int,vector<Alarm*> >& a)  const
    {  for_each(a.second.begin(),a.second.end(),PrintAlarm(tag));          }
    bool operator()(const Alarm* a) const { cout << setw(8) << left << tag << " " << *a << endl; return true;       }
  };
  struct PublishAlarm {
    const string& tag;
    FarmMonitor* monitor;
    PublishAlarm(const string& t, FarmMonitor* m) : tag(t), monitor(m) {}
    void operator()(pair<const int,vector<Alarm*> >& a)  const
    {  for_each(a.second.begin(),a.second.end(),PublishAlarm(tag,monitor));          }
    bool operator()(const Alarm* a) const { monitor->publish(tag,*a); return true;       }
  };
  struct CheckMonitor {
    time_t now;
    CheckMonitor(time_t n) : now(n) {}
    void operator()(const pair<const string, InternalMonitor*>& mon) { mon.second->check(now); }
  };
  template <class R, class T>
  struct FarmCount {
    R& count;
    T  function;
    FarmCount(R& cnt, T pmf) : count(cnt), function(pmf) { count = 0;   }
    void operator()(const pair<string, InternalMonitor*>& mon) 
    { if ( mon.second->name()[0] == 'h' ) count += (mon.second->*function)();      }
  };
  template <class R, class T> FarmCount<R,T> _farmCount(R& cnt, T pmf) { return FarmCount<R,T>(cnt,pmf); }
  struct AddStateSummary  {
    StateSummary& sum;
    AddStateSummary(StateSummary& s) : sum(s){}
    void operator()(const pair<const char,int>& p) { sum[p.first] += p.second; }
    void operator()(const pair<const string, InternalMonitor*>& p) { p.second->addStateSummary(sum); }
  };
  struct PrintStateSummary  {
    void operator()(const pair<char,int>& p) const { cout << ' ' << p.first << ':' << p.second; }
  };

  InternalMonitor* createSubfarmMonitor(FarmMonitor* parent, const string& title);
  InternalMonitor* createRecSubfarmMonitor(FarmMonitor* parent, const string& title);
  InternalMonitor* createCtrlSubfarmMonitor(FarmMonitor* parent, const string& title);
  InternalMonitor* createMonitoringMonitor(FarmMonitor* parent, const string& title);
  InternalMonitor* createStorageMonitor(FarmMonitor* parent, const string& title);
}


static void help() {
  cout << "  romon_farm -option [-option]" << endl
       << "       -all                         Show all subfarms." << endl
       << "       -p[artition]=<name>          Partition name providing monitoring information." << endl
       << endl;
}

namespace ROMon {
  InternalMonitor* createRecSubfarmMonitor(FarmMonitor* m, const string& title) { return createSubfarmMonitor(m,title);}

#ifndef OutputLogger_H
#define OutputLogger_H

#include <string>
#include <iostream>

  /** class OutputLogger

  Small helper class to catch output of a task

  @author M.Frank
  @version 1.0

  */
  class OutputLogger : public std::streambuf     { 
  public: 
    /// Standard Constructor
    OutputLogger (); 
    /// Standard destructor
    ~OutputLogger (); 
    /// Callback to fill stream buffer
    int overflow (int c); 
    /// Callback indicating EOF
    int underflow (); 

  private:  
    /// Save old stream buffer
    std::streambuf* _old; 
    /// String currently beeing assembled
    std::string _buf; 
  }; 
#endif

}

namespace {
  /// Extract node/service name from DNS info
  void getServiceNode(char* s, string& svc, string& node) {
    char* at = strchr(s,'@');
    *at = 0;
    svc = s;
    node = at+1;
  }
}

/// Standard Constructor
OutputLogger::OutputLogger()   {
  _buf = ""; 
  ::printf("Redirecting cout to OutputLogger....\n");
  _old = cout.rdbuf(this);
} 
 
/// Standard Destructor
OutputLogger::~OutputLogger ()    { 
  cout.rdbuf(_old); 
} 

/// Callback when filling the stream buffer 
int OutputLogger::overflow (int c)    {
  if (c == '\n')     {
    _buf += c;
    switch(::toupper(_buf[0])) {
    case 'D':
    case 'V':
    case 'I':
    case 'N':
    case 'W':
    case 'E':
    case 'F':
    default:
      ::printf(_buf.c_str());
    break;
    }
    _buf="";
  } 
  else     { 
    _buf += c; 
  } 
  return (c); 
} 

/// Callback on EOF
int OutputLogger::underflow ()  { 
  return (EOF); 
} 


InternalMonitor::InternalMonitor(FarmMonitor* parent, const string& title) 
: m_parent(parent), m_name(title)
{
  m_svc = 0;
  m_steer = ~0x0;
  m_numTask = 0;
  m_numEvent = 0;
  m_numUpdate = 0;
  m_lastUpdate = time(0);
  m_hasProblems = false;
  m_snapshotDiff = parent ? parent->updateTimeMax() : UPDATE_TIME_MAX;
}

InternalMonitor::~InternalMonitor() {
  disconnect();
}

/// Log message with tag
ostream& InternalMonitor::log(const string& tag,const string& node) {
  char txt[32];
  time_t when = time(0);
  ::strftime(txt,sizeof(txt),"%H:%M:%S",::localtime(&when));
  if ( node.empty() ) {
    cout << left << setw(9) << tag << setw(13) << left << m_name << left << setw(12) << txt << " ";
  }
  else {
    cout << left << setw(9) << tag << setw(13) << left << node << left << setw(12) << txt << " ";
  }
  return cout;
}

void InternalMonitor::disconnect() {
  if ( m_svc != 0 ) {
    ::dic_release_service(m_svc);
    m_svc = 0;
  }
}

/// Update display content
void InternalMonitor::update(const void* data, size_t /* len */) {
  return update(data);
}

/// Add state summary
void InternalMonitor::addStateSummary(StateSummary& sum) {
  for_each(m_stateSummary.begin(),m_stateSummary.end(),AddStateSummary(sum));
}

bool InternalMonitor::useSnapshot() const {
  time_t now = time(0);
  return (now - m_lastUpdate) >= m_snapshotDiff;
}

/// Check monitor for errors
void InternalMonitor::check(time_t now) {
  if ( now - lastUpdate() > 5*m_snapshotDiff ) {
    setTimeoutError();
  }
  else if ( hasProblems() ) {
  }
}

/// Count the number of tasks
void InternalMonitor::countTasks(const Nodeset& ns)   {
  m_numTask = 0;
  for (Nodeset::Nodes::const_iterator n=ns.nodes.begin(); n!=ns.nodes.end(); n=ns.nodes.next(n))
    m_numTask += (*n).tasks()->size();
}

/// Count the number of tasks
void InternalMonitor::countEvents(const Nodeset& ns)   {
  m_numEvent = 0;
  for (Nodeset::Nodes::const_iterator n=ns.nodes.begin(); n!=ns.nodes.end(); n=ns.nodes.next(n)) {
    const Node::Buffers& buffs = *(*n).buffers();
    for(Node::Buffers::const_iterator ib=buffs.begin(); ib!=buffs.end(); ib=buffs.next(ib))  {
      const MBMBuffer::Control& ctrl = (*ib).ctrl;
      m_numEvent += ctrl.tot_produced;
    }
  }
}

/// Update monitoring content
void InternalMonitor::updateContent(const Nodeset& ns) {
  if ( useSnapshot() ) {
    if ( (m_steer&COUNT_TASKS)  == COUNT_TASKS  ) countTasks(ns);
    if ( (m_steer&COUNT_EVENTS) == COUNT_EVENTS ) countEvents(ns);
    extractData(ns);
    analyzeData();
  }
}

/// Set timeout error
void InternalMonitor::setTimeoutError() {
  auto_ptr<AlarmInfo> alarms(new AlarmInfo(m_name,Alarms()));
  setAlarm(alarms->second,m_name,ERR_NO_UPDATES,time(0));
  log("ERROR") << "Update info is " << int(time(0)-m_lastUpdate) << " seconds old." 
	       << " Maximum is " << int(3*m_snapshotDiff) << " seconds " << endl;
  m_lastUpdate = time(0);
  m_parent->updateAlarms(alarms->first, alarms->second);
}

/// Update monitor content
void InternalMonitor::update(const void* address) {
  if ( address ) {
    const Nodeset* ns = (const Nodeset*)address;
    if ( ns->type == Nodeset::TYPE ) {
      updateContent(*ns);
    }
  }
}

/// Set a new alarm
void InternalMonitor::setAlarm(Alarms& alms, const string& node, int typ, time_t when, const string& dsc, const string& opt) {
  string subfarm = ::isdigit(node[node.length()-3]) ? node.substr(0,node.length()-2) : node;
  alms.push_back(Alarm(typ,when,subfarm,node,dsc,opt));
  m_hasProblems = true;
}

/// DIM command service callback
void InternalMonitor::dataHandler(void* tag, void* address, int* size) {
  if ( address && tag && *size > 0 ) {
    InternalMonitor* disp = *(InternalMonitor**)tag;
    unsigned char* ptr = new unsigned char[*size+sizeof(int)];
    *(int*)ptr = *size;
    ::memcpy(ptr+sizeof(int),address,*size);
    disp->m_lastUpdate = time(0);
    IocSensor::instance().send(disp,CMD_UPDATE,ptr);
  }
}

/// Interactor overload: Monitor callback handler
void InternalMonitor::handle(const Event& ev)    {
  switch(ev.eventtype) {
  case IocEvent: {
    switch(ev.type) {
    case CMD_UPDATE: {
      RTL::Lock lock(s_lock);
      unsigned char* ptr = (unsigned char*)ev.data;
      update(ptr + sizeof(int), *(int*)ptr);
      delete [] ptr;
      break;
    }
    default:
      break;
    }
    break;
  }
  default:
    break;
  }
}

/// Standard constructor
FarmMonitor::FarmMonitor(int argc, char** argv) 
  : InternalMonitor(0,""), m_current(0), m_mode(HLT_MODE), m_update_time_max(UPDATE_TIME_MAX)
{
  bool all = false;
  bool xml = false;
  char txt[128];
  string service;
  RTL::CLI cli(argc,argv,help);
  cli.getopt("partition", 2, m_name = "ALL");
  cli.getopt("match",     2, m_match = "*");
  cli.getopt("update",    2, m_update_time_max);
  cli.getopt("service",   2, service);

  all = 0 != cli.getopt("all",2);
  xml = 0 != cli.getopt("xml",2);
  m_mode = cli.getopt("reconstruction",2) == 0 ? HLT_MODE : RECO_MODE;
  if ( cli.getopt("taskmonitor",2) != 0 ) m_mode = CTRL_MODE;

  ::lib_rtl_create_lock(0,&s_lock);
  if ( m_mode == RECO_MODE && all && m_match=="*" )
    ::sprintf(txt,"Reconstruction farm display of all known subfarms ");
  else if ( m_mode == RECO_MODE && all )
    ::sprintf(txt,"Reconstruction farm display of all known subfarms with name '%s'",m_match.c_str());
  else if ( m_mode == RECO_MODE )
    ::sprintf(txt,"Reconstruction farm display of partition %s ",m_name.c_str());
  else if ( m_mode == CTRL_MODE && all && m_match=="*" )
    ::sprintf(txt,"Task Control farm display of all known subfarms ");
  else if ( m_mode == CTRL_MODE && all )
    ::sprintf(txt,"Task Control farm display of all known subfarms with name '%s'",m_match.c_str());
  else if ( m_mode == CTRL_MODE )
    ::sprintf(txt,"Task Control farm display of partition %s ",m_name.c_str());
  else if ( m_match == "*" && all )
    ::sprintf(txt,"HLT Farm display of all known subfarms ");
  else if ( all )
    ::sprintf(txt,"HLT Farm display of all known subfarms with the name '%s'",m_match.c_str());
  else
    ::sprintf(txt,"HLT Farm display of partition %s ",m_name.c_str());
  m_time = time(0);
  m_title = txt;
  log("INFO") << txt << endl;
  if ( xml )
    m_listener = auto_ptr<PartitionListener>(new PartitionListener(this,m_name,xml));
  else if ( !all )
    m_listener = auto_ptr<PartitionListener>(new PartitionListener(this,m_name));
  else
    m_svc = ::dic_info_service((char*)"DIS_DNS/SERVER_LIST",MONITORED,0,0,0,dnsDataHandler,(long)this,0,0);

  if ( service.empty() ) service = "/"+partition()+"/FarmMonitor";
  string svc = service+"/Alarms";
  //::dic_set_dns_node((char*)name().c_str());
  m_serviceID = ::dis_add_service((char*)svc.c_str(),(char*)"C",0,0,feedData,(long)this);
  string summary = service+"/AlarmSummary";
  m_summaryID    = ::dis_add_service((char*)summary.c_str(),(char*)"C",0,0,feedSummary,(long)this);
  string beat    = service+"/HartBeat";
  m_hartBeatID   = ::dis_add_service((char*)beat.c_str(),(char*)"I",&m_time,sizeof(m_time),0,(long)this);
  string cmd_svc = service+"/Command";
  m_commandID    = ::dis_add_cmnd((char*)cmd_svc.c_str(),(char*)"I",commandHandler,(long)this);
  m_runState  = 0;
  m_numEvent = 0;
  ::dis_start_serving((char*)service.c_str());
}

/// Standard destructor
FarmMonitor::~FarmMonitor()  {  
  disconnect();
  if ( m_hartBeatID ) ::dis_remove_service(m_hartBeatID);
  m_hartBeatID = 0;
  if ( m_summaryID ) ::dis_remove_service(m_summaryID);
  m_summaryID = 0;
  if ( m_serviceID ) ::dis_remove_service(m_serviceID);
  m_serviceID = 0;
  m_listener = auto_ptr<PartitionListener>(0);
  subMonitors().clear();
  ::printf("Farm display deleted and resources freed......\n");
}

/// DIM command service callback
void FarmMonitor::commandHandler(void* tag, void* address, int* size) {
  printf("Got command request\n");
  if ( address && tag && *size > 0 ) {
    FarmMonitor* disp = *(FarmMonitor**)tag;
    IocSensor::instance().send(disp,CMD_CLEAR,(void*)0);
  }
}

#if 0
/// DIM callback on dis_update_service
void FarmMonitor::feedData(void* tag, void** buf, int* size, int* first) {
  FarmMonitor* m = *(FarmMonitor**)tag;
  string data = "";
  string* s = m->m_current;
  if ( s ) {
    data = "FarmMonitor!#!"+(*s);
  }
  *size = *first || 0==s ? 0 : data.length()+1;
  *buf = (void*)(data.c_str());
}
#endif

/// DIM callback on dis_update_service
void FarmMonitor::feedData(void* tag, void** buf, int* size, int* first) {
  FarmMonitor* m = *(FarmMonitor**)tag;
  static string data;
  Alarm* a = m->m_currAlarm;
  if ( a ) {
    data = "";
    data = "FarmMonitor#";
    data += a->message();
    data += "#"+a->toString(a->tag);
    data.c_str();
  }
  *size = *first || 0==a ? 0 : data.length()+1;
  *buf = (void*)(data.c_str());
}

/// DIM callback on dis_update_service
void FarmMonitor::feedSummary(void* tag, void** buf, int* size, int* first) {
  FarmMonitor* m = *(FarmMonitor**)tag;
  string* s = m->m_current;
  *size = *first || 0==s ? 0 : s->length()+1;
  *buf = (void*)(0==s ? "" : s->c_str());
}

/// DIM command service callback
void FarmMonitor::dnsDataHandler(void* tag, void* address, int* size) {
  if ( address && tag && *size > 0 ) {
    FarmMonitor* disp = *(FarmMonitor**)tag;
    disp->update(address);
  }
}

/// Publish alarm summary
void FarmMonitor::publishSummary(const AlarmSummary& summary) {
  char text[1024];
  for(AlarmSummary::const_iterator i=summary.begin();i != summary.end(); ++i) {
    if ( (*i).first == ERR_REMOVEDALL ) {
      printf(" --------------------------- Summary: No alarms pending! \n");
      return;
    }
  }
  printf(" --------------------------- Summary \n");
  for(AlarmSummary::const_iterator j=summary.begin();j != summary.end(); ++j) {
    const AlarmSum& s = (*j).second;
    if ( s.nodes.size() == 1 ) {
      for(size_t k=0; k<s.nodes.size(); ++k) {
	const Alarm* a = s.nodes[k];
	::sprintf(text,"%-12s %-12s %s%s",a->node.c_str(),a->time().c_str(),a->message(),a->description.c_str());
	::printf("SUMMARY  %s\n",text);
      }
    }
    else if ( s.nodes.size() < 10 ) {
      map<string,vector<Alarm*> > alms;
      for(size_t k=0; k<s.nodes.size(); ++k)
	alms[s.nodes[k]->description].push_back(s.nodes[k]);
      for(map<string,vector<Alarm*> >::const_iterator l=alms.begin(); l!=alms.end();++l) {
	set<string> opts;
	const Alarm* a = ((*l).second)[0];
	::sprintf(text,"%-4d %-7s %-12s %s%s ",int(s.nodes.size()),
		  "alarms",a->time().c_str(),a->message(),(*l).first.c_str());
	for(size_t k=0; k<(*l).second.size(); ++k) {
	  const string& item = (*l).second[k]->node;
	  if ( opts.find(item) == opts.end() ) {
	    opts.insert(item);
	    ::strcat(text,item.c_str());
	    ::strcat(text," ");
	    if ( k>5 ) {
	      ::strcat(text," ..... ");
	      break;
	    }
	  }
	}
      }
      ::printf("SUMMARY  %s\n",text);
    }
    else  {
      const Alarm* a = s.nodes[0];
      ::sprintf(text,"%-4d %-8s %-12s %s",int(s.nodes.size()),"alarms",a->time().c_str(),a->message());
      set<string> opts;
      for(size_t k=0; k<s.nodes.size(); ++k) {
	const string& item = s.nodes[k]->node;
	if ( opts.find(item) == opts.end() ) {
	  opts.insert(item);
	  ::strcat(text,item.c_str());
	  ::strcat(text," ");
	  if ( k>5 ) {
	    ::strcat(text," ..... ");
	    break;
	  }
	}
      }
      ::printf("SUMMARY:%s\n",text);
    }
  }
}

/// Publish alarm
void FarmMonitor::publish(const string& tag, const Alarm& alm) {
  PrintAlarm p(tag);
  p(&alm);
  IocSensor::instance().send(this,CMD_SHOW_ALARM,new Alarm(alm,tag));
}

/// Update alarms from a subfarm
void FarmMonitor::updateAlarms(const string& subfarm, Alarms& alarms) {
  int cnt_new = 0, cnt_clr = 0;
  bool canHaveAlarms = true;//m_numEvent>0;  //m_runState < 0 && runState >= 2;
  if ( canHaveAlarms ) {
    for(Alarms::iterator i=alarms.begin(); i!=alarms.end(); ++i)   {
      TypeAlarms& a = m_alarms[(*i).code];
      if ( find_if(a.begin(),a.end(),FindAlarm(*i)) == a.end() )  {
	a.push_back(new Alarm(*i));
	m_newAlarms[(*i).code].push_back(a.back());
	++cnt_new;
      }
    }
  }

  // Have to do the reverse as well:
  // -- If the alarm is present with this code,node,subfarm, it has to be removed
  for(AlarmsByType::iterator p=m_alarms.begin(); p!=m_alarms.end(); ++p)  {
    TypeAlarms& a = (*p).second;
    for(size_t k=0; k<a.size(); ++k) {
      Alarm& alm = *a[k];
      // Only clear alarms which origine from the same subfarm....
      if ( canHaveAlarms ) {
	if ( alm.subfarm == subfarm ) {
	  if ( find_if(alarms.begin(),alarms.end(),FindAlarm(alm)) == alarms.end() ) {
	    m_clrAlarms[alm.code].push_back(&alm);
	    a.erase(a.begin()+k);
	    ++cnt_clr;
	    --k;
	  }
	}
      }
      else {
	m_clrAlarms[alm.code].push_back(&alm);
	a.erase(a.begin()+k);
	++cnt_clr;
	--k;
      }
    }
  }
  //cout << subfarm << " All:" << alarms.size() << " New:" << cnt_new << " Clear:" << cnt_clr << endl;
  //cout << "Statesummary:";
  //for_each(m_stateSummary.begin(),m_stateSummary.end(),PrintStateSummary());
  //cout << endl;
}

/// Handle new incoming alarm and update summary
void FarmMonitor::handleAlarmSummary(const string& alarm) {
  const char* text = alarm.c_str();
  unsigned int val;
  auto_ptr<AlarmSummary> summary(new AlarmSummary);
  AlarmSummary::iterator k;
  AlarmMap::iterator i;
  bool removeAll = false;
  if ( alarm.length()>5 && text[5] == 'A' ) {      // CLEARALL
    removeAll = true;
  }
  else if ( (i=m_sumAlarms.find(val=Alarm::hash32(text+17))) == m_sumAlarms.end() ) {
    if ( text[0] == 'D' ) {         // DECLARE
      Alarm* alm = new Alarm(text);
      m_sumAlarms[val] = alm;
    }
  }
  else  if ( text[0] == 'C' ) {     // CLEAR
    delete (*i).second;
    m_sumAlarms.erase(i);
  }
  if ( removeAll ) {
    (*summary)[ERR_REMOVEDALL].nodes.push_back(new Alarm(ERR_REMOVEDALL,time(0),"",""));
  }
  else {
    for(i=m_sumAlarms.begin(); i != m_sumAlarms.end(); ++i)
      (*summary)[(*i).second->code].nodes.push_back((*i).second);
  }
  publishSummary(*summary.get());
  //IocSensor::instance().send(this,CMD_SHOW_SUMMARY,summary.release());
}

/// DIM command service callback
void FarmMonitor::update(const void* address) {
  char c, *msg = (char*)address;
  string svc, node;
  size_t idx, idq;
  switch(c=msg[0]) {
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

/// Allow clients to check if the system is running
bool FarmMonitor::isRunning() const {
  return (m_lastNumEvt-m_numEvent) > 0;
}

/// Interactor overload: Monitor callback handler
void FarmMonitor::handle(const Event& ev) {
  RTL::Lock lock(s_lock);
  switch(ev.eventtype) {
  case TimeEvent:
    return;
  case IocEvent:
    switch(ev.type) {
      
    case CMD_SHOW_SUMMARY: {
      auto_ptr<AlarmSummary> val(ev.iocPtr<AlarmSummary>());
      publishSummary(*val.get());
      break;
    }
    case CMD_SHOW_ALARM: {
      auto_ptr<Alarm> val(ev.iocPtr<Alarm>());
      //handleAlarmSummary(*val);
      m_currAlarm = val.get();
      ::dis_update_service(m_serviceID);
      m_currAlarm = 0;
      break;
    }
    case CMD_RUNSTATE: {
      m_runState = ev.iocData<long>();
      /*
      if ( m_runState == 0 || m_runState == 1 ) {
	AlarmInfo alms;
	updateAlarms(alms.first, alms.second);
	IocSensor::instance().send(this,int(CMD_UPDATE),this);
	publish("CLEARALL",Alarm(ERR_NO_ERROR,time(0),"",""));
      }
      */
      log("INFO") << "Run State:" << m_runState << endl;
      break;
    }
    case CMD_PARTITIONID:
      log("INFO") << "Partition ID:" << ev.iocData<long>() << endl;
      break;
    case CMD_CLEAR:
      log("INFO") << "Received CLEARALL request....." << endl;
      m_time = ::time(0);
      m_alarms.clear();
      m_clrAlarms.clear();
      m_newAlarms.clear();
      publish("CLEARALL",Alarm(ERR_NO_ERROR,time(0),"",""));
      ::dis_update_service(m_hartBeatID);
      break;
    case CMD_SHOW:
      break;
    case CMD_UPDATE:
      /// Check for new alarms
      for_each(m_clrAlarms.begin(),m_clrAlarms.end(),PublishAlarm("CLEAR  ",this));
      for_each(m_newAlarms.begin(),m_newAlarms.end(),PublishAlarm("DECLARE",this));
      for_each(m_clrAlarms.begin(),m_clrAlarms.end(),DeleteAlarm());
      m_clrAlarms.clear();
      m_newAlarms.clear();
      //log("INFO") << "Farm events:" << m_numEvent << endl;
      m_time = ::time(0);
      ::dis_update_service(m_hartBeatID);
      break;
    case CMD_ADD: {
      auto_ptr<string> val(ev.iocPtr<string>());
      if ( find(m_farms.begin(),m_farms.end(),*val.get()) == m_farms.end() ) {
	m_farms.push_back(*ev.iocPtr<string>());
	connect(m_farms);
      }
      return;
    }
    case CMD_CONNECT: {
      m_farms = *auto_ptr<StringV>(ev.iocPtr<StringV>()).get();
      connect(m_farms);
      return;
    }
    case CMD_CHECK_CTRL: {
      time_t now = time(0);
      auto_ptr<AlarmInfo> alms(ev.iocPtr<AlarmInfo>());
      m_stateSummary.clear();
      for_each(m_farmMonitors.begin(),m_farmMonitors.end(),CheckMonitor(now));
      log("INFO") << "Checking sub monitors for errors....." << endl;
      updateAlarms(alms->first, alms->second);
      IocSensor::instance().send(this,int(CMD_UPDATE),this);
      break;
    }
    case CMD_CHECK: {
      time_t now = time(0);
      auto_ptr<AlarmInfo> alms(ev.iocPtr<AlarmInfo>());
      m_stateSummary.clear();
      for_each(m_farmMonitors.begin(),m_farmMonitors.end(),_farmCount(m_numTask,&InternalMonitor::taskCount));
      if ( now - m_lastUpdate > m_snapshotDiff ) {
	m_lastUpdate = now;
	m_lastNumEvt = m_numEvent;
	for_each(m_farmMonitors.begin(),m_farmMonitors.end(),_farmCount(m_numEvent,&InternalMonitor::evtCount));
      }
      if ( m_numTask > 0 ) {
	for_each(m_farmMonitors.begin(),m_farmMonitors.end(),CheckMonitor(now));
	for_each(m_farmMonitors.begin(),m_farmMonitors.end(),AddStateSummary(m_stateSummary));
	updateAlarms(alms->first, alms->second);
	IocSensor::instance().send(this,int(CMD_UPDATE),this);
      }
      break;
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

void FarmMonitor::connect(const vector<string>& farms) {
  SubMonitors::iterator k;
  SubMonitors copy;
  char txt[128];

  ::sprintf(txt,"Total number of subfarms:%d",int(farms.size()));
  log("INFO") << txt << endl;

  for (Farms::const_iterator i=farms.begin(); i != farms.end(); ++i) {
    k = m_farmMonitors.find(*i);
    if ( k == m_farmMonitors.end() ) {
      if ( m_mode == RECO_MODE )
	copy.insert(make_pair(*i,createRecSubfarmMonitor(this,*i)));
      else if ( m_mode == CTRL_MODE )
	copy.insert(make_pair(*i,createCtrlSubfarmMonitor(this,*i)));
      else if ( ::strncasecmp((*i).c_str(),"mona0",5)==0 )
	copy.insert(make_pair(*i,createMonitoringMonitor(this,*i)));
      else if ( ::strncasecmp((*i).c_str(),"storectl",8)==0 )
	copy.insert(make_pair(*i,createStorageMonitor(this,*i)));
      else
	copy.insert(make_pair(*i,createSubfarmMonitor(this,*i)));
    }
    else {
      copy.insert(*k);
      m_farmMonitors.erase(k);
    }
  }
  for (k=m_farmMonitors.begin(); k != m_farmMonitors.end(); ++k)
    delete (*k).second;
  m_farmMonitors = copy;
}

static size_t do_output(void*,int level,const char* fmt, va_list args) {
  char buffer[1024];
  size_t len = ::vsnprintf(buffer, sizeof(buffer), fmt, args);
  //::upic_write_message(buffer,"");
  switch(level) {
  case LIB_RTL_VERBOSE:
    cout << left << setw(8) << "VERBOSE" << setw(12) << left << "RTL" << ":" << buffer << endl;
    break;
  case LIB_RTL_DEBUG:
    cout << left << setw(8) << "DEBUG" << setw(12) << left << "RTL" << ":" << buffer << endl;
    break;
  case LIB_RTL_INFO:
    cout << left << setw(8) << "INFO" << setw(12) << left << "RTL" << ":" << buffer << endl;
    break;
  case LIB_RTL_WARNING:
    cout << left << setw(8) << "WARNING" << setw(12) << left << "RTL" << ":" << buffer << endl;
    break;
  case LIB_RTL_ERROR:
    cout << left << setw(8) << "ERROR" << setw(12) << left << "RTL" << ":" << buffer << endl;
    break;
  case LIB_RTL_FATAL:
    cout << left << setw(8) << "FATAL" << setw(12) << left << "RTL" << ":" << buffer << endl;
    break;
  case LIB_RTL_ALWAYS:
    cout << left << setw(8) << "ALWAYS" << setw(12) << left << "RTL" << ":" << buffer << endl;
    break;
  }
  return len;
}

extern "C" int romon_farmmon(int argc,char** argv) {
  OutputLogger logger;
  FarmMonitor* disp = new FarmMonitor(argc,argv);
  ::lib_rtl_install_printer(do_output,0);
  IocSensor::instance().run();
  delete disp;
  return 1;
}
