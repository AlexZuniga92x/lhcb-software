// $Id: RichPixelCreatorFromRichDigits.cpp,v 1.8 2004-04-19 23:06:14 jonesc Exp $

// local
#include "RichPixelCreatorFromRichDigits.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichPixelCreatorFromRichDigits
//
// 15/03/2002 : Chris Jones   Christopher.Rob.Jones@cern.ch
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
static const  ToolFactory<RichPixelCreatorFromRichDigits>          s_factory ;
const        IToolFactory& RichPixelCreatorFromRichDigitsFactory = s_factory ;

// Standard constructor
RichPixelCreatorFromRichDigits::RichPixelCreatorFromRichDigits( const std::string& type,
                                                                const std::string& name,
                                                                const IInterface* parent )
  : RichRecToolBase( type, name, parent ),
    m_pixels      ( 0 ),
    m_smartIDTool ( 0 ),
    m_allDone     ( false )
{

  declareInterface<IRichPixelCreator>(this);

  // Define job option parameters
  declareProperty( "RichRecPixelLocation",
                   m_richRecPixelLocation = RichRecPixelLocation::Default );
  declareProperty( "RecoDigitsLocation",
                   m_recoDigitsLocation = RichDigitLocation::Default );

}

StatusCode RichPixelCreatorFromRichDigits::initialize() {

  // Sets up various tools and services
  StatusCode sc = RichRecToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Acquire instances of tools
  acquireTool( "RichSmartIDTool", m_smartIDTool );

  // Setup incident services
  IIncidentSvc * incSvc = svc<IIncidentSvc>( "IncidentSvc", true );
  incSvc->addListener( this, IncidentType::BeginEvent );

  // Make sure we are ready for a new event
  InitNewEvent();

  return StatusCode::SUCCESS;
}

StatusCode RichPixelCreatorFromRichDigits::finalize() 
{
  // Execute base class method
  return RichRecToolBase::finalize();
}

// Method that handles various Gaudi "software events"
void RichPixelCreatorFromRichDigits::handle ( const Incident& incident )
{
  if ( IncidentType::BeginEvent == incident.type() ) InitNewEvent();
}

// Forms a new RichRecPixel object from a RichDigit
RichRecPixel *
RichPixelCreatorFromRichDigits::newPixel( const ContainedObject * obj ) const {

  // Try to cast to RichDigit
  const RichDigit * digit = dynamic_cast<const RichDigit*>(obj);
  if ( !digit ) {
    Warning("Parent not of type RichDigit");
    return NULL;
  }

  // RichDigit key
  const RichSmartID id = digit->key();

  // See if this RichRecPixel already exists
  if ( m_pixelDone[id] ) {
    return m_pixelExists[id];
  } else {

    RichRecPixel * newPixel = NULL;

    if ( id.isValid() ) {

      // Make a new RichRecPixel
      newPixel = new RichRecPixel();
      richPixels()->insert( newPixel );

      // Positions
      HepPoint3D & gPosition = newPixel->globalPosition();
      m_smartIDTool->globalPosition( id, gPosition );
      newPixel->localPosition() = m_smartIDTool->globalToPDPanel(gPosition);

      // Set smartID
      newPixel->setSmartID( id );

      // Set parent information
      newPixel->setParentPixel( digit );
      newPixel->setParentType( Rich::RecPixel::Digit );
    }

    // Add to reference map
    m_pixelExists[ id ] = newPixel;
    m_pixelDone  [ id ] = true;

    return newPixel;
  }

}

StatusCode RichPixelCreatorFromRichDigits::newPixels() const {

  if ( m_allDone ) return StatusCode::SUCCESS;
  m_allDone = true;

  // Obtain smart data pointer to RichDigits
  RichDigits * digits = get<RichDigits>( m_recoDigitsLocation );

  // Loop over RichDigits and create working pixels
  richPixels()->reserve( digits->size() );
  for ( RichDigits::iterator digit = digits->begin();
        digit != digits->end(); ++digit ) { newPixel( *digit ); }

  if ( msgLevel(MSG::DEBUG) ) {
    debug() << "Located " << digits->size() << " RichDigits at "
            << m_recoDigitsLocation << endreq
            << "Created " << richPixels()->size() << " RichRecPixels at "
            << m_richRecPixelLocation << endreq;
  }

  return StatusCode::SUCCESS;
}

RichRecPixels * RichPixelCreatorFromRichDigits::richPixels() const
{
  if ( !m_pixels ) {

    SmartDataPtr<RichRecPixels> tdsPixels( evtSvc(),
                                           m_richRecPixelLocation );
    if ( !tdsPixels ) {

      // Reinitialise the Pixel Container
      m_pixels = new RichRecPixels();

      // Register new RichRecPhoton container to Gaudi data store
      put( m_pixels, m_richRecPixelLocation );

    } else {

      // Set smartref to TES pixel container
      m_pixels = tdsPixels;

      // Remake local pixel reference map
      for ( RichRecPixels::const_iterator iPixel = tdsPixels->begin();
            iPixel != tdsPixels->end(); ++iPixel ) {
        m_pixelExists [ (*iPixel)->smartID() ] = *iPixel;
        m_pixelDone   [ (*iPixel)->smartID() ] = true;
      }

    }
  }

  return m_pixels;
}
