// $Id: HltPrepareMuonSeg.cpp,v 1.7 2008-07-30 13:42:04 graven Exp $
// Include files 

// from Gaudi

#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/SystemOfUnits.h"
// local
#include "HltPrepareMuonSeg.h"

//-----------------------------------------------------------------------------
// Implementation file for class : HltPrepareMuonSeg
//
// 2007-12-4 : Sandra Amato 
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( HltPrepareMuonSeg );

using namespace LHCb;

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
HltPrepareMuonSeg::HltPrepareMuonSeg( const std::string& name,
                                      ISvcLocator* pSvcLocator)
  : HltAlgorithm ( name , pSvcLocator )
  , m_selections(*this)
{
  declareProperty("OutputMuonTracksName"   ,
                  m_outputMuonTracksName = "Hlt/Tracks/ConfirmedTMuon");
  m_selections.declareProperties();

}
//=============================================================================
// Destructor
//=============================================================================
HltPrepareMuonSeg::~HltPrepareMuonSeg() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode HltPrepareMuonSeg::initialize() {
  StatusCode sc = HltAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm
  debug() << "==> Initialize" << endmsg;

  m_selections.retrieveSelections();
  m_selections.registerSelection();
  
  m_prepareMuonSeed = tool<IMuonSeedTool>("MuonSeedTool",this);

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode HltPrepareMuonSeg::execute() {
  
  debug() << "==> Execute" << endmsg;

  //Container with all T tracks
  Tracks* muontracks = new Tracks();
  put(muontracks,m_outputMuonTracksName);
  debug() << "Muon segments tracks size " << m_selections.input<1>()->size()<< endmsg;
  
   for ( std::vector<Track*>::const_iterator itT = m_selections.input<1>()->begin();
        m_selections.input<1>()->end() != itT; itT++ ) {
    Track* itMuonSeg = (*itT);
    Track* seedTrack = new Track();
    //    LHCb::Track seedTrack; 
    StatusCode sctmp = m_prepareMuonSeed->makeTrack( *itMuonSeg , *seedTrack );
    if( sctmp.isFailure() ){
      err()<<"Failed to prepare the seed"<<endmsg;
    }
    muontracks->insert(seedTrack); 
    m_selections.output()->push_back(seedTrack);
 }//for ( std::vector<Track*>::const_iterator itT = m_selections.input()->begin() 
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode HltPrepareMuonSeg::finalize() {
  
  debug() << "==> Finalize" << endmsg;
  
  return HltAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
