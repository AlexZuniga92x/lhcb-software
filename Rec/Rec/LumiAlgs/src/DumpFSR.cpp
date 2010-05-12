// $Id: DumpFSR.cpp,v 1.3 2010-05-12 08:11:08 panmanj Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IOpaqueAddress.h"
#include "GaudiKernel/IDataManagerSvc.h"

// event model
#include "Event/RawEvent.h"
#include "Event/ODIN.h"
#include "Event/LumiFSR.h"
#include "Event/EventCountFSR.h"
#include "Event/TimeSpanFSR.h"

// local
#include "DumpFSR.h"

//-----------------------------------------------------------------------------
// Implementation file for class : DumpFSR
// 
// 2009-02-27 : Jaap Panman
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( DumpFSR );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
DumpFSR::DumpFSR( const std::string& name,
                                    ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ),
    m_incSvc(0)
{
  // need to get the registry
  declareProperty( "RawEventLocation"   , m_rawEventLocation = LHCb::RawEventLocation::Default );
  // expect the data to be written at LHCb::LumiFSRLocation::Default
  declareProperty( "FileRecordLocation" , m_FileRecordName    = "/FileRecords"  );
  declareProperty( "FSRName"            , m_FSRName           = "/LumiFSR"     );
  declareProperty( "LowFSRName"         , m_LowFSRName        = "/LumiLowFSR"     );
  declareProperty( "EventCountFSRName"  , m_EventCountFSRName = "/EventCountFSR");
  declareProperty( "TimeSpanFSRName"    , m_TimeSpanFSRName   = "/TimeSpanFSR");
}
//=============================================================================
// Destructor
//=============================================================================
DumpFSR::~DumpFSR() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode DumpFSR::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  // get the File Records service
  m_fileRecordSvc = svc<IDataProviderSvc>("FileRecordDataSvc", true);
  // incident service
  m_incSvc = svc<IIncidentSvc> ( "IncidentSvc" , true );
  
  //check extended file incidents are defined
#ifdef GAUDI_FILE_INCIDENTS
  m_incSvc->addListener( this, IncidentType::BeginInputFile);
  m_incSvc->addListener( this, IncidentType::EndInputFile);
  if ( msgLevel(MSG::DEBUG) ) debug() << "registered with incSvc" << endmsg;
  //if not then the counting is not reliable
#else
  warn() << "cannot register with incSvc" << endmsg;
#endif //GAUDI_FILE_INCIDENTS

  // counting 
  m_current_fname = "";
  m_count_files = 0;
  m_count_events = 0;
  m_events_in_file = 0;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode DumpFSR::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;
  m_count_events++;
  m_events_in_file++;

  // wait after exactly one event on a file
  if ( m_events_in_file == 1 ) {
    dump_file( "After First Event" );
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode DumpFSR::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;
  info() << "number of files seen: " << m_count_files << endmsg;
  info() << "number of events seen: " << m_count_events << endmsg;

  dump_file("Finalize");

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

// ==========================================================================
// IIncindentListener interface
// ==========================================================================
void DumpFSR::handle( const Incident& incident )
{
  //check extended file incidents are defined
#ifdef GAUDI_FILE_INCIDENTS
  if(incident.type()==IncidentType::BeginInputFile)
  {
    m_current_fname = incident.source();
    if ( msgLevel(MSG::DEBUG) ) debug() << "==>from handle " << m_current_fname << endmsg;
    m_count_files++;
    m_events_in_file = 0;
    dump_file( "BeginInputFile" );

  }
  if(incident.type()==IncidentType::EndInputFile)
  {
    m_current_fname = incident.source();
    if ( msgLevel(MSG::DEBUG) ) debug() << "==>from handle " << m_current_fname << endmsg;
    dump_file( "EndInputFile" );

  }
#endif

}

//=============================================================================
void DumpFSR::dump_file( std::string txt ) {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==>" << txt << " " << m_current_fname << endmsg;


  // make an inventory of the FileRecord store
  std::string fileRecordRoot = m_FileRecordName;
  std::vector< std::string > addresses = navigate(fileRecordRoot, m_FSRName);
  for(std::vector< std::string >::iterator iAddr = addresses.begin() ; 
      iAddr != addresses.end() ; ++iAddr ){
    if ( msgLevel(MSG::INFO) ) {
      info() << "lu address: " << (*iAddr) << endmsg;
      std::string lumiRecordAddress = *iAddr;
      // read LumiFSR 
      if ( !exist<LHCb::LumiFSRs>(m_fileRecordSvc, lumiRecordAddress) ) {
	if ( msgLevel(MSG::WARNING) ) warning() << lumiRecordAddress << " not found" << endmsg ;
      } else {
	if ( msgLevel(MSG::DEBUG) ) verbose() << lumiRecordAddress << " found" << endmsg ;
	LHCb::LumiFSRs* lumiFSRs = get<LHCb::LumiFSRs>(m_fileRecordSvc, lumiRecordAddress);
	// look at all LumiFSRs (normally only one)
	LHCb::LumiFSRs::iterator lufsr;
	for ( lufsr = lumiFSRs->begin(); lufsr != lumiFSRs->end(); lufsr++ ) {
	  if ( msgLevel(MSG::INFO) ) info() << lumiRecordAddress << ": LumiFSR: " << *(*lufsr) << endmsg;
	}
      }
    }
  }  

  // make an inventory of the FileRecord store (LowLumi)
  std::vector< std::string > loAddresses = navigate(fileRecordRoot, m_LowFSRName);
  for(std::vector< std::string >::iterator iAddr = loAddresses.begin() ; 
      iAddr != loAddresses.end() ; ++iAddr ){
    if ( msgLevel(MSG::INFO) ) {
      info() << "lo address: " << (*iAddr) << endmsg;
      std::string lumiRecordAddress = *iAddr;
      // read LumiLowFSR 
      if ( !exist<LHCb::LumiFSRs>(m_fileRecordSvc, lumiRecordAddress) ) {
	if ( msgLevel(MSG::WARNING) ) warning() << lumiRecordAddress << " not found" << endmsg ;
      } else {
	if ( msgLevel(MSG::DEBUG) ) verbose() << lumiRecordAddress << " found" << endmsg ;
	LHCb::LumiFSRs* lumiFSRs = get<LHCb::LumiFSRs>(m_fileRecordSvc, lumiRecordAddress);
	// look at all LumiFSRs (normally only one)
	LHCb::LumiFSRs::iterator lufsr;
	for ( lufsr = lumiFSRs->begin(); lufsr != lumiFSRs->end(); lufsr++ ) {
	  if ( msgLevel(MSG::INFO) ) info() << lumiRecordAddress << ": LumiLowFSR: " << *(*lufsr) << endmsg;
	}
      }
    }
  }  
  
  //touch all EventCountFSRs
  std::vector< std::string > evAddresses = navigate(fileRecordRoot, m_EventCountFSRName);
  for(std::vector< std::string >::iterator iAddr = evAddresses.begin() ; 
      iAddr != evAddresses.end() ; ++iAddr ){
    if ( msgLevel(MSG::INFO) ) {
      info() << "ev address: " << (*iAddr) << endmsg;
      std::string eventCountRecordAddress = *iAddr;
      // read EventCountFSR 
      if ( !exist<LHCb::EventCountFSR>(m_fileRecordSvc, eventCountRecordAddress) ) {
	if ( msgLevel(MSG::WARNING) ) warning() << eventCountRecordAddress << " not found" << endmsg ;
      } else {
	if ( msgLevel(MSG::DEBUG) ) verbose() << eventCountRecordAddress << " found" << endmsg ;
	LHCb::EventCountFSR* eventCountFSR = get<LHCb::EventCountFSR>(m_fileRecordSvc, eventCountRecordAddress);
	// look at the EventCountFSR
	if ( msgLevel(MSG::INFO) ) info() << eventCountRecordAddress << ": EventCountFSR: " << *eventCountFSR << endmsg;
      }
    }
  }  
  
  //touch all TimeSpanFSRs (independently of the LumiFSRs)
  std::vector< std::string > tsAddresses = navigate(fileRecordRoot, m_TimeSpanFSRName);
  for(std::vector< std::string >::iterator iAddr = tsAddresses.begin() ; 
      iAddr != tsAddresses.end() ; ++iAddr ){
    if ( msgLevel(MSG::INFO) ) {
      info() << "ts address: " << (*iAddr) << endmsg;
      std::string timeSpanRecordAddress = *iAddr;
      // read TimeSpanFSR 
      if ( !exist<LHCb::TimeSpanFSRs>(m_fileRecordSvc, timeSpanRecordAddress) ) {
	if ( msgLevel(MSG::WARNING) ) warning() << timeSpanRecordAddress << " not found" << endmsg ;
      } else {
	if ( msgLevel(MSG::DEBUG) ) verbose() << timeSpanRecordAddress << " found" << endmsg ;
	LHCb::TimeSpanFSRs* timeSpanFSRs = get<LHCb::TimeSpanFSRs>(m_fileRecordSvc, timeSpanRecordAddress);
	// look at all TimeSpanFSRs (normally only one)
	LHCb::TimeSpanFSRs::iterator tsfsr;
	for ( tsfsr = timeSpanFSRs->begin(); tsfsr != timeSpanFSRs->end(); tsfsr++ ) {
	  if ( msgLevel(MSG::INFO) ) info() << timeSpanRecordAddress << ": TimeSpanFSR: " << *(*tsfsr) << endmsg;
	  ulonglong t0 = (*tsfsr)->earliest();
	  ulonglong t1 = (*tsfsr)->latest();
	  if ( msgLevel(MSG::DEBUG) ) debug() << timeSpanRecordAddress << "interval: " << t0 << "-" << t1 << endmsg;
	}
      }
    }
  }  
}

//=============================================================================
std::string DumpFSR::fileID() {
  // get the fileID from the event store
  std::string event_fname("");

  // get ODIN
  LHCb::ODIN* odin;
  if( exist<LHCb::ODIN>(LHCb::ODINLocation::Default) ){
    odin = get<LHCb::ODIN> (LHCb::ODINLocation::Default);
  }else{
    // should remain flagged as an error and stop the run
    error() << "ODIN cannot be loaded" << endmsg;
    return event_fname;
  }
  // obtain the run number from ODIN
  unsigned int run = odin->runNumber();  
  if ( msgLevel(MSG::VERBOSE) ) verbose() << "ODIN RunNumber: " << run << endmsg;

  // registry from raw data - only correct if file catalogue used 
  IOpaqueAddress* eAddr = 0;
  if( !exist<LHCb::RawEvent>(m_rawEventLocation) ){
    if ( msgLevel(MSG::VERBOSE) ) verbose() << m_rawEventLocation << " not found" << endmsg ;
    // then try from other bank
    if ( !exist<DataObject>("/Event") ){
      if ( msgLevel(MSG::VERBOSE) ) verbose() << "/Event" << " not found" << endmsg ;
      eAddr = odin->registry()->address();
    } else {
      // get the container
      DataObject* event = get<DataObject>("/Event");
      eAddr = event->registry()->address();
      }
  } else {
    LHCb::RawEvent* event = get<LHCb::RawEvent>(m_rawEventLocation);
    eAddr = event->registry()->address();
  }

  // obtain the fileID
  if ( eAddr ) {
    event_fname = eAddr->par()[0];
    if ( msgLevel(MSG::VERBOSE) ) verbose() << "RunInfo record from Event: " << event_fname << endmsg;
  } else {
    error() << "Registry cannot be loaded from Event" << endmsg;
    return event_fname;
  }
  if ( msgLevel(MSG::VERBOSE) ) verbose() << "ODIN RunNumber: " << run 
					  << " with RunInfo record: " << event_fname << endmsg;

  return event_fname;
}

//=============================================================================
std::vector< std::string > DumpFSR::navigate(std::string rootname, std::string tag) {
  // navigate recursively through the FileRecord store and report addresses which contain the tag
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Explore: " << rootname << " for " << tag << endmsg;
  std::vector< std::string > addresses;
  SmartDataPtr<DataObject>   root(m_fileRecordSvc, rootname);
  if ( root ) {
    explore(root->registry(), tag, addresses);
  }
  return addresses;
}

//=============================================================================
void DumpFSR::explore(IRegistry* pObj, std::string tag, std::vector< std::string >& addresses) {
  // add the addresses which contain the tag to the list and search through the leaves
  if ( 0 != pObj )    {
    std::string name = pObj->name();
    std::string::size_type f = name.find(tag);
    std::string id = pObj->identifier();

    // add this address to the list
    if ( f != std::string::npos ) addresses.push_back(id);

    // search through the leaves
    SmartIF<IDataManagerSvc> mgr(m_fileRecordSvc);
    if ( mgr )    {
      typedef std::vector<IRegistry*> Leaves;
      Leaves leaves;
      StatusCode sc = mgr->objectLeaves(pObj, leaves);
      if ( sc.isSuccess() )  {
        for ( Leaves::const_iterator iLeaf=leaves.begin(); iLeaf != leaves.end(); iLeaf++ )   {
	  // it is important to redefine leafRoot->registry() way back from the identifier 
	  std::string leafId = (*iLeaf)->identifier();
	  SmartDataPtr<DataObject> leafRoot(m_fileRecordSvc, leafId);
	  explore(leafRoot->registry(), tag, addresses);
        }
      }
    }
  }
}
