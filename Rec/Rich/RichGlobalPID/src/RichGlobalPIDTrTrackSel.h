// $Id: RichGlobalPIDTrTrackSel.h,v 1.6 2003-12-11 16:33:34 cattanem Exp $
#ifndef RICHRECALGS_RICHGLOBALPIDTRTRACKSEL_H
#define RICHRECALGS_RICHGLOBALPIDTRTRACKSEL_H 1

// Base class
#include "RichGlobalPIDAlgBase.h"

// base definitions
#include "RichRecBase/RichRecProcCode.h"
#include "RichRecBase/RichTrackSelector.h"

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// Event
#include "Event/ProcStatus.h"
#include "Event/RichRecStatus.h"

// CLHEP
#include "CLHEP/Units/PhysicalConstants.h"

// interfaces
#include "RichRecBase/IRichTrackCreator.h"
#include "RichRecBase/IRichExpectedTrackSignal.h"

/** @class RichGlobalPIDTrTrackSel RichGlobalPIDTrTrackSel.h
 *
 *  Track selection algorithm for Rich Global PID
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   12/12/2002
 */

class RichGlobalPIDTrTrackSel : public RichGlobalPIDAlgBase {

public:

  /// Standard constructor
  RichGlobalPIDTrTrackSel( const std::string& name,
                           ISvcLocator* pSvcLocator );

  virtual ~RichGlobalPIDTrTrackSel(); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

  // Private methods
private:

  /// Clean up after event abortion
  void deleteEvent();

  /// Determine the global PID Status of a track.
  /// Determines how the track will be used in the global likelihood
  Rich::GlobalPID::TkQuality trackStatus( RichRecTrack * track );

  // Private data members
private: 

  /// Location of processing status object in TES
  std::string m_procStatLocation;

  IRichExpectedTrackSignal * m_tkSignal; ///< Pointer to RichExpectedTrackSignal
  IRichTrackCreator * m_trackCr;         ///< Pointer to RichTrackCreator
  
  // Selection cuts
  double m_minPhysPtot; ///< Minimum momentum for physics quality tracks
  double m_minLLPtot;   ///< Minimum momentum for use in LL calculation
  bool m_resetToPion;   ///< Reset all track hypotheses to pion, ignoring initial types

  /// Maximum number of Tracks with RICH information
  int m_maxUsedTracks;

  /// Maximum total number of input TrStoredTracks
  int m_maxInputTracks;

  /// Track selector
  RichTrackSelector m_trSelector;

};

#endif // RICHRECALGS_RICHGLOBALPIDTRTRACKSEL_H
