
//-----------------------------------------------------------------------------
/** @file RichRayTraceCherenkovCone.cpp
 *
 *  Implementation file for tool : Rich::Rec::RayTraceCherenkovCone
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

// local
#include "RichRayTraceCherenkovCone.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//-----------------------------------------------------------------------------

// Standard constructor
RayTraceCherenkovCone::RayTraceCherenkovCone( const std::string& type,
                                              const std::string& name,
                                              const IInterface* parent )
  : Rich::Rec::ToolBase ( type, name, parent )
{
  // Define interface for this tool
  declareInterface<IRayTraceCherenkovCone>(this);
  // JOs
  //                                                    Aero  R1Gas R2Gas
  declareProperty( "BailoutFraction", m_bailoutFrac = { 0.75, 0.75, 0.75 } );
}

// Destructor
RayTraceCherenkovCone::~RayTraceCherenkovCone() {}

StatusCode RayTraceCherenkovCone::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = Rich::Rec::ToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Acquire instances of tools
  acquireTool( "RichRayTracing",          m_rayTrace,    nullptr, true );
  acquireTool( "RichSmartIDTool",         m_smartIDTool, nullptr, true );
  acquireTool( "RichCherenkovAngle",      m_ckAngle  );
  acquireTool( "RichRecGeometry",         m_geomTool );
  acquireTool( "RichPhotonEmissionPoint", m_emissPoint );

  _ri_debug << "Ray tracing fraction before bailout = " << m_bailoutFrac << endmsg;

  return sc;
}

// Trace a single photon
LHCb::RichTraceMode::RayTraceResult
RayTraceCherenkovCone::traceAphoton ( const CosSinPhi& sinCosPhi,
                                      const Rich::DetectorType rich,
                                      LHCb::RichRecRing * ring,
                                      const Gaudi::XYZPoint & emissionPoint,
                                      const Gaudi::XYZVector & photDir,
                                      const LHCb::RichTraceMode mode ) const
{
  // do the ray tracing
  const auto result =
    m_rayTrace->traceToDetector( rich, emissionPoint, photDir, m_photon, 
                                 ring->richRecSegment()->trackSegment(), 
                                 mode, Rich::top ); // Note forced side is not used..

  // Add a new point
  const auto & gP = m_photon.detectionPoint();
  ring->ringPoints().emplace_back
    ( LHCb::RichRecPointOnRing(gP,
                               m_geomTool->radCorrLocalPos(m_smartIDTool->globalToPDPanel(gP),
                                                           ring->richRecSegment()->trackSegment().radiator()),
                               m_photon.smartID(),
                               (LHCb::RichRecPointOnRing::Acceptance)(result),
                               m_photon.primaryMirror(),
                               m_photon.secondaryMirror(),
                               sinCosPhi.phi
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

  // Get the emission point
  Gaudi::XYZPoint emissionPoint;
  m_emissPoint->emissionPoint( segment, emissionPoint );

  // Do the ray-tracing
  return rayTrace ( segment->trackSegment().rich(),
                    emissionPoint,
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
  //debug() << "RichRecRing has " << ring->ringPoints().size() << " ring points" << endmsg;
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
    Gaudi::XYZPoint emissionPoint;
    m_emissPoint->emissionPoint( ring->richRecSegment(), emissionPoint );

    // which rich and radiator
    const auto rich = ring->richRecSegment()->trackSegment().rich();
    const auto rad  = ring->richRecSegment()->trackSegment().radiator();

    if ( msgLevel(MSG::DEBUG) )
    {
      debug() << "Ray tracing " << rich << " CK cone :-" << endmsg;
      debug() << " -> emissionPoint = " << emissionPoint << endmsg;
      debug() << " -> direction     = " << ring->richRecSegment()->trackSegment().bestMomentum() << endmsg;
      debug() << " -> CK theta      = " << ring->radius() << endmsg;
      debug() << " -> Av. photon en = " << ring->richRecSegment()->trackSegment().avPhotonEnergy() << endmsg;
      debug() << " -> " << mode << endmsg;
    }

    // cos and sin values
    const auto & cosSinPhi = cosSinValues(nPoints);

    // set number of points
    ring->setNTotalPointSamples(nPoints);

    // Bailout number
    const auto nBailout = static_cast<unsigned int> ( m_bailoutFrac[rad] * nPoints );

    // compute sin and cos theta
    double cosTheta(0), sinTheta(0);
    vdt::fast_sincos(ring->radius(),sinTheta,cosTheta);

    // loop around the ring
    unsigned int nOK(0), nPhot(0);
    for ( auto iP = cosSinPhi.begin(); iP != cosSinPhi.end(); ++iP, ++nPhot )
    {
      // Photon direction around loop
      const auto photDir =
        ring->richRecSegment()->trackSegment().vectorAtCosSinThetaPhi( cosTheta,     sinTheta,
                                                                       (*iP).cosPhi, (*iP).sinPhi );
      
      // do the tracing for this photon
      const auto result =
        traceAphoton ( *iP, rich, ring, emissionPoint, photDir, mode );
      // count raytraces that are in HPD panel
      if ( result >= LHCb::RichTraceMode::InHPDPanel ) { ++nOK; }

      // bailout check
      if ( 0 == nOK && nPhot >= nBailout ) break;
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
  if ( !forceTracing && !(ring->ringPoints().empty()) ) return StatusCode::SUCCESS;

  ring->ringPoints().clear();

  if ( ckTheta > 0 )
  {
    ring->ringPoints().reserve(nPoints);

    // Define rotation matrix
    // todo to allow the one from the track segment to be used.
    const auto z = direction.unit();
    auto y = z.Cross( Gaudi::XYZVector(0.,1.,0.) );
    y /= std::sqrt(y.Mag2());
    const auto x = y.Cross(z);
    Gaudi::Rotation3D rotation(x,y,z);

    if ( msgLevel(MSG::DEBUG) )
    {
      debug() << "Ray tracing " << rich << " CK cone :-" << endmsg;
      debug() << " -> emissionPoint = " << emissionPoint << endmsg;
      debug() << " -> direction     = " << direction << endmsg;
      debug() << " -> CK theta      = " << ckTheta << endmsg;
      debug() << " -> " << mode << endmsg;
    }

    // which radiator
    const auto rad = ring->richRecSegment()->trackSegment().radiator();

    // cos and sin values
    const auto & cosSinPhi = cosSinValues(nPoints);

    // set number of points
    ring->setNTotalPointSamples(nPoints);

    // Bailout number
    const auto nBailout = static_cast<unsigned int> ( m_bailoutFrac[rad] * nPoints );

    // compute sin and cos theta
    double sinCkTheta(0), cosCkTheta(0);
    vdt::fast_sincos( ckTheta, sinCkTheta, cosCkTheta );

    // loop around the ring
    unsigned int nOK(0), nPhot(0);
    for ( auto iP = cosSinPhi.begin(); iP != cosSinPhi.end(); ++iP, ++nPhot )
    {
      // Photon direction around loop
      const auto photDir =
        rotation * Gaudi::XYZVector( sinCkTheta * (*iP).cosPhi,
                                     sinCkTheta * (*iP).sinPhi,
                                     cosCkTheta );

      // do the tracing for this photon
      const auto result =
        traceAphoton ( *iP, rich, ring, emissionPoint, photDir, mode );
      // count raytraces that are in HPD panel
      if ( result >= LHCb::RichTraceMode::InHPDPanel ) { ++nOK; }

      // bailout check
      if ( 0 == nOK && nPhot >= nBailout ) break;
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
    const auto z = direction.unit();
    auto y = z.Cross( Gaudi::XYZVector(0.,1.,0.) );
    y /= std::sqrt(y.Mag2());
    const auto x = y.Cross(z);
    Gaudi::Rotation3D rotation(x,y,z);

    if ( msgLevel(MSG::DEBUG) )
    {
      debug() << "Ray tracing " << rich << " CK cone :-" << endmsg;
      debug() << " -> emissionPoint = " << emissionPoint << endmsg;
      debug() << " -> direction     = " << direction << endmsg;
      debug() << " -> CK theta      = " << ckTheta << endmsg;
      debug() << " -> " << mode << endmsg;
    }

    // cos and sin values
    const auto & cosSinPhi = cosSinValues(nPoints);

    // loop around the ring
    //const double sinCkTheta = std::sin(ckTheta);
    //const double cosCkTheta = std::cos(ckTheta);
    double sinCkTheta(0), cosCkTheta(0);
    vdt::fast_sincos( ckTheta, sinCkTheta, cosCkTheta );
    for ( const auto& csPhi : cosSinPhi )
    {

      // Photon direction around loop
      const auto photDir =
        rotation * Gaudi::XYZVector( sinCkTheta * csPhi.cosPhi,
                                     sinCkTheta * csPhi.sinPhi,
                                     cosCkTheta );

      // Ray trace to detector plane
      Gaudi::XYZPoint hitPoint;
      const auto result = m_rayTrace->traceToDetector( rich,
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
  const auto incPhi = Gaudi::Units::twopi / static_cast<double>(nPoints);
  double ckPhi = 0.0;
  for ( unsigned int iPhot = 0; iPhot < nPoints; ++iPhot, ckPhi+=incPhi )
  {
    vect.emplace_back( CosSinPhi(static_cast<float>(ckPhi)) );
  }
}

//-----------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( RayTraceCherenkovCone )

//-----------------------------------------------------------------------------
