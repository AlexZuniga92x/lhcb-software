
//-----------------------------------------------------------------------------
/** @file RichRayTraceCherenkovCone.h
 *
 *  Header file for tool : Rich::Rec::RayTraceCherenkovCone
 *
 *  CVS Log :-
 *  $Id: RichRayTraceCherenkovCone.h,v 1.15 2007-11-26 17:22:52 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

#ifndef RICHRECTOOLS_RichRayTraceCherenkovCone_H
#define RICHRECTOOLS_RichRayTraceCherenkovCone_H 1

// base class
#include "RichRecBase/RichRecToolBase.h"

// interfaces
#include "RichRecBase/IRichRayTraceCherenkovCone.h"
#include "RichRecBase/IRichCherenkovAngle.h"
#include "RichKernel/IRichRayTracing.h"
#include "RichKernel/IRichSmartIDTool.h"
#include "RichRecBase/IRichRecGeomTool.h"

// Event
#include "Event/RichRecRing.h"
#include "Event/RichRecSegment.h"

// RichKernel
#include "RichKernel/FastMaths.h"

namespace Rich
{
  namespace Rec
  {

    //-----------------------------------------------------------------------------
    /** @class RayTraceCherenkovCone RichRayTraceCherenkovCone.h
     *
     *  Tool to ray trace cherenkov photons in a cone around a given
     *  RichRecSegment direction, at the given angle or the angle correct
     *  for a given mass hypothesis.
     *
     *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
     *  @date   15/03/2002
     *
     *  @todo Find a way to merge the functionality of the similar ray tracing methods
     */
    //-----------------------------------------------------------------------------

    class RayTraceCherenkovCone : public Rich::Rec::ToolBase,
                                  virtual public IRayTraceCherenkovCone
    {

    public: // Methods for Gaudi Framework

      /// Standard constructor
      RayTraceCherenkovCone( const std::string& type,
                             const std::string& name,
                             const IInterface* parent );

      /// Destructor
      virtual ~RayTraceCherenkovCone(){}

      // Initialize method
      StatusCode initialize();


    public: // methods (and doxygen comments) inherited from public interface

      // Ray trace the Cherenkov cone for the given segment and mass hypothesis to the detector plane
      StatusCode rayTrace ( LHCb::RichRecSegment * segment,
                            const Rich::ParticleIDType id,
                            std::vector<Gaudi::XYZPoint> & points,
                            const unsigned int nPoints,
                            const LHCb::RichTraceMode mode = LHCb::RichTraceMode() ) const;

      // Ray trace the Cherenkov cone for the given segment and cherenkov angle to the detector plane
      StatusCode rayTrace ( LHCb::RichRecSegment * segment,
                            const double ckTheta,
                            std::vector<Gaudi::XYZPoint> & points,
                            const unsigned int nPoints,
                            const LHCb::RichTraceMode mode = LHCb::RichTraceMode() ) const;

      // Ray trace the Cherenkov cone using the given emission point, direction and Cherenkov angle
      StatusCode rayTrace ( const Rich::DetectorType rich,
                            const Gaudi::XYZPoint & emissionPoint,
                            const Gaudi::XYZVector & direction,
                            const double ckTheta,
                            std::vector<Gaudi::XYZPoint> & points,
                            const unsigned int nPoints,
                            const LHCb::RichTraceMode mode = LHCb::RichTraceMode() ) const;

      // Ray trace the Cherenkov cone for the given ring to the detector plane
      StatusCode rayTrace ( LHCb::RichRecRing * ring,
                            const unsigned int nPoints,
                            const LHCb::RichTraceMode mode = LHCb::RichTraceMode(),
                            const bool forceTracing = false ) const;

      // Ray trace the Cherenkov cone for the given ring to the detector plane
      StatusCode rayTrace ( const Rich::DetectorType rich,
                            const Gaudi::XYZPoint & emissionPoint,
                            const Gaudi::XYZVector & direction,
                            const double ckTheta,
                            LHCb::RichRecRing * ring,
                            const unsigned int nPoints,
                            const LHCb::RichTraceMode mode = LHCb::RichTraceMode(),
                            const bool forceTracing = false ) const;

    private: // methods

      // Trace a single photon
      LHCb::RichTraceMode::RayTraceResult traceAphoton ( const Rich::DetectorType rich,
                                                         LHCb::RichRecRing * ring,
                                                         const Gaudi::XYZPoint & emissionPoint,
                                                         const Gaudi::XYZVector & photDir,
                                                         const LHCb::RichTraceMode mode ) const;

    private: // data

      // Pointers to tool instances
      const IRayTracing * m_rayTrace;     ///< Optical ray tracing tool
      const ICherenkovAngle * m_ckAngle;  ///< Cherenkov angle calculator tool
      const ISmartIDTool * m_smartIDTool; ///< RichSmartID manipulation tool
      const IGeomTool * m_geomTool;       ///< Geometry tool

      /// Bailout number. If no ray tracing have worked after this number then give up
      std::vector<unsigned int> m_nBailout;

      // Temporary working photon object
      mutable LHCb::RichGeomPhoton m_photon;

    };

    // Trace a single photon
    inline LHCb::RichTraceMode::RayTraceResult 
    RayTraceCherenkovCone::traceAphoton ( const Rich::DetectorType rich,
                                          LHCb::RichRecRing * ring,
                                          const Gaudi::XYZPoint & emissionPoint,
                                          const Gaudi::XYZVector & photDir,
                                          const LHCb::RichTraceMode mode ) const
    {
      // do the ray tracing
      const LHCb::RichTraceMode::RayTraceResult result =
        m_rayTrace->traceToDetector( rich, emissionPoint, photDir, m_photon, mode, Rich::top,  
                                     ring->richRecSegment()->trackSegment().avPhotonEnergy() );
      // Add hit if tracing was OK
      //if ( result != LHCb::RichTraceMode::RayTraceFailed )
      {
        // Add a new point
        const Gaudi::XYZPoint & gP = m_photon.detectionPoint();
        ring->ringPoints().push_back
          ( LHCb::RichRecPointOnRing(gP,
                                     //m_smartIDTool->globalToPDPanel(gP),
                                     m_geomTool->radCorrLocalPos(m_smartIDTool->globalToPDPanel(gP),
                                                                 ring->richRecSegment()->trackSegment().radiator()),
                                     m_photon.smartID(),
                                     (LHCb::RichRecPointOnRing::Acceptance)(result) ) 
            );
      }
      return result;
    }

  }
}

#endif // RICHRECTOOLS_RichRayTraceCherenkovCone_H
