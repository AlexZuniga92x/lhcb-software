// $Id: $
// STL
#include <vector>
#include <algorithm>
#include <memory>

// from Gaudi
#include <GaudiKernel/ToolFactory.h>
#include <GaudiKernel/SystemOfUnits.h>
#include <GaudiKernel/boost_allocator.h>

// from LHCb
#include <Kernel/ILHCbMagnetSvc.h>

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
DECLARE_TOOL_FACTORY( MatchVeloMuon )

using std::vector;
using std::less;
const std::array<unsigned int,4> order{{ 3u, 4u, 5u, 2u }};

//=============================================================================
MatchVeloMuon::MatchVeloMuon( const std::string& type, const std::string& name,
                              const IInterface* parent )
    : GaudiHistoTool( type, name, parent )
    , m_fieldSvc{nullptr}
    , m_magnetHit{nullptr}
{
    declareInterface<ITracksFromTrack>( this );

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

    declareProperty( "SetQOverP", m_setQOverP = false );
}

//=============================================================================
MatchVeloMuon::~MatchVeloMuon()
{
    boost::singleton_pool<Candidate, sizeof( Candidate )>::release_memory();
}

//=============================================================================
StatusCode MatchVeloMuon::initialize()
{
    StatusCode sc = GaudiHistoTool::initialize();
    if ( sc.isFailure() ) return sc;

    m_hitManager = tool<Hlt1MuonHitManager>( "Hlt1MuonHitManager" );

    if ( m_setQOverP ) {
        // Magnetic Field
        m_fieldSvc = svc<ILHCbMagnetSvc>( "MagneticFieldSvc", true );
    }

    return sc;
}

//=============================================================================
StatusCode MatchVeloMuon::finalize()
{
    clean();
    boost::singleton_pool<Candidate, sizeof( Candidate )>::release_memory();
    return GaudiHistoTool::finalize();
}

//=============================================================================
StatusCode MatchVeloMuon::tracksFromTrack( const LHCb::Track& seed,
                                           std::vector<LHCb::Track*>& tracks )
{
    // Clean start
    clean();

    // Make a Candidate from the track
    Candidate veloSeed{ &seed }; 

    unsigned int seedStation = order[0] - 1;
    findSeeds( veloSeed, seedStation );

    for ( Candidate& c : m_seeds ) addHits( c );

    if ( msgLevel( MSG::DEBUG ) ) {
        debug() << "Found " << m_seeds.size() << " seeds." << endmsg;
        for ( const Candidate& c : m_seeds ) {
            debug() << "Found candidate with chi2/DoF " << c.chi2DoF() << endmsg;
        }
    }

    if ( produceHistos() ) {
        plot( m_seeds.size(), "NSeedHits", -0.5, 50.5, 51 );
        for ( const Candidate& c : m_seeds ) {
            plot( c.chi2DoF(), "Chi2DoFX", 0, 100, 100 );
        }
    }

    if ( !m_setQOverP ) {
        // in this case, we only care whether a good seed exists for the specified track...
        if ( std::any_of( std::begin(m_seeds), std::end(m_seeds), [=](const Candidate& c) {
            return c.chi2DoF() < m_maxChi2DoFX ;
        }) ) {
            // There is a good enough candidate, put the seed into the output
            // unmodified.
            tracks.push_back( const_cast<LHCb::Track*>( &seed ) );
        }
    } else {
        auto best =
            std::min_element( std::begin(m_seeds), std::end(m_seeds),
                              []( const Candidate& lhs, const Candidate& rhs ) {
                return lhs.chi2DoF() < rhs.chi2DoF();
            } );
        if ( best != std::end(m_seeds) && best->chi2DoF() < m_maxChi2DoFX ) {
            std::unique_ptr<LHCb::Track> out{seed.clone()};
            out->addToAncestors( seed );
            out->addInfo( 35, best->slope() - best->tx() );
            double down = m_fieldSvc->isDown() ? -1 : 1;
            double q = down * ( ( best->slope() < best->tx() ) - ( best->slope() > best->tx() ) );
            LHCb::State* state = out->stateAt( LHCb::State::EndVelo );
            state->setQOverP( q / best->p() );
            tracks.push_back( out.release() );
        }
    }
    return StatusCode::SUCCESS;
}

//=============================================================================
void MatchVeloMuon::findSeeds( const Candidate& veloSeed,
                               const unsigned int seedStation )
{
    // forward extrapolation, make seed point
    double zMagnet = m_zb + m_za * veloSeed.tx2();
    double xMagnet = 0., errXMagnet = 0.;
    veloSeed.xStraight( zMagnet, xMagnet, errXMagnet );
    double yMagnet = 0., errYMagnet = 0.;
    veloSeed.yStraight( zMagnet, yMagnet, errYMagnet );

    LHCb::MuonTileID id;
    m_magnetHit =
        new Hlt1MuonHit( id, xMagnet, errXMagnet, yMagnet, errYMagnet, zMagnet, 0. );

    double dSlope = dtx( m_minMomentum );
    const Hlt1MuonStation& station = m_hitManager->station( seedStation );
    double zStation = station.z();

    // Use sum rule for tan and approximate tan( dSlope ) with dSlope to
    // calculate window in x
    double dz = ( zStation - zMagnet ) / veloSeed.cosTy();
    // double sign = ( veloSeed.tx() > 0) - ( veloSeed.tx() < 0 );
    double tanMin = ( veloSeed.tx() - dSlope ) / ( 1 + veloSeed.tx() * dSlope );
    double xMin = xMagnet + dz * tanMin - m_xWindow;
    double tanMax = ( veloSeed.tx() + dSlope ) / ( 1 - veloSeed.tx() * dSlope );
    double xMax = xMagnet + dz * tanMax + m_xWindow;

    // Calculate window in y
    double yMuon = 0., yRange = 0;
    veloSeed.yStraight( zStation, yMuon, yRange );
    yRange += m_yWindow;

    double yMin = yMuon - yRange;
    double yMax = yMuon + yRange;

    if ( msgLevel( MSG::DEBUG ) ) {
        debug() << "Window: (" << xMin << "," << yMin << ") -> (" << xMax << ","
                << yMax << ")" << endmsg;
        debug() << "Hits in seed station:" << endmsg;
        for ( unsigned int r = 0; r < station.nRegions(); ++r ) {
            for ( Hlt1MuonHit* hit : m_hitManager->hits( xMin, xMax, seedStation, r ) ) {
                debug() << hit->x() << " " << hit->y() << endmsg;
            }
        }
    }

    for ( unsigned int r = 0; r < station.nRegions(); ++r ) {
        if ( !station.overlaps( r, xMin, xMax, yMin, yMax ) ) continue; //TODO: push into loop control -- request from station to a range of regions to loop over...

        // get hits, and add seed hits to container
        for ( Hlt1MuonHit* hit : m_hitManager->hits( xMin, xMax, seedStation, r ) ) {
            if ( hit->y() > yMax || hit->y() < yMin ) continue;
            m_seeds.emplace_back ( veloSeed );
            Candidate& seed = m_seeds.back();
            seed.addHit( m_magnetHit );
            seed.addHit( hit );

            seed.slope() = ( hit->x() - xMagnet ) / ( hit->z() - zMagnet );
            seed.p() = momentum( seed.slope() - seed.tx() );

        }
    }
}

//=============================================================================
void MatchVeloMuon::addHits( Candidate& seed )
{
    // First hit is in magnet
    double zMagnet = m_magnetHit->z();
    double xMagnet = m_magnetHit->x();

    unsigned int nMissed = 0;
    for ( unsigned int i = 1; i < order.size(); ++i ) {
        // find candidate hits
        unsigned int s = order[i] - 1;

        // Get the station we're looking at.
        const Hlt1MuonStation& station = m_hitManager->station( s );
        double zStation = station.z();

        // Calculate window in x and y for this station
        double yMuon = 0., yRange = 0;
        seed.yStraight( zStation, yMuon, yRange );
        yRange = m_yWindow;

        const double yMin = yMuon - yRange;
        const double yMax = yMuon + yRange;

        const double xMuon = ( zStation - zMagnet ) * seed.slope() + xMagnet;
        const double xRange = m_xWindow;

        const double xMin = xMuon - xRange;
        const double xMax = xMuon + xRange;

        // Look for the closest hit inside the search window
        const Hlt1MuonHit* closest = nullptr;
        double minDist2 = 0;
        for ( unsigned int r = 0; r < station.nRegions(); ++r ) {
            if ( !station.overlaps(r, xMin, xMax, yMin, yMax ) ) continue;

            for ( const Hlt1MuonHit* hit : m_hitManager->hits( xMin, xMax,  s, r ) ) {
                if ( hit->y() > yMax || hit->y() < yMin ) continue;

                double dist2 = ( xMuon - hit->x() ) * ( xMuon - hit->x() ) +
                               ( yMuon - hit->y() ) * ( yMuon - hit->y() );
                if ( !closest || dist2 < minDist2 ) {
                    closest = hit;
                    minDist2 = dist2;
                }
            }
        }

        if ( closest ) {
            seed.addHit( closest );
        } else {
            ++nMissed;
        }
        if ( nMissed > m_maxMissed ) break;
    }
    // If a new candidate is good, fit it.
    if ( nMissed <= m_maxMissed ) fitCandidate( seed );
}

//=============================================================================
void MatchVeloMuon::fitCandidate( Candidate& candidate ) const
{
    const Hlt1ConstMuonHits& hits = candidate.hits();

    double sumWeights = 0., sumZ = 0., sumX = 0.;

    for ( const Hlt1MuonHit* hit : hits ) {
        double dx = hit->dx();
        double weight = 4.0 / ( dx * dx );
        sumWeights += weight;
        sumZ += hit->z() * weight;
        sumX += hit->x() * weight;
    }
    double ZOverWeights = sumZ / sumWeights;

    double sumTmp2 = 0.;
    double b = 0.;
    for ( const Hlt1MuonHit* hit : hits ) {
        double err = hit->dx() / 2.;
        double tmp = ( hit->z() - ZOverWeights ) / err;
        sumTmp2 += tmp * tmp;
        b += tmp * hit->x() / err;
    }

    b /= sumTmp2;
    double a = ( sumX - sumZ * b ) / sumWeights;
    // double errA = sqrt( ( 1. + sumZ * sumZ / ( sumWeights * sumTmp2 ) ) /
    // sumWeights );
    // double errB = sqrt( 1. / sumTmp2 );

    double chi2 = 0.;
    for ( const Hlt1MuonHit* hit : hits ) {
        double err = hit->dx() / 2.;
        double tmp = ( hit->x() - a - b * hit->z() ) / err;
        chi2 += tmp * tmp;
    }

    candidate.nDoF() = hits.size() - 2;
    candidate.chi2() = chi2;
    candidate.slope() = b;
    candidate.p() = momentum( b - candidate.tx() );
}

//=============================================================================
void MatchVeloMuon::clean()
{
    // delete leftover seeds
    m_seeds.clear(); // leaves capacity invariant, hence we latch onto the max size
    delete m_magnetHit;
    m_magnetHit = nullptr;
}
