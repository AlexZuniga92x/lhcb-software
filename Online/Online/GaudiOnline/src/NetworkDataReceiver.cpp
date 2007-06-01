// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/GaudiOnline/src/NetworkDataReceiver.cpp,v 1.5 2007-06-01 13:49:45 frankm Exp $
//  ====================================================================
//  NetworkDataReceiver.cpp
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
#include "GaudiOnline/NetworkDataReceiver.h"
#include "GaudiOnline/MEPManager.h"
#include "GaudiKernel/strcasecmp.h"
#include "GaudiKernel/xtoa.h"
#include "Event/RawBank.h"
#include "MDF/MDFHeader.h"
#include "MBM/Producer.h"
#include "MBM/mepdef.h"
#include "RTL/Lock.h"
#include "RTL/rtl.h"
#include "WT/wtdef.h"
#include <algorithm>
#include <memory>
#include <cmath>

using namespace LHCb;
static const std::string s_reqMsg("EVENT_REQUEST");

// Standard algorithm constructor
NetworkDataReceiver::NetworkDataReceiver(const std::string& nam, ISvcLocator* pSvc)
: OnlineService(nam,pSvc), m_recvReq(0), m_recvError(0), m_recvBytes(0), 
  m_backlog(0), m_lastbacklog(0), m_mepMgr(0), m_prod(0), m_evtSelector(0)
{
  ::wtc_init();
  ::lib_rtl_create_lock(0,&m_lock);
  declareProperty("Buffer",           m_buffer);
  declareProperty("UseEventRequests", m_useEventRequests=false);
}

// Standard Destructor
NetworkDataReceiver::~NetworkDataReceiver()      {
  ::lib_rtl_delete_lock(m_lock);
}

// WT callback to declare asynchronously event data to MBM
int NetworkDataReceiver::rearm_net_request(unsigned int,void* param)  {
  std::auto_ptr<RecvEntry> h((RecvEntry*)param);
  h->handler->declareEventData(*h.get());
  return WT_SUCCESS;
}

// Initialize the object: allocate all necessary resources
StatusCode NetworkDataReceiver::initialize()   {
  StatusCode sc = OnlineService::initialize();
  MsgStream err(msgSvc(), name());
  if ( !sc.isSuccess() )  {
    return sc;
  }
  // Do NOT call base class initialization: we are not writing to file/socket!
  declareInfo("RecvCount",  m_recvReq=0,   "Total number of items received.");
  declareInfo("RecvErrors", m_recvError=0, "Total number of receive errors.");
  declareInfo("RecvBytes",  m_recvBytes=0, "Total number of bytes received from clients.");
  sc = service("MEPManager",m_mepMgr);
  if ( !sc.isSuccess() )  {
    info("Failed to access buffer manager service.");
    return sc;
  }  
  sc = subscribeNetwork();
  if ( !sc.isSuccess() )  {
    return sc;
  }
  m_prod = m_mepMgr->createProducer(m_buffer,RTL::processName());
  ::wtc_subscribe(WT_FACILITY_DAQ_EVENT,0,rearm_net_request,this);
  incidentSvc()->addListener(this,"DAQ_CANCEL");
  m_lastbacklog = 0;
  m_backlog = 0;
  return sc;
}

// Finalize the object: release all allocated resources
StatusCode NetworkDataReceiver::finalize()     {
  ::wtc_flush(WT_FACILITY_DAQ_EVENT);
  ::wtc_remove(WT_FACILITY_DAQ_EVENT);
  unsubscribeNetwork(); // Cannot really do anything if this fails...except printing
  for(Receivers::iterator i=m_receivers.begin(); i != m_receivers.end(); ++i)
    deleteNetRequest(*i);
  m_receivers.clear();
  if ( m_prod ) delete m_prod;
  m_prod = 0;
  if ( m_mepMgr ) m_mepMgr->release();
  m_mepMgr = 0;
  undeclareInfo("RecvCount");
  undeclareInfo("RecvErrors");
  undeclareInfo("RecvBytes");
  return OnlineService::finalize();
}

/// Incident handler implemenentation: Inform that a new incident has occured
void LHCb::NetworkDataReceiver::handle(const Incident& inc)    {
  if ( inc.type() == "DAQ_CANCEL" )  {
    MsgStream info(msgSvc(), name());
    info << MSG::INFO << "Executing DAQ_CANCEL" << endmsg;
    ::wtc_flush(WT_FACILITY_DAQ_EVENT);
    m_mepMgr->cancel();
  }
}

// Handle event data source registration
StatusCode 
NetworkDataReceiver::handleSourceRequest(int clientID,
                                         const std::string& source,
                                         const std::string& svc)  
{
  try {
    if ( 0 == receiver(source) )  {
      m_receivers.push_back(RecvEntry(this, source, svc, clientID));
      RecvEntry& entry = m_receivers.back();
      StatusCode sc = createNetRequest(entry);
      if ( sc.isSuccess() )  {
        MsgStream log(msgSvc(), name());
        log << MSG::DEBUG << "Added new datasource [" << clientID << "]:" << source 
            << " [" << svc << "]" << endmsg;
        return rearmRequest(entry);
      }
    }
    MsgStream info(msgSvc(), name());
    info << MSG::INFO << "Unknown datasource request from:" << source 
         << " [" << svc << "]" << endmsg;
    return StatusCode::SUCCESS;
  }
  catch(const std::exception& e)   {
    return errorException("Exception during data source request:",e);
  }
  catch(...)   {
    return error("Unknown exception during data source request.");
  }
  return StatusCode::FAILURE;
}

// Producer implementation: Request event from data source
NetworkDataReceiver::RecvEntry* NetworkDataReceiver::receiver(const std::string& nam)  {
  for(Receivers::iterator i=m_receivers.begin(); i!=m_receivers.end();++i)  {
    if ( 0 == strcasecmp(nam.c_str(),(*i).name.c_str()) )  {
      return &(*i);
    }
  }
  return 0;
}

/// Rearm network request for a single event source
StatusCode NetworkDataReceiver::rearmRequest(const RecvEntry& entry)   {
  if ( m_useEventRequests )  {
    return rearmNetRequest(entry);
  }
  return StatusCode::SUCCESS;
}

// Create network request for a single source
StatusCode NetworkDataReceiver::createNetRequest(RecvEntry& /* entry */)  {
  return StatusCode::SUCCESS;
}

// Reset event request
StatusCode NetworkDataReceiver::resetNetRequest(const RecvEntry& /* entry */)  {
  return StatusCode::SUCCESS;
}

// Rearm network request for a single source
void NetworkDataReceiver::deleteNetRequest(RecvEntry& /* entry */)  {
}

// Reset event request and insert entry into data queue of the buffer manager
StatusCode 
NetworkDataReceiver::handleEventData(const std::string& src,void* buf,size_t len)  {
  try  {
    RecvEntry* entry = receiver(src);
    if ( entry )  {
      // Need to clone entry - new message may be arriving before this one is handled...
      // (In mode without explicit rearm)
      RecvEntry* e = new RecvEntry(*entry);
      e->buffer = (char*)buf;
      e->size = len;
      StatusCode sc = resetNetRequest(*entry);
      ::wtc_insert(WT_FACILITY_DAQ_EVENT,e);
      int backlog = ++m_backlog;
      if ( backlog%100 == 0 ) {
        if ( m_lastbacklog != backlog ) {
          m_lastbacklog = backlog;
          error("Message backlog is now %d messages.",backlog);
	}
      }
      return sc;
    }
    error("Event receive entry from unknown source:"+src);
  }
  catch(std::exception& e)   {
    return errorException("Exception during event receive request:",e);
  }
  catch(...)   {
    return error("Unknown exception during event receive request.");
  }
  return StatusCode::FAILURE;
}

// Copy event data into buffer manager
StatusCode NetworkDataReceiver::copyEventData(void* to, void* from, size_t len)  {
  ::memcpy(to, from, len);
  return StatusCode::SUCCESS;
}

// Handle event declaration into the buffer manager
StatusCode NetworkDataReceiver::declareEventData(RecvEntry& entry)  {
  MsgStream log(msgSvc(), name());
  try {
    int ret = m_prod->getSpace(entry.size);
    if ( ret == MBM_NORMAL ) {
      MBM::EventDesc& e = m_prod->event();
      RawBank*   b = (RawBank*)e.data;
      MDFHeader* h = (MDFHeader*)b->data();
      e.type       = EVENT_TYPE_EVENT;
      e.len        = entry.size;
      if ( copyEventData(e.data,entry.buffer,entry.size).isSuccess() )  {
        ::memcpy(e.mask,h->subHeader().H1->triggerMask(),sizeof(e.mask));
        ret = m_prod->sendEvent();
        if ( MBM_NORMAL == ret )   {
          m_backlog--;
          return rearmRequest(entry);
        }
      }
      log << MSG::ERROR << "Failed to retrieve network event data from:" << entry.name << endmsg;
      if ( ret != MBM_REQ_CANCEL ) rearmRequest(entry);
      return StatusCode::SUCCESS;
    }
    // Cannot do anything - must handle and rearm new request
    log << MSG::ERROR << "Failed to get space for buffer manager." << endmsg;
    if ( ret != MBM_REQ_CANCEL ) rearmRequest(entry);
    return StatusCode::SUCCESS;
  }
  catch(std::exception& e)  {
    log << MSG::ERROR << "Got exception when declaring event from:" << entry.name << endmsg;
    log << e.what() << endmsg;
  }
  catch(...)  {
    log << MSG::ERROR << "Got unknown exception when declaring event from:" << entry.name << endmsg;
  }
  return rearmRequest(entry);
}

// Issue alarm message to error logger
void NetworkDataReceiver::sendAlarm(const std::string& msg)  {
  MsgStream error(msgSvc(), name());
  error << MSG::ERROR << msg << endmsg;
}

// Callback on task dead notification
StatusCode NetworkDataReceiver::taskDead(const std::string& task_name)  {
  MsgStream info(msgSvc(), name());
  info << MSG::INFO << "Dead task:" << task_name << "...." << endmsg;
  for(Receivers::iterator i=m_receivers.begin(); i!=m_receivers.end();++i)  {
    if ( 0 == strcasecmp(task_name.c_str(),(*i).name.c_str()) )  {
      m_receivers.erase(i);
      break;
    }
  }
  sendAlarm("Event provider:"+task_name+" died.");
  return StatusCode::SUCCESS;
}
