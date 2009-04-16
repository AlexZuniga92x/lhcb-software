// $Id: CopyParticle2MCRelations.h,v 1.3 2009-04-16 07:42:16 jpalac Exp $
#ifndef COPYPARTICLE2MCRELATIONS_H 
#define COPYPARTICLE2MCRELATIONS_H 1

// Include files
// from MicroDST
#include "MicroDST/RelationsClonerAlg.h"
#include <MicroDST/ICloneMCParticle.h>
#include "MicroDST/BindType2ClonerDef.h"
// from DaVinci
#include "Kernel/Particle2MCParticle.h"

/** @class CopyParticle2MCRelations CopyParticle2MCRelations.h
 *  
 *  MicroDSTAlgorithm to clone the associations between an LHCb::Particle
 *  and LHCb::MCParticles. 
 *  It inherits the std::string properties InputLocation and OutputPrefix from
 *  MicroDSTCommon. The associations are taken from the TES location 
 *  defined by InputLocation, and are cloned and put in TES location 
 *  "/Event" + OutputPrefix + InputLocation. If InputLocation already contains
 *  a leading "/Event" it is removed.
 *  The associations are typically in a Particle2MCParticle::WTable in a TES 
 *  location like "/Event/Phys/SomeParticleMakingAlg/P2MCPRelations".
 *  The algorithm picks up this relations table, which relates LHCb::Particles
 *  from "/Event/Phys/SomaParticleMakingAlg/Particles" to LHCb::MCParticles.
 *  For each LHCb::Particle to LHCb::MCParticle association, it picks up the 
 *  corresponding <b>previously cloned</b> LHCb::Particle from 
 *  "/Event/MyLocation/Phys/SomeParticleMakerAlg/Particles", and clones the 
 *  associated LHCb::MCParticles. If the LHCb::MCParticle has not been 
 *  previously cloned, no association is cloned or stored. The algorithm
 *  constructs a new Particle2MCParticle::WTable relating the clones and 
 *  stored it in a TES location as described above.
 *
 *  <b>Important conditions</b>: This algorithm relies on the LHCb::Particle
 *  set in the associations to have been previously cloned by CopyParticles,
 *  and the existence of the Particle2MCParticle::WTable associating the origin
 *  LHCb::Particles to LHCb::MCParticles
 *
 *  <b>Example</b>: Clone LHCb::Particle -> LHCb::MCParticle associations
 *  from "/Event/Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/P2MCPRelations"
 *  to "/Event/MyLocation/Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/P2MCPRelations"
 *  @code
 *  mySeq = GaudiSequencer('SeqDC06selBd2Jpsi2MuMu_Kst2KPi')
 *  copyP2MCP = CopyParticle2MCRelations()
 *  mySeq.Members += [copyP2MCP]
 *  copyP2MCP.InputLocation = "Phys/DC06selBd2Jpsi2MuMu_Kst2KPi/P2MCPRelations";
 *  copyP2MCP.OutputPrefix = "MyLocation";
 *  @endcode
 *
 *  @author Juan PALACIOS juan.palacios@nikhef.nl
 *  @date   2009-04-15
 */
//=============================================================================
template <> struct BindType2Cloner<Particle2MCParticle::WTable> 
{
  typedef LHCb::MCParticle toType;
  typedef ICloneMCParticle toCloner;
};
//=============================================================================
template<> struct Defaults<Particle2MCParticle::WTable>
{
  const static std::string clonerType;
};
const std::string Defaults<Particle2MCParticle::WTable>::clonerType = "MCParticleCloner";
//=============================================================================
template<> struct Location<Particle2MCParticle::WTable>
{
  const static std::string Default;
};
const std::string Location<Particle2MCParticle::WTable>::Default = "NO DEFAULT LOCATION";
//=============================================================================
typedef MicroDST::RelationsClonerAlg<Particle2MCParticle::WTable> CopyParticle2MCRelations;
DECLARE_NAMED_ALGORITHM_FACTORY( CopyParticle2MCRelations, CopyParticle2MCRelations )
#endif // COPYPARTICLE2MCRELATIONS_H
