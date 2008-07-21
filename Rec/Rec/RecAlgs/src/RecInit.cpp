// $Id: RecInit.cpp,v 1.7 2008-07-21 14:39:30 cattanem Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/GaudiException.h"
#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/Incident.h"
#include "GaudiKernel/IIncidentSvc.h"
#include "GaudiAlg/IGenericTool.h"

// from EventBase
#include "Event/ProcessHeader.h"

// from DAQEvent
#include "Event/RawEvent.h"
#include "Event/RawBank.h"
#include "Event/ODIN.h"

// from RecEvent
#include "Event/RecHeader.h"
#include "Event/ProcStatus.h"

// local
#include "RecInit.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RecInit
//
// 2006-03-14 : Marco Cattaneo
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( RecInit );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RecInit::RecInit( const std::string& name,
                  ISvcLocator* pSvcLocator)
  : LbAppInit ( name , pSvcLocator )
{

}
//=============================================================================
// Destructor
//=============================================================================
RecInit::~RecInit() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode RecInit::initialize() {
  StatusCode sc = LbAppInit::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  // Private tool to plot the memory usage
  m_memoryTool = tool<IGenericTool>( "MemoryTool", "BrunelMemory", this, true );

  // Pointer to IncidentSvc
  m_incidentSvc = svc<IIncidentSvc>("IncidentSvc",true);

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode RecInit::execute() {

  StatusCode sc = LbAppInit::execute(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by LbAppInit

  debug() << "==> Execute" << endmsg;
  // Plot the memory usage
  m_memoryTool->execute();

  // Get the run and event number from the ODIN bank
  LHCb::ODIN* odin = 0;
  try {
    odin = get<LHCb::ODIN> ( LHCb::ODINLocation::Default );
  }
  catch( const GaudiException& Exception ) {
    m_incidentSvc->fireIncident(Incident(name(),IncidentType::AbortEvent));
    this->setFilterPassed( false );    
    return Error( "ODIN missing, skipping event", StatusCode::SUCCESS );
  }
  
  unsigned int runNumber = odin->runNumber();
  ulonglong    evtNumber = odin->eventNumber();
  
  this->printEventRun( evtNumber, runNumber );

  // Initialize the random number
  std::vector<long int> seeds = getSeeds( runNumber, evtNumber );
  sc = this->initRndm( seeds );
  if ( sc.isFailure() ) return sc;  // error printed already by initRndm


  // Create the Reconstruction event header
  LHCb::RecHeader* header = new LHCb::RecHeader();
  header->setApplicationName( this->appName() );
  header->setApplicationVersion( this->appVersion() );
  header->setRunNumber( runNumber );
  header->setEvtNumber( evtNumber );
  header->setRandomSeeds( seeds );
  header->setCondDBTags( this->condDBTags() );
  put( header, LHCb::RecHeaderLocation::Default );

  // Create a ProcStatus if it does not already exist
  if( !exist<LHCb::ProcStatus>( LHCb::ProcStatusLocation::Default ) ) {
    LHCb::ProcStatus* procStat = new LHCb::ProcStatus();
    put( procStat, LHCb::ProcStatusLocation::Default );
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
