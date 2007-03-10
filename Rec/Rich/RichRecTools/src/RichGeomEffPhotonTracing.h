
//-----------------------------------------------------------------------------
/** @file RichGeomEffPhotonTracing.h
 *
 *  Header file for tool : Rich::Rec::GeomEffPhotonTracing
 *
 *  CVS Log :-
 *  $Id: RichGeomEffPhotonTracing.h,v 1.21 2007-03-10 13:19:20 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

#ifndef RICHRECTOOLS_RICHGEOMEFFPHOTONTRACING_H
#define RICHRECTOOLS_RICHGEOMEFFPHOTONTRACING_H 1

// STL
#include <algorithm>

// from Gaudi
#include "GaudiKernel/RndmGenerators.h"
#include "GaudiKernel/IRndmGenSvc.h"
#include "GaudiKernel/ParticleProperty.h"
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/PhysicalConstants.h"

// base class
#include "RichRecBase/RichRecToolBase.h"

// Event model
#include "Event/RichRecSegment.h"

// interfaces
#include "RichRecBase/IRichCherenkovAngle.h"
#include "RichRecBase/IRichGeomEff.h"
#include "RichKernel/IRichRayTracing.h"

// RichDet
#include "RichDet/DeRichSystem.h"

// GSL
#include "gsl/gsl_math.h"

// RichKernel
#include "RichKernel/RichGeomFunctions.h"

namespace Rich
{
  namespace Rec
  {

    //-----------------------------------------------------------------------------
    /** @class GeomEffPhotonTracing RichGeomEffPhotonTracing.h
     *
     *  Tool to perform a full detailed calculation of the geometrical
     *  efficiency for a given RichRecSegment and mass hypothesis.
     *
     *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
     *  @date   15/03/2002
     *
     *  @todo  Remove the use of random numbers if at all possible
     */
    //-----------------------------------------------------------------------------

    class GeomEffPhotonTracing : public Rich::Rec::ToolBase,
                                 virtual public IGeomEff
    {

    public: // Methods for Gaudi Framework

      /// Standard constructor
      GeomEffPhotonTracing( const std::string& type,
                            const std::string& name,
                            const IInterface* parent );

      /// Destructor
      virtual ~GeomEffPhotonTracing() {};

      // Initialize method
      StatusCode initialize();

      // Finalize method
      StatusCode finalize();

    public: // methods (and doxygen comments) inherited from public interface

      // Obtain geometrical efficiency for this track and hypothesis
      double geomEfficiency ( LHCb::RichRecSegment * segment,
                              const Rich::ParticleIDType id ) const;

      // Obtain scattered geometrical efficiency for this track and hypothesis
      double geomEfficiencyScat ( LHCb::RichRecSegment * segment,
                                  const Rich::ParticleIDType id ) const;

    private: // Private data

      // Pointers to tool instances
      const IRayTracing * m_rayTrace;    ///< Ray tracking tool
      const ICherenkovAngle * m_ckAngle; ///< Cherenkov angle tool

      const DeRichSystem * m_richSys;    ///< RICH detector system object

      /// Number of photons to use in geometrical efficiency calculation
      int m_nGeomEff;

      /** Number of photons to quit after in geometrical efficiency calculation
       *  if all so far have failed */
      int m_nGeomEffBailout;

      /// Increment parameter for PD efficiencies
      double m_pdInc;

      /// Flat random distribution between 0 and 1
      mutable Rndm::Numbers m_uniDist;

      /// Ray-tracing configuration object
      LHCb::RichTraceMode m_traceMode;

      /// Vector of sampling phi values around the Cherekov ring
      std::vector<double> m_phiValues;

      /// Flag to turn on or off the explicit checking of the HPD status
      bool m_hpdCheck;

      /// Flag to turn on or off checking of intersections with beampipe
      bool m_checkBeamPipe;

    };

  }
}

#endif // RICHRECTOOLS_RICHGEOMEFFPHOTONTRACING_H
