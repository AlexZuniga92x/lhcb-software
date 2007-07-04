// $Id: MuonSeedTool.cpp,v 1.2 2007-07-04 15:31:10 albrecht Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/DeclareFactoryEntries.h" 

// from Event
#include "Event/State.h"
#include "Event/Track.h"

#include "Event/L0MuonCandidate.h"
#include "MuonDet/IMuonPosTool.h"

// local
#include "MuonSeedTool.h"

//boost
#include <boost/assign/list_of.hpp>

using namespace LHCb;

//-----------------------------------------------------------------------------
// Implementation file for class : MuonSeedTool
//
// 2007-07-03 : Johannes Albrecht
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( MuonSeedTool );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
MuonSeedTool::MuonSeedTool( const std::string& type,
                            const std::string& name,
                            const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<IMuonSeedTool>(this);

  declareProperty("debugMode",m_debugMode = false );

  //standard parameters af of tune JA 2007-03-22
  declareProperty("sigmaX2", m_sigmaX2 = boost::assign::list_of(64.)(225.)(841.)(2916.) );
  declareProperty("sigmaY2", m_sigmaY2 = boost::assign::list_of(36.)(100.)(400.)(1600.) );
  declareProperty("sigmaTx2", m_sigmaTx2 = boost::assign::list_of(9.e-6)(16.e-6)(49.e-6)(169.e-6) );
  declareProperty("sigmaTy2", m_sigmaTy2 = boost::assign::list_of(16.e-6)(36.e-6)(64e-6)(100.e-6) );

}
//=============================================================================
// Destructor
//=============================================================================
MuonSeedTool::~MuonSeedTool() {} 

//=============================================================================

//-------------------------------------
StatusCode MuonSeedTool::initialize() 
{
  StatusCode sc = GaudiTool::initialize();
  if (sc.isFailure()){
    return sc;
  }
  
  m_iPosTool=tool<IMuonPosTool>( "MuonPosTool" );
  
  //tool for debug information
  m_DataStore = tool<L0ConfDataStore>("L0ConfDataStore");
  
  return StatusCode::SUCCESS;
}

StatusCode MuonSeedTool::makeTrack( const LHCb::L0MuonCandidate& muonL0Cand,
                                    LHCb::Track& seedTrack )
{

  LHCb::State seedState;
 
  std::vector<MuonTileID> mpads1 = muonL0Cand.muonTileIDs(0); 
  std::vector<MuonTileID> mpads2 = muonL0Cand.muonTileIDs(1); 
  std::vector<MuonTileID> mpads3 = muonL0Cand.muonTileIDs(2); 
  
  MuonTileID mpad2 = mpads2.front();
  
  LHCb::LHCbID id2 = LHCbID( mpad2 );
  seedTrack.addToLhcbIDs(id2);
  LHCb::LHCbID id3 = LHCbID( mpads3.front() );
  seedTrack.addToLhcbIDs(id3);
  
  int regionL0Cand;
  if (mpad2){
    regionL0Cand = mpad2.region();
  }
  else{
    err()<<"No valid Muon Tile in M2, quit loop"<<endmsg;
    return StatusCode::SUCCESS;
  }

  double x , y, z;
  double dxTileM1, dyTileM1, dzTileM1;
  int numberOfTiles = 0;
  double xTileM1 = 0.;
  double yTileM1 = 0.;
  double zTileM1 = 0.;
  StatusCode sc;
    
  for( std::vector<MuonTileID>::iterator it = (mpads1.begin());
       it != (mpads1.end()) ; 
       ++it ) {
      
    numberOfTiles++;
    MuonTileID mpad1 = *it;
      
    //positions in M1
    sc = m_iPosTool->calcTilePos( mpad1,
                                  x, dxTileM1,
                                  y, dyTileM1,
                                  z, dzTileM1);

    LHCb::LHCbID tmpId = LHCbID( mpad1 );
    seedTrack.addToLhcbIDs( tmpId );
    
    if (!sc) {
      err()<<"Unable to get Position for M1"<<endmsg;
      return StatusCode::SUCCESS;
    }
      
    xTileM1 += x;
    yTileM1 += y;
    zTileM1 += z;
  }
    
  xTileM1 = xTileM1 / double(numberOfTiles);
  yTileM1 = yTileM1 / double(numberOfTiles);
  zTileM1 = zTileM1 / double(numberOfTiles);
    

  double xTileM2 , yTileM2, zTileM2;
  double dxTileM2, dyTileM2, dzTileM2;
    
  sc = m_iPosTool->calcTilePos(mpad2,
                               xTileM2, dxTileM2,
                               yTileM2, dyTileM2,
                               zTileM2, dzTileM2);
  if (!sc) {
    err()<<"Unable to get Position for M2"<<endmsg;
    return StatusCode::SUCCESS;
  }

  //  double zT3 = 9315.;//middle T3
  double dxdz   = (xTileM2 - xTileM1)/ ( zTileM2 - zTileM1);
  double dydz   = (yTileM2 - yTileM1)/ ( zTileM2 - zTileM1);
    
  //double xT3 = xTileM2 - dxdz * (zTileM2 - zT3);
  //double yT3 = yTileM2 - dydz * (zTileM2 - zT3);

  seedState.setState( xTileM2 , yTileM2 , zTileM2 , dxdz , dydz , 0 );

  double L0p = fabs( muonL0Cand.pt()/sin(muonL0Cand.theta()) ) ;
  double L0pt = muonL0Cand.pt();//get pt with sign!
  double L0q    = (L0pt == 0) ? 0.0 : -1.*L0pt/fabs(L0pt);
  double qOverP = L0q / L0p;
    
  seedState.setQOverP( qOverP );
  Gaudi::TrackSymMatrix stateCov = Gaudi::TrackSymMatrix();
    
  stateCov(0,0) = m_sigmaX2[regionL0Cand];
  stateCov(1,1) = m_sigmaY2[regionL0Cand];
  stateCov(2,2) = m_sigmaTx2[regionL0Cand];
  stateCov(3,3) = m_sigmaTy2[regionL0Cand];
  stateCov(4,4) = 8.41e-6;

  seedState.setCovariance(stateCov);

  if( m_debugMode ){
    m_DataStore->region = regionL0Cand;
  }
    
  // add to states
  seedTrack.addToStates(seedState);
  //seedTrack.setType(LHCb::Track::Muon);
  
  return StatusCode::SUCCESS;
    
}
