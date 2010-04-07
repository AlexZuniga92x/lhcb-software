
//---------------------------------------------------------------------------------
/** @file RichDetailedTrSegMakerFromRecoTracks.cpp
 *
 * Implementation file for class : Rich::Rec::DetailedTrSegMakerFromRecoTracks
 *
 * CVS Log :-
 * $Id: RichDetailedTrSegMakerFromRecoTracks.cpp,v 1.11 2009-12-08 10:28:37 jonrob Exp $
 *
 * @author Chris Jones   Christopher.Rob.Jones@cern.ch
 * @date 14/01/2002
 */
//---------------------------------------------------------------------------------

// local
#include "RichDetailedTrSegMakerFromRecoTracks.h"

// Gaudi
#include "GaudiKernel/ToolFactory.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//-------------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( DetailedTrSegMakerFromRecoTracks );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
DetailedTrSegMakerFromRecoTracks::
DetailedTrSegMakerFromRecoTracks( const std::string& type,
                                  const std::string& name,
                                  const IInterface* parent )
  : BaseTrSegMakerFromRecoTracks ( type, name, parent   ),
    m_rayTracing         ( NULL                         ),
    m_richPartProp       ( NULL                         ),
    m_radTool            ( NULL                         ),
    m_nomZstates         ( 2*Rich::NRiches,      0      ),
    m_trExt1             ( NULL                         ),
    m_trExt2             ( NULL                         ),
    m_trExt1Name         ( ""                           ),
    m_trExt2Name         ( ""                           ),
    m_extrapFromRef      ( false                        ),
    m_minZmove           ( 1 * Gaudi::Units::mm         )
{
  using namespace boost::assign;
  using namespace Gaudi::Units;

  // context specific defaults
  if ( contextContains("HLT") )
  {
    m_trExt1Name   = "TrackParabolicExtrapolator";    
    m_trExt2Name   = "TrackLinearExtrapolator";
    // Rads                  Aerogel   Rich1Gas  Rich2Gas
    m_zTolerance   = list_of (3000*mm) (3000*mm) (4000*mm) ;
    m_minRadLength = list_of (0*mm)    (500*mm)  (1500*mm) ;
  }
  else // offline
  { 
    m_trExt1Name   = "TrackRungeKuttaExtrapolator";
    m_trExt2Name   = "TrackParabolicExtrapolator";
    // Rads                  Aerogel   Rich1Gas  Rich2Gas
    m_zTolerance   = list_of (800*mm)  (2000*mm) (3000*mm) ;
    m_minRadLength = list_of (0*mm)    (500*mm)  (1500*mm) ;
  }

  // job options

  declareProperty( "PrimaryTrackExtrapolator", m_trExt1Name    );
  declareProperty( "BackupTrackExtrapolator",  m_trExt2Name    );
  declareProperty( "ExtrapolateFromReference", m_extrapFromRef );
  declareProperty( "MinimumZMove",             m_minZmove      );

  // Nominal z positions of states at RICHes
  // do not have to be precisely accurate
  m_nomZstates[0] =   99.0*Gaudi::Units::cm; // Place to look for Rich1 entry state
  m_nomZstates[1] =  216.5*Gaudi::Units::cm; // Place to look for Rich1 exit state
  m_nomZstates[2] =  945.0*Gaudi::Units::cm; // Place to look for Rich2 entry state
  m_nomZstates[3] = 1190.0*Gaudi::Units::cm; // Place to look for Rich2 exit state
  declareProperty( "NominalStateZ", m_nomZstates );

  // tolerances on z positions
  declareProperty( "ZTolerances", m_zTolerance );

  // sanity checks on state information
  declareProperty( "ZSanityChecks", m_minStateDiff = list_of(1*mm)(25*mm)(50*mm) );

  // shifts for mirror correction
  declareProperty( "MirrorShiftCorr", m_mirrShift = list_of(35*cm)(150*cm) );

  // Type of track segments to create
  declareProperty( "SegmentType", m_trSegTypeJO = "AllStateVectors" );

  // min path length
  declareProperty( "MinRadiatorPathLength", m_minRadLength );

}

//=============================================================================
// Destructor
//=============================================================================
DetailedTrSegMakerFromRecoTracks::~DetailedTrSegMakerFromRecoTracks() { }

//=============================================================================
// Initialisation.
//=============================================================================
StatusCode DetailedTrSegMakerFromRecoTracks::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = BaseTrSegMakerFromRecoTracks::initialize();
  if ( sc.isFailure() ) return sc;

  // load primary track extrapolator ( backup is loaded on-demand )
  m_trExt1 = tool<ITrackExtrapolator>( m_trExt1Name );

  // Get the RICH tools
  acquireTool( "RichRayTracing",          m_rayTracing   );
  acquireTool( "RichParticleProperties",  m_richPartProp );
  acquireTool( "RichRadiatorTool",        m_radTool      );

  // get Detector elements for RICH1 and RICH2
  m_rich[Rich::Rich1] = getDet<DeRich>( DeRichLocations::Rich1 );
  m_rich[Rich::Rich2] = getDet<DeRich>( DeRichLocations::Rich2 );

  // get the radiators
  m_radiators[Rich::Aerogel]  = getDet<DeRichRadiator>( DeRichLocations::Aerogel  );
  m_radiators[Rich::Rich1Gas] = getDet<DeRichRadiator>( DeRichLocations::Rich1Gas );
  m_radiators[Rich::Rich2Gas] = getDet<DeRichRadiator>( DeRichLocations::Rich2Gas );

  if ( m_extrapFromRef )
  {
    info() << "Will perform all track extrapolations from reference states" << endmsg;
  }

  // Define the segment type
  if      ( "AllStateVectors" == m_trSegTypeJO )
  {
    info() << "Will create track segments using all State information" << endmsg;
    m_trSegType = LHCb::RichTrackSegment::UseAllStateVectors;
  }
  else if ( "Chord" == m_trSegTypeJO )
  {
    info() << "Will create track segments using the 'chord' direction definition" << endmsg;
    m_trSegType = LHCb::RichTrackSegment::UseChordBetweenStates;
  }
  else
  {
    return Error( "Unknown RichTrackSegment type " + m_trSegTypeJO );
  }

  info() << "Min radiator path lengths (aero/R1Gas/R2Gas) : " 
         << m_minRadLength << " mm " << endmsg;

  return sc;
}

//=============================================================================
// Constructs the track segments for a given Track
//=============================================================================
int
DetailedTrSegMakerFromRecoTracks::
constructSegments( const ContainedObject * obj,
                   std::vector<LHCb::RichTrackSegment*>& segments )
  const {

  // Try to cast input data to required type for this implementation
  const LHCb::Track * track = dynamic_cast<const LHCb::Track *>(obj);
  if ( !track )
  {
    Warning("Input data object is not of type Track").ignore();
    return 0;
  }
  if ( msgLevel(MSG::VERBOSE) )
  {
    verbose() << "Analysing Track key=" << track->key()
              << " history=" << track->history()
              << " : " << track->states().size() << " States at z =";
    for ( std::vector<LHCb::State*>::const_iterator iS = track->states().begin();
          iS != track->states().end(); ++iS )
    {
      if (*iS) verbose() << " " << (*iS)->z();
    }
    verbose() << endmsg;
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
    if ( !usedRads(rad) ) continue;
    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << " Considering radiator " << rad << endmsg;
    }

    // choose appropriate z start position for initial track states for this radiator
    const double zStart = ( Rich::Rich2Gas == rad ? m_nomZstates[2] : m_nomZstates[0] );

    // Get the track entry state points
    const LHCb::State * entryPStateRaw = &(track->closestState(zStart));
    if ( !entryPStateRaw ) { Error( "Problem getting track state" ).ignore(); continue; }

    // check tolerance
    if ( fabs(zStart-entryPStateRaw->z()) > m_zTolerance[rad] ) 
    {
      if ( msgLevel(MSG::VERBOSE) )
      {
        verbose() << "  -> Entry State : Requested z=" << zStart << " found z=" 
                  << entryPStateRaw->z() << " failed tolerance check dz=" 
                  << m_zTolerance[rad] << endmsg;
      }
      continue;
    }

    // check above electron threshold
    if ( m_richPartProp->thresholdMomentum(Rich::Electron,rad) > entryPStateRaw->p() )
    {
      if ( msgLevel(MSG::VERBOSE) )
      {
        verbose() << "  -> Below electron cherenkov threshold -> reject" << endmsg;
      }
      continue;
    }

    // choose appropriate z end position for initial track states for this radiator
    const double zEnd   = ( Rich::Rich2Gas == rad ? m_nomZstates[3] : 
                            Rich::Aerogel  == rad ? m_nomZstates[0] : 
                            m_nomZstates[1] );

    // Get the track enrty state points
    const LHCb::State * exitPStateRaw = &(track->closestState(zEnd));
    if ( !exitPStateRaw ) { Error( "Problem getting track state" ).ignore(); continue; }

    // check tolerance
    if ( fabs(zEnd-exitPStateRaw->z()) > m_zTolerance[rad] )
    {
      if ( msgLevel(MSG::VERBOSE) )
      {
        verbose() << "  -> Exit State  : Requested z=" << zEnd << " found z=" 
                  << exitPStateRaw->z() << " failed tolerance check dz=" 
                  << m_zTolerance[rad] << endmsg;
      }
      continue;
    }

    // Clone entry state
    LHCb::State * entryPState = entryPStateRaw->clone();
    if ( !entryPState ) { Warning("Failed to clone State").ignore(); continue; }

    // Clone exit state (for aero use entrance point)
    LHCb::State * exitPState = ( Rich::Aerogel == rad ?
                                 entryPStateRaw->clone() : exitPStateRaw->clone() );
    if ( !exitPState ) { Warning("Failed to clone State").ignore(); delete entryPState; continue; }

    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "  Found appropriate initial start/end States" << endmsg
                << "   EntryPos : "
                << entryPState->position() << endmsg
                << "   EntryDir : "
                << entryPState->slopes() << endmsg
                << "   ExitPos  : "
                << exitPState->position() << endmsg
                << "   ExitDir  : "
                << exitPState->slopes() << endmsg;
    }

    // use state closest to the entry point in radiator
    Gaudi::XYZPoint entryPoint1;
    RichRadIntersection::Vector intersects1;
    bool entryStateOK = false;
    if ( getNextInterPoint( entryPState->position(),
                            entryPState->slopes(),
                            *radiator, entryPoint1 ) )
    {
      // extrapolate state to the correct z
      if ( moveState( entryPState, entryPoint1.z(), entryPStateRaw ) )
      {
        // find radiator entry and exit points
        if ( entryPState &&
             fabs(entryPState->z() - entryPoint1.z()) < m_zTolerance[rad] )
        {
          if ( getRadIntersections( entryPState->position(),
                                    entryPState->slopes(),
                                    *radiator,
                                    intersects1 ) > 0 )
          {
            entryStateOK = true;
            entryPoint1 = intersects1.front().entryPoint();
            if ( msgLevel(MSG::VERBOSE) )
              verbose() << "      Entry state rad intersection points "
                        << intersects1 << endmsg;
          }
        }
      }
    }
    else if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "Failed to intersect entry state" << endmsg;
    }

    // If gas radiator try and use exit state to get exit point more precisely
    bool exitStateOK = false;
    Gaudi::XYZPoint entryPoint2;
    RichRadIntersection::Vector intersects2;
    if ( rad != Rich::Aerogel )
    {
      if ( getNextInterPoint(  exitPState->position(),
                               -exitPState->slopes(),
                               *radiator, entryPoint2 ) )
      {
        // extrapolate state to the correct z
        if ( moveState( exitPState, entryPoint2.z(), exitPStateRaw ) )
        {
          // find radiator entry and exit points
          if ( exitPState &&
               fabs(exitPState->z() - entryPoint2.z()) < m_zTolerance[rad] )
          {
            if ( getRadIntersections( exitPState->position(),
                                      exitPState->slopes(),
                                      *radiator,
                                      intersects2 ) > 0 )
            {
              exitStateOK = true;
              entryPoint2 = intersects2.front().entryPoint();
              if ( msgLevel(MSG::VERBOSE) )
                verbose() << "      Exit state rad intersection points "
                          << intersects2 << endmsg;
            }
          }
        }
      }
      else if ( msgLevel(MSG::VERBOSE) )
      {
        verbose() << "Failed to intersect exit state" << endmsg;
      }
    } // end !aerogel if

    // transport entry and exit states to best points
    bool sc = false;
    if ( entryStateOK && exitStateOK )
    {
      if (msgLevel(MSG::VERBOSE)) verbose() << "  Both states OK : Zentry=" << entryPoint1.z()
                                            << " Zexit=" << intersects2.back().exitPoint().z() 
                                            << endmsg;

      // make sure at current z positions
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  Checking entry point is at final z=" << entryPoint1.z() << endmsg;
      const bool sc1 = moveState( entryPState, entryPoint1.z(), entryPStateRaw );
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  Checking exit point is at final z=" << intersects2.back().exitPoint().z() 
                  << endmsg;
      const bool sc2 = moveState( exitPState,  intersects2.back().exitPoint().z(), exitPStateRaw );
      sc = sc1 && sc2;
      
    }
    else if ( entryStateOK )
    {
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  Entry state OK : Zentry=" << entryPoint1.z()
                  << " Zexit=" << intersects1.back().exitPoint().z() << endmsg;

      if ( Rich::Aerogel != rad )
      {
        // delete current exit state and replace with clone of raw entrance state
        delete exitPState;
        exitPState = entryPStateRaw->clone();
        if ( !exitPState ) { Warning("Failed to clone State").ignore(); delete entryPState; continue; }
      }

      // make sure at current z positions
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  Checking entry point is at final z= " << entryPoint1.z() << endmsg;
      const bool sc1 = moveState( entryPState, entryPoint1.z(), entryPStateRaw );
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  Checking exit point is at final z= " << intersects1.back().exitPoint().z() 
                  << endmsg;
      const bool sc2 = moveState( exitPState, intersects1.back().exitPoint().z(), exitPStateRaw );
      sc = sc1 && sc2;

    }
    else if ( exitStateOK )
    {
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  Exit state OK  : Zentry=" << entryPoint2.z()
                  << " Zexit=" << intersects2.back().exitPoint().z() << endmsg;

      // delete current entry state and replace with clone of raw entrance state
      delete entryPState;
      entryPState = exitPStateRaw->clone();
      if ( !entryPState ) { Warning("Failed to clone State").ignore(); delete exitPState; continue; }

      // make sure at current z positions
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  Checking entry point is at final z= " << entryPoint2.z() << endmsg;
      const bool sc1 = moveState( entryPState, entryPoint2.z(), entryPStateRaw );
      if (msgLevel(MSG::VERBOSE))
        verbose() << "  Checking exit point is at final z= " << intersects2.back().exitPoint().z() 
                  << endmsg;
      const bool sc2 = moveState( exitPState,  intersects2.back().exitPoint().z(), exitPStateRaw );
      sc = sc1 && sc2;

    }
    else
    {
      // no valid extrapolations, so quit skip this track/radiator
      delete entryPState;
      delete exitPState;
      if (msgLevel(MSG::VERBOSE)) verbose() << "  Both states failed" << endmsg;
      continue;
    }

    // Test final status code
    if ( !sc )
    {
      delete entryPState;
      delete exitPState;
      if (msgLevel(MSG::VERBOSE))
        verbose() << "    --> Failed to use state information. Quitting." << endmsg;
      continue;
    }

    //---------------------------------------------------------------------------------------------
    // Correction for beam pipe intersections
    if ( checkBeamPipe(rad) )
    {

      // Get intersections with beam pipe using DeRich object
      const Gaudi::XYZVector vect = exitPState->position() - entryPState->position();
      Gaudi::XYZPoint inter1, inter2;
      const DeRichBeamPipe::BeamPipeIntersectionType intType
        = deBeam(rad)->intersectionPoints( entryPState->position(), vect, inter1, inter2 );

      if (msgLevel(MSG::VERBOSE))
        verbose() << "  --> Beam Intersects : " << intType << " : " 
                  << inter1 << " " << inter2 << endmsg;

      sc = true;
      if ( intType == DeRichBeamPipe::NoIntersection )
      {
        if (msgLevel(MSG::VERBOSE))
          verbose() << "   --> No beam intersections -> No corrections needed" << endmsg;
      }
      else if ( intType == DeRichBeamPipe::FrontAndBackFace )
      {
        if (msgLevel(MSG::VERBOSE))
          verbose() << "   --> Inside beam pipe -> Reject segment" << endmsg;
        delete entryPState;
        delete exitPState;
        continue;
      }
      else if ( intType == DeRichBeamPipe::FrontFaceAndCone )
      {
        // Update entry point to exit point on cone
        if (msgLevel(MSG::VERBOSE))
          verbose() << "   --> Correcting entry point to point on cone" << endmsg;
        sc = moveState( entryPState, inter2.z(), entryPStateRaw );
      }
      else if ( intType == DeRichBeamPipe::BackFaceAndCone )
      {
        // Update exit point to entry point on cone
        if (msgLevel(MSG::VERBOSE))
          verbose() << "   --> Correcting exit point to point on cone" << endmsg;
        sc = moveState( exitPState, inter1.z(), exitPStateRaw );
      }
      if ( !sc )
      {
        delete entryPState;
        delete exitPState;
        if (msgLevel(MSG::VERBOSE))
          verbose() << "    --> Error fixing radiator entry/exit points for beam-pipe. Quitting." 
                    << endmsg;
        continue;
      }

    }
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    // a special hack for the Rich1Gas - since the aerogel volume
    // is placed INSIDE the Rich1Gas, the default entry point is wrong.
    //---------------------------------------------------------------------------------------------
    if ( Rich::Rich1Gas == rad ) fixRich1GasEntryPoint( entryPState, entryPStateRaw );
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    // check for intersection with spherical mirror for gas radiators
    // and if need be correct exit point accordingly
    //---------------------------------------------------------------------------------------------
    if ( Rich::Aerogel != rad  ) correctRadExitMirror( *radiator, exitPState, exitPStateRaw );
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    // Final check that info is reasonable
    //---------------------------------------------------------------------------------------------
    const bool Zcheck     = entryPState->z() > exitPState->z();
    const bool ZdiffCheck = (exitPState->z()-entryPState->z()) < m_minStateDiff[rad];
    counter( "Entry state after exit state for " + Rich::text(rad)     ) += (int)Zcheck;
    counter( "Track states for " + Rich::text(rad) + " too close in z" ) += (int)ZdiffCheck;
    if ( Zcheck || ZdiffCheck )
    {
      delete entryPState;
      delete exitPState;
      continue;
    }
    //---------------------------------------------------------------------------------------------

    //---------------------------------------------------------------------------------------------
    // Radiator path length cut
    //---------------------------------------------------------------------------------------------
    if ( (exitPState->position()-entryPState->position()).Mag2() < m_minRadLength[rad]*m_minRadLength[rad] )
    {
      if (msgLevel(MSG::VERBOSE))
        verbose() << "    --> Path length too short -> rejecting segment" << endmsg;
      delete entryPState;
      delete exitPState;
      continue;
    }
    //---------------------------------------------------------------------------------------------

    // Create final entry and exit state points and momentum vectors
    const Gaudi::XYZPoint  entryPoint( entryPState->position() );
    Gaudi::XYZVector entryStateMomentum( entryPState->slopes() );
    entryStateMomentum *= entryPState->p()/std::sqrt(entryStateMomentum.Mag2());
    const Gaudi::XYZPoint  exitPoint(exitPState->position());
    Gaudi::XYZVector exitStateMomentum( exitPState->slopes() );
    exitStateMomentum *= exitPState->p()/std::sqrt(exitStateMomentum.Mag2());

    // Update final intersections
    RichRadIntersection::Vector & final_intersects = ( entryStateOK ? intersects1 : intersects2 );
    final_intersects.front().setEntryPoint(entryPoint);
    final_intersects.front().setEntryMomentum(entryStateMomentum);
    final_intersects.back().setExitPoint(exitPoint);
    final_intersects.back().setExitMomentum(exitStateMomentum);

    // Errors for entry and exit states
    const LHCb::RichTrackSegment::StateErrors entryErrs ( entryPState->errX2(),
                                                          entryPState->errY2(),
                                                          entryPState->errTx2(),
                                                          entryPState->errTy2(),
                                                          entryPState->errP2() );
    const LHCb::RichTrackSegment::StateErrors exitErrs  ( exitPState->errX2(),
                                                          exitPState->errY2(),
                                                          exitPState->errTx2(),
                                                          exitPState->errTy2(),
                                                          exitPState->errP2() );

    // print out final points
    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "  Found final points :-" << endmsg
                << "   Entry : Pnt=" << entryPoint << " Mom=" << entryStateMomentum
                << " Ptot=" << std::sqrt(entryStateMomentum.Mag2()) << endmsg
                << "   Exit  : Pnt=" << exitPoint  << " Mom=" << exitStateMomentum
                << " Ptot=" << std::sqrt(exitStateMomentum.Mag2()) << endmsg;
    }

    try
    {

      // For gas radiators transport entry state to mid point to create middle point
      // information for three point RichTrackSegment constructor
      if ( rad != Rich::Aerogel )
      {

        // data for middle state
        LHCb::RichTrackSegment::StateErrors midErrs;
        Gaudi::XYZPoint midPoint;
        Gaudi::XYZVector midMomentum;
        const bool OK = createMiddleInfo( rad,
                                          entryPState, entryPStateRaw,
                                          exitPState,  exitPStateRaw,
                                          midPoint, midMomentum, midErrs );

        if ( OK )
        {
          // Using this information, make radiator segment
          // this version uses 3 states and thus incorporates some concept of track curvature
          segments.push_back( new LHCb::RichTrackSegment( m_trSegType,
                                                          final_intersects,
                                                          midPoint,  midMomentum,
                                                          rad, (*radiator)->rich(),
                                                          entryErrs, midErrs, exitErrs ) );
        }
        else
        {
          // Using this information, make radiator segment
          // this version uses 2 states and thus forces a straight line approximation
          segments.push_back( new LHCb::RichTrackSegment( m_trSegType,
                                                          final_intersects,
                                                          rad, (*radiator)->rich(),
                                                          entryErrs, exitErrs ) );
        }

      }
      else
      {

        // Using this information, make radiator segment
        // this version uses 2 states and thus forces a straight line approximation
        segments.push_back( new LHCb::RichTrackSegment( m_trSegType,
                                                        final_intersects,
                                                        rad, (*radiator)->rich(),
                                                        entryErrs, exitErrs ) );

      }

    }
    catch ( const std::exception & excpt )
    {
      Warning( "Exception whilst creating RichTrackSegment '"+std::string(excpt.what())+"'" ).ignore();
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
    for ( std::vector<LHCb::State*>::const_iterator iS = track->states().begin();
          iS != track->states().end(); ++iS )
    {
      if (*iS) verbose() << " " << (*iS)->z();
    }
    verbose() << endmsg;
  }

  // return value is number of segments formed
  return segments.size();
}
//====================================================================================================

//====================================================================================================
// creates middle point info
bool
DetailedTrSegMakerFromRecoTracks::
createMiddleInfo( const Rich::RadiatorType rad,
                  LHCb::State *& fState,
                  const LHCb::State * fStateRef,
                  LHCb::State *& lState,
                  const LHCb::State * lStateRef,
                  Gaudi::XYZPoint & midPoint,
                  Gaudi::XYZVector & midMomentum,
                  LHCb::RichTrackSegment::StateErrors & errors ) const
{
  if (msgLevel(MSG::VERBOSE))
    verbose() << "   --> Creating middle point information" << endmsg;

  // middle point z position
  const double midZ = (fState->position().z()+lState->position().z())/2;

  // move start state to this z
  const bool moveFirst = moveState( fState, midZ, fStateRef );

  // move end state to this z
  const bool moveLast = ( Rich::Rich1Gas == rad ?
                          moveState( lState, midZ, lStateRef ) : false );

  if ( moveFirst && moveLast )
  {
    midPoint     = fState->position() + (lState->position()-fState->position())/2;
    midMomentum  = (fState->slopes()+lState->slopes())/2;
    midMomentum *= (fState->p()+lState->p()) / (2.0*std::sqrt(midMomentum.Mag2()));
    errors = LHCb::RichTrackSegment::StateErrors( (fState->errX2()+lState->errX2())/2,
                                                  (fState->errY2()+lState->errY2())/2,
                                                  (fState->errTx2()+lState->errTx2())/2,
                                                  (fState->errTy2()+lState->errTy2())/2,
                                                  (fState->errP2()+lState->errP2())/2 );
  }
  else if ( moveFirst )
  {
    midPoint     = fState->position();
    midMomentum  = fState->slopes();
    midMomentum *= fState->p() / std::sqrt(midMomentum.Mag2());
    errors = LHCb::RichTrackSegment::StateErrors( fState->errX2(),
                                                  fState->errY2(),
                                                  fState->errTx2(),
                                                  fState->errTy2(),
                                                  fState->errP2() );
  }
  else if ( moveLast )
  {
    midPoint     = lState->position();
    midMomentum  = lState->slopes();
    midMomentum *= lState->p() / std::sqrt(midMomentum.Mag2());
    errors = LHCb::RichTrackSegment::StateErrors( lState->errX2(),
                                                  lState->errY2(),
                                                  lState->errTx2(),
                                                  lState->errTy2(),
                                                  lState->errP2() );
  }

  return ( moveFirst || moveLast );
}
//====================================================================================================

//====================================================================================================
// Get radiator intersections
unsigned int
DetailedTrSegMakerFromRecoTracks::
getRadIntersections( const Gaudi::XYZPoint  & point,
                     const Gaudi::XYZVector & direction,
                     const DeRichRadiator * rad,
                     RichRadIntersection::Vector & intersections ) const
{
  // clear the intersections
  intersections.clear();

  // get the intersections
  m_radTool->intersections( point, direction, rad->radiatorID(), intersections );

  // return the nuber of intersections found
  return intersections.size();
}
//====================================================================================================

//====================================================================================================
// Short cut method to get just first intersection point
bool
DetailedTrSegMakerFromRecoTracks::
getNextInterPoint( const Gaudi::XYZPoint&   point,
                   const Gaudi::XYZVector&  direction,
                   const DeRichRadiator * rad,
                   Gaudi::XYZPoint& interP ) const
{
  RichRadIntersection::Vector intersections;
  bool OK = false;
  if ( 0 < getRadIntersections(point,direction,rad,intersections) )
  {
    OK = true;
    interP = intersections.front().entryPoint();
  }
  return OK;
}
//====================================================================================================

//====================================================================================================
// fixup Rich1Gas entry point
void
DetailedTrSegMakerFromRecoTracks::fixRich1GasEntryPoint( LHCb::State *& state,
                                                         const LHCb::State * refState ) const
{
  RichRadIntersection::Vector intersections;
  if ( 0 < getRadIntersections ( state->position(),
                                 state->slopes(),
                                 m_radiators[Rich::Aerogel],
                                 intersections ) )
  {
    const Gaudi::XYZPoint & aerogelExitPoint = intersections.back().exitPoint();
    if ( aerogelExitPoint.z() > state->z() )
    {
      if (msgLevel(MSG::VERBOSE)) verbose() << "   Correcting Rich1Gas entry point" << endmsg;
      const bool sc = moveState( state, aerogelExitPoint.z(), refState );
      if ( !sc ) Warning( "Problem correcting RICH1Gas entry point for aerogel" ).ignore();
    }
  }
}
//====================================================================================================

//====================================================================================================
void
DetailedTrSegMakerFromRecoTracks::correctRadExitMirror( const DeRichRadiator* radiator,
                                                        LHCb::State *& state,
                                                        const LHCb::State * refState ) const
{
  if (msgLevel(MSG::VERBOSE))
    verbose() << "   --> Attempting Correction to exit point for spherical mirror" << endmsg;

  bool sc = true;

  // get rich information
  const Rich::DetectorType rich = radiator->rich();

  // initial z position of state
  const double initialZ = state->z();

  // move state to be on the inside of the mirror
  verbose() << "    --> Moving state first to be inside mirror" << endmsg;
  sc = sc && moveState( state, state->z() - m_mirrShift[rich], refState );
  bool correct = false;

  // find mirror intersection using the reflect method
  Gaudi::XYZPoint intersection( state->position() );
  Gaudi::XYZVector tempDir(state->slopes());

  if ( m_rayTracing->reflectSpherical( intersection,
                                       tempDir,
                                       m_rich[rich]->nominalCentreOfCurvature(m_rich[rich]->side(intersection)),
                                       m_rich[rich]->sphMirrorRadius() ) )
  {
    if ( intersection.z() < initialZ && radiator->geometry()->isInside(intersection) )
    {
      correct = true;
    }
  }

  // finally, update state
  if ( correct )
  {
    if (msgLevel(MSG::VERBOSE))
      verbose() << "    --> Found correction is needed" << endmsg;
    sc = sc && moveState( state, intersection.z(), refState );
  }
  else
  {
    if (msgLevel(MSG::VERBOSE))
      verbose() << "    --> Found correction not needed. Moving back to original position" << endmsg;
    sc = sc && moveState( state, initialZ, refState );
  }

  if ( !sc ) Warning( "Problem correcting segment exit to mirror intersection" ).ignore();
}
//====================================================================================================

//====================================================================================================
bool
DetailedTrSegMakerFromRecoTracks::moveState( LHCb::State *& stateToMove,
                                             const double z,
                                             const LHCb::State * refState ) const
{
  // Check if requested move is big enough to bother with
  if ( fabs(stateToMove->z() - z) > m_minZmove )
  {

    // debug printout
    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "    --> Extrapolating state from "
                << stateToMove->position() << endmsg;
    }

    if ( m_extrapFromRef && refState )
    {
      // Delete current working state and start fresh from reference state
      delete stateToMove;
      stateToMove = refState->clone();
      if ( msgLevel(MSG::VERBOSE) )
      {
        verbose() << "      --> Using reference state  "
                  << stateToMove->position() << endmsg;
      }
    }

    // try first with the primary extrapolator
    if ( !primaryExtrapolator()->propagate(*stateToMove,z) )
    {
      // if that fails, try the backup one
      if ( backupExtrapolator()->propagate(*stateToMove,z) )
      {
        Warning("'"+m_trExt1Name+"' failed -> successfully reverted to '"+
                m_trExt2Name+"'",StatusCode::SUCCESS).ignore();
      } else
      {
        // Both failed ...
        Warning("Failed to extrapolate state using either '"+
                m_trExt1Name+"' or '"+m_trExt2Name+"'").ignore();
        return false;
      }
    }

    // debug printout
    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "                            to   "
                << stateToMove->position() << endmsg;
    }

  }

  return true;
}
//====================================================================================================
