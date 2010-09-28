#ifndef COPYPARTICLE2TISTOSDECISIONS_H 
#define COPYPARTICLE2TISTOSDECISIONS_H 1

// Include files
// from Gaudi
#include "MicroDST/MicroDSTAlgorithm.h"

class ITriggerTisTos;

namespace LHCb {
  class HltDecReports;
}


/** @class CopyParticle2TisTosDecisions CopyParticle2TisTosDecisions.h
 *  
 *
 *  @author Juan Palacios
 *  @date   2010-09-28
 */
class CopyParticle2TisTosDecisions : public MicroDSTAlgorithm {
public: 
  /// Standard constructor
  CopyParticle2TisTosDecisions( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~CopyParticle2TisTosDecisions( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

private:

  void executeLocation(const std::string& particleLocation);

private:

  ITriggerTisTos* m_iTisTos;
  LHCb::HltDecReports* m_decReports;
  std::string m_decReportsLocation;
  
};
#endif // COPYPARTICLE2TISTOSDECISIONS_H
