
//---------------------------------------------------------------------------------
/** @file RichTrSegMakerFromRecoTracks.cpp
 *
 * Implementation file for class : RichTrSegMakerFromRecoTracks
 *
 * CVS Log :-
 * $Id: RichTrSegMakerFromRecoTracks.cpp,v 1.3 2005-11-15 13:39:28 jonrob Exp $
 *
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 * @date 14/01/2002
 */
//---------------------------------------------------------------------------------

// local
#include "RichTrSegMakerFromRecoTracks.h"

// Declaration of the Algorithm Factory
static const  ToolFactory<RichTrSegMakerFromRecoTracks>          Factory ;
const        IToolFactory& RichTrSegMakerFromRecoTracksFactory = Factory ;

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RichTrSegMakerFromRecoTracks::
RichTrSegMakerFromRecoTracks( const std::string& type,
                              const std::string& name,
                              const IInterface* parent )
  : RichToolBase         ( type, name, parent           ),
    m_rayTracing         ( 0                            ),
    m_richPartProp       ( 0                            ),
    m_zTolerance         ( Rich::NRadiatorTypes, 0      ),
    m_nomZstates         ( 2*Rich::NRiches,      0      ),
    m_mirrShift          ( Rich::NRiches,        0      ),
    m_minStateDiff       ( Rich::NRadiatorTypes, 0      ),
    m_trExt1             ( 0                            ),
    m_trExt2             ( 0                            ),
    m_Ext1               ( "TrackHerabExtrapolator"     ),
    m_Ext2               ( "TrackParabolicExtrapolator" ),
    m_usedRads           ( Rich::NRadiatorTypes, true   ),
    m_extrapFromRef      ( false                        )
{

  // the interface
  declareInterface<IRichTrSegMaker>(this);

  // job options

  declareProperty( "PrimaryTrackExtrapolator", m_Ext1     );
  declareProperty( "BackupTrackExtrapolator",  m_Ext2     );
  declareProperty( "UseRadiators",             m_usedRads );
  declareProperty( "ExtrapolateFromReference", m_extrapFromRef );

  // Nominal z positions of states at RICHes
  m_nomZstates[0] = 99.0*cm;    // Place to look for Rich1 entry state
  m_nomZstates[1] = 216.5*cm;   // Place to look for Rich1 exit state
  m_nomZstates[2] = 945.0*cm;   // Place to look for Rich2 entry state
  m_nomZstates[3] = 1190.0*cm;  // Place to look for Rich2 exit state
  declareProperty( "NominalStateZ", m_nomZstates );

  // tolerances on z positions
  m_zTolerance[Rich::Aerogel] = 800*mm;
  m_zTolerance[Rich::C4F10]   = 800*mm;
  m_zTolerance[Rich::CF4]     = 2000*mm;
  declareProperty( "ZTolerances", m_zTolerance );

  // sanity checks on state information
  m_minStateDiff[Rich::Aerogel] = 1*mm;
  m_minStateDiff[Rich::C4F10]   = 25*mm;
  m_minStateDiff[Rich::CF4]     = 50*mm;
  declareProperty( "ZSanityChecks", m_minStateDiff );

  // shifts for mirror correction
  m_mirrShift[Rich::Rich1] = 35*cm;
  m_mirrShift[Rich::Rich2] = 150*cm;
  declareProperty( "MirrorShiftCorr", m_mirrShift );

}

//=============================================================================
// Destructor
//=============================================================================
RichTrSegMakerFromRecoTracks::~RichTrSegMakerFromRecoTracks() { }

//=============================================================================
// Initialisation.
//=============================================================================
StatusCode RichTrSegMakerFromRecoTracks::initialize()
{
  // Sets up various tools and services
  StatusCode sc = RichToolBase::initialize();
  if ( sc.isFailure() ) return sc;

  // Get the track state extrapolators for the RICH
  m_trExt1 = tool<ITrackExtrapolator>( m_Ext1 );
  m_trExt2 = tool<ITrackExtrapolator>( m_Ext2 );

  // Get the RICH tools
  acquireTool( "RichRayTracing", m_rayTracing );
  acquireTool( "RichParticleProperties",  m_richPartProp );

  // get Detector elements for RICH1 and RICH2
  m_rich[Rich::Rich1] = getDet<DeRich>( DeRichLocation::Rich1 );
  m_rich[Rich::Rich2] = getDet<DeRich>( DeRichLocation::Rich2 );

  // get the radiators
  m_radiators[Rich::Aerogel] = getDet<DeRichRadiator>( DeRichRadiatorLocation::Aerogel );
  m_radiators[Rich::C4F10]   = getDet<DeRichRadiator>( DeRichRadiatorLocation::C4F10   );
  m_radiators[Rich::CF4]     = getDet<DeRichRadiator>( DeRichRadiatorLocation::CF4     );
  if ( !m_usedRads[Rich::Aerogel] )
  {
    Warning("Track segments for Aerogel are disabled",StatusCode::SUCCESS);
  }
  if ( !m_usedRads[Rich::C4F10] )
  {
    Warning("Track segments for C4F10 are disabled",StatusCode::SUCCESS);
  }
  if ( !m_usedRads[Rich::CF4] )
  {
    Warning("Track segments for CF4 are disabled",StatusCode::SUCCESS);
  }

  return sc;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode RichTrSegMakerFromRecoTracks::finalize()
{
  // Execute base class method
  return RichToolBase::finalize();
}

//=============================================================================
// Constructs the track segments for a given Track
//=============================================================================
int RichTrSegMakerFromRecoTracks::constructSegments( const ContainedObject * obj,
                                                     std::vector<RichTrackSegment*>& segments )
  const {

  // Try to cast input data to required type for this implementation
  const Track * track = dynamic_cast<const Track *>(obj);
  if ( !track )
  {
    Warning("::constructSegments : Input data object is not of type Track");
    return 0;
  }
  if ( msgLevel(MSG::VERBOSE) )
  {
    verbose() << "Analysing Track key=" << track->key()
              << " history=" << track->history()
              << " : " << track->states().size() << " States at z =";
    for ( std::vector<State*>::const_iterator iS = track->states().begin();
          iS != track->states().end(); ++iS )
    {
      if (*iS) verbose() << " " << (*iS)->z();
    }
    verbose() << endreq;
  }

  // make sure vector is empty
  segments.clear();

  // Loop over all radiators
  for ( Radiators::const_iterator radiator = m_radiators.begin();
        radiator != m_radiators.end();
        ++radiator )
  {

    // which radiator
    const Rich::RadiatorType rad = (*radiator)->radiatorID();
    // is this radiator in use ?
    if ( !m_usedRads[rad] ) continue;
    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << " Considering radiator " << rad << endreq;
    }

    // choose appropriate z start position for initial track states for this radiator
    const double zStart = ( Rich::CF4 == rad ? m_nomZstates[2] : m_nomZstates[0] );
    // Get the track entry state points
    const State * entryPStateRaw = &(track->closestState(zStart));
    if ( !entryPStateRaw || fabs(zStart-entryPStateRaw->z()) > m_zTolerance[rad] ) continue;

    // check above electron threshold
    if ( m_richPartProp->thresholdMomentum(Rich::Electron,rad) > entryPStateRaw->p() )
    {
      if ( msgLevel(MSG::VERBOSE) )
      {
        verbose() << "  -> Below electron cherenkov threshold -> reject" << endreq;
      }
      continue;
    }

    // choose appropriate z end position for initial track states for this radiator
    const double zEnd   = ( Rich::CF4 == rad ? m_nomZstates[3] : m_nomZstates[1] );
    // Get the track enrty state points
    const State * exitPStateRaw = &(track->closestState(zEnd));
    if ( !exitPStateRaw || fabs(zEnd-exitPStateRaw->z()) > m_zTolerance[rad] ) continue;

    // Clone entry state
    State * entryPState = entryPStateRaw->clone();
    if ( !entryPState ) { Warning("Failed to clone State"); continue; }

    // Clone exit state (for aero use entrance point)
    State * exitPState = ( Rich::Aerogel == rad ?
                           entryPStateRaw->clone() : exitPStateRaw->clone() );
    if ( !exitPState ) { Warning("Failed to clone State"); delete entryPState; continue; }

    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "  Found appropriate initial start/end States" << endreq
                << "   EntryPos : " << HepPoint3D(entryPState->x(),entryPState->y(),entryPState->z()) << endreq
                << "   EntryDir : " << HepVector3D(entryPState->tx(),entryPState->ty(),1) << endreq
                << "   ExitPos  : " << HepPoint3D(exitPState->x(),exitPState->y(),exitPState->z()) << endreq
                << "   ExitDir  : " << HepVector3D(exitPState->tx(),exitPState->ty(),1) << endreq;
    }

    // use state closest to the entry point in radiator
    HepPoint3D entryPoint1, exitPoint1;
    bool entryStateOK = false;
    HepPoint3D firstPoint ( entryPState->x(), entryPState->y(), entryPState->z() );
    HepVector3D firstDir  ( entryPState->tx(), entryPState->ty(), 1 );
    if ( (*radiator)->nextIntersectionPoint( firstPoint,
                                             firstDir,
                                             entryPoint1 ) )
    {
      // extrapolate state to the correct z
      if ( moveState( entryPState, entryPoint1.z(), entryPStateRaw ) )
      {
        // find radiator entry and exit points
        if ( entryPState &&
             fabs(entryPState->z() - entryPoint1.z()) < m_zTolerance[rad] )
        {
          if ( (*radiator)->intersectionPoints( HepPoint3D(entryPState->x(),entryPState->y(),entryPState->z()),
                                                HepVector3D(entryPState->tx(),entryPState->ty(),1),
                                                entryPoint1,
                                                exitPoint1) )
          {
            entryStateOK = true;
            if ( msgLevel(MSG::VERBOSE) )
              verbose() << "      Entry state rad intersection points "
                        << entryPoint1 << " " << exitPoint1 << endreq;
          }
        }
      }
    }
    else if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "Failed to intersect entry state" << endreq;
    }

    // If gas radiator try and use exit state to get exit point more precisely
    bool exitStateOK = false;
    HepPoint3D entryPoint2, exitPoint2;
    if ( rad != Rich::Aerogel )
    {
      HepPoint3D lastPoint  ( exitPState->x(), exitPState->y(), exitPState->z() );
      HepVector3D lastDir   ( -exitPState->tx(), -exitPState->ty(), -1 );
      if ( (*radiator)->nextIntersectionPoint(lastPoint,
                                              lastDir,
                                              entryPoint2) )
      {
        // extrapolate state to the correct z
        if ( moveState( exitPState, entryPoint2.z(), exitPStateRaw ) )
        {
          // find radiator entry and exit points
          if ( exitPState &&
               fabs(exitPState->z() - entryPoint2.z()) < m_zTolerance[rad] )
          {
            if ( (*radiator)->intersectionPoints(HepPoint3D(exitPState->x(),exitPState->y(),exitPState->z()),
                                                 HepVector3D(exitPState->tx(),exitPState->ty(),1),
                                                 entryPoint2,
                                                 exitPoint2) )
            {
              exitStateOK = true;
              if ( msgLevel(MSG::VERBOSE) )
                verbose() << "      Exit state rad intersection points "
                          << entryPoint2 << " " << exitPoint2 << endreq;
            }
          }
        }
      }
      else if ( msgLevel(MSG::VERBOSE) )
      {
        verbose() << "Failed to intersect exit state" << endreq;
      }
    } // end !aerogel if

    // transport entry and exit states to best points
    StatusCode sc;
    if ( entryStateOK && exitStateOK )
    {
      if (msgLevel(MSG::VERBOSE)) verbose() << "  Both states OK : Zentry=" << entryPoint1.z()
                                            << " Zexit=" << exitPoint2.z() << endreq;

      // make sure at current z positions
      if (msgLevel(MSG::VERBOSE)) verbose() << "  Checking entry point is at final z=" << entryPoint1.z() << endreq;
      const StatusCode sc1 = moveState( entryPState, entryPoint1.z(), entryPStateRaw );
      if (msgLevel(MSG::VERBOSE)) verbose() << "  Checking exit point is at final z=" << exitPoint2.z() << endreq;
      const StatusCode sc2 = moveState( exitPState,  exitPoint2.z(), exitPStateRaw );
      sc = sc1 && sc2;

    }
    else if ( entryStateOK )
    {
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  Entry state OK : Zentry=" << entryPoint1.z()
                  << " Zexit=" << exitPoint1.z() << endreq;

      if ( Rich::Aerogel != rad )
      {
        // delete current exit state and replace with clone of raw entrance state
        delete exitPState;
        exitPState = entryPStateRaw->clone();
        if ( !exitPState ) { Warning("Failed to clone State"); delete entryPState; continue; }
      }

      // make sure at current z positions
      if (msgLevel(MSG::VERBOSE)) verbose() << "  Checking entry point is at final z= " << entryPoint1.z() << endreq;
      const StatusCode sc1 = moveState( entryPState, entryPoint1.z(), entryPStateRaw );
      if (msgLevel(MSG::VERBOSE)) verbose() << "  Checking exit point is at final z= " << exitPoint1.z() << endreq;
      const StatusCode sc2 = moveState( exitPState, exitPoint1.z(), exitPStateRaw );
      sc = sc1 && sc2;

    }
    else if ( exitStateOK )
    {
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  Exit state OK  : Zentry=" << entryPoint2.z()
                  << " Zexit=" << exitPoint2.z() << endreq;

      // delete current exit state and replace with clone of raw entrance state
      delete entryPState;
      entryPState = exitPStateRaw->clone();
      if ( !entryPState ) { Warning("Failed to clone State"); delete exitPState; continue; }

      // make sure at current z positions
      if (msgLevel(MSG::VERBOSE)) verbose() << "  Checking entry point is at final z= " << entryPoint2.z() << endreq;
      const StatusCode sc1 = moveState( entryPState, entryPoint2.z(), entryPStateRaw );
      if (msgLevel(MSG::VERBOSE)) verbose() << "  Checking exit point is at final z= " << exitPoint2.z() << endreq;
      const StatusCode sc2 = moveState( exitPState,  exitPoint2.z(), exitPStateRaw );
      sc = sc1 && sc2;

    }
    else
    {
      delete entryPState;
      delete exitPState;
      if (msgLevel(MSG::VERBOSE)) verbose() << "  Both states failed" << endreq;
      continue;
    }
    if ( sc.isFailure() )
    {
      delete entryPState;
      delete exitPState;
      if (msgLevel(MSG::VERBOSE)) verbose() << "    --> Failed to use state information. Quitting." << endreq;
      continue;
    }

    // a special hack for the C4F10 - since the aerogel volume
    // is placed INSIDE the C4F10, the default entry point is wrong.
    if ( Rich::C4F10 == rad ) fixC4F10EntryPoint( entryPState, entryPStateRaw );

    // check for intersection with spherical mirror for gas radiators
    // and if need be correct exit point accordingly
    if ( rad != Rich::Aerogel ) correctRadExitMirror( *radiator, exitPState, exitPStateRaw );

    // Final check that info is reaonable
    if ( (entryPState->z() > exitPState->z()) )
    {
      delete entryPState;
      delete exitPState;
      Warning( "Entry state after exit state for " + Rich::text(rad) + " -> rejecting segment",
               StatusCode::SUCCESS, 5 );
      continue;
    }
    if ( (exitPState->z()-entryPState->z()) < m_minStateDiff[rad] )
    {
      delete entryPState;
      delete exitPState;
      Warning( "Track states for " + Rich::text(rad) + " too close in z -> rejecting segment",
               StatusCode::SUCCESS, 5 );
      continue;
    }


    // Create final entry and exit state points and momentum vectors
    const HepPoint3D  entryPoint(entryPState->x(),entryPState->y(),entryPState->z());
    HepVector3D entryStateMomentum( entryPState->tx(),entryPState->ty(), 1 );
    entryStateMomentum.setMag( entryPState->p() );
    const HepPoint3D  exitPoint(exitPState->x(),exitPState->y(),exitPState->z());
    HepVector3D exitStateMomentum( exitPState->tx(),exitPState->ty(), 1 );
    exitStateMomentum.setMag( exitPState->p() );

    // Errors for entry and exit states
    const RichTrackSegment::StateErrors entryErrs ( entryPState );
    const RichTrackSegment::StateErrors exitErrs  ( exitPState  );

    // print out final points
    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "  Found final points :-" << endreq
                << "   Entry : Pnt=" << entryPoint << " Mom=" << entryStateMomentum
                << " Ptot=" << entryStateMomentum.mag() << endreq
                << "   Exit  : Pnt=" << exitPoint  << " Mom=" << exitStateMomentum
                << " Ptot=" << exitStateMomentum.mag() << endreq;
    }

    try
    {

      // For gas radiators transport entry state to mid point to create middle point
      // information for three point RichTrackSegment constructor
      if ( rad != Rich::Aerogel && moveState( entryPState,
                                              (entryPoint.z()+exitPoint.z())/2,
                                              entryPStateRaw ) )
      {

        // middle point information
        const HepPoint3D midPoint(entryPState->x(),entryPState->y(),entryPState->z());
        HepVector3D midStateMomentum( entryPState->tx(),entryPState->ty(), 1 );
        midStateMomentum.setMag( entryPState->p() );

        // errors for middle state
        const RichTrackSegment::StateErrors midErrs( entryPState );

        // Using this information, make radiator segment
        // this version uses 3 states and thus incorporates some concept of track curvature
        //segments.push_back( new RichTrackSegment( RichTrackSegment::UseChordBetweenStates(),
        segments.push_back( new RichTrackSegment( RichTrackSegment::UseAllStateVectors(),
                                                  entryPoint,entryStateMomentum,
                                                  midPoint,  midStateMomentum,
                                                  exitPoint, exitStateMomentum,
                                                  rad, (*radiator)->rich(),
                                                  entryErrs, midErrs, exitErrs ) );

      }
      else
      {

        // Using this information, make radiator segment
        // this version uses 2 states and thus forces a straight line approximation
        //segments.push_back( new RichTrackSegment( RichTrackSegment::UseChordBetweenStates(),
        segments.push_back( new RichTrackSegment( RichTrackSegment::UseAllStateVectors(),
                                                  entryPoint,entryStateMomentum,
                                                  exitPoint, exitStateMomentum,
                                                  rad, (*radiator)->rich(),
                                                  entryErrs, exitErrs ) );

      }

    }
    catch ( const std::exception & excpt )
    {
      Warning( "Exception whilst creating RichTrackSegment '"+std::string(excpt.what())+"'" );
    }

    // Clean up cloned states
    delete entryPState;
    delete exitPState;

  } // end loop over radiators

  // Final printout of states, to see if anything has changed ...
  if ( msgLevel(MSG::VERBOSE) )
  {
    verbose() << "Finished with Track key=" << track->key()
              << " history=" << track->history()
              << " : " << track->states().size() << " States at z =";
    for ( std::vector<State*>::const_iterator iS = track->states().begin();
          iS != track->states().end(); ++iS )
    {
      if (*iS) verbose() << " " << (*iS)->z();
    }
    verbose() << endreq;
  }

  // return value is number of segments formed
  return segments.size();
}


//====================================================================================================
// fixup C4F10 entry point
void RichTrSegMakerFromRecoTracks::fixC4F10EntryPoint( State *& state,
                                                       const State * refState ) const
{

  HepPoint3D dummyPoint, aerogelExitPoint;
  if ( m_radiators[Rich::Aerogel]->intersectionPoints(HepPoint3D(state->x(),state->y(),state->z()),
                                                      HepVector3D(state->tx(),state->ty(),1),
                                                      dummyPoint,
                                                      aerogelExitPoint) ) {
    if ( aerogelExitPoint.z() > state->z() )
    {
      if (msgLevel(MSG::VERBOSE)) verbose() << "   Correcting C4F10 entry point" << endreq;
      moveState( state, aerogelExitPoint.z(), refState );
    }
  }

}
//====================================================================================================

//====================================================================================================
void RichTrSegMakerFromRecoTracks::correctRadExitMirror( const DeRichRadiator* radiator,
                                                         State *& state,
                                                         const State * refState ) const
{

  // get rich information
  const Rich::DetectorType rich = radiator->rich();

  // move state to be on the inside of the mirror
  moveState( state, state->z() - m_mirrShift[rich], refState );
  bool correct = false;

  // find mirror intersection using the reflect method
  HepPoint3D intersection(state->x(),state->y(),state->z());
  HepVector3D tempDir(state->tx(),state->ty(),1);

  if ( m_rayTracing->reflectSpherical( intersection,
                                       tempDir,
                                       m_rich[rich]->nominalCentreOfCurvature(m_rich[rich]->side(intersection)),
                                       m_rich[rich]->sphMirrorRadius() ) )
  {
    if ( radiator->geometry()->isInside(intersection) ) { correct = true; }
  }

  // finally, update state
  if ( correct )
  {
    if (msgLevel(MSG::VERBOSE)) verbose() << "   Correcting exit point to spherical mirror" << endreq;
    moveState( state, intersection.z(), refState );
  }
  else
  {
    if (msgLevel(MSG::VERBOSE)) verbose() << "   Failed to correct exit state to spherical mirror" << endreq;
    moveState( state, state->z() + m_mirrShift[rich], refState );
  }

}
//====================================================================================================

//====================================================================================================
StatusCode RichTrSegMakerFromRecoTracks::moveState( State *& stateToMove,
                                                    const double z,
                                                    const State * refState ) const
{
  // Check if requested move is big enough to bother with
  if ( fabs(stateToMove->z() - z) > 1*mm ) {

    // debug printout
    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "    --> Extrapolating state from "
                << HepPoint3D(stateToMove->x(),stateToMove->y(),stateToMove->z()) << endreq;
    }

    if ( m_extrapFromRef && refState )
    {
      // Delete current working state and start fresh from reference state
      delete stateToMove;
      stateToMove = refState->clone();
      if ( msgLevel(MSG::VERBOSE) )
      {
        verbose() << "      --> Using reference state  "
                  << HepPoint3D(stateToMove->x(),stateToMove->y(),stateToMove->z()) << endreq;
      }
    }

    // try first with the primary extrapolator
    if ( !m_trExt1->propagate(*stateToMove,z) )
    {
      // if that fails, try the backup one
      if ( m_trExt2->propagate(*stateToMove,z) )
      {
        Warning(m_Ext1+" failed -> reverted to "+m_Ext2,StatusCode::SUCCESS);
      } else
      {
        // Both failed ...
        Warning("Failed to transport state using "+m_Ext1+" or "+m_Ext2);
        return StatusCode::FAILURE;
      }
    }

    // debug printout
    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "                            to   "
                << HepPoint3D(stateToMove->x(),stateToMove->y(),stateToMove->z()) << endreq;
    }

  }

  return StatusCode::SUCCESS;
}
//====================================================================================================
