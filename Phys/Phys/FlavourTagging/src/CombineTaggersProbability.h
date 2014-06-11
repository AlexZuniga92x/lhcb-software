#ifndef COMBINETAGGERSPROBABILITY_H 
#define COMBINETAGGERSPROBABILITY_H 1
// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "GaudiKernel/AlgTool.h"
#include "ICombineTaggersTool.h"            // Interface

/** @class CombineTaggersProbability CombineTaggersProbability.h CombineTaggersProbability.h
 *
 *  v1.3
 *  @author Marco Musy
 *  @date   2006-10-02
 */

class CombineTaggersProbability : public GaudiTool,
                                  virtual public ICombineTaggersTool {
public:

  /// Standard constructor
  CombineTaggersProbability( const std::string& type,
                             const std::string& name,
                             const IInterface* parent );

  virtual ~CombineTaggersProbability( ); ///< Destructor

  /// Initialize
  StatusCode initialize();

  int combineTaggers(LHCb::FlavourTag& theTag, std::vector<LHCb::Tagger*>&, int signalType,  bool flag_nnetTaggers, bool flag_CharmTagger );

private:

  double m_ProbMin, m_ProbMin_OS;
  double m_P0_Cal_OS, m_P1_Cal_OS, m_Eta_Cal_OS;
  double m_P0_Cal_OSnnet, m_P1_Cal_OSnnet, m_Eta_Cal_OSnnet;

};
#endif // COMBINETAGGERSPROBABILITY_H
