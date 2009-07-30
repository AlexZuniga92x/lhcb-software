
//---------------------------------------------------------------------------------------------
/** @file RichDelegatedTrackCreatorFromRecoTracks.cpp
 *
 *  Implementation file for tool : Rich::Rec::DelegatedTrackCreatorFromRecoTracks
 *
 *  CVS Log :-
 *  $Id: RichDelegatedTrackCreatorFromRecoTracks.cpp,v 1.5 2009-07-30 11:25:33 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//---------------------------------------------------------------------------------------------

// local
#include "RichDelegatedTrackCreatorFromRecoTracks.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec;

//---------------------------------------------------------------------------------------------
DECLARE_TOOL_FACTORY( DelegatedTrackCreatorFromRecoTracks );

// Standard constructor
DelegatedTrackCreatorFromRecoTracks::
DelegatedTrackCreatorFromRecoTracks( const std::string& type,
                                     const std::string& name,
                                     const IInterface* parent )
  : TrackCreatorBase ( type, name, parent ),
    m_trTracks       ( NULL  ),
    m_allDone        ( false ),
    m_tkToPtn        ( Rich::Rec::Track::NTrTypes, 0 )
{

  // declare interface for this tool
  declareInterface<ITrackCreator>(this);

  // job options
  declareProperty( "RecoTracksLocation",
                   m_trTracksLocation = LHCb::TrackLocation::Default );
  // the real track tools to delegate the work to
  declareProperty( "ToolsByTrackType", m_names );

  // make hypo rings
  declareProperty( "BuildMassHypothesisRings", m_buildHypoRings = false );

}

StatusCode DelegatedTrackCreatorFromRecoTracks::initialize()
{
  // Sets up various tools and services
  StatusCode sc = TrackCreatorBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // setup mapping between track type and tool pointer
  Rich::Map< std::string, const ITrackCreator * > tmpMap;
  for ( ToolList::iterator it = m_names.begin();
        it != m_names.end(); ++it )
  {
    const int slash = (*it).find_first_of( "/" );
    const std::string trackType = ( slash>0 ? (*it).substr(0,slash) : *it );
    const std::string toolType  = ( slash>0 ? (*it).substr(slash+1) : *it );
    info() << "Track type '" << trackType
           << "' will use RichTrackCreator '" << toolType << "'" << endmsg;
    const Rich::Rec::Track::Type tkType = Rich::Rec::Track::type(trackType);
    if ( NULL == tmpMap[toolType] )
    {
      if ( !m_tkToPtn[tkType] )
      {
        const std::string fullToolName = name()+"."+toolType;
        sc = joSvc()->addPropertyToCatalogue( fullToolName,
                                              getProperty("BuildMassHypothesisRings") );
        if ( sc.isFailure() ) return Error( "Failed to propagate JobOptions to '"+fullToolName+"'", sc );
        acquireTool( toolType, m_tkToPtn[tkType], this );
      }
      tmpMap[toolType] = m_tkToPtn[tkType];
    }
    else
    {
      m_tkToPtn[tkType] = tmpMap[toolType];
    }

  }
  m_names.clear();

  return sc;
}

StatusCode DelegatedTrackCreatorFromRecoTracks::finalize()
{
  // Execute base class method
  return TrackCreatorBase::finalize();
}

StatusCode DelegatedTrackCreatorFromRecoTracks::newTracks() const
{

  if ( !m_allDone )
  {
    m_allDone = true;

    // Iterate over all reco tracks, and create new RichRecTracks
    richTracks()->reserve( recoTracks()->size() );
    for ( LHCb::Tracks::const_iterator track = recoTracks()->begin();
          track != recoTracks()->end();
          ++track) { newTrack( *track ); } // Make new RichRecTrack

  }

  return StatusCode::SUCCESS;
}

long DelegatedTrackCreatorFromRecoTracks::nInputTracks() const
{
  return ( recoTracks() ? recoTracks()->size() : 0 );
}

const LHCb::Tracks *
DelegatedTrackCreatorFromRecoTracks::recoTracks() const
{
  if ( !m_trTracks )
  {
    // Obtain smart data pointer to Tracks
    m_trTracks = get<LHCb::Tracks>( m_trTracksLocation );
    debug() << "located " << m_trTracks->size() << " Tracks at "
            << m_trTracksLocation << endmsg;
  }

  return m_trTracks;
}

// Forms a new RichRecTrack object from a Track
LHCb::RichRecTrack *
DelegatedTrackCreatorFromRecoTracks::newTrack ( const ContainedObject * obj ) const
{

  // Is this a Track ?
  const LHCb::Track * trTrack = dynamic_cast<const LHCb::Track*>(obj);
  if ( !trTrack ) return NULL;

  // track type
  const Rich::Rec::Track::Type trType = Rich::Rec::Track::type(trTrack);
  if ( Rich::Rec::Track::Unknown == trType )
  {
    Warning( "Track of unknown algorithm type" ).ignore();
    return NULL;
  }

  // Is track a usable type
  if ( !Rich::Rec::Track::isUsable(trType) ) return NULL;

  // See if this RichRecTrack already exists
  if ( bookKeep() && m_trackDone[trTrack->key()] )
  {

    // track already done
    return richTracks()->object(trTrack->key());

  }
  else
  {

    if ( msgLevel(MSG::VERBOSE) )
    {
      verbose() << "Track " << trTrack->key()
                << " type " << trType
                << " -> delegating to " << tkTool(trType)->name() << endmsg;
    }

    // Add to reference map
    if ( bookKeep() ) m_trackDone[trTrack->key()] = true;

    // delegate work to appropriate tool
    return tkTool(trType)->newTrack( trTrack );

  }

  return 0;
}

void DelegatedTrackCreatorFromRecoTracks::InitNewEvent()
{
  RichTrackCreatorBase::InitNewEvent();
  m_allDone  = false;
  m_trTracks = NULL;
}
