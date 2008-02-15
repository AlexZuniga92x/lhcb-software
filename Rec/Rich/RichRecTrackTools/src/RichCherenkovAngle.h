
//-----------------------------------------------------------------------------
/** @file RichCherenkovAngle.h
 *
 *  Header file for tool : Rich::Rec::CherenkovAngle
 *
 *  CVS Log :-
 *  $Id: RichCherenkovAngle.h,v 1.2 2008-02-15 10:31:36 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

#ifndef RICHRECTOOLS_RICHCHERENKOVANGLE_H
#define RICHRECTOOLS_RICHCHERENKOVANGLE_H 1

// from Gaudi
#include "GaudiKernel/ParticleProperty.h"
#include "GaudiKernel/IParticlePropertySvc.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/PhysicalConstants.h"

// base class
#include "RichRecBase/RichRecToolBase.h"

// Event model
#include "Event/RichRecSegment.h"

// Rich Kernel
#include "RichKernel/BoostArray.h"

// interfaces
#include "RichKernel/IRichRayTracing.h"
#include "RichKernel/IRichSmartIDTool.h"
#include "RichRecBase/IRichCherenkovAngle.h"
#include "RichRecBase/IRichExpectedTrackSignal.h"
#include "RichKernel/IRichRefractiveIndex.h"
#include "RichKernel/IRichParticleProperties.h"

// GSL
#include "gsl/gsl_math.h"

namespace Rich
{
  namespace Rec
  {

    //-----------------------------------------------------------------------------
    /** @class CherenkovAngle RichCherenkovAngle.h
     *
     *  Tool calculating the expected Cherenkov angle
     *
     *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
     *  @date   15/03/2002
     */
    //-----------------------------------------------------------------------------

    class CherenkovAngle : public Rich::Rec::ToolBase,
                           virtual public ICherenkovAngle
    {

    public: // Methods for Gaudi Framework

      /// Standard constructor
      CherenkovAngle( const std::string& type,
                      const std::string& name,
                      const IInterface* parent );

      /// Destructor
      virtual ~CherenkovAngle() {}

      // Initialize method
      StatusCode initialize();

    public: // methods (and doxygen comments) inherited from public interface

      // Returns average Cherenkov angle for given particle hypothesis
      double avgCherenkovTheta( LHCb::RichRecSegment * segment,
                                const Rich::ParticleIDType id,
                                const bool useEmittedSpectrum = false ) const;

      // Returns average Cherenkov angle for the current mass hypothesis
      // Assigned to the segment
      double avgCherenkovTheta( LHCb::RichRecSegment * segment ) const;

      // Computes the nominal saturated Cherenkov angle for a given radiator medium
      double nominalSaturatedCherenkovTheta( const Rich::RadiatorType rad ) const;

      // Computes the average ring radius on the detector plane, in local HPD coordinates,
      // for the given mass hypothesis.
      double avCKRingRadiusLocal( LHCb::RichRecSegment * segment,
                                  const Rich::ParticleIDType id,
                                  const unsigned int nSamples = 6 ) const;

      // Computes the average ring radius on the detector plane, in local HPD coordinates,
      // for the given cherenkov angle
      double avCKRingRadiusLocal( LHCb::RichRecSegment * segment,
                                  const double ckTheta,
                                  const unsigned int nSamples = 6 ) const;

      // Computes the average ring radius on the detector plane, in local HPD coordinates,
      double satCKRingRadiusLocal( LHCb::RichRecSegment * segment,
                                   const unsigned int nSamples = 6 ) const;

    private: // methods

      /// Compute and store the average radii values for the mass hypotheses
      void computeRadii( LHCb::RichRecSegment * segment,
                         const unsigned int nSamples ) const;

    private: // Private data

      /// Pointer to RichExpectedTrackSignal interface
      const IExpectedTrackSignal * m_signal;

      /// Pointer to RichParticleProperties interface
      const IParticleProperties * m_richPartProp;

      /// Pointer to refractive index tool
      const IRefractiveIndex * m_refIndex;

      /// Pointer to the RichSmartID tool
      const ISmartIDTool * m_smartIDTool;

      /// Pointer to the ray tracking tool
      const IRayTracing * m_rayTrace;

      /// Storage for nominal saturated Cherenkov angles
      boost::array< double, Rich::NRadiatorTypes > m_nomCK;

      /// Particle ID types to consider in the photon creation checks
      Rich::Particles m_pidTypes;

    };

  }
}

#endif // RICHRECTOOLS_RICHCHERENKOVANGLE_H
