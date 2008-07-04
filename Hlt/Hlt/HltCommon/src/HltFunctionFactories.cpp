// $Id: HltFunctionFactories.cpp,v 1.10 2008-07-04 12:45:33 graven Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 

// local
#include "HltFunctions.h"
#include "HltFunctionFactories.h"
#include "HltTFunctionFactory.icpp"

//-----------------------------------------------------------------------------
// Implementation file for class : HltFunctionFactories
//
// 2007-12-04 : Jose Angel Hernando Morata
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( HltTrackFunctionFactory );

DECLARE_TOOL_FACTORY( HltVertexFunctionFactory );


//=============================================================================
HltTrackFunctionFactory::HltTrackFunctionFactory( const std::string& type,
                                                  const std::string& name,
                                                  const IInterface* parent )
  : HltTFunctionFactory<LHCb::Track> ( type, name , parent )
{
}

HltTrackFunctionFactory::~HltTrackFunctionFactory() {
  // TODO HltTFunctionFactory needs to delete the m_creators
} 

StatusCode HltTrackFunctionFactory::initialize() {

  StatusCode sc = HltTFunctionFactory<LHCb::Track>::initialize();

  declare<Hlt::PT>("PT");
  declare<Hlt::PT>("PT0");
  declare<Hlt::TrackFlag>("IsBackward");

  declare<Hlt::rIP,zen::abs_min,Hlt::VertexSelection>("rIP");
  declare<Hlt::IP,zen::abs_min,Hlt::VertexSelection>("IP");

  declare<Hlt::DOCA,zen::abs_min,Hlt::TrackSelection>("DOCA");
  declare<Hlt::MatchIDsFraction,zen::abs_max,Hlt::TrackSelection>("MatchIDsFraction");
  declare<Hlt::DoShareM3,zen::abs_max,Hlt::TrackSelection>("DoShareM3");

  declare<ITrackBiFunctionTool,zen::abs_min,Hlt::TrackSelection>
    ("Calo2DChi2","HltRZVeloTCaloMatch");

  declare<ITrackBiFunctionTool,zen::abs_min,Hlt::TrackSelection>
    ("Calo3DChi2","HltVeloTCaloMatch");

  declare<ITrackBiFunctionTool,zen::abs_min,Hlt::TrackSelection>
    ("RZVeloTMatch","HltMatchTVeloTracks");

  declare<ITrackFunctionTool>("MuonIDDistance","HltMuonIDDistanceTool");

  declare<ITrackFunctionTool>("IsMuon","HltIsMuonTool");

  declare<ITrackFunctionTool>("ptAtOrigin","PtTransporter");

  return sc;
}



//=============================================================================


HltVertexFunctionFactory::HltVertexFunctionFactory( const std::string& type,
                                                  const std::string& name,
                                                  const IInterface* parent )
  : HltTFunctionFactory<LHCb::RecVertex> ( type, name , parent )
{
}

HltVertexFunctionFactory::~HltVertexFunctionFactory() {
  // TODO HltTFunctionFactory needs to delete the m_creators
} 

StatusCode HltVertexFunctionFactory::initialize() {

  StatusCode sc = HltTFunctionFactory<LHCb::RecVertex>::initialize();
  
  declare<Hlt::DZ,zen::abs_min,Hlt::VertexSelection>("VertexDz");
  declare<Hlt::FC,zen::abs_min,Hlt::VertexSelection>("VertexPointing");
  declare<Hlt::VertexMinIP,zen::abs_min,Hlt::VertexSelection>("VertexMinIP");
  declare<Hlt::VertexMatchIDsFraction,zen::abs_max,Hlt::VertexSelection>("VertexMatchIDsFraction");
  
  declare<Hlt::VertexDimuonMass>("VertexDimuonMass");
  declare<Hlt::VertexSumPT>("VertexSumPT");
  declare<Hlt::VertexMinPT>("VertexMinPT");
  declare<Hlt::VertexMaxPT>("VertexMaxPT");

  return sc;
}



//=============================================================================

