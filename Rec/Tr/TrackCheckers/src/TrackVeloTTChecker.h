// $Id: TrackVeloTTChecker.h,v 1.3 2006-05-19 13:13:16 erodrigu Exp $
#ifndef TRACKCHECKERS_TRACKVELOTTCHECKER_H 
#define TRACKCHECKERS_TRACKVELOTTCHECKER_H 1

// Include files

// from Gaudi
#include "GaudiAlg/GaudiTupleAlg.h"

// From STDet
#include "STDet/DeSTDetector.h"

namespace LHCb {
  class Track;
}

/** @class TrackVeloTTChecker TrackVeloTTChecker.h
 *  
 *
 *  @author Eduardo Rodrigues (adaptations to new Track Event Model)
 *  @date   2005-11-04
 *  @author :  Yuehong Xie
 *  @date   :  15/10/2002
 *
 */

using namespace LHCb;

class TrackVeloTTChecker : public GaudiTupleAlg {
public: 

  /// Standard constructor
  TrackVeloTTChecker( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~TrackVeloTTChecker( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

  MCParticle* VeloTrackMCTruth( LHCb::Track* track );

  double CenterVeloTr( LHCb::Track* track );

private:

  int m_nEvents;     ///< Number of events processed

  DeSTDetector* m_TTDet;

};
#endif // TRACKCHECKERS_TRACKVELOTTCHECKER_H
