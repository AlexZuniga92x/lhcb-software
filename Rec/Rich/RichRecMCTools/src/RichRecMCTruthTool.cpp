
//-----------------------------------------------------------------------------
/** @file RichRecMCTruthTool.cpp
 *
 *  Implementation file for RICH reconstruction tool : RichRecMCTruthTool
 *
 *  CVS Log :-
 *  $Id: RichRecMCTruthTool.cpp,v 1.31 2008-03-25 16:46:21 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   08/07/2004
 */
//-----------------------------------------------------------------------------

#include "GaudiKernel/ToolFactory.h"

// local
#include "RichRecMCTruthTool.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec::MC;

//-----------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( MCTruthTool );

// Standard constructor
MCTruthTool::MCTruthTool( const std::string& type,
                          const std::string& name,
                          const IInterface* parent )
  : RichRecToolBase ( type, name, parent ),
    m_truth         ( NULL ),
    m_mcSegToRingLinks ( NULL ),
    m_trLoc         ( "/Event/"+LHCb::TrackLocation::Default ),
    m_otherTrLoc    ( "/Event/"+LHCb::TrackLocation::HltForward )
{
  // interface
  declareInterface<Rich::Rec::MC::IMCTruthTool>(this);

  // Context specific track locations
  if ( context() == "HLT" )
  {
    m_trLoc      = "/Event/"+LHCb::TrackLocation::HltForward;
    m_otherTrLoc = "/Event/"+LHCb::TrackLocation::Default;
  }

  // job options
  declareProperty( "TrackLocation", m_trLoc );
}

// destructor
MCTruthTool::~MCTruthTool() { cleanUpLinkers(); }

// initalization
StatusCode MCTruthTool::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = RichRecToolBase::initialize();
  if ( sc.isFailure() ) return sc;

  // Acquire instances of tools
  acquireTool( "RichMCTruthTool", m_truth );

  // Setup incident services
  incSvc()->addListener( this, IncidentType::BeginEvent );

  return sc;
}

// Method that handles various Gaudi "software events"
void MCTruthTool::handle ( const Incident& incident )
{
  if ( IncidentType::BeginEvent == incident.type() ) { InitNewEvent(); }
}

MCTruthTool::TrackToMCP *
MCTruthTool::trackToMCPLinks( const LHCb::Track * track ) const
{
  debug() << "trackToMCPLinks called for " << track << endreq;
  const std::string & loc = ( track ? objectLocation(track->parent()) : m_trLoc );
  MCTruthTool::TrackToMCP *& linker = m_trToMCPLinks[loc];
  if ( !linker )
  {
    trackToMCPLinks(loc);
    if ( !linker && loc != m_trLoc )
    {
      linker = trackToMCPLinks(m_trLoc);
    }
  }
  return linker;
}

MCTruthTool::TrackToMCP *
MCTruthTool::trackToMCPLinks( const std::string & loc ) const
{
  MCTruthTool::TrackToMCP *& linker = m_trToMCPLinks[loc];
  if ( !linker )
  {
    if ( msgLevel(MSG::DEBUG) )
      debug() << " -> Attempting to load TrackToMCP Linker for '" << loc << "'" << endreq;
    linker = new TrackToMCP( evtSvc(), loc );
    if ( !linker->direct() )
    {
      if ( msgLevel(MSG::DEBUG) )
        debug() << "  -> Linker for Tracks to MCParticles not found for '"
                << loc << "'" << endreq;
      // delete object and set to NULL, to force retrying next time this method is called.
      delete linker;
      linker = NULL;
    }
    else
    {
      debug() << "  -> Found " << linker->direct()->relations().size()
              << " Track to MCParticle associations" << endreq;
    }
  }
  return linker;
}

MCTruthTool::MCRichSegToMCCKRing *
MCTruthTool::mcSegToRingLinks() const
{
  if ( !m_mcSegToRingLinks )
  {
    const std::string & loc = LHCb::MCRichSegmentLocation::Default+"2MCCKRings";
    if ( msgLevel(MSG::DEBUG) )
      debug() << "Attempting to load MCRichSegToMCCKRing Linker for '"
              << loc << "'" << endreq;
    m_mcSegToRingLinks = new MCRichSegToMCCKRing( evtSvc(), loc );
    if ( !m_mcSegToRingLinks->direct() )
    {
      if ( msgLevel(MSG::DEBUG) )
        debug() << "Linker for MCRichTracks to MCCKRings not found for '"
                << loc << "'" << endreq;
      // delete object and set to NULL, to force retrying next time this method is called.
      delete m_mcSegToRingLinks;
      m_mcSegToRingLinks = NULL;
    }
    else
    {
      debug() << "Found " << m_mcSegToRingLinks->direct()->relations().size()
              << " Track to MCRichTrack->MCCKRings associations" << endreq;
    }
  }
  return m_mcSegToRingLinks;
}

const LHCb::MCParticle *
MCTruthTool::mcParticle( const LHCb::Track * track,
                         const double minWeight ) const
{
  // Try with linkers
  MCTruthTool::TrackToMCP * linker = trackToMCPLinks(track);
  if ( linker && linker->direct() )
  {
    TrToMCTable::Range range = linker->direct()->relations(track);
    if ( msgLevel(MSG::DEBUG) )
    {
      debug() << "Found " << range.size() << " association(s) for Track " << track->key()
              << " " << track->type() << " " << track->history() << endreq;
    }
    const LHCb::MCParticle * mcp = NULL;
    double bestWeight = -1;
    for ( TrToMCTable::Range::iterator iMC = range.begin(); iMC != range.end(); ++iMC )
    {
      if ( msgLevel(MSG::DEBUG) )
        debug() << " -> Found MCParticle " << iMC->to() << " weight=" << iMC->weight() << endreq;
      if ( iMC->weight() > minWeight && iMC->weight() > bestWeight )
      {
        bestWeight = iMC->weight();
        mcp = iMC->to();
      }
    }
    return mcp;
  }
  // If get here MC association failed
  if ( msgLevel(MSG::DEBUG) )
    debug() << "No MC association available for Tracks" << endreq;
  return NULL;
}

const LHCb::MCParticle *
MCTruthTool::mcParticle( const LHCb::RichRecTrack * richTrack,
                         const double minWeight ) const
{
  if ( !richTrack ) return NULL;
  const ContainedObject * obj = richTrack->parentTrack();
  if ( !obj )
  {
    if ( msgLevel(MSG::DEBUG) )
    {
      debug() << "RichRecTrack " << richTrack->key()
              <<  " has NULL pointer to parent" << endreq;
    }
    return NULL;
  }

  // What sort of track is this ...

  if ( const LHCb::Track * offTrack = dynamic_cast<const LHCb::Track*>(obj) )
  {
    // Track
    debug() << "RichRecTrack " << richTrack->key()
            << " has parent track Track " << offTrack->key() << endreq;
    return mcParticle( offTrack, minWeight );
  }
  else if ( const LHCb::MCParticle * mcPart = dynamic_cast<const LHCb::MCParticle*>(obj) )
  {
    // MCParticle
    debug() << "RichRecTrack " << richTrack->key()
            << " has parent track MCParticle " << mcPart->key() << endreq;
    return mcPart;
  }
  else
  {
    Warning( "Unknown RichRecTrack parent track type" );
    return NULL;
  }

}

void
MCTruthTool::mcRichHits( const LHCb::RichRecPixel * richPixel,
                         SmartRefVector<LHCb::MCRichHit> & hits ) const
{
  m_truth->mcRichHits( richPixel->hpdPixelCluster(), hits );
}

bool
MCTruthTool::mcParticle( const LHCb::RichRecPixel * richPixel,
                         std::vector<const LHCb::MCParticle*> & mcParts ) const
{
  return m_truth->mcParticles( richPixel->hpdPixelCluster(), mcParts );
}

const LHCb::MCParticle *
MCTruthTool::trueRecPhoton( const LHCb::RichRecPhoton * photon ) const
{
  return ( !photon ? NULL :
           trueRecPhoton( photon->richRecSegment(), photon->richRecPixel() ) );
}

const LHCb::MCParticle *
MCTruthTool::trueRecPhoton( const LHCb::RichRecSegment * segment,
                            const LHCb::RichRecPixel * pixel ) const
{
  // protect against bad input data
  if ( !segment || !pixel ) { return NULL; }

  const LHCb::RichRecTrack * track = segment->richRecTrack();
  const LHCb::MCParticle * mcTrack = ( track ? mcParticle(track) : NULL );
  if ( !mcTrack ) return NULL;

  return trueRecPhoton( mcTrack, pixel->hpdPixelCluster() );
}

const LHCb::MCParticle *
MCTruthTool::trueRecPhoton( const LHCb::MCParticle * mcPart,
                            const Rich::HPDPixelCluster& cluster ) const
{
  for ( LHCb::RichSmartID::Vector::const_iterator iS = cluster.smartIDs().begin();
        iS != cluster.smartIDs().end(); ++iS )
  {
    const LHCb::MCParticle * mcP = trueRecPhoton( mcPart, *iS );
    if ( NULL != mcP ) return mcP;
  }
  return NULL;
}

const LHCb::MCParticle *
MCTruthTool::trueRecPhoton( const LHCb::MCParticle * mcPart,
                            const LHCb::RichSmartID id ) const
{
  // Get MCParticles for the pixel
  std::vector<const LHCb::MCParticle *> mcParts;
  m_truth->mcParticles(id,mcParts);

  // Loop over all MCParticles associated to the pixel
  for ( std::vector<const LHCb::MCParticle *>::const_iterator iMCP = mcParts.begin();
        iMCP != mcParts.end(); ++iMCP )
  {
    if ( mcPart == (*iMCP) ) return mcPart;
  }

  return NULL;
}

const LHCb::MCParticle *
MCTruthTool::trueCherenkovPhoton( const LHCb::RichRecPhoton * photon ) const
{
  return ( !photon ? NULL :
           trueCherenkovPhoton( photon->richRecSegment(), photon->richRecPixel() ) );
}

const LHCb::MCParticle *
MCTruthTool::trueCherenkovPhoton( const LHCb::RichRecSegment * segment,
                                  const LHCb::RichRecPixel * pixel ) const
{
  if ( !segment || !pixel ) return NULL;
  if ( msgLevel(MSG::DEBUG) )
  {
    debug() << "Testing RichRecSegment " << segment->key()
            << " and RichRecPixel " << pixel->key() << endreq;
  }

  const LHCb::MCParticle * mcPart = trueRecPhoton( segment, pixel );
  return ( !mcPart ? NULL :
           trueCherenkovRadiation( pixel, segment->trackSegment().radiator() ) );
}

const LHCb::MCParticle *
MCTruthTool::trueCherenkovPhoton( const LHCb::MCParticle * mcPart,
                                  const LHCb::RichSmartID id,
                                  const Rich::RadiatorType rad ) const
{
  const LHCb::MCParticle * mcPart2 = trueRecPhoton( mcPart, id );
  return ( !mcPart2 ? NULL :
           trueCherenkovRadiation( id, rad ) );
}

const LHCb::MCParticle *
MCTruthTool::trueCherenkovRadiation( const LHCb::RichRecPixel * pixel,
                                     const Rich::RadiatorType rad ) const
{
  return trueCherenkovRadiation( pixel->hpdPixelCluster(), rad );
}

const LHCb::MCParticle *
MCTruthTool::trueCherenkovRadiation( const Rich::HPDPixelCluster& cluster,
                                     const Rich::RadiatorType rad ) const
{
  for ( LHCb::RichSmartID::Vector::const_iterator iS = cluster.smartIDs().begin();
        iS != cluster.smartIDs().end(); ++iS )
  {
    const  LHCb::MCParticle * mcP = trueCherenkovRadiation(*iS,rad);
    if ( NULL != mcP ) return mcP;
  }
  return NULL;
}

const LHCb::MCParticle *
MCTruthTool::trueCherenkovRadiation( const LHCb::RichSmartID id,
                                     const Rich::RadiatorType rad ) const
{
  // Test if hit is background
  if ( m_truth->isBackground(id) ) return NULL;

  // Test if hit is from correct radiator
  if ( !m_truth->isCherenkovRadiation(id,rad) ) return NULL;

  // All OK so find correct MCParticle
  std::vector<const LHCb::MCParticle *> mcParts;
  m_truth->mcParticles(id,mcParts);

  // Should do something better than just return first, but what ??
  return ( mcParts.empty() ? NULL : mcParts.front() );
}

Rich::ParticleIDType
MCTruthTool::mcParticleType( const LHCb::Track * track,
                             const double minWeight ) const
{
  return m_truth->mcParticleType( mcParticle(track,minWeight) );
}

Rich::ParticleIDType
MCTruthTool::mcParticleType( const LHCb::RichRecTrack * richTrack,
                             const double minWeight ) const
{
  return m_truth->mcParticleType( mcParticle(richTrack,minWeight) );
}

Rich::ParticleIDType
MCTruthTool::mcParticleType( const LHCb::RichRecSegment * richSegment,
                             const double minWeight ) const
{
  return ( richSegment ? 
           mcParticleType(richSegment->richRecTrack(),minWeight) : 
           Rich::Unknown );
}

const LHCb::MCParticle *
MCTruthTool::mcParticle( const LHCb::RichRecSegment * richSegment,
                         const double minWeight ) const
{
  return ( richSegment ? 
           mcParticle( richSegment->richRecTrack(), minWeight ) :
           NULL );
}

bool
MCTruthTool::mcRichOpticalPhoton( const LHCb::RichRecPixel * richPixel,
                                  SmartRefVector<LHCb::MCRichOpticalPhoton> & phots ) const
{
  phots.clear();
  SmartRefVector<LHCb::MCRichHit> hits;
  mcRichHits(richPixel,hits);
  for ( SmartRefVector<LHCb::MCRichHit>::const_iterator iHit = hits.begin();
        iHit != hits.end(); ++iHit )
  {
    // protect against bad hits
    if ( !(*iHit) ) continue;

    // Find MC photon
    const LHCb::MCRichOpticalPhoton * phot = m_truth->mcOpticalPhoton( *iHit );
    if ( phot ) phots.push_back( phot );
  }

  // Return boolean indicating if any photons were found
  return !phots.empty();
}

const LHCb::MCRichSegment *
MCTruthTool::mcRichSegment( const LHCb::RichRecSegment * segment ) const
{
  if ( !segment ) return NULL;
  const LHCb::MCRichTrack * mcTrack = mcRichTrack( segment );
  return ( mcTrack ?
           mcTrack->segmentInRad(segment->trackSegment().radiator()) : 0 );
}

const SmartRefVector<LHCb::MCRichSegment> *
MCTruthTool::mcRichSegments( const LHCb::RichRecTrack * track ) const
{
  const LHCb::MCRichTrack * mcTrack = mcRichTrack( track );
  return ( mcTrack ? &(mcTrack->mcSegments()) : 0 );
}

const LHCb::MCRichTrack *
MCTruthTool::mcRichTrack( const LHCb::RichRecSegment * segment ) const
{
  return ( segment ? mcRichTrack( segment->richRecTrack() ) : 0 );
}

const LHCb::MCRichTrack *
MCTruthTool::mcRichTrack( const LHCb::RichRecTrack * track ) const
{
  return m_truth->mcRichTrack( mcParticle(track) );
}

bool
MCTruthTool::isBackground( const LHCb::RichRecPixel * pixel ) const
{
  return m_truth->isBackground( pixel->hpdPixelCluster() );
}

// CRJ : Remove use of MCRichHits ??
const LHCb::MCRichHit *
MCTruthTool::trueCherenkovHit( const LHCb::RichRecPhoton * photon ) const
{
  // Track MCParticle
  const LHCb::MCParticle * trackMCP = mcParticle( photon->richRecSegment() );
  if ( trackMCP )
  {
    // Loop over all MCRichHits for the pixel associated to this photon
    SmartRefVector<LHCb::MCRichHit> hits;
    mcRichHits( photon->richRecPixel(), hits );
    for ( SmartRefVector<LHCb::MCRichHit>::const_iterator iHit = hits.begin();
          iHit != hits.end(); ++iHit )
    {
      if ( !(*iHit) ) continue;
      const LHCb::MCParticle * pixelMCP = (*iHit)->mcParticle();
      if ( pixelMCP == trackMCP ) return *iHit;
    }

  }

  // Not a true combination...
  return NULL;
}

const LHCb::MCRichOpticalPhoton *
MCTruthTool::trueOpticalPhoton( const LHCb::RichRecPhoton * photon ) const
{
  return ( !photon ? NULL :
           trueOpticalPhoton( photon->richRecSegment(), photon->richRecPixel() ) );
}

const LHCb::MCRichOpticalPhoton *
MCTruthTool::trueOpticalPhoton( const LHCb::RichRecSegment * segment,
                                const LHCb::RichRecPixel * pixel ) const
{
  // local container of photons
  std::vector<const LHCb::MCRichOpticalPhoton *> photons;

  // Is this a true cherenkov combination
  const LHCb::MCParticle * mcPart = trueCherenkovPhoton(segment,pixel);
  if ( mcPart )
  {
    if ( msgLevel(MSG::VERBOSE) )
      verbose() << "Finding MCRichOpticalPhotons for pixel " << pixel->hpdPixelCluster() << endreq;

    // Now find associated MCRichOpticalPhoton
    const SmartRefVector<LHCb::MCRichHit> & hits = m_truth->mcRichHits(mcPart);
    if ( msgLevel(MSG::VERBOSE) )
    { verbose() << "Found " << hits.size() << " MCRichHits" << endreq; }
    for ( SmartRefVector<LHCb::MCRichHit>::const_iterator iHit = hits.begin();
          iHit != hits.end(); ++iHit )
    {
      if ( !(*iHit) ) continue;
      if ( msgLevel(MSG::VERBOSE) )
      { verbose() << " -> Found mc hit in " << (*iHit)->radiator() << endreq; }
      // compare mc hit id to pixel id(s)
      for ( LHCb::RichSmartID::Vector::const_iterator iS = pixel->hpdPixelCluster().smartIDs().begin();
            iS != pixel->hpdPixelCluster().smartIDs().end(); ++iS )
      {
        const LHCb::RichSmartID hitid ( (*iS).pixelSubRowDataIsValid() ?
                                        (*iHit)->sensDetID() :
                                        (*iHit)->sensDetID().pixelID() );
        if ( msgLevel(MSG::VERBOSE) )
        { verbose() << "  -> " << (*iHit)->radiator() << " MCRichHit " << hitid << endreq; }
        if ( (*iS) == hitid )
        {
          // check radiator type
          if ( segment->trackSegment().radiator() == (*iHit)->radiator() )
          {
            if ( msgLevel(MSG::VERBOSE) )
            { verbose() << "   -> hit selected" << endreq; }
            photons.push_back( m_truth->mcOpticalPhoton(*iHit) );
          }
        } // same as hit ID
      } // loop over cluster smart IDs
    } // loop over hits
  }

  // did we find more than one photon ?
  // this is not handled well at the moment
  if ( photons.size() >= 2 )
  {
    Warning( "Found more than one associated MCRichOpticalPhoton. This is not handled well at present -> Returning first",
             StatusCode::SUCCESS, 1 );
  }

  return ( photons.empty() ? NULL : photons.front() );
}

const LHCb::RichRecRing *
MCTruthTool::mcCKRing( const LHCb::RichRecSegment * segment ) const
{
  const LHCb::MCRichSegment * mcSeg = mcRichSegment(segment);
  if ( !mcSeg ) return NULL;

  if ( mcSegToRingLinks() )
  {
    MCRichSegToMCCKRTable::Range range = mcSegToRingLinks()->direct()->relations(mcSeg);
    if ( msgLevel(MSG::DEBUG) )
    {
      debug() << "Found " << range.size() << " MCCKRing association(s) for MCRichTrack " << mcSeg->key()
              << endreq;
    }
    const LHCb::RichRecRing * ring(NULL);
    double bestWeight = -999999;
    for ( MCRichSegToMCCKRTable::Range::iterator iMC = range.begin(); iMC != range.end(); ++iMC )
    {
      if ( msgLevel(MSG::DEBUG) )
        debug() << " Found " << iMC->to() << " weight=" << iMC->weight() << endreq;
      if ( iMC->weight() > bestWeight )
      {
        bestWeight = iMC->weight();
        ring = iMC->to();
      }
    }
    return ring;
  }
  // If get here MC association failed
  if ( msgLevel(MSG::DEBUG) )
    debug() << "No MC association available for MCRichSegment -> MCCKRings" << endreq;
  return NULL;
}

void MCTruthTool::cleanUpLinkers()
{
  for ( TrackLocToMCPs::iterator iP = m_trToMCPLinks.begin();
        iP != m_trToMCPLinks.end(); ++iP ) { delete iP->second; }
  m_trToMCPLinks.clear();
  if ( m_mcSegToRingLinks ) { delete m_mcSegToRingLinks; m_mcSegToRingLinks = NULL; }
}

bool MCTruthTool::trackToMCPAvailable() const
{
  TrackToMCP * linker = trackToMCPLinks(m_trLoc);
  if ( linker && linker->direct() ) return true;
  linker = trackToMCPLinks(m_otherTrLoc);
  return ( linker && linker->direct() );
}

bool MCTruthTool::pixelMCHistoryAvailable() const
{
  return m_truth->richMCHistoryAvailable();
}

bool MCTruthTool::photonMCAssocAvailable() const
{
  return ( trackToMCPAvailable() && pixelMCHistoryAvailable() );
}

bool MCTruthTool::extendedMCAvailable() const
{
  return m_truth->extendedMCAvailable();
}
