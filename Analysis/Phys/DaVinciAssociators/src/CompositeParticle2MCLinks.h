// $Id: CompositeParticle2MCLinks.h,v 1.16 2007-11-02 00:38:14 xieyu Exp $
#ifndef CompositeParticle2MCLinks_H 
#define CompositeParticle2MCLinks_H 1

// Include files
// from STL
#include <string>

// from LHCb
#include "Kernel/IParticlePropertySvc.h"

// local
#include "Kernel/Particle2MCLinker.h"
#include "AsctAlgorithm.h"

/** @class CompositeParticle2MCLinks CompositeParticle2MCLinks.h
 *  
 *  @author Gerhard Raven
 *  @date   September 2002
 */
class CompositeParticle2MCLinks : public AsctAlgorithm {
  friend class AlgFactory<CompositeParticle2MCLinks>;
  
public:
  /// Standard constructor
  CompositeParticle2MCLinks(const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~CompositeParticle2MCLinks( ); ///< Destructor

  virtual StatusCode initialize();    ///< Algorithm initialization
  virtual StatusCode execute   ();    ///< Algorithm execution
  virtual StatusCode finalize  ();    ///< Algorithm finalization
  
  bool isResonance(const LHCb::Particle*) const;

private:
  // Properties
  bool                      m_allowExtraMCPhotons;
  bool                      m_inclusiveMode;
  bool                      m_skipResonances;
  double                    m_maxResonanceLifeTime;
  std::string               m_asctMethod;
  bool                      m_ignorePID;
  // Local variables
  LHCb::IParticlePropertySvc*   m_ppSvc;
  Object2MCLinker<>*            m_p2MCLink;
  Object2MCLinker<>*            m_p2MCComp;
  int                       m_gamma;
  Particle2MCLinker::Linker*  m_linkerTable;
  int                       m_nrel;
  int                       m_nass;
  // Private methods
  bool associateTree(const LHCb::Particle *p, const LHCb::MCParticle* m) ;

  bool addDaughters( const LHCb::Particle* m,
                     std::vector<const LHCb::Particle*>& daughters) const;
  bool addMCDaughters( const LHCb::MCParticle* m,
                       std::vector<const LHCb::MCParticle*>& daughters) const;
};
#endif // CompositeParticle2MCLinks_H
