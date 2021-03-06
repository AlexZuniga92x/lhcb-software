
// from Gaudi
#include "GaudiKernel/ToolFactory.h"

// MicroDST
#include <MicroDST/ICloneTrack.h>

// local
#include "RecVertexClonerWithTracks.h"
#include "RecVertexClonerFunctors.h"

//-----------------------------------------------------------------------------
// Implementation file for class : RecVertexClonerWithTracks
//
// 2007-12-05 : Juan PALACIOS
//-----------------------------------------------------------------------------

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RecVertexClonerWithTracks::RecVertexClonerWithTracks( const std::string& type,
                                                      const std::string& name,
                                                      const IInterface* parent )
  : base_class    ( type, name , parent ),
    m_trackCloner ( NULL )
{
  declareProperty("ICloneTrack", m_trackClonerType = "TrackCloner" );
}

//=============================================================================

StatusCode RecVertexClonerWithTracks::initialize()
{
  const StatusCode sc = base_class::initialize();
  if ( sc.isFailure() ) return sc;

  m_trackCloner = tool<ICloneTrack>( m_trackClonerType,
                                     this->parent() );

  return sc;
}

//=============================================================================

LHCb::RecVertex*
RecVertexClonerWithTracks::operator() (const LHCb::RecVertex* vertex)
{
  return this->clone(vertex);
}

//=============================================================================

LHCb::RecVertex* RecVertexClonerWithTracks::clone(const LHCb::RecVertex* vertex)
{
  if (!vertex) return NULL;

  // Basic clone of the vertex
  LHCb::RecVertex* vertexClone =
    cloneKeyedContainerItem<MicroDST::BasicRecVertexCloner>(vertex);
  if (!vertexClone) return NULL;

  // get the list of tracks with weights from the original
  const LHCb::RecVertex::TrackWithWeightVector tracks = vertex->tracksWithWeights();

  // clear the list of tracks in the cone
  vertexClone->clearTracks();

  // Clone tracks and add them, with the correct weight, to the clone
  for ( LHCb::RecVertex::TrackWithWeightVector::const_iterator iT = tracks.begin();
        iT != tracks.end(); ++iT )
  {
    vertexClone->addToTracks( (*m_trackCloner)(iT->first), iT->second );
  }

  return vertexClone;
}

//=============================================================================
// Destructor
//=============================================================================
RecVertexClonerWithTracks::~RecVertexClonerWithTracks() {}

//=============================================================================

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( RecVertexClonerWithTracks )
