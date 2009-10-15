// $Id: HltSelectionsBase.cpp,v 1.1 2009-10-15 12:32:16 pkoppenb Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 
#include "Kernel/IWriteSelResult.h"
#include "Kernel/IANNSvc.h"

// local
#include "HltSelectionsBase.h" 
  
//-----------------------------------------------------------------------------
// Implementation file for class : HltSelectionsBase
//
// 2007-03-29 : Patrick Koppenburg
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( HltSelectionsBase );
//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
HltSelectionsBase::HltSelectionsBase( const std::string& name,
                                      ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
    , m_isTrigger(false)
{
  declareProperty( "Hlt2SelectionID" , m_hlt2SelectionID = "Hlt2SelectionID");
}
//=============================================================================
// Destructor
//=============================================================================
HltSelectionsBase::~HltSelectionsBase() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode HltSelectionsBase::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); 
  if ( sc.isFailure() ) return sc;
  if (msgLevel(MSG::DEBUG)) debug() << "HltSelectionsBase initialize" << endmsg ;

  hltPairs dummy = selections(); // initialise

  return StatusCode::SUCCESS;
}

//=========================================================================
//  fill selections map
//=========================================================================
const hltPairs& HltSelectionsBase::selections ( ) {

  if ( m_selections.empty()){
  
    m_selections = svc<IANNSvc>("HltANNSvc")->items(m_hlt2SelectionID);
    if ( m_selections.empty()) Exception("Empty selections!");
    if (msgLevel(MSG::INFO)) info() << "Selections" ;
    for ( hltPairs::const_iterator n = m_selections.begin() ; m_selections.end() != n ; n++){
      if (msgLevel(MSG::INFO)) info() << ", " << n->first << " : " << n->second ;
    }
    if (msgLevel(MSG::INFO)) info() << endmsg ;
  }
  
  return m_selections ;
}
//=============================================================================
// Main execution
//=============================================================================
StatusCode HltSelectionsBase::execute() {
  err() << "This is a base class. Do not invoke." << endmsg ;
  return StatusCode::FAILURE ;
}
//=============================================================================
//  Finalize
//=============================================================================
StatusCode HltSelectionsBase::finalize() {

  if (msgLevel(MSG::DEBUG)) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize(); 
}

