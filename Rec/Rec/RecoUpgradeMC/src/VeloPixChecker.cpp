
// Include files 

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

#include "Event/Track.h"
#include "Event/MCParticle.h"
#include "Linker/LinkedTo.h"
#include "LHCbMath/Line.h"
#include "LHCbMath/GeomFun.h"
#include "Event/MCHit.h"

#include "Event/VeloPixCluster.h"
#include "Event/VeloPixLiteMeasurement.h"
// local
#include "VeloPixChecker.h"


//-----------------------------------------------------------------------------
// Implementation file for class : VeloPixChecker
//
// 2006-05-11 : Olivier Callot
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( VeloPixChecker );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
VeloPixChecker::VeloPixChecker( const std::string& name,
                                    ISvcLocator* pSvcLocator)
  : GaudiTupleAlg ( name , pSvcLocator )
{
  declareProperty( "NtupleName"           , m_tupleName = "Tracks"    );
  declareProperty( "InputTracks"          , m_container = "Rec/Track/PreparedVeloPix" );
}
//=============================================================================
// Destructor
//=============================================================================
VeloPixChecker::~VeloPixChecker() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode VeloPixChecker::initialize() {
  StatusCode sc = GaudiTupleAlg::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiAlgorithm

  debug() << "==> Initialize" << endmsg;

  m_positiontool =  tool<IVeloPixClusterPosition>("VeloPixClusterPosition");
  m_linkTool = tool<ILHCbIDsToMCHits>("LHCbIDsToMCHits","IDsToMCHits",this);
  m_veloPix = getDet<DeVeloPix>( DeVeloPixLocation::Default );

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode VeloPixChecker::execute() {

  debug() << "==> Execute" << endmsg;

  LHCb::Tracks* tracks = get<LHCb::Tracks>( m_container );
  verbose()<<m_container<<" size: "<<tracks->size()<<endreq;
  
  LinkedTo<LHCb::MCParticle, LHCb::Track> link( evtSvc(), msgSvc(), m_container );


  m_clusters = get<LHCb::VeloPixLiteCluster::VeloPixLiteClusters>(LHCb::VeloPixLiteClusterLocation::Default );
  
  for ( LHCb::Tracks::const_iterator itT = tracks->begin(); tracks->end() != itT ; ++itT ) {
    LHCb::Track* tr = *itT;
    Gaudi::XYZPoint position = tr->position();
    Gaudi::XYZPoint firstposition = tr->position();
    Gaudi::XYZVector slopes  = tr->slopes();
    double momentum          = tr->p()*tr->charge();
    double transmomentum     = tr->pt();
    std::vector< const LHCb::Measurement * > meas = tr->measurements ();
    // Check backward
    verbose()<<"ere"<<endreq;
    
    bool isBack = tr-> checkFlag(LHCb::Track::Backward);
    
    // define for first hit info in ntuple
    LHCb::MCHit* hit = NULL;
    Gaudi::XYZPoint thePixPoint(0.,0.,0.);
    double dx(0.),dy(0.);
    IVeloPixClusterPosition::toolInfo clusInfo;
    
    // get infos from the first hit
    if(meas.size()!=0){
      double bestZ = 10000.;
      LHCb::LHCbID theID;
      if (isBack) bestZ = -10000.;
      //if(meas.size()==0)continue;
    
    
      for (std::vector< const LHCb::Measurement * >::const_iterator i_meas = meas.begin(); meas.end()!=i_meas ; i_meas++){
        const LHCb::LHCbID & theLHCBID = (*i_meas)->lhcbID ();
        
        LHCb::VeloPixChannelID 	veloPixID  = theLHCBID.velopixID ();
        
        const DeVeloPixSquareType* sqDet =static_cast<const DeVeloPixSquareType*>(m_veloPix->squareSensor(veloPixID.sensor()));
        if (isBack && sqDet->z()>bestZ){
          bestZ=sqDet->z();
          theID.setDetectorType( theLHCBID.detectorType());  // create LHCbId from int. Clumsy !"
          theID.setID( veloPixID.channelID() );
        }
        else if(!isBack && sqDet->z()<bestZ){
          bestZ=sqDet->z();
          theID.setDetectorType( theLHCBID.detectorType() );  // create LHCbId from int. Clumsy !"
          theID.setID(veloPixID.channelID() );
        }
      }

      if ( ! theID.isVeloPix ()) continue;
      ILHCbIDsToMCHits::LinkMap testMap;
      StatusCode sc = linkTool()->link(theID,testMap);
      ILHCbIDsToMCHits::LinkMap::iterator it = testMap.begin();
      hit = it->first;
      
      // Get the VeloPixLiteCluster corresponding to the first measurment
      const LHCb::VeloPixLiteCluster* liteclus = m_clusters->object( theID.velopixID () );
     	
      //const LHCb::StateVector* theStateVector = new LHCb::StateVector (tr->firstState().stateVector (), bestZ);
      // Get the position (halfbox) and errors (most important is error, position is can be accessed via detelem)
      //IVeloPixClusterPosition::toolInfo clusInfo = m_positiontool->position(liteclus,*theStateVector) ;
      clusInfo = m_positiontool->position(liteclus) ;
      const DeVeloPixSquareType* sqDet =static_cast<const DeVeloPixSquareType*>(m_veloPix->squareSensor(clusInfo.pixel.sensor()));
      thePixPoint = sqDet->globalXYZ(clusInfo.pixel.pixel(),clusInfo.fractionalPosition) ;
      std::pair<double,double> pixSize = sqDet->PixelSize(clusInfo.pixel.pixel());
      //Check about errors too...
      dx = pixSize.first*clusInfo.fractionalError.first ;
      if (sqDet->isLong(clusInfo.pixel.pixel())) dx = 0.1 ;//fixed to 0.1 mm whatever is the angle for long pixel
      dy = pixSize.second*clusInfo.fractionalError.second;
    }
    

    LHCb::MCParticle* part = link.first( tr );
    while( 0 != part ) {
      Gaudi::XYZPoint mcPosition = part->originVertex()->position();
      if ( mcPosition.z() > 700. ) {  // ignore tracks created after the Velo.
        part = link.next();
        continue;
      }
      
      
      Gaudi::XYZVector mcSlopes  = part->momentum().Vect();
      mcSlopes /= mcSlopes.z();
      double mcMomentum = part->momentum().P();
      double mcTransMomentum = part->momentum().Pt();
      int charge = part->particleID().threeCharge()/3;
      mcMomentum *= charge;

      Gaudi::XYZVector MCToRecPosition(position.x() - mcPosition.x(),position.y() - mcPosition.y(),position.z() - mcPosition.z());
      
      Gaudi::XYZPoint positionForIp(position.x(),position.y(),position.z());
      
      Gaudi::Math::Line<Gaudi::XYZPoint,Gaudi::XYZVector> line(positionForIp,slopes);

      position += ( mcPosition.z() - position.z() ) * slopes;


      double ip = Gaudi::Math::impactParameter(mcPosition,line);

      // Fill the ntuple  
      verbose()<<m_container<<endreq;
      
      Tuple tPoint = nTuple( m_tupleName );
      if (hit!=NULL){ 
        tPoint->column("hit1_dxCalc",dx);
        tPoint->column("hit1_dyCalc",dy);
        tPoint->column("hit1_dx",hit->midPoint () .x()-thePixPoint.x());
        tPoint->column("hit1_dy",hit->midPoint () .y()-thePixPoint.y());
        tPoint->column("hit1_dz",hit->midPoint () .z()-thePixPoint.z());
        tPoint->column("hit1_dtx",hit->displacement ().x()/hit->displacement ().z()-slopes.x());
        tPoint->column("hit1_dty",hit->displacement ().y()/hit->displacement ().z()-slopes.y());
        tPoint->column("chip1",clusInfo.pixel.chip());
        tPoint->column("pixelX1",clusInfo.pixel.pixel_lp());
        tPoint->column("pixelY1",clusInfo.pixel.pixel_hp());
      }
      else{
        tPoint->column("hit1_dxCalc",0.);
        tPoint->column("hit1_dyCalc",0.);
        tPoint->column("hit1_dx",0.);
        tPoint->column("hit1_dy",0.);
        tPoint->column("hit1_dz",0.);
        tPoint->column("hit1_dtx",0.);
        tPoint->column("hit1_dty",0.);
        tPoint->column("chip1",0.);
        tPoint->column("pixelX1",0.);
        tPoint->column("pixelY1",0.);
      }
      
      tPoint->column( "x"   , positionForIp.x() );
      tPoint->column( "y"   , positionForIp.y() );
      tPoint->column( "z"   , positionForIp.z() );
      tPoint->column( "tx"  , slopes.x() );
      tPoint->column( "ty"  , slopes.y() );
      tPoint->column( "p"   , momentum );
      tPoint->column( "dp"  , momentum-mcMomentum );
      tPoint->column( "pt"   , transmomentum );
      tPoint->column( "chi2ndof" , tr->chi2PerDoF() );
      tPoint->column( "nhits" , tr->nLHCbIDs () );
      tPoint->column( "mcpt" ,  mcTransMomentum);
      tPoint->column( "ip"   , ip );
      tPoint->column( "Extrapolation_dx"  , position.x() - mcPosition.x() );
      tPoint->column( "Extrapolation_dy"  , position.y() - mcPosition.y() );
      tPoint->column( "firstMeas"  ,firstposition.z()- mcPosition.z() );
      tPoint->column( "Extrapolation_dtx" , slopes.x() - mcSlopes.x() );
      tPoint->column( "Extrapolation_dty" , slopes.y() - mcSlopes.y() );
      tPoint->column( "mctx" , mcSlopes.x() );
      tPoint->column( "mcty" , mcSlopes.y() );
      tPoint->write();

      if ( msgLevel( MSG::DEBUG) ) {
        debug() << format( "Track%4d MC%5d x%8.3f y%8.3f z%8.3f tx%8.3f ty%8.3f",
                           tr->key(), part->key(), position.x(), position.y(), position.z(),
                           slopes.x(), slopes.y() );
        debug() << format( " dx%8.3f dy%8.3f dp/p%8.4f%%",  position.x() - mcPosition.x(),
                           position.y() - mcPosition.y(), 100.*(1.-mcMomentum/momentum) ) << endreq;
      }
      part = link.next();
    }
  }
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode VeloPixChecker::finalize() {

  debug() << "==> Finalize" << endmsg;

  return GaudiTupleAlg::finalize();  // must be called after all other actions
}

//=============================================================================
