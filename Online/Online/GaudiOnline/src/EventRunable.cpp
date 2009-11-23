// $Id: EventRunable.cpp,v 1.16 2009-11-23 07:26:46 frankb Exp $
#include "GaudiKernel/SmartIF.h"
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IAppMgrUI.h"
#include "GaudiKernel/SvcFactory.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiKernel/IDataProviderSvc.h"
#include "GaudiOnline/EventRunable.h"
#include "GaudiOnline/MEPManager.h"

// Instantiation of a static factory class used by clients to create instances of this service

DECLARE_NAMESPACE_SERVICE_FACTORY(LHCb,EventRunable)

using namespace LHCb;
using namespace std;

// Standard Constructor
EventRunable::EventRunable(const string& nam, ISvcLocator* svcLoc)   
: OnlineService(nam, svcLoc), m_mepMgr(0), m_dataSvc(0),
  m_receiveEvts(false), m_nerr(0), m_evtCount(0), m_errorFired(false), m_eventTMO(false)
{
  declareProperty("EvtMax",         m_evtMax=1);
  declareProperty("NumErrorToStop", m_nerrStop=-1);
  declareProperty("MEPManager",     m_mepMgrName="LHCb::MEPManager/MEPManager");
  declareProperty("TimeoutIncident",m_tmoIncident="DAQ_TIMEOUT");
  declareProperty("ForceTMOExit",   m_forceTMOExit = 0);
}

// Standard Destructor
EventRunable::~EventRunable()   
{
}

// IInterface implementation : queryInterface
StatusCode EventRunable::queryInterface(const InterfaceID& riid, void** ppIf)   {
  if ( riid == IRunable::interfaceID() )  {
    *ppIf = (IRunable*)this;
    addRef();
    return StatusCode::SUCCESS;
  }
  return OnlineService::queryInterface(riid, ppIf);
}

// IService implementation: initialize the service
StatusCode EventRunable::initialize()   {
  StatusCode sc = OnlineService::initialize();
  if ( !sc.isSuccess() )     {
    return error("Failed to initialize service base class.");
  }
  if ( !m_mepMgrName.empty() )  {
    if ( !(sc=service(m_mepMgrName,m_mepMgr)).isSuccess() )  {
      return error("Failed to access MEP manager service.");
    }
  }
  if( !(sc=service("EventDataSvc",m_dataSvc,true)).isSuccess() )  {
    return error("Error retrieving EventDataSvc interface IDataProviderSvc.");
  }
  incidentSvc()->addListener(this,"DAQ_CANCEL");
  incidentSvc()->addListener(this,"DAQ_ERROR");
  incidentSvc()->addListener(this,m_tmoIncident);
  declareInfo("EvtCount",m_evtCount=0,"Number of events processed");
  return sc;
}

// IService implementation: finalize the service
StatusCode EventRunable::finalize()     {
  releaseInterface(m_dataSvc);
  releaseInterface(m_mepMgr);
  return OnlineService::finalize();
}

/// Incident handler implemenentation: Inform that a new incident has occured
void EventRunable::handle(const Incident& inc)    {
  info("Got incident:"+inc.source()+" of type "+inc.type());
  if ( inc.type() == "DAQ_CANCEL" )  {
    m_receiveEvts = false;
    if ( !m_mepMgrName.empty() )  {
      if ( 0 == m_mepMgr ) {
        error("Got incident:"+inc.source()+
              " -- Internal error:"+m_mepMgrName+" is not assigned.");
      }
      else {
        m_mepMgr->cancel();
      }
    }
  }
  else if ( inc.type() == "DAQ_ERROR" )  {
    m_errorFired = true;
    if ( !m_mepMgrName.empty() )  {
      if ( 0 == m_mepMgr ) {
        error("Got incident:"+inc.source()+
              " -- Internal error:"+m_mepMgrName+" is not assigned.");
      }
      else {
        m_mepMgr->cancel();
      }
    }
  }
  else if ( inc.type() == "DAQ_ENABLE" )  {
    m_receiveEvts = true;
  }
  else if ( inc.type() == m_tmoIncident )  {
    m_eventTMO = true;
  }
}

/// IRunable implementation : Run the class implementation
StatusCode EventRunable::run()   {
  SmartIF<IAppMgrUI> ui(serviceLocator());
  if ( ui )    {
    m_receiveEvts = true;
    while ( m_receiveEvts )   {
      // loop over the events
      
      if(m_errorFired) {
	//    Incident incident(name(),"DAQ_ERROR");
	//    m_incidentSvc->fireIncident(incident);
        return error("An error was reported. Aborting execution.");
      }
      
      DataObject* pObj = 0;
      m_eventTMO = false;
      StatusCode sc = StatusCode::SUCCESS;
      try  {
	sc = ui->nextEvent(m_evtMax);
      }
      catch(const exception& e) {
	sc = m_eventTMO ? StatusCode::SUCCESS : StatusCode::FAILURE;
	info(string("Caught unhandled exception in main eventy loop:")+e.what());
      }
      catch(...) {
	sc = m_eventTMO ? StatusCode::SUCCESS : StatusCode::FAILURE;
	info(string("Caught unknown exception in main eventy loop."));
      }
      if ( sc.isSuccess() )  {
        m_evtCount++;
	if ( m_eventTMO )    {
	  m_incidentSvc->fireIncident(Incident(name(),"EVENT_TIMEOUT"));
	  if ( m_forceTMOExit ) {
	    Incident incident(name(),"DAQ_ERROR");
	    m_incidentSvc->fireIncident(incident);
	    return StatusCode::FAILURE;
	  }
	}
        if ( m_nerr > 0 )  {
          Incident incident(name(),"DAQ_ERROR_CLEAR");
          m_incidentSvc->fireIncident(incident);
        }
        m_nerr = 0;
        if ( !m_dataSvc->findObject("/Event",pObj).isSuccess() )  {
          info("End of event input reached.");
          break;
        }
        continue;
      }
      if ( !m_receiveEvts ) {	
	info("Cancel received: Stop event processing.");
	break;
      }
      else if ( ui->FSMState() != Gaudi::StateMachine::RUNNING ) {
	info("Cancel received: Stop event processing -- no more in state RUNNING.");
	break;
      }

      /// Consecutive errors: go into error state
      error("Failed to process event.");
      m_nerr++;
      if ( (m_nerrStop > 0) && (m_nerr > m_nerrStop) )  {
        Incident incident(name(),"DAQ_ERROR");
        m_incidentSvc->fireIncident(incident);
        return StatusCode::FAILURE;
      }
      //else if ( m_nerr > 0 )  {
      //  Incident incident(name(),"DAQ_ERROR");
      //  m_incidentSvc->fireIncident(incident);
      //}
    }
    return StatusCode::SUCCESS;
  }
  return error("Failed to access Application manager UI.");
}
