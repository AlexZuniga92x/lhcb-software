
//-----------------------------------------------------------------------------
/** @file RichParticleProperties.cpp
 *
 *  Implementation file for tool : RichParticleProperties
 *
 *  CVS Log :-
 *  $Id: RichParticleProperties.cpp,v 1.1 2005-10-13 16:11:07 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

// local
#include "RichParticleProperties.h"

//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
static const  ToolFactory<RichParticleProperties>          s_factory ;
const        IToolFactory& RichParticlePropertiesFactory = s_factory ;

// Standard constructor
RichParticleProperties::RichParticleProperties ( const std::string& type,
                                                 const std::string& name,
                                                 const IInterface* parent )
  : RichToolBase( type, name, parent ) 
{

  // declare interface
  declareInterface<IRichParticleProperties>(this);

}

StatusCode RichParticleProperties::initialize() 
{

  // Sets up various tools and services
  const StatusCode sc = RichToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Acquire instances of tools
  const IRichRefractiveIndex * refIndex;
  acquireTool( "RichRefractiveIndex", refIndex );

  // Retrieve particle property service
  IParticlePropertySvc * ppSvc = svc<IParticlePropertySvc>( "ParticlePropertySvc", true );

  // Retrieve particle masses
  m_particleMass[Rich::Electron] = ppSvc->find("e+" )->mass()/MeV;
  m_particleMass[Rich::Muon]     = ppSvc->find("mu+")->mass()/MeV;
  m_particleMass[Rich::Pion]     = ppSvc->find("pi+")->mass()/MeV;
  m_particleMass[Rich::Kaon]     = ppSvc->find("K+" )->mass()/MeV;
  m_particleMass[Rich::Proton]   = ppSvc->find("p+" )->mass()/MeV;

  // cache squares of masses
  m_particleMassSq[Rich::Electron] = gsl_pow_2( m_particleMass[Rich::Electron] );
  m_particleMassSq[Rich::Muon]     = gsl_pow_2( m_particleMass[Rich::Muon]     );
  m_particleMassSq[Rich::Pion]     = gsl_pow_2( m_particleMass[Rich::Pion]     );
  m_particleMassSq[Rich::Kaon]     = gsl_pow_2( m_particleMass[Rich::Kaon]     );
  m_particleMassSq[Rich::Proton]   = gsl_pow_2( m_particleMass[Rich::Proton]   );

  // release service
  release(ppSvc);

  // Informational Printout
  debug() << " Particle masses (MeV/c^2)     = " << m_particleMass << endreq;

  // Setup momentum thresholds
  for ( int iRad = 0; iRad < Rich::NRadiatorTypes; ++iRad ) 
  {
    const Rich::RadiatorType rad = static_cast<Rich::RadiatorType>(iRad);
    debug() << " Particle thresholds (MeV/c^2) : " << rad << " : ";
    for ( int iHypo = 0; iHypo < Rich::NParticleTypes; ++iHypo ) 
    {
      const double index = refIndex->refractiveIndex(rad);
      m_momThres[iRad][iHypo] = m_particleMass[iHypo]/sqrt(index*index - 1.0);
      m_momThres2[iRad][iHypo] = m_momThres[iRad][iHypo]*m_momThres[iRad][iHypo];
      debug() << m_momThres[iRad][iHypo] << " ";
    }
    debug() << endreq;
  }
  
  // release tool
  releaseTool(refIndex);

  return sc;
}

StatusCode RichParticleProperties::finalize()
{
  // Execute base class method
  return RichToolBase::finalize();
}

double RichParticleProperties::beta( const double ptot,
                                     const Rich::ParticleIDType id ) const
{
  const double Esquare  = ptot*ptot + m_particleMassSq[id];
  return ( Esquare > 0 ? ptot/sqrt(Esquare) : 0 );
}

double RichParticleProperties::mass( const Rich::ParticleIDType id ) const
{
  return m_particleMass[id];
}

double RichParticleProperties::massSq( const Rich::ParticleIDType id ) const
{
  return m_particleMassSq[id];
}

double RichParticleProperties::thresholdMomentum( const Rich::ParticleIDType id,
                                                  const Rich::RadiatorType rad ) const
{
  return m_momThres[rad][id];
}

double RichParticleProperties::thresholdMomentumSq( const Rich::ParticleIDType id,
                                                    const Rich::RadiatorType rad ) const
{
  return m_momThres2[rad][id];
}
