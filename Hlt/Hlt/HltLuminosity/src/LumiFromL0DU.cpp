// $Id: LumiFromL0DU.cpp,v 1.4 2008-08-25 10:59:04 panmanj Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
#include "GaudiKernel/IAlgManager.h"

#include "Event/HltLumiSummary.h"

#include "HltBase/ANNSvc.h"

// local
#include "LumiFromL0DU.h"

using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : LumiFromL0DU
//
// 2008-07-21 : Jaap Panman
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( LumiFromL0DU );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
LumiFromL0DU::LumiFromL0DU( const std::string& name,
                                  ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
  declareProperty( "InputSelection" ,      m_InputSelectionName  = "Trig/L0/L0DUReport");
  declareProperty( "CounterName"    ,      m_CounterName);
  declareProperty( "ValueName"      ,      m_ValueName);
  declareProperty( "OutputContainer",      m_OutputContainerName =  LHCb::HltLumiSummaryLocation::Default );

}
//=============================================================================
// Destructor
//=============================================================================
LumiFromL0DU::~LumiFromL0DU() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode LumiFromL0DU::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  info() << "InputSelection         " << m_InputSelectionName  << endmsg;
  info() << "CounterName            " << m_CounterName         << endmsg;
  info() << "ValueName              " << m_ValueName           << endmsg;
  info() << "OutputContainer        " << m_OutputContainerName << endmsg;


  // ------------------------------------------
  IANNSvc* annSvc = svc<IANNSvc>("LumiANNSvc");

  boost::optional<IANNSvc::minor_value_type> 
    x = annSvc->value("LumiCounters", m_CounterName);

  if (!x) {
    warning() << "LumiCounter not found with name: " << m_CounterName <<  endmsg;
  } else {
    m_Counter = x->second;
    info() << "ExtraInfo key value: " << m_Counter << endmsg;
  }
  // ------------------------------------------
 
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode LumiFromL0DU::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  // load the L0DU configuration
  int nCounter =  0;
  if ( !exist<LHCb::L0DUReport>(m_InputSelectionName) ){
    warning() << m_InputSelectionName << " not found" << endmsg ;
  } else {
    // get the container
    m_L0DUReport = get<LHCb::L0DUReport>(m_InputSelectionName);
    
    if ( !m_L0DUReport ) { 
      err() << "Could not find location " 
	    <<  m_InputSelectionName << endreq;
      return StatusCode::FAILURE ;
    }
    // get the value using its name from the L0Report
    double value = m_L0DUReport->dataValue(m_ValueName);
    debug() << "found value for " << m_ValueName << " " << value << endreq ;

    if ( msgLevel(MSG::DEBUG) ) {
      // for debugging, get also value in alternative way
      const LHCb::L0DUConfig* m_configuration = m_L0DUReport->configuration();
      if ( NULL == m_configuration ) { 
        debug() << "cannot find L0DUConfig" << endreq ;
      }
      else {
        LHCb::L0DUElementaryData::Map data = m_configuration->data();
        LHCb::L0DUElementaryData::Map::const_iterator it = data.find( m_ValueName );
        if( it == data.end() ) {
          debug() << "cannot find map element for " << m_ValueName << endreq ;
        }
        else {
          double other_value = ((*it).second)->value();
          debug() << "found other value for " << m_ValueName << " " << other_value << endreq ;
        }
      }
    }
    nCounter = (int) value;
  }

  // get container
  LHCb::HltLumiSummarys* sums = getOrCreate<HltLumiSummarys,HltLumiSummarys>(m_OutputContainerName);
  if (sums->empty()) sums->insert(new LHCb::HltLumiSummary() );
  // add track counter
  (*sums->begin())->addInfo( m_Counter, nCounter);

  setFilterPassed(true);

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode LumiFromL0DU::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================

