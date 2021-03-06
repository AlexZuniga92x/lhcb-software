// Include files 

// from Gaudi
#include "GaudiKernel/PhysicalConstants.h"

// local
#include "STTrackTuple.h"

// track interfaces
#include "TrackInterfaces/IHitExpectation.h"

// kernel
#include "Kernel/ISTChannelIDSelector.h"
#include "Kernel/STLexicalCaster.h"
#include "Kernel/ITDetectorPlot.h"
#include "Kernel/TTDetectorPlot.h"
#include "Kernel/HitPattern.h"
#include "Kernel/Trajectory.h"

// detector
#include "STDet/DeSTDetector.h"
#include "STDet/DeSTSector.h"
#include "STDet/DeTTSector.h"
#include "STDet/DeSTSensor.h"

// Event
#include "Event/STCluster.h"
#include "Event/Measurement.h"
#include "Event/Node.h"
#include "Event/FitNode.h"
#include "Event/VPMeasurement.h"

// AIDA
#include "AIDA/IProfile1D.h"
#include "AIDA/IProfile2D.h"
#include "AIDA/IHistogram2D.h"
#include "AIDA/IHistogram1D.h"

#include "Event/ODIN.h"

#include "LoKi/select.h"

#include <sstream>

#include <cmath>

#include <boost/bind.hpp>
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>

#include <iostream>

// Namespaces
using namespace std;
using namespace LHCb;
using namespace ST;
using namespace AIDA;
using namespace Gaudi;
using namespace boost::assign;

//-----------------------------------------------------------------------------
// Implementation file for class : STTrackTuple
//
// 2009-06-16 : Johan Luisier
// 2010-07-27 : Frederic Dupertuis
// 2015-04-15 : Ilya Komarov and Elena Graverini
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( STTrackTuple )


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
STTrackTuple::STTrackTuple( const std::string& name,
                            ISvcLocator* pSvcLocator)
  : TrackMonitorTupleBase ( name , pSvcLocator ),
    m_totalExpected( 0u ),
    m_totalFound( 0u ),
    m_binSize( 10000. )
{
  declareProperty( "DetType"            , m_detType = "IT" );
  declareProperty( "ClusterCollector"   , m_clustercollectorName = m_detType+"ClusterCollector");  
  declareProperty( "HitExpectation"     , m_expectedHitsToolName = m_detType+"HitExpectation");  
  std::vector<double> tmp = list_of( 300. * Gaudi::Units::um );
  declareProperty( "Cuts"               , m_spacialCuts = tmp );
  declareProperty( "XLayerCut"          , m_xCut = 300. * Gaudi::Units::um );
  declareProperty( "StereoLayerCut"     , m_stereoCut = 300. * Gaudi::Units::um );
  declareProperty( "defaultCut"         , m_defaultCut = 300. * Gaudi::Units::um );
  declareProperty( "MinExpectedSectors" , m_minExpSectors = 1 );
  declareProperty( "MinNbResSectors"    , m_minNbResSectors = -10 );
  declareProperty( "MaxNbResSectors"    , m_maxNbResSectors = 10 );
  declareProperty( "MinDistToEdgeX"     , m_minDistToEdgeX = 2. );
  declareProperty( "MinDistToEdgeY"     , m_minDistToEdgeY = 2. );
  declareProperty( "MinStationPassed"   , m_minStationPassed = 1 );
  declareProperty( "MinCharge"          , m_minCharge = 0. );
  declareProperty( "EfficiencyPlot"     , m_effPlot = true );
  declareProperty( "ResidualsPlot"      , m_resPlot = false );
  //declareProperty( "TakeEveryHit"       , m_everyHit = false);
  declareProperty( "HitsOnTrack"        , m_hitsOnTrack = true );
  declareProperty( "SingleHitPerSector" , m_singlehitpersector = false );
  declareProperty( "EfficiencyMode"     , m_efficiencyMode = true );
  //declareProperty( "BranchByTrack"      , m_branchByTrack = true);
  declareProperty( "SkipEdges"          , m_skipEdges = true );
  declareProperty( "SaveSectorPositions", m_savePositions = false );
}

//=============================================================================
// Destructor
//=============================================================================
STTrackTuple::~STTrackTuple() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode STTrackTuple::initialize()
{
  StatusCode sc( TrackMonitorTupleBase::initialize() ); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by TrackMonitorTupleBase
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;
  
  m_clustercollectorName = m_detType+"ClusterCollector";
  m_expectedHitsToolName = m_detType+"HitExpectation";
  
  m_clustercollector = tool< ISTClusterCollector >( "STClusterCollector", m_clustercollectorName );
  m_expectedHits  = tool< IHitExpectation >( m_detType+"HitExpectation", m_expectedHitsToolName );
  
  m_tracker = getDet< DeSTDetector >( DeSTDetLocation::location( m_detType ) );

  m_NbrOfCuts = m_spacialCuts.size();
  m_foundSector.resize(m_NbrOfCuts);
  
  const DeSTDetector::Sectors& sectors = m_tracker->sectors();

  // Retrieve and print sector positions 
  // (Done only once per job)
  if (m_savePositions)
  {
    Tuple positions = nTuple("SectorPositions");
    std::vector<DeSTSensor*> sensorsInSector;
    BOOST_FOREACH(DeSTSector * sector, sectors)
    {
      // Printouts
      //std::cout << sector->nickname() << "\t" << int( sector->elementID() ) << "\t\t" ;
      //Gaudi::XYZPoint pos = sector->toGlobal( Gaudi::XYZPoint(0.,0.,0.) );
      // std::cout << pos.x() << " " << pos.y() << " " << pos.z() << std::endl;

      positions -> column( "sectorID", int( sector->elementID() ) );
      sensorsInSector = sector -> sensors();
      std::vector<float> sensor_x;     sensor_x.reserve( sensorsInSector.size() );
      std::vector<float> sensor_y;     sensor_y.reserve( sensorsInSector.size() );
      std::vector<float> sensor_z;     sensor_z.reserve( sensorsInSector.size() );
      std::vector<float> corner1_x;    corner1_x.reserve( sensorsInSector.size() );
      std::vector<float> corner1_y;    corner1_y.reserve( sensorsInSector.size() );
      std::vector<float> corner1_z;    corner1_z.reserve( sensorsInSector.size() );
      std::vector<float> corner2_x;    corner2_x.reserve( sensorsInSector.size() );
      std::vector<float> corner2_y;    corner2_y.reserve( sensorsInSector.size() );
      std::vector<float> corner2_z;    corner2_z.reserve( sensorsInSector.size() );
      std::vector<float> corner3_x;    corner3_x.reserve( sensorsInSector.size() );
      std::vector<float> corner3_y;    corner3_y.reserve( sensorsInSector.size() );
      std::vector<float> corner3_z;    corner3_z.reserve( sensorsInSector.size() );
      std::vector<float> corner4_x;    corner4_x.reserve( sensorsInSector.size() );
      std::vector<float> corner4_y;    corner4_y.reserve( sensorsInSector.size() );
      std::vector<float> corner4_z;    corner4_z.reserve( sensorsInSector.size() );
      std::vector<DeSTSensor*>::const_iterator sensor = sensorsInSector.begin();
      for(; sensor != sensorsInSector.end(); ++sensor)
      {
        // Sensor centre
        Gaudi::XYZPoint pos = (*sensor)->globalCentre();//toGlobal( Gaudi::XYZPoint(0.,0.,0.) );
        sensor_x.push_back( float(pos.x()) );
        sensor_y.push_back( float(pos.y()) );
        sensor_z.push_back( float(pos.z()) );
        // Sensor corners
        std::unique_ptr<LHCb::Trajectory> firsttraj = (*sensor)->trajectory(1, 0.);
        Gaudi::XYZPoint corner1 = firsttraj->beginPoint();
        Gaudi::XYZPoint corner2 = firsttraj->endPoint();
        std::unique_ptr<LHCb::Trajectory> lasttraj = (*sensor)->trajectory(512, 0.);
        Gaudi::XYZPoint corner3 = lasttraj->beginPoint();
        Gaudi::XYZPoint corner4 = lasttraj->endPoint();
        corner1_x.push_back( float(corner1.x()) );
        corner1_y.push_back( float(corner1.y()) );
        corner1_z.push_back( float(corner1.z()) );
        corner2_x.push_back( float(corner2.x()) );
        corner2_y.push_back( float(corner2.y()) );
        corner2_z.push_back( float(corner2.z()) );
        corner3_x.push_back( float(corner3.x()) );
        corner3_y.push_back( float(corner3.y()) );
        corner3_z.push_back( float(corner3.z()) );
        corner4_x.push_back( float(corner4.x()) );
        corner4_y.push_back( float(corner4.y()) );
        corner4_z.push_back( float(corner4.z()) );
      }
      positions -> farray( "sensor_x",  sensor_x, "len",  sensorsInSector.size() );
      positions -> farray( "sensor_y",  sensor_y, "len",  sensorsInSector.size() );
      positions -> farray( "sensor_z",  sensor_z, "len",  sensorsInSector.size() );
      positions -> farray( "corner1_x", corner1_x, "len",  sensorsInSector.size() );
      positions -> farray( "corner1_y", corner1_y, "len",  sensorsInSector.size() );
      positions -> farray( "corner1_z", corner1_z, "len",  sensorsInSector.size() );
      positions -> farray( "corner2_x", corner2_x, "len",  sensorsInSector.size() );
      positions -> farray( "corner2_y", corner2_y, "len",  sensorsInSector.size() );
      positions -> farray( "corner2_z", corner2_z, "len",  sensorsInSector.size() );
      positions -> farray( "corner3_x", corner3_x, "len",  sensorsInSector.size() );
      positions -> farray( "corner3_y", corner3_y, "len",  sensorsInSector.size() );
      positions -> farray( "corner3_z", corner3_z, "len",  sensorsInSector.size() );
      positions -> farray( "corner4_x", corner4_x, "len",  sensorsInSector.size() );
      positions -> farray( "corner4_y", corner4_y, "len",  sensorsInSector.size() );
      positions -> farray( "corner4_z", corner4_z, "len",  sensorsInSector.size() );
      positions -> write();
    } // end loop (sectors)
  }

  DeSTDetector::Sectors::const_iterator iterS = sectors.begin();
  for ( ; iterS != sectors.end(); ++iterS)
  {
    STChannelID elemID = (*iterS)->elementID();
    m_expectedSector[elemID.uniqueSector()] = 0u;
    m_nameMapSector[elemID.uniqueSector()] = *iterS;
    for (unsigned int i=0 ; i < m_NbrOfCuts; ++i )
    {
      m_foundSector[i][elemID.uniqueSector()] = 0u; 
    }
  } // end loop (iterS)

  unsigned int iCut;
  for (iCut = 0; iCut < m_NbrOfCuts; ++iCut)
  {
    if ( std::abs( m_xCut - m_spacialCuts[iCut] ) < .005 )
      m_whichCut[0] = iCut;
    if ( std::abs( m_stereoCut -  m_spacialCuts[iCut]) < .005 )
      m_whichCut[1] = iCut;
  } // end loop (iCut)

  for(iCut = 1; iCut < m_NbrOfCuts; iCut++)
  {
    m_binSize = std::min( m_binSize,
                          m_spacialCuts[ iCut ] - m_spacialCuts[ iCut - 1 ] );
  }
  //m_binSize = std::min( m_binSize, .01 );
  //m_binSize = std::max( m_binSize, .01 );
  m_binNumber = static_cast<unsigned int>( (m_spacialCuts.back() -
                                            m_spacialCuts.front()) / m_binSize );
  m_binNumber++;
  
  return StatusCode::SUCCESS;
} // STTrackTuple::initialize()




//=============================================================================
// Main execution
//=============================================================================
StatusCode STTrackTuple::execute()
{
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  //const DeSTDetector::Sectors& sectors = m_tracker->sectors();

  // Book ntuple
  Tuple tup_test = nTuple ("TrackMonTuple") ;
  
  // Read tracks
  Tracks* tracks = get< LHCb::Tracks >( inputContainer() );

  Tracks::const_iterator It, Begin( tracks -> begin() ),
    End( tracks -> end() );

  std::vector<LHCbID> expectedHits;
  std::vector<unsigned int> expectedSectors;
  std::vector<unsigned int> existingSectors;
  std::vector<unsigned int> foundSectors;
  std::vector<unsigned int>::iterator itFoundSectors;
  ISTClusterCollector::Hits prefoundHits;
  ISTClusterCollector::Hits foundHits;
  StatusCode inSensor;
  bool inActiveSensor;
  unsigned int nbFoundHits,nbExpHits,nbExpSectors,nbFoundSectors;
  int nbResHits,nbResSectors;
  State trackState;
  // if computing hit efficiency use only active regions
  Gaudi::XYZPoint toleranceOnEdge(- std::max(0.,m_minDistToEdgeX), - std::max(0.,m_minDistToEdgeY), 0.);
  // otherwise keep the whole sensor surface
  if (!m_skipEdges)
  {
    toleranceOnEdge = Gaudi::XYZPoint(0., 0., 0.);
  }

  DeSTSector* sector = 0;
  std::vector<DeSTSensor*> sensors;

  // Loop on tracks
  for (It = Begin; It != End; It++)
  {
    expectedHits.clear();
    prefoundHits.clear();
    foundHits.clear();
    expectedSectors.clear();
    foundSectors.clear();

    // ask for tracks crossing enough stations
    if( ! hasMinStationPassed( (*It) ) )
      continue;
        
    // collect the list of expected hits
    m_expectedHits -> collect( **It, expectedHits );

    // remove tracks with too few expected hits
    nbExpHits = expectedHits.size();
    if ( nbExpHits < m_minExpSectors ) 
      continue; 

    // collect the list of found hits
    m_clustercollector -> execute( **It, prefoundHits );
    nbFoundHits = prefoundHits.size();
    
    // Number of hits not found
    nbResHits = (int)(nbFoundHits) - (int)(nbExpHits);
    
    if(fullDetail()){
      plot(nbFoundHits, "Control/NbFoundHits","Number of found hits",-0.5,24.5,25);
      plot(nbExpHits,   "Control/NbExpHits",  "Number of expected hits",-0.5,24.5,25);
      plot(nbResHits,   "Control/NbResHits",  "Number of found - expected hits",-5.5,7.5,13);
    }
        
    // convert the expected hits into expected sectors, cutting, sensor edges if m_skipEdges=true
    BOOST_FOREACH(LHCbID id, expectedHits)
    {
      if( m_minDistToEdgeX > 0. || m_minDistToEdgeY > 0.){
        sector = m_tracker -> findSector( id.stID() );
        //info()<<"Sector is "<<sector<<endl;
        if ( sector == 0 ){
          info() << "Should not happen !" << endmsg;
          continue;
        }
        sensors = sector -> sensors();
        inActiveSensor = false;
        BOOST_FOREACH(DeSTSensor* sensor, sensors)
        {
          inSensor = extrapolator() -> propagate( (**It), sensor -> plane(), trackState, sensor -> thickness() );
          if( inSensor.isSuccess() ){
            inActiveSensor = sensor -> globalInActive(trackState.position(), toleranceOnEdge) ;
            if( inActiveSensor ) break;
          }
        } // end loop (find active sensor)
        if( inActiveSensor )
        {
          expectedSectors.push_back(id.stID().uniqueSector());
        }
      }// end if (m_minDistToEdgeX > 0. || m_minDistToEdgeY > 0.)
    }// end loop (convert the expected hits into expected sectors, cutting sensor edges)
    nbExpSectors = expectedSectors.size();

    // remove tracks with too few expected hits
    if ( nbExpSectors < m_minExpSectors ) 
      continue; 
        
    // Fill foundHits and foundSectors
    BOOST_FOREACH(ISTClusterCollector::Hit hit, prefoundHits)
    {
      // if the hit sector is in the list of expected sectors
      if(find( expectedSectors.begin(), expectedSectors.end(), hit.cluster->channelID().uniqueSector() ) != expectedSectors.end()){
        // push hit in foundHits and sector in foundSectors, if not already there
        if(find(foundSectors.begin(), foundSectors.end(), hit.cluster->channelID().uniqueSector()) == foundSectors.end()){
          foundHits.push_back(hit);
          foundSectors.push_back(hit.cluster->channelID().uniqueSector());
        }else if( !m_singlehitpersector )
          {foundHits.push_back(hit);} // push hit in foundHits anyways
        else
          continue;
      }
    }

    nbExpHits = nbExpSectors;
    nbFoundHits = foundHits.size();
    nbResHits = (int)(nbFoundHits) - (int)(nbExpHits);
    
    nbFoundSectors = foundSectors.size();
    nbResSectors = (int)(nbFoundSectors) - (int)(nbExpSectors);
    
    // remove tracks with too many (found - expected) hits
    if ( nbResHits > m_maxNbResSectors ) {
      if (msgLevel(MSG::DEBUG)) debug() << "Possible track expectation mismatch";
      continue;
    }
    
    // remove tracks with too few (found - expected) sectors
    if ( nbResSectors < m_minNbResSectors ) {
      if (msgLevel(MSG::DEBUG)) debug() << "Possible track expectation mismatch";
      continue;
    }
    
    if(fullDetail()){
      plot(nbFoundHits, "Control/NbFoundHitsFinal","Number of found hits after full selection",-0.5,24.5,25);
      plot(nbExpHits, "Control/NbExpHitsFinal","Number of expected hits after full selection",-0.5,24.5,25);
      plot(nbResHits, "Control/NbResHitsFinal","Number of found - expected hits after full selection",-5.5,7.5,13);
      plot(nbFoundSectors, "Control/NbFoundSectorsFinal","Number of found sectors after full selection",-0.5,24.5,25);
      plot(nbExpSectors, "Control/NbExpSectorsFinal","Number of expected sectors after full selection",-0.5,24.5,25);
      plot(nbResSectors, "Control/NbResSectorsFinal","Number of found - expected sectors after full selection",-5.5,7.5,13);
    }
        
    std::vector<unsigned int>::iterator iterExp = expectedSectors.begin();

    // Count tracks
    tup_test -> column ("TrackCounter", 1);
    // Run number
    if( exist<LHCb::ODIN>(LHCb::ODINLocation::Default) )
    {
      LHCb::ODIN* report = get<LHCb::ODIN>(LHCb::ODINLocation::Default);
      if ( ! tup_test->column( "RunNumber", report->runNumber() ) ) return StatusCode::FAILURE;
      if (msgLevel(MSG::DEBUG)) debug() << "Run number:  " << report->runNumber() << endreq;
    }

    // Add track information to the tuple
    tup_test->column("p",        (*It)->p());
    tup_test->column("pt",       (*It)->pt());
    tup_test->column("phi",      (*It)->phi());
    tup_test->column("eta",      (*It)->pseudoRapidity());
    tup_test->column("charge",   (*It)->charge());
    tup_test->column("nDoF",     (*It)->nDoF());
    tup_test->column("chi2",     (*It)->chi2());
    tup_test->column("probChi2", (*It)->probChi2());
    // Number of ST hits on track
    unsigned int numTTHits = 0;
    unsigned int numITHits = 0;
    // Add vectors of clusters information to the ntuple
    unsigned int maxhits = 30; // default length of vectors
    std::vector<unsigned int> clusterSize;   clusterSize.reserve(maxhits);
    std::vector<int> clusterStrip;           clusterStrip.reserve(maxhits);
    std::vector<double> clusterCharge;       clusterCharge.reserve(maxhits);
    std::vector<double> clusterNoise;        clusterNoise.reserve(maxhits);
    std::vector<double> clusterISfrac;       clusterISfrac.reserve(maxhits);
    std::vector<float> cluster_x;            cluster_x.reserve(maxhits);
    std::vector<float> cluster_y;            cluster_y.reserve(maxhits);
    std::vector<float> cluster_z;            cluster_z.reserve(maxhits);
    std::vector<float> trackState_x;         trackState_x.reserve(maxhits);
    std::vector<float> trackState_y;         trackState_y.reserve(maxhits);
    std::vector<float> trackState_z;         trackState_z.reserve(maxhits);
    std::vector<int> clusterSTchanID;        clusterSTchanID.reserve(maxhits);
    std::vector<int> clusterSTchanMapID;     clusterSTchanMapID.reserve(maxhits);
    std::vector<double> hit_residual;        hit_residual.reserve(maxhits);
    std::vector<double> traj_mu;             traj_mu.reserve(maxhits);


    if (!m_efficiencyMode)
    {
      // Use only hits from tracks
      if (m_hitsOnTrack)
      {
        // Add: residual error, measurement error
        std::vector<double> hit_errMeasure;      hit_errMeasure.reserve(maxhits);
        std::vector<double> hit_errResidual;     hit_errResidual.reserve(maxhits);
        // Vector of measurements on the track
        std::vector<const LHCb::STMeasurement*> measVector; measVector.reserve(maxhits);
        // Read the nodes on the track
        LHCb::Track::ConstNodeRange nodes = (*It) -> nodes();
        LHCb::Track::ConstNodeRange::const_iterator iNodes = nodes.begin();
        double counter = 1;
        for ( ; iNodes != nodes.end(); ++iNodes )
        {
          std::cout<<"Node number "<<counter<<std::endl;
          counter+=1;
          const LHCb::FitNode* fNode = dynamic_cast<const LHCb::FitNode*>(*iNodes);
          if ( fNode->hasMeasurement() == false ) continue;
          if ( (*iNodes)->type() != LHCb::Node::HitOnTrack ) continue;
          if ( (*iNodes)->errResidual2() <= TrackParameters::lowTolerance ) continue;
          // Count number of TT and IT hits
          if ( fNode->measurement().type() != LHCb::Measurement::TT ) numTTHits++;
          if ( fNode->measurement().type() != LHCb::Measurement::IT ) numITHits++;
          // One detector at a time
          //if ( m_detType == "TT" && fNode->measurement().type() != LHCb::Measurement::TT ) continue;
          //if ( m_detType == "IT" && fNode->measurement().type() != LHCb::Measurement::IT ) continue;
          if ( m_detType == "TT" && !(fNode->measurement().type() == LHCb::Measurement::TT  || fNode->measurement().type() == LHCb::Measurement::TTLite)) continue;
          if ( m_detType == "IT" && !(fNode->measurement().type() == LHCb::Measurement::IT  || fNode->measurement().type() == LHCb::Measurement::ITLite)) continue;
          // Retrieve the measurement from the track node
          const STMeasurement* aHit = dynamic_cast<const STMeasurement*>(&fNode->measurement());
          measVector.push_back(aHit);
          // Fill residual and residual error
          hit_residual.push_back( (*iNodes)->residual() );
          hit_errResidual.push_back( (*iNodes)->errResidual() );
          // Fill position of the track state
          const LHCb::State & aState = (*iNodes)->state();
          trackState_x.push_back( (float)(aState.x()) );
          trackState_y.push_back( (float)(aState.y()) );
          trackState_z.push_back( (float)(aState.z()) );
          // Retrieve the cluster from the measurement
          const LHCb::STCluster * aCluster = aHit->cluster();
          // Fill cluster ID, size, strip, charge, interstrip fraction, noise
          clusterSTchanID.push_back( (int)(aCluster->channelID()) );
          if ( m_detType == "TT" )
          {
            clusterSTchanMapID.push_back( TThistoBin(aCluster->channelID()) );
          }
          else
          {
            clusterSTchanMapID.push_back( IThistoBin(aCluster->channelID()) ); 
          }
          clusterSize.push_back(   aCluster->size() );
          clusterStrip.push_back(  aCluster->strip() );
          clusterCharge.push_back( aCluster->totalCharge() );
          double interStripFrac = (aHit->cluster())->interStripFraction();
          clusterISfrac.push_back( interStripFrac );
          const DeSTSector* sector = m_tracker->findSector(aCluster->channelID());
          double noise = sector->noise(aCluster->channelID()); 
          clusterNoise.push_back( noise );
          // Find the position of the charge deposit on the hit strip
          std::unique_ptr<LHCb::Trajectory> traj = m_tracker -> trajectory( aCluster->channelID(), interStripFrac );
          double mu = traj.get()->muEstimate(aState.position());
          LHCb::Trajectory::Point midStripPosition = traj.get()->position(mu);
          traj_mu.push_back(mu);
          //std::cout << "at mu = " << mu << ": " << midStripPosition.z() << ", state is at " << aState.z() << std::endl;
          // Fill position of the charge deposit
          cluster_x.push_back( (float)(midStripPosition.x()) );
          cluster_y.push_back( (float)(midStripPosition.y()) );
          cluster_z.push_back( (float)(midStripPosition.z()) );
          // Fill measurement error
          hit_errMeasure.push_back( aHit->errMeasure() );
        } // end loop (iNodes)

        // Count the number of hits on each ST layer
        const DeSTDetector::Layers& layers = m_tracker->layers();
        BOOST_FOREACH(DeSTLayer * layer, layers)
        {
          unsigned int nHitsOnLayer = hitsOnLayer( measVector, layer );
          tup_test -> column(layer->nickname()+"_nHits", nHitsOnLayer);
        } // end loop (layers)

        // Fill information available only in "hits on track" mode
        tup_test -> farray( "trackState_x",    trackState_x, "len",    maxhits );
        tup_test -> farray( "trackState_y",    trackState_y, "len",    maxhits );
        tup_test -> farray( "trackState_z",    trackState_z, "len",    maxhits );
        tup_test -> farray( "hit_errMeasure",  hit_errMeasure, "len",  maxhits );
        tup_test -> farray( "hit_errResidual", hit_errResidual, "len", maxhits );
      }// end if (hits on track)

      // Use hits found by the cluster collector
      // Efficiency-mode-like. But no itearation over expected sectors
      else
      {
        // Count the number of hits on each ST layer
        const DeSTDetector::Layers& layers = m_tracker->layers();
        BOOST_FOREACH(DeSTLayer * layer, layers)
        {
          unsigned int nHitsOnLayer = hitsOnLayer( foundHits, layer );
          tup_test -> column(layer->nickname()+"_nHits", nHitsOnLayer);
        } // end loop (layers)

        // Loop on the found hits
        BOOST_FOREACH(ISTClusterCollector::Hit aHit, foundHits)
        {
          // Count the number of TT and IT hits
          if ( aHit.cluster->isTT() ) numTTHits++;
          if ( aHit.cluster->isIT() ) numITHits++;
          // Fill vectors of cluster information
          if ( (aHit.cluster->isTT() && m_detType == "TT") || (aHit.cluster->isIT() && m_detType == "IT") )
          {
            clusterSTchanID.push_back( (int)(aHit.cluster->channelID()) );
            if ( m_detType == "TT" )
            {
              clusterSTchanMapID.push_back( TThistoBin(aHit.cluster->channelID()) );
            }
            else
            {
              clusterSTchanMapID.push_back( IThistoBin(aHit.cluster->channelID()) ); 
            }
            clusterSize.push_back( aHit.cluster->size() );
            clusterStrip.push_back( aHit.cluster->strip() );
            clusterCharge.push_back( aHit.cluster->totalCharge() );
            double interStripFrac = aHit.cluster->interStripFraction();
            clusterISfrac.push_back( interStripFrac );
            const DeSTSector* sector = m_tracker->findSector(aHit.cluster->channelID());
            double noise = sector->noise(aHit.cluster->channelID()); 
            clusterNoise.push_back( noise );
            // Find the closest state on the track
            DeSTSensor* sensor = findSensor(sector, aHit.cluster->channelID());
            const LHCb::State & aState = (*It)->closestState( sensor->plane() ); // closestState works only with planes!
            // Fill the position of the found state
            trackState_x.push_back( (float)(aState.x()) );
            trackState_y.push_back( (float)(aState.y()) );
            trackState_z.push_back( (float)(aState.z()) );
            // Find the position of the charge deposit on the hit strip
            std::unique_ptr<LHCb::Trajectory> traj = m_tracker -> trajectory( aHit.cluster->channelID(), interStripFrac );
            double mu = traj.get()->muEstimate(aState.position());
            traj_mu.push_back(mu);
            LHCb::Trajectory::Point hitPosition = traj.get()->position(mu);
            cluster_x.push_back( (float)(hitPosition.x()) );
            cluster_y.push_back( (float)(hitPosition.y()) );
            cluster_z.push_back( (float)(hitPosition.z()) );
            hit_residual.push_back( aHit.residual );
          } // end if (one detector at a time)
        } // end loop (foundHits)

        // Fill information available only in "every hit" mode
        tup_test -> farray( "closestTrackState_x",    trackState_x, "len", maxhits );
        tup_test -> farray( "closestTrackState_y",    trackState_y, "len", maxhits );
        tup_test -> farray( "closestTrackState_z",    trackState_z, "len", maxhits );
      } // end if (every hit)
    } 

    // Efficiency mode
    else
    {
      std::vector<bool> isExpected;  isExpected.reserve(maxhits);
      std::vector<bool> isFound;     isFound.reserve(maxhits);
      std::vector<int> sectorSTchanMapID;        sectorSTchanMapID.reserve(maxhits);
      // Count the number of hits on each ST layer
      for ( ; iterExp != expectedSectors.end(); ++iterExp)
      { 
        ++m_totalExpected;
        ++m_expectedSector[*iterExp];
        DeSTSector* isector = m_nameMapSector[*iterExp];
        isExpected.push_back(true);
        bool HitWasFound = false;
        if ( m_detType == "TT" ){ sectorSTchanMapID.push_back( TThistoBin(isector->elementID()) );}
        else {sectorSTchanMapID.push_back( IThistoBin(isector->elementID()) ); }
        BOOST_FOREACH(ISTClusterCollector::Hit aHit, foundHits)
        {
          if ( aHit.cluster->channelID().uniqueSector() == *iterExp )
          {
            HitWasFound = true;
            if ( aHit.cluster->isTT() ) numTTHits++;
            if ( aHit.cluster->isIT() ) numITHits++;
            isFound.push_back(true);
            clusterSTchanID.push_back( (int)(aHit.cluster->channelID()) );
            if ( m_detType == "TT" )
            {clusterSTchanMapID.push_back( TThistoBin(aHit.cluster->channelID()) );}
            else
            {clusterSTchanMapID.push_back( IThistoBin(aHit.cluster->channelID()) );}
            clusterSize.push_back( aHit.cluster->size() );
            clusterStrip.push_back( aHit.cluster->strip() );
            clusterCharge.push_back( aHit.cluster->totalCharge() );
            double interStripFrac = aHit.cluster->interStripFraction();
            clusterISfrac.push_back( interStripFrac );
            const DeSTSector* sector = m_tracker->findSector(aHit.cluster->channelID());
            double noise = sector->noise(aHit.cluster->channelID()); 
            clusterNoise.push_back( noise );
            DeSTSensor* sensor = findSensor(sector, aHit.cluster->channelID());
            const LHCb::State & aState = (*It)->closestState( sensor->plane() ); // closestState works only with planes!
            // Find the position of the charge deposit on the hit strip
            std::unique_ptr<LHCb::Trajectory> traj = m_tracker -> trajectory( aHit.cluster->channelID(), interStripFrac );
            double mu = traj.get()->muEstimate(aState.position());
            traj_mu.push_back(mu);
            LHCb::Trajectory::Point hitPosition = traj.get()->position(mu);
            cluster_x.push_back( (float)(hitPosition.x()) );
            cluster_y.push_back( (float)(hitPosition.y()) );
            cluster_z.push_back( (float)(hitPosition.z()) );
            hit_residual.push_back( aHit.residual );
            continue;
          }
        }
        if (!HitWasFound)
        {
          isFound.push_back(false);
          clusterSTchanID.push_back( (int)(isector->elementID()) );
          if ( m_detType == "TT" )
          {clusterSTchanMapID.push_back( TThistoBin(isector->elementID()) );}
          else
          {clusterSTchanMapID.push_back( IThistoBin(isector->elementID()) );}
          clusterSize.push_back( 0 );
          clusterStrip.push_back( 0 );
          clusterCharge.push_back( 0 );
          clusterISfrac.push_back( 0 );
          clusterNoise.push_back( 0 );
          traj_mu.push_back(0);
          cluster_x.push_back( 0 );
          cluster_y.push_back( 0 );
          cluster_z.push_back( 0 );
          hit_residual.push_back( 0 );
        }          
      }
    tup_test -> farray( "isExpected",               isExpected,          "len",  maxhits );
    tup_test -> farray( "isFound",                  isFound,             "len",  maxhits );
    tup_test -> farray( "sectorSTchanMapID",        sectorSTchanMapID,   "len",  maxhits );
    } // end if (branchBySector)


    // Fill information  
    tup_test -> farray( "clusterSTchanID",    clusterSTchanID,      "len",  maxhits );
    tup_test -> farray( "clusterSTchanMapID", clusterSTchanMapID,   "len",  maxhits );
    tup_test -> farray( "clusterSize",        clusterSize,          "len",  maxhits );
    tup_test -> farray( "clusterStrip",       clusterStrip,         "len",  maxhits );
    tup_test -> farray( "clusterCharge",      clusterCharge,        "len",  maxhits );
    tup_test -> farray( "clusterNoise",       clusterNoise,         "len",  maxhits );
    tup_test -> farray( "clusterISfrac",      clusterISfrac,        "len",  maxhits );
    tup_test -> farray( "traj_mu",            traj_mu,              "len",  maxhits );
    tup_test -> farray( "cluster_x",          cluster_x,            "len",  maxhits );
    tup_test -> farray( "cluster_y",          cluster_y,            "len",  maxhits );
    tup_test -> farray( "cluster_z",          cluster_z,            "len",  maxhits );
    tup_test -> farray( "hit_residual",       hit_residual,         "len",  maxhits );
    tup_test->column("numIThits", numITHits);
    tup_test->column("numTThits", numTTHits);


    // Write the ntuple
    tup_test -> write () ;
  } // end loop (tracks)

  return StatusCode::SUCCESS;
} // STTrackTuple::execute()


//=============================================================================
//  Finalize
//=============================================================================
StatusCode STTrackTuple::finalize()
{
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;
  
  if ( (m_totalExpected == 0) && (m_efficiencyMode) )
  {
    warning() << "I'm sorry, but no track was found, please check that"
              << " you selected the right track type." << std::endl
              << "Your selection was : ";
    for (unsigned int i( 0 ); i < m_wantedTypes.size(); i++)
      warning() << m_wantedTypes[ i ] << ' ';
    warning() << endmsg;
    return TrackMonitorTupleBase::finalize();
  }
  
  return TrackMonitorTupleBase::finalize();  // must be called after all other actions
} // STTrackTuple::finalize()


/**
 * Stupid function that allows to print the number with 2 decimals,
 * since using std::setprecision and std::fixed directly in the info()
 * screws everythings up.
 *
 * @param nbr number (double) to print
 * @param digits number of wanted decimal (2 is the default)
 *
 * @return a string with the correct number of decimals.
 */
std::string STTrackTuple::formatNumber(const double& nbr,
                                       const unsigned int& digits ) const
{
  std::stringstream ss;
  ss.setf(std::ios::fixed, std::ios::floatfield);
  ss << std::setprecision( digits ) << nbr;
  return ss.str();
} // STTrackTuple::formatNumber()


/**
 * Checks whether a track crosses enough stations.
 *
 * @return \e true if yes.
 */
bool STTrackTuple::hasMinStationPassed(LHCb::Track* const& aTrack) const
{
  std::set<unsigned int> stations;
  const std::vector<LHCb::LHCbID>& ids = aTrack -> lhcbIDs();
  for (std::vector<LHCb::LHCbID>::const_iterator iter = ids.begin(); iter != ids.end(); ++iter){
    if (m_detType == "IT"){
      if (iter->isIT() == true ) 
        stations.insert( iter->stID().station() );
    }else{
      if (iter->isTT() == true ) 
        stations.insert( iter->stID().station() );
    }
  }
  if(fullDetail())
    plot(stations.size(), "Control/NbCrossedStation","Number of stations crossed by the track",-0.5,3.5,4);
  if(stations.size() >= m_minStationPassed)
    return true;
  return false;
} // STTrackTuple::hasMinStationPassed()


/**
 * Find to what sensor of the given sector a strip belongs.
 *
 * @param sector sector to which the strip belongs
 * @param id STChannelID of the strip
 *
 * @return the sensor.
 */
DeSTSensor* STTrackTuple::findSensor(const DeSTSector* sector,
                                    STChannelID id)
{
  std::vector<DeSTSensor*> sensors = sector->sensors();
  DeSTSensor* foundSensor = new DeSTSensor();
  for (std::vector<DeSTSensor*>::const_iterator iter = sensors.begin(); iter != sensors.end(); ++iter)
  {
    if ( (*iter)->contains(id) )
    {
      foundSensor = (*iter);
    }
  }
  return foundSensor;
} // STTrackTuple::findSensor()


/**
 * Check if a layer was crossed by a track
 *
 * @param aLayer handle to the DeSTlayer
 *
 * @return true if yes.
 */
bool STTrackTuple::crossedLayer(LHCb::Track* const& aTrack,
                                DeSTLayer * &aLayer) const
{
  const std::vector<LHCb::LHCbID>& ids = aTrack -> lhcbIDs();
  for (std::vector<LHCb::LHCbID>::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
  {
    if ( aLayer->contains( iter->stID() ) )
      return true;
    //unsigned int theLayer = (iter->stID()).uniqueLayer();
    //if (theLayer == aLayer)
    //  return true;
  }
  return false;
} // STTrackTuple::crossedLayer()


/**
 * Counts the number of hits on a layer
 *
 * @param measVector vector of measurements on the track
 * @param aLayer handle to the DeSTlayer
 *
 * @return number of hits on the layer.
 */
unsigned int STTrackTuple::hitsOnLayer(std::vector<const LHCb::STMeasurement*> measVector,
                                     DeSTLayer * &aLayer) const
{
  unsigned int counter = 0;
  for (std::vector<const LHCb::STMeasurement*>::const_iterator iter = measVector.begin(); iter != measVector.end(); ++iter)
  {
    if ( aLayer->contains( (*iter)->channelID() ) )
      counter = counter + 1;
  }
  return counter;
} // STTrackTuple::hitsOnLayer()


/**
 * Counts the number of hits on a layer
 *
 * @param hits hits found by STClusterCollector
 * @param aLayer handle to the DeSTlayer
 *
 * @return number of hits on the layer.
 */
unsigned int STTrackTuple::hitsOnLayer(ISTClusterCollector::Hits hits,
                                    DeSTLayer * &aLayer) const
{
  unsigned int counter = 0;
  BOOST_FOREACH(ISTClusterCollector::Hit aHit, hits)
  {
    if ( aLayer->contains( (aHit.cluster)->channelID() ) )
      counter = counter + 1;
  }
  return counter;
} // STTrackTuple::hitsOnLayer()


/**
 * Find the LHCbID of an STMeasurement
 *
 * @param aTrack handle to the track
 * @param aHit an STMeasurement
 *
 * @return the LHCbID.
 */
LHCb::LHCbID STTrackTuple::findHitId(LHCb::Track* const& aTrack,
                                    const LHCb::STMeasurement* aHit) const
{
  LHCb::LHCbID hitId(0);
  const std::vector<LHCb::LHCbID>& ids = aTrack -> lhcbIDs();
  for (std::vector<LHCb::LHCbID>::const_iterator iter = ids.begin(); iter != ids.end(); ++iter)
  {
    if ( (aHit->cluster())->channelID() == iter->stID() )
      hitId = (*iter);
  }
  return hitId;
} // STTrackTuple::findHitId()


// Hit type namespace
namespace {
  enum HitType {VeloR=0, VeloPhi, VPX, VPY, TT, IT, OT, Muon, HitTypeUnknown} ;
  const std::string HitTypeName[] = {"VeloR","VeloPhi","VPX","VPY","TT","IT","OT","Muon"} ;
  //double      HitTypeMaxRes[] = {0.1,0.1,0.1,0.1,0.5,0.5,2.0,10} ;
  
  inline HitType hittypemap( const LHCb::Measurement& meas ) {
    LHCb::Measurement::Type type = meas.type() ;

    HitType rc = HitTypeUnknown ;
    switch( type ) {
    case LHCb::Measurement::Calo:
    case LHCb::Measurement::Origin :
    case LHCb::Measurement::Unknown: rc = HitTypeUnknown ; break ;
      
    case LHCb::Measurement::VP:
      {
        const LHCb::VPMeasurement* vpmeas = dynamic_cast<const LHCb::VPMeasurement*>(&meas);
        rc = vpmeas && vpmeas->projection() == LHCb::VPMeasurement::X ? VPX : VPY;
      }
      break;
    case LHCb::Measurement::VeloLiteR:
    case LHCb::Measurement::VeloR:   rc = VeloR ; break ;

    case LHCb::Measurement::VeloLitePhi:
    case LHCb::Measurement::VeloPhi: rc = VeloPhi ; break ;

    case LHCb::Measurement::UT :
    case LHCb::Measurement::UTLite :
    case LHCb::Measurement::TTLite:
    case LHCb::Measurement::TT: rc = TT; break;

    case LHCb::Measurement::FT :
    case LHCb::Measurement::ITLite:
    case LHCb::Measurement::IT: rc = IT; break;

    case LHCb::Measurement::OT: rc = OT; break;

    case LHCb::Measurement::Muon: rc = Muon; break;
    }
    return rc;
  }
} // end (namespace)


/**
 * Find the error on the residual of a hit
 *
 * @param aTrack handle to the track
 * @param aID the LHCbID of the hit strip
 *
 * @return the square of the residual error.
 */
double STTrackTuple::retrieveErrResidual2(LHCb::Track* const& aTrack,
                                        LHCb::LHCbID aID) const
{
  double errResidual2(0);
  HitType mtype = VeloR; // initialize to avoid compiler warning
  LHCb::Track::ConstNodeRange nodes = aTrack -> nodes();
  for( LHCb::Track::ConstNodeRange::const_iterator inode = nodes.begin() ;
        inode != nodes.end(); ++inode)
  {
    if( (*inode)->type() == LHCb::Node::HitOnTrack 
        // discard extremely small fraction of hits with zero error
        // on residual. (e.g. a downstream track with only one
        // active TT hit)
        && (*inode)->errResidual2() > TrackParameters::lowTolerance 
        && (mtype = hittypemap( (*inode)->measurement() ) )!=HitTypeUnknown )
    {
      if ( (*inode)->measurement().lhcbID() == aID )
      {
        errResidual2 = (*inode)->errResidual2();
      }// end if (node is associated to the current measurement)
    }// end if (node not discarded)
  }// end loop on nodes
  return errResidual2;
} // STTrackTuple::retrieveErrResidual2

/**
ChanelID->number converters, copy from STEfficiency. May be can be done smarter.
**/
unsigned int STTrackTuple::TThistoBin(const LHCb::STChannelID& chan) const {

  // convert sector to a flat number (300 in total)
  return (chan.station()-1)*26*3*2 + (chan.layer()-1)*26*3 + (chan.detRegion()-1)*26 + (chan.sector() -1);
  }

unsigned int STTrackTuple::IThistoBin(const LHCb::STChannelID& chan) const {
  // convert sector to a flat number (336 in total)
  return (chan.station()-1)*7*4*4 + (chan.layer()-1)*7*4 + (chan.detRegion()-1)*7 + (chan.sector() -1);
}



//=============================================================================
