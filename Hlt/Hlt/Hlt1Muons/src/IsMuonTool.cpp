// $Id: $
// STL
#include <set>

// boost
#include <boost/foreach.hpp>

// from Gaudi
#include <GaudiKernel/ToolFactory.h>
#include <DetDesc/Condition.h>

// LHCb
#include <Event/Track.h>
#include <Kernel/MuonTileID.h>

// Muon Detector
#include <MuonDet/DeMuonDetector.h>

// Hlt1Muon
#include <Hlt1Muons/IOutputTool.h>

// local
#include "IsMuonTool.h"
#include "Hlt1MuonHit.h"
#include "Hlt1MuonHitManager.h"
#include "Hlt1MuonStation.h"

//-----------------------------------------------------------------------------
// Implementation file for class : IsMuonTool
//
// 2011-01-19 : Roel Aaij
//-----------------------------------------------------------------------------

using std::vector;
using std::set;
using std::string;

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( IsMuonTool );

//=============================================================================
IsMuonTool::IsMuonTool(const string& type,
                       const string& name,
                       const IInterface* parent )
   : GaudiTool ( type, name , parent ),
     m_nStations( 5 ),    m_nRegions( 4 ),
     m_foiFactor( 1. ),   m_stationZ( m_nStations ),
     m_occupancy( 5, 0 ), m_regionHits( 4 )
{
   declareInterface<ITrackSelector>(this);

}

//=============================================================================
StatusCode IsMuonTool::initialize()
{
   StatusCode sc = GaudiTool::initialize();
   if ( sc.isFailure() ) return sc;

   m_hitManager = tool< Hlt1MuonHitManager >( "Hlt1MuonHitManager" );
   m_outputTool = tool< IOutputTool >( "OutputTool" );

   // fill local arrays of pad sizes and region sizes
   m_det = getDet< DeMuonDetector >( "/dd/Structure/LHCb/DownstreamRegion/Muon" );

   m_padSizeX.resize( m_nStations * m_nRegions );
   m_padSizeY.resize( m_nStations * m_nRegions );
   m_regionInnerX.resize( m_nStations );
   m_regionOuterX.resize( m_nStations );
   m_regionInnerY.resize( m_nStations );
   m_regionOuterY.resize( m_nStations );

   for( unsigned int s = 0; s < m_nStations; ++s ){
      m_regionInnerX[ s ] = m_det->getInnerX( s );
      m_regionOuterX[ s ] = m_det->getOuterX( s );
      m_regionInnerY[ s ] = m_det->getInnerY( s );
      m_regionOuterY[ s ] = m_det->getOuterY( s );

      m_stationZ[ s ] = m_det->getStationZ( s );

      for( unsigned int r = 0; r < m_nRegions; ++r ) {
         m_padSizeX[ s * m_nRegions + r ] = m_det->getPadSizeX( s, r );
         m_padSizeY[ s * m_nRegions + r ] = m_det->getPadSizeY( s, r );
      }
   }

   // Condition pointers
   Condition* foiFactor = 0;
   Condition* xFOIParameters = 0;
   Condition* yFOIParameters = 0;
   Condition* preSelMomentum = 0; 
   Condition* momentumCuts = 0;

   // Register conditions
   registerCondition< IsMuonTool >( "Conditions/ParticleID/Muon/FOIfactor", foiFactor );
   registerCondition< IsMuonTool >( "Conditions/ParticleID/Muon/XFOIParameters", xFOIParameters );
   registerCondition< IsMuonTool >( "Conditions/ParticleID/Muon/YFOIParameters", yFOIParameters );
   registerCondition< IsMuonTool >( "Conditions/ParticleID/Muon/PreSelMomentum", preSelMomentum );
   registerCondition< IsMuonTool >( "Conditions/ParticleID/Muon/MomentumCuts", momentumCuts );

   // Update conditions
   sc = runUpdate();
   if ( sc.isFailure() ) {
      Error( "Could not update conditions from the CondDB" );
      return sc;
   }

   // Get the parameters
   m_foiFactor = foiFactor->param< double >( "FOIfactor" );
   m_xfoiParam1 = xFOIParameters->paramVect< double >( "XFOIParameters1" );
   m_xfoiParam2 = xFOIParameters->paramVect< double >( "XFOIParameters2" );
   m_xfoiParam3 = xFOIParameters->paramVect< double >( "XFOIParameters3" );

   m_yfoiParam1 = yFOIParameters->paramVect< double >( "YFOIParameters1" );
   m_yfoiParam2 = yFOIParameters->paramVect< double >( "YFOIParameters2" );
   m_yfoiParam3 = yFOIParameters->paramVect< double >( "YFOIParameters3" );

   m_preSelMomentum = preSelMomentum->param< double >( "PreSelMomentum" );

   m_momentumCuts = momentumCuts->paramVect< double >( "MomentumCuts" );

   return sc;
}

//=============================================================================
IsMuonTool::~IsMuonTool()
{

} 

//=============================================================================
bool IsMuonTool::accept( const LHCb::Track& track ) const
{
   // Clear temporary storage
   m_occupancy.assign( 5, 0 );
   m_trackX.clear();
   m_trackY.clear();

   // do the track extrapolations
   extrapolateTrack( track );

   // track is in acceptance? Track has minimum momentum?
   bool accept = preSelection( track );
   if ( !accept ) return accept;

   // find the coordinates in the fields of interest
   findHits( track );

   return isMuon( track.p() );
}

//=============================================================================
void IsMuonTool::extrapolateTrack(const LHCb::Track& track ) const {

   // get state closest to M1 for extrapolation
   const LHCb::State& state = track.closestState( 9450. );

   // Project the state into the muon stations
   for( unsigned int station = 0; station < m_nStations ; ++station ) {
      // x(z') = x(z) + (dx/dz * (z' - z))
      m_trackX.push_back( state.x() + state.tx() *
                          ( m_stationZ[ station ] - state.z() ) );
      m_trackY.push_back( state.y() + state.ty() *
                          ( m_stationZ[ station ] - state.z() ) );
   }
}

//=============================================================================
bool IsMuonTool::preSelection( const LHCb::Track& track ) const
{
   // compare momentum and position to acceptance
   if ( track.p() < m_preSelMomentum ) return false;

   // Outer acceptance
   if ( !( fabs( m_trackX[ 0 ] ) < m_regionOuterX[ 0 ] &&
           fabs( m_trackY[ 0 ] ) < m_regionOuterY[ 0 ] ) ||
        !( fabs( m_trackX[ m_nStations - 1 ] ) <
           m_regionOuterX[ m_nStations - 1 ] &&
           fabs( m_trackY[ m_nStations - 1 ] ) <
           m_regionOuterY[ m_nStations - 1 ] ) ) {
      // outside M1 - M5 region
      return false;
   }

   // Inner acceptance
   if ( ( fabs( m_trackX[ 0 ] ) < m_regionInnerX[ 0 ] &&
          fabs( m_trackY[ 0 ] ) < m_regionInnerY[ 0 ] )  ||
        ( fabs( m_trackX[ m_nStations - 1 ] ) <
          m_regionInnerX[ m_nStations - 1 ] &&
          fabs( m_trackY[ m_nStations - 1 ] ) <
          m_regionInnerY[ m_nStations - 1 ] ) ) {
      // inside M1 - M5 chamber hole
      return false;
   }

   return true;
}

//=============================================================================
void IsMuonTool::findHits( const LHCb::Track& track ) const
{
   // Start from 1 because M1 does not matter for IsMuon
   for ( unsigned int s = 1 ; s < m_nStations; ++s ) {
      const Hlt1MuonStation& station = m_hitManager->station( s );

      BOOST_FOREACH( Hlt1ConstMuonHits& hits, m_regionHits ) {
         hits.clear();
      }

      // Convert Hlt1 regions to standard muon station regions
      for( unsigned int r = 0; r < station.nRegions(); ++r ) {
         // Get hits
         Hlt1MuonHitRange hits = m_hitManager->hits( - m_regionOuterX[ s ], s, r );
         BOOST_FOREACH( const Hlt1MuonHit* hit, hits ) {
            const LHCb::MuonTileID& tile = hit->tile();
            m_regionHits[ tile.region() ].push_back( hit );
         }
      }

      for ( unsigned int region = 0; region < m_nRegions; ++region ) {
         
         const double foiXDim = m_foiFactor * foiX( s, region, track.p() );
         const double foiYDim = m_foiFactor * foiY( s, region, track.p() );

         const Hlt1ConstMuonHits& hits = m_regionHits[ region ];
         for ( Hlt1ConstMuonHits::const_iterator it = hits.begin(), end = hits.end();
               it != end; ++it ) {
            
            const Hlt1MuonHit* hit = *it;
            double x  = hit->x();
            double y  = hit->y();

            // check if the hit is in the window
            if(  ( fabs( x - m_trackX[ s ] ) < foiXDim ) &&
                 ( fabs( y - m_trackY[ s ] ) < foiYDim )  ) {
               
               m_occupancy[ s ] += 1;
            }
         } // hits
      } // region
   } // station
}

//=============================================================================
bool IsMuonTool::isMuon( const double p ) const
{
   const double pr1 = m_preSelMomentum;
   const double pr2 = m_momentumCuts[ 0 ];
   const double pr3 = m_momentumCuts[ 1 ];

   set< unsigned int > stations;
   for ( unsigned int i = 1; i < m_occupancy.size(); ++i ) {
      if ( m_occupancy[ i ] != 0 ) stations.insert( i );
   }

   bool id = false;

   if ( p > pr1 && p < pr2 ) {
      if ( stations.count( 1 ) && stations.count( 2 ) )
         id = true;
   } else if ( p > pr2 && p < pr3 ) {
      if ( stations.count( 1 ) && stations.count( 2 ) &&
           ( stations.count( 3 ) || stations.count( 4 ) ) )
         id = true;
   } else if ( p > pr3 ) {
      if ( stations.count( 1 ) && stations.count( 2 ) &&
           stations.count( 3 ) && stations.count( 4 ) )
         id = true;
   }

   if ( msgLevel(MSG::DEBUG) ) {
      debug() << "IsMuon = " << id  << endmsg;
      debug() << "pr1="      << pr1 << endmsg;
      debug() << "pr2="      << pr2 << endmsg;
      debug() << "pr3="      << pr3 << endmsg;
      debug() << "IsMuon p=" << p   << endmsg;
   }

   return id;
}

//=============================================================================
double IsMuonTool::foiX( const int station, const int region, const double p ) const
{
   double dx = m_padSizeX[ station * m_nRegions + region ] / 2.;
   
   if ( p < 1000000. ) {
      return ( m_xfoiParam1[ station * m_nRegions + region ] +
               m_xfoiParam2[ station * m_nRegions + region ] *
               exp( - m_xfoiParam3[ station * m_nRegions + region ] 
                    * p / Gaudi::Units::GeV ) ) * dx;
   } else {
      return m_xfoiParam1[ station * m_nRegions + region ] * dx;
   }
}

//=============================================================================
double IsMuonTool::foiY( const int station, const int region, const double p ) const
{
   double dy = m_padSizeY[ station * m_nRegions + region ] / 2.;

   if ( p < 1000000. ){
      return ( m_yfoiParam1[ station * m_nRegions + region ] +
               m_yfoiParam2[ station * m_nRegions + region ] *
               exp( - m_yfoiParam3[ station * m_nRegions + region ] 
                   * p / Gaudi::Units::GeV ) ) * dy;
   } else {
      return m_yfoiParam1[ station * m_nRegions + region ] * dy;
   }
}
