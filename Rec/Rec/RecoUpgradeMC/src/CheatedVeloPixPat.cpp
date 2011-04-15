// Include files

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
// Linker
#include "Linker/AllLinks.h"
#include "Event/MCTrackInfo.h"
#include "Event/Track.h"
// local
#include "CheatedVeloPixPat.h"

#include "Event/Track.h"
#include "Event/MCParticle.h"
#include "Linker/LinkedTo.h"
#include "LHCbMath/Line.h"
#include "LHCbMath/GeomFun.h"
#include "Event/MCHit.h"

#include "Event/VeloPixCluster.h"
#include "Event/VeloPixLiteMeasurement.h"
#include "VeloPixTrack.h"
#include "VeloPixHit.h"
//-----------------------------------------------------------------------------
// Implementation file for class : CheatedVeloPixPat
//
// 2010-02-25 Laurence Carson / Victor Coco
// 2011-03-16 ReWrite by Wenbin Qian to do a realistic cheated tracking
//-----------------------------------------------------------------------------

DECLARE_ALGORITHM_FACTORY( CheatedVeloPixPat );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
CheatedVeloPixPat::CheatedVeloPixPat( const std::string& name,
                                      ISvcLocator* pSvcLocator)
  : GaudiTupleAlg ( name , pSvcLocator ),
    m_veloPixFitter(0),
    m_veloPixFitterName("")
{
  declareProperty( "outputTracksLocation",    m_outputTracksLocation = LHCb::TrackLocation::VeloPix);
  declareProperty( "MinimalMCHitForTrack",    m_minIDs = 2 );
  declareProperty( "UseLinearFit",    m_UseLinearFit = true );
  declareProperty( "VeloPixFitterName",    m_veloPixFitterName =  "Tf::PatVeloPixFitLHCbIDs/FitVeloPix" );
  declareProperty("StateAtBeam",               m_stateAtBeam    = true );  
}
//=============================================================================
// Destructor
//=============================================================================
CheatedVeloPixPat::~CheatedVeloPixPat() {};

//=============================================================================
// Initialization
//=============================================================================
StatusCode CheatedVeloPixPat::initialize() {
  StatusCode sc = GaudiTupleAlg::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm
  debug() << "==> Initialize" << endmsg;
  // retrieve the postion tool
  m_positiontool =  tool<IVeloPixClusterPosition>("VeloPixClusterPosition");
  
  m_veloPix = getDet<DeVeloPix>( DeVeloPixLocation::Default );
  
  m_veloPixFitter = tool<ITrackFitter>(m_veloPixFitterName, this ) ;
  
  return StatusCode::SUCCESS;
};

//=============================================================================
// Main execution
//=============================================================================
StatusCode CheatedVeloPixPat::execute() {
  debug() << "==> Execute" << endmsg;
  
  verbose()<<"tag 0"<<endreq;
  m_clusters = get<LHCb::VeloPixLiteCluster::VeloPixLiteClusters>(LHCb::VeloPixLiteClusterLocation::Default );
  
  Tuple tuple = nTuple("mcinfo");  
  
  LHCb::MCParticles* mcParts =
    get<LHCb::MCParticles>( LHCb::MCParticleLocation::Default );
  
  //== Build a table (vector of vectors) of ids per MCParticle, indexed by MCParticle key.
  AllLinks<LHCb::MCParticle,ContainedObject> allIds( evtSvc(), msgSvc(), "Pat/LHCbID" );
  std::vector< std::vector<int> > linkedIds;
  LHCb::MCParticle* part = allIds.first();
  // loop over the MCPs in the linker
  while ( NULL != part ) {
    unsigned int minSize = part->key();
    //increase the size of the vector so that it have at least part->key() element
    //to access it via key... not very nice but practical.
    while ( linkedIds.size() <= minSize ) {
      std::vector<int> dum;
      linkedIds.push_back( dum );
    }
    linkedIds[part->key()].push_back( allIds.key() );//push back this lhcbid value
    part = allIds.next();
  }
  
  //== Prepare the container for tracks, and location 
  LHCb::Tracks * outputTracks;
  // create output track container on TES, if it does not exist
  if ( exist<LHCb::Tracks>( m_outputTracksLocation ) ) {
    outputTracks = get<LHCb::Tracks>( m_outputTracksLocation );
  } else {
    outputTracks = new LHCb::Tracks();
    put(outputTracks, m_outputTracksLocation );
  }
  // make space for the new tracks 
  outputTracks->reserve(1000);  
  
  MCTrackInfo trackInfo( evtSvc(), msgSvc());
  

  //== Loop over MCParticle and create LHCb::Track out of the LHCbIDs
  LHCb::MCParticles::const_iterator itP;
  for ( itP = mcParts->begin(); mcParts->end() != itP; ++itP ) {
    LHCb::MCParticle* part = *itP;
    VeloPixTrack newtrack;

    std::vector<IVeloPixClusterPosition::toolInfo> veloPixPositions;
    std::vector<Gaudi::XYZPoint> veloPixPoints;
    unsigned int countHits = 0;
    unsigned int prevSensor = 200;
    // Loop over the LHCbID corresponding to this MCParticle

    if( (unsigned int) part->key() >= linkedIds.size()) continue;
    for ( std::vector<int>::const_iterator itIm = linkedIds[part->key()].begin();
          linkedIds[part->key()].end() != itIm; ++itIm ) {
      // Get the Lite cluster corresponding to the LHCbID
      LHCb::LHCbID temp;
      
      temp.setDetectorType( (*itIm) >> 28 );  // create LHCbId from int. Clumsy !"
      temp.setID( (*itIm) );
      if ( ! temp.isVeloPix ()) continue;
      const LHCb::VeloPixLiteCluster* liteclus = m_clusters->object( temp.velopixID () );
        // Get the position (halfbox) and errors (most important is error, position is can be accessed via detelem)
      IVeloPixClusterPosition::toolInfo clusInfo = m_positiontool->position(liteclus) ;
      veloPixPositions.push_back(clusInfo) ;
      // Get the MCPostion of the Hit to check were not 10mm away...
      const DeVeloPixSquareType* sqDet =static_cast<const DeVeloPixSquareType*>(m_veloPix->squareSensor(clusInfo.pixel.sensor()));
        
      Gaudi::XYZPoint thePixPoint = sqDet->globalXYZ(clusInfo.pixel.pixel(),clusInfo.fractionalPosition) ;
      std::pair<double,double> pixSize = sqDet->PixelSize(clusInfo.pixel.pixel());
      
      double dx = pixSize.first*clusInfo.fractionalError.first ;
      if (sqDet->isLong(clusInfo.pixel.pixel())) dx = 0.1 ;//fixed to 0.1 mm whatever is the angle for long pixel
      double dy = pixSize.second*clusInfo.fractionalError.second;
      
      VeloPixHit *hit = new VeloPixHit(thePixPoint.x(),thePixPoint.y(),thePixPoint.z(),dx,dy,int(clusInfo.pixel.sensor()));
      
      hit->setLHCbID(temp);
      if (prevSensor != clusInfo.pixel.sensor()){
        prevSensor= clusInfo.pixel.sensor();         
        newtrack.addXHit(hit);
      }
      else delete hit; 
      countHits = newtrack.getHitsNum();
    }
    VeloPixTracks refittracks;
    VeloPixHits bhits = newtrack.hits();
    if(bhits.size()>2){
      double m_dist = 0.052;
//      double m_chi2 = 0.0001;
      for(VeloPixHits::iterator itH = bhits.begin(); (itH+2) != bhits.end(); itH++){
        if((*itH)->getused()) continue;        
        if((*(itH+1))->getused()) continue;
        VeloPixTrack refittrack;
        refittrack.addXHit(&(*(*itH)));
        refittrack.addXHit(&(*(*(itH+1))));
        if(((*(itH+1))->sensor()-(*itH)->sensor()) > 4 ) continue;
        int pvsensor = (*(itH+1))->sensor();
        for(VeloPixHits::iterator itH1 = itH+2; itH1 != bhits.end();itH1++){
          if((*itH1)->getused()) continue;
          if(((*itH1)->sensor()-pvsensor) >4 ) break;
          pvsensor = (*itH1)->sensor();
          if(refittrack.getHitsNum()>1){
            if(fabs((*itH1)->x()-refittrack.xAtz((*itH1)->z())) > m_dist ) continue;
            refittrack.addXHit(&(*(*itH1))); 
                       if( refittrack.getChi2()>(13+2*refittrack.getHitsNum())) {
           // if( refittrack.probChi2(refittrack.getChi2(),refittrack.getHitsNum()-2) < m_chi2 ) {
              refittrack.removeXHit(&(*(*itH1)));
              continue;
            }
            refittrack.UpdateYHits();
//            if( refittrack.probChi2(refittrack.getChi2(),2*(refittrack.getHitsNum()-2)) < m_chi2 ) {
             if( refittrack.getChi2()>(4*refittrack.getHitsNum()+9)) {
              refittrack.removeXHit(&(*(*itH1)));
              continue;
            }
            (*itH1)->addused();
            
            if(refittrack.getHitsNum() == 3){
              (*itH)->addused();
              (*(itH+1))->addused();}
          }
        }
        if(refittrack.getHitsNum()>2) refittracks.push_back(refittrack);
      }
    }
    // Be aware that the hit from same sensors are counted only once, but are all added to the list of ID
    if (countHits>m_minIDs && refittracks.size()>0){   //here, we make some requirement on the tracks to simulate the case for real one
      for(VeloPixTracks::iterator ivtr = refittracks.begin(); ivtr != refittracks.end(); ivtr++ ){
        int sensor = 200;
        VeloPixHits hits = (*ivtr).hits();
        std::vector<double> mc_dx,mc_dy;
        std::vector<int> sens;
        int Crossed = 0; int totMissed = 0;
        for(VeloPixHits::iterator itH = hits.begin(); itH != hits.end(); itH++){
          if(sensor == 200) sens.push_back(0);
          else {sens.push_back((*itH)->sensor()-sensor);
            if(((*itH)->sensor()-sensor)%2 == 1){               
              Crossed = 1;
              totMissed += int(((*itH)->sensor()-sensor)/2);
            }
            else totMissed += int(((*itH)->sensor()-sensor)/2)-1;
          }
          sensor = (*itH)->sensor();
          mc_dx.push_back((*itH)->x()-(*ivtr).xAtz((*itH)->z()));
          mc_dy.push_back((*itH)->y()-(*ivtr).yAtz((*itH)->z()));
        }
        tuple->column("mc_chi2",(*ivtr).getChi2());
        tuple->column("mc_ndof",(*ivtr).getHitsNum());
        tuple->column("totMissed",totMissed);
        tuple->column("Crossed",Crossed);
        tuple->farray("mc_dx",mc_dx.begin(),mc_dx.end(),"hits",40);
        tuple->farray("mc_dy",mc_dy.begin(),mc_dy.end(),"hits",40);
        tuple->farray("sens",sens.begin(),sens.end(),"hits",40);
        tuple->write();
        
        LHCb::Track *newTrack = new LHCb::Track();
        
        newTrack->setType( LHCb::Track::Velo );
        newTrack->setHistory( LHCb::Track::PatVeloPixCheated);
        newTrack->setPatRecStatus( LHCb::Track::PatRecIDs );
        
        double zMin = 1.e9;
        double zMax = -1.e9;
        
        for( VeloPixHits::iterator itH = hits.begin();hits.end() != itH; itH++){
          newTrack->addToLhcbIDs( LHCb::LHCbID((*itH)->lhcbID()) );
        
          if( (*itH)->z() > zMax ) zMax = (*itH)->z();
          if( (*itH)->z() < zMin ) zMin = (*itH)->z();
        }
        
        LHCb::State state;
        double zBeam = (*ivtr).zBeam();
        bool backward = zBeam > zMax;
        newTrack->setFlag( LHCb::Track::Backward, backward );  
     
        
        if ( m_stateAtBeam ) {
          state.setLocation( LHCb::State::ClosestToBeam );
          state.setState( (*ivtr).state( zBeam ) );
          state.setCovariance( (*ivtr).covariance( zBeam ) );
          newTrack->addToStates( state );
        } else {
          state.setLocation( LHCb::State::FirstMeasurement );
          state.setState( (*ivtr).state( zMin ) );
          state.setCovariance( (*ivtr).covariance( zMin ) );
          newTrack->addToStates( state );
        }     
        
        if(!backward){
          state.setLocation( LHCb::State::EndVelo );
          state.setState( (*ivtr).state(zMax) );
          state.setCovariance( (*ivtr).covariance( zMax ));
          newTrack->addToStates( state );
        }
        
        
        
        if (m_UseLinearFit == true){
          
          StatusCode scFit = m_veloPixFitter->fit(*newTrack, LHCb::ParticleID(211));
          
          if(!scFit.isSuccess()){
            warning()<<"The linear fit failed"<<endreq;
          }
        }
        outputTracks->insert(newTrack);
      }
    }
    for(VeloPixHits::iterator itH = bhits.begin(); itH != bhits.end(); itH++){
      delete *itH;      
    }
  }
    
  return StatusCode::SUCCESS;
};


//=============================================================================
//  Finalize
//=============================================================================
StatusCode CheatedVeloPixPat::finalize() {
  debug() << "==> Finalize" << endmsg;
  return GaudiTupleAlg::finalize();  // must be called after all other actions
};


//=============================================================================

