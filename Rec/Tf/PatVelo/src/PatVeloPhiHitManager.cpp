// $Id: PatVeloPhiHitManager.cpp,v 1.3 2008-08-26 17:52:20 dhcroft Exp $

#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/IRegistry.h"
#include "GaudiKernel/IIncidentSvc.h"

#include "PatVeloPhiHitManager.h"


//-----------------------------------------------------------------------------
// Implementation file for class : PatVeloPhiHitManager
//
// 2007-08-08 : Kurt Rinnert <kurt.rinnert@cern.ch>
//-----------------------------------------------------------------------------

namespace Tf {
  DECLARE_TOOL_FACTORY( PatVeloPhiHitManager );


  //=============================================================================
  // Standard constructor, initializes variables
  //=============================================================================
  PatVeloPhiHitManager::PatVeloPhiHitManager(const std::string& type,
      const std::string& name,
      const IInterface* parent)
    : Tf::ExtendedVeloPhiHitManager<PatVeloPhiHit>(type, name, parent)
    {
      declareInterface<PatVeloPhiHitManager>(this);
    }

  //=============================================================================
  // Initialization
  //=============================================================================
  StatusCode PatVeloPhiHitManager::initialize()
  {
    StatusCode sc = Tf::ExtendedVeloPhiHitManager<PatVeloPhiHit>::initialize(); // must be executed first
    if (sc.isFailure()) return sc;  // error printed already by GaudiTool

    debug() << "==> Initialize" << endmsg;

    return StatusCode::SUCCESS;
  }

  //=============================================================================
  // Finalize
  //=============================================================================
  StatusCode PatVeloPhiHitManager::finalize()
  {
    debug() << "==> Finalize" << endmsg;

    return Tf::ExtendedVeloPhiHitManager<PatVeloPhiHit>::finalize();  // must be called after all other actions
  }

  //=============================================================================
  // Preparation of measurements
  //=============================================================================
  void PatVeloPhiHitManager::prepareHits() 
  {
    for (unsigned int half=0; half<m_nHalfs; ++half) { // loop over velo halfs
      DefaultStationIterator si   = m_defaultHitManager->stationsHalfBegin(half);
      DefaultStationIterator send = m_defaultHitManager->stationsHalfEnd(half);

      for ( ; si != send; ++si) {
        DefaultStation* defaultStation = *si;
        unsigned int defaultStationNumber = defaultStation->stationNumber();
        Station* station =  m_stationsHalf[half][defaultStationNumber];
        if (station->hitsPrepared()) continue;
        station->clear();

        for (unsigned int zone=0; zone<m_nZones; ++zone) { // loop over inner/outer zones
          Tf::VeloPhiHitRange hits = defaultStation->hits(zone);
          Tf::VeloPhiHits::const_iterator hi   = hits.begin();
          Tf::VeloPhiHits::const_iterator hend = hits.end();

          m_data[half][defaultStationNumber][zone].reserve(std::distance(hi,hend));
          for ( ; hi != hend; ++hi) { // import all hits
            m_data[half][defaultStationNumber][zone].push_back(PatVeloPhiHit(*hi)); 
            station->zone(zone).push_back(&(m_data[half][defaultStationNumber][zone].back()));
          }
        }
        station->setHitsPrepared(true);
      }
    }
    m_dataValid = true;
  }

  //=============================================================================
  // Preparation of measurements for one station, actual implementation
  //=============================================================================
  void PatVeloPhiHitManager::prepareHits(Station* station) 
  {
    DefaultStation*    defaultStation = m_defaultHitManager->stationNoPrep(station->sensorNumber());
    if (!defaultStation->hitsPrepared()) {
      m_defaultHitManager->prepareHits(defaultStation);
    }
    unsigned int defaultStationNumber = defaultStation->stationNumber();
    unsigned int half = station->veloHalf();

    for (unsigned int zone=0; zone<m_nZones; ++zone) { // loop over inner/outer zones
      Tf::VeloPhiHitRange hits = defaultStation->hits(zone);
      Tf::VeloPhiHits::const_iterator hi   = hits.begin();
      Tf::VeloPhiHits::const_iterator hend = hits.end();

      m_data[half][defaultStationNumber][zone].reserve(std::distance(hi,hend));
      for ( ; hi != hend; ++hi ) { // import all hits
        m_data[half][defaultStationNumber][zone].push_back(PatVeloPhiHit(*hi)); 
        station->zone(zone).push_back(&(m_data[half][defaultStationNumber][zone].back()));
      }
    }
    station->setHitsPrepared(true);
  }

  //=============================================================================
  // Reset all used flags to unused
  //=============================================================================
  void PatVeloPhiHitManager::resetUsedFlagOfHits() 
  {
    for (unsigned int half=0; half<m_nHalfs; ++half) { // loop over velo halfs
      DefaultStationIterator si   = m_defaultHitManager->stationsHalfBegin(half);
      DefaultStationIterator send = m_defaultHitManager->stationsHalfEnd(half);

      for ( ; si != send; ++si) {
        for (unsigned int zone=0; zone<m_nZones; ++zone) { // loop over inner/outer zones
          Tf::VeloPhiHitRange hits = (*si)->hits(zone);
          Tf::VeloPhiHits::const_iterator hi   = hits.begin();
          Tf::VeloPhiHits::const_iterator hend = hits.end();
          for ( ; hi != hend; ++hi ) { // import all hits
	    (*hi)->resetUsedFlag();
          }
        }
      }
    }
  }
}






