// $Id: OnOfflineTool.cpp,v 1.30 2010-08-11 11:57:30 gligorov Exp $
// Include files

// from Gaudi
#include "GaudiKernel/ToolFactory.h"
#include "GaudiKernel/IToolSvc.h"

// from Event model
#include "Event/RecVertex.h"

#include "Kernel/IRelatedPVFinder.h"
// local
#include "OnOfflineTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : OnOfflineTool
//
// 2005-06-08 : Patrick KOPPENBURG
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( OnOfflineTool );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
OnOfflineTool::OnOfflineTool( const std::string& type,
                              const std::string& name,
                              const IInterface* parent )
  : GaudiTool ( type, name , parent )
  , m_onlinePVLocation(LHCb::RecVertexLocation::Velo3D)
  , m_offlinePVLocation(LHCb::RecVertexLocation::Primary)
  , m_online(false)
  , m_offlineDistTool("LoKi::DistanceCalculator")
  , m_onlineDistTool("LoKi::TrgDistanceCalculator")
  , m_offlineVertexFitter("OfflineVertexFitter" )
  , m_onlineVertexFitter("LoKi::FastVertexFitter")
  , m_offlineCombiner("OfflineVertexFitter" )
  , m_onlineCombiner("LoKi::FastVertexFitter")
  , m_offlinePVRelatorName("GenericParticle2PVRelator__p2PVWithIPChi2_OfflineDistanceCalculatorName_/P2PVWithIPChi2")
  , m_onlinePVRelatorName("GenericParticle2PVRelator__p2PVWithIP_OnlineDistanceCalculatorName_/OnlineP2PVWithIP")
  , m_offlineTESTrunk("Phys")
  , m_onlineTESTrunk("Hlt2")
{

  declareInterface<IOnOffline>(this);

  declareProperty( "OfflinePVLocation",    m_offlinePVLocation    );
  declareProperty( "OnlinePVLocation",     m_onlinePVLocation     );

  declareProperty( "Online",               m_online               );

  declareProperty
    ( "OfflineDistanceTool" ,
      m_offlineDistTool     ,
      "The tool to be used as Off-Line Distance Calclulator" ) ;
  declareProperty
    ( "OnlineDistanceTool"  ,
      m_onlineDistTool      ,
      "The tool to be used as On-Line  Distance Calclulator" ) ;
  declareProperty
    ( "OfflineVertexFitter" ,
      m_offlineVertexFitter ,
      "The tool to be used as Off-Line Vertex Fitter" ) ;
  declareProperty
    ( "OnlineVertexFitter"  ,
      m_onlineVertexFitter  ,
      "The tool to be used as On-Line  Vertex Fitter" ) ;

  declareProperty
    ( "OfflineParticleCombiner",
      m_offlineCombiner         ,
      "The tool to be used as Off-Line creator of composed particles" ) ;
  declareProperty
    ( "OnlineParticleCombiner"  ,
      m_onlineCombiner          ,
      "The tool to be used as On-Line  creator of composed particles"  ) ;

  declareProperty( "OfflinePVRelatorName", m_offlinePVRelatorName );
  declareProperty( "OnlinePVRelatorName",  m_onlinePVRelatorName  );

  declareProperty( "OfflineTrunkOnTES", m_offlineTESTrunk );
  declareProperty( "OnlineTrunkOnTES",  m_onlineTESTrunk  );

}

//=============================================================================
// Destructor
//=============================================================================
OnOfflineTool::~OnOfflineTool() {}

// ============================================================================
// initialize
// ============================================================================
StatusCode OnOfflineTool::initialize()
{
  const StatusCode sc = GaudiTool::initialize();
  if (!sc) return sc ;

  if ( context() == "HLT" || context() == "Hlt" )
  {
    if (msgLevel(MSG::DEBUG)) 
      debug() << "Running in HLT context: Online = true" << endmsg ;
    m_online = true ;
  }
  else if ( context() != "" && context() != "Offline" )
  {
    Warning( "Unexpected context '" + context() + 
             "'. Assuming offline mode, please check !" ).ignore();
  }

  // check it is not global
  const IToolSvc* par = dynamic_cast<const IToolSvc*>( this->parent() );
  if ( 0 != par )
  { return Error ( "Parent of OnOfflineTool is ToolSvc. OnOfflineTool *must* be private" ) ; }

  return sc;
}

// ============================================================================
const std::string& OnOfflineTool::primaryVertexLocation() const
{ return online() ? m_onlinePVLocation : m_offlinePVLocation ; }
// ============================================================================
const std::string& OnOfflineTool::distanceCalculatorType() const
{ return online() ? m_onlineDistTool : m_offlineDistTool ; }
// ============================================================================
const std::string& OnOfflineTool::relatedPVFinderType() const
{ return online() ? m_onlinePVRelatorName : m_offlinePVRelatorName ; }
// ============================================================================
const std::string& OnOfflineTool::particleCombinerType() const
{ return online() ? m_onlineCombiner : m_offlineCombiner ; }
// ============================================================================
const std::string& OnOfflineTool::vertexFitterType() const
{ return online() ? m_onlineVertexFitter : m_offlineVertexFitter ; }
// ============================================================================
const std::string& OnOfflineTool::trunkOnTES() const
{ return online() ? m_onlineTESTrunk : m_offlineTESTrunk ; }
// ============================================================================
// The END
// ============================================================================
