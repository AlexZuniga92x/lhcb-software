// $Id: RichGeomEffPhotonTracing.h,v 1.4 2004-06-18 11:21:29 jonesc Exp $
#ifndef RICHRECTOOLS_RICHGEOMEFFPHOTONTRACING_H
#define RICHRECTOOLS_RICHGEOMEFFPHOTONTRACING_H 1

// from Gaudi
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/ParticleProperty.h"

// base class
#include "RichRecBase/RichRecToolBase.h"

// Event model
#include "Event/RichRecSegment.h"

// CLHEP
#include "CLHEP/Units/PhysicalConstants.h"

// interfaces
#include "RichRecBase/IRichCherenkovAngle.h"
#include "RichRecBase/IRichGeomEff.h"
#include "RichKernel/IRichRayTracing.h"

// GSL
#include "gsl/gsl_math.h"

/** @class RichGeomEffPhotonTracing RichGeomEffPhotonTracing.h
 *
 *  Tool to perform a full detailed calculation of the geometrical
 *  efficiency for a given RichRecSegment and mass hypothesis.
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */

class RichGeomEffPhotonTracing : public RichRecToolBase,
                                 virtual public IRichGeomEff {

public:

  /// Standard constructor
  RichGeomEffPhotonTracing( const std::string& type,
                            const std::string& name,
                            const IInterface* parent );

  /// Destructor
  virtual ~RichGeomEffPhotonTracing() {};

  /// Initialize method
  StatusCode initialize();

  /// Finalize method
  StatusCode finalize();

  /// Obtain geometrical efficiency for this track and hypothesis
  double geomEfficiency ( RichRecSegment * segment,
                          const Rich::ParticleIDType id ) const;

  /// Obtain scattered geometrical efficiency for this track and hypothesis
  double geomEfficiencyScat ( RichRecSegment * segment,
                              const Rich::ParticleIDType id ) const;

private: // Private data

  // Pointers to tool instances
  IRichRayTracing * m_rayTrace;
  IRichCherenkovAngle * m_ckAngle;

  /// Number of photons to use in geometrical efficiency calculation
  int m_nGeomEff;

  /// Number of photons to quit after in geometrical efficiency calculation
  /// if all so far have failed
  int m_nGeomEffBailout;

  /// Increment parameter for PD efficiencies
  double m_pdInc;

  /// Cherenkov phi incrementation
  double m_incPhi;

  typedef std::vector<double> AngleVector;
  /// Cache Sin(angle) for geometrical efficiency calculation
  AngleVector m_sinCkPhi;
  /// Cache Cos(angle) for geometrical efficiency calculation
  AngleVector m_cosCkPhi;

  /// Flat Randomn distribution between 0 and 1
  mutable Rndm::Numbers m_uniDist;

};

#endif // RICHRECTOOLS_RICHGEOMEFFPHOTONTRACING_H
