// $Id: $for velo pix track fit
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

#include "Event/Track.h"
#include "Event/VeloPixCluster.h"
#include "Event/VeloPixLiteMeasurement.h"
// local
#include "VeloPixPatLinear.h"
#include "VeloPixHit.h"
#include "VeloPixTrack.h"


//-----------------------------------------------------------------------------
// Implementation file for class : VeloPixPatLinear
//
// 2011-02-11 : Wenbin Qian
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( VeloPixPatLinear )


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
VeloPixPatLinear::VeloPixPatLinear( const std::string& name,
                                    ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ),
    m_sensor(48)
{
  declareProperty("outputTracksLocation",      m_outputTracksLocation = LHCb::TrackLocation::VeloPix);
  declareProperty("TrackChi2LimitePerNDOF",    m_chi2 = 0.00001);
  declareProperty("HitsWindows",               m_dist = 0.052*Gaudi::Units::mm);
  declareProperty("MaxMissed",                 m_maxMissed = 1);
  declareProperty( "FractionForMerge", m_fractionForMerge= 0.60   );
  declareProperty("StateAtBeam",               m_stateAtBeam    = true );  
}
//=============================================================================
// Destructor
//=============================================================================
VeloPixPatLinear::~VeloPixPatLinear() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode VeloPixPatLinear::initialize() {
  StatusCode sc = GaudiAlgorithm::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;

  m_veloPix = getDet<DeVeloPix>(DeVeloPixLocation::Default);
  
  m_positiontool = tool<IVeloPixClusterPosition>("VeloPixClusterPosition");

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode VeloPixPatLinear::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;
  m_clusters = get<LHCb::VeloPixLiteCluster::VeloPixLiteClusters>(LHCb::VeloPixLiteClusterLocation::Default );

  m_hits.clear();
  for(int i=0; i<m_sensor;i++){
    VeloPixHits dum;
    m_hits.push_back(dum);
  }

    
  //here,get position information from clusterlite
  LHCb::VeloPixLiteCluster::VeloPixLiteClusters::const_iterator iclust;
  for(iclust = m_clusters->begin(); iclust != m_clusters->end(); iclust++){  
    IVeloPixClusterPosition::toolInfo clusInfo = m_positiontool->position(&(*iclust));
    const DeVeloPixSquareType* sqDet =static_cast<const DeVeloPixSquareType*>(m_veloPix->squareSensor(clusInfo.pixel.sensor()));
    Gaudi::XYZPoint thePixPoint = sqDet->globalXYZ(clusInfo.pixel.pixel(),clusInfo.fractionalPosition) ; 
    std::pair<double,double> pixSize = sqDet->PixelSize(clusInfo.pixel.pixel());
    
    double dx = pixSize.first*clusInfo.fractionalError.first ;
    if (sqDet->isLong(clusInfo.pixel.pixel())) dx = 0.1 ;//fixed to 0.1 mm whatever is the angle for long pixel
    double dy = pixSize.second*clusInfo.fractionalError.second;
    LHCb::VeloPixChannelID pixid = (*iclust).channelID();
    int sensornum = m_veloPix->sensorNum(pixid);
    VeloPixHit* hit = new VeloPixHit(thePixPoint.x(),thePixPoint.y(),thePixPoint.z(),dx,dy,sensornum);
    hit->setLHCbID(LHCb::LHCbID(pixid));
    m_hits[sensornum].push_back(hit);   
  }


  //make room for tracks
  LHCb::Tracks* outputTracks;
  if ( exist<LHCb::Tracks>( m_outputTracksLocation ) ) {
    outputTracks = get<LHCb::Tracks>( m_outputTracksLocation );
  } else {
    outputTracks = new LHCb::Tracks();
    put(outputTracks, m_outputTracksLocation );
  }
  outputTracks->reserve(5000);
  m_tracks.clear();

 
  for(int i = 0; i<m_sensor-6; i++){
    VeloPixHits sensor0 = m_hits[i];
    if(sensor0.size()<1) continue;
    findQuadruplets(i);
  }
  
   for(int i = 0; i<m_sensor-4; i++){
    VeloPixHits sensor0 = m_hits[i];
    if(sensor0.size()<1) continue;
    findTriplets(i);
   }
   addAnotherSideHits(); 
   for(int i = 0; i<m_sensor-3; i++){
    VeloPixHits sensor0 = m_hits[i];
    if(sensor0.size()<1) continue;
    findCrossed(i);
   }
   mergeClones();
   makeLHCbTracks( outputTracks );
   return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode VeloPixPatLinear::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg; 

  return GaudiAlgorithm::finalize();  // must be called after all other actions
}

//=============================================================================
void VeloPixPatLinear::findQuadruplets( int sens0)
{
  VeloPixHits sensor0 = m_hits[sens0];
  
  VeloPixHits sensor1;
  VeloPixHits sensor2;
  VeloPixHits sensor3;
 
  for( int iCase = 0; 4 > iCase; ++iCase){
    if( iCase == 0 ){
      sensor1 = m_hits[sens0+2];
      sensor2 = m_hits[sens0+4];
      sensor3 = m_hits[sens0+6];
    }
    if( iCase == 1 && sens0<(m_sensor-8)){
        sensor1 = m_hits[sens0+4];
        sensor2 = m_hits[sens0+6];
        sensor3 = m_hits[sens0+8];
     }
      if( iCase == 2 && sens0<(m_sensor-8) ){
        sensor1 = m_hits[sens0+2];
        sensor2 = m_hits[sens0+6];
        sensor3 = m_hits[sens0+8];
      }
      if( iCase == 3 && sens0<(m_sensor-8)){
        sensor1 = m_hits[sens0+2];
        sensor2 = m_hits[sens0+4];
        sensor3 = m_hits[sens0+8];
      }
        
    if ( sensor1.size()<1 ) continue;
    if ( sensor2.size()<1 ) continue;
    if ( sensor3.size()<1 ) continue;
    
    VeloPixHits::const_iterator c0, c3, c1, c2;
    for( c0 = sensor0.begin();sensor0.end() != c0; ++c0){
      if((*c0)->getused()) continue;
      VeloPixTrack newtrack;
      newtrack.addXHit(&(*(*c0)));
      for( c3 = sensor3.begin();sensor3.end() != c3; ++c3){
        if((*c3)->getused()) continue;
        newtrack.addXHit(&(*(*c3)));
        for( c1 = sensor1.begin();sensor1.end() != c1; ++c1){
       if((*c1)->getused()) continue;
          if(fabs((*c1)->x()-newtrack.xAtz((*c1)->z())) > m_dist ) continue;
          newtrack.addXHit(&(*(*c1)));
//          if( newtrack.probChi2(newtrack.getChi2(),newtrack.getHitsNum()-2)< m_chi2 ){ 
          if( newtrack.getChi2()>(2*newtrack.getHitsNum()+13) ){
             newtrack.removeXHit(&(*(*c1)));
            continue;
          }
          for( c2 = sensor2.begin();sensor2.end() != c2; ++c2){
            if((*c2)->getused()) continue;
            if(fabs((*c2)->x()-newtrack.xAtz((*c2)->z())) > m_dist ) continue;
            newtrack.addXHit(&(*(*c2)));
//            if( newtrack.probChi2(newtrack.getChi2(),newtrack.getHitsNum()-2) < m_chi2 ) {
if( newtrack.getChi2()>(2*newtrack.getHitsNum()+13) ){
               newtrack.removeXHit(&(*(*c2)));
              continue;
            }
            newtrack.UpdateYHits();
//            if( newtrack.probChi2(newtrack.getChi2(),2*(newtrack.getHitsNum()-2)) < m_chi2 ) {
if( newtrack.getChi2()>(4*newtrack.getHitsNum()+9) ){ 
             newtrack.removeXHit(&(*(*c2)));
              continue;
            }
            (*c0)->addused();
            (*c1)->addused();
            (*c2)->addused();
            (*c3)->addused();
            
            if(iCase == 0)
              extendTrack(newtrack, sens0+8, true);
            else extendTrack(newtrack, sens0+10, true);
            m_tracks.push_back(newtrack);
            newtrack.removeXHit(&(*(*c2)));
          }//loop c2
          newtrack.removeXHit(&(*(*c1)));
        }//loop c1
        newtrack.removeXHit(&(*(*c3)));
      }//loop c3
      newtrack.removeXHit(&(*(*c0)));
    }//loop c0
  }//loop icase
}


void VeloPixPatLinear::findTriplets (int sens0)
{
  VeloPixHits sensor0 = m_hits[sens0];
  
  VeloPixHits sensor1;
  VeloPixHits sensor2;
 
  for( int iCase = 0; 3 > iCase; ++iCase){
    if( iCase == 0 ){
      sensor1 = m_hits[sens0+2];
      sensor2 = m_hits[sens0+4];
    }
    
      if( iCase == 1 && sens0< (m_sensor-6) ){
        sensor1 = m_hits[sens0+4];
        sensor2 = m_hits[sens0+6];
      }
      if( iCase == 2 && sens0< (m_sensor-6) ){
        sensor1 = m_hits[sens0+2];
        sensor2 = m_hits[sens0+6];
      }
       
    if ( sensor1.size()<1 ) continue;
    if ( sensor2.size()<1 ) continue;
       
    VeloPixHits::const_iterator c0, c1, c2;
    for( c0 = sensor0.begin();sensor0.end() != c0; ++c0){
      if((*c0)->getused()) continue;
      VeloPixTrack newtrack;
      newtrack.addXHit(&(*(*c0)));
      for( c2 = sensor2.begin();sensor2.end() != c2; ++c2){
        if((*c2)->getused()) continue;
        newtrack.addXHit(&(*(*c2)));
        for( c1 = sensor1.begin();sensor1.end() != c1; ++c1){
          if((*c1)->getused()) continue;
          if(fabs((*c1)->x()-newtrack.xAtz((*c1)->z())) > m_dist ) continue;
          newtrack.addXHit(&(*(*c1)));
//          if( newtrack.probChi2(newtrack.getChi2(),newtrack.getHitsNum()-2) < m_chi2) {
if( newtrack.getChi2()>(2*newtrack.getHitsNum()+13) ){
            newtrack.removeXHit(&(*(*c1)));
            continue;
          }
          newtrack.UpdateYHits();
//          if( newtrack.probChi2(newtrack.getChi2(),2*(newtrack.getHitsNum()-2)) < m_chi2) {
if( newtrack.getChi2()>(4*newtrack.getHitsNum()+9) ){
            newtrack.removeXHit(&(*(*c1)));
            continue;
          }
          (*c0)->addused();
          (*c1)->addused();
          (*c2)->addused();
          if(iCase == 0)
            extendTrack(newtrack, sens0+6, true);
          else  extendTrack(newtrack, sens0+8, true);
          m_tracks.push_back(newtrack);
          newtrack.removeXHit(&(*(*c1)));
        }//loop c1
        newtrack.removeXHit(&(*(*c2)));
      }//loop c3
      newtrack.removeXHit(&(*(*c0)));
    }//loop c0
  }//loop icase
}

void VeloPixPatLinear::findCrossed (int sens0)
{
  VeloPixHits sensor0 = m_hits[sens0];
             
  VeloPixHits sensor1;
  VeloPixHits sensor2;
          
  for( int iCase = 0; 6 > iCase; ++iCase){
    if( iCase == 0 ){
      sensor1 = m_hits[sens0+1];
      sensor2 = m_hits[sens0+3];  
    }
      if( iCase == 1 && sens0<(m_sensor-5) ){
        sensor1 = m_hits[sens0+3];
        sensor2 = m_hits[sens0+5];
      }
      if( iCase == 2 && sens0<(m_sensor-5)){
        sensor1 = m_hits[sens0+1];
        sensor2 = m_hits[sens0+5];
      }
    if( iCase == 3 ){
      sensor1 = m_hits[sens0+2];
      sensor2 = m_hits[sens0+3];
    }     
      if( iCase == 4 && sens0< (m_sensor-5)){
        sensor1 = m_hits[sens0+4];
        sensor2 = m_hits[sens0+5];
      }
      if( iCase == 5 && sens0<(m_sensor-5)){
        sensor1 = m_hits[sens0+2];
        sensor2 = m_hits[sens0+5];
      }
   
    if ( sensor1.size()<1 ) continue;
    if ( sensor2.size()<1 ) continue;
          
    VeloPixHits::const_iterator c0, c1, c2;
    for( c0 = sensor0.begin();sensor0.end() != c0; ++c0){
      if((*c0)->getused()) continue;
      VeloPixTrack newtrack;
      newtrack.addXHit(&(*(*c0)));
      for( c2 = sensor2.begin();sensor2.end() != c2; ++c2){
        if((*c2)->getused()) continue; 
        newtrack.addXHit(&(*(*c2)));
        for( c1 = sensor1.begin();sensor1.end() != c1; ++c1){
          if((*c1)->getused()) continue;
          if(fabs((*c1)->x()-newtrack.xAtz((*c1)->z())) > m_dist ) continue;
          newtrack.addXHit(&(*(*c1)));
//          if( newtrack.probChi2(newtrack.getChi2(),newtrack.getHitsNum()-2) < m_chi2) {
if( newtrack.getChi2()>(2*newtrack.getHitsNum()+13) ){
            newtrack.removeXHit(&(*(*c1)));
            continue;
          }
          newtrack.UpdateYHits();
//          if( newtrack.probChi2(newtrack.getChi2(),2*(newtrack.getHitsNum()-2)) < m_chi2) {
if( newtrack.getChi2()>(4*newtrack.getHitsNum()+9) ){
            newtrack.removeXHit(&(*(*c1)));
            continue; 
          }
          (*c0)->addused();
          (*c1)->addused();
          (*c2)->addused();
          if(iCase == 0)
            extendTrack(newtrack, sens0+5, true);
          else extendTrack(newtrack, sens0+7, true);
           m_tracks.push_back(newtrack);
          newtrack.removeXHit(&(*(*c1)));
        }//loop c1
        newtrack.removeXHit(&(*(*c2)));
      }//loop c3
      newtrack.removeXHit(&(*(*c0)));
    }//loop c0
  }//loop icase
}

void VeloPixPatLinear::extendTrack( VeloPixTrack& newtrack, int sens0,  bool forward)
{
  int nMiss = 0;
  int sign;
  if(forward) sign = 1;
  else sign = -1;
  
  bool inside = sens0<m_sensor;
  while ( m_maxMissed >= nMiss && inside){
    bool addhit = false;
    VeloPixHits sensor1 = m_hits[sens0];
    if(sensor1.size()<1){
      nMiss++;
      sens0 = sens0 + sign *2;
      inside =((sens0 > 0) && (sens0 < m_sensor));
      continue;
    }

    VeloPixHits::iterator itbest;
    double bestchi2 = 10000;
    for(VeloPixHits::iterator itH = sensor1.begin(); sensor1.end() != itH; ++itH ){
      if(fabs((*itH)->x()-newtrack.xAtz((*itH)->z())) > m_dist) continue;
      newtrack.addXHit(&(*(*itH)));
//      if( newtrack.probChi2(newtrack.getChi2(),newtrack.getHitsNum()-2) < m_chi2 ) {
if( newtrack.getChi2()>(2*newtrack.getHitsNum()+13) ){
        newtrack.removeXHit(&(*(*itH)));
        continue;
      }
      newtrack.UpdateYHits();
//      if( newtrack.probChi2(newtrack.getChi2(),2*(newtrack.getHitsNum()-2)) < m_chi2 ) {
if( newtrack.getChi2()>(4*newtrack.getHitsNum()+9) ){
        newtrack.removeXHit(&(*(*itH)));
        continue;
      }
      addhit = true;
      if( newtrack.getChi2() < bestchi2 ){
        bestchi2 = newtrack.getChi2();
        itbest = itH;
      }
      newtrack.removeXHit(&(*(*itH)));
    }
    if(!addhit) nMiss++;
    else {
      (*itbest)->addused();
      newtrack.addXHit(&(*(*itbest)));
    }
    sens0 = sens0 + sign * 2;
    inside =((sens0 > 0) && (sens0 < m_sensor));
  }
}

void VeloPixPatLinear::addAnotherSideHits()
{
  if(m_tracks.size() == 0) return;
  VeloPixTracks::iterator itT;
  for( itT = m_tracks.begin(); itT != m_tracks.end();++itT){
    int firstsensor = (*((*itT).hits().begin()))->sensor()-1;
    int endsensor = (*( (*itT).hits().end()-1))->sensor()+1;
    if(firstsensor>-1){
      const DeVeloPixSquareType *firsttype = m_veloPix->squareSensor(firstsensor);
      double firstz = firsttype->z();
      Gaudi::XYZPoint firstpoint((*itT).xAtz(firstz),(*itT).yAtz(firstz),firstz);
      if(firsttype->isInActiveArea(firstpoint)) extendTrack((*itT),firstsensor,false);
    }
    if(endsensor < m_sensor){
      const DeVeloPixSquareType *endtype = m_veloPix->squareSensor(endsensor);
      double endz = endtype->z();
      Gaudi::XYZPoint endpoint((*itT).xAtz(endz),(*itT).yAtz(endz),endz);
      if(endtype->isInActiveArea(endpoint)) extendTrack((*itT),endsensor,true);
    } 
  }
}



void VeloPixPatLinear::makeLHCbTracks( LHCb::Tracks* outputTracks)
{
  if(m_tracks.size()){
    for( VeloPixTracks::iterator itT = m_tracks.begin(); m_tracks.end() != itT; ++itT ){
      if(!(*itT).isValid()) continue;
      LHCb::Track *newtrack = new LHCb::Track();
      newtrack->setType( LHCb::Track::Velo );
      newtrack->setHistory( LHCb::Track::PatVeloPix );
      newtrack->setPatRecStatus( LHCb::Track::PatRecIDs );
      
      double zMin = 1.e9;
      double zMax = -1.e9;

      VeloPixHits hits = (*itT).hits();
      for( VeloPixHits::iterator itH = hits.begin();hits.end() != itH; itH++){
        newtrack->addToLhcbIDs( (*itH)->lhcbID() );
        if( (*itH)->z() > zMax ) zMax = (*itH)->z();
        if( (*itH)->z() < zMin ) zMin = (*itH)->z();
      }
      
      LHCb::State state;
      double zBeam = (*itT).zBeam();
      bool backward = zBeam > zMax;
      newtrack->setFlag( LHCb::Track::Backward, backward );
      
      if ( m_stateAtBeam ) {
        state.setLocation( LHCb::State::ClosestToBeam );
        state.setState( (*itT).state( zBeam ) );
        state.setCovariance( (*itT).covariance( zBeam ) );
        newtrack->addToStates( state );
      } else {
        state.setLocation( LHCb::State::FirstMeasurement );
        state.setState( (*itT).state( zMin ) );
        state.setCovariance( (*itT).covariance( zMin ) );
        newtrack->addToStates( state );
      }

      if(!backward){
        state.setLocation( LHCb::State::EndVelo );
        state.setState( (*itT).state(zMax) );
        state.setCovariance( (*itT).covariance( zMax ));
        newtrack->addToStates( state );
      }
      
      outputTracks->insert( newtrack );  
    }
  }
}
void VeloPixPatLinear::mergeClones ( ) {
  if ( m_tracks.size() <= 1 ) return;
  VeloPixTracks::iterator it1, it2;
  for ( it1 = m_tracks.begin(); m_tracks.end()-1 > it1 ; ++it1 ) {
    if ( !(*it1).isValid() ) continue;
    int n1 = (*it1).hits().size();
    for ( it2 = it1+1; m_tracks.end() != it2 ; ++it2 ) {
      if ( !(*it2).isValid() ) continue;
      int n2 = (*it2).hits().size();
      int minN = n1 > n2 ? n2 : n1;
      int nCommon = 0;
      VeloPixHits::iterator itH1,itH2;
      VeloPixHits hits1 = (*it1).hits();
      VeloPixHits hits2 = (*it2).hits();
      for ( itH1 = hits1.begin(); hits1.end() != itH1 ; ++itH1 ) {
        for ( itH2 = hits2.begin(); hits2.end() != itH2 ; ++itH2 ) {
          if ( (*itH1)== (*itH2) ) {++nCommon;
            break;}
        }
      }
      
      if ( nCommon > m_fractionForMerge * minN ) {
        if ( n2 > n1 ) {
          (*it1).setValid( false );
          break;
        } else if ( n1 > n2 ) {
          (*it2).setValid( false );
        } else if ( (*it1).getChi2() < (*it2).getChi2() ) {
          (*it2).setValid( false );
        } else {
          (*it1).setValid( false );
          break;
        }
      }
    } 
  }
}

