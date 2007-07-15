// $Id: L0ETC.cpp,v 1.2 2007-07-15 16:57:48 pkoppenb Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/IRegistry.h" 

// LHCb
#include "Event/MCHeader.h"
#include "Event/L0DUReport.h"

// local
#include "L0ETC.h"

//-----------------------------------------------------------------------------
// Implementation file for class : L0ETC
//
// 2007-07-13 : Patrick Koppenburg
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( L0ETC );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
L0ETC::L0ETC( const std::string& name,
              ISvcLocator* pSvcLocator)
  : GaudiTupleAlg ( name , pSvcLocator )
    , m_events(0)
{
    declareProperty( "CollectionName", m_collectionName = "<not set>");
}
//=============================================================================
// Destructor
//=============================================================================
L0ETC::~L0ETC() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode L0ETC::initialize() {
  StatusCode sc = GaudiTupleAlg::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiTupleAlg

  debug() << "==> Initialize" << endmsg;

  /// @todo would be nice to get that from /Event or from L0DU.
  m_l0channels.push_back("L0electron_High");
  m_l0channels.push_back("L0electron_Low");
  m_l0channels.push_back("L0photon_High");
  m_l0channels.push_back("L0photon_Low");
  m_l0channels.push_back("L0localPi0_High");
  m_l0channels.push_back("L0localPi0_Low");
  m_l0channels.push_back("L0globalPi0_High");
  m_l0channels.push_back("L0globalPi0_Low");
  m_l0channels.push_back("L0hadron_High");
  m_l0channels.push_back("L0hadron_Low");
  m_l0channels.push_back("L0muon_High");
  m_l0channels.push_back("L0muon_Low");
  m_l0channels.push_back("L0dimuon_High");
  m_l0channels.push_back("L0dimuon_Low");

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode L0ETC::execute() {

  debug() << "==> Execute" << endmsg;

  // always pass, will use selection on tag to reduce later...
  setFilterPassed(true);

  m_events++ ;
  
  // This is what it is about...
  Tuple tup = evtCol(m_collectionName);
  
  // pick up the location of the event --
  // this is what makes the tag collection a collection...
  DataObject* pObject = get<DataObject>("/Event");
  if (pObject!=0) {
    StatusCode sc = tup->column("Address", pObject->registry()->address() ); 
    if (!sc) return sc;
  } else {
    Error("    not able to retrieve IOpaqueAddress");
  }
  
  // Retrieve informations about event
  if (exist<LHCb::MCHeader>(LHCb::MCHeaderLocation::Default)){
    const LHCb::MCHeader *evtHeader = get<LHCb::MCHeader>(LHCb::MCHeaderLocation::Default);
    tup->column("event", (int)evtHeader->evtNumber() );
    tup->column("run",   (int)evtHeader->runNumber() );
  } else {
    Warning("    not able to retrieve MCHeader");
  }

  // get L0 result
  if ( exist<LHCb::L0DUReport>(LHCb::L0DUReportLocation::Default)){
    LHCb::L0DUReport* L0 = get<LHCb::L0DUReport>(LHCb::L0DUReportLocation::Default);
    tup->column ( "L0",  L0->decision() ) ;
    for ( std::vector<std::string>::const_iterator c = m_l0channels.begin();
          c != m_l0channels.end(); ++c){
      verbose() << *c << " says " << L0->channelDecisionByName(*c) << endmsg ;
      tup->column ( (*c), L0->channelDecisionByName(*c)  ) ;
    }
  } else {
    Warning("    not able to retrieve L0DUReport");
    tup->column ( "L0", false ) ;
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode L0ETC::finalize() {

  debug() << "==> Finalize" << endmsg;
  info() << "Number of events " << m_events << endmsg ;

  return GaudiTupleAlg::finalize();  // must be called after all other actions
}

//=============================================================================
