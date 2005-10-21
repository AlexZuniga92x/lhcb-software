
//-----------------------------------------------------------------------------
/** @file RichPhotonRecoUsingQuarticSolnAllSph.cpp
 *
 * Implementation file for class : RichPhotonRecoUsingQuarticSolnAllSph
 *
 * CVS Log :-
 * $Id: RichPhotonRecoUsingQuarticSolnAllSph.cpp,v 1.6 2005-10-21 13:34:41 jonrob Exp $
 *
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 * @author Antonis Papanestis
 * @date 2003-11-14
 */
//-----------------------------------------------------------------------------

// local
#include "RichPhotonRecoUsingQuarticSolnAllSph.h"

// Declaration of the Algorithm Factory
static const  ToolFactory<RichPhotonRecoUsingQuarticSolnAllSph>          Factory ;
const        IToolFactory& RichPhotonRecoUsingQuarticSolnAllSphFactory = Factory ;

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RichPhotonRecoUsingQuarticSolnAllSph::
RichPhotonRecoUsingQuarticSolnAllSph( const std::string& type,
                                      const std::string& name,
                                      const IInterface* parent )
  : RichToolBase      ( type, name, parent ),
    m_mirrorSegFinder ( 0 ),
    m_rayTracing      ( 0 ),
    m_idTool          ( 0 ),
    m_refIndex        ( 0 ),
    m_nQits           ( Rich::NRadiatorTypes )
{

  // declare interface
  declareInterface<IRichPhotonReconstruction>(this);

  // job options
  declareProperty( "FindUnambiguousPhotons",    m_testForUnambigPhots = false );
  declareProperty( "UseMirrorSegmentAllignment", m_useAlignedMirrSegs = true  );
  declareProperty( "AssumeFlatSecondaries",     m_forceFlatAssumption = false );
  m_nQits[Rich::Aerogel] = 1;
  m_nQits[Rich::C4F10]   = 1;
  m_nQits[Rich::CF4]     = 1;
  declareProperty( "NQuarticIterationsForSecMirrors", m_nQits                 );

}

//=============================================================================
// Destructor
//=============================================================================
RichPhotonRecoUsingQuarticSolnAllSph::~RichPhotonRecoUsingQuarticSolnAllSph() { }

//=============================================================================
// Initialisation.
//=============================================================================
StatusCode RichPhotonRecoUsingQuarticSolnAllSph::initialize()
{
  // initialise base class
  const StatusCode sc = RichToolBase::initialize();
  if ( sc.isFailure() ) return sc;

  // get the detector elements
  m_rich[Rich::Rich1] = getDet<DeRich>( DeRichLocation::Rich1 );
  m_rich[Rich::Rich2] = getDet<DeRich>( DeRichLocation::Rich2 );

  // Get tools
  acquireTool( "RichMirrorSegFinder", m_mirrorSegFinder );
  acquireTool( "RichRayTracing",      m_rayTracing      );
  acquireTool( "RichSmartIDTool",     m_idTool, 0, true );
  acquireTool( "RichRefractiveIndex", m_refIndex        );

  // check iterations
  if ( m_nQits[Rich::Aerogel] < 1 ) return Error( "# Aerogel iterations < 1" );
  if ( m_nQits[Rich::C4F10]   < 1 ) return Error( "# C4F10   iterations < 1" );
  if ( m_nQits[Rich::CF4]     < 1 ) return Error( "# CF4     iterations < 1" );

  // information printout about configuration
  if ( m_testForUnambigPhots )
  {
    info() << "Will test for unambiguous photons" << endreq;
  }
  else
  {
    info() << "Will not test for unambiguous photons" << endreq;
  }
  if ( m_useAlignedMirrSegs )
  {
    info() << "Will use fully alligned mirror segments for reconstruction" << endreq;
  }
  else
  {
    info() << "Will use nominal mirrors for reconstruction" << endreq;
  }
  info() << "Number of secondary mirror iterations (aero/C4F10/CF4) : "
         << m_nQits << endreq;

  if ( m_forceFlatAssumption )
    Warning( "Assuming perfectly flat secondary mirrors", StatusCode::SUCCESS );

  return sc;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode RichPhotonRecoUsingQuarticSolnAllSph::finalize()
{
  return RichToolBase::finalize();
}

//=========================================================================
//  reconstruct a photon from track segment and smartID
//=========================================================================
StatusCode RichPhotonRecoUsingQuarticSolnAllSph::
reconstructPhoton ( const RichTrackSegment& trSeg,
                    const RichSmartID& smartID,
                    RichGeomPhoton& gPhoton ) const
{
  return reconstructPhoton( trSeg, m_idTool->globalPosition(smartID), gPhoton );
}

//-------------------------------------------------------------------------
//  reconstruct a photon from track segment and detection point
//-------------------------------------------------------------------------
StatusCode RichPhotonRecoUsingQuarticSolnAllSph::
reconstructPhoton ( const RichTrackSegment& trSeg,
                    const HepPoint3D& detectionPoint,
                    RichGeomPhoton& gPhoton ) const
{

  // Detector information (RICH, radiator and HPD panel)
  const Rich::DetectorType rich     = trSeg.rich();
  const Rich::RadiatorType radiator = trSeg.radiator();
  const Rich::Side side             = m_rich[rich]->side( detectionPoint );

  // Emission point to use for photon reconstruction
  // operate directly on photon data member for efficiency
  // Starting value is the "best" point on the track segment
  HepPoint3D & emissionPoint = gPhoton.emissionPoint();
  emissionPoint = trSeg.bestPoint();

  // Final reflection points on sec and spherical mirrors
  // operate directly on photon data
  HepPoint3D & sphReflPoint = gPhoton.sphMirReflectionPoint();
  HepPoint3D & secReflPoint = gPhoton.flatMirReflectionPoint();

  // fraction of segment path length accessible to the photon
  double fraction(1);

  // Pointers to best sec and spherical mirror segments
  const DeRichSphMirror* sphSegment = 0;
  const DeRichSphMirror* secSegment = 0;

  // flag to say if this photon candidate is un-ambiguous - default to false
  bool unambigPhoton( false );

  // find the reflection of the detection point in the sec mirror
  // (virtual detection point) starting with nominal values
  // At this we are assuming a flat nominal mirror common to all segments
  double distance = m_rich[rich]->nominalPlane(side).distance(detectionPoint);
  HepPoint3D virtDetPoint =
    ( detectionPoint - 2.0 * distance * m_rich[rich]->nominalPlane(side).normal() );

  // --------------------------------------------------------------------------------------
  // For gas radiators, try start and end points to see if photon is unambiguous
  // NOTE : For this we are using the virtual detection point determined using
  // the noimnal flat secondary mirror plane. Now the secondary mirrors are actually
  // spherical this may be introducing some additional uncertainties.
  // --------------------------------------------------------------------------------------
  if ( m_testForUnambigPhots )
  {
    if ( radiator == Rich::Aerogel )
    {
      // use default emission point and assume unambiguous since path length is so short..
      unambigPhoton = true;
    }
    else  // gas radiators
    {

      // First emission point, at start of track segment
      const HepPoint3D emissionPoint1 = trSeg.bestPoint( 0.01 );

      // Find mirror segments for this emission point
      const DeRichSphMirror* sphSegment1 = 0;
      const DeRichSphMirror* secSegment1 = 0;
      HepPoint3D sphReflPoint1, secReflPoint1;
      if ( !findMirrorData( rich, side, virtDetPoint, emissionPoint1,
                            sphSegment1, secSegment1, sphReflPoint1, secReflPoint1 ) )
      {
        //return Warning( "Failed to reconstruct photon for start of segment" );
        return StatusCode::FAILURE;
      }

      // now do it again for emission point #2, at end of segment
      const HepPoint3D emissionPoint2 = trSeg.bestPoint( 0.99 );

      // Find mirror segments for this emission point
      const DeRichSphMirror* sphSegment2 = 0;
      const DeRichSphMirror* secSegment2 = 0;
      HepPoint3D sphReflPoint2, secReflPoint2;
      if ( !findMirrorData( rich, side, virtDetPoint, emissionPoint2,
                            sphSegment2, secSegment2, sphReflPoint2, secReflPoint2 ) )
      {
        //return Warning( "Failed to reconstruct photon for end of segment" );
        return StatusCode::FAILURE;
      }

      // Get gas emission point
      if ( !getBestGasEmissionPoint( radiator, sphReflPoint1, sphReflPoint2,
                                     detectionPoint, trSeg,
                                     emissionPoint, fraction ) )
      {
        return StatusCode::FAILURE;
      }

      // Is this an unambiguous photon - I.e. only one possible mirror combination
      if ( (sphSegment1 == sphSegment2) && (secSegment1 == secSegment2) )
      {
        // Set pointers to the mirror detector objects
        sphSegment = sphSegment1;
        secSegment = secSegment1;
        // rough guesses at reflection points (improved later on)
        sphReflPoint = (sphReflPoint1+sphReflPoint2)/2;
        secReflPoint = (secReflPoint1+secReflPoint2)/2;
        // photon is not unambiguous
        unambigPhoton = true;
      }

    } // end radiator type if
  } // end do test if
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // if aerogel (which hasn't yet been treated at all) or ambiguous gas photon, try again
  // using best emission point and nominal mirror geometries to get the spherical and sec
  // mirrors. Also, force this reconstruction if the above unambiguous test was skipped
  // --------------------------------------------------------------------------------------
  if ( !m_testForUnambigPhots || Rich::Aerogel == radiator || !unambigPhoton )
  {

    if ( !solveQuarticEq( emissionPoint,
                          m_rich[rich]->nominalCentreOfCurvature(side),
                          virtDetPoint,
                          m_rich[rich]->sphMirrorRadius(),
                          sphReflPoint ) )
    {
      //return Warning( "Failed to reconstruct photon using nominal mirrors" );
      return StatusCode::FAILURE;
    }

    // Get secondary mirror reflection point
    m_rayTracing->intersectPlane( sphReflPoint,
                                  virtDetPoint - sphReflPoint,
                                  m_rich[rich]->nominalPlane(side),
                                  secReflPoint);

    // Get pointers to the spherical and sec mirror detector objects
    sphSegment = m_mirrorSegFinder->findSphMirror ( rich, side, sphReflPoint );
    secSegment = m_mirrorSegFinder->findSecMirror ( rich, side, secReflPoint );

  }
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // Finally, reconstruct the photon using best emission point and the best mirror segments
  // --------------------------------------------------------------------------------------
  if ( m_useAlignedMirrSegs )
  {

    if ( m_forceFlatAssumption )
    {
      // assume secondary mirrors are perfectly flat

      distance     = secSegment->centreNormalPlane().distance(detectionPoint);
      virtDetPoint = detectionPoint - 2.0 * distance * secSegment->centreNormal();

      // solve the quartic
      if ( !solveQuarticEq( emissionPoint,
                            sphSegment->centreOfCurvature(),
                            virtDetPoint,
                            sphSegment->radius(),
                            sphReflPoint ) )
      {
        //return Warning( "Failed to reconstruct photon using mirror segments" );
        return StatusCode::FAILURE;
      }

    }
    else
    {
      // Default mode. Use full spherical nature of secondaries

      // Iterate to final solution, improving the secondary mirror info
      int iIt(0);
      while ( iIt < m_nQits[radiator] )
      {

        // Get secondary mirror reflection point,
        // using the best actual secondary mirror segment
        m_rayTracing->intersectPlane( sphReflPoint,
                                      virtDetPoint - sphReflPoint,
                                      secSegment->centreNormalPlane(),
                                      secReflPoint );

        // Construct plane tangential to secondary mirror passing through reflection point
        const HepPlane3D
          plane( HepNormal3D( secSegment->centreOfCurvature() - secReflPoint ).unit(),
                 secReflPoint );

        // re-find the reflection of the detection point in the sec mirror
        // (virtual detection point) with this mirror plane
        distance     = plane.distance(detectionPoint);
        virtDetPoint = detectionPoint - 2.0 * distance * plane.normal();

        // solve the quartic using the new data
        if ( !solveQuarticEq( emissionPoint,
                              sphSegment->centreOfCurvature(),
                              virtDetPoint,
                              sphSegment->radius(),
                              sphReflPoint ) )
        {
          //return Warning( "Failed to reconstruct photon using mirror segments" );
          return StatusCode::FAILURE;
        }

        // increment iteration counter and continue until max iterations has been done
        ++iIt;
      }

    } // end mirror type if

  }
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // check that spherical mirror reflection point is on the same side as track point
  // --------------------------------------------------------------------------------------
  if ( rich == Rich::Rich2 )
  {
    if ( sphReflPoint.x() * virtDetPoint.x() < 0.0 )
    {
      //return Warning( "RICH2 : Reflection point on wrong side" );
      return StatusCode::FAILURE;
    }
  }
  else // RICH 1
  {
    if ( sphReflPoint.y() * virtDetPoint.y() < 0.0 )
    {
      //return Warning( "RICH1 : Reflection point on wrong side" );
      return StatusCode::FAILURE;
    }
  }
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // If using alligned mirror segments, get the final sec mirror reflection
  // point using the best mirror segments available at this point
  // --------------------------------------------------------------------------------------
  if ( m_useAlignedMirrSegs )
  {
    m_rayTracing->intersectPlane( sphReflPoint,
                                  virtDetPoint - sphReflPoint,
                                  secSegment->centreNormalPlane(),
                                  secReflPoint );
  }
  // --------------------------------------------------------------------------------------

  // --------------------------------------------------------------------------------------
  // calculate the cherenkov angles using the photon and track vectors
  // --------------------------------------------------------------------------------------
  HepVector3D photonDirection = sphReflPoint - emissionPoint;
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
  // Set (remaining) photon parameters
  // --------------------------------------------------------------------------------------
  gPhoton.setCherenkovTheta         ( static_cast<float>(thetaCerenkov) );
  gPhoton.setCherenkovPhi           ( static_cast<float>(phiCerenkov)   );
  gPhoton.setActiveSegmentFraction  ( static_cast<float>(fraction)      );
  gPhoton.setDetectionPoint         ( detectionPoint );
  gPhoton.setMirrorNumValid         ( unambigPhoton  );
  gPhoton.setSphMirrorNum           ( sphSegment ? sphSegment->mirrorNumber() : 0 );
  gPhoton.setFlatMirrorNum          ( secSegment ? secSegment->mirrorNumber() : 0 );
  // --------------------------------------------------------------------------------------

  return StatusCode::SUCCESS;
}

//=========================================================================
// Find mirror segments and reflection points for given data
//=========================================================================
bool RichPhotonRecoUsingQuarticSolnAllSph::
findMirrorData( const Rich::DetectorType rich,
                const Rich::Side side,
                const HepPoint3D& virtDetPoint,
                const HepPoint3D& emissionPoint,
                const DeRichSphMirror*& sphSegment,
                const DeRichSphMirror*& secSegment,
                HepPoint3D& sphReflPoint,
                HepPoint3D& secReflPoint ) const
{
  // solve quartic equation with nominal values and find spherical mirror reflection point
  if ( !solveQuarticEq( emissionPoint,
                        m_rich[rich]->nominalCentreOfCurvature(side),
                        virtDetPoint,
                        m_rich[rich]->sphMirrorRadius(),
                        sphReflPoint ) ) { return false; }

  // find the spherical mirror segment
  sphSegment = m_mirrorSegFinder->findSphMirror(rich, side, sphReflPoint );

  // find the sec mirror intersction point and secondary mirror segment
  m_rayTracing->intersectPlane( sphReflPoint,
                                virtDetPoint - sphReflPoint,
                                m_rich[rich]->nominalPlane(side),
                                secReflPoint);
  secSegment = m_mirrorSegFinder->findSecMirror(rich, side, secReflPoint);

  return true;
}

//=========================================================================
// Compute the best emission point for the gas radiators using
// the given spherical mirror reflection points
//=========================================================================
bool RichPhotonRecoUsingQuarticSolnAllSph::
getBestGasEmissionPoint( const Rich::RadiatorType radiator,
                         const HepPoint3D& sphReflPoint1,
                         const HepPoint3D& sphReflPoint2,
                         const HepPoint3D& detectionPoint,
                         const RichTrackSegment& trSeg,
                         HepPoint3D & emissionPoint,
                         double & fraction ) const
{

  if ( radiator == Rich::C4F10 )
  {
    // First reflection and hit point on same y side ?
    const bool sameSide1 = ( sphReflPoint1.y() * detectionPoint.y() > 0 );
    const bool sameSide2 = ( sphReflPoint2.y() * detectionPoint.y() > 0 );
    if ( sameSide1 && sameSide2 )
    {
      emissionPoint = trSeg.bestPoint();
    }
    else if ( sameSide1 )
    {
      fraction = fabs( sphReflPoint1.y() / (sphReflPoint1.y() - sphReflPoint2.y()) );
      emissionPoint = trSeg.bestPoint( fraction/2 );
    }
    else if ( sameSide2 )
    {
      fraction = fabs( sphReflPoint2.y() / (sphReflPoint1.y() - sphReflPoint2.y()) );
      emissionPoint = trSeg.bestPoint( 1-fraction/2 );
    }
    else
    {
      //Warning( "C4F10 : Both RICH hits opposite side to track hit" );
      return false;
    }

  }
  else if ( radiator == Rich::CF4 )
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
      fraction = fabs( sphReflPoint1.x()/(sphReflPoint1.x()-sphReflPoint2.x()) );
      emissionPoint = trSeg.bestPoint( fraction/2 );
    }
    else if ( sameSide2 )
    {
      fraction = fabs( sphReflPoint2.x()/(sphReflPoint1.x()-sphReflPoint2.x()) );
      emissionPoint = trSeg.bestPoint( 1-fraction/2 );
    }
    else
    {
      //Warning( "CF4 : Both RICH hits opposite side to track hit" );
      return false;
    }
  }
  else { Error( "::getBestGasEmissionPoint() called for Aerogel segment !!" ); }

  return true;
}

//=========================================================================
// Correct Aerogel Cherenkov angle theta for refraction at exit of aerogel
//=========================================================================
void RichPhotonRecoUsingQuarticSolnAllSph::
correctAeroRefraction( const RichTrackSegment& trSeg,
                       HepVector3D& photonDirection,
                       double & thetaCerenkov ) const
{
  // Normalise photon vector
  photonDirection.setMag(1);
  // get refractive indices for aerogel and c4f10
  const double refAero
    = m_refIndex->refractiveIndex( Rich::Aerogel, trSeg.avPhotonEnergy() );
  const double refc4f10
    = m_refIndex->refractiveIndex( Rich::C4F10,   trSeg.avPhotonEnergy() );
  const double RratioSq = (refAero*refAero)/(refc4f10*refc4f10);
  // Apply Snells law and update angle
  HepVector3D newDir( 0, 0,
                      sqrt(1.-(1.-photonDirection.z()*photonDirection.z())/RratioSq ) );
  const double R = photonDirection.y()/photonDirection.x();
  newDir.setX( sqrt( (1.-newDir.z()*newDir.z())/(1. + R*R) ) );
  if ( photonDirection.x() < 0 ) newDir.setX( -newDir.x() );
  newDir.setY( R*newDir.x() );
  const double ctc = newDir.dot( trSeg.bestMomentum().unit() );
  thetaCerenkov = ( ctc>1 ? 0 : acos(ctc) );
}

//=========================================================================
// Setup and solve quartic equation in the form
// x^4 + a x^3 + b x^2 + c x + d = 0
//=========================================================================
bool RichPhotonRecoUsingQuarticSolnAllSph::
solveQuarticEq (const HepPoint3D& emissionPoint,
                const HepPoint3D& CoC,
                const HepPoint3D& virtDetPoint,
                const double radius,
                HepPoint3D& sphReflPoint) const
{

  // vector from mirro centre of curvature to assumed emission point
  HepVector3D evec = emissionPoint - CoC;
  const double e2 = evec.mag2();
  if ( 0 == e2 )  { return false; }

  // vector from mirror centre of curvature to virtual detection point
  const HepVector3D dvec = virtDetPoint - CoC;
  const double d2 = dvec.mag2();
  if ( 0 == d2 )  { return false; }

  // various quantities needed to create quartic equation
  // see LHCB/98-040 section 3, equation 3
  const double e     = sqrt(e2);
  const double d     = sqrt(d2);
  const double gamma = evec.angle(dvec);
  const double dx    = d * cos(gamma);
  const double dy    = d * sin(gamma);
  const double r2    = radius * radius;

  // Fill array for quartic equation
  double a[5];
  a[0] =    4 * e2 * d2;
  a[1] =  - 4 * e2 * dy * radius;
  a[2] =   (dy * dy * r2) + ((e+dx) * (e+dx) * r2) - a[0] ;
  a[3] =   2 * e * dy * (e-dx) * radius;
  a[4] =   ( e2 - r2 ) * dy * dy ;

  /*
  // -----------------------------------------------------------------------
  // use full 'GSL' function
  gsl_complex solutions[4];
  if ( 0 == gsl_poly_complex_solve_quartic( a[1]/a[0], // a
  a[2]/a[0], // b
  a[3]/a[0], // c
  a[4]/a[0], // d
  &solutions[0],
  &solutions[1],
  &solutions[2],
  &solutions[3] ) ) { return false; }

  // normal vector to reflection plane
  const HepVector3D nvec1 = evec.cross(dvec);

  // get the required results
  HepVector3D delta[2];
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

  // -----------------------------------------------------------------------
  // use 'hacked' RICH version
  const double sinbeta = solve_quartic_RICH( a[1]/a[0], // a
                                             a[2]/a[0], // b
                                             a[3]/a[0], // c
                                             a[4]/a[0]  // d
                                             );

  // normal vector to reflection plane
  const HepVector3D nvec2 = evec.cross(dvec);

  // use results to form reflection point
  evec.setMag( radius );
  evec.rotate( asin(sinbeta), nvec2 );
  sphReflPoint = CoC + evec;

  // -----------------------------------------------------------------------

  return true;
}
