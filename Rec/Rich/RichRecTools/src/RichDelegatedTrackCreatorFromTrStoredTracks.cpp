// $Id: RichDelegatedTrackCreatorFromTrStoredTracks.cpp,v 1.1 2004-06-10 14:39:21 jonesc Exp $

// local
#include "RichDelegatedTrackCreatorFromTrStoredTracks.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichDelegatedTrackCreatorFromTrStoredTracks
//
// 15/03/2002 : Chris Jones   Christopher.Rob.Jones@cern.ch
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
static const  ToolFactory<RichDelegatedTrackCreatorFromTrStoredTracks>          s_factory ;
const        IToolFactory& RichDelegatedTrackCreatorFromTrStoredTracksFactory = s_factory ;

// Standard constructor
RichDelegatedTrackCreatorFromTrStoredTracks::
RichDelegatedTrackCreatorFromTrStoredTracks( const std::string& type,
                                             const std::string& name,
                                             const IInterface* parent )
  : RichRecToolBase  ( type, name, parent ),
    m_tracks         ( 0     ),
    m_trTracks       ( 0     ),
    m_allDone        ( false )
{

  // declare interface for this tool
  declareInterface<IRichTrackCreator>(this);

  // job options
  declareProperty( "TrStoredTracksLocation",
                   m_trTracksLocation = TrStoredTrackLocation::Default );
  declareProperty( "RichRecTrackLocation",
                   m_richRecTrackLocation = RichRecTrackLocation::Default );
  // the real track tools to delegate the work to
  declareProperty( "ToolsByTrackType", m_names );

}

StatusCode RichDelegatedTrackCreatorFromTrStoredTracks::initialize()
{
  // Sets up various tools and services
  StatusCode sc = RichRecToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Setup incident services
  IIncidentSvc * incSvc = svc<IIncidentSvc>( "IncidentSvc", true );
  incSvc->addListener( this, IncidentType::BeginEvent );

  // setup mapping between track type and tool pointer
  m_nameToPnt.clear();
  m_trackToTool.clear();
  for ( ToolList::iterator it = m_names.begin();
        it != m_names.end(); ++it ) {
    std::string trackType = (*it);
    std::string toolType  = (*it);
    const int slash = (*it).find_first_of( "/" );
    if ( slash > 0 ) {
      trackType = (*it).substr( 0, slash );
      toolType  = (*it).substr( slash+1 );
    }
    debug() << " Tracktype '" << trackType
            << "' will use tool '" << toolType << "'" << endreq;
    m_trackToTool[trackType] = toolType;
  }
  m_names.clear();

  // Make sure we are ready for a new event
  InitNewEvent();

  return StatusCode::SUCCESS;
}

StatusCode RichDelegatedTrackCreatorFromTrStoredTracks::finalize()
{
  // Execute base class method
  return RichRecToolBase::finalize();
}

// Method that handles various Gaudi "software events"
void RichDelegatedTrackCreatorFromTrStoredTracks::handle ( const Incident & incident )
{
  if ( IncidentType::BeginEvent == incident.type() ) { InitNewEvent(); }
}

const StatusCode RichDelegatedTrackCreatorFromTrStoredTracks::newTracks() const {

  if ( ! m_allDone ) {
    m_allDone = true;

    // Load tracks
    if ( !m_trTracks ) { if ( !loadTrStoredTracks() ) return StatusCode::FAILURE; }

    // Iterate over all reco tracks, and create new RichRecTracks
    richTracks()->reserve( m_trTracks->size() );
    for ( TrStoredTracks::const_iterator track = m_trTracks->begin();
          track != m_trTracks->end();
          ++track) { newTrack( *track ); } // Make new RichRecTrack

  }

  return StatusCode::SUCCESS;
}

const long RichDelegatedTrackCreatorFromTrStoredTracks::nInputTracks() const
{
  if ( !m_trTracks ) { loadTrStoredTracks(); }
  return ( m_trTracks ? m_trTracks->size() : 0 );
}

const bool RichDelegatedTrackCreatorFromTrStoredTracks::loadTrStoredTracks() const
{

  // Obtain smart data pointer to TrStoredTracks
  m_trTracks = get<TrStoredTracks>( m_trTracksLocation );
  debug() << "located " << m_trTracks->size() << " TrStoredTracks at "
          << m_trTracksLocation << endreq;

  return StatusCode::SUCCESS;
}

// Forms a new RichRecTrack object from a TrStoredTrack
RichRecTrack *
RichDelegatedTrackCreatorFromTrStoredTracks::newTrack ( const ContainedObject * obj ) const
{

  // Is this a TrStoredTrack ?
  const TrStoredTrack * trTrack = dynamic_cast<const TrStoredTrack*>(obj);
  if ( !trTrack ) return NULL;

  // track type
  const Rich::Track::Type trType = Rich::Track::type(trTrack);
  if ( Rich::Track::Unknown == trType ) {
    Warning( "TrStoredTrack of unknown algorithm type");
    return NULL;
  }

  // Is track a usable type
  if ( !Rich::Track::isUsable(trType) ) return NULL;

  // See if this RichRecTrack already exists
  const unsigned long key = static_cast<unsigned long>(trTrack->key());
  if ( m_trackDone[key] ) {

    // track already done
    return richTracks()->object(key);

  } else {

    if ( msgLevel(MSG::VERBOSE) ) {
      verbose() << "TrStoredTrack " << trTrack->key()
                << " type " << Rich::Track::type(trTrack) 
                << " -> delegating to " << toolName(trTrack) << endreq;
    }
    
    // delegate work to appropriate tool
    return tkTool(trTrack)->newTrack( trTrack );
    
  }
  
  return 0;
}

RichRecTracks * RichDelegatedTrackCreatorFromTrStoredTracks::richTracks() const
{
  if ( !m_tracks ) {
    SmartDataPtr<RichRecTracks> tdsTracks( evtSvc(), m_richRecTrackLocation );
    if ( !tdsTracks ) {

      // Reinitialise the track Container
      m_tracks = new RichRecTracks();

      // Register new RichRecTrack container to Gaudi data store
      put( m_tracks, m_richRecTrackLocation );

    } else {

      debug() << "Found " << tdsTracks->size() << " pre-existing RichRecTracks in TES at "
              << m_richRecTrackLocation << endreq;

      // Set smartref to TES track container
      m_tracks = tdsTracks;

      // Remake local track reference map
      for ( RichRecTracks::const_iterator iTrack = tdsTracks->begin();
            iTrack != tdsTracks->end();
            ++iTrack ) {
        m_trackDone[(*iTrack)->key()] = true;
      }

    }
  }
  return m_tracks;
}
