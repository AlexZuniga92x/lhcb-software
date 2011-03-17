// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/GaudiOnline/src/MEPManager.cpp,v 1.25 2010-09-21 14:28:25 frankb Exp $
//  ====================================================================
//  MEPManager.cpp
//  --------------------------------------------------------------------
//
//  Author    : Markus Frank
//
//  ====================================================================
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/MsgStream.h"
#include "GaudiKernel/xtoa.h"
#include "GaudiOnline/MEPManager.h"
#include "RTL/rtl.h"
#include "MBM/bmdef.h"
#include "MBM/Producer.h"
#include <stdexcept>
#include <cctype>
#include <cstdio>

using namespace std;
using namespace LHCb;

extern "C" int mep_install(int argc , char** argv);

/// Standard service constructor
MEPManager::MEPManager(const string& nam, ISvcLocator* loc)
: Service(nam, loc), m_partitionID(0x103), m_mepID(MEP_INV_DESC)
{
  m_procName = RTL::processName();
  declareProperty("Buffers",          m_buffers);
  declareProperty("PartitionID",      m_partitionID);
  declareProperty("PartitionName",    m_partitionName="");
  declareProperty("InitFlags",        m_initFlags);
  declareProperty("PartitionBuffers", m_partitionBuffers=false);
  declareProperty("MapUnusedBuffers", m_mapUnused=true);
  declareProperty("HandleSignals",    m_handleSignals=false);
  declareProperty("ConnectWhen",      m_connectWhen="initialize");
}

/// Default destructor
MEPManager::~MEPManager()    {
}

StatusCode MEPManager::error(const string& msg)   const {
  MsgStream err(msgSvc(), name());
  err << MSG::ERROR << msg << endmsg;
  return StatusCode::FAILURE;
}

/// Query interfaces of Interface
StatusCode MEPManager::queryInterface(const InterfaceID& riid,
                                            void** ppvInterface) 
{
  if ( IMEPManager::interfaceID().versionMatch(riid) )   {
    *ppvInterface = (IMEPManager*)this;
    addRef();
    return SUCCESS;
  }
  return Service::queryInterface(riid,ppvInterface);
}

/// Initialize buffers for MEP usage
StatusCode MEPManager::initializeBuffers()  {
  MsgStream log(msgSvc(), name());
  if ( !m_initFlags.empty() )  {
    size_t ikey = 0;
    char *items[64], txt[32];
    string tmp = m_initFlags;
    for(char* tok=::strtok((char*)tmp.c_str()," "); tok; tok=::strtok(NULL," ")) {
      if ( m_partitionBuffers && ::toupper(tok[1]) == 'I' )  {
        if(strcmp(tok+3,"MEP") && strcmp(tok+3,"EVENT") && strcmp(tok+3,"RESULT")) {
          string bm_name = tok;
          bm_name += "_";
          if ( m_partitionName.empty() )
	    bm_name += _itoa(m_partitionID,txt,16);
	  else
	    bm_name += m_partitionName;
          items[ikey++] = strcpy(new char[bm_name.length()+1],bm_name.c_str());
          continue;
        }
      }
      items[ikey++] = strcpy(new char[strlen(tok)+1],tok);
    }
    for(size_t i=0; i<ikey; ++i)  {
      if ( ::strchr(items[i],' ') != 0 ) {
        *strchr(items[i],' ') = 0;
      }
    }
    StatusCode sc = mep_install(ikey, items);
    for(size_t j=0; j<ikey; ++j)  {
      delete [] items[j];
    }
    return sc;
  }
  return StatusCode::SUCCESS;
}

/// Connect to optional MBM buffer
StatusCode MEPManager::connectBuffer(const string& nam)  {
  char txt[32];
  string bm_name = nam;
  if ( m_partitionBuffers ) {
    bm_name += "_";
    if ( m_partitionName.empty() )
      bm_name += _itoa(m_partitionID,txt,16);
    else
      bm_name += m_partitionName;
  }
  if( m_buffMap.find(bm_name) == m_buffMap.end() ) {
    BMID bmid = ::mbm_include(bm_name.c_str(),m_procName.c_str(),m_partitionID);
    if ( bmid == MBM_INV_DESC )  {
      return error("Failed to connect to buffer "+bm_name+" as "+m_procName);
    }
    m_bmIDs.push_back(bmid);
    m_buffMap[bm_name] = bmid;
    m_buffMap[nam] = bmid;
  }
  return StatusCode::SUCCESS;
}

/// Connect to specified buffers
StatusCode MEPManager::connectBuffers()  {
  if ( m_buffers.size() > 0 )  {
    MsgStream log(msgSvc(), name());
    typedef vector<string> _V;
    int flags = 0;
    for(_V::const_iterator i=m_buffers.begin(); i != m_buffers.end(); ++i )  {
      const string& nam = *i;
      if ( nam == "EVENT" )
        flags |= USE_EVT_BUFFER;
      else if ( nam == "RESULT" )
        flags |= USE_RES_BUFFER;
      else if ( nam == "MEP" )
        flags |= USE_MEP_BUFFER;
      else if ( !connectBuffer(nam).isSuccess() )
        return error("Failed to connect to MBM buffer:"+nam);
      else 
        log << MSG::DEBUG << "Included in MBM buffer:" << nam << endmsg;
    }
    if ( flags != 0 )  {
      log << MSG::DEBUG << "Including in MEP buffers" << endmsg;
      m_mepID = ::mep_include(m_procName.c_str(), m_partitionID, flags);
      if ( m_mepID == MEP_INV_DESC )  {
        return error("Failed to include into MEP buffers!");
      }
      if ( m_handleSignals ) {
	::mep_set_signal_handler(m_mepID,true);
      }
      if ( flags&USE_MEP_BUFFER ) {
	log << MSG::DEBUG << " MEP    buffer start: " << (void*)m_mepID->mepStart << endmsg;
      }
      if ( flags&USE_EVT_BUFFER ) {
	log << MSG::DEBUG << " EVENT  buffer start: " << (void*)m_mepID->evtStart << endmsg;
      }
      if ( flags&USE_RES_BUFFER ) {
	log << MSG::DEBUG << " RESULT buffer start: " << (void*)m_mepID->resStart << endmsg;
      }
    }
  }
  return StatusCode::SUCCESS;
}

StatusCode MEPManager::initialize()  {
  StatusCode sc = Service::initialize();
  if ( !sc.isSuccess() )  {
    return error("Failed to initialize base class Service.");
  }
  return (m_connectWhen == "initialize") ? i_init() : sc;
}

StatusCode MEPManager::start()  {
  StatusCode sc = Service::start();
  if ( !sc.isSuccess() )  {
    return error("Failed to initialize base class Service.");
  }
  return (m_connectWhen == "start") ? i_init() : sc;
}

StatusCode MEPManager::restart()  {
  return StatusCode::SUCCESS;
}

StatusCode MEPManager::i_init()  {
  MsgStream log(msgSvc(), name());
  setProperties();
  m_procName = RTL::processName();
  log << MSG::INFO << m_connectWhen << "> Process name:" << m_procName
      << " Connecting to MBM Buffers." << endmsg;
  m_bmIDs.clear();
  m_buffMap.clear();
  mep_map_unused_buffers(m_mapUnused);
  if ( !initializeBuffers().isSuccess() )  {
    return error("Failed to initialize MEP buffers!");
  }
  if ( !connectBuffers().isSuccess() )  {
    return error("Failed to connect to MEP buffers!");
  }
  return StatusCode::SUCCESS;
}

StatusCode MEPManager::finalize()  {
  if (m_connectWhen == "initialize") i_fini();
  return Service::finalize();
}

StatusCode MEPManager::stop()  {
  if (m_connectWhen == "start") i_fini();
  return Service::finalize();
}

StatusCode MEPManager::i_fini()  {
  MsgStream log(msgSvc(), name());
  log << MSG::INFO << "Excluding from buffers. No more buffer access possible." << endmsg;
  m_buffMap.clear();
  if ( m_mepID != MEP_INV_DESC )  {
    if ( m_handleSignals ) {
      ::mep_set_signal_handler(m_mepID,false);
    }
    mep_exclude(m_mepID);
    m_mepID = MEP_INV_DESC;
  }
  for(vector<BMID>::iterator i=m_bmIDs.begin(); i != m_bmIDs.end(); ++i)  {
    if ( *i != MBM_INV_DESC ) ::mbm_exclude(*i);
  }
  m_bmIDs.clear();
  return StatusCode::SUCCESS;
}

/// Cancel connection to specified buffers
StatusCode MEPManager::cancel()  {
  if ( m_mepID != MEP_INV_DESC ) {
    ::mep_cancel_request(m_mepID);
  }
  for(vector<BMID>::iterator i=m_bmIDs.begin(); i != m_bmIDs.end(); ++i)  {
    if ( *i != MBM_INV_DESC ) ::mbm_cancel_request(*i);
  }
  return StatusCode::SUCCESS;
}

/// Create producer
MBM::Producer* MEPManager::createProducer(const string& buffer,const string& instance) {
  map<string,BMID>::iterator i=m_buffMap.find(buffer);
  if ( i == m_buffMap.end() ) {
    string bm_name = buffer;
    bm_name += "_";
    if ( m_partitionName.empty() ) {
      char txt[32];
      bm_name += _itoa(m_partitionID,txt,16);
      i=m_buffMap.find(buffer);
    }
    else  {
      bm_name += m_partitionName;
      i=m_buffMap.find(buffer);
    }
  }

  if ( i != m_buffMap.end() ) {
    BMID bmid = (*i).second;
    return new MBM::Producer(bmid,instance,partitionID());
  }
  return 0;
}

DECLARE_NAMESPACE_SERVICE_FACTORY(LHCb,MEPManager)
