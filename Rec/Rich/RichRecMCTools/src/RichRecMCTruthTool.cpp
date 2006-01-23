
//-----------------------------------------------------------------------------
/** @file RichRecMCTruthTool.cpp
 *
 *  Implementation file for RICH reconstruction tool : RichRecMCTruthTool
 *
 *  CVS Log :-
 *  $Id: RichRecMCTruthTool.cpp,v 1.18 2006-01-23 14:09:59 jonrob Exp $
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   08/07/2004
 */
//-----------------------------------------------------------------------------

// local
#include "RichRecMCTruthTool.h"

// namespaces
using namespace LHCb;

//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
static const  ToolFactory<RichRecMCTruthTool>          s_factory ;
const        IToolFactory& RichRecMCTruthToolFactory = s_factory ;

// Standard constructor
RichRecMCTruthTool::RichRecMCTruthTool( const std::string& type,
                                        const std::string& name,
                                        const IInterface* parent )
  : RichRecToolBase ( type, name, parent ),
    m_truth         ( 0 ),
    m_trToMCPLinks  ( 0 )
{
  // interface
  declareInterface<IRichRecMCTruthTool>(this);
}

StatusCode RichRecMCTruthTool::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = RichRecToolBase::initialize();
  if ( sc.isFailure() ) return sc;

  // Acquire instances of tools
  acquireTool( "RichMCTruthTool", m_truth );

  // Setup incident services
  incSvc()->addListener( this, IncidentType::BeginEvent );

  // initialise
  m_emptyContainer.clear();

  return sc;
}

StatusCode RichRecMCTruthTool::finalize()
{
  // Execute base class method
  return RichRecToolBase::finalize();
}

// Method that handles various Gaudi "software events"
void RichRecMCTruthTool::handle ( const Incident& incident )
{
  if ( IncidentType::BeginEvent == incident.type() ) { InitNewEvent(); }
}

RichRecMCTruthTool::TrackToMCP *
RichRecMCTruthTool::trackToMCPLinks() const
{
  if ( !m_trToMCPLinks )
  {
    m_trToMCPLinks =
      new TrackToMCP( evtSvc(), msgSvc(), TrackLocation::Default );
    if ( m_trToMCPLinks->notFound() )
    {
      Warning( "Linker for Tracks to MCParticles not found for '" +
               TrackLocation::Default + "'" );
    }
  }
  return m_trToMCPLinks;
}

const MCParticle *
RichRecMCTruthTool::mcParticle( const Track * track ) const
{
  // Try with linkers
  if ( trackToMCPLinks() && !trackToMCPLinks()->notFound() )
  {
    return trackToMCPLinks()->first(track->key());
  }
  // If get here MC association failed
  Warning( "No MC association available for Tracks" );
  return NULL;
}

const MCParticle *
RichRecMCTruthTool::mcParticle( const RichRecTrack * richTrack ) const
{
  const ContainedObject * obj = richTrack->parentTrack();
  if ( !obj )
  {
    Warning ( "RichRecTrack has NULL pointer to parent" );
    return NULL;
  }

  // What sort of track is this ...

  if ( const Track * offTrack = dynamic_cast<const Track*>(obj) )
  {
    // Track
    debug() << "RichRecTrack " << richTrack->key()
            << " has parent track Track " << offTrack->key() << endreq;
    return mcParticle( offTrack );
  }
  else if ( const MCParticle * mcPart = dynamic_cast<const MCParticle*>(obj) )
  {
    // MCParticle
    debug() << "RichRecTrack " << richTrack->key()
            << " has parent track MCParticle " << mcPart->key() << endreq;
    return mcPart;
  }
  else
  {
    Warning ( "Unknown RichRecTrack parent track type" );
    return NULL;
  }

}

const SmartRefVector<MCRichHit> &
RichRecMCTruthTool::mcRichHits( const RichRecPixel * richPixel ) const
{
  const MCRichDigit * mcDigit = mcRichDigit(richPixel);
  return ( mcDigit ? mcDigit->hits() : m_emptyContainer );
}

const MCRichDigit *
RichRecMCTruthTool::mcRichDigit( const RichRecPixel * richPixel ) const
{
  // protect against bad pixels
  if ( !richPixel )
  {
    Warning ( "::mcRichDigit : NULL RichRecPixel pointer" );
    return NULL;
  }

  const MCRichDigit * mcDigit = 0;
  if ( Rich::PixelParent::RawBuffer == richPixel->parentType() )
  {

    // use RichSmartID to locate MC information
    mcDigit = m_truth->mcRichDigit( richPixel->smartID() );

  }
  else if ( Rich::PixelParent::Digit == richPixel->parentType() )
  {

    // try to get parent RichDigit
    const RichDigit * digit = dynamic_cast<const RichDigit*>( richPixel->parentPixel() );
    if ( !digit ) { Warning ( "RichRecPixel has no associated RichDigit" ); }

    // All OK, so find and return MCRichDigit for this RichDigit
    mcDigit = m_truth->mcRichDigit( digit->richSmartID() );

  }
  else if ( Rich::PixelParent::NoParent == richPixel->parentType() )
  {

    // Pixel has no parent, so MC association cannot be done
    Warning( "Parentless RichRecPixel -> MC association impossible",StatusCode::SUCCESS );
    return NULL;

  }
  else
  {

    // unknown Pixel type
    Warning( "Do not know how to access MC for RichRecPixel type " +
             Rich::text(richPixel->parentType()) );

  }

  if ( !mcDigit ) Warning( "Failed to find MCRichDigit for RichRecPixel" );
  return mcDigit;
}

bool
RichRecMCTruthTool::mcParticle( const RichRecPixel * richPixel,
                                std::vector<const MCParticle*> & mcParts ) const
{
  return m_truth->mcParticles( richPixel->smartID(), mcParts );
}

const MCParticle *
RichRecMCTruthTool::trueRecPhoton( const RichRecPhoton * photon ) const
{
  return ( !photon ? NULL :
           trueRecPhoton( photon->richRecSegment(), photon->richRecPixel() ) );
}

const MCParticle * RichRecMCTruthTool::trueRecPhoton( const RichRecSegment * segment,
                                                      const RichRecPixel * pixel ) const
{
  // protect against bad input data
  if (!segment) { Warning("::trueRecPhoton : NULL RichRecSegment"); return 0; }
  if (!pixel)   { Warning("::trueRecPhoton : NULL RichRecPixel");   return 0; }

  const RichRecTrack * track = segment->richRecTrack();
  const MCParticle * mcTrack = ( track ? mcParticle(track) : NULL );
  if ( !mcTrack ) return NULL;

  // Get MCParticles for the pixel
  std::vector<const MCParticle *> mcParts;
  m_truth->mcParticles(pixel->smartID(),mcParts);

  // Loop over all MCParticles associated to the pixel
  for ( std::vector<const MCParticle *>::const_iterator iMCP = mcParts.begin();
        iMCP != mcParts.end(); ++iMCP )
  {
    if ( mcTrack == (*iMCP) ) return mcTrack;
  }

  return NULL;
}

const MCParticle *
RichRecMCTruthTool::trueCherenkovPhoton( const RichRecPhoton * photon ) const
{
  return ( !photon ? NULL :
           trueCherenkovPhoton( photon->richRecSegment(), photon->richRecPixel() ) );
}

const MCParticle *
RichRecMCTruthTool::trueCherenkovPhoton( const RichRecSegment * segment,
                                         const RichRecPixel * pixel ) const
{
  if ( !segment || !pixel ) return NULL;
  if ( msgLevel(MSG::DEBUG) )
  {
    debug() << "Testing RichRecSegment " << segment->key()
            << " and RichRecPixel " << pixel->key() << endreq;
  }
  const MCParticle * mcPart = trueRecPhoton( segment, pixel );
  return ( !mcPart ? NULL :
           trueCherenkovRadiation( pixel, segment->trackSegment().radiator() ) );
}

const MCParticle *
RichRecMCTruthTool::trueCherenkovRadiation( const RichRecPixel * pixel,
                                            const Rich::RadiatorType rad ) const
{
  // Test if hit is background
  if ( m_truth->isBackground(pixel->smartID()) ) return NULL;

  // Test if hit is from correct radiator
  if ( !m_truth->isCherenkovRadiation(pixel->smartID(),rad) ) return NULL;

  // All OK so find correct MCParticle
  std::vector<const MCParticle *> mcParts;
  m_truth->mcParticles(pixel->smartID(),mcParts);
  return ( mcParts.empty() ? NULL : mcParts.front() );

  /*
  // Loop over all MCRichHits for this pixel
  const SmartRefVector<MCRichHit> & hits = mcRichHits( pixel );
  for ( SmartRefVector<MCRichHit>::const_iterator iHit = hits.begin();
  iHit != hits.end(); ++iHit ) {
  if ( *iHit &&
  rad == (*iHit)->radiator() &&
  !m_truth->isBackground(*iHit) ) return (*iHit)->mcParticle();
  }
  return NULL;
  */
}

Rich::ParticleIDType
RichRecMCTruthTool::mcParticleType( const LHCb::Track * track ) const
{
  return m_truth->mcParticleType( mcParticle(track) );
}

Rich::ParticleIDType
RichRecMCTruthTool::mcParticleType( const RichRecTrack * richTrack ) const
{
  return m_truth->mcParticleType( mcParticle(richTrack) );
}

Rich::ParticleIDType
RichRecMCTruthTool::mcParticleType( const RichRecSegment * richSegment ) const
{
  return ( richSegment ? mcParticleType(richSegment->richRecTrack()) : Rich::Unknown );
}

const MCParticle *
RichRecMCTruthTool::mcParticle( const RichRecSegment * richSegment ) const
{
  return ( richSegment ? mcParticle( richSegment->richRecTrack() ) : NULL );
}

bool
RichRecMCTruthTool::mcRichOpticalPhoton( const RichRecPixel * richPixel,
                                         SmartRefVector<MCRichOpticalPhoton> & phots ) const
{
  phots.clear();
  const SmartRefVector<MCRichHit> & hits = mcRichHits(richPixel);
  for ( SmartRefVector<MCRichHit>::const_iterator iHit = hits.begin();
        iHit != hits.end(); ++iHit )
  {
    // protect against bad hits
    if ( !(*iHit) ) continue;

    // Find MC photon
    const MCRichOpticalPhoton * phot = m_truth->mcOpticalPhoton( *iHit );
    if ( phot ) phots.push_back( phot );
  }

  // Return boolean indicating if any photons were found
  return !phots.empty();
}

const MCRichSegment *
RichRecMCTruthTool::mcRichSegment( const RichRecSegment * segment ) const
{
  if ( !segment ) return NULL;
  const MCRichTrack * mcTrack = mcRichTrack( segment );
  return ( mcTrack ?
           mcTrack->segmentInRad(segment->trackSegment().radiator()) : 0 );
}

const SmartRefVector<MCRichSegment> *
RichRecMCTruthTool::mcRichSegments( const RichRecTrack * track ) const
{
  const MCRichTrack * mcTrack = mcRichTrack( track );
  return ( mcTrack ? &(mcTrack->mcSegments()) : 0 );
}

const MCRichTrack *
RichRecMCTruthTool::mcRichTrack( const RichRecSegment * segment ) const
{
  return ( segment ? mcRichTrack( segment->richRecTrack() ) : 0 );
}

const MCRichTrack *
RichRecMCTruthTool::mcRichTrack( const RichRecTrack * track ) const
{
  return m_truth->mcRichTrack( mcParticle(track) );
}

bool RichRecMCTruthTool::isBackground( const RichRecPixel * pixel ) const
{
  return m_truth->isBackground( pixel->smartID() );
}

const MCRichHit *
RichRecMCTruthTool::trueCherenkovHit( const RichRecPhoton * photon ) const
{
  // Track MCParticle
  const MCParticle * trackMCP = mcParticle( photon->richRecSegment() );
  if ( trackMCP )
  {
    // Loop over all MCRichHits for the pixel associated to this photon
    const SmartRefVector<MCRichHit> & hits = mcRichHits( photon->richRecPixel() );
    for ( SmartRefVector<MCRichHit>::const_iterator iHit = hits.begin();
          iHit != hits.end(); ++iHit )
    {
      if ( !(*iHit) ) continue;
      const MCParticle * pixelMCP = (*iHit)->mcParticle();
      if ( pixelMCP == trackMCP ) return *iHit;
    }

  }

  // Not a true combination...
  return NULL;
}

const MCRichOpticalPhoton *
RichRecMCTruthTool::trueOpticalPhoton( const RichRecPhoton * photon ) const
{
  // get true MCRichHit
  const MCRichHit * mchit = trueCherenkovHit( photon );
  if ( !mchit ) return NULL;

  // return associated MCRichOpticalPhoton
  return m_truth->mcOpticalPhoton( mchit );
}

const MCRichOpticalPhoton *
RichRecMCTruthTool::trueOpticalPhoton( const RichRecSegment * segment,
                                       const RichRecPixel * pixel ) const
{
  // Is this a true cherenkov combination
  const MCParticle * mcPart = trueCherenkovPhoton(segment,pixel);
  if ( mcPart )
  {
    // Now find associated MCRichOpticalPhoton
    const SmartRefVector<MCRichHit> & hits = mcRichHits(pixel);
    for ( SmartRefVector<MCRichHit>::const_iterator iHit = hits.begin();
          iHit != hits.end(); ++iHit )
    {
      if ( *iHit &&
           (*iHit)->mcParticle() == mcPart ) return m_truth->mcOpticalPhoton(*iHit);
    }
  }

  // Not a true combination...
  return NULL;
}

