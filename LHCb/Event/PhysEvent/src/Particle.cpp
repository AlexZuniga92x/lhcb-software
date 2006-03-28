// $Id: Particle.cpp,v 1.15 2006-03-28 13:54:01 jpalac Exp $
// Include files 

// STD and STL
#include <iostream> // from Gaudi

// local
#include "Event/Particle.h"

#include "Kernel/MatrixManip.h"

//-----------------------------------------------------------------------------
// Implementation file for class : Particle
//                                 derived from class KeyedObjeect
//
// 19/03/2002 : Gloria Corti
// 25/07/2005 : P. Koppenburg
//-----------------------------------------------------------------------------

//=============================================================================
// Copy constructor
//=============================================================================
LHCb::Particle::Particle(const LHCb::Particle& part)
  : KeyedObject<int>()
  , m_particleID( part.particleID() )
  , m_measuredMass( part.measuredMass() )
  , m_measuredMassErr( part.measuredMassErr() )
  , m_momentum( part.momentum() )
  , m_referencePoint( part.referencePoint() )
  , m_momCovMatrix( part.momCovMatrix() )
  , m_posCovMatrix( part.posCovMatrix() )
  , m_posMomCovMatrix( part.posMomCovMatrix() )
  , m_extraInfo( part.extraInfo() )
{
}

//=============================================================================
// Assignment operator
//=============================================================================
LHCb::Particle& LHCb::Particle::operator=(const LHCb::Particle& orig) {
  
  // protect against self assignement
  if( this != &orig ) {
    m_particleID = orig.particleID();
    m_momentum = orig.momentum();
    m_referencePoint = orig.referencePoint();
    m_measuredMass = orig.measuredMass();
    m_measuredMassErr = orig.measuredMassErr();  
    m_posCovMatrix = orig.posCovMatrix();
    m_momCovMatrix = orig.momCovMatrix();
    m_posMomCovMatrix = orig.posMomCovMatrix();
  }
  return *this;
}
//=============================================================================
// Create and return the covariance matrix
//=============================================================================
Gaudi::SymMatrix7x7 LHCb::Particle::covMatrix() const 
{

  using namespace MatrixManip;

  Gaudi::Matrix7x7 full;

  full.Place_at(Standardize(m_posCovMatrix),0,0);
  full.Place_at(Standardize(m_momCovMatrix),4,4);
  full.Place_at(m_posMomCovMatrix,0,4);

  return Symmetrize(full);
	
}

