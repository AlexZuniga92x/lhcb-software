// Gaudi
#include "GaudiKernel/AlgFactory.h" 
// Event/MCEvent
#include "Event/MCHit.h"
#include "Event/MCTrackInfo.h"
// Event/TrackEvent
#include "Event/Track.h"
#include "Event/StateParameters.h"
// Event/LinkerEvent
#include "Linker/LinkedTo.h"
#include "Linker/LinkedFrom.h"
// Det/VPDet
#include "VPDet/DeVP.h"

// Local
#include "PrFitPolinomial.h"
#include "PrCheatedVP.h"

using namespace LHCb;
//-----------------------------------------------------------------------------
// Implementation file for class : PrCheatedVP
//
// 2012-07-26 : Olivier Callot
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( PrCheatedVP )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PrCheatedVP::PrCheatedVP( const std::string& name,
                              ISvcLocator* pSvcLocator)
  : GaudiAlgorithm (name, pSvcLocator) {
  declareProperty("useMCHits", m_useMCHits = true);
  declareProperty("binaryClusters", m_binaryClusters = false);
}
//=============================================================================
// Destructor
//=============================================================================
PrCheatedVP::~PrCheatedVP() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode PrCheatedVP::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;
  m_vp = getDet<DeVP>( DeVPLocation::Default );

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode PrCheatedVP::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;
  LHCb::Tracks* vTracks = new LHCb::Tracks();
  put( vTracks, LHCb::TrackLocation::Velo );
  
  const LHCb::MCParticles* partCont = get<LHCb::MCParticles>(LHCb::MCParticleLocation::Default);
  LinkedFrom<ContainedObject,LHCb::MCParticle> vLink( evtSvc(), msgSvc(), LHCb::VPClusterLocation::Default);
  LinkedTo<LHCb::MCHit, LHCb::VPCluster> links = LinkedTo<LHCb::MCHit, LHCb::VPCluster>(evtSvc(), msgSvc(), LHCb::VPClusterLocation::Default + "2MCHits" );
  LinkedFrom<LHCb::VPCluster, MCParticle> link(evtSvc(), msgSvc(), LHCb::VPClusterLocation::Default);

  MCTrackInfo trackInfo( evtSvc(), msgSvc() );
  LHCb::MCHits* vHits = get<LHCb::MCHits>( LHCb::MCHitLocation::VP );
   
  LHCb::MCParticles::const_iterator itP = partCont->begin();
  for (; itP != partCont->end(); ++itP){
    LHCb::MCParticle* part = *itP;
    if ( 0 == trackInfo.fullInfo( part ) ) continue;
    if ( !trackInfo.hasVelo( part ) ) continue;
//    if (abs(part->particleID().pid()) == 11) continue; //cut out electrons
    std::vector<Gaudi::XYZPoint> vPoints;  
    LHCb::Track* vTr = new LHCb::Track;
    
    if(m_useMCHits){
      std::vector<int> range = vLink.keyRange( part );
      for ( std::vector<int>::iterator itI = range.begin(); range.end() != itI; ++itI ) {
        LHCb::VPChannelID vId( *itI );
        vTr->addToLhcbIDs( LHCb::LHCbID( vId ) );
      }
      for ( LHCb::MCHits::const_iterator vHitIt = vHits->begin() ;
           vHits->end() != vHitIt ; vHitIt++ ) {
        if ( (*vHitIt)->mcParticle() ==  part ) {
          vPoints.push_back( (*vHitIt)->midPoint() );
        }
      }
    }
    else{
      LHCb::VPCluster* cluster = link.first(part);
      while (cluster) {
        LHCb::LHCbID id = LHCb::LHCbID(cluster->channelID());
        vTr->addToLhcbIDs( id );
        Gaudi::XYZPoint clusterPoint;
        if(m_binaryClusters) clusterPoint = getBinaryXYZ(cluster);
        else clusterPoint = getXYZ(cluster);
        vPoints.push_back( clusterPoint );
        cluster = link.next();
      }
    }
    
    double m_zVelo = 0.;
    double dz = 0.;
    PrFitPolinomial fitX( 1 );
    PrFitPolinomial fitY( 1 );
    for ( std::vector<Gaudi::XYZPoint>::iterator pt = vPoints.begin() ; 
         vPoints.end() != pt  ; pt++ ) {
      dz = (*pt).z() - m_zVelo;
      fitX.fill(  (*pt).x(), dz );
      fitY.fill(  (*pt).y(), dz );
    }
    fitX.solve();
    fitY.solve();
    
    LHCb::State vState;
    vState.setLocation( LHCb::State::ClosestToBeam );
    vState.setState( fitX.param(0), fitY.param(0), m_zVelo, fitX.param(1), fitY.param(1), 0. );
    vTr->addToStates( vState );
    if ( 0 > part->momentum().z() ) vTr->setFlag( LHCb::Track::Backward, true );
//    if( 0 > part->momentum().z() ) continue; //option to cut out backwards tracks
    vTr->setType( LHCb::Track::Velo );
    vTracks->insert( vTr );
  }
  
  /* //Run patter recognition with MCHits and their XYZ locations
  LHCb::MCParticles::const_iterator itP = partCont->begin();
  for (; itP != partCont->end(); ++itP){
    LHCb::MCParticle* part = *itP;
    if ( 0 == trackInfo.fullInfo( part ) ) continue;
    if ( !trackInfo.hasVelo( part ) ) continue;
    std::vector<int> range = vLink.keyRange( part );
    LHCb::Track* vTr = new LHCb::Track;
    for ( std::vector<int>::iterator itI = range.begin(); range.end() != itI; ++itI ) {
      LHCb::VPChannelID vId( *itI );
      vTr->addToLhcbIDs( LHCb::LHCbID( vId ) );
    }
    //== build a state from MCHits
    std::vector<Gaudi::XYZPoint> vPoints;  

//    for ( iClus = Clusters->begin(); Clusters->end() != iClus; ++iClus ) {
//      LHCb::MCHit* mchit = links.first((*iClus)->channelID());
//      if(!mchit) continue;
//      if(mchit->mcParticle() == part){
//        Gaudi::XYZPoint clusterPoint;
//        getXYZ((*iClus), clusterPoint);
////        vPoints.push_back( clusterPoint );
//        vPoints.push_back( mchit->midPoint() );
//      }
//    }
    
    for ( LHCb::MCHits::const_iterator vHitIt = vHits->begin() ;
          vHits->end() != vHitIt ; vHitIt++ ) {
      if ( (*vHitIt)->mcParticle() ==  part ) {
        vPoints.push_back( (*vHitIt)->midPoint() );
       }
    }
    
    double m_zVelo = 0.;
    double dz = 0.;
    PrFitPolinomial fitX( 1 );
    PrFitPolinomial fitY( 1 );
    for ( std::vector<Gaudi::XYZPoint>::iterator pt = vPoints.begin() ; 
          vPoints.end() != pt  ; pt++ ) {
      dz = (*pt).z() - m_zVelo;
      fitX.fill(  (*pt).x(), dz );
      fitY.fill(  (*pt).y(), dz );
    }
    fitX.solve();
    fitY.solve();
    
    LHCb::State vState;
    vState.setLocation( LHCb::State::ClosestToBeam );
    vState.setState( fitX.param(0), fitY.param(0), m_zVelo, fitX.param(1), fitY.param(1), 0. );
    vTr->addToStates( vState );
    if ( 0 > part->momentum().z() ) vTr->setFlag( LHCb::Track::Backward, true );
    vTracks->insert( vTr );
  }
  //*/
  return StatusCode::SUCCESS;
}

//===========================================================================
//   Get XYZ point for VPCluster associated MCHit
//===========================================================================
Gaudi::XYZPoint PrCheatedVP::getXYZ(LHCb::VPCluster* cluster) {
  
  const DeVPSensor* vpSensor = m_vp->sensorOfChannel(cluster->channelID());
  Gaudi::XYZPoint pointGlobal = vpSensor->channelToPoint(cluster->channelID(), cluster->lCluster().interPixelFraction());
  return pointGlobal;
  
}

//===========================================================================
//   Get binary XYZ point for VPCluster associated MCHit
//===========================================================================
Gaudi::XYZPoint PrCheatedVP::getBinaryXYZ(LHCb::VPCluster* cluster) {
  
  const DeVPSensor* vpSensor = m_vp->sensorOfChannel(cluster->channelID());
  Gaudi::XYZPoint pointGlobal = vpSensor->channelToPoint(cluster->channelID(), false);
  return pointGlobal;
  
}

//=============================================================================
