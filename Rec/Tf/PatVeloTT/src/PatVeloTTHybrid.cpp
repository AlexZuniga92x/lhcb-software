// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// local
#include "PatVeloTTHybrid.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PatVeloTTHybrid
//
// 2007-05-08 : Mariusz Witek
//-----------------------------------------------------------------------------


// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( PatVeloTTHybrid )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PatVeloTTHybrid::PatVeloTTHybrid( const std::string& name,
                      ISvcLocator* pSvcLocator)
: GaudiAlgorithm ( name , pSvcLocator ),
   m_veloTTTime(0)
{
  if ( "Hlt" == context() ) {
    m_inputTracksName =  "";
    m_outputTracksName = "";
  }
  else {
    m_inputTracksName =  LHCb::TrackLocation::Velo;
    m_outputTracksName = LHCb::TrackLocation::VeloTT;
  } 

  declareProperty("InputTracksName"    , m_inputTracksName);
  declareProperty("OutputTracksName"   , m_outputTracksName);
  declareProperty("removeUsedTracks"   , m_removeUsedTracks = false); 
  declareProperty("InputUsedTracksNames"    , m_inputUsedTracksNames);
  declareProperty("fitTracks"          , m_fitTracks = false);
  declareProperty("Fitter"             , m_fitterName = "TrackMasterFitter" );
  declareProperty("maxChi2"            , m_maxChi2          = 1280.); 
  declareProperty( "TimingMeasurement", m_doTiming = false);
    
}
//=============================================================================
// Destructor
//=============================================================================
PatVeloTTHybrid::~PatVeloTTHybrid() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode PatVeloTTHybrid::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  m_veloTTTool = tool<ITracksFromTrack>("PatVeloTTHybridTool", "PatVeloTTHybridTool");

  m_tracksFitter = tool<ITrackFitter>( m_fitterName, "Fitter", this );

  m_ttHitManager   = tool<Tf::TTStationHitManager <PatTTHit> >("PatTTStationHitManager");

  info() << " InputTracksName    = " << m_inputTracksName            << endmsg;
  info() << " OutputTracksName   = " << m_outputTracksName           << endmsg;

  if ( m_doTiming) {
    m_timerTool = tool<ISequencerTimerTool>( "SequencerTimerTool" );
    m_timerTool->increaseIndent();
    m_veloTTTime = m_timerTool->addTimer( "Internal VeloTTHybrid Tracking" );
    m_timerTool->decreaseIndent();
  }

         
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode PatVeloTTHybrid::execute() {

  if ( m_doTiming ) m_timerTool->start( m_veloTTTime );
  
  m_ttHitManager->prepareHits();

  LHCb::Tracks* outputTracks  = new LHCb::Tracks();
  outputTracks->reserve(200);
  put(outputTracks, m_outputTracksName);

  LHCb::Track::Range inputTracks   = get<LHCb::Track::Range>( m_inputTracksName ); 
  if( inputTracks.size() == 0 ){
    debug() << " Input Tracks container: " <<  m_inputTracksName << " is empty! Skipping" << endmsg;
    return StatusCode::SUCCESS;
  }
  
  std::vector<LHCb::Track*> tmpTracks;
  tmpTracks.reserve(5);
  
  counter("#seeds") += inputTracks.size();

  for(const LHCb::Track* veloTr: inputTracks){
    
    m_veloTTTool->tracksFromTrack(*veloTr, tmpTracks).ignore();
    
    LHCb::Track* bestTrack = nullptr;
    
    float maxChi2 = m_maxChi2;

    for ( LHCb::Track* fitTrack : tmpTracks ) {

      if (m_fitTracks) {  
        StatusCode sc = m_tracksFitter -> fit( *fitTrack );
        std::cout << "chi2 after KF " << fitTrack->chi2PerDoF() << std::endl;

        if( !sc ){
          warning() << "Could not fit track" << endmsg;
          delete fitTrack;
          continue;
        }
        
      }
      if( fitTrack->chi2PerDoF() < maxChi2){
        maxChi2 = fitTrack->chi2PerDoF();
        bestTrack = fitTrack;
      }
      
    }

    tmpTracks.clear();
    
    if( bestTrack != nullptr){
      outputTracks->insert(bestTrack);
    }
  }
  

  counter("#tracks") += outputTracks->size();
  if ( m_doTiming ) m_timerTool->stop( m_veloTTTime );
  
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode PatVeloTTHybrid::finalize() {

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}



//=========================================================================
// Remove Velo tracks that has been used by other algorithms
//=========================================================================
void PatVeloTTHybrid::removeUsedTracks( std::vector<LHCb::Track*>& veloTracks){

  // collect tracks from indicated containers

  std::vector< LHCb::Track*> usedTracks;

  std::vector< std::string >:: iterator itTrName;
  for (itTrName = m_inputUsedTracksNames.begin(); 
       itTrName != m_inputUsedTracksNames.end(); itTrName++) {

    // Retrieving tracks
    LHCb::Tracks* stracks = get<LHCb::Tracks>(*itTrName);
    if(!stracks) {
      Warning("Tracks not found at given location: ",StatusCode::SUCCESS).ignore(); 
      continue;
    }

    for (LHCb::Tracks::iterator istrack = stracks->begin(); 
         stracks->end() != istrack; istrack++) {
  
      LHCb::Track* str = (*istrack);
      usedTracks.push_back(str);
    }
  }

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

  veloTracks = tmpTracks;

}


//=========================================================================
// Check if two tracks have same VElo hits
//=========================================================================
bool PatVeloTTHybrid::matchingTracks( LHCb::Track* vttcand, LHCb::Track* trackused) 
{
  

  const std::vector<LHCb::LHCbID>& ids1 = vttcand->lhcbIDs();
  const std::vector<LHCb::LHCbID>& ids2 = trackused->lhcbIDs();

  bool match = false;

  unsigned int nCommon = 0;
  
  int nvelo1 = 0;
  int nvelo2 = 0;
  for ( unsigned int i1 = 0; i1 < ids1.size(); ++i1 ) {
    if(ids1[i1].checkDetectorType(LHCb::LHCbID::Velo) 
       || ids1[i1].checkDetectorType(LHCb::LHCbID::VP) ) nvelo1++;
  }
  for ( unsigned int i2 = 0; i2 < ids2.size(); ++i2 ) {
    if(ids2[i2].checkDetectorType(LHCb::LHCbID::Velo) 
       || ids2[i2].checkDetectorType(LHCb::LHCbID::VP) ) nvelo2++;
  }

  // Calculate the number of common LHCbIDs
  for ( unsigned int i1 = 0; i1 < ids1.size(); ++i1 ) {
    if(!ids1[i1].checkDetectorType(LHCb::LHCbID::Velo) 
       && !ids1[i1].checkDetectorType(LHCb::LHCbID::VP) ) continue;
    for ( unsigned int i2 = 0; i2 < ids2.size(); ++i2 ) {
      if(!ids2[i2].checkDetectorType(LHCb::LHCbID::Velo) 
         && !ids2[i2].checkDetectorType(LHCb::LHCbID::VP)) continue;

      if ( ids1[i1].channelID() == ids2[i2].channelID()  ) {
        ++nCommon;
        break;
      }

    }
  }  

  float rat1 = 0.;
  float rat2 = 0.;
  if(nvelo1) rat1 = 1.0*nCommon/nvelo1;
  if(nvelo2) rat2 = 1.0*nCommon/nvelo2;

  if(rat1>0.9 && rat2>0.9) {
    match = true;
  }

  return match;
}



//=============================================================================
