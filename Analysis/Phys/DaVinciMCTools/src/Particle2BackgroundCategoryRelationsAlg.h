#ifndef PARTICLE2BACKGROUNDCATEGORYRELATIONSALG_H 
#define PARTICLE2BACKGROUNDCATEGORYRELATIONSALG_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiAlgorithm.h"
#include "Kernel/IBackgroundCategory.h"          // Interface
#include "Relations/Relations.h"
#include "Kernel/IPhysDesktop.h"

/** @class Particle2BackgroundCategoryRelationsAlg Particle2BackgroundCategoryRelationsAlg.h
 *  
 *  Algorithm which takes as input a TES container of Particles, gets the BackgroundCategory
 *  for each one, and writes a Relations table between these Particles and their
 *  BackgroundCategories to the TES. 
 *
 *  @author V. Gligorov
 *  @date   2009-11-30
 */

class Particle2BackgroundCategoryRelationsAlg : public GaudiAlgorithm {
public: 
  /// Standard constructor
  Particle2BackgroundCategoryRelationsAlg( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~Particle2BackgroundCategoryRelationsAlg( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization

private:

  std::string m_particleLocation;

  IBackgroundCategory* m_bkg;  
};
#endif // PARTICLE2BACKGROUNDCATEGORYRELATIONSALG_H
