// $Id: PatMatch.cpp,v 1.9 2009-02-25 22:37:58 mschille Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

// local
#include "PatMatch.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PatMatch
//
// 2007-02-07 : Olivier Callot
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( PatMatch );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PatMatch::PatMatch( const std::string& name,
                    ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
  declareProperty( "VeloInput"       , m_veloLocation  = LHCb::TrackLocation::Velo );
  declareProperty( "SeedInput"       , m_seedLocation  = LHCb::TrackLocation::Seed );
  declareProperty( "MatchOutput"     , m_matchLocation = LHCb::TrackLocation::Match );
}
//=============================================================================
// Destructor
//=============================================================================
PatMatch::~PatMatch() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode PatMatch::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  m_matchTool = tool<IMatchTool>("PatMatchTool", this);
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode PatMatch::execute()
{
  LHCb::Tracks* matchs = new LHCb::Tracks;
  put(matchs, m_matchLocation);
  matchs->reserve(200);
 
  LHCb::Tracks* velos  = get<LHCb::Tracks>( m_veloLocation );
  LHCb::Tracks* seeds  = get<LHCb::Tracks>( m_seedLocation ); 
  
  StatusCode sc = m_matchTool->match( *velos , *seeds , *matchs);
  
  if(sc.isFailure()) Warning("PatMatchTool failed",sc).ignore();
  
  return sc;
}
 
//=============================================================================
//  Finalize
//=============================================================================
StatusCode PatMatch::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
