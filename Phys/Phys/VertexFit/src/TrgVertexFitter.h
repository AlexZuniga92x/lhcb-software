// $Id: TrgVertexFitter.h,v 1.7 2007-10-11 17:11:40 pkoppenb Exp $
#ifndef TRGVERTEXFITTER_H 
#define TRGVERTEXFITTER_H 1

// Include files
// from Gaudi
#include "GaudiAlg/GaudiTool.h"
#include "Kernel/IVertexFit.h"            // Interface

/** @class TrgVertexFitter TrgVertexFitter.h
 *  
 *  Fast unconstrained vertex fitter that works only with particles 
 *  made from TrgTracks and assumes cylindrical errors and straight 
 *  trajectories
 *
 *  @author Hugo Ruiz Perez
 *  @date   2005-01-31
 */
class TrgVertexFitter : public GaudiTool, virtual public IVertexFit {
public: 
  /// Standard constructor
  TrgVertexFitter( const std::string& type, 
                   const std::string& name,
                   const IInterface* parent);

  StatusCode initialize();

  /// Method to fit a vertex 
  StatusCode fit( const LHCb::Particle::ConstVector&,  LHCb::Vertex& ) const ;  

  /// Method to fit a vertex returning a Particle (that should already know its PID)
  StatusCode fit( const LHCb::Particle::ConstVector&,  LHCb::Particle&, LHCb::Vertex& ) const ; 

  virtual ~TrgVertexFitter( ); ///< Destructor


  StatusCode reFit( LHCb::Particle& particle ) const;
  
  StatusCode combine( const LHCb::Particle::ConstVector& daughters , 
                      LHCb::Particle&        mother   , 
                      LHCb::Vertex&          vertex   ) const;

  /// add not active for fast vertex fitter
  StatusCode add(const LHCb::Particle* p,
                 LHCb::Vertex& v) const;
  
  /// remove not active for fast vertex fitter
  StatusCode remove(const LHCb::Particle* p,
                    LHCb::Vertex& v) const;

private:

  StatusCode doFit( const LHCb::Particle::ConstVector& partsToFit,  
                    LHCb::Vertex& V) const ;

  StatusCode vertexPositionAndError(const double& AX, 
                                    const double& BX, 
                                    const double& CX,
                                    const double& DX, 
                                    const double& EX,
                                    const double& AY, 
                                    const double& BY, 
                                    const double& CY,
                                    const double& DY, 
                                    const double& EY,
                                    double& vX, 
                                    double& vY, 
                                    double& vZ, 
                                    LHCb::Vertex& V) const ;

  bool isResonance(const LHCb::Particle& P) const {
    if ( P.daughters().empty() ) return false ;
    const int pid = abs(P.particleID().pid()) ;
    return (!( pid==3122 || pid==3222 || pid==310 || // s
               pid==411 || pid==421 || pid==431 || pid==4122 || // c
               pid==511 || pid==521 || pid==531 || pid==541 || pid==5122 || // b
               pid==15 )) ;
    /// @todo The list of resonances should be obtained from particle property service, but that may be slowish
  };

private:

  /// Use daughers' tracks if composite is found
  bool m_useDaughters;
  
  int m_photonID; ///< Photon particle ID

  IParticleStuffer* m_stuffer ; ///< Particle stuffer

  double m_epsilon ; ///< smallest acceptable value for doubles

};
#endif // TRGVERTEXFITTER_H
