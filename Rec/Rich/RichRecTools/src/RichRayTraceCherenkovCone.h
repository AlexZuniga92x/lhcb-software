
//-----------------------------------------------------------------------------
/** @file RichRayTraceCherenkovCone.h
 *
 *  Header file for tool : Rich::Rec::RayTraceCherenkovCone
 *
 *  CVS Log :-
 *  $Id: RichRayTraceCherenkovCone.h,v 1.17 2008-02-17 19:26:22 jonrob Exp $
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
#include "RichKernel/RichMap.h"

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

    private:

      /** @class CosSinPhi RichRayTraceCherenkovCone.h
       *
       *  Utility class to cache cos and sin values
       *
       *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
       *  @date   17/02/2008
       */
      class CosSinPhi
      {
      public:
        typedef std::vector<CosSinPhi> Vector;
      public:
        CosSinPhi( const double _cosPhi = 0,
                   const double _sinPhi = 0 )
          : cosPhi(_cosPhi), sinPhi(_sinPhi) { }
      public:
        double cosPhi; ///< Cos(CK phi)
        double sinPhi; ///< Sin(CK phi)
      };

      /// Returns the cos and sin phi values for the given number of points
      inline const CosSinPhi::Vector & cosSinValues( const unsigned int nPoints ) const
      {
        CosSinPhi::Vector & vect = m_cosSinPhi[nPoints];
        if ( vect.empty() ) fillCosSinValues( vect, nPoints );
        return vect;
      }

      /// Fill the cosna nd sin vector for the given number of points
      void fillCosSinValues( CosSinPhi::Vector & vect, 
                             const unsigned int nPoints ) const;

    private:

      // Pointers to tool instances
      const IRayTracing * m_rayTrace;     ///< Optical ray tracing tool
      const ICherenkovAngle * m_ckAngle;  ///< Cherenkov angle calculator tool
      const ISmartIDTool * m_smartIDTool; ///< RichSmartID manipulation tool
      const IGeomTool * m_geomTool;       ///< Geometry tool

      /// Bailout number. If no ray tracing have worked after this number then give up
      std::vector<unsigned int> m_nBailout;

      /// Temporary working photon object
      mutable LHCb::RichGeomPhoton m_photon;

      /// Cache of sin and cos values
      mutable Rich::Map<unsigned int,CosSinPhi::Vector> m_cosSinPhi;

    };

  }
}

#endif // RICHRECTOOLS_RichRayTraceCherenkovCone_H
