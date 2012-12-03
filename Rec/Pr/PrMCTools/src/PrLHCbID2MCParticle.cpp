// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

#include "Event/MCParticle.h"

// includes from DigiEvent
#include "Event/VeloCluster.h"
#include "Event/VeloLiteCluster.h"
#include "Event/VLCluster.h"
#include "Event/VPCluster.h"
#include "Event/STCluster.h"
#include "Event/OTTime.h"
#include "Event/FTCluster.h"
#include "Event/FTRawCluster.h"

// local
#include "PrLHCbID2MCParticle.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PrLHCbID2MCParticle
//
// 2010-03-22 Victor Coco
//-----------------------------------------------------------------------------

DECLARE_ALGORITHM_FACTORY( PrLHCbID2MCParticle )


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
  PrLHCbID2MCParticle::PrLHCbID2MCParticle( const std::string& name,
                                              ISvcLocator* pSvcLocator)
    : GaudiAlgorithm ( name , pSvcLocator ),
      m_otHitCreator("Tf::OTHitCreator"),
      m_linker(NULL),
      m_detectorLink(NULL)
{
  declareProperty( "TargetName",  m_targetName = "Pr/LHCbID" );
}
//=============================================================================
// Destructor
//=============================================================================
PrLHCbID2MCParticle::~PrLHCbID2MCParticle() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode PrLHCbID2MCParticle::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  m_debug = msgLevel( MSG::DEBUG );
  if( m_debug ) debug() << "==> Initialize" << endmsg;
  m_otReady = false;
  m_stClusterNames.push_back( LHCb::STClusterLocation::TTClusters );
  m_stClusterNames.push_back( LHCb::STClusterLocation::ITClusters );
  m_stClusterNames.push_back( LHCb::STClusterLocation::UTClusters );
  m_stLiteClusterNames.push_back(  LHCb::STClusterLocation::TTClusters );
  m_stLiteClusterNames.push_back(  LHCb::STClusterLocation::ITClusters );
  m_stLiteClusterNames.push_back(  LHCb::STClusterLocation::UTClusters );

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode PrLHCbID2MCParticle::execute() {

  if (m_debug) debug() << "==> Execute" << endmsg;

  LinkerWithKey<LHCb::MCParticle> lhcbLink( evtSvc(), msgSvc(), m_targetName );
  m_linker = &lhcbLink;

  //== Velo

  if ( exist<LHCb::VeloClusters>(LHCb::VeloClusterLocation::Default) ) {    
    LinkedTo<LHCb::MCParticle> veloLink( evtSvc(), msgSvc(), LHCb::VeloClusterLocation::Default );
    m_detectorLink = &veloLink;    
    LHCb::VeloClusters* clusters = get<LHCb::VeloClusters>(LHCb::VeloClusterLocation::Default);
    LHCb::VeloClusters::const_iterator iClus;
    for(iClus = clusters->begin(); iClus != clusters->end(); ++iClus) {
      LHCb::LHCbID myId = LHCb::LHCbID( (*iClus)->channelID() );
      int id            = myId.veloID();
      int size          = (*iClus)->pseudoSize();
      linkAll( myId, id, size );
    }
  } else if ( exist<LHCb::VeloLiteCluster::VeloLiteClusters>(LHCb::VeloLiteClusterLocation::Default) ) {    
    LinkedTo<LHCb::MCParticle> veloLink( evtSvc(), msgSvc(), LHCb::VeloClusterLocation::Default );
    m_detectorLink = &veloLink;    
    LHCb::VeloLiteCluster::VeloLiteClusters* clusters = 
      get<LHCb::VeloLiteCluster::VeloLiteClusters>(LHCb::VeloLiteClusterLocation::Default);
    LHCb::VeloLiteCluster::VeloLiteClusters::const_iterator iClus;
    for(iClus = clusters->begin(); iClus != clusters->end(); ++iClus) {
      LHCb::LHCbID myId = LHCb::LHCbID( (*iClus).channelID() );
      int id            = myId.veloID();
      int size          = (*iClus).pseudoSize();
      linkAll( myId, id, size );
    }
  }

  //== Velo Light for upgrade

  if ( exist<LHCb::VLClusters>(LHCb::VLClusterLocation::Default) ) {    
    LinkedTo<LHCb::MCParticle> vlLink( evtSvc(), msgSvc(), LHCb::VLClusterLocation::Default );
    m_detectorLink = &vlLink;    
    LHCb::VLClusters* clusters = get<LHCb::VLClusters>(LHCb::VLClusterLocation::Default);
    LHCb::VLClusters::const_iterator iClus;
    for(iClus = clusters->begin(); iClus != clusters->end(); ++iClus) {
      LHCb::LHCbID myId = LHCb::LHCbID( (*iClus)->channelID() );
      int id            = myId.vlID();
      linkAll( myId, id );
    }
  }

  //== Velo Pixel

  if ( exist<LHCb::VPClusters>(LHCb::VPClusterLocation::VPClusterLocation ) ) {
    LinkedTo<LHCb::MCParticle> vPLink( evtSvc(), msgSvc(), LHCb::VPClusterLocation::VPClusterLocation );
    m_detectorLink = &vPLink;
    LHCb::VPClusters* clusters = get<LHCb::VPClusters>(LHCb::VPClusterLocation::VPClusterLocation );
    LHCb::VPClusters::const_iterator iClus;
    for(iClus = clusters->begin(); iClus != clusters->end(); ++iClus) {
      LHCb::LHCbID myId = (*iClus)->lCluster().channelID();
      int id            = myId.vpID();
      linkAll( myId, id );
    }
  }

  //== TT, IT, UT (All ST-like detectors)

  for ( unsigned int kk = 0; m_stClusterNames.size() > kk; ++kk ) {
    std::string clusterName = m_stClusterNames[kk];
    std::string liteName    = m_stLiteClusterNames[kk];
    if ( exist<LHCb::STCluster::Container>( clusterName ) ) {
      LinkedTo<LHCb::MCParticle> ttLink( evtSvc(), msgSvc(), clusterName );
      m_detectorLink = &ttLink;
      const LHCb::STCluster::Container* cont = get<LHCb::STCluster::Container>( clusterName );
      for(  LHCb::STCluster::Container::const_iterator iclus = cont->begin();
            iclus != cont->end(); ++iclus) {
        LHCb::LHCbID myId = LHCb::LHCbID((*iclus)->channelID());
        int size          = (*iclus)->size();
        int id            = myId.stID();
        linkAll( myId, id, size );
      }
    } else if ( exist<LHCb::STLiteCluster::STLiteClusters>( liteName ) ) {    
      LinkedTo<LHCb::MCParticle> ttLink( evtSvc(), msgSvc(), clusterName );
      m_detectorLink = &ttLink;    
      LHCb::STLiteCluster::STLiteClusters* clusters = get<LHCb::STLiteCluster::STLiteClusters>( liteName );
      LHCb::STLiteCluster::STLiteClusters::const_iterator iClus;
      for(iClus = clusters->begin(); iClus != clusters->end(); ++iClus) {
        LHCb::LHCbID myId = LHCb::LHCbID( (*iClus).channelID() );
        int id            = myId.stID();
        int size          = (*iClus).pseudoSize();
        linkAll( myId, id, size );
      }
    }
  }


  //== OT coordinates
  if ( exist<LHCb::LinksByKey>( "Link/" + LHCb::OTTimeLocation::Default ) ) {
    LinkedTo<LHCb::MCParticle> otLink( evtSvc(), msgSvc(),LHCb::OTTimeLocation::Default );
    if ( !otLink.notFound() ) {
      m_detectorLink = &otLink;
      if ( !m_otReady ) {
        m_otHitCreator.retrieve().ignore();
        m_otReady = true;
      }
      Tf::OTHitRange othits = m_otHitCreator->hits();
      for (Tf::OTHitRange::const_iterator otSTH = othits.begin();
           otSTH < othits.end();otSTH++){
        LHCb::LHCbID myId = (*otSTH)->lhcbID();
        int id            = myId.otID();
        linkAll( myId, id );
      }
    }
  }
  
  //== FT
  if ( exist<FastClusterContainer<LHCb::FTRawCluster,int> >(LHCb::FTRawClusterLocation::Default) ) {
    LinkedTo<LHCb::MCParticle> ftLink( evtSvc(), msgSvc(),LHCb::FTClusterLocation::Default );
    if ( !ftLink.notFound() ) {
      m_detectorLink = &ftLink;
      typedef FastClusterContainer<LHCb::FTRawCluster,int> FTRawClusters;
      FTRawClusters* clus = get<FTRawClusters>( LHCb::FTRawClusterLocation::Default );
      FTRawClusters::const_iterator iClus;
      for(iClus = clus->begin(); iClus != clus->end(); ++iClus) {
        LHCb::LHCbID myId = LHCb::LHCbID( (*iClus).channelID() );
        int id            = myId.ftID();
        linkAll( myId, id );
      }
    }
  }
  return StatusCode::SUCCESS;
}

//=========================================================================
//  link all particles to the specified id
//=========================================================================
void PrLHCbID2MCParticle::linkAll( LHCb::LHCbID myId, int firstId, int size ) {
  m_partList.clear();
  while( size > 0 ) {
    const LHCb::MCParticle* part = m_detectorLink->first( firstId );
    while ( 0 != part ) {       
      if ( std::find( m_partList.begin(), m_partList.end(), part ) == m_partList.end() ) {
        m_partList.push_back( part );
      }
      part = m_detectorLink->next();
    }
    ++firstId;
    --size;
  }
  if ( m_debug ) info() << format( "For ID %8x  MC: ", myId.lhcbID() );
  for ( std::vector<const LHCb::MCParticle*>::const_iterator itP = m_partList.begin(); 
        m_partList.end() != itP; ++itP ) {
    if ( m_debug ) info() << (*itP)->key() << " ";
    m_linker->link( myId.lhcbID(), *itP );
  }
  if ( m_debug ) info() << endmsg;
}
//=============================================================================
