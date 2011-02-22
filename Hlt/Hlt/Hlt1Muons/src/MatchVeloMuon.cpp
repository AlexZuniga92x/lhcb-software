// $Id: $
// STL
#include <vector>
#include <algorithm>

// boost
#include <boost/foreach.hpp>
#include <boost/assign/list_of.hpp>
#include <boost/bind.hpp>

// from Gaudi
#include <GaudiKernel/ToolFactory.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <GaudiKernel/boost_allocator.h>

// local
#include "MatchVeloMuon.h"
#include "Hlt1MuonHit.h"
#include "Candidate.h"
#include "Hlt1MuonHitManager.h"
#include "Hlt1MuonStation.h"
#include "Candidate.h"

//-----------------------------------------------------------------------------
// Implementation file for class : MatchVeloMuon
//
// 2010-12-02 : Roel Aaij
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( MatchVeloMuon );

using boost::bind;
using boost::assign::list_of;
using std::vector;
using std::less;

//=============================================================================
MatchVeloMuon::MatchVeloMuon( const std::string& type,
                              const std::string& name,
                              const IInterface* parent )
   : GaudiHistoTool ( type, name , parent ), m_nRegions( 0 ),
     m_magnetHit( 0 ), m_candidates( 0 ), m_seeds( 0 )
{
   declareInterface< ITracksFromTrack >(this);

   declareProperty( "TXTolerance", m_txTolerance = 1.10 );
   declareProperty( "XWindow", m_xWindow = 300 );
   declareProperty( "YWindow", m_yWindow = 300 );

   declareProperty( "MinMomentum", m_minMomentum = 6 * Gaudi::Units::GeV );
   declareProperty( "KickScale", m_kickScale = 1255 * Gaudi::Units::MeV );
   declareProperty( "KickOffset", m_kickOffset = 175 * Gaudi::Units::MeV );
   
   declareProperty( "MaxChi2DoFX", m_maxChi2DoFX = 10 );

   // Focal plane parametrisation from B. Hommels
   declareProperty( "MagnetPlaneParA", m_za = 1.069 * Gaudi::Units::m );
   declareProperty( "MagnetPlaneParB", m_zb = 5.203 * Gaudi::Units::m );

   declareProperty( "MaxMissedHits", m_maxMissed = 2 );
   setProduceHistos( false ); // yes, this indeed changes the default ;-)
   
   m_order = list_of( 3 )( 4 )( 5 )( 2 );
}

//=============================================================================
MatchVeloMuon::~MatchVeloMuon()
{
   boost::singleton_pool< Candidate, sizeof( Candidate ) >::release_memory();
} 

//=============================================================================
StatusCode MatchVeloMuon::initialize()
{
   StatusCode sc = GaudiHistoTool::initialize();
   if ( sc.isFailure() ) return sc;

   m_hitManager = tool< Hlt1MuonHitManager >( "Hlt1MuonHitManager" );

   return sc;
}

//=============================================================================
StatusCode MatchVeloMuon::finalize()
{
   clean();
   bool released = boost::singleton_pool< Candidate, sizeof( Candidate ) >::release_memory();
   if ( !released ) {
      Warning( "some candidates still use memory, purging", StatusCode::SUCCESS );
      boost::singleton_pool< Candidate, sizeof( Candidate ) >::purge_memory();
   }
   return GaudiHistoTool::finalize();
}

//=============================================================================
StatusCode MatchVeloMuon::tracksFromTrack( const LHCb::Track &seed,
                                           std::vector< LHCb::Track* >& tracks )
{
   // Clean start
   clean();

   // Make a Candidate from the track
   std::auto_ptr< Candidate > veloSeed( new Candidate( &seed ) );

   unsigned int seedStation = m_order[ 0 ] - 1;
   findSeeds( veloSeed.get(), seedStation );
   if ( produceHistos() ) plot( m_seeds.size(), "NSeedHits", -0.5, 50.5, 51 );

   BOOST_FOREACH( Candidate* c, m_seeds ) { addHits( c ); }

   // Find the best candidate.
   Candidates::const_iterator best = std::min_element
      ( m_candidates.begin(), m_candidates.end(), bind
        ( less< double >(), bind< double >( &Candidate::chi2DoF, _1 ),
          bind< double >( &Candidate::chi2DoF, _2 ) ) );

   if ( best == m_candidates.end() ) {
      // No candidates found, we're done.
      return StatusCode::SUCCESS;
   } else {
      // There is a best candidate, put the seed into the output unmidified.
      tracks.push_back( const_cast< LHCb::Track* >( &seed ) );
      return StatusCode::SUCCESS;
   }
}

//=============================================================================
void MatchVeloMuon::findSeeds( const Candidate* veloSeed, const unsigned int seedStation )
{
   // forward extrapolation, make seed point
   double zMagnet = m_zb + m_za * veloSeed->tx2();
   double xMagnet = 0., errXMagnet = 0.;
   veloSeed->xStraight( zMagnet, xMagnet, errXMagnet );
   double yMagnet = 0., errYMagnet = 0.;
   veloSeed->yStraight( zMagnet, yMagnet, errYMagnet );

   LHCb::MuonTileID id;
   m_magnetHit = new Hlt1MuonHit( id, xMagnet, errXMagnet,
                                  yMagnet, errYMagnet,
                                  zMagnet, 0. );

   double dSlope = dtx( m_minMomentum );
   const Hlt1MuonStation& station = m_hitManager->station( seedStation );
   double zStation = station.z();

   // Use sum rule for tan and approximate tan( dSlope ) with dSlope to 
   // calculate window in x
   double dz = ( zStation - zMagnet ) / veloSeed->cosTy();
   // double sign = ( veloSeed->tx() > 0) - ( veloSeed->tx() < 0 );
   double tanMin = ( veloSeed->tx() - dSlope ) / ( 1 + veloSeed->tx() * dSlope );
   double xMin = xMagnet + dz * tanMin * m_txTolerance;
   double tanMax = ( veloSeed->tx() + dSlope ) / ( 1 - veloSeed->tx() * dSlope );
   double xMax = xMagnet + dz * tanMax * m_txTolerance;

   // Calculate window in y
   double yMuon = 0., yRange = 0;
   veloSeed->yStraight( zStation, yMuon, yRange );
   yRange += m_yWindow;

   double yMin = yMuon - yRange;
   double yMax = yMuon + yRange;

   for ( unsigned int r = 0; r < station.nRegions(); ++r ) {
      const Hlt1MuonRegion& region = station.region( r );
      if ( !region.overlap( xMin, xMax, yMin, yMax ) ) continue;

      // Get hits
      Hlt1MuonHitRange hits = m_hitManager->hits( xMin, seedStation, r );
         
      if ( hits.empty() ) continue; 

      // add seed hits to container
      BOOST_FOREACH( Hlt1MuonHit* hit, hits ) {
         if ( hit->x() > xMax ) break;
         if ( hit->y() > yMax || hit->y() < yMin ) continue;
         Candidate* seed = new Candidate( *veloSeed );
         seed->addHit( m_magnetHit );
         seed->addHit( hit );

         seed->slope() = ( hit->x() - xMagnet ) / ( hit->z() - zMagnet );
         seed->p()     = momentum( seed->slope() - seed->tx() );

         m_seeds.push_back( seed );
      }
   }
}

//=============================================================================
void MatchVeloMuon::addHits( Candidate* seed )
{
   // First hit is in magnet
   double zMagnet = m_magnetHit->z();
   double xMagnet = m_magnetHit->x();

   unsigned int nMissed = 0;
   for ( unsigned int i = 1; i < m_order.size(); ++i ) {
      // find candidate hits
      unsigned int s = m_order[ i ] - 1;

      // Get the station we're looking at.
      const Hlt1MuonStation& station = m_hitManager->station( s );
      double zStation = station.z();

      // Calculate window in x and y for this station
      double yMuon = 0., yRange = 0;
      seed->yStraight( zStation, yMuon, yRange );
      yRange = m_yWindow;

      const double yMin = yMuon - yRange;
      const double yMax = yMuon + yRange;

      const double xMuon = ( zStation - zMagnet ) * seed->slope() + xMagnet;
      const double xRange = m_xWindow;

      const double xMin = xMuon - xRange;
      const double xMax = xMuon + xRange;

      // Look for the closest hit inside the search window
      const Hlt1MuonHit* closest = 0;
      double minDist2 = 0;
      for ( unsigned int r = 0; r < station.nRegions(); ++r ) {
         const Hlt1MuonRegion& region = station.region( r );
         if ( !region.overlap( xMin, xMax, yMin, yMax ) ) continue;
         Hlt1MuonHitRange hits = m_hitManager->hits( xMin, s, r );
         BOOST_FOREACH( Hlt1MuonHit* hit, hits ) {
            if ( hit->x() > xMax ) break;
            if ( hit->y() > yMax || hit->y() < yMin ) continue;
            double dist2 =  ( xMuon - hit->x() ) * ( xMuon - hit->x() )
                          + ( yMuon - hit->y() ) * ( yMuon - hit->y() ) ;
            if ( !closest || dist2 < minDist2 ) {
               closest = hit;
               minDist2 = dist2;
            }
         }
      }

      if ( closest != 0 ) {
         seed->addHit( closest );
      } else {
         ++nMissed ;
      }
      if ( nMissed >= m_maxMissed ) break;
   }
   // If a new candidate is good, store it.
   if ( nMissed < m_maxMissed ) {
      fitCandidate( seed );
      if ( produceHistos() ) plot( seed->chi2DoF(), "Chi2DoFX", 0, 100, 100 );
      if ( seed->chi2DoF() < m_maxChi2DoFX ) m_candidates.push_back( seed );
   }
}

//=============================================================================
void MatchVeloMuon::fitCandidate( Candidate* candidate ) const
{
   const Hlt1ConstMuonHits& hits = candidate->hits();

   double sumWeights = 0., sumZ = 0., sumX = 0., sumTmp2 = 0.;

   BOOST_FOREACH( const Hlt1MuonHit* hit, hits ) {
      double err = hit->dx() / 2.;
      double weight = 1.0 / ( err * err );
      sumWeights += weight;
      sumZ += hit->z() * weight;
      sumX += hit->x() * weight;
   }
   double ZOverWeights = sumZ / sumWeights;

   double b = 0;
   BOOST_FOREACH( const Hlt1MuonHit* hit, hits ) {
      double err = hit->dx() / 2.;
      double tmp = ( hit->z() - ZOverWeights ) / err;
      sumTmp2 += tmp * tmp;
      b += tmp * hit->x() / err;
   }

   b /= sumTmp2;
   double a = ( sumX - sumZ * b ) / sumWeights;
   // double errA = sqrt( ( 1. + sumZ * sumZ / ( sumWeights * sumTmp2 ) ) / sumWeights );
   // double errB = sqrt( 1. / sumTmp2 );

   double chi2 = 0.;
   BOOST_FOREACH( const Hlt1MuonHit* hit, hits ) {
      double err = hit->dx() / 2.;
      double tmp = ( hit->x() - a - b * hit->z() ) / err;
      chi2 += tmp * tmp;
   }

   candidate->nDoF() = hits.size() - 2;
   candidate->chi2() = chi2;
   candidate->slope() = b;
   candidate->p()     = momentum( b - candidate->tx() );
}

//=============================================================================
void MatchVeloMuon::clean()
{
   // delete leftover seeds
   BOOST_FOREACH( Candidate* candidate, m_seeds ) { delete candidate; }
   m_seeds.clear();
   m_candidates.clear();
   delete m_magnetHit;
   m_magnetHit = 0;
}
