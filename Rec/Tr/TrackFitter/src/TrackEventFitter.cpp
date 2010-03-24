// $Id: TrackEventFitter.cpp,v 1.24 2010-03-24 12:50:15 rlambert Exp $
// Include files
// -------------
// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

// from TrackEvent
#include "Event/Track.h"

// local
#include "TrackEventFitter.h"

using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : TrackEventFitter
//
// 2005-05-30 : Eduardo Rodrigues
//-----------------------------------------------------------------------------

DECLARE_ALGORITHM_FACTORY( TrackEventFitter );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TrackEventFitter::TrackEventFitter( const std::string& name,
                                    ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
  , m_tracksFitter("TrackMasterFitter/Fitter",this)
  , m_makeNewContainer(true)
{
  declareProperty( "TracksInContainer", 
                   m_tracksInContainer  = TrackLocation::Default );
  declareProperty( "TracksOutContainer", 
                   m_tracksOutContainer = ""                  );
  declareProperty( "Fitter", m_tracksFitter) ;
}

//=============================================================================
// Destructor
//=============================================================================
TrackEventFitter::~TrackEventFitter() {}; 

//=============================================================================
// Initialization
//=============================================================================
StatusCode TrackEventFitter::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  m_tracksFitter.retrieve().ignore() ;
  // = tool<ITrackFitter>( m_fitterName, "Fitter", this );
  
  if ( m_tracksOutContainer == "" ) {
    m_tracksOutContainer = m_tracksInContainer;
    m_makeNewContainer   = false;
  }

  // Print out the user-defined settings
  // -----------------------------------
  info()
    << " " << endmsg
    << "=========== " << name() << " Settings ============"
    << endmsg
    << "  Tracks input container   : " << m_tracksInContainer << endmsg
    << "  Tracks output container  : " << m_tracksOutContainer << endmsg
    << "  Fitter name              : " << m_tracksFitter->name() << endmsg
    << "=================================================="
    << endmsg
    << " " << endmsg;

  return StatusCode::SUCCESS;
};

//=============================================================================
// Main execution
//=============================================================================
StatusCode TrackEventFitter::execute() {

  debug() << "==> Execute" << endmsg;

  // Retrieve the Tracks container
  // -----------------------------
  Tracks* tracksCont = get<Tracks>( m_tracksInContainer );

  // Make container for tracks
  // -------------------------
  Tracks* tracksNewCont = 0;
  if ( m_makeNewContainer ) tracksNewCont = new Tracks();

  // Loop over the tracks and fit them
  // ---------------------------------
  unsigned int nFitFail = 0;
  Tracks::const_iterator iTrack = tracksCont -> begin();
  for ( ; iTrack != tracksCont->end(); ++iTrack ) {

    // If needed make a new track keeping the same key
    Track& track = ( m_makeNewContainer ) ?
      *( (*iTrack) -> cloneWithKey() ) :  *(*iTrack);

    if ( msgLevel( MSG::DEBUG ) ) {
      debug() << "#### Fitting Track # " << track.key() << " ####" << endmsg
              << "  # of states before fit:" << track.nStates() << endmsg ;
      if(  track.nStates()>0 ) {
	if( msgLevel( MSG::VERBOSE ) ) {
	  verbose() << "  States are: " << endreq;
	  const std::vector<State*>& allstates = track.states();
	  for ( unsigned int it = 0; it < allstates.size(); it++ ) {
	    verbose() << "  - z = " << allstates[it]->z() << endreq
		      << "  - stateVector = "
		      << allstates[it]->stateVector() << endreq
		      << "  - covariance = " << endreq
		      << allstates[it]->covariance() << endreq;
	  }
	} else {
	  debug() << "  First state vector = " << track.firstState().stateVector() 
		  << " at z = " << track.firstState().z() << endmsg ;
	}
      }
    }

    StatusCode sc = m_tracksFitter -> fit( track );

    if ( sc.isSuccess() ) {
      // Add the track to the new Tracks container
      // -----------------------------------------
      if ( m_makeNewContainer ) tracksNewCont -> add( &track );
      if ( msgLevel( MSG::DEBUG ) ) {
        debug() << "Fitted successfully track # " << track.key() << endmsg;
      }
      // Update counters
      if( track.nDoF()>0) {
	double chisqprob = track.probChi2() ;
	counter("chisqprobSum") += chisqprob ;
	counter("badChisq") += bool(chisqprob<0.01) ;
      }
      counter("numOutliers") += track.nMeasurementsRemoved() ;
    }
    else {
      track.setFlag( Track::Invalid, true );
      ++nFitFail;
      if ( msgLevel( MSG::DEBUG ) )
        debug() << "Unable to fit the track # " << track.key() << endmsg;
    }
  } // loop over input Tracks

  // Update counters
  // ---------------
  unsigned int nTracks = tracksCont -> size();
  counter("nTracks") += nTracks;
  counter("nFitted") += ( nTracks - nFitFail );

  if ( msgLevel( MSG::DEBUG ) ) {
    if ( nFitFail == 0 )
      debug() << "All " << nTracks << " tracks fitted succesfully." << endmsg;
    else
      debug() << "Fitted successfully " << (nTracks-nFitFail)
              << " out of " << nTracks << endmsg;
  }

  // Store the Tracks in the TES
  // ---------------------------
  if ( m_makeNewContainer )
    put( tracksNewCont, m_tracksOutContainer );
  
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode TrackEventFitter::finalize() {

  debug() << "==> Finalize" << endmsg;

  float perf = 0.;
  double nTracks = counter("nTracks").flag();
  if ( nTracks > 1e-3 )
    perf = float(100.0*counter("nFitted").flag() / nTracks);

  info() << "  Fitting performance   : "
         << format( " %7.2f %%", perf ) << endmsg;

  m_tracksFitter.release().ignore() ;
  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
