// $Id: PartitionListener.cpp,v 1.15 2008-11-13 09:02:30 frankb Exp $
//====================================================================
//  ROLogger
//--------------------------------------------------------------------
//
//  Package    : ROLogger
//
//  Description: Readout monitoring in the LHCb experiment
//
//  Author     : M.Frank
//  Created    : 29/1/2008
//
//====================================================================
// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/ROLogger/src/PartitionListener.cpp,v 1.15 2008-11-13 09:02:30 frankb Exp $

// Framework include files
#include "ROLogger/PartitionListener.h"
#include "UPI/UpiSensor.h"
#include "CPP/IocSensor.h"
#include "UPI/upidef.h"
#include "ROLoggerDefs.h"

#include <vector>
#include <memory>
#include <cstring>

extern "C" {
#include "dic.h"
}
#ifdef _WIN32
#include <winsock2.h>
#else
#include <netdb.h>
#include <arpa/inet.h>
#endif

using namespace ROLogger;
using namespace std;
typedef vector<string> _SV;
static std::string facility = "gaudi";

/// Standard constructor with object setup through parameters
PartitionListener::PartitionListener(Interactor* parent,const string& nam) : m_parent(parent), m_name(nam)
{
  string name;
  name = "RunInfo/" + m_name + "/HLTsubFarms";
  m_subFarmDP = ::dic_info_service((char*)name.c_str(),MONITORED,0,0,0,subFarmHandler,(long)this,0,0);
  ::upic_write_message2("Subfarm content for %s_RunInfo from:%s",m_name.c_str(),name.c_str());
  name = "RunInfo/" + m_name + "/HLTnodeList";
  m_nodesDP   = ::dic_info_service((char*)name.c_str(),MONITORED,0,0,0,nodeHandler,(long)this,0,0);
  ::upic_write_message2("Nodelist content for %s_RunInfo from:%s",m_name.c_str(),name.c_str());
  name = "RunInfo/" + m_name + "/CALIBnodeList";
  m_calibNodesDP   = ::dic_info_service((char*)name.c_str(),MONITORED,0,0,0,calibNodeHandler,(long)this,0,0);
  ::upic_write_message2("Calibration nodelist content for %s_RunInfo from:%s",m_name.c_str(),name.c_str());
  name = "RunInfo/" + m_name + "/MonitoringSlice";
  m_monSliceDP     = ::dic_info_service((char*)name.c_str(),MONITORED,0,0,0,monSliceHandler,(long)this,0,0);
  ::upic_write_message2("Monitoring slice info for %s_RunInfo from:%s",m_name.c_str(),name.c_str());
  name = "RunInfo/" + m_name + "/StorageSlice";
  m_storSliceDP    = ::dic_info_service((char*)name.c_str(),MONITORED,0,0,0,storSliceHandler,(long)this,0,0);
  ::upic_write_message2("Storage slice info for %s_RunInfo from:%s",m_name.c_str(),name.c_str());
}

/// Standard destructor
PartitionListener::~PartitionListener() {
  ::dic_release_service(m_calibNodesDP);
  ::dic_release_service(m_subFarmDP);
  ::dic_release_service(m_nodesDP);
  ::dic_release_service(m_storSliceDP);
  ::dic_release_service(m_monSliceDP);
}

/// DIM command service callback
void PartitionListener::storSliceHandler(void* tag, void* address, int* size) {
  PartitionListener* h = *(PartitionListener**)tag;
  if(*size > 0)  {
    string slice = (char*)address;
    if ( slice.empty() ) {
      IocSensor::instance().send(h->m_parent,CMD_DISCONNECT_STORAGE,(void*)0);
    }
    else {
      string *s = new string(fmcLogger("STORECTL01",(char*)address));
      IocSensor::instance().send(h->m_parent,CMD_CONNECT_STORAGE,s);
    }
  }
}

/// DIM command service callback
void PartitionListener::monSliceHandler(void* tag, void* address, int* size) {
  PartitionListener* h = *(PartitionListener**)tag;
  if(*size > 0)  {
    string slice = (char*)address;
    if ( slice.empty() ) {
      IocSensor::instance().send(h->m_parent,CMD_DISCONNECT_MONITORING,(void*)0);
    }
    else {
      string *s = new string(fmcLogger("MONA08",(char*)address));
      IocSensor::instance().send(h->m_parent,CMD_CONNECT_MONITORING,s);
    }
  }
}

/// DIM command service callback
void PartitionListener::subFarmHandler(void* tag, void* address, int* size) {
  string svc;
  auto_ptr<_SV> f(new _SV());
  PartitionListener* h = *(PartitionListener**)tag;
  for(const char* data = (char*)address, *end=data+*size;data<end;data += strlen(data)+1)
    f->push_back(fmcLogger(data,facility));
  if ( h->name() == "LHCb" ) f->push_back(fmcLogger("CALD07",facility));
  IocSensor::instance().send(h->m_parent,CMD_UPDATE_FARMS,f.release());
}

static void get_nodes(void* address, int* size, _SV* n) {
  for(const char* data = (char*)address, *end=data+*size;data<end;data += strlen(data)+1) {
    in_addr addr;
    addr.s_addr = inet_addr(data);
#ifdef _WIN32
    hostent* he = gethostbyaddr((const char*)&addr,sizeof(addr),AF_INET);
#else
    hostent* he = gethostbyaddr(&addr,sizeof(addr),AF_INET);
#endif
    if ( he ) {
      string node = he->h_name;
      size_t idx = node.find("-d1"); 
      if ( idx != string::npos ) node = node.substr(0,idx); 
      idx = node.find(".");
      if ( idx != string::npos ) node = node.substr(0,idx); 
      for(size_t i=0; i<node.length();++i) node[i] = ::toupper(node[i]);
      n->push_back(node);
    }
    else {
      ::upic_write_message2("Failed to resolve network address:%s",data);
    }
  }
}

/// DIM command service callback
void PartitionListener::nodeHandler(void* tag, void* address, int* size) {
  auto_ptr<_SV> n(new _SV());
  PartitionListener* h = *(PartitionListener**)tag;
  get_nodes(address,size,n.get());
  n->push_back("STORECTL01");
  n->push_back("STORERECV01");
  n->push_back("STORERECV02");
  n->push_back("STORESTRM01");
  n->push_back("STORESTRM02");
  n->push_back("MONA08");
  n->push_back("MONA0801");
  n->push_back("MONA0802");
  n->push_back("MONA0803");
  n->push_back("MONA0804");
  n->push_back("MONA0805");
  n->push_back("MONA0806");
  n->push_back("MONA0807");
  n->push_back("MONA0808");
  n->push_back("MONA0809");
  n->push_back("MONA0810");
  if ( h->name() == "LHCb" ) n->push_back("CALD0701");
  IocSensor::instance().send(h->m_parent,CMD_UPDATE_NODES,n.release());
}

/// DIM command service callback
void PartitionListener::calibNodeHandler(void* tag, void* address, int* size) {
  auto_ptr<_SV> n(new _SV());
  PartitionListener* h = *(PartitionListener**)tag;
  get_nodes(address,size,n.get());
  if ( h ) {
    //IocSensor::instance().send(h->m_parent,CMD_ADD_NODES,n.release());
  }
}
