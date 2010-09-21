// $Id: BootAnalyser.cpp,v 1.3 2010-09-21 18:17:55 frankb Exp $
//====================================================================
//  ROMon
//--------------------------------------------------------------------
//
//  Package    : ROMon
//
//  Description: Structure of basic items used to monitor the boot status
//               of the CPU farm(s)
//
//  Author     : M.Frank
//  Created    : 20/09/2010
//
//====================================================================
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/ROMon/src/BootAnalyser.cpp,v 1.3 2010-09-21 18:17:55 frankb Exp $

#ifndef ONLINE_ROMON_BOOTANALYZER_H
#define ONLINE_ROMON_BOOTANALYZER_H

// Framework include files
#include "CPP/Interactor.h"
#include "ROMon/BootMon.h"
#include "ROMon/DataFile.h"

// C++ include files
#include <map>

/*
 *   ROMon namespace declaration
 */
namespace ROMon {

  /**@class SubfarmBootStatus BootMon.h ROMon/BootMon.h
   *
   * Class which represents the boot status of a subfarm
   *
   * @author M.Frank
   */
  class SubfarmBootStatus {
  public:
    typedef std::map<std::string,BootNodeStatus*> Nodes;
    typedef std::map<std::string,BootNodeStatus*> Hosts;

  protected:
    /// Buffer pointer
    char*              m_ptr;
    /// List of individual node monitors known to this sub-farm
    BootNodeStatusset* m_status;
    std::string        m_name;
    Nodes              m_nodes;
    Hosts              m_hosts;
    DataFile           m_file;
    /// DIM service identifier
    int                m_id;
    /// DIM client ID to task supervisor summary
    int                m_tsID;
  public:
    /// Initializing constructor
    SubfarmBootStatus(const std::string& n);
    /// Default destructor
    ~SubfarmBootStatus();
    /// Check status if initialization was successful....
    bool ok() const {  return m_id != 0; }
    /// Access to the subfarm name
    const std::string& name() const { return m_name; }
    /// Access to the nodes map (Object indexed by host name)
    Nodes& nodes() { return m_nodes; }
    /// Access to the hosts map (Object indexed by host address)
    Hosts& hosts() { return m_hosts; }
    // Start the sub-farm monitor
    int start();
    /// Start initial message file scan
    int scan();
    /// Start subfarm boot monitoring
    int check();
    /// Dump knowledge base
    void dump();
    /// DIM callback on dis_update_service
    static void feedData(void* tag, void** buf, int* size, int* first);
    /// DIM callback on dic_info_service
    static void summaryHandler(void* tag, void* buf, int* size);
  };

  /**@class BootMonitor BootMon.h ROMon/BootMon.h
   *
   * Class which represents the boot status of all subfarms
   *
   * @author M.Frank
   */
  class BootMonitor : public Interactor {
  public:
    typedef std::map<std::string,SubfarmBootStatus*> Clusters;
    
  protected:
    /// Buffer pointer
    char*                  m_ptr;
    /// Collection of contained cluster names
    BootClusterCollection* m_clusterdata;
    /// Boot cluster container
    Clusters               m_clusters;
    /// Monitor name
    std::string            m_name;
    /// DIM service identifier
    int                    m_id;

  public:
    /// Initializing constructor
    BootMonitor(const std::string& name);
    /// Default destructor
    virtual ~BootMonitor();
    /// Start the BOOT monitor service for all subfarms known
    virtual int start();
    /// Scan subfarms for new information
    virtual void scan();
    /// Handle interaction event
    virtual void handle(const Event& ev);
    /// DIM callback on dis_update_service
    static void feedData(void* tag, void** buf, int* size, int* first);
  };

}
#endif // ONLINE_ROMON_BOOTANALYZER_H

// Framework include files
#include "RTL/rtl.h"
#include "RTL/readdir.h"
#include "CPP/Event.h"
#include "CPP/TimeSensor.h"
#include "ROMon/CPUMon.h"
#include "ROMonDefs.h"

// C++ include files
#include <fcntl.h>
#include <cerrno>
#include <fstream>
#include <netinet/in.h>
#include <netdb.h>
#include <arpa/inet.h>
#include "TaskSupervisorParser.h"

extern "C" {
  #include "dis.h"
  #include "dic.h"
}

using namespace ROMon;
using namespace std;

#define LHCB_NODE_LIST "/group/online/hltip.txt"
#define CLUSTERLOGS_FARMS "/clusterlogs/farm"

#define MIN_LINE_LENGTH 15
#define CMD_DATA                   12345
#define SCAN_TIMEDIFF              15
#define PUBLISH_TIMEDIFF           15
#define DHCP_BOOT_DIFF            200

static bool s_use_ts = true;

namespace ROMon {
  class BootLogLine {
    const char* s;
    time_t      tm;
  public:
    BootLogLine(const char* p) : s(p), tm(0) {}
    time_t time()  {
      if ( 0 == tm ) {
	static unsigned int y = 0;
	struct tm t;
	if( 0 == y ) {
	  time_t now = ::time(0);
	  ::localtime_r(&now,&t);
	  y = t.tm_year;
	}
	if (::strptime(s,"%b %d %T",&t) == s+15) {
	  t.tm_year = y;
	  return tm=::mktime(&t);
	}
	return 0;
      }
      return tm;
    }
    const char* isDHCPRequest()  {
      static string tag = "dhcpd: DHCPREQUEST for ";
      char *pos = ::strstr(s+15,tag.c_str());
      if ( pos )  {
	if ( ::strstr(pos,"unknown lease") == 0 )  {
	  *::strchr(pos=pos+tag.length(),' ') = 0;
	  return pos;
	}
      }
      return 0;
    }

    const char* isMountRequest()   {
      static string tag = " mount request from ";
      char *pos = ::strstr(s+15,tag.c_str());
      if ( pos )  {
	*::strchr(pos=pos+tag.length(),':') = 0;
	return pos;
      }
      return 0;
    }

  };
}

struct BootDataProcessor : public DataFile::DataProcessor {
  SubfarmBootStatus& status;
  mutable time_t start;
  BootNodeStatus* nodeByName(const char* host) const  {
    char hname[64];
    const char* q = ::strchr(host,' ');
    if ( q && (q-host)<32 ) {
      ::memcpy(hname,host,q-host);
      hname[q-host] = 0;
      host = hname;
    }
    SubfarmBootStatus::Hosts::iterator i = status.hosts().find(host);
    if ( i == status.hosts().end() ) {
      i = status.nodes().find(host);
    }
    if ( i != status.nodes().end() ) {
      return (*i).second;
    }
    cout << "Unknown host:" << host << endl;
    return 0;
  }
  BootNodeStatus* updateStatus(BootNodeStatus* boot, int flag) const {
    if ( boot )   {
      if ( 0 == (boot->status&flag) ) {
	::lib_rtl_output(LIB_RTL_INFO,"%s> Set flag %s [%08X] -> %08X",
			 boot->name,BootNodeStatus::flagName(flag).c_str(),flag,boot->status);
	boot->status |= flag;
      }
    }
    return boot;
  }
  BootNodeStatus* updateStatus(const char* info, int flag) const {
    return updateStatus(nodeByName(info), flag);
  }

  BootDataProcessor(SubfarmBootStatus& s) : status(s), start(0) {}

  bool operator()(const char* ptr) const {
    const char* info = ptr+16;
    BootLogLine line(ptr);
    const char* host;
    BootNodeStatus* boot = 0;

    if ( !start ) start = line.time();
    if ( ::strstr(info,"kernel: Initializing CPU") )
      boot = updateStatus(info,BootNodeStatus::CPU_STARTED);
    else if ( ::strstr(info,"kernel: PCI: Setting") )
      boot = updateStatus(info,BootNodeStatus::PCI_STARTED);
    else if ( ::strstr(info,"Listening on interface eth0") )
      boot = updateStatus(info,BootNodeStatus::ETH0_STARTED);
    else if ( ::strstr(info,"Listening on interface eth1") )
      boot = updateStatus(info,BootNodeStatus::ETH1_STARTED);
    else if ( ::strstr(info,"kernel: TCP established") )
      boot = updateStatus(info,BootNodeStatus::TCP_STARTED);
    else if ( ::strstr(info,"FMC start-up succeeded") ) {
      boot = updateStatus(info,BootNodeStatus::FMC_STARTED);
      if ( boot ) boot->fmcStart = line.time();
    }
    else if ( (host=line.isMountRequest()) ) {
      boot = updateStatus(nodeByName(host), BootNodeStatus::MOUNT_REQUESTED);
      if ( boot ) boot->mountReq = line.time();
    }
    else if ( (host=line.isDHCPRequest()) ) {
      SubfarmBootStatus::Nodes::iterator i=status.nodes().find(host);
      if ( i != status.nodes().end() ) {
	(*i).second->status = BootNodeStatus::DHCP_REQUESTED;
	(*i).second->dhcpReq = line.time();
	::lib_rtl_output(LIB_RTL_INFO,"%s> %s Found DHCP request for node:%s",
			 status.name().c_str(),::lib_rtl_timestr(),host);
      }
      else {
	::lib_rtl_output(LIB_RTL_INFO,"%s> %s Unknown DHCP request for node:%s",
			 status.name().c_str(),::lib_rtl_timestr(),host);
      }
    }
    return true;
  }
};

/// Initializing constructor
SubfarmBootStatus::SubfarmBootStatus(const string& n) 
  : m_ptr(0), m_status(0), m_name(n), m_file("/clusterlogs/farm/"+n+"/messages"), m_id(0), m_tsID(0)
{}

/// Default destructor
SubfarmBootStatus::~SubfarmBootStatus() {
  if ( 0 != m_tsID ) {
    ::dic_release_service(m_tsID);
    m_tsID = 0;
  }
  if ( 0 != m_id ) {
    ::dis_remove_service(m_id);
    m_id = 0;
  }
  if ( m_ptr ) {
    delete [] m_ptr;
    m_ptr = 0;
  }
  m_file.close();
  m_status = 0;
}

static void error_user_routine (int severity, int error_code, char* message) {
  ::lib_rtl_output(LIB_RTL_ERROR,"Error handler called: %d %d %s.",severity,error_code,message);
}
// Start the sub-farm monitor
int SubfarmBootStatus::start() {
  ROMon::Inventory inv;
  string root = ::getenv("ROMONDATA") ? ::getenv("ROMONDATA") : "file://";
  string fname = "file://" + root + "/" + strupper(name()) + ".xml";
  string nam = strupper(name());
  XML::TaskSupervisorParser ts;
  //::lib_rtl_output(LIB_RTL_ALWAYS,"Parsing file %s with DOM.",fname.c_str());

  ts.parseFile(fname);
  ts.getInventory(inv);
  Inventory::NodeCollectionMap::const_iterator ni=inv.nodecollections.find(nam);
  if ( ni != inv.nodecollections.end() ) {
    const Inventory::NodeCollection::NodeList& nl = (*ni).second.nodes;
    Inventory::NodeCollection::NodeList::const_iterator niter = nl.begin();
    m_ptr = new char[sizeof(BootNodeStatus)*(nl.size()+1)+sizeof(BootNodeStatusset)];

    m_status = new(m_ptr) BootNodeStatusset(name());
    string svc_name = "/"+strupper(name())+"/TaskSupervisor/Summary";
    if ( s_use_ts ) {
      m_tsID = ::dic_info_service((char*)svc_name.c_str(),TIMED,15,0,0,summaryHandler,(long)this,0,0);
      dic_add_error_handler(error_user_routine);
    }

    BootNodeStatus* n = new(m_status->nodes.begin()) BootNodeStatus();
    for(; niter != nl.end(); ++niter)  {
      const string& node = (*niter).first;
      hostent* h = ::gethostbyname(node.c_str());
      if ( h ) {
	in_addr* ina = (in_addr*)h->h_addr;
	string addr = inet_ntoa(*ina);
	n->address = ina->s_addr;
	n->setName(node);
	nodes()[addr] = n;
	hosts()[node] = n;
	n = new(m_status->nodes.add(n)) BootNodeStatus();
	::lib_rtl_output(LIB_RTL_DEBUG,"...%s> Added node %-10s with address:%-12s [%s]",
			 name().c_str(),node.c_str(),addr.c_str(),::inet_ntoa(*ina));
      }
    }
    string svc = "/"+strlower(name())+"/BootMonitor";
    m_id = ::dis_add_service((char*)svc.c_str(),(char*)"C",0,0,feedData,(long)this);
    ::lib_rtl_output(LIB_RTL_INFO,"Boot Monitor Service %s for %s started.",svc.c_str(),name().c_str());
  }
  return 1;
}

/// DIM callback on dis_update_service
void SubfarmBootStatus::feedData(void* tag, void** buf, int* size, int* /* first */) {
  SubfarmBootStatus* h = *(SubfarmBootStatus**)tag;
  ro_gettime(&h->m_status->time,(unsigned int*)&h->m_status->millitm);
  *size = h->m_status->length();
  *buf = h->m_ptr;
}

/// DIM callback on dic_info_service
void SubfarmBootStatus::summaryHandler(void* tag, void* buf, int* size) {
  SubfarmBootStatus* h = *(SubfarmBootStatus**)tag;
  if ( *size > 0 ) {
    time_t now = ::time(0);
    Hosts::iterator ih;
    Hosts& hosts = h->hosts();
    const SubfarmSummary* s = (SubfarmSummary*)buf;
    for( ih=hosts.begin(); ih!=hosts.end(); ++ih ) {
      //(*ih).second->status = 0;
    }
    SubfarmSummary::Nodes::const_iterator ni = s->nodes.begin();
    for( ; ni != s->nodes.end(); ni = s->nodes.next(ni) ) {
      ih=hosts.find((*ni).name);
      if ( ih != hosts.end() ) {
	BootNodeStatus* n = (*ih).second;
	//cout << (*ni).name << " " << (*ni).state << " " << (*ni).status << endl;
	if ( (*ni).status == NodeSummary::OK && (*ni).state == NodeSummary::ALIVE ) {
	  n->status = (BootNodeStatus::FMC_STARTED+
		       BootNodeStatus::TCP_STARTED+
		       BootNodeStatus::ETH1_STARTED+
		       BootNodeStatus::ETH0_STARTED+
		       BootNodeStatus::PCI_STARTED+
		       BootNodeStatus::CPU_STARTED+
		       BootNodeStatus::MOUNT_REQUESTED+
		       BootNodeStatus::DHCP_REQUESTED);
	  if ( n->dhcpReq  <= 0 ) n->dhcpReq  = now;
	  if ( n->mountReq <= 0 ) n->mountReq = now;
	  if ( n->fmcStart <= 0 ) n->fmcStart = now;
	}
	else {
	  n->status &= ~BootNodeStatus::FMC_STARTED;
	  n->fmcStart = 0;
	}
      }
    }
  }
}

/// Start initial message file scan
int SubfarmBootStatus::scan() {
  BootDataProcessor proc(*this);
  int res = m_file.scan(proc);
  // fill defaults if there were no values in the data file
  for(Nodes::iterator i=m_nodes.begin(); i!=m_nodes.end();++i) {
    BootNodeStatus* n = (*i).second;
    if ( 0==n->mountReq && n->status&BootNodeStatus::MOUNT_REQUESTED ) n->mountReq = proc.start;
    if ( 0==n->dhcpReq  && n->status&BootNodeStatus::DHCP_REQUESTED  ) n->dhcpReq  = proc.start;
    if ( 0==n->fmcStart && n->status&BootNodeStatus::FMC_STARTED     ) n->fmcStart = proc.start;
  }
  return res;
}

/// Start subfarm boot monitoring
int SubfarmBootStatus::check() {
  BootDataProcessor proc(*this);
  int res = m_file.scan(proc);
  if ( res ) {
    if ( m_file.migrated() ) {
      m_file.close();
      res = m_file.scan(proc);
    }
    // Those who clain all is fine, but FMC does not start, may have crashed and wait for boot
    // If these nodes cannot be ping'ed they are bad!
    for(Nodes::iterator i=m_nodes.begin(); i!=m_nodes.end();++i) {
      BootNodeStatus* n = (*i).second;
      if ( n->status&BootNodeStatus::MOUNT_REQUESTED ) {
	if ( n->status&BootNodeStatus::DHCP_REQUESTED ) {
	  if ( n->status&BootNodeStatus::TCP_STARTED ) {
	    if ( 0 == (n->status&BootNodeStatus::FMC_STARTED) ) {
	      //if ( abs(time(0)-n->dhcpReq) > DHCP_BOOT_DIFF ) {
		string cmd = "/bin/ping -c 1 -w 1 -s 2 ";
		cmd += n->name;
		::lib_rtl_output(LIB_RTL_ERROR,"Executing command:%s",cmd.c_str());
		FILE* f = ::lib_rtl_pipe_open(cmd.c_str(),"r");
		if ( f ) {
		  char text[2048];
		  //memset(text,0,sizeof(text));
		  int nb = ::fread(text,1,sizeof(text)-1,f);
		  //::lib_rtl_output(LIB_RTL_ERROR,text);
		  if ( nb > 0 ) {
		    text[nb] = 0;
		    if ( ::strstr(text," 0% packet loss") == 0 ) {
		      n->status = 0;
		      ::lib_rtl_output(LIB_RTL_ERROR,"Reset node status of %s to %08X",n->name,n->status);
		    }
		  }
		  else {
		    ::lib_rtl_output(LIB_RTL_ERROR,"Read-error: %d bytes read:%s.",nb,::lib_rtl_error_message(::lib_rtl_get_error()));
		  }
		  ::lib_rtl_pipe_close(f);
		}
		else {
		  ::lib_rtl_output(LIB_RTL_ERROR,"Error:%s.",::lib_rtl_error_message(::lib_rtl_get_error()));
		}
	      }
	    //}
	  }
	}
      }
    }
  }
  ::dis_update_service(m_id);
  return res;
}

/// Dump knowledge base
void SubfarmBootStatus::dump() {
  struct in_addr ina;
  char t1[64], t2[64];
  for(Nodes::const_iterator i=m_nodes.begin(); i!=m_nodes.end();++i) {
    BootNodeStatus* n = (*i).second;
    ina.s_addr = n->address;
    ::strftime(t1,sizeof(t1),"%F %H:%M:%S",::localtime(&n->dhcpReq));
    ::strftime(t2,sizeof(t2),"%F %H:%M:%S",::localtime(&n->mountReq));
    ::lib_rtl_output(LIB_RTL_INFO,"%s> Node %-10s with address:%-15s DHCP:%s Mount:%s",
		     name().c_str(),n->name,::inet_ntoa(ina), t2,t1);
  }
}

/// Initializing constructor
BootMonitor::BootMonitor(const std::string& n) : m_ptr(0), m_clusterdata(0), m_name(n), m_id(0) {
}

/// Default destructor
BootMonitor::~BootMonitor() {
  if ( 0 != m_id ) ::dis_remove_service(m_id);
  m_id = 0;
  if ( m_ptr ) delete [] m_ptr;
  m_ptr = 0;
  m_clusterdata = 0;
  for(Clusters::iterator i=m_clusters.begin(); i!=m_clusters.end(); ++i)
    delete (*i).second;
  m_clusters.clear();
}

/// Start the BOOT monitor service for all subfarms known
int BootMonitor::start() {
  string dnam;
  struct dirent* dp;
  struct stat statbuff;
  DIR* dir=::opendir(CLUSTERLOGS_FARMS);
  if ( dir ) {
    while( (dp=::readdir(dir)) != 0 ) {
      const char* entry = dp->d_name;
      if ( entry[0] == '.' ) continue;
      dnam = CLUSTERLOGS_FARMS;
      dnam += "/";
      dnam += entry;
      if ( 0 == ::stat(dnam.c_str(),&statbuff) ) {
	if ( S_ISDIR(statbuff.st_mode) ) {
	  //if ( dnam.find("hltd08") == string::npos ) continue;
	  ::lib_rtl_output(LIB_RTL_INFO,"Loading subfarm boot status of %s.",entry);
	  SubfarmBootStatus* sf = 0;
	  try {
	    sf = new SubfarmBootStatus(entry);
	    if ( sf->start() ) {
	      if ( sf->scan() ) {
		sf->dump();
		m_clusters[entry] = sf;
		continue;
	      }
	    }
	    delete sf;
	  }
	  catch(const exception& e) {
	    ::lib_rtl_output(LIB_RTL_ERROR,"Exception  while processing subfarm:%s.",dnam.c_str());
	    ::lib_rtl_output(LIB_RTL_ERROR,"--> We will ignore subfarm:%s -- %s",dnam.c_str(),e.what());
	    if ( sf ) delete sf;
	  }
	}
      }
      else {
	::lib_rtl_output(LIB_RTL_ERROR,"Failed to stat entry:%s",dnam.c_str());
      }
    }
    ::closedir(dir);
    m_ptr = new char[(m_clusters.size()+1)*sizeof(BootClusterItem)+sizeof(BootClusterCollection)];
    m_clusterdata = new(m_ptr) BootClusterCollection();
    BootClusterCollection::iterator ci=m_clusterdata->begin();
    for(Clusters::const_iterator i=m_clusters.begin(); i!=m_clusters.end();++i) {
      ::strncpy((*ci).name,(*i).second->name().c_str(),sizeof((*ci).name));
      (*ci).name[sizeof((*ci).name)-1]=0;
      ci = m_clusterdata->add(ci);
    }
    string svc = "/BootMonitor/Clusters";
    m_id = ::dis_add_service((char*)svc.c_str(),(char*)"C",0,0,feedData,(long)this);
    ::lib_rtl_output(LIB_RTL_INFO,"Boot Monitor Service %s started.",svc.c_str());
    return 1;
  }
  return 0;
}

/// DIM callback on dis_update_service
void BootMonitor::feedData(void* tag, void** buf, int* size, int* /* first */) {
  BootMonitor* h = *(BootMonitor**)tag;
  *size = h->m_clusterdata->length();
  *buf = h->m_ptr;
}

void BootMonitor::scan() {
  for(Clusters::iterator i=m_clusters.begin(); i!=m_clusters.end(); ++i) {
    ::lib_rtl_output(LIB_RTL_VERBOSE,"Checking subfarm boot status of %s.",(*i).first.c_str());
    (*i).second->check();
  }
}

/// Handle interaction event
void BootMonitor::handle(const Event& ev) {
  try {
    switch(ev.eventtype) {
    case TimeEvent:
      if (ev.timer_data == (void*)CMD_DATA ) {
	::lib_rtl_output(LIB_RTL_INFO,"BootMonitor> %s Checking %d clusters for boot information.",
			 ::lib_rtl_timestr(),int(m_clusters.size()));
	scan();
        TimeSensor::instance().add(this,SCAN_TIMEDIFF,(void*)CMD_DATA);
      }
      break;
    default:
      break;
    }
  }
  catch(const exception& e) {
    ::lib_rtl_output(LIB_RTL_ERROR,"Exception in callback processing:%s",e.what());
  }
  catch(...) {
    ::lib_rtl_output(LIB_RTL_ERROR,"UNKNOWN exception in callback processing.");
  }
}


static void help() {
  ::printf("You have to supply a file name\n");
  ::exit(0);
}

//typedef std::vector<std::string* > dyn_string;
extern "C" int run_boot_analyser(int argc, char *argv[])    {
  string node = RTL::nodeNameShort();
  string server = "/"+node+"/"+RTL::processName();
  RTL::CLI cli(argc,argv,help);
  s_use_ts = cli.getopt("nots",1)==0;
  ::lib_rtl_install_printer(ro_rtl_print,0);
  ro_trl_set_print_level(LIB_RTL_ERROR);
  if ( cli.getopt("VERBOSE",4)!=0 )
    ro_trl_set_print_level(LIB_RTL_VERBOSE);
  else if ( cli.getopt("DEBUG",4)!=0 )
    ro_trl_set_print_level(LIB_RTL_DEBUG);
  else if ( cli.getopt("INFO",4)!=0 )
    ro_trl_set_print_level(LIB_RTL_INFO);
  else if ( cli.getopt("WARNING",4)!=0 )
    ro_trl_set_print_level(LIB_RTL_WARNING);
  else if ( cli.getopt("ERROR",4)!=0 )
    ro_trl_set_print_level(LIB_RTL_ERROR);

  BootMonitor mon(server);
  mon.start();
  TimeSensor::instance().add(&mon,1,(void*)CMD_DATA);
  ::dis_start_serving((char*)server.c_str());
  TimeSensor::instance().run();
  return 1;
}
