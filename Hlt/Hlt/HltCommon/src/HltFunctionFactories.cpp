// $Id: HltFunctionFactories.cpp,v 1.41 2010-08-28 23:22:16 gligorov Exp $
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

  declare("L0ET");
  declare<Hlt::PT>("PT");
  declare<Hlt::PT>("PT0");
  declare<Hlt::P>("P");
  declare<Hlt::PTFraction>("PTFraction");
  declare<Hlt::TrackPerpSlope>("TrackPerpSlope");  
  declare<Hlt::TrackFlag>("IsBackward");
  declare<Hlt::MissedVeloHits>("MissedVeloHits");
  declare<Hlt::NumberOfASideVeloHits>("NumberOfASideVeloHits");
  declare<Hlt::NumberOfCSideVeloHits>("NumberOfCSideVeloHits");
  declare<Hlt::NumberOfTrackHits>("NumberOfTrackHits");

  declare<Hlt::IPS,zen::abs_min,const Hlt::VertexSelection>("IPS");
  declare<Hlt::IPS,zen::abs_min,const Hlt::VertexSelection>("FitIPS");
  declare<Hlt::IP,zen::abs_min,const Hlt::VertexSelection>("IP");
  declare<Hlt::IP,zen::abs_min,const Hlt::VertexSelection>("FitIP");

  declare<Hlt::DOCA,zen::abs_min,const Hlt::TrackSelection>("DOCA");
  declare<Hlt::MatchIDsFraction,zen::abs_max,const Hlt::TrackSelection>("MatchIDsFraction");
  declare<Hlt::DoShareM3,zen::abs_max,const Hlt::TrackSelection>("DoShareM3");

  declare<Hlt::FitChi2OverNdf>("FitChi2OverNdf");
  declare<Hlt::FitMuChi2>("FitMuChi2");
  declare<Hlt::FitCleanedChi2OverNdf>("FitCleanedChi2OverNdf");

  declare<ITrackBiFunctionTool,zen::abs_min,const Hlt::TrackSelection> ("Calo3DChi2","HltVeloTCaloMatch");
  declare<ITrackBiFunctionTool,zen::abs_min,const Hlt::TrackSelection> ("RZVeloTMatch","HltMatchTVeloTracks");

  declare<ITrackFunctionTool>("MuonIDDistance","HltMuonIDDistanceTool");
  declare<ITrackFunctionTool>("DeltaP","HltDeltaPTool");
  declare<ITrackFunctionTool>("IsMuon","HltIsMuonTool");
  declare<ITrackFunctionTool>("IsPhoton","HltIsPhotonTool");
  declare<ITrackFunctionTool>("AntiEleConf","HltAntiEleConf");
  declare<ITrackBiFunctionTool,zen::abs_min,const Hlt::TrackSelection> ("Ecal3DChi2","HltVeloEcalMatch");

  declare<ITrackFunctionTool>("ptAtOrigin","PtTransporter");

  declare<Hlt::CosThetaStar,zen::abs_min,const Hlt::TrackSelection> ("CosThetaStar");
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

  declare<Hlt::VertexZPosition>("VertexZPosition");
  declare<Hlt::VertexTransversePosition>("VertexTransversePosition");

  //declare<Hlt::DS,zen::abs_min,const Hlt::VertexSelection>("VertexDistanceSignificance");
  declare<Hlt::D,zen::abs_min,const Hlt::VertexSelection>("VertexDistance");
  declare<Hlt::D,zen::abs_min,const Hlt::VertexSelection>("FitVertexDistance");
  declare<Hlt::DZ,zen::abs_min,const Hlt::VertexSelection>("VertexDz");
  declare<Hlt::DZ,zen::abs_min,const Hlt::VertexSelection>("FitVertexDz");
  declare<Hlt::FC,zen::abs_min,const Hlt::VertexSelection>("VertexPointing");
  declare<Hlt::FC,zen::abs_min,const Hlt::VertexSelection>("FitVertexPointing");
  
  declare<Hlt::VertexMinIP,zen::abs_min,const Hlt::VertexSelection>("VertexMinIP");
  declare<Hlt::VertexMinIP,zen::abs_min,const Hlt::VertexSelection>("FitVertexMinIP");
  declare<Hlt::VertexTrack1IP,zen::abs_min,const Hlt::VertexSelection>("VertexTrack1IP");
  declare<Hlt::VertexTrack1IP,zen::abs_min,const Hlt::VertexSelection>("FitVertexTrack1IP");
  declare<Hlt::VertexTrack2IP,zen::abs_min,const Hlt::VertexSelection>("VertexTrack2IP");
  declare<Hlt::VertexTrack2IP,zen::abs_min,const Hlt::VertexSelection>("FitVertexTrack2IP");

  declare<Hlt::VertexMatchIDsFraction,zen::abs_max,const Hlt::VertexSelection>("VertexMatchIDsFraction");
  declare<Hlt::VertexTracksMatchIDsFraction>("VertexTracksMatchIDsFraction");

  declare<Hlt::VertexAngle>("VertexAngle");
  declare<Hlt::VertexAngle>("FitVertexAngle");
  declare<Hlt::VertexDOCA>("VertexDOCA");
  declare<Hlt::VertexDOCA>("FitVertexDOCA");
  declare<Hlt::VertexDimuonMass>("VertexDimuonMass");
  declare<Hlt::VertexDimuonMass>("FitVertexDimuonMass");
  declare<Hlt::VertexDikaonMass>("VertexDikaonMass");
  declare<Hlt::VertexDikaonMass>("FitVertexDikaonMass");
  declare<Hlt::VertexPT>("VertexPT");
  declare<Hlt::VertexSumPT>("VertexSumPT");
  declare<Hlt::VertexMinPT>("VertexMinPT");
  declare<Hlt::VertexMaxPT>("VertexMaxPT");
  declare<Hlt::VertexMinP>("VertexMinP");
  declare<Hlt::VertexMaxP>("VertexMaxP");
  declare<Hlt::VertexMinPT>("FitVertexMinPT");
  declare<Hlt::VertexMaxPT>("FitVertexMaxPT");
  declare<Hlt::VertexMinP>("FitVertexMinP");
  declare<Hlt::VertexMaxP>("FitVertexMaxP");
  declare<Hlt::VertexTrack1PT>("VertexTrack1PT");
  declare<Hlt::VertexTrack1PT>("FitVertexTrack1PT");
  declare<Hlt::VertexTrack2PT>("VertexTrack2PT");
  declare<Hlt::VertexTrack2PT>("FitVertexTrack2PT");

  declare<Hlt::VertexMaxChi2OverNdf>("FitVertexMaxChi2OverNdf");
  declare<Hlt::VertexTrack1Chi2OverNdf>("FitVertexTrack1Chi2OverNdf");
  declare<Hlt::VertexTrack2Chi2OverNdf>("FitVertexTrack2Chi2OverNdf");
  declare<Hlt::VertexMaxMuChi2>("FitVertexMaxMuChi2");

  declare<Hlt::VertexNumberOfASideTracks>("VertexNumberOfASideTracks");
  declare<Hlt::VertexNumberOfCSideTracks>("VertexNumberOfCSideTracks");

  declare<IRecVertexFunctionTool>("VertexDiElectronMass","VertexDiElectronMassTool");
  return sc;
}

//=============================================================================
