
//-----------------------------------------------------------------------------
/** @file RichPhotonRecoUsingQuarticSoln.cpp
 *
 * Implementation file for class : Rich::Rec::PhotonRecoUsingQuarticSoln
 *
 * CVS Log :-
 * $Id: RichPhotonRecoUsingQuarticSoln.cpp,v 1.7 2008-08-15 14:37:39 jonrob Exp $
 *
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 * @author Antonis Papanestis
 * @date 2003-11-14
 */
//-----------------------------------------------------------------------------

// local
#include "RichPhotonRecoUsingQuarticSoln.h"

// boost
#include "boost/assign/list_of.hpp"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

// Declaration of the Algorithm Factory
DECLARE_TOOL_FACTORY ( PhotonRecoUsingQuarticSoln );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
PhotonRecoUsingQuarticSoln::
PhotonRecoUsingQuarticSoln( const std::string& type,
                            const std::string& name,
                            const IInterface* parent )
  : PhotonRecoBase        ( type, name, parent ),
    m_mirrorSegFinder     ( NULL ),
    m_rayTracing          ( NULL ),
    m_idTool              ( NULL ),
    m_emissPoint          ( NULL ),
    m_snellsLaw           ( NULL ),
    m_testForUnambigPhots ( Rich::NRadiatorTypes, true  ),
    m_rejectAmbigPhots    ( Rich::NRadiatorTypes, false ),
    m_useAlignedMirrSegs  ( Rich::NRadiatorTypes, true  ),
    m_deBeam              ( Rich::NRiches               ),
    m_checkBeamPipe       ( Rich::NRadiatorTypes, true  ),
    m_checkPrimMirrSegs   ( Rich::NRadiatorTypes, false ),
    m_minActiveFrac       ( Rich::NRadiatorTypes, 0.2   ),
    m_minSphMirrTolIt     ( Rich::NRadiatorTypes        )
{
  using namespace boost::assign;

  // Initialise
  m_deBeam[Rich::Rich1] = NULL;
  m_deBeam[Rich::Rich2] = NULL;
  
  // context specific defaults
  if ( context() == "HLT" || context() == "Hlt" )
  {
    m_nQits = list_of (3) (3) (3) ;
  }
  else // offline
  { 
    m_nQits = list_of (3) (3) (3) ;
  }

  // job options
  declareProperty( "FindUnambiguousPhotons",    m_testForUnambigPhots         );
  declareProperty( "UseMirrorSegmentAllignment", m_useAlignedMirrSegs         );
  declareProperty( "AssumeFlatSecondaries",     m_forceFlatAssumption = false );
  declareProperty( "NQuarticIterationsForSecMirrors", m_nQits                 );
  declareProperty( "UseSecondaryMirrors",                m_useSecMirs = true  );
  declareProperty( "RejectAmbiguousPhotons",       m_rejectAmbigPhots         );
  declareProperty( "CheckBeamPipe", m_checkBeamPipe );
  declareProperty( "CheckPrimaryMirrorSegments", m_checkPrimMirrSegs );
  declareProperty( "MinActiveFraction", m_minActiveFrac );
  m_minSphMirrTolIt[Rich::Aerogel]  = std::pow(0.10 * Gaudi::Units::mm,2);
  m_minSphMirrTolIt[Rich::Rich1Gas] = std::pow(0.08 * Gaudi::Units::mm,2);
  m_minSphMirrTolIt[Rich::Rich2Gas] = std::pow(0.05 * Gaudi::Units::mm,2);
  declareProperty( "MinSphMirrTolIt", m_minSphMirrTolIt );

  // default fudge factors for this impl.
  m_ckFudge = list_of (-0.000358914) (-0.000192933) (-3.49182e-05) ;

}

//=============================================================================
// Destructor
//=============================================================================
PhotonRecoUsingQuarticSoln::~PhotonRecoUsingQuarticSoln() { }

//=============================================================================
// Initialisation.
//=============================================================================
StatusCode PhotonRecoUsingQuarticSoln::initialize()
{
  // initialise base class
  const StatusCode sc = PhotonRecoBase::initialize();
  if ( sc.isFailure() ) return sc;

  // get the detector elements
  m_rich[Rich::Rich1] = getDet<DeRich>( DeRichLocations::Rich1 );
  m_rich[Rich::Rich2] = getDet<DeRich>( DeRichLocations::Rich2 );

  // Get tools
  acquireTool( "RichMirrorSegFinder", m_mirrorSegFinder );
  acquireTool( "RichRayTracing",      m_rayTracing      );
  acquireTool( "RichSmartIDTool",     m_idTool, 0, true );
  acquireTool( "RichPhotonEmissionPoint", m_emissPoint  );
  acquireTool( "RichSnellsLawRefraction", m_snellsLaw   );

  // loop over radiators
  for ( Rich::Radiators::const_iterator rad = Rich::radiators().begin();
        rad != Rich::radiators().end(); ++rad )
  {

    // If rejection of ambiguous photons is turned on
    // make sure test is turned on
    if ( m_rejectAmbigPhots[*rad] ) m_testForUnambigPhots[*rad]  = true;

    // If we are testing for photons that hit the beam pipe, turn on ambig photon test
    if ( m_checkBeamPipe[*rad] )    m_testForUnambigPhots[*rad]  = true;

    // information printout about configuration
    if ( m_testForUnambigPhots[*rad] )
    {      info() << "Will test for unambiguous     " << *rad << " photons" << endreq; }
    else { info() << "Will not test for unambiguous " << *rad << " photons" << endreq; }

    if ( m_rejectAmbigPhots[*rad] )
    {      info() << "Will reject ambiguous " << *rad << " photons" << endreq; }
    else { info() << "Will accept ambiguous " << *rad << " photons" << endreq; }

    if ( m_useAlignedMirrSegs[*rad] )
    {      info() << "Will use fully alligned mirror segments for " << *rad << " reconstruction" << endreq;  }
    else { info() << "Will use nominal mirrors for " << *rad << " reconstruction" << endreq; }

    if ( m_checkBeamPipe[*rad] )
    {      info() << "Will check for " << *rad << " photons that hit the beam pipe" << endreq; }

    if ( m_checkPrimMirrSegs[*rad] )
    {      info() << "Will check for full intersecton with mirror segments for " << *rad << endreq; }

    info() << "Minimum active " << *rad << " segment fraction = " << m_minActiveFrac[*rad] << endreq;

  }

  if ( !m_useSecMirs )
  {
    Warning( "Will ignore secondary mirrors", StatusCode::SUCCESS );
    // no point doing interations
    m_forceFlatAssumption = true;
  }
  else
  {
    if ( m_forceFlatAssumption )
    {
      Warning( "Assuming perfectly flat secondary mirrors", StatusCode::SUCCESS );
    }
    else
    {
      // check iterations
      if ( m_nQits[Rich::Aerogel]  < 1 ) return Error( "# Aerogel  iterations < 1" );
      if ( m_nQits[Rich::Rich1Gas] < 1 ) return Error( "# Rich1Gas iterations < 1" );
      if ( m_nQits[Rich::Rich2Gas] < 1 ) return Error( "# Rich2Gas iterations < 1" );
      info() << "Assuming spherical secondary mirrors : # iterations (Aero/R1Gas/R2Gas) = "
             << m_nQits << endreq;
    }
  }

  return sc;
}

//=========================================================================
//  reconstruct a photon from track segment and pixel
//=========================================================================
StatusCode
PhotonRecoUsingQuarticSoln::
reconstructPhoton ( const LHCb::RichRecSegment * segment,
                    const LHCb::RichRecPixel * pixel,
                    LHCb::RichGeomPhoton& gPhoton ) const
{

  // the geometrical track segment
  const LHCb::RichTrackSegment & trSeg = segment->trackSegment();

  // detection point
  const Gaudi::XYZPoint& detectionPoint = pixel->globalPosition();

  // SmartIDs
  const Rich::HPDPixelCluster& smartIDs = pixel->hpdPixelCluster();

  // Detector information (RICH, radiator and HPD panel)
  const Rich::DetectorType rich     = trSeg.rich();
  const Rich::RadiatorType radiator = trSeg.radiator();
  const Rich::Side         side     = m_rich[rich]->side( detectionPoint );

  // Emission point to use for photon reconstruction
  // operate directly on photon data member for efficiency
  Gaudi::XYZPoint & emissionPoint = gPhoton.emissionPoint();
  m_emissPoint->emissionPoint( segment, pixel, emissionPoint );

  // Photon direction at emission point
  // Again, operator directly on data member
  Gaudi::XYZVector & photonDirection = gPhoton.emissionDir();

  // Final reflection points on sec and spherical mirrors
  // operate directly on photon data
  Gaudi::XYZPoint & sphReflPoint = gPhoton.sphMirReflectionPoint();
  Gaudi::XYZPoint & secReflPoint = gPhoton.flatMirReflectionPoint();

  // fraction of segment path length accessible to the photon
  float fraction(1);

  // Pointers to best sec and spherical mirror segments
  const DeRichSphMirror * sphSegment = NULL;
  const DeRichSphMirror * secSegment = NULL;

  // flag to say if this photon candidate is un-ambiguous - default to false
  bool unambigPhoton( false );

  // find the reflection of the detection point in the sec mirror
  // (virtual detection point) starting with nominal values
  // At this we are assuming a flat nominal mirror common to all segments
  double distance = m_rich[rich]->nominalPlane(side).Distance(detectionPoint);
  Gaudi::XYZPoint virtDetPoint =
    ( m_useSecMirs ?
      detectionPoint - 2.0 * distance * m_rich[rich]->nominalPlane(side).Normal() :
      detectionPoint
      );

  // --------------------------------------------------------------------------------------
  // For gas radiators, try start and end points to see if photon is unambiguous
  // NOTE : For this we are using the virtual detection point determined using
  // the noimnal flat secondary mirror plane. Now the secondary mirrors are actually
  // spherical this may be introducing some additional uncertainties.
  // --------------------------------------------------------------------------------------
  if ( m_testForUnambigPhots[radiator] )
  {
    if ( radiator == Rich::Aerogel )
    {
      // use default emission point and assume unambiguous since path length is so short..
      unambigPhoton = true;
    }
    else  // gas radiators
    {

      // flag for beam pipe check. Default is OK
      bool beamTestOK(true);

      // -------------------------------------------------------------------------------
      // First emission point, at start of track segment
      Gaudi::XYZPoint emissionPoint1;
      m_emissPoint->emissionPoint( segment, pixel, 0.01, emissionPoint1 );
      // Find mirror segments for this emission point
      const DeRichSphMirror* sphSegment1 = NULL;
      const DeRichSphMirror* secSegment1 = NULL;
      Gaudi::XYZPoint sphReflPoint1, secReflPoint1;
      if ( !findMirrorData( rich, side, virtDetPoint, emissionPoint1,
                            sphSegment1, secSegment1, sphReflPoint1, secReflPoint1 ) )
      {
        //return Warning( Rich::text(radiator)+" : Failed to reconstruct photon for start of segment" );
        return StatusCode::FAILURE;
      }
      if ( m_checkBeamPipe[radiator] )
      {
        beamTestOK = !deBeam(rich)->testForIntersection( emissionPoint1,
                                                         sphReflPoint1-emissionPoint1 );
      }
      // -------------------------------------------------------------------------------

      // -------------------------------------------------------------------------------
      // now do it again for emission point #2, at end of segment
      Gaudi::XYZPoint emissionPoint2;
      m_emissPoint->emissionPoint( segment, pixel, 0.99, emissionPoint2 );
      // Find mirror segments for this emission point
      const DeRichSphMirror* sphSegment2 = NULL;
      const DeRichSphMirror* secSegment2 = NULL;
      Gaudi::XYZPoint sphReflPoint2, secReflPoint2;
      if ( !findMirrorData( rich, side, virtDetPoint, emissionPoint2,
                            sphSegment2, secSegment2, sphReflPoint2, secReflPoint2 ) )
      {
        //return Warning( Rich::text(radiator)+" : Failed to reconstruct photon for end of segment" );
        return StatusCode::FAILURE;
      }
      if ( !beamTestOK && m_checkBeamPipe[radiator] )
      {
        beamTestOK = !deBeam(rich)->testForIntersection( emissionPoint2,
                                                         sphReflPoint2-emissionPoint2 );
      }
      // -------------------------------------------------------------------------------

      // -------------------------------------------------------------------------------
      if ( !beamTestOK )
      {
        // both start and end points failed beam pipe test -> reject
        //return Warning( Rich::text(radiator)+" : Failed beam pipe intersection checks" );
        return StatusCode::FAILURE;
      }
      // -------------------------------------------------------------------------------

      // -------------------------------------------------------------------------------
      // Test to see if photon direction hits the real physical mirror segments
      // -------------------------------------------------------------------------------
      if ( m_checkPrimMirrSegs[radiator] )
      {
        // primary mirrors
        const bool ok = ( sphSegment1->intersects( emissionPoint1,
                                                   sphReflPoint1-emissionPoint1 ) ||
                          sphSegment2->intersects( emissionPoint2,
                                                   sphReflPoint2-emissionPoint2 ) );
        if ( !ok )
        {
          //return Warning( Rich::text(radiator)+" : Failed mirror segment intersection checks" );
          return StatusCode::FAILURE;
        }
      }
      // -------------------------------------------------------------------------------

      // -------------------------------------------------------------------------------
      // Get the best gas emission point
      if ( !getBestGasEmissionPoint( radiator,
                                     sphReflPoint1,
                                     sphReflPoint2,
                                     detectionPoint,
                                     segment, pixel,
                                     emissionPoint,
                                     fraction ) )
      {
        //return Warning( Rich::text(radiator)+" : Failed to compute best gas emission point" );
        return StatusCode::FAILURE;
      }
      // -------------------------------------------------------------------------------

      // -------------------------------------------------------------------------------
      // Is this an unambiguous photon - I.e. only one possible mirror combination
      if ( (sphSegment1 == sphSegment2) && (secSegment1 == secSegment2) )
      {
        // Set pointers to the mirror detector objects
        sphSegment = sphSegment1;
        secSegment = secSegment1;
        // rough guesses at reflection points (improved later on)
        sphReflPoint = sphReflPoint1 + (sphReflPoint2-sphReflPoint1)/2.0;
        secReflPoint = secReflPoint1 + (secReflPoint2-secReflPoint1)/2.0;
        // photon is not unambiguous
        unambigPhoton = true;
      }
      // -------------------------------------------------------------------------------

    } // end radiator type if

    // if configured to do so reject ambiguous photons
    if ( m_rejectAmbigPhots[radiator] && !unambigPhoton )
    {
      //return Warning( Rich::text(radiator)+" : Failed ambiguous photon test" );
      return StatusCode::FAILURE;
    }

  } // end do test if
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // Active segment fraction cut
  // --------------------------------------------------------------------------------------
  if ( fraction < m_minActiveFrac[radiator] )
  {
    //return Warning( Rich::text(radiator)+" : Failed active segment fraction cut" );
    return StatusCode::FAILURE;
  }
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // if aerogel (which hasn't yet been treated at all) or ambiguous gas photon, or if
  // ambiguous photon check above has been skipped, try again
  // using best emission point and nominal mirror geometries to get the spherical and sec
  // mirrors. Also, force this reconstruction if the above unambiguous test was skipped
  // --------------------------------------------------------------------------------------
  if ( !m_testForUnambigPhots[radiator] || Rich::Aerogel == radiator || !unambigPhoton )
  {
    if ( !findMirrorData(rich,side,
                         virtDetPoint,emissionPoint,
                         sphSegment,secSegment,sphReflPoint,secReflPoint ) )
    {
      //return Warning( Rich::text(radiator)+" : Failed backup photon reconstruction" );
      return StatusCode::FAILURE;
    }
  }
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // Finally reconstruct the photon using best emission point and the best mirror segments
  // --------------------------------------------------------------------------------------
  if ( m_useAlignedMirrSegs[radiator] )
  {
    if ( m_forceFlatAssumption )
    {
      // assume secondary mirrors are perfectly flat

      if ( m_useSecMirs )
      {
        distance     = secSegment->centreNormalPlane().Distance(detectionPoint);
        virtDetPoint = detectionPoint - 2.0 * distance * secSegment->centreNormal();
      }

      // solve the quartic
      if ( !solveQuarticEq( emissionPoint,
                            sphSegment->centreOfCurvature(),
                            virtDetPoint,
                            sphSegment->radius(),
                            sphReflPoint ) )
      {
        //return Warning( Rich::text(radiator)+" : Failed to reconstruct photon using mirror segments" );
        return StatusCode::FAILURE;
      }

    }
    else
    {
      // Default mode. Use full spherical nature of secondaries

      // Iterate to final solution, improving the secondary mirror info
      int iIt(0);
      Gaudi::XYZPoint last_mirror_point(0,0,0);
      while ( iIt < m_nQits[radiator] )
      {

        // Get secondary mirror reflection point,
        // using the best actual secondary mirror segment at this point
        const bool sc = m_rayTracing->intersectPlane( sphReflPoint,
                                                      virtDetPoint - sphReflPoint,
                                                      secSegment->centreNormalPlane(),
                                                      secReflPoint );
        if ( !sc )
        {
          //return Warning( Rich::text(radiator)+" : Failed to intersect nominal secondary mirror plane" );
          return StatusCode::FAILURE;
        }
        // (re)find the secondary mirror
        secSegment = m_mirrorSegFinder->findSecMirror( rich, side, secReflPoint );

        // Construct plane tangential to secondary mirror passing through reflection point
        const Gaudi::Plane3D plane( secSegment->centreOfCurvature() - secReflPoint,
                                    secReflPoint );

        // re-find the reflection of the detection point in the sec mirror
        // (virtual detection point) with this mirror plane
        distance     = plane.Distance(detectionPoint);
        virtDetPoint = detectionPoint - 2.0 * distance * plane.Normal();

        // solve the quartic using the new data
        if ( !solveQuarticEq( emissionPoint,
                              sphSegment->centreOfCurvature(),
                              virtDetPoint,
                              sphSegment->radius(),
                              sphReflPoint ) )
        {
          //return Warning(  Rich::text(radiator)+" : Failed to reconstruct photon using mirror segments" );
          return StatusCode::FAILURE;
        }

        // (re)find the spherical mirror segment
        sphSegment = m_mirrorSegFinder->findSphMirror( rich, side, sphReflPoint );

        // for iterations after the first, see if we are still moving
        // If not, abort iterations early
        if ( iIt > 0 )
        {
          if ( (last_mirror_point-secReflPoint).Mag2() < m_minSphMirrTolIt[radiator] ) break;
        }

        // store last sec mirror point
        last_mirror_point = secReflPoint;

        // increment iteration counter
        ++iIt;
      }

    } // end mirror type if
  }
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // check that spherical mirror reflection point is on the same side as detection point
  // and (if configured to do so) photon does not cross between detector sides
  // --------------------------------------------------------------------------------------
  if ( !sameSide(radiator,sphReflPoint,virtDetPoint) )
  {
    //return Warning( Rich::text(radiator)+" : Reflection point on wrong side" );
    return StatusCode::FAILURE;
  }
  if ( m_checkPhotCrossSides[radiator] && !sameSide(radiator,sphReflPoint,emissionPoint) )
  {
    //return Warning( Rich::text(radiator)+" : Photon cross between detector sides" );
    return StatusCode::FAILURE;
  }
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // If using aligned mirror segments, get the final sec mirror reflection
  // point using the best mirror segments available at this point
  // --------------------------------------------------------------------------------------
  if ( m_useSecMirs && m_useAlignedMirrSegs[radiator] )
  {
    const bool sc = m_rayTracing->intersectPlane( sphReflPoint,
                                                  virtDetPoint - sphReflPoint,
                                                  secSegment->centreNormalPlane(),
                                                  secReflPoint );
    if ( !sc )
    {
      //return Warning( Rich::text(radiator)+" : Failed final secondary mirror plane intersection" );
      return StatusCode::FAILURE;
    }
  }
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // calculate the cherenkov angles using the photon and track vectors
  // --------------------------------------------------------------------------------------
  photonDirection = (sphReflPoint-emissionPoint).Unit();
  double thetaCerenkov(0), phiCerenkov(0);
  trSeg.angleToDirection( photonDirection, thetaCerenkov, phiCerenkov );
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // Correct Cherenkov theta for refraction at exit of aerogel
  // --------------------------------------------------------------------------------------
  if ( Rich::Aerogel == radiator )
  {
    correctAeroRefraction( trSeg, photonDirection, thetaCerenkov );
  }
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // Test final data to see if photon direction hits the real physical mirror segments
  // --------------------------------------------------------------------------------------
  //if ( m_checkPrimMirrSegs[radiator] )
  //{
  //  // primary mirrors
  //  const bool ok = ( sphSegment->intersects(emissionPoint,photonDirection) );
  //  if ( !ok ) return StatusCode::FAILURE;
  //}
  // --------------------------------------------------------------------------------------

  //---------------------------------------------------------------------------------------
  // Apply fudge factor correction for small biases in CK theta
  // To be understood
  //---------------------------------------------------------------------------------------
  thetaCerenkov += m_ckFudge[radiator];
  //---------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // Set (remaining) photon parameters
  // --------------------------------------------------------------------------------------
  gPhoton.setCherenkovTheta         ( static_cast<float>(thetaCerenkov) );
  gPhoton.setCherenkovPhi           ( static_cast<float>(phiCerenkov)   );
  gPhoton.setActiveSegmentFraction  ( fraction       );
  gPhoton.setDetectionPoint         ( detectionPoint );
  gPhoton.setSmartID                ( smartIDs.primaryID() );
  gPhoton.setUnambiguousPhoton      ( unambigPhoton  );
  gPhoton.setPrimaryMirror          ( sphSegment );
  gPhoton.setSecondaryMirror        ( secSegment );
  if ( msgLevel(MSG::VERBOSE) )
  {
    verbose() << "Created photon " << gPhoton << endreq;
  }
  // --------------------------------------------------------------------------------------

  return StatusCode::SUCCESS;
}

//=========================================================================
// Find mirror segments and reflection points for given data
//=========================================================================
bool
PhotonRecoUsingQuarticSoln::
findMirrorData( const Rich::DetectorType rich,
                const Rich::Side side,
                const Gaudi::XYZPoint& virtDetPoint,
                const Gaudi::XYZPoint& emissionPoint,
                const DeRichSphMirror*& sphSegment,
                const DeRichSphMirror*& secSegment,
                Gaudi::XYZPoint& sphReflPoint,
                Gaudi::XYZPoint& secReflPoint ) const
{
  // solve quartic equation with nominal values and find spherical mirror reflection point
  if ( !solveQuarticEq( emissionPoint,
                        m_rich[rich]->nominalCentreOfCurvature(side),
                        virtDetPoint,
                        m_rich[rich]->sphMirrorRadius(),
                        sphReflPoint ) ) { return false; }

  // find the spherical mirror segment
  sphSegment = m_mirrorSegFinder->findSphMirror( rich, side, sphReflPoint );

  if ( m_useSecMirs )
  {
    // find the sec mirror intersction point and secondary mirror segment
    const StatusCode sc = m_rayTracing->intersectPlane( sphReflPoint,
                                                        virtDetPoint - sphReflPoint,
                                                        m_rich[rich]->nominalPlane(side),
                                                        secReflPoint );
    if ( sc.isFailure() ) { return false; }
    // find the secondary mirror
    secSegment = m_mirrorSegFinder->findSecMirror( rich, side, secReflPoint );
  }

  return true;
}

//=========================================================================
// Compute the best emission point for the gas radiators using
// the given spherical mirror reflection points
//=========================================================================
bool
PhotonRecoUsingQuarticSoln::
getBestGasEmissionPoint( const Rich::RadiatorType radiator,
                         const Gaudi::XYZPoint& sphReflPoint1,
                         const Gaudi::XYZPoint& sphReflPoint2,
                         const Gaudi::XYZPoint& detectionPoint,
                         const LHCb::RichRecSegment * segment,
                         const LHCb::RichRecPixel * pixel,
                         Gaudi::XYZPoint & emissionPoint,
                         float & fraction ) const
{
  const LHCb::RichTrackSegment& trSeg = segment->trackSegment();

  double alongTkFrac = 0.5;

  if ( radiator == Rich::Rich1Gas )
  {
    // First reflection and hit point on same y side ?
    const bool sameSide1 = ( sphReflPoint1.y() * detectionPoint.y() > 0 );
    const bool sameSide2 = ( sphReflPoint2.y() * detectionPoint.y() > 0 );
    if ( sameSide1 && sameSide2 )
    {
      m_emissPoint->emissionPoint( segment, pixel, emissionPoint );
    }
    else if ( sameSide1 )
    {
      fraction = std::fabs( sphReflPoint1.y() / (sphReflPoint1.y() - sphReflPoint2.y()) );
      alongTkFrac = fraction/2.0;
      m_emissPoint->emissionPoint( segment, pixel, alongTkFrac, emissionPoint );
    }
    else if ( sameSide2 )
    {
      fraction = std::fabs( sphReflPoint2.y() / (sphReflPoint1.y() - sphReflPoint2.y()) );
      alongTkFrac = 1.0-fraction/2.0;
      m_emissPoint->emissionPoint( segment, pixel, alongTkFrac, emissionPoint );
    }
    else
    {
      //Warning( "Rich1Gas : Both RICH spherical mirror hits opposite side to detection point" );
      return false;
    }

  }
  else if ( radiator == Rich::Rich2Gas )
  {
    // First sphReflPoint and hit point on same x side ?
    const bool sameSide1 = ( sphReflPoint1.x() * detectionPoint.x() > 0 );
    const bool sameSide2 = ( sphReflPoint2.x() * detectionPoint.x() > 0 );
    if ( sameSide1 && sameSide2 )
    {
      emissionPoint = trSeg.bestPoint();
    }
    else if ( sameSide1 )
    {
      fraction = std::fabs( sphReflPoint1.x()/(sphReflPoint1.x()-sphReflPoint2.x()) );
      alongTkFrac = fraction/2.0;
      m_emissPoint->emissionPoint( segment, pixel, alongTkFrac, emissionPoint );
    }
    else if ( sameSide2 )
    {
      fraction = std::fabs( sphReflPoint2.x()/(sphReflPoint1.x()-sphReflPoint2.x()) );
      alongTkFrac = 1.0-fraction/2.0;
      m_emissPoint->emissionPoint( segment, pixel, alongTkFrac, emissionPoint );
    }
    else
    {
      //Warning( "Rich2Gas : Both RICH spherical mirror hits opposite side to detection point" );
      return false;
    }
  }
  else { Error( "::getBestGasEmissionPoint() called for Aerogel segment !!" ); }

  if ( msgLevel(MSG::VERBOSE) )
  {
    verbose() << radiator << " best emission point correction :- " << endreq
              << " -> Photon detection point = " << detectionPoint << endreq
              << " -> Sph. Mirror ptns       = " << sphReflPoint1 << " " << sphReflPoint2 << endreq
              << " -> Segment entry/exit     = " << trSeg.entryPoint() << " " << trSeg.exitPoint() << endreq
              << " -> Segment fraction       = " << fraction << endreq
              << " -> Emm. Ptn. Along traj   = " << alongTkFrac << endreq
              << " -> Best Emission point    = " << emissionPoint << endreq;
  }

  return true;
}

//=========================================================================
// Correct Aerogel Cherenkov angle theta for refraction at exit of aerogel
//=========================================================================
void
PhotonRecoUsingQuarticSoln::
correctAeroRefraction( const LHCb::RichTrackSegment& trSeg,
                       Gaudi::XYZVector& photonDirection,
                       double & thetaCerenkov ) const
{
  // apply Snell's Law
  m_snellsLaw->gasToAerogel( photonDirection, trSeg.avPhotonEnergy() );
  // update CK theta
  const double ctc = photonDirection.Dot( trSeg.bestMomentum().Unit() );
  thetaCerenkov = ( ctc>1 ? 0 : acos(ctc) );
}

//=========================================================================
// Setup and solve quartic equation in the form
// x^4 + a x^3 + b x^2 + c x + d = 0
//=========================================================================
bool
PhotonRecoUsingQuarticSoln::
solveQuarticEq ( const Gaudi::XYZPoint& emissionPoint,
                 const Gaudi::XYZPoint& CoC,
                 const Gaudi::XYZPoint& virtDetPoint,
                 const double radius,
                 Gaudi::XYZPoint& sphReflPoint ) const
{

  // vector from mirror centre of curvature to assumed emission point
  Gaudi::XYZVector evec = emissionPoint - CoC;
  const double e2 = evec.Mag2();
  if ( e2 < 1e-99 ) { return false; }

  // vector from mirror centre of curvature to virtual detection point
  const Gaudi::XYZVector dvec = virtDetPoint - CoC;
  const double d2 = dvec.Mag2();
  if ( d2 < 1e-99 ) { return false; }

  // various quantities needed to create quartic equation
  // see LHCB/98-040 section 3, equation 3
  const double e        = sqrt(e2);
  const double d        = sqrt(d2);
  const double cosgamma = evec.Dot(dvec) / (e*d);
  const double singamma = std::sqrt( 1.0 - cosgamma*cosgamma );
  //const double singamma = sin( acos(cosgamma) );
  const double dx       = d * cosgamma;
  const double dy       = d * singamma;
  const double r2       = radius * radius;

  // Fill array for quartic equation
  const double a0 =   4 * e2 * d2;
  const double a1 = - ( 4 * e2 * dy * radius ) / a0;
  const double a2 =  ( (dy * dy * r2) + ((e+dx) * (e+dx) * r2) - a0 ) / a0;
  const double a3 =  ( 2 * e * dy * (e-dx) * radius ) / a0;
  const double a4 =  ( ( e2 - r2 ) * dy * dy ) / a0;

  // -----------------------------------------------------------------------

  // use simplified RICH version of quartic solver
  const double sinbeta = solve_quartic_RICH( a1, // a
                                             a2, // b
                                             a3, // c
                                             a4  // d
                                             );

  // normal vector to reflection plane
  const Gaudi::XYZVector nvec2 = evec.Cross(dvec);

  // Set vector mag to radius
  evec *= radius/e;

  // create rotation
  const Gaudi::Rotation3D rotn( Gaudi::AxisAngle(nvec2,asin(sinbeta)) );

  // rotate vector and update reflection point
  sphReflPoint = CoC + rotn*evec;

  // -----------------------------------------------------------------------

  /*
  // -----------------------------------------------------------------------
  // use full 'GSL' function
  // CRJ : Note, not yet updated from CLHEP so will not compile

  gsl_complex solutions[4];
  if ( 0 == gsl_poly_complex_solve_quartic( a1, // a
  a2, // b
  a3, // c
  a4, // d
  &solutions[0],
  &solutions[1],
  &solutions[2],
  &solutions[3] ) ) { return false; }

  // normal vector to reflection plane
  const Gaudi::XYZVector nvec1 = evec.cross(dvec);

  // get the required results
  Gaudi::XYZVector delta[2];
  int j = 0;
  for ( int i = 0; i<4 && j<2; ++i )
  {
  if ( 0 == GSL_IMAG(solutions[i]) )
  {
  delta[j] = evec;
  delta[j].setMag(radius);
  delta[j++].rotate( asin(GSL_REAL(solutions[i])), nvec1);
  }
  }

  // Finally, form the reflection point on the spherical mirror
  sphReflPoint = ( delta[0].z() > delta[1].z() ? CoC + delta[0] : CoC + delta[1] );

  // -----------------------------------------------------------------------
  */

  return true;
}
