// $Id: CompositeParticle2MCLinks.h,v 1.10 2004-08-03 15:32:59 phicharp Exp $
#ifndef CompositeParticle2MCLinks_H 
#define CompositeParticle2MCLinks_H 1

// Include files
// from STL
#include <string>

// from Gaudi
#include "GaudiKernel/IParticlePropertySvc.h"

// local
#include "DaVinciAssociators/Particle2MCAsct.h"
#include "DaVinciAssociators/Particle2MCLink.h"
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
  

private:
  // Properties
  bool                      m_allowExtraMCPhotons;
  bool                      m_inclusiveMode;
  bool                      m_skipResonances;
  double                    m_maxResonanceLifeTime;
  std::string               m_asctMethod;
  // Local variables
  IParticlePropertySvc*     m_ppSvc;
  Object2MCLink*            m_p2MCLink;
  Object2MCLink*            m_p2MCComp;
  int                       m_gamma;
  Particle2MCLink::Linker*  m_linkerTable;
  Particle2MCAsct::Table*   m_table;
  int                       m_nrel;
  int                       m_nass;
  // Private methods
  bool associateTree(const Particle *p, const MCParticle* m) ;

  bool addDaughters( const Particle* m,
                     std::vector<const Particle*>& daughters) const;
  bool addMCDaughters( const MCParticle* m,
                       std::vector<const MCParticle*>& daughters) const;  
  /*
  class associator : public std::unary_function<MCParticle*,bool> {
  public:
    associator(CompositeParticle2MCLinks* t,Particle* p,
               Particle2MCAsct::Table* table) 
      : m_t(t), m_p(p), m_table(table) {};
    bool operator()(const MCParticle* m) const {
      return m_t->associateTree( m_p, m);
    }
           
  private: 
    CompositeParticle2MCLinks *m_t;
    Particle *m_p;
    Particle2MCAsct::Table *m_table;
  };
  friend class associator;
  */
};
#endif // Particle2MCLinks_H
