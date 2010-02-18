// $Id: PatVeloSpaceTracking.cpp,v 1.12 2010-02-18 14:12:07 dhcroft Exp $
// Include files
// from Gaudi
#include "GaudiKernel/AlgFactory.h"
// local
#include "PatVeloSpaceTracking.h"
#include "PatVeloPhiList.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PatVeloSpaceTracking
//
// 2005-06-09 : Olivier Callot
//-----------------------------------------------------------------------------

namespace Tf {
  DECLARE_ALGORITHM_FACTORY( PatVeloSpaceTracking );

  //=============================================================================
  // Standard constructor, initializes variables
  //=============================================================================
  PatVeloSpaceTracking::PatVeloSpaceTracking( const std::string& name,
      ISvcLocator* pSvcLocator)
    : GaudiAlgorithm ( name , pSvcLocator )
    {
      declareProperty( "InputTracksName",
          m_inputTracksLocation = LHCb::TrackLocation::RZVelo );
      declareProperty( "OutputTracksName",
          m_outputTracksLocation = LHCb::TrackLocation::Velo );
      declareProperty( "SpaceToolName",
          m_spaceToolName = "Tf::PatVeloSpaceTool/PatVeloSpaceTool" );

      declareProperty("TrackSelectorName",
          m_trackSelectorName = "None");
    }

  //=============================================================================
  // Destructor
  //=============================================================================
  PatVeloSpaceTracking::~PatVeloSpaceTracking() {};

  //=============================================================================
  // Initialization
  //=============================================================================
  StatusCode PatVeloSpaceTracking::initialize() {
    StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
    if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

    debug() << "==> Initialize" << endmsg;

    std::string outputName = m_outputTracksLocation;

    m_trackSelector = NULL;
    if (m_trackSelectorName != "None")
      m_trackSelector = tool<ITrackSelector> ( m_trackSelectorName, this);

    m_PatVeloSpaceTool = 
      tool<ITracksFromTrack>( m_spaceToolName,this);

    return StatusCode::SUCCESS;
  };

  //=============================================================================
  // Main execution
  //=============================================================================

  bool PatVeloSpaceTracking::accept(const LHCb::Track& track) {
    // JAH: method to accept this track if there is a track selector tool
    if (!m_trackSelector) return true;
    bool ok =  m_trackSelector->accept(track);
    if( msgLevel( MSG::VERBOSE ) ) verbose() << " accepting track "
      << track.key()<< " "
        << ok << endreq;
    return ok;
  }

  StatusCode PatVeloSpaceTracking::execute() {

    bool isDebug   = msgLevel( MSG::DEBUG   );
    if(isDebug)  debug() << "==> Execute" << endmsg;

    //=== Build SpaceTrack from the Tracks, only for those to follow.

    // find input track container on TES,
    //   // bail out if it does not exist
    if ( exist<LHCb::Tracks>( m_inputTracksLocation ) ) {
      m_inputTracks = get<LHCb::Tracks>( m_inputTracksLocation );
    } else {
      error() << "Input track container not found at "
        << m_inputTracksLocation << "." << endmsg;
      return StatusCode::SUCCESS;
    }

    // create output track container on TES, if it does not exist
    if ( exist<LHCb::Tracks>( m_outputTracksLocation ) ) {
      m_outputTracks = get<LHCb::Tracks>( m_outputTracksLocation );
    } else {
      m_outputTracks = new LHCb::Tracks();
      put(m_outputTracks, m_outputTracksLocation );
    }
    m_outputTracks->reserve(500);

    if( msgLevel( MSG::VERBOSE ) ){
      info() <<"Retrieved " << m_inputTracks->size()
        << " tracks from " << m_inputTracksLocation << endreq;
    }

    std::vector<LHCb::Track*> output;

    // copy m_inputTracks container into tracks
    // [NOTE copy of the vector of pointers not the underlying tracks]
    std::vector<LHCb::Track*> tracks(m_inputTracks->size());
    copy(m_inputTracks->begin(),m_inputTracks->end(),tracks.begin());

    std::stable_sort( tracks.begin(), tracks.end(), greater_trackLength() );

    for ( std::vector<LHCb::Track*>::const_iterator itT = tracks.begin();
        tracks.end() != itT; ++itT ) {
      LHCb::Track& track = **itT;

      if( track.checkFlag(LHCb::Track::Invalid) ) {
        Warning("Invalid RZ Velo track as input: skipping conversion to 3D").ignore();
        continue;
      }

      // JAH: accept this track
      if (!accept(track)) continue;

      StatusCode sc = m_PatVeloSpaceTool->tracksFromTrack(track, output);
      if (!sc) {
        std::vector<LHCb::Track*>::iterator iOut;
        for( iOut = output.begin() ; iOut != output.end() ; ++iOut ){
          delete *iOut; // remove track
        }
        return sc;
      }
    }

    std::vector<LHCb::Track*>::iterator iOut;
    for( iOut = output.begin() ; iOut != output.end() ; ++iOut ){
      m_outputTracks->insert(*iOut);
    }

    if ( isDebug ) debug() << "Found " <<output.size()
        << " new 3D VELO tracks, in total "
        << m_outputTracks->size() << " in "
        << m_outputTracksLocation
        << endreq;

    return StatusCode::SUCCESS;

  }


  //=============================================================================
  //  Finalize
  //=============================================================================
  StatusCode PatVeloSpaceTracking::finalize() {

    debug() << "==> Finalize" << endmsg;

    return GaudiAlgorithm::finalize();  // must be called after all other actions
  }

}
