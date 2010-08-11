// $Id: RecVertexClonerWithTracks.cpp,v 1.6 2010-08-11 12:52:52 jpalac Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 

#include <MicroDST/ICloneTrack.h>

// local
#include "RecVertexClonerWithTracks.h"
#include "RecVertexClonerFunctors.h"
//-----------------------------------------------------------------------------
// Implementation file for class : RecVertexClonerWithTracks
//
// 2007-12-05 : Juan PALACIOS
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( RecVertexClonerWithTracks );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
RecVertexClonerWithTracks::RecVertexClonerWithTracks( const std::string& type,
                                                      const std::string& name,
                                                      const IInterface* parent )
  : 
  base_class ( type, name , parent ),
  m_trackCloner(0),
  m_trackClonerType("TrackCloner")
{
  declareProperty("ICloneTrack", m_trackClonerType);
}
//=============================================================================
StatusCode RecVertexClonerWithTracks::initialize() 
{
  
  StatusCode sc = base_class::initialize();

  debug() << "Going to get TrackCloner" << endmsg;

  m_trackCloner = tool<ICloneTrack>(m_trackClonerType, 
                                    this->parent() );

  if (m_trackCloner) {
    debug() << "Found TrackCloner " << m_trackCloner->name() << endmsg;
  } else {
    error() << "Failed to find TrackCloner" << endmsg;
  }
  return sc;

}
//=============================================================================
LHCb::VertexBase* RecVertexClonerWithTracks::operator() (const LHCb::VertexBase* vertex)
{
  const LHCb::RecVertex* recVertex = dynamic_cast<const LHCb::RecVertex*>(vertex);
  return (0 != recVertex) ? this->clone(recVertex) : 0;
}
//=============================================================================
LHCb::RecVertex* RecVertexClonerWithTracks::operator() (const LHCb::RecVertex* vertex)
{
  return this->clone(vertex);
}
//=============================================================================
LHCb::RecVertex* RecVertexClonerWithTracks::clone(const LHCb::RecVertex* vertex)
{
  
  LHCb::RecVertex* vertexClone = 
    cloneKeyedContainerItem<MicroDST::BasicRecVertexCloner>(vertex);

  const SmartRefVector< LHCb::Track >& tracks = vertex->tracks();

  if (!tracks.empty()) {
    typedef SmartRefVector<LHCb::Track>::const_iterator tk_iterator;
    for (tk_iterator iTrack = tracks.begin(); 
         iTrack != tracks.end();
         iTrack++) {
      vertexClone->addToTracks( (*m_trackCloner)( iTrack->target() ) );
    }
  }
  
  return vertexClone;
  
}
//=============================================================================
StatusCode RecVertexClonerWithTracks::finalize() 
{
  return base_class::finalize();
}
//=============================================================================
// Destructor
//=============================================================================
RecVertexClonerWithTracks::~RecVertexClonerWithTracks() {} 

//=============================================================================
