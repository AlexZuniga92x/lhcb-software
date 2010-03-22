// $Id: MuonPIDChecker.cpp,v 1.22 2010-03-22 02:54:25 polye Exp $
// Include files 
#include <cmath>
#include <iomanip>


// from Gaudi
#include "GaudiKernel/AlgFactory.h" 

#include "Linker/LinkedTo.h"

// local
#include "MuonPIDChecker.h"

//-----------------------------------------------------------------------------
// Implementation file for class : MuonPIDChecker
//
// 2010-03-20 : Erica Polycarpo Macedo
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( MuonPIDChecker );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
MuonPIDChecker::MuonPIDChecker( const std::string& name,
                                    ISvcLocator* pSvcLocator)
  : GaudiHistoAlg ( name , pSvcLocator )
{
  // Source of track to ID
  declareProperty("TrackLocation",
      m_TracksPath = LHCb::TrackLocation::Default);
  // Source of MuonPID
  declareProperty("MuonIDLocation",
      m_MuonPIDsPath = LHCb::MuonPIDLocation::Default);
  // Source of MuonTracks 
  declareProperty("MuonTrackLocation",
      m_MuonTracksPath = LHCb::TrackLocation::Muon);
  // Look at Long,Downstream or Both types of tracks
  declareProperty("TrackType", m_TrackType = 0 ); // Long
  // OutputLevel for Histograms  
  // "None", "Online", "OfflineExpress", "OfflineFull", "Expert"
  //   0         1          2                 3            4
  declareProperty( "HistosOutput", m_HistosOutput= 3 );
  // Swap between real and MC data   
  declareProperty( "RunningMC", m_RunningMC = false );

  declareProperty( "MonitorCutValues", m_monitCutValues );

  // Limits for the DLL histos
  declareProperty( "DLLlower", m_DLLlower = -1. ); // -10 for DLLFlag = 0
  declareProperty( "DLLupper", m_DLLupper = 6.0 ); // 10 for DLLFlag = 0
  declareProperty( "DLLnbins", m_DLLnbins = 35 ); // 50 for DLLFlag = 0

}
//=============================================================================
// Destructor
//=============================================================================
MuonPIDChecker::~MuonPIDChecker() {} 

//=============================================================================
// Initialization
//=============================================================================
StatusCode MuonPIDChecker::initialize() {
  StatusCode sc = GaudiHistoAlg::initialize(); // must be executed first
  if ( sc.isFailure() ) return sc;  // error printed already by GaudiHistoAlg

    // Get Geometry parameters
  MuonBasicGeometry basegeometry( detSvc(),msgSvc());
  m_NStation= basegeometry.getStations();
  m_NRegion = basegeometry.getRegions();
  if ( msgLevel(MSG::DEBUG) ) debug()   <<"initialize:: Nstations, NRegions "<<
      m_NStation<< ", " << m_NRegion << endmsg;

  unsigned int i=0;
  while(i<m_NStation){
    m_stationNames.push_back(basegeometry.getStationName(i));
    if ( msgLevel(MSG::DEBUG) ) debug()   <<"initialize:: station "<<i<<" "<<m_stationNames[i]<<endmsg;
    i++;
  }
  m_mudet=getDet<DeMuonDetector>("/dd/Structure/LHCb/DownstreamRegion/Muon");
  for(unsigned int station = 0 ; station < m_NStation ; station++ ){
    // z position of each station 
    m_stationZ.push_back(m_mudet->getStationZ(station));
  }

  // Reset overall counters
  m_nMonitorCuts = m_monitCutValues.size();
  if ( msgLevel(MSG::DEBUG) ) debug() << "initialize:: #of cuts = "<<m_nMonitorCuts<< endmsg;
  for (int j=0;j<m_nMonitorCuts+2;j++) {
      m_ntotTr.push_back(0);
      if ( msgLevel(MSG::DEBUG) ) debug() <<"initialize:: cut "<<i<<""<<m_monitCutValues[j]<<endmsg;
  }
  if ( msgLevel(MSG::DEBUG) ) debug() <<"initialize:: size of ntotTr"<<m_ntotTr.size()<<endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode MuonPIDChecker::execute() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;
  if ( msgLevel(MSG::DEBUG) ) debug() <<"initialize:: TrackType "<< m_TrackType <<endmsg;
  if ( msgLevel(MSG::DEBUG) ) debug() <<"initialize:: HistosOutput "<<m_HistosOutput <<endmsg;

  // Get tracks to loop over
  LHCb::Tracks* trTracks = get<LHCb::Tracks>(m_TracksPath);
  if (!trTracks){
    Warning("execute:: Failed to get Track container", StatusCode::SUCCESS);
  }

  // get  MuonPID objects 
  LHCb::MuonPIDs* pMuids=get<LHCb::MuonPIDs>(m_MuonPIDsPath);
  if (!pMuids){
    Warning("execute:: Failed to get MuonPID container", StatusCode::SUCCESS);
  }

  // Get muon tracks to loop over
  LHCb::Tracks* muTracks = get<LHCb::Tracks>(m_MuonTracksPath);
  if (!muTracks){
    Warning("execute:: Failed to get MuonTrack container", StatusCode::SUCCESS);
  }

  if ( msgLevel(MSG::DEBUG) ) debug() << "execute:: Start loop over tracks" << endmsg;

  // Reset event counters  
  m_nTr=0; 
  m_nTrPreSel=0; 
  m_nTrIsMuonLoose=0; 
  m_nTrIsMuon=0; 
  LHCb::Tracks::const_iterator iTrack;
  for (iTrack = trTracks->begin() ; iTrack != trTracks->end() ; iTrack++){

    if(!((*iTrack)->checkFlag(LHCb::Track::Clone))  &&
       ( ((*iTrack)->checkType(LHCb::Track::Long) && (m_TrackType==0 || m_TrackType==2)) ||
         ((*iTrack)->checkType(LHCb::Track::Downstream) && (m_TrackType==1 || m_TrackType==2))) ){

      m_nTr++;
      resetTrInfo();
      if ( msgLevel(MSG::DEBUG) ) debug() << "execute::isDown " << (*iTrack)->checkType(LHCb::Track::Downstream)<<endmsg;
      if ( msgLevel(MSG::DEBUG) ) debug() << "execute::isLong " << (*iTrack)->checkType(LHCb::Track::Long)<<endmsg;
      if ( msgLevel(MSG::DEBUG) ) debug() << "execute::TrackType " << m_TrackType <<endmsg;
       
      const LHCb::Track *pTrack = *iTrack;
      StatusCode scget=getTrackInfo(pTrack);
      if ( msgLevel(MSG::DEBUG) ) debug() << "execute:: track info retrieved" << endmsg;

      if (scget.isFailure()) {
	Warning("execute:: Failed to get track Info ");
	continue;
      }

      getMuonPIDInfo(pTrack, pMuids);
      if ( msgLevel(MSG::DEBUG) ) debug() << "execute:: MuonPID info retrieved" << endmsg;

      getMuonTrackInfo(pTrack, muTracks);
      if ( msgLevel(MSG::DEBUG) ) debug() << "execute:: MuonTrack info retrieved" << endmsg;

     if (m_TrIsPreSel>0) {
       m_nTrPreSel++;
       if ( msgLevel(MSG::DEBUG) ) debug() << "execute:: Track is PreSelected " << m_nTr<<endmsg;
       // Find region hit by the track 
       m_TrRegionM2 = findTrackRegion(1)+1;
       if ( msgLevel(MSG::DEBUG) ) debug() << "execute:: Region was retrieved =  R"<< m_TrRegionM2<<endmsg;
       fillPreSelPlots(m_HistosOutput);
       if ( msgLevel(MSG::DEBUG) ) debug() << "execute:: PreSel Histos filled" << endmsg; 
       fillHitMultPlots(m_HistosOutput);
       if ( msgLevel(MSG::DEBUG) ) debug() << "execute:: Hit Mult Histos filled" << endmsg; 
       if (m_TrIsMuonLoose>0){
	 m_nTrIsMuonLoose++;
         fillIMLPlots(m_HistosOutput);
	 if ( msgLevel(MSG::DEBUG) ) debug() << "execute:: IsMuonLoose Histos filled" << endmsg; 
	 if (m_TrIsMuon>0) {
            m_nTrIsMuon++;
	    fillIMPlots(m_HistosOutput);
	    if ( msgLevel(MSG::DEBUG) ) debug() << "execute:: IsMuon Histos filled" << endmsg; 
         }
	   
       }

     } // track is preselected

    }// track type is satisfied                 
  }// loop over tracks

  fillMultiplicityPlots(m_HistosOutput);

  m_ntotTr[3] += m_nTrIsMuon;
  m_ntotTr[2] += m_nTrIsMuonLoose;
  m_ntotTr[1] += m_nTrPreSel;
  m_ntotTr[0] += m_nTr;
  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode MuonPIDChecker::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;
  if (0<m_ntotTr[1]) {
    double rate[m_nMonitorCuts], rateError[m_nMonitorCuts];
    std::vector<std::string> CutName; 
    CutName.push_back("IsMuonLoose         = "); 
    CutName.push_back("IsMuon              = "); 
    CutName.push_back("IsMuoLoose MuonProb > "); 
    CutName.push_back("IsMuon     MuonProb > "); 
    CutName.push_back("IsMuoLoose DLL      > "); 
    CutName.push_back("IsMuon     DLL      > "); 
    CutName.push_back("IsMuonLoose NShared > "); 
    CutName.push_back("IsMuon     NShared  > "); 
    for (int i=2;i<m_nMonitorCuts+2;i++) {
        rate[i-2]= 100.* (double)m_ntotTr[i]/(double)m_ntotTr[1];
	rateError[i-2]= getRateError( rate[i-2]/100., m_ntotTr[1]);
	if ( msgLevel(MSG::DEBUG) ) debug() << "finalize:: cut"<<i-2 <<":"<< m_ntotTr[i] <<endmsg;
    }
    
    info()<< "----------------------------------------------------------------"<< endmsg;
    if (m_TrackType==0) info()<< "                  MuonID Rates for Long Tracks "<< endmsg;
    if (m_TrackType==1) info()<< "                MuonID Rates for Downstream Tracks "<< endmsg;
    if (m_TrackType==2) info()<< "             MuonID Rates for Long+Downstream Tracks "<< endmsg;
    info()<< "----------------------------------------------------------------"<< endmsg;
    info()<< "  Criterium                          Rate After IsMuonLoose (%) "<< endmsg;
    info()<< "----------------------------------------------------------------"<< endmsg;
    for (int i=0;i<m_nMonitorCuts;i=i+2) 
      info()<< CutName[i] << format( "%3.1f      :", m_monitCutValues[i])<< format("  %7.3f +-%7.3f ", 
	  rate[i], rateError[i])  << endmsg;
                       
    info()<< "----------------------------------------------------------------"<< endmsg;
    info()<< "  Criterium                            Rate After IsMuon (%) "<< endmsg;
    info()<< "----------------------------------------------------------------"<< endmsg;
    for (int i=1;i<m_nMonitorCuts;i=i+2) 
      info()<< CutName[i] << format( "%3.1f      :", m_monitCutValues[i])<< format("  %7.3f +-%7.3f ", 
	  rate[i], rateError[i])  << endmsg;
                       
  }
  info()<< "----------------------------------------------------------------"<< endmsg;
  info()<< "Total Number of Tracks analysed: "<< m_ntotTr[0] << endmsg; 
  info()<< "Total Number of PreSelected Tracks: "<< m_ntotTr[1] << endmsg; 
  info()<< "Total Number of IsMuonLoose Candidates: "<< m_ntotTr[2] << endmsg; 
  info()<< "Total Number of IsMuon Candidates: "<< m_ntotTr[3] << endmsg; 
  info()<< "----------------------------------------------------------------"<< endmsg;
  
  return GaudiHistoAlg::finalize();  // must be called after all other actions
}

//======================================================================
// Extrapolate Tracks to get X,Y of extrapolation point in each station 
//======================================================================
StatusCode MuonPIDChecker::trackExtrapolate(const LHCb::Track *pTrack){

    // get state closest to M1 for extrapolation
    const LHCb::State * state = &(pTrack->closestState(9450));


    if(!state){
      err() << " Failed to get state from track " << endmsg;
      return StatusCode::FAILURE;
    }

   //Project the state into the muon stations
   unsigned int station;
   for(station = 0; station < m_NStation ; station++){
      // x(z') = x(z) + (dx/dz * (z' - z))
      m_trackX.push_back(state->x() + ( state->tx() *
                                        (m_stationZ[station] - state->z()) ));

      m_trackY.push_back(state->y() + ( state->ty() *
                                        (m_stationZ[station] - state->z()) ));
      if ( msgLevel(MSG::DEBUG) ) debug() << "trackExtrapolate:: M"<<station << 
	      "  x= "<< m_trackX[station]<< " and y = "<<m_trackY[station]<<endmsg;
   }
   return StatusCode::SUCCESS;
}

//=====================================================================
int MuonPIDChecker::findTrackRegion(const int sta){
//=====================================================================
// comment: Returns the muon detector region of the extrapolated track;
// authors: G. Lanfranchi & S. Furcas, 
// date:    10/5/09
//=====================================================================
  int chnum = -1; 
  int regnum = -1;
  if ( msgLevel(MSG::DEBUG) ) debug() << "findTrackRegion:: M"<<sta << 
      "  x= "<< m_trackX[sta]<< " and y = "<<m_trackY[sta]<<endmsg;
  m_mudet->Pos2StChamberNumber(m_trackX[sta],m_trackY[sta],sta,chnum,regnum).ignore();
  return regnum;
    
}

//=====================================================================
//  Reset Track Variables  
//====================================================================
void MuonPIDChecker::MuonPIDChecker::resetTrInfo() {
    // reset info about tracks
    m_Trp0 = -10000.;
    m_TrpT = -10000.;
    m_trackX.clear();
    m_trackY.clear();
    // MuonPID Info
    m_TrIsPreSel=0;
    m_TrIsMuon = 1000;
    m_TrIsMuonLoose = 1000;
    m_TrMuonLhd= -1000.;
    m_TrNMuonLhd = -1000.;
    m_TrNShared=1000;
    // Muon Track Info
    m_TrChi2 =-1000;
    m_TrDist2 =-1000;
    m_TrCLquality =-1000;
    m_TrCLarrival =-1000;
    m_Trquality =-1000;
    for (unsigned int i=0;i<20;i++) m_Trnhitsfoi[i] = 0;
    // MC info
    m_TrMCp0 = -1000;
    m_TrnLinks = 0;
    m_TrzDecay = -1000.;
    m_TrType = 1000;

    return;
}
//=====================================================================
//  Fill MuonPID Info      
//====================================================================
void MuonPIDChecker::getMuonPIDInfo(const LHCb::Track *pTrack, LHCb::MuonPIDs* pMuids) {

      int nMuonPIDs=0; //number of MuonPIDs associated to track
      // link between track and MuonPID 
      LHCb::MuonPIDs::const_iterator imuid;
      for (imuid = pMuids->begin() ; imuid != pMuids->end() ; imuid++){
        if ((*imuid)->idTrack() == pTrack){  // found Associated MuonPID
          nMuonPIDs++;
          // Preselection
          if ((*imuid)->PreSelMomentum() && (*imuid)->InAcceptance() ){
            m_TrIsPreSel=1;
            m_TrIsMuon = (*imuid)->IsMuon();
            m_TrIsMuonLoose = (*imuid)->IsMuonLoose();
            if (m_TrIsMuonLoose< m_TrIsMuon){ // Sanity Check  
              Warning("getMuonPIDInfo:: Muon Track IsMuon < IsMuonLoose"); 
	      if ( msgLevel(MSG::DEBUG) ) debug() << "getMuonPIDInfo::Muon Track" << m_nTr
		<<"  IsMuon ="<< m_TrIsMuon <<
		  "< IsMuonLoose= "<< m_TrIsMuonLoose << endmsg;
            
	    }
	    m_TrMuonLhd= (*imuid)->MuonLLMu();
	    m_TrNMuonLhd = (*imuid)->MuonLLBg();
	    m_TrNShared = (*imuid)->nShared();
	    if ( msgLevel(MSG::DEBUG) ) debug() << "getMuonPIDInfo::Muon Track" << m_nTr
		<<"  TrMuonLhd ="<< m_TrMuonLhd << " TrNMuonLhd= "<< m_TrNMuonLhd<< endmsg;
	    
          } // Pre-selection
        }  // Association to Track
      } // loop over MuonPIDs
      if (nMuonPIDs>1) Warning("getMuonPIDInfo:: nMuonPIDs associated to track >1");
      return;
}
//=====================================================================
//  Fill Muon Track Info      
//====================================================================
void MuonPIDChecker::getMuonTrackInfo(const LHCb::Track *pTrack, LHCb::Tracks* muTracks) {

  LHCb::Tracks::const_iterator imuTrack;
  std::vector<int> assocHits(m_NRegion*m_NStation);
  for (imuTrack = muTracks->begin() ; imuTrack != muTracks->end() ; imuTrack++){
    
    // Get Track ancestors
    const LHCb::Track *trParent=NULL;
    int nmothers=0;
    const SmartRefVector<LHCb::Track> Trmothers=(*imuTrack)->ancestors();
    for (SmartRefVector<LHCb::Track>::const_iterator imother=Trmothers.begin();
	imother!=Trmothers.end();imother++){
        trParent = *imother;
	nmothers++;
    }
    if (trParent == NULL){ 
      Warning( "getMuonTrackInfo:: failed to get Muon Track ancestor") ; 
      continue;
    }
    // if muon track ancestor is the current track get info 
    if (trParent == pTrack){
    
      // Sanity Checks   
      unsigned int muTrPS=0;
      if ( (*imuTrack)->info(301,0) && (*imuTrack)->info(302,0)) muTrPS=1;
      if (muTrPS != m_TrIsPreSel) 
        Warning("getMuonTrackInfo:: PS flag is different for Muon Track and MuonPID");
      unsigned int TrIsMuonLoose = (unsigned int)(*imuTrack)->info(303,0);
      if (TrIsMuonLoose != m_TrIsMuonLoose) 
        Warning("getMuonTrackInfo:: IML flag is different for Muon Track and MuonPID");
      unsigned int TrIsMuon = (unsigned int)(*imuTrack)->info(304,0);
      if (TrIsMuon != m_TrIsMuon) 
      	Warning("getMuonTrackInfo:: IM flag is different for Muon Track and MuonPID");
 
      // Get Info
      m_Trquality = (*imuTrack)->info(300,0);
      m_TrChi2 = (*imuTrack)->chi2PerDoF();
      m_TrDist2 = (*imuTrack)->info(305,0);
      m_TrCLquality = (*imuTrack)->info(308,0);
      m_TrCLarrival = (*imuTrack)->info(309,0);
      if ( msgLevel(MSG::DEBUG) ) {
	debug() << "getMuonTrackInfo::Muon Track" << m_nTr
	  <<"  Trquality ="<< m_Trquality << " TrChi2 = "<< m_TrChi2 << endmsg;
	debug() << "getMuonTrackInfo::Muon Track" << m_nTr
	  <<"  TrDist2   ="<< m_TrDist2 << " TrCLquality="<< m_TrCLquality << endmsg;
	debug() << "getMuonTrackInfo::Muon Track" << m_nTr
	  <<"  TrCLArrival   ="<< m_TrCLarrival << endmsg;
      }
      // Look at coords
      std::vector<LHCb::LHCbID> mucoords = (*imuTrack) -> lhcbIDs();
      std::vector<LHCb::LHCbID>::iterator iID;
      for (iID = mucoords.begin(); iID != mucoords.end(); iID++) {
	if (!(iID->isMuon())) continue;

	LHCb::MuonTileID mutile = iID->muonID();
	int region = mutile.region();
	int station = mutile.station();
	int nStatReg = station*m_NRegion+region;
	m_Trnhitsfoi[nStatReg]++;
	if ( msgLevel(MSG::DEBUG) ) debug() << "getMuonTrackInfo:: " << 
	  "nHits in FOI in MR"<<nStatReg << " = "<<m_Trnhitsfoi[nStatReg]<<endmsg;
      } //end of loop over lhcbIDs 

    }
  }
  return;
}
//=====================================================================
//  Fill Multiplicity Plots         
//====================================================================
void MuonPIDChecker::fillMultiplicityPlots(int level) {

  //fill multiplicity plots
  if (level>0){
    plot1D(m_nTrIsMuonLoose, "hNIMLtracks","IsMuonLoose Track multiplicity", -0.5, 11.5, 12);
    plot1D(m_nTrIsMuon, "hNIMtracks","IsMuonLoose Track multiplicity", -0.5, 11.5, 12);
    if (m_nTr>0) plot1D((double)m_nTrIsMuonLoose/m_nTr, "hNIMLtracksRatio","#IsMuonLoose/#Tracks", 0., 1.1, 22);
  }
  if (level>1){
    plot1D(m_nTr, "hNtracks","Track multiplicity", -0.5, 60.5, 61);
    plot1D(m_nTrPreSel, "hPSNtracks","PreSelection Track multiplicity", -0.5, 50.5, 51);
    if (m_nTrPreSel>0){
      plot1D((double)m_nTrIsMuonLoose/m_nTrPreSel, "hNIMLPStracksRatio","#IsMuonLoose/#PSTracks", 0., 1.1, 22);
      plot1D((double)m_nTrIsMuon/m_nTrPreSel, "hNIMPStracksRatio","#IsMuon/#PSTracks", 0., 1.1, 22);
    }
    if (m_nTr>0) plot1D((double)m_nTrIsMuon/m_nTr, "hNIMtracksRatio","#IsMuon/#Tracks", 0., 1.1, 22);
  }
  return;
}
//=====================================================================
//  Fill Plots for PreSelected Tracks        
//====================================================================
void MuonPIDChecker::fillPreSelPlots(int level) {
  if (level>1){
    plot1D( m_TrRegionM2, "hPSRegion", "MS Region for PS  tracks",0.5,4.5,4); 
    plot1D( m_Trp0, "hPSMomentum", "PreSelected Track Momentum (GeV/c^2)", -100., 100., 100);
    plot1D( m_TrpT, "hPSPT", "PreSelected Track p_T (GeV/c^2)", -10., 10., 100);
    plot1D( m_TrIsMuonLoose, "hIML_PS"  , " IsMuonLoose for PS Tracks ", -0.5 , 1.5, 2 );
    plot1D( m_TrIsMuon, "hIM_PS"  , " IsMuon for PS Tracks ", -0.5 , 1.5, 2 );
  }
  if (level>2){
    //Efficiencies
    profile1D( m_Trp0, m_TrIsMuonLoose, "hEffvsP_IML", "IML Efficiency vs P (GeV/c^2)", -100., 100., 100);
    profile1D( m_TrpT, m_TrIsMuonLoose, "hEffvsPT_IML", "IML Efficiency vs PT (GeV/c^2)", -10., 10., 100);
    profile1D( m_Trp0, m_TrIsMuon, "hEffvsP_IM", "IM Efficiency vs P (GeV/c^2)", -100., 100., 100);
    profile1D( m_TrpT, m_TrIsMuon, "hEffvsPT_IM", "IM Efficiency vs PT (GeV/c^2)", -10., 10., 100);
  }
  return; 
}
//=====================================================================
//  Fill Plots for IsMuonLooseCandidates 
//====================================================================
void MuonPIDChecker::fillIMLPlots(int level) {

  if (exp(m_TrMuonLhd)<m_monitCutValues[2]) m_ntotTr[4]++;
  if ((m_TrMuonLhd-m_TrNMuonLhd)<m_monitCutValues[4]) m_ntotTr[6]++;
  if (m_TrNShared<m_monitCutValues[6]) m_ntotTr[8]++;

  if (level>0){
    plot1D( m_Trp0, "hIMLMomentum", "IsMuonLoose Candidate Momentum (GeV/c^2)", -100., 100., 100);
    plot1D( m_TrpT, "hIMLPT", "IsMuonLoose Candidate p_T (GeV/c^2)", -10., 10., 100);
    plot1D( m_TrRegionM2, "hIMLRegion", "MS Region for IML  tracks",0.5,4.5,4); 
  }

  if (level>1){
    plot1D( m_TrNShared, "hNShared_IML"  , " NShared for PS Tracks ", -0.5 , 5.5, 6 );
    plot1D( m_TrDist2, "hDist2_IML", "Muon Dist for IML candidates", 0., 600., 100); 
    char hname1[13];
    char htitle1[48];
    sprintf ( hname1, "hDist2_IML_R%d",m_TrRegionM2);
    sprintf ( htitle1, "Muon Dist for IML candidates at R%d",m_TrRegionM2);
    plot1D( m_TrDist2, hname1 , htitle1 , 0., 600., 100); 

    char hname2[14];
    char htitle2[48];
    plot1D( exp(m_TrMuonLhd), "hProbMu_IML", "Muon Probability for IML candidates", -0.1, 1.1, 60);
    sprintf ( hname2, "hProbMu_IML_R%d",m_TrRegionM2);
    sprintf ( htitle2, "Muon Probability for IML candidates at R%d",m_TrRegionM2);
    plot1D( exp(m_TrMuonLhd), hname2 , htitle2 , -0.1, 1.1, 60);

    plot1D( exp(m_TrNMuonLhd), "hProbNMu_IML", "Non-Muon Probability for IML candidates", -0.1, 1.1, 60);
    plot1D( m_TrMuonLhd-m_TrNMuonLhd, "hProbNMu_IML", "Muon DLL for IML candidates",m_DLLlower, m_DLLupper, m_DLLnbins); 

    plot1D( m_trackX[1], "hNIMLvsXM2", "MS X position at M2 for IML  tracks",-5500, 5500, 550);
    plot1D( m_trackY[1], "hNIMLvsYM2", "MS Y position at M2 for IML  tracks",-5500, 5500, 550);

  }
  if (level>2){
    plot1D( m_TrIsMuon, "hIM_IML"  , " IsMuon for IML Tracks ", -0.5 , 1.5, 2 );
    plot1D(m_TrChi2, "hChi2_IML", "Chi2 per nDOF for IML Candidates", 0., 200., 100);
    plot1D(m_Trquality, "hQuality_IML", "Track Quality for IML Candidates", 0., 200., 100);
    plot1D(m_TrCLquality, "hCLQuality_IML", "Track CL Quality for IML Candidates", -0.1, 1.1, 60);
    plot1D(m_TrCLarrival, "hCLArrival_IML", "Track CL Arrival for IML Candidates", -0.1, 1.1, 60);
  }     
  if (level>3){
    profile1D( std::abs(m_Trp0), exp(m_TrMuonLhd), "hProbMuvsP_IML", "Mean Muon Prob vs p for IML tracks", 0.,100.,100);
    profile1D( std::abs(m_Trp0), exp(m_TrNMuonLhd), "hNProbMuvsP_IML", "Mean non-Muon Prob vs P for IML tracks ", 0.,100.,100);
    char hname[12];
    char htitle[48];
    sprintf ( hname, "hDLL_IML_R%d",m_TrRegionM2);
    sprintf ( htitle, "Muon DLL for IML candidates at R%d",m_TrRegionM2);
    plot1D( m_TrMuonLhd-m_TrNMuonLhd, hname , htitle , m_DLLlower, m_DLLupper, m_DLLnbins);
  }
  return;
}
//=====================================================================
//  Fill Plots for IsMuon Candidates    
//====================================================================
void MuonPIDChecker::fillIMPlots(int level) {
  if (exp(m_TrMuonLhd)<m_monitCutValues[3]) m_ntotTr[5]++;
  if ((m_TrMuonLhd-m_TrNMuonLhd)<m_monitCutValues[5]) m_ntotTr[7]++;
  if (m_TrNShared<m_monitCutValues[7]) m_ntotTr[9]++;
  if (level>1){
    plot1D( m_Trp0, "hIMMomentum", "IsMuon candidate Momentum (GeV/c^2)", -100., 100., 100);
    plot1D( m_TrpT, "hIMPT", "IsMuon candidate p_T (GeV/c^2)", -10., 10., 100);
    plot1D( m_TrRegionM2, "hIMRegion", "MS Region for IM tracks",0.5,4.5,4); 
    plot1D( m_TrNShared, "hNShared_IM"  , " NShared for PS Tracks ", -0.5 , 5.5, 6 );
    plot1D( m_TrDist2, "hDist2_IM", "Muon Dist for IM candidates", 0., 600., 100); 
    char hname[16];
    char htitle[48];
    sprintf ( hname, "hDist2_IM_R%d",m_TrRegionM2);
    sprintf ( htitle, "Muon Dist for IM candidates at R%d",m_TrRegionM2);
    plot1D( m_TrDist2, hname , htitle , 0., 600., 100); 

    plot1D( exp(m_TrMuonLhd), "hProbMu_IM", "Muon Probability for IM candidates", -0.1, 1.1, 60);
    sprintf ( hname, "hProbMu_IM_R%d",m_TrRegionM2);
    sprintf ( htitle, "Muon Probability for IM candidates at R%d",m_TrRegionM2);
    plot1D( exp(m_TrMuonLhd), hname , htitle , -0.1, 1.1, 60);
    plot1D( exp(m_TrNMuonLhd), "hProbNMu_IM", "Non-Muon Probability for IM candidates", -0.1, 1.1, 60);
    plot1D( m_TrMuonLhd-m_TrNMuonLhd, "hProbNMu_IM", "Muon DLL for IM candidates",m_DLLlower, m_DLLupper, m_DLLnbins); 
    plot1D( m_trackX[1], "hNIMvsXM2", "MS X position at M2 for IM  tracks",-5500, 5500, 550);
    plot1D( m_trackY[1], "hNIMvsYM2", "MS Y position at M2 for IM  tracks",-5500, 5500, 550);
  }
  if (level>2){
    plot1D(m_TrChi2, "hChi2_IM", "Chi2 per nDOF for IM Candidates", 0., 200., 100);
    plot1D(m_Trquality, "hQuality_IM", "Track Quality for IM Candidates", 0., 200., 100);
    plot1D(m_TrCLquality, "hCLQuality_IM", "Track CL Quality for IM Candidates", -0.1, 1.1, 60);
    plot1D(m_TrCLarrival, "hCLArrival_IM", "Track CL Arrival for IM Candidates", -0.1, 1.1, 60);
  }
  if (level>3){
    profile1D( std::abs(m_Trp0), exp(m_TrMuonLhd), "hProbMuvsP_IM", "Mean Muon Prob vs p for IM tracks", 0.,100.,100);
    profile1D( std::abs(m_Trp0), exp(m_TrNMuonLhd), "hNProbMuvsP_IM", "Mean non-Muon Prob vs P for IM tracks ", 0.,100.,100);
    char hname[12];
    char htitle[48];
    sprintf ( hname, "hDLL_IM_R%d",m_TrRegionM2);
    sprintf ( htitle, "Muon DLL for IM candidates at R%d",m_TrRegionM2);
    plot1D( m_TrMuonLhd-m_TrNMuonLhd, hname , htitle , m_DLLlower, m_DLLupper, m_DLLnbins);
  }
  return;
}
//=====================================================================
//  Fill Plots for Hit Multiplicities    
//====================================================================
void MuonPIDChecker::fillHitMultPlots(int level) {
  char hname[16], htitle[48];

  if (level>2){
    std::vector<unsigned int>  nhitsfoiS(m_NStation);    
    unsigned int NhitsFOI=0;
    for (unsigned int i=0; i<m_NStation; i++){
      for (unsigned int j=0; j<m_NRegion;j++){
	  nhitsfoiS[i]+=m_Trnhitsfoi[i*m_NRegion+j];
      }
      NhitsFOI += nhitsfoiS[i];
      sprintf (hname, "hNHhitsFOIvsR_M%d",i+1); 
      sprintf (htitle, "Number of hits in FOI for M%d vs MS Region",i+1); 
      if (level>3)plot2D(m_TrRegionM2, nhitsfoiS[i], hname, htitle, 0.5,4.5,-0.5,10.5,4,11); 
      sprintf (hname, "hAvNHhitsFOIvsR_M%d",i+1); 
      sprintf (htitle, "Mean Number of hits in FOI for M%d vs MS Region",i+1); 
      profile1D( m_TrRegionM2, nhitsfoiS[i],hname ,htitle , 0.5, 4.5, 4);

      if (level > 3){
	sprintf ( hname, "hNhitsFOIvsX_M%d", i+1);
	sprintf (htitle, "Number of hits in FOI for M%d vs MS X pos.",i+1); 
	plot2D(m_trackX[i], nhitsfoiS[i], hname, htitle, -0.5,10.5, -5000, 5000, 200); 
	sprintf ( hname, "hNhitsFOIvsY_M%d", i+1);
	sprintf (htitle, "Number of hits in FOI for M%d vs MS Y pos.",i+1); 
	plot2D(m_trackY[i], nhitsfoiS[i], hname, htitle, -0.5,10.5, -5000, 5000, 200); 
	sprintf ( hname, "hAvNhitsFOIvsX_M%d", i+1);
	sprintf (htitle, "Mean Number of hits in FOI for M%d vs MS X pos.",i+1); 
	profile1D(m_trackX[i], nhitsfoiS[i], hname, htitle, -5000, 5000, 200); 
	sprintf ( hname, "hAvNhitsFOIvsY_M%d", i+1);
	sprintf (htitle, "Mean Number of hits in FOI for M%d vs MS Y pos.",i+1); 
	profile1D(m_trackY[i], nhitsfoiS[i], hname, htitle, -5000, 5000, 200); 
      }
    }
    sprintf ( hname, "hAvTotNhitsFOIvsR");
    sprintf (htitle, "Average Number of hits in FOI vs Region"); 
    profile1D(m_TrRegionM2, NhitsFOI, hname, htitle ,0.5,4.5,4); 
    sprintf ( hname, "hAvTotNhitsFOIvsX");
    sprintf (htitle, "Average Number of hits in FOI vs X (M2)"); 
    profile1D(m_trackX[1], NhitsFOI, hname, htitle ,-5000, 5000, 200);
    sprintf ( hname, "hAvTotNhitsFOIvsY");
    sprintf (htitle, "Average Number of hits in FOI vs Y (M2)"); 
    profile1D(m_trackY[1], NhitsFOI, hname, htitle ,-5000, 5000, 200);
  }
  return;
}
//=====================================================================
//  Fill MuonPID Info      
//====================================================================
StatusCode MuonPIDChecker::getTrackInfo(const LHCb::Track* pTrack) {
   // get momentum
   // get state in zero position 
  const LHCb::State * stateP0 = &(pTrack->firstState());
  if (!stateP0){
    if ( msgLevel(MSG::DEBUG) ) debug() << "getTrackInfo:: Failed to get stateP0 for track "<< m_nTr<< endmsg;
    Warning("getTrackInfo:: Failed to get stateP0 for track ",StatusCode::FAILURE); 
  }
  if( std::abs( stateP0->qOverP() ) > 0.001 / Gaudi::Units::GeV ) {
    m_Trp0 = (1./stateP0->qOverP())/Gaudi::Units::GeV;
    m_TrpT = (stateP0->pt()/Gaudi::Units::GeV)*(int)(m_Trp0/fabs(m_Trp0));
  }else if( stateP0->qOverP() > 0. ) {
    m_Trp0 = 1000. / Gaudi::Units::GeV;
    m_TrpT = 1000. / Gaudi::Units::GeV;
  }else {
    m_Trp0 = -1000. / Gaudi::Units::GeV;
    m_TrpT = -1000. / Gaudi::Units::GeV;
  }
  if ( msgLevel(MSG::DEBUG) ) debug() << "getTrackInfo:: TrP0=" << m_Trp0 << " TrPT= "<< m_TrpT << endmsg;

  StatusCode sc = trackExtrapolate(pTrack);
  if (sc.isFailure()) Warning("getTrackInfo:: Failed to extrapolate track ", StatusCode::FAILURE);

  if ( msgLevel(MSG::DEBUG) ){

    for (unsigned int i=0;i<m_NStation;i++) 
	    debug() << "getTrackInfo:: M"<<i<<": TrX =" << m_trackX[i]<< endmsg;
  }
  return StatusCode::SUCCESS;
}
//=====================================================================
//  Rate Error             
//====================================================================
double MuonPIDChecker::getRateError( double rate , int den){

  double RateError = 100.*sqrt(rate*(1.-rate)/(double)den);
  return RateError;
}
