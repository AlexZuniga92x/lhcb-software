// $Id: PatVeloTT.cpp,v 1.7 2009-03-11 18:09:59 smenzeme Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

#include "PatVeloTTTool.h"

// local
#include "PatVeloTT.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PatVeloTT
//
// 2007-05-08 : Mariusz Witek
//-----------------------------------------------------------------------------


// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( PatVeloTT );
  

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PatVeloTT::PatVeloTT( const std::string& name,
                      ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator )
{
  if ( "Hlt" == context() ) {
    m_inputTracksName =  LHCb::TrackLocation::HltVelo;
    m_outputTracksName = LHCb::TrackLocation::HltVeloTT;
  }
  else {
    m_inputTracksName =  LHCb::TrackLocation::Velo;
    m_outputTracksName = LHCb::TrackLocation::VeloTT;
  } 

  declareProperty("InputTracksName"    , m_inputTracksName);
  declareProperty("OutputTracksName"   , m_outputTracksName);
  declareProperty("removeUsedTracks"   , m_removeUsedTracks = true); 
  declareProperty("InputUsedTracksNames"    , m_inputUsedTracksNames);
  declareProperty("fitTracks"          , m_fitTracks = true);
  declareProperty("Fitter"             , m_fitterName = "TrackMasterFitter" );
  declareProperty("maxChi2"            , m_maxChi2          = 5.); 
  declareProperty("TrackSelectorName"  , m_trackSelectorName = "None");
}
//=============================================================================
// Destructor
//=============================================================================
PatVeloTT::~PatVeloTT() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode PatVeloTT::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if(msgLevel(MSG::DEBUG)) debug() << "==> Initialize" << endmsg;

  m_veloTTTool = tool<PatVeloTTTool>("PatVeloTTTool", "PatVeloTTTool");
  
  m_trackSelector = NULL;
  if (m_trackSelectorName != "None")
    m_trackSelector = tool<ITrackSelector>( m_trackSelectorName, this);

  m_tracksFitter = tool<ITrackFitter>( m_fitterName, "Fitter", this );

  m_ttHitManager   = tool<Tf::TTStationHitManager <PatTTHit> >("PatTTStationHitManager");
  
  info() << " InputTracksName    = " << m_inputTracksName            << endmsg;
  info() << " OutputTracksName   = " << m_outputTracksName           << endmsg;
         
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode PatVeloTT::execute() {

  
  
  m_ttHitManager->prepareHits();



  LHCb::Tracks* outputTracks  = new LHCb::Tracks();
  outputTracks->reserve(200);
  put(outputTracks, m_outputTracksName);


  LHCb::Tracks* inputTracks   = get<LHCb::Tracks>( m_inputTracksName ); 


  std::vector<LHCb::Track*> veloTracks;

  // collect tracks pointers in local vector
  LHCb::Tracks::const_iterator pItr;
  for(pItr = inputTracks->begin(); inputTracks->end() != pItr; ++pItr){
    LHCb::Track* veloTr = (*pItr);
    if (!acceptTrack(*veloTr)) continue;
    veloTracks.push_back(veloTr);
  }

  if ( m_removeUsedTracks ) {
    removeUsedTracks( veloTracks);
  }
  
  // reconstruct tracks in TT
  std::vector<LHCb::Track*>::iterator itv;  
  std::vector<LHCb::Track*>::iterator itvtmp;  

  std::vector<LHCb::Track*> tmptracks;
  std::vector<LHCb::Track*> fittracks;
  
  for(itv = veloTracks.begin(); itv != veloTracks.end(); ++itv){

    LHCb::Track* velotr = *itv;
 
    m_veloTTTool->recoVeloTT(*velotr, tmptracks);

    for (itvtmp = tmptracks.begin(); itvtmp != tmptracks.end(); ++itvtmp ) {

      LHCb::Track* ptr = (*itvtmp);
      StatusCode sc = StatusCode::SUCCESS;   
      if (m_fitTracks) {  
        sc = m_tracksFitter -> fit( *ptr );   
      }
      
      if ( sc == StatusCode::SUCCESS ) {
        fittracks.push_back(ptr);
      }
      else
	delete ptr;

    }

    // choose best track
    LHCb::Track* bestTrack = 0;
    
    std::vector<LHCb::Track*>::iterator tracks;
    if(fittracks.size() > 0) {
      std::sort(fittracks.begin(),fittracks.end(),compChi2());
      bestTrack = *(fittracks.begin());
      tracks = fittracks.begin();
       
      if(bestTrack) {
        if(bestTrack->chi2PerDoF() < m_maxChi2) {
          outputTracks->insert(bestTrack);
          tracks++;
        }
      }
      
      for (; tracks != fittracks.end(); tracks++){
        delete (*tracks);
      }
    }
    tmptracks.clear();
    fittracks.clear();
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode PatVeloTT::finalize() {

  if(msgLevel(MSG::DEBUG)) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
bool PatVeloTT::acceptTrack(const LHCb::Track& track) 
{
  bool ok = !(track.checkFlag( LHCb::Track::Invalid) );
  ok = ok && (!(track.checkFlag( LHCb::Track::Backward) ));
  if (m_trackSelector) 
    ok = ok && (m_trackSelector->accept(track));
  
  if(msgLevel(MSG::DEBUG)) debug() << " accept track " << ok << endmsg;
  return ok;
}

//=========================================================================
// Remove Velo tracks that has been used by other algorithms
//=========================================================================
void PatVeloTT::removeUsedTracks( std::vector<LHCb::Track*>& veloTracks){

  // collect tracks from indicated containers

  std::vector< LHCb::Track*> usedTracks;

  std::vector< std::string >:: iterator itTrName;
  for (itTrName = m_inputUsedTracksNames.begin(); 
       itTrName != m_inputUsedTracksNames.end(); itTrName++) {

    // Retrieving tracks
    LHCb::Tracks* stracks = get<LHCb::Tracks>(*itTrName);
    if(!stracks) {
      warning() << "Tracks not found at location: " << *itTrName << endmsg; 
      continue;
    }

    for (LHCb::Tracks::iterator istrack = stracks->begin(); 
         stracks->end() != istrack; istrack++) {
  
      LHCb::Track* str = (*istrack);
      usedTracks.push_back(str);
    }
  }

  if(msgLevel(MSG::DEBUG)) debug() << " # used tracks to check: " << usedTracks.size() << endmsg;

  if( 0 == usedTracks.size() ) return; 

  std::vector< LHCb::Track*> tmpTracks;

  std::vector<LHCb::Track*>::iterator itv;  
  for(itv = veloTracks.begin(); itv != veloTracks.end(); ++itv){

    LHCb::Track* velocand = *itv;
    bool found = false;
    std::vector< LHCb::Track*>::iterator itTrack;
    for (itTrack = usedTracks.begin(); itTrack != usedTracks.end(); itTrack++) {
      LHCb::Track* tused = *itTrack;      
      if(matchingTracks(velocand, tused)) {
        found = true;
        break;
      }
    }
    if(!found)  tmpTracks.push_back(velocand);  
  }
  if(msgLevel(MSG::DEBUG)) debug() << " # used tracks found: " << veloTracks.size() - tmpTracks.size() << endmsg;

  veloTracks = tmpTracks;

}


//=========================================================================
// Check if two tracks have same VElo hits
//=========================================================================
bool PatVeloTT::matchingTracks( LHCb::Track* vttcand, LHCb::Track* trackused) 
{
  

  const std::vector<LHCb::LHCbID>& ids1 = vttcand->lhcbIDs();
  const std::vector<LHCb::LHCbID>& ids2 = trackused->lhcbIDs();

  bool match = false;

  unsigned int nCommon = 0;
  
  int nvelo1 = 0;
  int nvelo2 = 0;
  for ( unsigned int i1 = 0; i1 < ids1.size(); ++i1 ) {
    if(ids1[i1].checkDetectorType(LHCb::LHCbID::Velo)) nvelo1++;
  }
  for ( unsigned int i2 = 0; i2 < ids2.size(); ++i2 ) {
    if(ids2[i2].checkDetectorType(LHCb::LHCbID::Velo)) nvelo2++;
  }

  // Calculate the number of common LHCbIDs
  for ( unsigned int i1 = 0; i1 < ids1.size(); ++i1 ) {
    if(!ids1[i1].checkDetectorType(LHCb::LHCbID::Velo)) continue;
    for ( unsigned int i2 = 0; i2 < ids2.size(); ++i2 ) {
      if(!ids2[i2].checkDetectorType(LHCb::LHCbID::Velo)) continue;

      if ( ids1[i1].channelID() == ids2[i2].channelID() ) {
        ++nCommon;
        break;
      }

    }
  }  

  double rat1 = 0.;
  double rat2 = 0.;
  if(nvelo1) rat1 = 1.0*nCommon/nvelo1;
  if(nvelo2) rat2 = 1.0*nCommon/nvelo2;

  if(rat1>0.9 && rat2>0.9) {
    match = true;
  }

  return match;
}



//=============================================================================
