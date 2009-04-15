// Include files

// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

// local
#include "PatVeloAlignTrackFilter.h"

//-----------------------------------------------------------------------------
// Implementation file for class : PatVeloAlignTrackFilter
//
// 2008-09-08 : Marco Gersabeck
//-----------------------------------------------------------------------------

using namespace Gaudi::Units;
using namespace LHCb;
using namespace Gaudi;

namespace Tf {
  DECLARE_ALGORITHM_FACTORY( PatVeloAlignTrackFilter );

  //=============================================================================
  // Standard constructor, initializes variables
  //=============================================================================

  PatVeloAlignTrackFilter::PatVeloAlignTrackFilter( const std::string& name,
      ISvcLocator* pSvcLocator)
    : GaudiAlgorithm ( name , pSvcLocator )
    {
      declareProperty( "RHitManagerName", m_rHitManagerName="DefaultVeloRHitManager" );
      declareProperty( "PhiHitManagerName", m_phiHitManagerName="DefaultVeloPhiHitManager" );
      declareProperty( "VeloClusterLocation", m_clusterCont=LHCb::VeloClusterLocation::Default );
      declareProperty( "CheckADC", m_checkADC = false );
      declareProperty( "MinRCharge", m_min_r_charge = 5 );
      declareProperty( "MaxRCharge", m_max_r_charge = 200 );
      declareProperty( "MinPhiCharge", m_min_phi_charge = 5 );
      declareProperty( "MaxPhiCharge", m_max_phi_charge = 200 );
      declareProperty( "MinTot", m_min_tot = 20 );
      declareProperty( "MaxTot", m_max_tot = 5000 );
      declareProperty( "MinRSlice", m_min_rslice = 18 );
      declareProperty( "MinCell", m_min_cell = 18 );
      declareProperty( "MaxCell", m_max_cell = 100 );
      declareProperty( "MaxDiffCell", m_max_diff_cell = -1 );
      declareProperty( "MaxSensor", m_max_sensor = 25 );
      declareProperty( "Overlaps", m_overlaps = false );
      declareProperty( "NBinsR", m_nBinsR = 36 );
      declareProperty( "NBinsPhi", m_nBinsPhi = 36 );
      declareProperty( "NStations", m_nStations = 42 );
      declareProperty( "BigCell", m_bigCell = true );
      declareProperty( "MinOverlap", m_min_overlap = 3 );

      m_numberOfRSectors = 4;
      m_numberOfPSectors = 2;
      m_radiusOffset = 7.;
    }

  //=============================================================================
  // Destructor
  //=============================================================================

  PatVeloAlignTrackFilter::~PatVeloAlignTrackFilter() {}; 

  //=============================================================================
  // Initialization
  //=============================================================================

  StatusCode PatVeloAlignTrackFilter::initialize() {
    StatusCode sc = GaudiAlgorithm::initialize();
    if ( sc.isFailure() ) return sc;

    m_debugLevel = msgLevel( MSG::DEBUG ) ;
    m_verboseLevel = msgLevel( MSG::VERBOSE ) ;

    if ( m_debugLevel ) debug() << "==> Initialize" << endmsg;

    m_rHitManager   = tool<DefaultVeloRHitManager>  ( "Tf::DefaultVeloRHitManager", m_rHitManagerName  );
    m_phiHitManager = tool<DefaultVeloPhiHitManager>( "Tf::DefaultVeloPhiHitManager", m_phiHitManagerName  );

    return StatusCode::SUCCESS;
  };

  //=============================================================================
  // Main execution
  //=============================================================================

  StatusCode PatVeloAlignTrackFilter::execute() {

    if ( m_debugLevel ) debug() << "==> Execute" << endmsg;

    setFilterPassed(false);

    // reset all arrays
    for ( int i = 0; i < m_nBinsR; i++ ) {
      for ( int j = 0; j < m_nBinsPhi; j++ ) {
        for ( int k = 0; k < m_nStations; k++ ) {
          m_3Dhits[ i ][ j ][ k ] = 0;
        }
        m_hits[ i ][ j ] = 0;
        m_hitsR[ i ][ j ] = 0;
        m_hitsL[ i ][ j ] = 0;
      }
      m_rhits[ i ] = 0;
    }
    m_tothits = 0;

    // get cluster container
    if ( m_debugLevel ) debug() << "Retrieving " << m_clusterCont << endmsg;
    if ( !exist<LHCb::VeloClusters>(m_clusterCont ) ) {
      
      if ( m_debugLevel )
        debug() << " ----  No VeloClusters container retrieved --- " << endmsg;
      
      return (StatusCode::SUCCESS);
      
    } else {
      m_clusters = get<LHCb::VeloClusters>(m_clusterCont );
      
      if ( m_debugLevel ) {
        debug() << "---- VeloCluster Container retrieved from default location ----" << endmsg;
        debug()<< "Number of clusters found in TES: " << m_clusters->size() <<endmsg;
      }
    }

    DefaultVeloRHitManager::Station *stationR0;
    DefaultVeloPhiHitManager::Station *stationP0;
    unsigned int zoneR0;
    unsigned int zoneP0;

    std::vector<VeloRHit*>::const_iterator   cR0;
    std::vector<VeloPhiHit*>::const_iterator   cP0;
    LHCb::VeloCluster *cluster;
    double charge = 0.;

    // loop over all hits to find space points
    for (unsigned int half=0; half < 2; ++half) {

      if ( m_debugLevel ) debug() << "Looking for hits in half " << half << endreq;

      DefaultVeloRHitManager::StationIterator        rStationsBegin        = m_rHitManager->stationsHalfBegin(half);
      DefaultVeloRHitManager::StationIterator        rStationsEnd          = m_rHitManager->stationsHalfEnd(half);

      DefaultVeloRHitManager::StationIterator stationItr0;
      for (stationItr0  = rStationsBegin;
          stationItr0 != rStationsEnd; 
          ++stationItr0 ) {

        stationR0 = *stationItr0;     
        if(!stationR0->sensor()->isReadOut()) continue;       // only look at read-out sensors

        for (zoneR0 = 0; zoneR0 < m_numberOfRSectors; ++zoneR0) {

          if (stationR0->empty(zoneR0)) continue;      

          for (zoneP0 = 0; zoneP0 < m_numberOfPSectors; ++zoneP0) {
            stationP0 = m_phiHitManager->station(stationR0->sensor()->associatedPhiSensor()->sensorNumber());
            if (stationP0->empty(zoneP0)) continue;

            if ( m_verboseLevel ) verbose() << "Looking for r/phi hits" << endreq;

            VeloRHitRange hitsR0 = stationR0->hits(zoneR0);
            for(cR0 = hitsR0.begin(); hitsR0.end() != cR0; ++cR0) {

              if ( m_checkADC ) {
                cluster = (LHCb::VeloCluster*)m_clusters->containedObject( (*cR0)->channelID() ); 
                charge = cluster->totalCharge();
                if ( m_debugLevel ) debug() << "R signal " << charge << endreq;
                if ( ( m_min_r_charge > charge ) || ( m_max_r_charge < charge ) ) {
                  continue;
                }
              }

              double r = (*cR0)->rHalfBox();
              int r_coord = (int)floor( ( r - m_radiusOffset ) * m_nBinsR / 36. );
              if ( m_verboseLevel ) verbose() << "R coord matching: " << (*cR0)->rHalfBox() << " " << r_coord << endreq;
              m_rhits[ r_coord ]++;

              VeloPhiHitRange hitsP0 = stationP0->hits(zoneP0);
              for(cP0 = hitsP0.begin(); hitsP0.end() != cP0; ++cP0) {

                if ( m_checkADC ) {
                  cluster = (LHCb::VeloCluster*)m_clusters->containedObject( (*cP0)->channelID() ); 
                  charge = cluster->totalCharge();
                  if ( m_debugLevel ) debug() << "Phi signal " << charge << endreq;
                  if ( ( m_min_phi_charge > charge ) || ( m_max_phi_charge < charge ) ) {
                    continue;
                  }
                }

                double phi = (*cP0)->sortCoordHalfBox();
                if ( Gaudi::Units::pi < phi ) phi -= 2 * Gaudi::Units::pi;
                if ( m_verboseLevel ) {
                  verbose() << "Found r/phi hit at " << r << " " << phi << endreq;
                  verbose() << "R range " << stationP0->sensor()->halfboxRRange( zoneP0 ).first 
                            << " " << stationP0->sensor()->halfboxRRange( zoneP0 ).second << endreq;
                  verbose() << "Phi range " << stationR0->sensor()->halfboxPhiRange( zoneR0 ).first 
                            << " " << stationR0->sensor()->halfboxPhiRange( zoneR0 ).second << endreq;
                }

                // check whether r and phi information matches
                if ( ( r < stationP0->sensor()->halfboxRRange( zoneP0 ).first )
                     || ( r > stationP0->sensor()->halfboxRRange( zoneP0 ).second ) 
                     || ( phi < stationR0->sensor()->halfboxPhiRange( zoneR0 ).first ) 
                     || ( phi > stationR0->sensor()->halfboxPhiRange( zoneR0 ).second ) ) {
                  continue;
                }

                // compute 'bin' and store entry in arrays
                double phi_raw = (*cP0)->sensor()->halfboxPhi( (*cP0)->strip(), 0, r ) / Gaudi::Units::pi * 180.;
                phi_raw += phi_raw < 0. ? 360. : 0.;
                int phi_coord = (int)floor( phi_raw * m_nBinsPhi / 360. );
                if ( ( r_coord < 0 ) || ( r_coord >= m_nBinsR ) || ( phi_coord < 0 ) || ( phi_coord >= m_nBinsPhi ) ) {
                  warning() << "coordinate(s) out of range: r = " << r_coord << ", phi = " << phi_coord << endreq;
                }
                else {
                  m_3Dhits[ r_coord ][ phi_coord ][ (*cR0)->sensor()->sensorNumber() ]++;
                  m_hits[ r_coord ][ phi_coord ]++;
                  // distinguish right/left half
                  if ( 0 == ( (*cR0)->sensor()->sensorNumber() % 2 ) ) m_hitsL[ r_coord ][ phi_coord ]++;
                  if ( 1 == ( (*cR0)->sensor()->sensorNumber() % 2 ) ) m_hitsR[ r_coord ][ phi_coord ]++;
                  m_tothits++;
                }
              } // end of loop over Phi hits
            } // end of loop over R hits
          } // end of loop over Phi-sectors
        } // end of loop over R-sectors
      } // end of loop over R-sensors is here
    } // end of loop over VELO halfs is here

  if ( m_verboseLevel ) {
    // plot matrix of space-points found
    verbose() << "   ";
    for ( int i = 0; i < m_nBinsR; i++ ) {
      verbose() << (int)( i / 10 ) << " ";
    }
    verbose() << endreq;
    verbose() << "   ";
    for ( int i = 0; i < m_nBinsR; i++ ) {
      verbose() << (int)( i % 10 ) << " ";
    }
    verbose() << endreq;
    verbose() << "-----------------------------------------------------------------------------------" << endreq;
    for ( int i = 0; i < m_nBinsPhi; i++ ) {
      verbose() << (int)( i / 10 ) << (int)( i % 10 ) << " ";
      for ( int j = 0; j < m_nBinsR; j++ ) {
        verbose() << m_hits[ j ][ i ] << " ";
      }
      verbose() << endreq;
    }
    verbose() << "-----------------------------------------------------------------------------------" << endreq;
    for ( int i = 0; i < m_nBinsR; i++ ) {
      verbose() << m_rhits[ i ] << " ";
    }
    verbose() << endreq;
    verbose() << "-----------------------------------------------------------------------------------" << endreq;
    verbose() << m_tothits << endreq;
  }

  // check event for possible track parallel to the beam
  if ( ( m_tothits >= m_min_tot ) && ( m_tothits <= m_max_tot ) ) {
    for ( int i = 0; i < m_nBinsR; i++ ) {
      if ( m_rhits[ i ] < m_min_rslice ) {
        if ( m_verboseLevel ) verbose() << "Not enough hits in r slice" << endreq;
        continue;
      }
      for ( int j = 0; j < m_nBinsPhi; j++ ) {
        if ( ( m_hits[ i ][ j ] < m_min_cell ) || ( m_hits[ i ][ j ] > m_max_cell ) ) {
          if ( m_verboseLevel ) verbose() << "Number of hits in cell out of range" << endreq;
          continue;
        }
        int cell_cluster_hits = hitsInCell( i, j );
        if ( m_verboseLevel ) {
          verbose() << "Cell " << i << ", " << j << " has " << m_hits[ i ][ j ] << endreq;
          verbose() << "Cell cluster " << i << ", " << j << " has " << cell_cluster_hits << endreq;
        }
        if ( 0 <= m_max_diff_cell ) {
          if ( ( cell_cluster_hits - m_hits[ i ][ j ] ) > m_max_diff_cell ) {
            if ( m_debugLevel ) debug() << "Too many hits around interesting cell" << endreq;
            continue;
          }
        }
        else {
          if ( ( cell_cluster_hits - m_hits[ i ][ j ] ) > m_hits[ i ][ j ] ) {
            if ( m_debugLevel ) debug() << "More hits around interesting cell than inside" << endreq;
            continue;
          }
        }
        int max_hits = 0;
        int hits;
        int n_sensors_with_hits = 0;
        for ( int k = 0; k < m_nStations; k++ ) {
          hits = sensorHitsInCell( i, j, k );
          if ( m_verboseLevel ) verbose() << "Cell cluster " << i << ", " << j << " has " << hits << " at station " << k << endreq;
          if ( hits > max_hits ) max_hits = hits;
          if ( hits > 0 ) n_sensors_with_hits++;
        }
        if ( max_hits > m_max_sensor ) {
          if ( m_debugLevel ) debug() << "Too many hits on single sensor: " << max_hits << endreq;
          continue;
        }
        if ( n_sensors_with_hits < m_min_cell ) {
          if ( m_debugLevel ) debug() << "Too few sensors with hits: " << n_sensors_with_hits << endreq; 
          continue;
        }

        // at this point the event should be kept
        if ( m_debugLevel ) debug() << "Found track with " << m_hits[ i ][ j ] << " cell hits, " << cell_cluster_hits << " cell cluster hits on " << n_sensors_with_hits << " sensors" << endreq;

        if ( !m_overlaps ) {
          setFilterPassed(true);
        }
        else { // only for overlap detection
          int hitsR = hitsInRightCell( i, j );
          int hitsL = hitsInLeftCell( i, j );
          // check whether there are enough hits on either side
          if ( ( m_min_overlap < hitsR ) && ( m_min_overlap < hitsL ) ) {
            if ( m_debugLevel ) debug() << "Found overlap track" << endreq;
            setFilterPassed(true);
          }
        }
      } // loop over phi bins
    } // loop over r bins
  } // if total number of hits in range

  return StatusCode::SUCCESS;
};

int PatVeloAlignTrackFilter::hitsInCell( int rhit, int phit ) {
  if ( m_verboseLevel ) verbose() << "==> hitsInCell" << endmsg;

  int cluster_hits = 0;
  if ( m_bigCell ) {
    if ( rhit == 0 ) {
      cluster_hits = m_hits[ rhit ]    [ phit ] 
                   + m_hits[ rhit + 1 ][ phit ] 
                   + m_hits[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit + 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit + 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] ;
    }
    else if ( rhit == m_nBinsR - 1 ) {
      cluster_hits = m_hits[ rhit - 1 ][ phit ] 
                   + m_hits[ rhit ]    [ phit ] 
                   + m_hits[ rhit - 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit - 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] ;
    }
    else {
      cluster_hits = m_hits[ rhit - 1 ][ phit ] 
                   + m_hits[ rhit ]    [ phit ] 
                   + m_hits[ rhit + 1 ][ phit ] 
                   + m_hits[ rhit - 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit + 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit - 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
                   + m_hits[ rhit + 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] ;
    }
  }
  else {
    cluster_hits = m_hits[ rhit ][ phit ];
  }

  return cluster_hits;
}

int PatVeloAlignTrackFilter::sensorHitsInCell( int rhit, int phit, int sensor ) {
  if ( m_verboseLevel ) verbose() << "==> sensorHitsInCell" << endmsg;

  int cluster_hits = 0;
  if ( m_bigCell ) {
    if ( rhit == 0 ) {
      cluster_hits = m_3Dhits[ rhit ]    [ phit ]                                  [ sensor ] 
                   + m_3Dhits[ rhit + 1 ][ phit ]                                  [ sensor ]
                   + m_3Dhits[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ][ sensor ] 
                   + m_3Dhits[ rhit + 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ][ sensor ]
                   + m_3Dhits[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ][ sensor ] 
                   + m_3Dhits[ rhit + 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ][ sensor ] ;
    }
    else if ( rhit == m_nBinsR - 1 ) {
      cluster_hits = m_3Dhits[ rhit - 1 ][ phit ]                                  [ sensor ] 
                   + m_3Dhits[ rhit ]    [ phit ]                                  [ sensor ]
                   + m_3Dhits[ rhit - 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ][ sensor ] 
                   + m_3Dhits[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ][ sensor ]
                   + m_3Dhits[ rhit - 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ][ sensor ] 
                   + m_3Dhits[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ][ sensor ] ;
    }
    else {
      cluster_hits = m_3Dhits[ rhit - 1 ][ phit ]                                  [ sensor ]
                   + m_3Dhits[ rhit ]    [ phit ]                                  [ sensor ]
                   + m_3Dhits[ rhit + 1 ][ phit ]                                  [ sensor ]
                   + m_3Dhits[ rhit - 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ][ sensor ]
                   + m_3Dhits[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ][ sensor ]
                   + m_3Dhits[ rhit + 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ][ sensor ]
                   + m_3Dhits[ rhit - 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ][ sensor ]
                   + m_3Dhits[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ][ sensor ]
                   + m_3Dhits[ rhit + 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ][ sensor ] ;
    }
  }
  else {
    cluster_hits = m_3Dhits[ rhit ][ phit ][ sensor ];
  }

  return cluster_hits;
}

int PatVeloAlignTrackFilter::hitsInLeftCell( int rhit, int phit ) {
  if ( m_verboseLevel ) verbose() << "==> hitsInLeftCell" << endmsg;

  int hitsL = 0;
  if ( m_bigCell ) {
    if ( rhit == 0 ) {
      hitsL = m_hitsL[ rhit ]    [ phit ] 
            + m_hitsL[ rhit + 1 ][ phit ]
            + m_hitsL[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
            + m_hitsL[ rhit + 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ]
            + m_hitsL[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
            + m_hitsL[ rhit + 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ];
    }
    else if ( rhit == m_nBinsR - 1 ) {
      hitsL = m_hitsL[ rhit - 1 ][ phit ] 
            + m_hitsL[ rhit ]    [ phit ]
            + m_hitsL[ rhit - 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
            + m_hitsL[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ]
            + m_hitsL[ rhit - 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
            + m_hitsL[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ];
    }
    else {
      hitsL = m_hitsL[ rhit - 1 ][ phit ] 
            + m_hitsL[ rhit ]    [ phit ] 
            + m_hitsL[ rhit + 1 ][ phit ]
            + m_hitsL[ rhit - 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
            + m_hitsL[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
            + m_hitsL[ rhit + 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ]
            + m_hitsL[ rhit - 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
            + m_hitsL[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
            + m_hitsL[ rhit + 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ];
    }
  }
  else {
    hitsL = m_hitsL[ rhit ][ phit ];
  }

  return hitsL;
}

int PatVeloAlignTrackFilter::hitsInRightCell( int rhit, int phit ) {
  if ( m_verboseLevel ) verbose() << "==> hitsInRightCell" << endmsg;

  int hitsR = 0;
  if ( m_bigCell ) {
    if ( rhit == 0 ) {
      hitsR = m_hitsR[ rhit ]    [ phit ] 
            + m_hitsR[ rhit + 1 ][ phit ]
            + m_hitsR[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
            + m_hitsR[ rhit + 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ]
            + m_hitsR[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
            + m_hitsR[ rhit + 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ];
    }
    else if ( rhit == m_nBinsR - 1 ) {
      hitsR = m_hitsR[ rhit - 1 ][ phit ] 
            + m_hitsR[ rhit ]    [ phit ]
            + m_hitsR[ rhit - 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
            + m_hitsR[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ]
            + m_hitsR[ rhit - 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
            + m_hitsR[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ];
    }
    else {
      hitsR = m_hitsR[ rhit - 1 ][ phit ] 
            + m_hitsR[ rhit ]    [ phit ] 
            + m_hitsR[ rhit + 1 ][ phit ]
            + m_hitsR[ rhit - 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
            + m_hitsR[ rhit ]    [ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ] 
            + m_hitsR[ rhit + 1 ][ ( phit + m_nBinsPhi - 1 ) % m_nBinsPhi ]
            + m_hitsR[ rhit - 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
            + m_hitsR[ rhit ]    [ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ] 
            + m_hitsR[ rhit + 1 ][ ( phit + m_nBinsPhi + 1 ) % m_nBinsPhi ];
    }
  }
  else {
    hitsR = m_hitsR[ rhit ][ phit ]; 
  }

  return hitsR;
}

//=============================================================================
//  Finalize
//=============================================================================

StatusCode PatVeloAlignTrackFilter::finalize() {
  if ( m_debugLevel ) debug() << "==> Finalize" << endmsg;

  return GaudiAlgorithm::finalize();
}
}
