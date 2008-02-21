
//-----------------------------------------------------------------------------
/** @file RichRayTraceCherenkovCone.cpp
 *
 *  Implementation file for tool : Rich::Rec::RayTraceCherenkovCone
 *
 *  CVS Log :-
 *  $Id: RichRayTraceCherenkovCone.cpp,v 1.25 2008-02-21 16:52:05 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

// local
#include "RichRayTraceCherenkovCone.h"

// Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/PhysicalConstants.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//-----------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( RayTraceCherenkovCone );

// Standard constructor
RayTraceCherenkovCone::RayTraceCherenkovCone( const std::string& type,
                                              const std::string& name,
                                              const IInterface* parent )
  : RichRecToolBase ( type, name, parent ),
    m_rayTrace      ( NULL ),
    m_ckAngle       ( NULL ),
    m_smartIDTool   ( NULL ),
    m_geomTool      ( NULL ),
    m_nBailout      ( Rich::NRadiatorTypes, 50 )
{
  // Define interface for this tool
  declareInterface<IRayTraceCherenkovCone>(this);
  // JOs
  declareProperty( "BailoutTries", m_nBailout );
}

StatusCode RayTraceCherenkovCone::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = RichRecToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Acquire instances of tools
  acquireTool( "RichRayTracing",     m_rayTrace );
  acquireTool( "RichCherenkovAngle", m_ckAngle  );
  acquireTool( "RichSmartIDTool",    m_smartIDTool, 0, true );
  acquireTool( "RichRecGeometry",    m_geomTool );

  info() << "# ray tracing attempts before bailout = " << m_nBailout << endreq;

  return sc;
}

// Trace a single photon
LHCb::RichTraceMode::RayTraceResult
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

  // Add a new point
  const Gaudi::XYZPoint & gP = m_photon.detectionPoint();
  ring->ringPoints().push_back
    ( LHCb::RichRecPointOnRing(gP,
                               m_geomTool->radCorrLocalPos(m_smartIDTool->globalToPDPanel(gP),
                                                           ring->richRecSegment()->trackSegment().radiator()),
                               m_photon.smartID(),
                               (LHCb::RichRecPointOnRing::Acceptance)(result),
                               m_photon.primaryMirror(),
                               m_photon.secondaryMirror()
                               )
      );

  return result;
}

StatusCode
RayTraceCherenkovCone::rayTrace ( LHCb::RichRecSegment * segment,
                                  const Rich::ParticleIDType id,
                                  std::vector<Gaudi::XYZPoint> & points,
                                  const unsigned int nPoints,
                                  const LHCb::RichTraceMode mode ) const
{
  return rayTrace( segment,
                   m_ckAngle->avgCherenkovTheta(segment, id),
                   points, nPoints, mode );
}

StatusCode
RayTraceCherenkovCone::rayTrace ( LHCb::RichRecSegment * segment,
                                  const double ckTheta,
                                  std::vector<Gaudi::XYZPoint> & points,
                                  const unsigned int nPoints,
                                  const LHCb::RichTraceMode mode ) const
{
  // make sure segment is valid
  if ( !segment ) Exception( "Null RichRecSegment pointer!" );

  // Do the ray-tracing
  return rayTrace ( segment->trackSegment().rich(),
                    segment->trackSegment().bestPoint(),
                    segment->trackSegment().bestMomentum(),
                    ckTheta, points, nPoints, mode );
}

StatusCode
RayTraceCherenkovCone::rayTrace ( LHCb::RichRecRing * ring,
                                  const unsigned int nPoints,
                                  const LHCb::RichTraceMode mode,
                                  const bool forceTracing ) const
{
  if ( !ring ) return Error( "Null RichRecRing pointer!" );
  //debug() << "RichRecRing has " << ring->ringPoints().size() << " ring points" << endreq;
  if ( !forceTracing && !(ring->ringPoints().empty()) ) return StatusCode::SUCCESS;
  if ( !ring->richRecSegment() )
  {
    return Warning( "RingRecRing has no associated segment. Cannot perform ray tracing" );
  }
  ring->ringPoints().clear();

  if ( ring->radius() > 0 )
  {
    ring->ringPoints().reserve(nPoints);

    // emission point
    const Gaudi::XYZPoint & emissionPoint = ring->richRecSegment()->trackSegment().bestPoint();

    // which rich and radiator
    const Rich::DetectorType rich = ring->richRecSegment()->trackSegment().rich();
    const Rich::RadiatorType rad  = ring->richRecSegment()->trackSegment().radiator();

    if ( msgLevel(MSG::DEBUG) )
    {
      debug() << "Ray tracing " << rich << " CK cone :-" << endreq;
      debug() << " -> emissionPoint = " << emissionPoint << endreq;
      debug() << " -> direction     = " << ring->richRecSegment()->trackSegment().bestMomentum() << endreq;
      debug() << " -> CK theta      = " << ring->radius() << endreq;
      debug() << " -> Av. photon en = " << ring->richRecSegment()->trackSegment().avPhotonEnergy() << endreq;
      debug() << " -> " << mode << endreq;
    }

    // cos and sin values
    const CosSinPhi::Vector & cosSinPhi = cosSinValues(nPoints);

    // set number of points
    ring->setNTotalPointSamples(nPoints);

    // loop around the ring
    unsigned int nOK(0), nPhot(0);
    const double cosTheta(std::cos(ring->radius()));
    const double sinTheta(std::sin(ring->radius()));
    for ( CosSinPhi::Vector::const_iterator iP = cosSinPhi.begin();
          iP != cosSinPhi.end(); ++iP, ++nPhot )
    {
      // Photon direction around loop
      const Gaudi::XYZVector photDir =
        ring->richRecSegment()->trackSegment().vectorAtCosSinThetaPhi( cosTheta,     sinTheta,
                                                                       (*iP).cosPhi, (*iP).sinPhi );
      
      // do the tracing for this photon
      const LHCb::RichTraceMode::RayTraceResult result =
        traceAphoton ( rich, ring, emissionPoint, photDir, mode );
      // count raytraces that are in HPD panel
      if ( result >= LHCb::RichTraceMode::InHPDPanel ) ++nOK;

      // bailout check
      if ( 0 == nOK && nPhot >= m_nBailout[rad] ) break;
    }

    // All was OK
    return StatusCode::SUCCESS;

  }
  else
  {
    // cannot perform ray-tracing
    return StatusCode::FAILURE;
  }

}

StatusCode
RayTraceCherenkovCone::rayTrace ( const Rich::DetectorType rich,
                                  const Gaudi::XYZPoint & emissionPoint,
                                  const Gaudi::XYZVector & direction,
                                  const double ckTheta,
                                  LHCb::RichRecRing * ring,
                                  const unsigned int nPoints,
                                  const LHCb::RichTraceMode mode,
                                  const bool forceTracing ) const
{
  if ( !ring ) return Error( "Null RichRecRing pointer!" );
  //debug() << "RichRecRing has " << ring->ringPoints().size() << " ring points" << endreq;
  if ( !forceTracing && !(ring->ringPoints().empty()) ) return StatusCode::SUCCESS;
  ring->ringPoints().clear();

  if ( ckTheta > 0 )
  {
    ring->ringPoints().reserve(nPoints);

    // Define rotation matrix
    // todo to allow the one from the track segment to be used.
    const Gaudi::XYZVector z = direction.unit();
    Gaudi::XYZVector y = z.Cross( Gaudi::XYZVector(0.,1.,0.) );
    y /= std::sqrt(y.Mag2());
    const Gaudi::XYZVector x = y.Cross(z);
    Gaudi::Rotation3D rotation(x,y,z);

    if ( msgLevel(MSG::DEBUG) )
    {
      debug() << "Ray tracing " << rich << " CK cone :-" << endreq;
      debug() << " -> emissionPoint = " << emissionPoint << endreq;
      debug() << " -> direction     = " << direction << endreq;
      debug() << " -> CK theta      = " << ckTheta << endreq;
      debug() << " -> " << mode << endreq;
    }

    // which rich and radiator
    const Rich::DetectorType rich = ring->richRecSegment()->trackSegment().rich();
    const Rich::RadiatorType rad  = ring->richRecSegment()->trackSegment().radiator();

    // cos and sin values
    const CosSinPhi::Vector & cosSinPhi = cosSinValues(nPoints);

    // set number of points
    ring->setNTotalPointSamples(nPoints);

    // loop around the ring
    const double sinCkTheta = std::sin(ckTheta);
    const double cosCkTheta = std::cos(ckTheta);
    unsigned int nOK(0), nPhot(0);
    for ( CosSinPhi::Vector::const_iterator iP = cosSinPhi.begin();
          iP != cosSinPhi.end(); ++iP, ++nPhot )
    {
      // Photon direction around loop
      const Gaudi::XYZVector photDir =
        rotation * Gaudi::XYZVector( sinCkTheta * (*iP).cosPhi,
                                     sinCkTheta * (*iP).sinPhi,
                                     cosCkTheta );

      // do the tracing for this photon
      const LHCb::RichTraceMode::RayTraceResult result =
        traceAphoton ( rich, ring, emissionPoint, photDir, mode );
      // count raytraces that are in HPD panel
      if ( result >= LHCb::RichTraceMode::InHPDPanel ) ++nOK;

      // bailout check
      if ( 0 == nOK && nPhot >= m_nBailout[rad] ) break;
    }

    // All was OK
    return StatusCode::SUCCESS;

  }
  else
  {
    // cannot perform ray-tracing
    return StatusCode::FAILURE;
  }

}

StatusCode
RayTraceCherenkovCone::rayTrace ( const Rich::DetectorType rich,
                                  const Gaudi::XYZPoint & emissionPoint,
                                  const Gaudi::XYZVector & direction,
                                  const double ckTheta,
                                  std::vector<Gaudi::XYZPoint> & points,
                                  const unsigned int nPoints,
                                  const LHCb::RichTraceMode mode ) const
{
  if ( ckTheta > 0 )
  {

    // Define rotation matrix
    const Gaudi::XYZVector z = direction.unit();
    Gaudi::XYZVector y = z.Cross( Gaudi::XYZVector(0.,1.,0.) );
    y /= std::sqrt(y.Mag2());
    const Gaudi::XYZVector x = y.Cross(z);
    Gaudi::Rotation3D rotation(x,y,z);

    if ( msgLevel(MSG::DEBUG) )
    {
      debug() << "Ray tracing " << rich << " CK cone :-" << endreq;
      debug() << " -> emissionPoint = " << emissionPoint << endreq;
      debug() << " -> direction     = " << direction << endreq;
      debug() << " -> CK theta      = " << ckTheta << endreq;
      debug() << " -> " << mode << endreq;
    }

    // cos and sin values
    const CosSinPhi::Vector & cosSinPhi = cosSinValues(nPoints);

    // loop around the ring
    const double sinCkTheta = std::sin(ckTheta);
    const double cosCkTheta = std::cos(ckTheta);
 
    for ( CosSinPhi::Vector::const_iterator iP = cosSinPhi.begin();
          iP != cosSinPhi.end(); ++iP )
    {

      // Photon direction around loop
      const Gaudi::XYZVector photDir =
        rotation * Gaudi::XYZVector( sinCkTheta * (*iP).cosPhi,
                                     sinCkTheta * (*iP).sinPhi,
                                     cosCkTheta );

      // Ray trace to detector plane
      Gaudi::XYZPoint hitPoint;
      const LHCb::RichTraceMode::RayTraceResult result =
        m_rayTrace->traceToDetector( rich,
                                     emissionPoint,
                                     photDir,
                                     hitPoint,
                                     mode );
      if ( mode.traceWasOK(result) )
      {
        points.push_back( hitPoint );
      }

    }

    return StatusCode::SUCCESS;

  }
  else
  {
    // cannot perform ray-tracing
    return StatusCode::FAILURE;
  }

}

void RayTraceCherenkovCone::fillCosSinValues( CosSinPhi::Vector & vect,
                                              const unsigned int nPoints ) const
{
  // Set up cached parameters for photon tracing
  vect.clear();
  const double incPhi = Gaudi::Units::twopi / static_cast<double>(nPoints);
  double ckPhi = 0.0;
  for ( unsigned int iPhot = 0; iPhot < nPoints; ++iPhot, ckPhi+=incPhi )
  {
    vect.push_back( CosSinPhi( std::cos(ckPhi), std::sin(ckPhi) ) );
  }
}
