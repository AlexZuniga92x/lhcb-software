
//---------------------------------------------------------------------------------
/** @file RichTrackCreatorBase.cpp
 *
 *  Implementation file for tool base class : RichTrackCreatorBase
 *
 *  CVS Log :-
 *  $Id: RichTrackCreatorBase.cpp,v 1.2 2005-06-23 15:13:05 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   20/05/2005
 */
//---------------------------------------------------------------------------------

// local
#include "RichRecBase/RichTrackCreatorBase.h"

//-----------------------------------------------------------------------------

// Standard constructor, initializes variables
RichTrackCreatorBase::RichTrackCreatorBase( const std::string& type,
                                            const std::string& name,
                                            const IInterface* parent )
  : RichRecToolBase        ( type, name, parent ),
    m_hasBeenCalled        ( false              ),
    m_tracks               ( 0                  ),
    m_richRecTrackLocation ( RichRecTrackLocation::Default  ),
    m_Nevts                ( 0                  ),
    m_bookKeep             ( false              )
{

  // Define the interface
  declareInterface<IRichTrackCreator>(this);

  // job options
  declareProperty( "DoBookKeeping",        m_bookKeep                        );
  declareProperty( "DoBookKeeping",        m_bookKeep                        );
  declareProperty( "TrackMomentumCuts",    m_trSelector.setMomentumCuts()    );
  declareProperty( "TrackSelection",       m_trSelector.selectedTrackTypes() );

}

StatusCode RichTrackCreatorBase::initialize()
{
  // base class initilize
  const StatusCode sc = RichRecToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Configure output location depending on processing stage
  // to be replace by common "context" when available
  if      ( processingStage() == "Offline" )
  {
    m_richRecTrackLocation = RichRecTrackLocation::Offline;
  }
  else if ( processingStage() == "HLT" )
  {
    m_richRecTrackLocation = RichRecTrackLocation::HLT;
  }
  if ( msgLevel(MSG::DEBUG) )
  {
    debug() << "RichRecTrack location : " << m_richRecTrackLocation << endreq;
  }

  // Configure track selector
  if ( !m_trSelector.configureTrackTypes() ) return StatusCode::FAILURE;
  m_trSelector.printTrackSelection( info() );

  // Setup incident services
  incSvc()->addListener( this, IncidentType::BeginEvent );
  incSvc()->addListener( this, IncidentType::EndEvent   );

  return sc;
}

StatusCode RichTrackCreatorBase::finalize()
{
  // print stats
  printStats();

  // base class finalize
  return RichRecToolBase::finalize();
}

// Method that handles various Gaudi "software events"
void RichTrackCreatorBase::handle ( const Incident& incident )
{
  // Update prior to start of event. Used to re-initialise data containers
  if ( IncidentType::BeginEvent == incident.type() )
  {
    InitNewEvent();
  }
  // Debug printout at the end of each event
  else if ( IncidentType::EndEvent == incident.type() )
  {
    FinishEvent();
  }
}

void RichTrackCreatorBase::printStats() const
{

  if ( m_Nevts > 0 && !m_nTracksAll.empty() )
  {

    // Statistical tools
    RichPoissonEffFunctor eff("%6.2f +-%5.2f");
    RichStatDivFunctor occ("%8.2f +-%5.2f");

    // Print out final track stats
    info() << "================================================================================" << endreq
           << "                    Track Selection Summary : " << m_Nevts << " events" << endreq
           << "--------------------------------------------------------------------------------" << endreq;
    for ( TrackTypeCount::iterator i = m_nTracksAll.begin();
          i != m_nTracksAll.end(); ++i )
    {
      std::string name =
        ( (*i).first.second ? "Unique " : "NonUnique " ) + Rich::text( (*i).first.first );
      name.resize(17,' ');
      info() << "  " << name << " :" << occ((*i).second.selectedTracks,m_Nevts)
             << " tracks/event : RICH eff " << eff((*i).second.selectedTracks,(*i).second.triedTracks)
             << " % " << endreq;
      if ( (*i).second.aeroSegs>0 )
        info() << "                    :"
               << occ((*i).second.aeroSegs,m_Nevts)  << " Aerogel  segments/event" << endreq;
      if ( (*i).second.c4f10Segs>0 )
        info() << "                    :"
               << occ((*i).second.c4f10Segs,m_Nevts) << " C4F10    segments/event" << endreq;
      if ( (*i).second.cf4Segs>0 )
        info() << "                    :"
               << occ((*i).second.cf4Segs,m_Nevts)   << " CF4      segments/event" << endreq;
    }
    info() << "================================================================================" << endreq;

  }

}

void RichTrackCreatorBase::InitNewEvent()
{
  m_hasBeenCalled = false;
  if ( bookKeep() ) m_trackDone.clear();
  m_tracks   = 0;
  if ( msgLevel(MSG::DEBUG) )
  {
    m_nTracksLast = m_nTracksAll;
  }
}

void RichTrackCreatorBase::FinishEvent()
{
  if ( m_hasBeenCalled ) ++m_Nevts;
  if ( msgLevel(MSG::DEBUG) )
  {
    // Print out of track count for this event
    unsigned int nTotTried(0), nTotSel(0);
    for ( TrackTypeCount::iterator i = m_nTracksAll.begin(); i != m_nTracksAll.end(); ++i )
    {
      nTotTried += (*i).second.triedTracks - m_nTracksLast[(*i).first].triedTracks;
      nTotSel   += (*i).second.selectedTracks - m_nTracksLast[(*i).first].selectedTracks;
    }
    debug() << "Selected " << nTotSel << "/" << nTotTried << " TrStoredTracks :";
    for ( TrackTypeCount::iterator i = m_nTracksAll.begin(); i != m_nTracksAll.end(); ++i )
    {
      const std::string name =
        ( (*i).first.second ? "Unique:" : "NonUnique:" ) + Rich::text( (*i).first.first );
      debug() << " " << name << "=("
              << (*i).second.selectedTracks - m_nTracksLast[(*i).first].selectedTracks << "/"
              << (*i).second.triedTracks - m_nTracksLast[(*i).first].triedTracks << ")";
    }
    debug() << endreq;
  }
}

RichRecTracks * RichTrackCreatorBase::richTracks() const
{
  if ( !m_tracks ) {

    if ( !exist<RichRecTracks>(m_richRecTrackLocation) )
    {

      // Reinitialise the track Container
      m_tracks = new RichRecTracks();

      // Register new RichRecTrack container to Gaudi data store
      put( m_tracks, m_richRecTrackLocation );

    }
    else
    {

      // get tracks from TES
      m_tracks = get<RichRecTracks>(m_richRecTrackLocation);
      if ( msgLevel(MSG::DEBUG) )
      {
        debug() << "Found " << m_tracks->size() << " pre-existing RichRecTracks in TES at "
                << m_richRecTrackLocation << endreq;
      }

      if ( m_bookKeep )
      {
        // Remake local track reference map
        for ( RichRecTracks::const_iterator iTrack = m_tracks->begin();
              iTrack != m_tracks->end();
              ++iTrack )
        {
          m_trackDone[(*iTrack)->key()] = true;
        }
      }

    }

  }

  return m_tracks;
}
