// $Id: RichMassHypothesisRingCreator.cpp,v 1.2 2004-05-31 22:02:06 jonrob Exp $

// local
#include "RichMassHypothesisRingCreator.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RichMassHypothesisRingCreator
//
// 15/03/2002 : Chris Jones   Christopher.Rob.Jones@cern.ch
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
static const  ToolFactory<RichMassHypothesisRingCreator>          s_factory ;
const        IToolFactory& RichMassHypothesisRingCreatorFactory = s_factory ;

// Standard constructor
RichMassHypothesisRingCreator::RichMassHypothesisRingCreator( const std::string& type,
                                                              const std::string& name,
                                                              const IInterface* parent )
  : RichRecToolBase( type, name, parent ),
    m_rayTrace     ( 0 ),
    m_ckAngle      ( 0 ),
    m_rings        ( 0 )
{

  declareInterface<IRichMassHypothesisRingCreator>(this);

  // Define job option parameters
  declareProperty( "RingsLocation",
                   m_ringLocation = RichRecRingLocation::SegmentHypoRings );

}

StatusCode RichMassHypothesisRingCreator::initialize() {

  // Sets up various tools and services
  StatusCode sc = RichRecToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Acquire instances of tools
  acquireTool( "RichCherenkovAngle", m_ckAngle  );
  acquireTool( "RichRayTraceCKCone", m_rayTrace );

  // Setup incident services
  IIncidentSvc * incSvc = svc<IIncidentSvc>( "IncidentSvc", true );
  incSvc->addListener( this, IncidentType::BeginEvent );

  // Make sure we are ready for a new event
  InitNewEvent();

  return StatusCode::SUCCESS;
}

StatusCode RichMassHypothesisRingCreator::finalize()
{
  // Execute base class method
  return RichRecToolBase::finalize();
}

// Method that handles various Gaudi "software events"
void RichMassHypothesisRingCreator::handle ( const Incident& incident )
{
  if ( IncidentType::BeginEvent == incident.type() ) InitNewEvent();
}

void RichMassHypothesisRingCreator::newMassHypoRings( RichRecSegment * segment ) const
{
  for ( int iHypo = 0; iHypo < Rich::NParticleTypes; ++iHypo ) {
    newMassHypoRing( segment, static_cast<const Rich::ParticleIDType>(iHypo) );
  }
}

// Forms a new RichRecRing object from a RichDigit
RichRecRing *
RichMassHypothesisRingCreator::newMassHypoRing( RichRecSegment * segment,
                                                const Rich::ParticleIDType id ) const
{
  if ( !segment ) return 0;

  // does the ring already exist ?
  return ( segment->hypothesisRings().dataIsValid(id) ?
           segment->hypothesisRings()[id] : buildRing(segment, id) );
}

RichRecRing * RichMassHypothesisRingCreator::buildRing( RichRecSegment * segment,
                                                        const Rich::ParticleIDType id ) const
{

  RichRecRing * newRing = 0;

  // Cherenkov theta for this segment/hypothesis combination
  const double ckTheta = m_ckAngle->avgCherenkovTheta( segment, id );
  if ( ckTheta > 0 ) {

    if ( msgLevel(MSG::VERBOSE) ) {
      verbose() << "Creating " << id 
                << " hypothesis ring for RichRecSegment " << segment->key() 
                << endreq;
    }

    // Make a new ring and give to container
    newRing = new RichRecRing();
    massHypoRings()->insert( newRing );

    // set the segment information
    newRing->setRichRecSegment( segment );

    // set centre point information
    newRing->setCentrePointGlobal( segment->pdPanelHitPoint()      );
    newRing->setCentrePointLocal ( segment->pdPanelHitPointLocal() );

    // Set cherenkov information
    newRing->setRadius( ckTheta );

    // set the PID for this segment
    newRing->setMassHypo( id );

    // detector inforamtion
    newRing->setRich( segment->trackSegment().rich() );

  }

  // set data in segment
  segment->hypothesisRings().setData( id, newRing );

  // return final pointer
  return newRing;
}

RichRecRings * RichMassHypothesisRingCreator::massHypoRings() const
{
  if ( !m_rings ) {

    SmartDataPtr<RichRecRings> tdsRings( evtSvc(), m_ringLocation );
    if ( !tdsRings ) {

      // Reinitialise the Ring Container
      m_rings = new RichRecRings();

      // Register new RichRecRing container to Gaudi data store
      put( m_rings, m_ringLocation );

    } else {

      debug() << "Found " << tdsRings->size() << " pre-existing RichRecRings in TES at "
              << m_ringLocation << endreq;

      // Set smartref to TES Ring container
      m_rings = tdsRings;

    }
  }

  return m_rings;
}
