
//-----------------------------------------------------------------------------
/** @file RichMCTruePhotonEmissionPoint.cpp
 *
 *  Implementation file for tool : Rich::Rec::MC::TruePhotonEmissionPoint
 *
 *  @author Chris Jones   Christopher.Rob.Jones@cern.ch
 *  @date   15/03/2002
 */
//-----------------------------------------------------------------------------

// local
#include "RichMCTruePhotonEmissionPoint.h"

// All code is in general Rich reconstruction namespace
using namespace Rich::Rec::MC;

//-----------------------------------------------------------------------------

// Standard constructor
TruePhotonEmissionPoint::TruePhotonEmissionPoint ( const std::string& type,
                                                   const std::string& name,
                                                   const IInterface* parent )
  : Rich::Rec::ToolBase ( type, name, parent )
{
  // tool interface
  declareInterface<IPhotonEmissionPoint>(this);
  // JOs
  declareProperty( "UseAvMCSegment", m_useAvMCSeg = true );
}

TruePhotonEmissionPoint::~TruePhotonEmissionPoint() { }

StatusCode TruePhotonEmissionPoint::initialize()
{
  // Sets up various tools and services
  const StatusCode sc = Rich::Rec::ToolBase::initialize();
  if ( sc.isFailure() ) { return sc; }

  // Acquire instances of tools
  acquireTool( "RichRecMCTruthTool", m_mcRecTool );

  // Print a warning this tool is being used, as it should only be
  // used in specialist studies.
  warning() << "Cheating photon emission point using MC" << endmsg;

  // return
  return sc;
}

void
TruePhotonEmissionPoint::bestAvMCSegmentPoint( const LHCb::RichRecSegment * segment,
                                               Gaudi::XYZPoint & emissPnt ) const
{
  // Get the MC segment for this segment
  const auto * mcSegment =
    ( m_useAvMCSeg ? m_mcRecTool->mcRichSegment(segment) : nullptr );
  if ( UNLIKELY( nullptr != mcSegment ) )
  {
    // Create average point from all true photons
    emissPnt = Gaudi::XYZPoint(0,0,0);
    for ( const auto & mcPhot : mcSegment->mcRichOpticalPhotons() )
    {
      emissPnt += Gaudi::XYZVector(mcPhot->emissionPoint());
    }
    emissPnt /= mcSegment->mcRichOpticalPhotons().size();
  }
  else
  {
    // just use the reco value
    emissPnt = segment->trackSegment().bestPoint();
  }
}

bool TruePhotonEmissionPoint::emissionPoint( const LHCb::RichRecSegment * segment,
                                             const LHCb::RichRecPixel * pixel,
                                             Gaudi::XYZPoint & emissPnt ) const
{
  // First a sanity check to make sure we have a segment...
  if ( segment )
  {
    if ( pixel )
    {
      // We have both a pixel and segment, so first try and find the
      // MC photon for this specific pair. If that fails instead use
      // just the segment to compute an average for all MC photons
      // associated with it
      const auto * mcPhoton = m_mcRecTool->trueOpticalPhoton(segment,pixel);
      if ( mcPhoton ) { emissPnt = mcPhoton->emissionPoint();   }
      else            { bestAvMCSegmentPoint(segment,emissPnt); }
    }
    else
    {
      // No pixel, so just use the segment
      bestAvMCSegmentPoint(segment,emissPnt);
    }
    return true;
  }
  return false;
}

void
TruePhotonEmissionPoint::bestAvMCSegmentPoint( const LHCb::RichRecSegment * segment,
                                               const double fracDist,
                                               Gaudi::XYZPoint & emissPnt ) const
{
  // get the MC RichSegment for this segment
  const auto * mcSegment =
    ( m_useAvMCSeg ? m_mcRecTool->mcRichSegment(segment) : nullptr );
  if ( UNLIKELY( nullptr != mcSegment ) )
  {
    // just compute the point the given fraction along the line from the MC
    // entry and exit points.
    // Not perfect... Could be improved using the MC trajectory info..
    emissPnt = ( mcSegment->entryPoint() +
                 ( fracDist*( mcSegment->exitPoint() - mcSegment->entryPoint() ) ) );
  }
  else
  {
    // just use the reco value
    emissPnt = segment->trackSegment().bestPoint(fracDist);
  }
}

bool TruePhotonEmissionPoint::emissionPoint( const LHCb::RichRecSegment * segment,
                                             const LHCb::RichRecPixel * pixel,
                                             const double fracDist,
                                             Gaudi::XYZPoint & emissPnt ) const
{
  // First a sanity check to make sure we have a segment...
  if ( segment )
  {
    if ( pixel )
    {
      // We have both a pixel and segment, so first try and find the
      // MC photon for this specific pair. If that fails instead use
      // just the segment to compute an average for all MC photons
      // associated with it
      const auto * mcPhoton = m_mcRecTool->trueOpticalPhoton(segment,pixel);
      if ( mcPhoton ) { emissPnt = mcPhoton->emissionPoint();            }
      else            { bestAvMCSegmentPoint(segment,fracDist,emissPnt); }
    }
    else
    {
      // No pixel, so just use the segment
      bestAvMCSegmentPoint(segment,fracDist,emissPnt);
    }
    return true;
  }
  return false;
}

//-----------------------------------------------------------------------------

DECLARE_TOOL_FACTORY( TruePhotonEmissionPoint )

//-----------------------------------------------------------------------------
