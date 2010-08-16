//-----------------------------------------------------------------------------
/** @file CombinedParticleMaker.h
 *
 * Header file for Particle maker CombinedParticleMaker
 *
 * CVS Log :-
 * $Id: CombinedParticleMaker.h,v 1.21 2010-08-16 16:40:38 odescham Exp $
 *
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 * @date 2006-05-03
 */
//-----------------------------------------------------------------------------

#ifndef PARTICLEMAKER_COMBINEDPARTICLEMAKER_H
#define PARTICLEMAKER_COMBINEDPARTICLEMAKER_H 1

#include "ChargedParticleMakerBase.h"
// from Gaudi
#include "GaudiKernel/HashMap.h"

// from EventSys
#include "Event/ProtoParticle.h"

// Interfaces
#include "Kernel/IProtoParticleFilter.h"

/** @class CombinedParticleMaker CombinedParticleMaker.h
 *
 *  Particle maker to be used as DEFAULT unique one for charged
 *  Particles.
 *
 *  Uses all detector information available for a given hypothesis.
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date 2006-05-03
 *
 *  @todo Add error on measured momentum (from PDG ?)
 *  @todo Re-assess how the confidence level is calculated
 */

class CombinedParticleMaker : public ChargedParticleMakerBase {

public:

  /// Standard constructor
  CombinedParticleMaker( const std::string& name,ISvcLocator* pSvcLocator );

  virtual ~CombinedParticleMaker( ); ///< Destructor

  /// Make the particles
  virtual StatusCode makeParticles( LHCb::Particle::Vector & parts );

  /// Standard initialization of algorithm
  virtual StatusCode initialize();

  /// Standard finalization of algorithm
  virtual StatusCode finalize();

protected:

  /** Fill Particle Parameters from ProtoParticle given a ParticleProperty
   *  @param proto     Pointer to ProtoParticle
   *  @param pprop     Pointer to ParticleProperty
   *  @param particle  Pointer to Particle
   *  @return StatusCode
   */
  virtual StatusCode fillParticle
    ( const LHCb::ProtoParticle    * proto,
      const LHCb::ParticleProperty * pprop,
      LHCb::Particle               * particle ) const;
  
  /// Set the Particle confidence level
  virtual void setConfLevel
  ( const LHCb::ProtoParticle*    proto,
    const LHCb::ParticleProperty* pprop,
    LHCb::Particle*               particle ) const;

  /// Test the PID information consistency
  virtual void checkPIDInfo( const LHCb::ProtoParticle * proto ) const;

  std::string m_elProtoFilter; ///< The tool type to use for electron selection
  std::string m_muProtoFilter; ///< The tool type to use for muon selection
  std::string m_piProtoFilter; ///< The tool type to use for pion selection
  std::string m_kaProtoFilter; ///< The tool type to use for kaon selection
  std::string m_prProtoFilter; ///< The tool type to use for proton selection

  /// The protoFilter to be used
  IProtoParticleFilter* m_protoTool;

  
  /// Particle property
  const LHCb::ParticleProperty* m_partProp ;

  // tallies

  /// Simple utility tally class
  class TrackTally
  {
  public:
    /// Default constructor
    TrackTally() : totProtos(0), selProtos(0), el(0), mu(0), pi(0), ka(0), pr(0) { }
    unsigned long totProtos; ///< Total number of ProtoParticles considered
    unsigned long selProtos; ///< Total number of ProtoParticles selected
    unsigned long el;        ///< Total number of electrons created
    unsigned long mu;        ///< Total number of muons created
    unsigned long pi;        ///< Total number of pions created
    unsigned long ka;        ///< Total number of kaons created
    unsigned long pr;        ///< Total number of protons created
    /// Increment the count for the given particle type (as string)
    inline void addToType( const std::string & type )
    {
      if      ( "e+"  == type ) { ++el; }
      else if ( "mu+" == type ) { ++mu; }
      else if ( "pi+" == type ) { ++pi; }
      else if ( "K+"  == type ) { ++ka; }
      else if ( "p+"  == type ) { ++pr; }
    }
  };

  /// Map type containing tally for various track types
  typedef GaudiUtils::HashMap < const LHCb::Track::Types, TrackTally > TrackMap;

  /// Total number of tracks considered and selected
  TrackMap m_nTracks;

  /// Minimum percentage to include in final summary printout
  double m_minPercForPrint;

  /// Flag to turn on consistency checks on PID information
  bool m_testPIDinfo;

};

#endif // PARTICLEMAKER_COMBINEDPARTICLEMAKER_H
