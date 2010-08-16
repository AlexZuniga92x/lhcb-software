// $Id: ParticleMakerBase.h,v 1.5 2010-08-16 16:40:38 odescham Exp $
#ifndef PARTICLEMAKERBASE_H 
#define PARTICLEMAKERBASE_H 1

// Include files
// from STL
#include <string>

// from DaVinci
#include "Kernel/DVAlgorithm.h"
// PartProp
#include "Kernel/ParticleProperty.h" 
// Brem adder
#include "Kernel/IBremAdder.h"

/** @class ParticleMakerBase ParticleMakerBase.h
 *
 *  Base class for all Particle Maker Algorithms
 *
 *  @author P. Koppenburg
 *  @date   2009-04-21
 */
namespace
{
  std::string to_upper( const std::string& in )
  {
    std::string  out( in );
    std::transform( in.begin() , in.end() , out.begin () , ::toupper ) ;
    return out ;
  };
};

class ParticleMakerBase : public DVAlgorithm {
public:
  /// Standard constructor
  ParticleMakerBase( const std::string& name, ISvcLocator* pSvcLocator );

  virtual ~ParticleMakerBase( ); ///< Destructor

  virtual StatusCode execute();    ///< Algorithm execution

  virtual StatusCode initialize() ; ///< Finalize
  virtual StatusCode finalize(){return DVAlgorithm::finalize();} ; ///< Finalize

protected:
  /// The method that each implementation should implement
  virtual StatusCode makeParticles(LHCb::Particle::Vector& ) = 0 ;

  /// protoparticles
  const LHCb::ProtoParticles* protos(){
    const LHCb::ProtoParticles* pp = 0 ;
    if ( exist<LHCb::ProtoParticles>( m_input )){
      pp = get< LHCb::ProtoParticles > (m_input);
    } else {
      Exception("No ProtoParticles at "+m_input);
    }
    return pp ;
  }

  // BremStrahlung correction for electron
  void addBrem(LHCb::Particle* particle){
    bool ok = false;
    for( std::vector<std::string>::iterator p = m_addBremPhoton.begin();m_addBremPhoton.end() != p ; ++p){
      if( *p == m_pid){
        ok=true;
        break;
      }
    }
    
    if( !ok )return;
    if (  !m_brem->addBrem( particle ) )return;
    if (msgLevel(MSG::DEBUG)) debug() << " ------- BremStrahlung has been added to the particle " 
                                      << particle << " (PID=" << m_pid << ")" << endmsg;
    counter("Applying Brem-correction to " + Gaudi::Utils::toString(particle->particleID().pid()) )+=1;
  }
  


protected:

  /// ID of the particle 
  std::string             m_pid   ;
  /// ID of the anti-particle 
  std::string             m_apid  ;
  /// properties of particle
  const LHCb::ParticleProperty* m_pp    ;
  
  /// properties of anti-particle
  const LHCb::ParticleProperty* m_app   ;

  /// Input Location of protoparticles
  std::string m_input ;

  // list of PIDs for which BremStrahlung correction is activated
  std::vector<std::string> m_addBremPhoton;

private:
  /// Track selector tool
  IBremAdder* m_brem;

};
#endif // PARTICLEMAKERBASE
