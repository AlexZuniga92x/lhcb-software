// $Id: OTCluster2MCHitAlg.h,v 1.3 2002-08-07 15:55:06 jvantilb Exp $
#ifndef OTASSOCIATORS_OTCLUSTER2MCHITALG_H
#define OTASSOCIATORS_OTCLUSTER2MCHITALG_H 1

#include "Relations/IAssociator.h" 
#include "Relations/IRelation.h"
#include "Relations/Relation1D.h"

#include "GaudiKernel/Algorithm.h"

class OTCluster;
class MCHit;

/** @class OTCluster2MCHitAlg OTCluster2MCHitAlg.h
 *  
 *  Algorithm which makes the association from OTClusters to MCHits. This
 *  is used by the associator tool. There is a flag to make relations with 
 *  spillover hits as well. By default those are not associated.
 * 
 *  @author Jeroen van Tilburg
 *  @date   14/05/2002
 */

class OTCluster2MCHitAlg : public Algorithm {
    friend AlgFactory<OTCluster2MCHitAlg>;
  
public:

  /// Standard constructor
  OTCluster2MCHitAlg( const std::string& name, ISvcLocator* pSvcLocator );

  /// Destructor
  virtual ~OTCluster2MCHitAlg(); 

  /// initialize
  virtual StatusCode initialize();    

  /// execute
  virtual StatusCode execute();    

  /// finalize
  virtual StatusCode finalize();    

  /// find the MCHit given a OTCluster
  virtual StatusCode associateToTruth(const OTCluster* aCluster,
                                      MCHit*& aHit);

  /// path to put relation table
  std::string outputData() const;
 
protected:

private:

  std::string m_outputData; ///< path to put relation table
  bool m_spillOver;      ///< Flag to make relations with spillover hits as well
  MCHits* m_mcHits;      ///< Container of MCHits used to identify spillover

};

inline std::string OTCluster2MCHitAlg::outputData() const {
  return  m_outputData;
}


#endif // OTASSOCIATORS_OTCLUSTER2MCHITALG_H
