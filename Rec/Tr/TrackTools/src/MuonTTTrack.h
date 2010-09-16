// $Id: $
#ifndef MUONTTTRACK_H 
#define MUONTTTRACK_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "MuonInterfaces/MuonTrack.h" 
#include "MuonInterfaces/MuonHit.h" 
#include "MuonInterfaces/IMuonTrackMomRec.h"
#include "MuonInterfaces/IMuonTrackRec.h"
#include "Event/Track.h"
#include "Event/Particle.h"

#include "MCInterfaces/IMuonPad2MCTool.h"
#include "Event/MCParticle.h"
#include "Event/State.h"
#include "Event/RecVertex.h"

#include "TrackInterfaces/ITrackFitter.h"
#include "TrackInterfaces/ITrackExtrapolator.h"

#include "TrackInterfaces/IAddTTClusterTool.h" 
#include "PatKernel/PatTTHit.h"

#include "Kernel/LHCbID.h"
#include "MCInterfaces/ILHCbIDsToMCParticles.h"

#include "GaudiKernel/Chrono.h"


/** @class MuonTTTrack MuonTTTrack.h
 *  
 * \brief  Make a MuonTTTrack: Get muon standalone track, add TT hits, refit
 * 
 * Parameters:
 * - ToolName: Name for the tool that makes muon standalone track.
 * - Extrapolator: Name for the track extrapolator.
 * - MC: To enable MC association.
 * - FillMuonStubInfo: Fill parameters of muon stub in info fields of track;
 * - OutputLocation: The location the tracks should be written to.
 *
 *  @author Michel De Cian
 *  @date   2010-09-14
 */

class MuonTTTrack : public GaudiAlgorithm {
public: 
  /// Standard constructor
  MuonTTTrack( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~MuonTTTrack( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

protected:

private:

  // -- Methods
  StatusCode fillPVs(std::vector<double>& PVPos);
  void fillMuonStubInfo(LHCb::Track* track, MuonTrack* muTrack);
  StatusCode iterateToPV(LHCb::Track* track, LHCb::State& finalState, std::vector<double> PVPos, double qOverP);
  LHCb::MCParticle* assocMCParticle(const std::vector<MuonHit*> muonHits);
  bool isIDOnTrack(LHCb::LHCbID id, LHCb::MCParticle* mcp);

  // -- Variables
  std::string m_outputLoc;
  std::string m_extrapolatorName;
  bool m_MC;
  bool m_addTTHits;
  bool m_fillMuonStubInfo;
  std::string m_trackToolName;
  const LHCb::MuonPIDs* m_muonPids;
  std::map<const LHCb::Track*, const LHCb::Track*> m_muonMap;

  // -- Tools
  IAddTTClusterTool* m_ttHitAddingTool;
  ILHCbIDsToMCParticles* m_lhcbid2mcparticles;
  IMuonPad2MCTool* m_muonPad2MC;
  IMuonTrackRec* m_trackTool;
  IMuonTrackMomRec* m_momentumTool;
  ITrackExtrapolator*   m_extrapolator;
  ITrackFitter*         m_trackFitter;

  
};
#endif // GETMUONTRACK_H
