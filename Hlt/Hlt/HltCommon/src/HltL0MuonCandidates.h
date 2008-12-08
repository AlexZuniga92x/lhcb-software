// $Id: HltL0MuonCandidates.h,v 1.1 2008-12-08 12:14:05 graven Exp $
#ifndef HLTCOMMON_HLTL0MUONPREPARE_H 
#define HLTCOMMON_HLTL0MUONPREPARE_H 1

// Include files
// from Gaudi
#include "HltBase/HltAlgorithm.h"
#include "HltBase/HltSelectionContainer.h"
#include "HltBase/IMuonSeedTool.h"
#include "Event/L0MuonCandidate.h"

/** @class HltHadAlleyPreTrigger HltHadAlleyPreTrigger.h
 *  
 *
 *  @author Jose Angel Hernando Morata
 *  @date   2006-07-28
 */
class HltL0MuonCandidates : public HltAlgorithm {
public: 
  /// Standard constructor
  HltL0MuonCandidates( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~HltL0MuonCandidates( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution


private:
  Hlt::SelectionContainer2<LHCb::Track,LHCb::L0MuonCandidate> m_selection;

  std::string m_l0Location;  
  std::string m_l0Channel;  
  IMuonSeedTool* m_maker;

  bool checkClone(LHCb::L0MuonCandidate* muon);

};
#endif // HLTHADALLEYPRETRIGGER_H
