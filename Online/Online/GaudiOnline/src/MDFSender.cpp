// $Header: /afs/cern.ch/project/cvs/reps/lhcb/Online/GaudiOnline/src/MDFSender.cpp,v 1.3 2006-11-28 13:54:22 frankb Exp $
//	====================================================================
//  MDFSender.cpp
//	--------------------------------------------------------------------
//
//	Author    : Markus Frank
//
//	====================================================================
#include "GaudiOnline/MDFSender.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "GaudiKernel/DataObject.h"
#include "GaudiKernel/MsgStream.h"
#include "AMS/amsdef.h"
#include "WT/wtdef.h"

#include <cmath>

#include "GaudiKernel/AlgFactory.h"
DECLARE_NAMESPACE_ALGORITHM_FACTORY(LHCb,MDFSender);

/// Standard algorithm constructor
LHCb::MDFSender::MDFSender(const std::string& nam, ISvcLocator* pSvc)
:	MDFWriter(MDFIO::MDF_BANKS, nam, pSvc), m_current(0), m_allEvtSent(0), 
  m_allEvtError(0), m_allBytesSent(0)
{
  declareProperty("PrintFreq",   m_freq = 0.);
  declareProperty("Recipients",  m_recipients);
}

/// Standard Destructor
LHCb::MDFSender::~MDFSender()      {
}

/// Initialize
StatusCode LHCb::MDFSender::initialize()   {
  std::string nam;
  if ( m_recipients.empty() )  {
    MsgStream log(msgSvc(),name());
    log << MSG::FATAL << "You need to set the option recipients "
        << "to send event over the network!" << endmsg;
    return StatusCode::FAILURE;
  }
  m_current = 0;
  declareInfo("EvtCount", m_allEvtSent=0,  "Total number of events sent to receiver.");
  declareInfo("ErrCount", m_allEvtError=0, "Total number of event send errors to receiver.");
  declareInfo("ByteCount",m_allBytesSent=0,"Total number of bytes sent to receiver.");
  m_evtSent.resize(m_recipients.size()+1);
  m_evtError.resize(m_recipients.size()+1);
  m_bytesSent.resize(m_recipients.size()+1);
  for(size_t i=0; i<m_recipients.size(); ++i)  {
    nam = "EvtCount_" + m_recipients[i];
    declareInfo(nam, m_evtSent[i]=0,  "Number of events sent to:"+m_recipients[i]);
    nam = "ByteCount_" + m_recipients[i];
    declareInfo(nam, m_bytesSent[i]=0,"Number of bytes sent to:"+m_recipients[i]);
    nam = "ErrCount_" + m_recipients[i];
    declareInfo(nam, m_evtError[i]=0, "Number of event send errors to:"+m_recipients[i]);
  }
  return StatusCode::SUCCESS;
}

/// Finalize
StatusCode LHCb::MDFSender::finalize()     {    
  return StatusCode::FAILURE;
}

/// Write MDF record from serialization buffer
StatusCode LHCb::MDFSender::writeBuffer(void* const /* ioDesc */, const void* data, size_t len)  {
  longlong prtCount = fabs(m_freq) > 1./LONGLONG_MAX ? longlong(1./m_freq) : LONGLONG_MAX;
  /// Now send the data ....
  if( m_current >= m_recipients.size() )  {
    m_current = 0;
  }
  if ( m_recipients.empty() )  {
    MsgStream log(msgSvc(),name());
    log << MSG::ERROR << "Failed to send MDF record [No known recipient]." << endmsg;
    return StatusCode::FAILURE;
  }
  const std::string& tar = m_recipients[m_current];
  int sc = amsc_send_message( data, len, tar.c_str(), WT_FACILITY_DAQ_EVENT, 0);
  if ( AMS_SUCCESS != sc )   {
    MsgStream log(msgSvc(),name());
    log << MSG::ERROR << "Failed to send MDF to " << tar << ". status:" << sc << ". " << endmsg;
    ++m_evtError[m_current];
    ++m_allEvtError;
    return StatusCode::FAILURE;
  }
  m_allBytesSent += len;
  m_bytesSent[m_current] += len;
  ++m_allEvtSent;
  ++m_evtSent[m_current];
  ++m_current;
  if ( 0 == (m_allEvtSent%prtCount) )  {
    MsgStream log(msgSvc(),name());
    log << MSG::INFO << "Sent " << m_evtSent 
        << " Events with a total of " << m_allBytesSent << " Bytes." << endmsg;
  }
  return StatusCode::SUCCESS;
}
