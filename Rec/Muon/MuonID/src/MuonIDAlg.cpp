 // Include files
#include <cstdio>
#include <map>
#include <vector>

// from Gaudi
#include "GaudiKernel/AlgFactory.h"

// local
#include "MuonIDAlg.h"

//boost
#include <boost/assign/list_of.hpp>


//-----------------------------------------------------------------------------
// Implementation file for class : MuonIDAlg
//
// first version
// 02/05/2002 : David Hutchcroft
//
// new FOI parameterization, acceptance cut for inner chamber holes,
// back to original algorithm; code expects all five chambers, even though
// it only checks hits in M2-M5 by now.
// 09/04/2003 : jtmn, miriam
//
// new Event Model version: calculates de muon LL for muons and non muons
// and the number of tracks that share hits, which are the new members of 
// the muonPID object. 
// 14/12/2005 : Erica Polycarpo, Miriam Gandelman 
//
// new FOI uses 3 parameters
// 16/10/2006 : Erica Polycarpo, Miriam Gandelman
//
// small fixes (DLL binning for pions, Landau with 9 parameters, muonPID key)
// 08/02/2007 : Erica Polycarpo, Miriam Gandelman 
//
// added the method makeMuonTrack to create a track object for each MuonPID
// added a foifactor to enlarge the foi if needed. default = 1.
// 07/05/2007 : Erica Polycarpo, Miriam Gandelman
//
// added a smartRef to the muonTracks
// 30/06/2007 : Miriam Gandelman
//
// MuonPID info made also available in muonTracks (PreSelMom, InAcceptance,IsMuon...)
// New property FindQuality. Adds 'quality' info to tracks. This quality can be chi2 from a 
//                           fit with hits in FOI and a state from seed.
// New property AllMuonTracks. Build muonTracks with all info for all seeds even if they 
// are not Muon Candidates.
// 28/04/2009 : Xabier Cid Vidal, Jose Angel Hernando Morata, Erica Polycarpo
//
// Introduction of IsMuonLoose as candidate selector. 
//===================================================
// OR of the hits in FOI (instead of the AND) defines the IsMuonLoose boolean variable
// DLL is built and MuonTrack are stored if IsMuonLoose=true
// IsMuon info is still stored in MuonPID objects
// IsMuonLoose and IsMuon can be used with/without weights (settable via property).
// New property: Weight_flag
// 10/05/2009: X.Cid Vidal, S.Furcas, J.A.Hernando Morata, G. Lanfranchi, E. Polycarpo. 
//
// New DLL definition:
//=====================
//  Use the distance built with the closest hit to make a hypothesis test for muons and 
//  non-muons; the distance distributions are binned in p-bins and muon detector regions;
//  the integral of the distributions is used to evaluate the compatibility of a given track 
//  with the muon (Probmu) /non-muon (Prob non-mu) hypotheses. 
//  Prob(mu) and Prob(non-mu) in the case of correct hypothesis are p-independent.
//  Prob(mu) and Prob(non-mu) are stored in the MuonPID object.  
//
// New properties: DLL_flag (to pass from the old DLL to the binned-integrated DLL)
//                 Parameters of the Landaus for Muons and non-Muons for the binned DLL.
// 03/06/2009: G. Lanfranchi, S. Furcas.
//
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( MuonIDAlg );

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
MuonIDAlg::MuonIDAlg( const std::string& name,
                ISvcLocator* pSvcLocator)
  : GaudiAlgorithm ( name , pSvcLocator ) {

  // Source of track to ID
  declareProperty("TrackLocation",
                  m_TracksPath = LHCb::TrackLocation::Default);

  // Destination of MuonPID
  declareProperty("MuonIDLocation",
                  m_MuonPIDsPath = LHCb::MuonPIDLocation::Default);

  // Destination of MuonPID
  declareProperty("MuonTrackLocation",
                  m_MuonTracksPath = LHCb::TrackLocation::Muon);

 // Destination of MuonPID (all tracks)
  declareProperty("MuonTrackLocationAll",
                   m_MuonTracksPathAll = LHCb::TrackLocation::Muon+"/AllMuonTracks");


  // Pre-selection momentum
  declareProperty( "PreSelMomentum", m_PreSelMomentum = 3000.0);

  // Different depths of stations considered in different momentum ranges
  declareProperty( "MomentumCuts", m_MomentumCuts );

  // function that defines the field of interest size
  // here momentum is scaled to Gaudi::Units::GeV....
  // new formula: p(1) + p(2)*momentum + p(3)*exp(-p(4)*momentum)

  declareProperty( "XFOIParameter1", m_xfoiParam1 );
  declareProperty( "XFOIParameter2", m_xfoiParam2 );
  declareProperty( "XFOIParameter3", m_xfoiParam3 );
  declareProperty( "YFOIParameter1", m_yfoiParam1 );
  declareProperty( "YFOIParameter2", m_yfoiParam2 );
  declareProperty( "YFOIParameter3", m_yfoiParam3 );

  declareProperty("FOIfactor",m_foifactor = 1.);

  declareProperty("distMuon",m_distMuon );
  declareProperty("distPion",m_distPion );

  //want to find quality?
  declareProperty("FindQuality", m_FindQuality = true);
  //Create container with all muonTracks (even if not in acceptance or !IsMuon) 
  declareProperty("AllMuonTracks", m_DoAllMuonTracks = false);
  //declare which quantity you want to get stored
  declareProperty("myMuIDTool",m_myMuIDTool="Chi2MuIDTool");


  //flag to introduce weights in IsMuon/IsMuonLoose:
  declareProperty("Weight_flag",m_weightFlag = false);
  
  //--------------------
  // flag to use DLL:
  //--------------------
  // 0 -- default 
  // 1 -- binned distance with closest hit + integral
  //-----------------------------
  declareProperty("DLL_flag",m_dllFlag = 0);
  
  // Muons - Region1:
  declareProperty( "MupbinsR1", m_MupBinsR1 );
  declareProperty( "MuLandauParameterR1_1", m_MuLanParR1_1 );
  declareProperty( "MuLandauParameterR1_2", m_MuLanParR1_2 );
  declareProperty( "MuLandauParameterR1_3", m_MuLanParR1_3 );
  declareProperty( "MuLandauParameterR1_4", m_MuLanParR1_4 );
  declareProperty( "MuLandauParameterR1_5", m_MuLanParR1_5 );
  declareProperty( "MuLandauParameterR1_6", m_MuLanParR1_6 );
  declareProperty( "MuLandauParameterR1_7", m_MuLanParR1_7 );

  // Muons - Region2:
  declareProperty( "MupbinsR2", m_MupBinsR2 );
  declareProperty( "MuLandauParameterR2_1", m_MuLanParR2_1 );
  declareProperty( "MuLandauParameterR2_2", m_MuLanParR2_2 );
  declareProperty( "MuLandauParameterR2_3", m_MuLanParR2_3 );
  declareProperty( "MuLandauParameterR2_4", m_MuLanParR2_4 );
  declareProperty( "MuLandauParameterR2_5", m_MuLanParR2_5 );

  // Muons - Region3:
  declareProperty( "MupbinsR3", m_MupBinsR3 );
  declareProperty( "MuLandauParameterR3_1", m_MuLanParR3_1 );
  declareProperty( "MuLandauParameterR3_2", m_MuLanParR3_2 );
  declareProperty( "MuLandauParameterR3_3", m_MuLanParR3_3 );
  declareProperty( "MuLandauParameterR3_4", m_MuLanParR3_4 );
  declareProperty( "MuLandauParameterR3_5", m_MuLanParR3_5 );

  // Muons - Region4:
  declareProperty( "MupbinsR4", m_MupBinsR4 );
  declareProperty( "MuLandauParameterR4_1", m_MuLanParR4_1 );
  declareProperty( "MuLandauParameterR4_2", m_MuLanParR4_2 );
  declareProperty( "MuLandauParameterR4_3", m_MuLanParR4_3 );
  declareProperty( "MuLandauParameterR4_4", m_MuLanParR4_4 );
  declareProperty( "MuLandauParameterR4_5", m_MuLanParR4_5 );

  // Non-Muons - Region 1-2-3-4:
  declareProperty( "NonMuLandauParameterR1", m_NonMuLanParR1 );
  declareProperty( "NonMuLandauParameterR2", m_NonMuLanParR2 );
  declareProperty( "NonMuLandauParameterR3", m_NonMuLanParR3 );
  declareProperty( "NonMuLandauParameterR4", m_NonMuLanParR4 );

  declareProperty( "step", m_x = 0.2); // step for the integral
  declareProperty( "nMax_bin", m_nMax = 4000 ); // number of steps

  declareProperty( "nMupBinsR1", m_nMupBinsR1 = 6);
  declareProperty( "nMupBinsR2", m_nMupBinsR2 = 4);
  declareProperty( "nMupBinsR3", m_nMupBinsR3 = 4);
  declareProperty( "nMupBinsR4", m_nMupBinsR4 = 4);

  declareProperty( "MupBinsR1", m_MupBinsR1);
  declareProperty( "MupBinsR2", m_MupBinsR2);
  declareProperty( "MupBinsR3", m_MupBinsR3);
  declareProperty( "MupBinsR4", m_MupBinsR4);
  
}

//=============================================================================
// Destructor
//=============================================================================
MuonIDAlg::~MuonIDAlg() {};

//=============================================================================
// Initialisation. Check parameters
//=============================================================================
StatusCode MuonIDAlg::initialize() {

  // Sets up various tools and services
  const StatusCode sc = GaudiAlgorithm::initialize();
  if ( sc.isFailure() ) { return sc; }

  info()   << "MuonIDAlg v7r3" << endmsg;

  if (msgLevel(MSG::DEBUG) ) {
    debug()  << "==> Initialise" << endmsg;
    debug()  << "Input tracks in: " << m_TracksPath << endmsg;
    debug()  << "Output MuonPID in: " << m_MuonPIDsPath<< endmsg;  
  }
  
  m_ntotmu=0;
  m_mullfail=0;
  m_NStation = 0;
  m_NRegion = 0;
  MuonBasicGeometry basegeometry( detSvc(),msgSvc());
  m_NStation= basegeometry.getStations();
  m_NRegion = basegeometry.getRegions();
  int i=0;
  while(i<m_NStation){
    m_stationNames.push_back(basegeometry.getStationName(i));
    // debug()   <<" station "<<i<<" "<<m_stationNames[i]<<endmsg;
    i++;
  }

  // set the size of the local vectors
  m_padSizeX.resize(m_NStation * m_NRegion);
  m_padSizeY.resize(m_NStation * m_NRegion);
  m_regionInnerX.resize(m_NStation);
  m_regionOuterX.resize(m_NStation);
  m_regionInnerY.resize(m_NStation);
  m_regionOuterY.resize(m_NStation);
  
  // fill local arrays of pad sizes and region sizes
  m_mudet=getDet<DeMuonDetector>("/dd/Structure/LHCb/DownstreamRegion/Muon");
  int station,region;
  for(station = 0 ; station < m_NStation ; station++ ){
    m_regionInnerX[station] = m_mudet->getInnerX(station);
    m_regionOuterX[station] = m_mudet->getOuterX(station);
    m_regionInnerY[station] = m_mudet->getInnerY(station);
    m_regionOuterY[station] = m_mudet->getOuterY(station);
    m_stationZ[station] = m_mudet->getStationZ(station);
    for(region = 0 ; region < m_NRegion ; region++ ){
      m_padSizeX[station * m_NRegion + region]=m_mudet->getPadSizeX(station,region);
      m_padSizeY[station * m_NRegion + region]=m_mudet->getPadSizeY(station,region);
      
      if(m_padSizeX[station * m_NRegion + region]==0){
        return Error( "Muon Chamber Pad Size could not be retrieved !!!" );
      }
    }
  }

  if (msgLevel(MSG::DEBUG) ) {
    debug()  << "-Geometry information ----------------"<< endmsg;
    debug()  << "Outer X M1 = " << m_regionOuterX[0] << endmsg;
    debug()  << "Outer Y M1 = " << m_regionOuterY[0] << endmsg;
    debug()  << "Outer X M5 = " << m_regionOuterX[4] << endmsg;
    debug()  << "Outer Y M5 = " << m_regionOuterY[4] << endmsg;
    debug()  << "Inner X M1 = " << m_regionInnerX[0] << endmsg;
    debug()  << "Inner Y M1 = " << m_regionInnerY[0] << endmsg;
    debug()  << "Inner X M5 = " << m_regionInnerX[4] << endmsg;
    debug()  << "Inner Y M5 = " << m_regionInnerY[4] << endmsg;
    debug()  << "stationZ M1 = " << m_stationZ[0] << endmsg;
    debug()  << "stationZ M2 = " << m_stationZ[1] << endmsg;
    debug()  << "stationZ M3 = " << m_stationZ[2] << endmsg;
    debug()  << "stationZ M4 = " << m_stationZ[3] << endmsg;
    debug()  << "stationZ M5 = " << m_stationZ[4] << endmsg;
    debug()  << "--------------------------------------"<< endmsg;
  }
  
  if( m_MomentumCuts.empty() ||
      m_xfoiParam1.size() != (unsigned)m_NStation*m_NRegion ||
      m_xfoiParam2.size() != (unsigned)m_NStation*m_NRegion ||
      m_xfoiParam3.size() != (unsigned)m_NStation*m_NRegion ||
      m_yfoiParam1.size() != (unsigned)m_NStation*m_NRegion ||
      m_yfoiParam2.size() != (unsigned)m_NStation*m_NRegion ||
      m_yfoiParam3.size() != (unsigned)m_NStation*m_NRegion){ 
    return Error(format("OPTIONS initialising MuonID are missing or wrong size for %d stations and %d regions", 
                        m_NStation, m_NRegion));
    
  }
  
  if( m_MomentumCuts.size() != 2 ) return Error(" OPTIONS are wrong: size of MomentumCuts vector is not correct");
  
  if (msgLevel(MSG::DEBUG) ) {
    debug()  << "MuonIDAlg::  Momentum bins are (MeV/c) " <<endmsg;
    debug()  << " PreSelMomentum = "<<  m_PreSelMomentum << endmsg;
  }
  
  std::vector<double>::const_iterator iMom;
  if (msgLevel(MSG::DEBUG) ){
    for(iMom = m_MomentumCuts.begin() ; iMom != m_MomentumCuts.end() ; iMom++){
      debug()  << "Mom. cuts = " << *iMom << endmsg ;
    }
  }
  
  
  if( m_distPion.size() != 9 || m_distMuon.size() != 9) 
    return Error("OPTIONS are wrong: size of m_distPion or m_distMuon vector is not correct");
  
  //  Load MeasurementProvider Tool
  m_Chi2MuIDTool  = tool<ImuIDTool>(m_myMuIDTool,"myMuIDTool",this);
  
  // GL&SF: Check that Landaus parameters are properly uploaded:
  m_parLandauMu = 6;
  m_parLandauNonMu = 3;
  
  if( m_MuLanParR1_1.size() != m_parLandauMu || m_MuLanParR1_2.size() != m_parLandauMu || 
      m_MuLanParR1_3.size() != m_parLandauMu || m_MuLanParR1_4.size() != m_parLandauMu || 
      m_MuLanParR1_5.size() != m_parLandauMu || m_MuLanParR1_6.size() != m_parLandauMu ||
      m_MuLanParR1_5.size() != m_parLandauMu ) {
    return Error("OPTIONS initialising Landau Parameters for muons in R1 are missing");
  }
  if( m_MuLanParR2_1.size() != m_parLandauMu || m_MuLanParR2_2.size() != m_parLandauMu || 
      m_MuLanParR2_3.size() != m_parLandauMu || m_MuLanParR2_4.size() != m_parLandauMu || 
      m_MuLanParR2_5.size() != m_parLandauMu ) {
    return Error("OPTIONS initialising Landau Parameters for muons in R2 are missing");
  }
  if( m_MuLanParR3_1.size() != m_parLandauMu || m_MuLanParR3_2.size() != m_parLandauMu || 
      m_MuLanParR3_3.size() != m_parLandauMu || m_MuLanParR3_4.size() != m_parLandauMu || 
      m_MuLanParR3_5.size() != m_parLandauMu ){
    return Error("OPTIONS initialising Landau Parameters for muons in R3 are missing");
  }
  if( m_MuLanParR4_1.size() != m_parLandauMu || m_MuLanParR4_2.size() != m_parLandauMu || 
      m_MuLanParR4_3.size() != m_parLandauMu || m_MuLanParR4_4.size() != m_parLandauMu || 
      m_MuLanParR4_5.size() != m_parLandauMu ) {
    return Error("OPTIONS initialising Landau Parameters for muons in R4 are missing");
  }
  
  if( m_NonMuLanParR1.size() != m_parLandauNonMu || m_NonMuLanParR2.size() != m_parLandauNonMu || 
      m_NonMuLanParR3.size() != m_parLandauNonMu || m_NonMuLanParR4.size() != m_parLandauNonMu ) {
    return Error("OPTIONS initialising Landau Parameters for non-muons are missing");
  }

  // GL&SF: Check bin size:
  if( (m_MupBinsR1.size()!=(unsigned)m_nMupBinsR1) || (m_MupBinsR2.size()!=(unsigned)m_nMupBinsR2) ||
      (m_MupBinsR3.size()!=(unsigned)m_nMupBinsR3) || (m_MupBinsR4.size()!=(unsigned)m_nMupBinsR4)){
    return Error("OPTIONS initialising momentum bins for muons are missing");
  }
  
  // GL&SF: Check that dll flag is properly set:
  if (m_dllFlag==0) info() << " -----> DLL standard (old method) " << endmsg;
  if (m_dllFlag==1) info() << " -----> DLL  new (binned-integrated method) " << endmsg;
  
  if (m_dllFlag<0 || m_dllFlag>1) 
    return Error("DLL flag set to a not existing value: allowed values are: 0=DLL old, 1=DLL integrated");
  
  // GL&SF: Check that parameters of the integral are fine
  if ((int)(m_x*m_nMax) !=800) return Error(format("DLL integral cannot be calculated, parameters are wrong: x, N %8.3f, %8.3f",
                                                   m_x,m_nMax));
  
  // GL&SF: Calculate Landaus normalizations:
  StatusCode sc2 = calcLandauNorm();
  if ( sc2.isFailure() ) return Error(" Normalizations of Landaus not properly set ",sc2);
  
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode MuonIDAlg::execute() {

  if (msgLevel(MSG::DEBUG) ) debug()  << "==> Execute" << endmsg;
  m_nmu = 0;

  StatusCode sc = fillCoordVectors();
  if(sc.isFailure()){
    return sc;
  }
  
  LHCb::Tracks* trTracks = get<LHCb::Tracks>(m_TracksPath);

  if (msgLevel(MSG::DEBUG) ) debug()  << "Number of input tracks for MuonID: " << trTracks->size() << endmsg;
  
  // CRJ : Handle the case MuonPID data is already on the TES
  LHCb::MuonPIDs * pMuids(NULL);
  if ( exist<LHCb::MuonPIDs>(m_MuonPIDsPath) )
  {
    // data already in TES, so clear and reuse
    pMuids = get<LHCb::MuonPIDs>(m_MuonPIDsPath);
    pMuids->clear();
    Warning( "MuonPIDs already exist at '" + m_MuonPIDsPath + "' -> Will Replace", 
             StatusCode::SUCCESS ).ignore();
  }
  else
  {
    // make new data and give to gaudi
    pMuids = new LHCb::MuonPIDs();
    put(pMuids,m_MuonPIDsPath);
  }

  LHCb::Tracks * mutracks = new LHCb::Tracks();
  //LHCb::Tracks * mutracks_all = m_DoAllMuonTracks ? new LHCb::Tracks() : 0;
  LHCb::Tracks * mutracks_all = new LHCb::Tracks();
  
  LHCb::Tracks::const_iterator iTrack;
  for( iTrack = trTracks->begin() ; iTrack != trTracks->end() ; iTrack++){
    // in the clone killed output we want only
    // unique && (matched || forward || downstream)
    if(!(*iTrack)->checkFlag(LHCb::Track::Clone)  &&
       ((*iTrack)->checkType(LHCb::Track::Long) ||
        (*iTrack)->checkType(LHCb::Track::Downstream))){
      
      // do the track extrapolations
      StatusCode sc = trackExtrapolate(*iTrack);
      if ( sc.isFailure() ){
        warning() << " trackExtrapolate failed for track " << *iTrack << endmsg;
        continue;
      }

      // Do the complete ID, calculating IsMuon and likelihoods
      LHCb::MuonPID * pMuid = new LHCb::MuonPID;
      pMuid->setIDTrack(*iTrack);
      sc = doID(pMuid);
      if(sc.isFailure()){
        warning() << " doID failed for track " << *iTrack << endmsg;
      }
      
      pMuids->insert( pMuid, (*iTrack)->key() );
      LHCb::Track* mutrack = 0;

      // Build mutrack if IsMuonLoose is 1
      if (pMuid->IsMuonLoose()) {
        mutrack = makeMuonTrack(*pMuid);
        pMuid->setMuonTrack( mutrack );
        mutracks->insert( mutrack, (*iTrack)->key() );
      }
      
      // If required, save all tracks. If tracks already created, simply clone them
      if (m_DoAllMuonTracks) {
        LHCb::Track* mutrack_all;
        if (pMuid->IsMuonLoose())  mutrack_all=mutrack->clone();
        else mutrack_all = makeMuonTrack(*pMuid);
        mutracks_all->insert( mutrack_all, (*iTrack)->key() );        
      }

      sc = calcSharedHits(pMuid, pMuids);
      if (sc.isFailure()){
        warning() << " calcSharedHits failed for track " << *iTrack << endmsg;
      }

    } // long tracks
  }  // loop over tracks

  // Debug : muon identification event summary
  if (msgLevel(MSG::DEBUG) ) {
    debug()  << "MuonIDAlg:: Number of MuonPID objects created: " << pMuids->size()
           << endmsg;
    debug()  << "MuonIDAlg:: Number of tracks with IsMuonLoose = True : " << m_nmu
           << endmsg;
  }
  m_ntotmu += m_nmu;

  // Register the PIDTracks container to the TES
  put(mutracks,m_MuonTracksPath);

  if (msgLevel(MSG::DEBUG) ) debug()  << "execute:: Muon Tracks registered  " << endmsg;

  // Register the PIDTracksIsMuonLoose container to the TES
  if (m_DoAllMuonTracks) put(mutracks_all,m_MuonTracksPathAll);
  else delete mutracks_all;
  if (msgLevel(MSG::DEBUG) ) debug()  << "execute:: All Muon Tracks registered or deleted " << endmsg;

  clearCoordVectors();

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode MuonIDAlg::finalize() {

  if (msgLevel(MSG::DEBUG) ) {
    debug()  << "==> Finalize" << endmsg;
    debug()  << "==> Total number of tracks with IsMuon=1 : " <<
    m_ntotmu << endmsg;
  }
  info()  << "==> Total number of tracks with IsMuon=1 failing likelihood (p<0): " <<
    m_mullfail << endmsg;


  // Execute base class method
  return GaudiAlgorithm::finalize();
}

//=============================================================================
// fill vectors of x,y,z positions for the MuonCoords
//=============================================================================
StatusCode MuonIDAlg::fillCoordVectors(){

  clearCoordVectors(); // sets up Coord vectors of vectors

  // get the MuonCoords for each station in turn
  LHCb::MuonCoords* coords = get<LHCb::MuonCoords>(LHCb::MuonCoordLocation::MuonCoords);
  if ( coords==0 ) return Error(" Cannot retrieve MuonCoords ");

  // loop over the coords
  LHCb::MuonCoords::const_iterator iCoord;
  for ( iCoord = coords->begin() ; iCoord != coords->end() ; iCoord++ ){
    int region = (*iCoord)->key().region();
    int station = (*iCoord)->key().station();
    double x,dx,y,dy,z,dz;
    LHCb::MuonTileID tile=(*iCoord)->key();
    StatusCode sc = m_mudet->Tile2XYZ(tile,x,dx,y,dy,z,dz);
    if (sc.isFailure()){
      warning() << "Failed to get x,y,z of tile " << tile << endmsg;
      continue;
    }
    m_coordPos[station*m_NRegion+region].
      push_back(coordExtent_(x,dx,y,dy,*iCoord));
  }

  return StatusCode::SUCCESS;
}

//=====================================
// Find Landau Parameters:
// G. Lanfranchi & S. Furcas
// 23-5-2009
//=====================================
StatusCode MuonIDAlg::find_LandauParam(const double& p,const std::vector<int>& trackRegion, double *parMu, double *parNonMu){
  // Track region is defined in M2 or M3
  int region=trackRegion[1]; // M2 
  if (region<0) region=trackRegion[2]; // M3 
  if (region==0){//Region 1
    if (p>m_PreSelMomentum && p<m_MupBinsR1[0]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR1_1[i]; } } 
    if (p>m_MupBinsR1[0] && p<m_MupBinsR1[1]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR1_2[i]; } } 
    if (p>m_MupBinsR1[1] && p<m_MupBinsR1[2]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR1_3[i]; } } 
    if (p>m_MupBinsR1[2] && p<m_MupBinsR1[3]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR1_4[i]; } }
    if (p>m_MupBinsR1[3] && p<m_MupBinsR1[4]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR1_5[i]; } }
    if (p>m_MupBinsR1[4] && p<m_MupBinsR1[5]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR1_6[i]; } }
    if (p>m_MupBinsR1[5]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR1_7[i]; } }
    for (int i=0;i<3;i++){parNonMu[i] = m_NonMuLanParR1[i]; }
  }else if (region==1){// Region 2
    if (p>m_PreSelMomentum  && p<m_MupBinsR2[0]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR2_1[i]; } } 
    if (p>m_MupBinsR2[0] && p<m_MupBinsR2[1]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR2_2[i]; } } 
    if (p>m_MupBinsR2[1] && p<m_MupBinsR2[2]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR2_3[i]; } } 
    if (p>m_MupBinsR2[2] && p<m_MupBinsR2[3]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR2_4[i]; } } 
    if (p>m_MupBinsR2[3] ) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR2_5[i]; } } 
    for (int i=0;i<3;i++){parNonMu[i] = m_NonMuLanParR2[i]; } 
  }else if (region==2){// Region 3
    if (p>m_PreSelMomentum  && p<m_MupBinsR3[0]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR3_1[i]; } } 
    if (p>m_MupBinsR3[0] && p<m_MupBinsR3[1]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR3_2[i]; } } 
    if (p>m_MupBinsR3[1] && p<m_MupBinsR3[2]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR3_3[i]; } } 
    if (p>m_MupBinsR3[2] && p<m_MupBinsR3[3]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR3_4[i]; } } 
    if (p>m_MupBinsR3[3] ) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR3_5[i]; } } 
    for (int i=0;i<3;i++){parNonMu[i] = m_NonMuLanParR3[i]; } 
  }else if (region==3){// Region 4
    if (p>m_PreSelMomentum  && p<m_MupBinsR4[0]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR4_1[i]; } } 
    if (p>m_MupBinsR4[0]  && p<m_MupBinsR4[1]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR4_2[i]; } } 
    if (p>m_MupBinsR4[1]  && p<m_MupBinsR4[2]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR4_3[i]; } }
    if (p>m_MupBinsR4[2]  && p<m_MupBinsR4[3]) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR4_4[i]; } }
    if (p>m_MupBinsR4[3] ) {for (int i=0;i<6;i++){parMu[i] = m_MuLanParR4_5[i]; } } 
    for (int i=0;i<3;i++){parNonMu[i] = m_NonMuLanParR4[i]; }
  } else return Error(" Not valid region ");
  return StatusCode::SUCCESS;
}



//=============================================================================
// Clear the coord vector
//=============================================================================
void MuonIDAlg::clearCoordVectors(){
  m_coordPos.clear();
  m_coordPos.resize(m_NStation * m_NRegion);
  m_muonMap.clear();
}
 
//=============================================================================
// Do the identification
//=============================================================================
StatusCode MuonIDAlg::doID(LHCb::MuonPID *pMuid){
   
   // Initializes data members
   pMuid->setIsMuon(0);
   pMuid->setIsMuonLoose(0);
   pMuid->setNShared(0);
   pMuid->setPreSelMomentum(0);
   pMuid->setInAcceptance(0);
   pMuid->setMuonLLMu(-10000.);
   pMuid->setMuonLLBg(-10000.);
   
   // First do a preselection:
   // track is in acceptance? Track has minimum momentum?
   bool passed = true;
   StatusCode sc = preSelection( pMuid,passed );
   if(sc.isFailure()){
     warning() <<" preSelection failed to MuonPID object" << pMuid << endmsg;
     return sc;
   }
   
  // OK: track failed preselection say so and return
   if(!passed){
     if (msgLevel(MSG::DEBUG) ) debug() << " Track failed preselection " << endmsg;
     return StatusCode::SUCCESS;
   }
   
   // find the coordinates in the fields of interest
   sc = setCoords( pMuid );
   if(sc.isFailure()){
     warning() <<" setCoords failed to MuonPID object" << pMuid << endmsg;
     return sc;
   }
   
   // apply ID: depends on the track momentum

   // find the momentum bin we are in
   // it can be 0, 1 or 2
   int momentumBin = 0;
   
   // m_PreSelMomentum = 3000 MeV
   if (m_MomentumPre> m_PreSelMomentum && m_MomentumPre < m_MomentumCuts[0]) {momentumBin = 0;}
   if (m_MomentumPre >= m_MomentumCuts[0] && m_MomentumPre < m_MomentumCuts[1]) {momentumBin = 1;}
   if (m_MomentumPre >= m_MomentumCuts[1]) {momentumBin = 2;}
   
   // now: implement original algorithm:
   // bin 0 M1.and.M2.and.M3
   // bin 1 M1.and.M2.and.M3.and.(M4.or.M5)
   // bin 2 M1.and.M2.and.M3.and.M4.and.M5
   
   std::vector<int> stations;
   bool w[5];
   for(int ist=1;ist<5;ist++){
     if (m_occupancy[ist]>0) stations.push_back(ist);
   }
 
   if (msgLevel(MSG::DEBUG) ) debug()<<"stations="<<stations<<endmsg;
   //bool myIsMuon = IsMuon(stations,m_MomentumPre);
   //bool myIsMuonLoose = IsMuonLoose(stations,m_MomentumPre);

   //GL & SF: IsMuon/IsMuonLoose definition (with/without weights):
   bool myIsMuon = IsMuon(stations,m_MomentumPre,w);
   bool myIsMuonLoose = IsMuonLoose(stations,m_MomentumPre,w);

   if (msgLevel(MSG::DEBUG) ) {
     debug()<<"myIsMuon="<<myIsMuon<<endmsg;
     debug()<<"myIsMuonLoose="<<myIsMuonLoose<<endmsg;
   }
   

   int station;
   
   pMuid->setIsMuon(myIsMuon);
   pMuid->setIsMuonLoose(myIsMuonLoose);
   
   // calculate MuProb
   double MuProb = calcMuProb( pMuid );
   if (msgLevel(MSG::DEBUG) ) debug()  << "MuProb= " << MuProb <<endmsg;
   
   // calculate Muon DLL
   if(m_dllFlag == 0){   //  (old Erica&Miriam code):
     sc = calcMuonLL( pMuid );
     if (msgLevel(MSG::DEBUG) ) debug() << " calcMuonLL failed (P<0) to MuonPID object " << pMuid << endmsg;
     if(sc.isFailure()){
       if (myIsMuonLoose) m_mullfail++; 
     }
   }  else {             // DLL with distance+integral:
     sc = calcMuonLL_dist(pMuid,m_MomentumPre); 
     if(sc.isFailure()){
       if (msgLevel(MSG::DEBUG) ) debug() << " calcMuonLL(binned) failed (P<0) to MuonPID object " << pMuid << endmsg;
       if (myIsMuonLoose) m_mullfail++; 
     }
   }

  
  //increment number of IsMuonLoose=true tracks for monitoring
  if(pMuid->IsMuonLoose()) m_nmu++;

  if (msgLevel(MSG::DEBUG) ) {
    debug()  << "IsMuonLoose = " << pMuid->IsMuonLoose() 
           << " bin = "   << momentumBin <<" " << " p = " << m_MomentumPre << endmsg; 
    debug()  << " coord in FOI ("; 
    for(station = 0; station < m_NStation ; station++ ){
      debug() << m_occupancy[station] << "," ;
    }
   debug() << ")" << endmsg;
  }
  
  return StatusCode::SUCCESS;
 }



//=============================================================================
// Find out if st myst is in input array of stations
//=============================================================================
bool MuonIDAlg::stInStations(const int myst,const std::vector<int>& stations)
{
  
  for (std::vector<int>::const_iterator it = stations.begin();
         it != stations.end(); ++it) 
    {
      int ist = *it;
      if (ist==myst) return true;
    }
  return false;
}


//=============================================================================
// Common IsMuon (with weights) requirements from set 
// of stations with hits in FOI  
// modified  Sara & Gaia
//=============================================================================

bool MuonIDAlg::IsMuon(const std::vector<int>& stations,const double& p,bool *w)
{

  const double pr1=m_PreSelMomentum;
  const double pr2=m_MomentumCuts[0];
  const double pr3=m_MomentumCuts[1];
  
  double mom=p;
  //double mom=p/Gaudi::Units::GeV;

  P_weights(p,w);

  if (msgLevel(MSG::DEBUG) ) {
    debug()<<"IsMuon"<<endmsg;
    debug()<<"pr1="<<pr1<<endmsg;
    debug()<<"pr2="<<pr2<<endmsg;
    debug()<<"pr3="<<pr3<<endmsg;
    debug()<<"IsMuon p="<<mom<<endmsg;
  }
  
  if (mom>pr1 && mom<pr2)
  {
    if (stInStations(1,stations) && stInStations(2,stations)&& w[1] && w[2]) return true;
  }
  
  else if (mom>pr2 && mom<pr3)
  {
    if (stInStations(1,stations) && stInStations(2,stations) && 
        (stInStations(3,stations) || stInStations(4,stations)) && 
        (w[1]&&w[2]&&(w[3]||w[4]))) return true;
  }
  else if (mom>pr3)
  {   
    if (stInStations(1,stations) && stInStations(2,stations) 
        && stInStations(3,stations) && stInStations(4, stations) &&(w[1]&&w[2]&&w[3] &&w[4])) return true;
  }         
    
  return false;
}


//=============================================================================
// Common IsMuonLoose (with weights) requirements from set 
// of stations with hits in FOI  
// modified  Sara & Gaia
//=============================================================================
bool MuonIDAlg::IsMuonLoose(const std::vector<int>& stations,const double& p, bool *w)
{
  //double mom=p/Gaudi::Units::GeV;
  double mom=p;
  std::vector<int> vstations_rel1 = boost::assign::list_of(1)(2)(3);
  std::vector<int> vstations_rel2 = boost::assign::list_of(1)(2)(3)(4);

  if (msgLevel(MSG::DEBUG) ) {
    debug()<<"vstations_rel1"<<vstations_rel1<<endmsg;
    debug()<<"vstations_rel2"<<vstations_rel2<<endmsg;
  }
  
  const double pr1=m_PreSelMomentum;
  const double pr2=m_MomentumCuts[0];

  if (msgLevel(MSG::DEBUG) ) {
    debug()<<"IsMuonLoose"<<endmsg;
    debug()<<"pr1="<<pr1<<endmsg;
    debug()<<"pr2="<<pr2<<endmsg;
    debug()<<"p IsMuonLoose="<<mom<<endmsg;
  }

  P_weights(mom,w);

  if (mom>pr1 && mom<pr2)
  {
    int j=0;
    for (std::vector<int>::const_iterator it = stations.begin();
         it != stations.end(); ++it) 
    {
      int ist = *it;
      if (stInStations(ist,vstations_rel1)&& w[ist]) j+=1;
    }
    if (msgLevel(MSG::DEBUG) ) debug()<<"first bin, j="<<j<<endmsg;
    if (j>1) return true;
  }
   
  else if (mom>pr2)
  {
    int j=0;
    for (std::vector<int>::const_iterator it = stations.begin();
         it != stations.end(); ++it) 
    {
      int ist = *it;
      if (stInStations(ist,vstations_rel2)&& w[ist]) j+=1;
    }
    if (msgLevel(MSG::DEBUG) ) debug()<<"second bin, j="<<j<<endmsg;
    if (j>2) return true;
  }
  return false;
}

//=============================================================================
// Weights for momentum dependance (Sara & Gaia)
//=============================================================================
void MuonIDAlg::P_weights(const double& p, bool *w){

  if (msgLevel(MSG::DEBUG) ) debug() <<" Weights for momentum dependance "<<endmsg;

  for(int i=0;i<5;i++){ w[i]=0;}

  TRandom1 r;
  double Prob[5];
  double eff[5];
  
  for(int i=0;i<5;i++){ eff[i]=r.Rndm();}
  
  Prob[0]=1.0;
  Prob[1]=1.0;
  Prob[2]=1.0;
  Prob[3]=0;
  Prob[4]=0;

  double a,t;  
  //Prob to reach M4 station: 
  if (p>3500) {
    t = p-3500.;
    a = 0.001;
    if(m_weightFlag){
      Prob[3] = pow(a*t,3)/(1.+pow(a*t,3));
    }  
    else{
      Prob[3]=1;
    }    
  }
  
  //Prob to reach M5 station: 
  if (p>4200) {    
    t = p-4200.;
    a = 0.0009; 
    if(m_weightFlag){
      Prob[4] = pow(a*t,4)/(1.+pow(a*t,4));
    }
    else{
      Prob[4]=1; 
    }
    
  }

  for(int i=0;i<5;i++){
    if(eff[i]<Prob[i]) w[i] = kTRUE;
  }
}

//=============================================================================
// calculate MuProb
//=============================================================================
// if found a muon make a probability from the DxDz matching
double MuonIDAlg::calcMuProb(LHCb::MuonPID * pMuid){

  if(pMuid->IsMuonLoose()){
    // slope calculated in M2 and M3
    m_xMatchStation = 1;
    // find slope difference between track and Coords in M2-M3
    double coordSlopeX = ((m_CoordX[m_xMatchStation] -
                           m_CoordX[m_xMatchStation+1])/
                          (m_stationZ[m_xMatchStation] -
                           m_stationZ[m_xMatchStation+1]));
    double dSlopeX = fabs( m_trackSlopeX - coordSlopeX );

    // formula to make this a probability is
    // dSlopeX < 0.005 = 1.0
    // 0.005 < dSlopeX < 0.085 = 1. - ( (dSlopeX-0.005) / 0.08 )
    // 0.085 < dSlopeX = 0.
    if( dSlopeX < 0.005 ) {
      return 1.0;
    }else if( dSlopeX > 0.085 ){
      return 0.0;
    } else {
      return (1.0 - ( (dSlopeX-0.005) / 0.08 ) );
    }
  }else{
    // not passed selection as a muon so probability is zero
    return 0.0;
  }
}


StatusCode MuonIDAlg::calcMuonLL_dist(LHCb::MuonPID * pMuid, const double& p){
//=============================================================================
// comment: Calculate the muon DLL with the integral of the binned closest distance:
// authors: G. Lanfranchi & S. Furcas, 
// date:    10/5/09
//=============================================================================

  pMuid->setMuonLLMu(-10000.);
  pMuid->setMuonLLBg(-10000.);

  // calculate dll only for IsMuonLoose:
  if ( !pMuid->IsMuonLoose() ) return StatusCode::SUCCESS;

  // Initialize some variables:
  double myDist=-1.;
  double ProbMu=-1.;
  double ProbNonMu = -1.;
  double parMu[6];
  double parNonMu[3];
  for(int i=0;i<6;i++){parMu[i]=0;}
  for(int i=0;i<3;i++){parNonMu[i]=0;}
  std::vector<int> trackRegion(m_NStation,-1); 

  // Calculate Distance using the closest hit:
  myDist = calc_closestDist(pMuid,p,closest_region);
  if (myDist<=0) return Error(" Closest Distance < 0 ");
  
  //EP: Store dist to fill Muon Track extra info
  m_dist_out=myDist;

  // Region of the track extrapolated:
  for (int sta=0;sta<m_NStation; sta++){
    trackRegion[sta] = findTrackRegion(sta);
    if (trackRegion[sta]<0 && msgLevel(MSG::DEBUG) ) 
      debug() << format(" Track extrapolation in station %d gives not-existent region ",sta)
              << endmsg;
  }
  

  // Find Landau's parameters for a given track:
  StatusCode sc = find_LandauParam(p, trackRegion, parMu, parNonMu);
  if (sc.isFailure()) {
    return Error(" Find Landau Parameters: no valid region",sc);
  }
  

  // Calculate Prob(mu) and Prob(non-mu) for a given track;
  ProbMu = calc_ProbMu(myDist, parMu);
  if (ProbMu<0) return Error("ProbMu <0", StatusCode::FAILURE); 

  ProbNonMu = calc_ProbNonMu(myDist, parNonMu);
  if (ProbNonMu<0) return Error("ProbNonMu <0", StatusCode::FAILURE);

  // Set in the MuonPID object the ProbMu & ProbNonMu (Not the Log!)
  pMuid->setMuonLLMu(log(ProbMu));
  pMuid->setMuonLLBg(log(ProbNonMu));


  return StatusCode::SUCCESS;
}

//=============================================================================
// Calculate the muon DLL
//=============================================================================
StatusCode MuonIDAlg::calcMuonLL(LHCb::MuonPID * muonid){

  if ( !muonid->IsMuonLoose() ) {
    return StatusCode::SUCCESS;
  }

  const LHCb::Track* pTrack = muonid->idTrack();
  // do the track extrapolations
  StatusCode sc = trackExtrapolate(pTrack);
  if (!sc){
    warning() << "trackExtrapolate fails for track" << pTrack << endmsg;
    muonid->setMuonLLMu(-10000.);
    muonid->setMuonLLBg(-10000.);
    return StatusCode::FAILURE;
  }

  int nhits=0;
  double dist = 0.;
  std::vector<LHCb::MuonCoord*> & mcoord = m_muonMap[muonid];
  std::vector<LHCb::MuonCoord*>::const_iterator iCoord;
  for ( iCoord = mcoord.begin() ; iCoord != mcoord.end() ; iCoord++ ){

    double x,dx,y,dy,z,dz;
    LHCb::MuonTileID tile=(*iCoord)->key();
    StatusCode sc =
      m_mudet->Tile2XYZ(tile,x,dx,y,dy,z,dz);
    if (sc.isFailure()){
      warning() << "Failed to get x,y,z of tile " << tile << endmsg;
      continue;
    }

    int station = (*iCoord)->key().station();
    if (station > 0 ) {
      dist = dist + pow(((x - m_trackX[station])/dx),2) +
        pow(((y - m_trackY[station])/dy),2) ;
      nhits++;
    }
  }

  dist = dist / nhits; 
  //EP: Store dist to fill Muon Track extra info
  m_dist_out=dist;
     
  double LklhMu = m_distMuon[0] * (TMath::Landau (dist,m_distMuon[1],m_distMuon[2])) + 
    m_distMuon[3] * (TMath::Landau (dist,m_distMuon[4],m_distMuon[5])) +
    m_distMuon[6] * (TMath::Landau (dist,m_distMuon[7],m_distMuon[8]));
  double LklhPi = m_distPion[0] * (TMath::Landau (dist,m_distPion[1],m_distPion[2])) + 
    m_distPion[3] * (TMath::Landau (dist,m_distPion[4],m_distPion[5])) +
    m_distPion[6] * (TMath::Landau (dist,m_distPion[7],m_distPion[8]));

  if (LklhMu<0 || LklhPi<0) return StatusCode::FAILURE;
  muonid->setMuonLLMu(log(LklhMu));
  muonid->setMuonLLBg(log(LklhPi));

  return StatusCode::SUCCESS;

}

//=============================================================================
// calculate the number of tracks that share hits
//=============================================================================
StatusCode MuonIDAlg::calcSharedHits( LHCb::MuonPID* muonid, LHCb::MuonPIDs * pMuids ) {

  // if not a muon candidate do nothing
  if ( !muonid->IsMuonLoose() ) return StatusCode::SUCCESS;

  // calculate the distance from the hit to the extrapolated position
  StatusCode sc = calcDist(muonid);
  if( sc.isFailure() ) {
    muonid->setNShared(100);
    return Warning( "calcDist 1 failure", sc );
  }

  double dist1 = m_dist;
  if (msgLevel(MSG::DEBUG) ) debug()  << " mdist 1=  " << m_dist << endmsg;

  // loop over the muonIDs
  LHCb::MuonPIDs::const_iterator iMuon;
  for( iMuon = pMuids->begin(); pMuids->end() != iMuon; ++iMuon ) {

    // skip if this muonID is the same as the input or if IsMuonLoose is false
    if ( *iMuon == muonid || !(*iMuon)->IsMuonLoose() ) continue;

    // see if there are shared hits between the muonIDs
    if ( compareHits( muonid, *iMuon ) ){

      // get dist for this muonID
      StatusCode sc = calcDist(*iMuon);
      if( sc.isFailure() ) {
        warning() << " calcDist 2 failure" << endmsg;
        continue;
      }

      double dist2 = m_dist;
      if (msgLevel(MSG::DEBUG) ) debug()  << " mdist 2=  " << m_dist << endmsg;
      // the muonID which gets the number of shared hits is the one
      // which has the biggest dist
      if( dist2 < dist1 ) {
        int nNSH = muonid->nShared();
        muonid->setNShared(nNSH+1);
      }
      else{
        int nNSH = (*iMuon)->nShared();
        (*iMuon)->setNShared(nNSH+1);
      }
    }
  }
  if (msgLevel(MSG::DEBUG) ) debug()  << "nShared=  " <<  muonid->nShared() << endmsg;

  return StatusCode::SUCCESS;
}

//=============================================================================
//  Method to compare the Coords of the muonIDs
//============================================================================
bool MuonIDAlg::compareHits( LHCb::MuonPID* muonid1, LHCb::MuonPID* muonid2 ){

  bool theSame = false;

  std::vector<LHCb::MuonCoord*> & mcoord1 = m_muonMap[muonid1];
  std::vector<LHCb::MuonCoord*>::const_iterator iCoord1;
  std::vector<LHCb::MuonCoord*> & mcoord2 = m_muonMap[muonid2];
  std::vector<LHCb::MuonCoord*>::const_iterator iCoord2;
  for( iCoord1 = mcoord1.begin() ; iCoord1 != mcoord1.end() ; iCoord1++ ){
    for( iCoord2 = mcoord2.begin() ; iCoord2 != mcoord2.end() ; iCoord2++ ){
      if( (*iCoord1)->key() == (*iCoord2)->key() ) {
        if ( ((*iCoord1)->key().station()) > 0 )  theSame = true;
      }
    }
  }

  return theSame;

}

//=============================================================================
//  Method to calculate the distance from the hit to the extrapolated position
//============================================================================
StatusCode MuonIDAlg::calcDist( LHCb::MuonPID* muonid ){

  m_dist = 0.;

  if ( !muonid->IsMuonLoose() ) {
    return StatusCode::SUCCESS;
  }
  const LHCb::Track* pTrack = muonid->idTrack();
  // do the track extrapolations
  StatusCode sc = trackExtrapolate(pTrack);
  if(!sc){
    warning() << " trackExtrapolate fails for track " <<  pTrack << endmsg;
    return StatusCode::FAILURE;
  }

  std::vector<LHCb::MuonCoord*> & mcoord = m_muonMap[muonid];
  std::vector<LHCb::MuonCoord*>::const_iterator iCoord;
  double mCoordX[5] = {0.,0.,0.,0.,0.};
  double mCoordY[5] = {0.,0.,0.,0.,0.};
  double mCoordDX[5] = {0.,0.,0.,0.,0.};
  double mCoordDY[5] = {0.,0.,0.,0.,0.};
  for( iCoord = mcoord.begin() ; iCoord != mcoord.end() ; iCoord++ ){
    double x,dx,y,dy,z,dz;
    LHCb::MuonTileID tile=(*iCoord)->key();
    StatusCode sc =
      m_mudet->Tile2XYZ(tile,x,dx,y,dy,z,dz);
    if(sc.isFailure()){
      warning()<< "Failed to get x,y,z of tile " << tile << endmsg;
      continue;
    }
    int station = (*iCoord)->key().station();
    if(mCoordX[station] == 0) {
      // this is the first coord found
      mCoordX[station] = x;
      mCoordY[station] = y;
      // difference between x,y and the extrapolated x,y
      mCoordDX[station] = ( x - m_trackX[station] ) / dx;
      mCoordDY[station] = ( y - m_trackY[station] ) / dy;

    }else{
      // get best match in X and Y
      if( (x - m_trackX[station])*(x - m_trackX[station]) +
          (y - m_trackY[station])*(y - m_trackY[station])  <
          pow((mCoordX[station]*dx - m_trackX[station]),2) +
          pow((mCoordY[station]*dy - m_trackY[station]),2) ){
        // this Coord is a better match
        mCoordX[station] = x;
        mCoordY[station] = y;
        mCoordDX[station] = ( x - m_trackX[station] ) / dx;
        mCoordDY[station] = ( y - m_trackY[station] ) / dy;
      }
    }
  } // end of Coords loop

  // calculate the square of the distances
  int nstn = 0;
  for( int stn = 0 ; stn < 5 ; stn++ ){
    if (msgLevel(MSG::DEBUG) ) {
      debug()  << " mCoordDX =  " << stn << mCoordDX[stn] << endmsg;
      debug()  << " mCoordDY =  " << stn << mCoordDY[stn] << endmsg;
    }
    if ( mCoordDX[stn] != 0. ) {
      nstn++;
      m_dist += mCoordDX[stn]*mCoordDX[stn] + mCoordDY[stn]*mCoordDY[stn];
    }
  }
  m_dist = m_dist / nstn; //divide by the number of station

  return StatusCode::SUCCESS;
}

//=============================================================================
// apply the preselection
//=============================================================================
StatusCode MuonIDAlg::preSelection(LHCb::MuonPID * pMuid, bool &passed){
  passed = true;

  // compare momentum and position to acceptance
  // use the first state on track for the preSel
  if (m_MomentumPre < m_PreSelMomentum ){ 
    // failed preselection momentum cut
    pMuid->setPreSelMomentum(0);
    passed = false;
  }else{
    pMuid->setPreSelMomentum(1);
  }
  pMuid->setInAcceptance(1);
  // in first and last station acceptance
  if (msgLevel(MSG::DEBUG) ) {
    debug()  << "trackX0 = " << m_trackX[0] << endmsg;
    debug()  << "trackX4 = " << m_trackX[4] << endmsg;
    debug()  << "trackY0 = " << m_trackY[0] << endmsg;
    debug()  << "trackY4 = " << m_trackY[4] << endmsg;
  }
  if(  ! (fabs(m_trackX[0]) <  m_regionOuterX[0] &&
          fabs(m_trackY[0]) <  m_regionOuterY[0] )  ||
       ! (fabs(m_trackX[m_NStation-1]) <
          m_regionOuterX[m_NStation-1] &&
          fabs(m_trackY[m_NStation-1]) <
          m_regionOuterY[m_NStation-1] )
       ) {
    // outside M1 - M5 region
    pMuid->setInAcceptance(0);
    passed = false;
  }

  // Inner acceptance
  if(   (fabs(m_trackX[0]) <  m_regionInnerX[0] &&
         fabs(m_trackY[0]) <  m_regionInnerY[0] )  ||
        (fabs(m_trackX[m_NStation-1]) <
         m_regionInnerX[m_NStation-1] &&
         fabs(m_trackY[m_NStation-1]) <
         m_regionInnerY[m_NStation-1] ) ) {

    // inside M1 - M5 chamber hole
    pMuid->setInAcceptance(0);
    passed = false;
  }


  return StatusCode::SUCCESS;
}

//=============================================================================
// get the coords
//=============================================================================
StatusCode MuonIDAlg::setCoords(LHCb::MuonPID *pMuid){

  int station;
  for(station = 0 ; station < m_NStation ; station++){
    int region;
    for(region = 0 ; region < m_NRegion ; region++){

      if( !m_coordPos[station*m_NRegion + region].empty() ){

        std::vector<coordExtent_>::const_iterator itPos;
        for(itPos = m_coordPos[station*m_NRegion + region].begin();
            itPos != m_coordPos[station*m_NRegion + region].end();
            itPos++){

          double x = itPos->m_x;
          double dx = itPos->m_dx;
          double y = itPos->m_y;
          double dy = itPos->m_dy;


	  // not optimal this should be called only once per station, region	 
          double foiXDim = m_foifactor*foiX( station, region, m_MomentumPre, dx);
          double foiYDim = m_foifactor*foiY( station, region, m_MomentumPre, dy);      
	  
          // check if the hit is in the window
          if(  ( fabs( x - m_trackX[station] ) < foiXDim ) &&
               ( fabs( y - m_trackY[station] ) < foiYDim )  ) {

            if (msgLevel(MSG::DEBUG) ) {
	      debug()  << "FOIfactor : " << m_foifactor << endmsg;

	      debug()  << "ratioX = " << fabs( x - m_trackX[station])
                     << "ratioY = " << fabs( y - m_trackY[station]) << " foiXDim = "
                     << foiXDim <<" foiYDim = " << foiYDim <<endmsg;
	      debug()  << "padX = " << dx << " padY = " << dy << endmsg;
	    }

            // it is in the window
            // add the hit to the MuonPID
            m_muonMap[pMuid].push_back(itPos->m_pCoord);
            m_occupancy[station]++;

            // also need track angle in X in M2-M3 for angle matching
            if(1 == m_occupancy[station]) {
              // this is the first coord found
              m_CoordX[station] = x;
            }else{
              // get best match X
              if( fabs(x - m_trackX[station]) <
                  fabs(m_CoordX[station] - m_trackX[station]) ){
                // this Coord is a better match
                m_CoordX[station] = x;
              }
            }
          }
        }// itPos
      }
    } //region
  } //station
  return StatusCode::SUCCESS;
}


StatusCode MuonIDAlg::get_closest(LHCb::MuonPID *pMuid, double *closest_x, double *closest_y, double *closest_region){
//=============================================================================
// comment: get the closest hit
// authors: G. Lanfranchi & S. Furcas, 
// date: 10/5/09
//=============================================================================

  double foiXDim,foiYDim;  
  int nhits=0;

  for (int ista=0; ista<5; ista++){
    closest_x[ista] = -1;
    closest_y[ista] = -1;
    small_dist[ista] = 10000000.;
    Fdist[ista] = 0;
    closest_region[ista] = -1;
  }  

  std::vector<LHCb::MuonCoord*> & mcoord = m_muonMap[pMuid];
  std::vector<LHCb::MuonCoord*>::const_iterator iCoord;
  for ( iCoord = mcoord.begin() ; iCoord != mcoord.end() ; iCoord++ ){
    
    double x,dx,y,dy,z,dz;
    LHCb::MuonTileID tile=(*iCoord)->key();
    StatusCode sc =
      m_mudet->Tile2XYZ(tile,x,dx,y,dy,z,dz);
    if (sc.isFailure()){
      warning() << "Failed to get x,y,z of tile " << tile << endmsg;
      continue;
    }
    
    int station = (*iCoord)->key().station();
    int region = (*iCoord)->key().region();
    foiXDim = m_foifactor*foiX( station, region, m_MomentumPre, dx);
    foiYDim = m_foifactor*foiY( station, region, m_MomentumPre, dy);

    // only for M2-M3-M4-M5:
    if (station > 0 ) {

      if(  ( fabs( x - m_trackX[station] ) < foiXDim ) &&
           ( fabs( y - m_trackY[station] ) < foiYDim )  ) {
        
        Fdist[station] =((x - m_trackX[station])/dx * (x - m_trackX[station])/dx) 
          +((y - m_trackY[station])/dy * (y - m_trackY[station])/dy);
        
        nhits++;
        
        if(Fdist[station] < small_dist[station]){
          
          small_dist[station] = Fdist[station];
          closest_x[station]  = (x - m_trackX[station]) / dx;
          closest_y[station]  = (y - m_trackY[station]) / dy;
          closest_region[station] = region;

        }//Fdist
      }//foi
    }//station
  }//coord

  return StatusCode::SUCCESS;
}

//=====================================================================
int MuonIDAlg::findTrackRegion(const int sta){
//=====================================================================
// comment: Returns the muon detector region of the extrapolated track;
// authors: G. Lanfranchi & S. Furcas, 
// date:    10/5/09
//=====================================================================
  int chnum = -1;
  int regnum = -1;  
  m_mudet->Pos2StChamberNumber(m_trackX[sta],m_trackY[sta],sta,chnum,regnum).ignore();  
  return regnum;
  
}
 


//=====================================================================
double MuonIDAlg::calc_closestDist(LHCb::MuonPID *pMuid, const double& p, double *closest_region){
//=====================================================================
// comment: Returns the squared distance calculated with closest hit
// authors: G. Lanfranchi & S. Furcas, 
// date:    10/5/09
//=====================================================================
  
  double closest_dist=0;
  
  StatusCode sc = get_closest(pMuid,closest_x,closest_y,closest_region);
  
  if ( sc.isFailure() ){
    warning() << " Closest_hit failed " << endmsg;
    return -1;
  }  


  if(p>m_PreSelMomentum && p<m_MomentumCuts[0]){
    //3 or 2 stations
    if(m_occupancy[1]>0 && m_occupancy[2]>0 && m_occupancy[3]>0){//M2 &&M3 && M4
      closest_dist = (closest_x[1]*closest_x[1]+closest_y[1]*closest_y[1]) +
        (closest_x[2]*closest_x[2]+closest_y[2]*closest_y[2])+
        (closest_x[3]*closest_x[3]+closest_y[3]*closest_y[3]);
      closest_dist = closest_dist/3.;
      return closest_dist;
    }
    if(m_occupancy[1]>0 && m_occupancy[2]>0){//M2 &&M3
      closest_dist = (closest_x[1]*closest_x[1]+closest_y[1]*closest_y[1]) +
        (closest_x[2]*closest_x[2]+closest_y[2]*closest_y[2]);
      closest_dist = closest_dist/2.;
      return closest_dist;
    }
    if(m_occupancy[1]>0 && m_occupancy[3]>0){//M2 &&M4
      closest_dist = (closest_x[1]*closest_x[1]+closest_y[1]*closest_y[1]) +
        (closest_x[3]*closest_x[3]+closest_y[3]*closest_y[3]);
      closest_dist = closest_dist/2.;
      return closest_dist;
    }
    if(m_occupancy[2]>0 && m_occupancy[3]>0){//M3 &&M4
      closest_dist = (closest_x[2]*closest_x[2]+closest_y[2]*closest_y[2]) +
        (closest_x[3]*closest_x[3]+closest_y[3]*closest_y[3]);
      closest_dist = closest_dist/2.;
      return closest_dist;
    }
  }//3-6
  
  if(p>m_MomentumCuts[0]){
    if(m_occupancy[1]>0 && m_occupancy[2]>0 && m_occupancy[3]>0 && m_occupancy[4]>0){//M2 &&M3&&M4&&M5
      closest_dist = (closest_x[1]*closest_x[1]+closest_y[1]*closest_y[1]) +
        (closest_x[2]*closest_x[2]+closest_y[2]*closest_y[2])+
        (closest_x[3]*closest_x[3]+closest_y[3]*closest_y[3])+
        (closest_x[4]*closest_x[4]+closest_y[4]*closest_y[4]);
      closest_dist = closest_dist/4.;
      return closest_dist;
    }
    if(m_occupancy[1]>0 && m_occupancy[2]>0 && m_occupancy[3]){//M2 && M3 && M4
      closest_dist = (closest_x[1]*closest_x[1]+closest_y[1]*closest_y[1]) +
        (closest_x[2]*closest_x[2]+closest_y[2]*closest_y[2])+
        (closest_x[3]*closest_x[3]+closest_y[3]*closest_y[3]);
      closest_dist = closest_dist/3.;
      return closest_dist;
    }
    if(m_occupancy[1]>0 && m_occupancy[2]>0 && m_occupancy[4]){//M2 && M3 && M5
      closest_dist = (closest_x[1]*closest_x[1]+closest_y[1]*closest_y[1]) +
        (closest_x[2]*closest_x[2]+closest_y[2]*closest_y[2]) +
        (closest_x[4]*closest_x[4]+closest_y[4]*closest_y[4]);
      closest_dist = closest_dist/3.;
      return closest_dist;
    }
    if(m_occupancy[2]>0 && m_occupancy[3]>0 && m_occupancy[4]>0){//M3 &&M4 && M5
      closest_dist = (closest_x[2]*closest_x[2]+closest_y[2]*closest_y[2]) +
        (closest_x[3]*closest_x[3]+closest_y[3]*closest_y[3])+
        (closest_x[4]*closest_x[4]+closest_y[4]*closest_y[4]);
      closest_dist = closest_dist/3.;
      return closest_dist;
    }   
    if(m_occupancy[1]>0 && m_occupancy[3]>0 && m_occupancy[4]>0){//M2 &&M4 && M5
      closest_dist = (closest_x[1]*closest_x[1]+closest_y[1]*closest_y[1]) +
        (closest_x[3]*closest_x[3]+closest_y[3]*closest_y[3])+
        (closest_x[4]*closest_x[4]+closest_y[4]*closest_y[4]);
      closest_dist = closest_dist/3.;
      return closest_dist;
    }
  }
  
  if(!closest_dist) return 0;
  return closest_dist;
  
}


//=====================================================================
double MuonIDAlg::calc_ProbMu(const double& dist0, const double *parMu){
//=====================================================================
// comment: Returns for a given track of dist0 the probability to be a muon;
// authors: G. Lanfranchi & S. Furcas, 
// date:    10/5/09
//=====================================================================
  
  double x = m_x;     // x-width for the integral
  int nMax = m_nMax;   // number of steps
  
  double landau1 = 0;
  double landau2 = 0;
  double landau = 0;
  double ProbMu=0;

  for (Int_t i=0;i<nMax;i++){ 
    double dist = x*Float_t(i);
    if (dist<dist0){
      landau1 = TMath::Landau(dist,parMu[0],parMu[1]);
      landau2 = TMath::Landau(dist,parMu[2],parMu[3]);  
      landau = (landau1 + parMu[4]*landau2);
      ProbMu = ProbMu+x*landau;
    } 
  }
  if(parMu[5]>0){
    return ProbMu = ProbMu/parMu[5];  
  }else{
    warning() << "normalization out of control " << endmsg;
    return -1;
  }
  
}


double MuonIDAlg::calc_ProbNonMu(const double& dist0, const double *parNonMu){
//=====================================================================
// comment: Returns for a given track of dist0 the probability to be a non-muon;
// authors: G. Lanfranchi & S. Furcas, 
// date:    10/5/09
//=====================================================================

  float x = (float)(m_x);     // x-width for the integral
  int nMax = m_nMax;   // number of steps  
  double landau = 0;
  double Prob=0;

  for (Int_t i=0;i<nMax;i++){ 
    double dist = x*Float_t(i);
    if (dist<dist0){
      landau = TMath::Landau(dist,parNonMu[0],parNonMu[1]);
      Prob = Prob+x*landau;
    } 
  }
  if(parNonMu[2]>0){
    if (msgLevel(MSG::DEBUG) ) debug() << "probnmu, parNonMu[2] : "<< Prob <<","<< parNonMu[2] << endmsg;
    return Prob = Prob/parNonMu[2];  
  }else{
    warning() << "ProbNonMu: normalization out of control " << endmsg;
    return -1;
  }
  return -1;  
}

StatusCode MuonIDAlg::calcLandauNorm(){
//=====================================================================
// comment: Normalizations for Landaus for Muons and Non-Muons
// authors: G. Lanfranchi & S. Furcas, 
// date:    10/5/09
//=====================================================================

  double par[5];
  double parnmu[3];
  for(int i=1;i<5;i++){par[i]=0;}  
  for(int i=1;i<3;i++){parnmu[i]=0;}  
  double Norm = -1;

  //===================
  //  Muons - Region 1
  //===================
  for(int i=0;i<5;i++){par[i]=m_MuLanParR1_1[i];}  
  Norm = calcNorm(par);
  if (Norm<0 || Norm==0) return Error("normalization of Muon R1 bin1 out of control");
  m_MuLanParR1_1[5] = Norm;
  
  for(int i=0;i<5;i++){par[i]=m_MuLanParR1_2[i];}  
  Norm = calcNorm(par);
  if (Norm<0 || Norm==0) return Error("normalization of Muon R1 bin2 out of control");
  m_MuLanParR1_2[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR1_3[i];}  
  Norm = calcNorm(par);
  if (Norm<0 || Norm==0) return Error("normalization of Muon R1 bin3 out of control");
  m_MuLanParR1_3[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR1_4[i];}  
  Norm = calcNorm(par);
  if (Norm<0 || Norm==0) return Error("normalization of Muon R1 bin4 out of control");
  m_MuLanParR1_4[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR1_5[i];}  
  Norm = calcNorm(par);
  if (Norm<0 || Norm==0) return Error("normalization of Muon R1 bin5 out of control");
  m_MuLanParR1_5[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR1_6[i];}  
  Norm = calcNorm(par);
  if (Norm<0 || Norm==0) return Error("normalization of Muon R1 bin6 out of control");
  m_MuLanParR1_6[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR1_7[i];}  
  Norm = calcNorm(par);
  if (Norm<0 || Norm==0) return Error("normalization of Muon R1 bin7 out of control");
  m_MuLanParR1_7[5] = Norm;

  //====================
  //  Muons - Region 2
  //====================

  for(int i=0;i<5;i++){par[i]=m_MuLanParR2_1[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R2 bin1 out of control");
  m_MuLanParR2_1[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR2_2[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R2 bin2 out of control");
  m_MuLanParR2_2[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR2_3[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R2 bin3 out of control");
  m_MuLanParR2_3[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR2_4[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R2 bin4 out of control");
  m_MuLanParR2_4[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR2_5[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R2 bin5 out of control");
  m_MuLanParR2_5[5] = Norm;

  //====================
  //  Muons - Region 3
  //====================

  for(int i=0;i<5;i++){par[i]=m_MuLanParR3_1[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R3 bin1 out of control");
  m_MuLanParR3_1[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR3_2[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R3 bin2 out of control");
  m_MuLanParR3_2[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR3_3[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R3 bin3 out of control");
  m_MuLanParR3_3[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR3_4[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R3 bin4 out of control");
  m_MuLanParR3_4[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR3_5[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R3 bin5 out of control");
  m_MuLanParR3_5[5] = Norm;

  //====================
  //  Muons - Region 4
  //====================
  for(int i=0;i<5;i++){par[i]=m_MuLanParR4_1[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R4 bin1 out of control");
  m_MuLanParR4_1[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR4_2[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R4 bin2 out of control");
  m_MuLanParR4_2[5] = Norm;
  
  for(int i=0;i<5;i++){par[i]=m_MuLanParR4_3[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R4 bin3 out of control");
  m_MuLanParR4_3[5] = Norm;

  for(int i=0;i<5;i++){par[i]=m_MuLanParR4_4[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R4 bin4 out of control");
  m_MuLanParR4_4[5] = Norm;
  
  for(int i=0;i<5;i++){par[i]=m_MuLanParR4_5[i];}  
  Norm = calcNorm(par);  
  if (Norm<0 || Norm==0) return Error("normalization of Muon R4 bin5 out of control");
  m_MuLanParR4_5[5] = Norm;


  //=========================
  //  Non-Muons - Region 1
  //=========================

  for(int i=0;i<3;i++){parnmu[i]=m_NonMuLanParR1[i];}  
  Norm = calcNorm_nmu(parnmu);  
  if (Norm<0 || Norm==0) return Error("normalization of Non-Muon R1 out of control");
  m_NonMuLanParR1[2] = Norm;
  
  for(int i=0;i<3;i++){parnmu[i]=m_NonMuLanParR2[i];}  
  Norm = calcNorm_nmu(parnmu);  
  if (Norm<0 || Norm==0)  return Error("normalization of Non-Muon R2 out of control");
  m_NonMuLanParR2[2] = Norm;
  
  for(int i=0;i<3;i++){parnmu[i]=m_NonMuLanParR3[i];}  
  Norm = calcNorm_nmu(parnmu);  
  if (Norm<0 || Norm==0)  return Error("normalization of Non-Muon R3 out of control");
  m_NonMuLanParR3[2] = Norm;
  
  for(int i=0;i<3;i++){parnmu[i]=m_NonMuLanParR4[i];}  
  Norm = calcNorm_nmu(parnmu);  
  if (Norm<0 || Norm==0)  return Error("normalization of Non-Muon R4 out of control");
  m_NonMuLanParR4[2] = Norm;
  
  return StatusCode::SUCCESS;
  
}

//=====================================================================
double MuonIDAlg::calcNorm(double *par){
//=====================================================================
// comment: Calculate Normalizations for muons
// authors: G. Lanfranchi & S. Furcas,
// date:    10/5/09
//=====================================================================
  
  double Norm = 0.;
  double landau1 = 0;
  double landau2 = 0;
  double landau = 0;

  
  for (Int_t i=0;i<m_nMax;i++){ 
    double dist = m_x*(double)i;
    landau1 = TMath::Landau(dist,par[0],par[1]);
    landau2 = TMath::Landau(dist,par[2],par[3]);  
    landau = (landau1 + par[4]*landau2);
    Norm = Norm+m_x*landau;
  }
  return Norm;
}


double MuonIDAlg::calcNorm_nmu(double *par){
//=====================================================================
// comment: Calculate Normalizations for non-muons
// authors: G. Lanfranchi & S. Furcas, 
// date:    10/5/09
//=====================================================================

  double Norm = 0.;
  double landau = 0;
  
  for (Int_t i=0;i<m_nMax;i++){ 
    double dist = m_x*Float_t(i);
    landau = TMath::Landau(dist,par[0],par[1]);
    Norm = Norm+m_x*landau;
  }
  return Norm;
}
//=============================================================================
// Track extrapolation
//=============================================================================
StatusCode MuonIDAlg::trackExtrapolate(const LHCb::Track *pTrack){

  resetTrackLocals();

  // get first state for the preSel cut
  const LHCb::State * state1 = &(pTrack->firstState());
  // get state closest to M1 for extrapolation
  const LHCb::State * state = &(pTrack->closestState(9450.));

  if(!state1) return Error(" Failed to get 1st state from track ");
  if(!state)  return Error(" Failed to get state from track ");

  // get the momentum (MeV/c)
  m_Momentum = state->p();
  m_MomentumPre = state1->p();

  // get the track slope
  m_trackSlopeX = state->tx();

  // Project the state into the muon stations
  int station;
  for(station = 0; station < m_NStation ; station++){
    // x(z') = x(z) + (dx/dz * (z' - z))
    m_trackX.push_back(state->x() + ( state->tx() *
                                      (m_stationZ[station] - state->z()) ));
    m_trackY.push_back(state->y() + ( state->ty() *
                                      (m_stationZ[station] - state->z()) ));
  }

  return StatusCode::SUCCESS;
}

//=============================================================================
// return the FOI in x in a station and region for momentum (in MeV/c)
// this is a simpler version of the parameterization:
// foi = par0 + par2*exp(-par3*p)
//=============================================================================
double MuonIDAlg::foiX(const int &station, const int &region, const double &p,
                    const double &dx){
  if (p < 1000000. ){
    return ( m_xfoiParam1[ station * m_NRegion + region ] +
      m_xfoiParam2[ station * m_NRegion + region ]*
      exp(-m_xfoiParam3[ station * m_NRegion + region ]*p/Gaudi::Units::GeV ) )*dx;
  }else{
    return m_xfoiParam1[ station * m_NRegion + region ] * dx;
  }

  //in the future optimize this checking that 2*dx =m_padSizeX[station * m_NRegion + region]
  //then eliminates dx from function
}

//=============================================================================
// return the FOI in y in a station and region for momentum (in MeV/c)
//=============================================================================
double MuonIDAlg::foiY(const int &station, const int &region, const double &p,
                    const double &dy){
  if ( p < 1000000. ){
    return ( m_yfoiParam1[ station * m_NRegion + region ] +
	m_yfoiParam2[ station * m_NRegion + region ]*
	exp(-m_yfoiParam3[ station * m_NRegion + region ]*p/Gaudi::Units::GeV ) )*dy;
  }else{
    return m_yfoiParam1[ station * m_NRegion + region ] * dy;
  }

}

//=============================================================================
// reset vectors
//=============================================================================
void MuonIDAlg::resetTrackLocals(){
  m_Momentum = -1.;
  m_MomentumPre = -1.;
  m_trackSlopeX = 0.;
  m_trackX.clear();
  m_trackY.clear();
  m_occupancy.clear();
  m_CoordX.clear();
  m_occupancy.resize(m_NStation,0);
  m_CoordX.resize(m_NStation,0.);
  m_dist_out = -1;
}

//=============================================================================
// make the muon segments
//=============================================================================
LHCb::Track* MuonIDAlg::makeMuonTrack(const LHCb::MuonPID& mupid){

  LHCb::Track* mtrack = new LHCb::Track( mupid.key() );
  
  const LHCb::Track* mother = mupid.idTrack();
  mtrack->addToStates( mother->closestState(9450.) );
  
  std::vector<LHCb::MuonCoord*> & mcoord = m_muonMap[(LHCb::MuonPID*) &mupid];
  std::vector<LHCb::MuonCoord*>::const_iterator iCoord;
  
  std::vector<LHCb::LHCbID> ids_init;
  
  //charge lhcbids to track and to fit
  for(iCoord = mcoord.begin(); iCoord != mcoord.end(); ++iCoord ) {
    LHCb::MuonTileID tile= (*iCoord)->key();
    LHCb::LHCbID id(tile);
    mtrack->addToLhcbIDs(id);
    mtrack->setType(LHCb::Track::Muon);
    mtrack->setHistory(LHCb::Track::MuonID);
    // LHCb::Measurement* muMeas= m_measProvider->measurement(id);
    //  mtrack->addToMeasurements( *muMeas );
    // delete muMeas;
    
    ids_init.push_back(id);
  }
  
  if (msgLevel(MSG::DEBUG) ) debug()<<"ids ready to get chi2"<<endmsg;
  
  double Quality=-1;
  if (m_FindQuality) {  
    // get chi2 value    
    LHCb::Track mtrack_partial;
    bool isMuonCandidate=false;
    if (msgLevel(MSG::DEBUG) ) debug()<<"m_Chi2MuIDTool="<<m_Chi2MuIDTool<<endmsg;
    if (!ids_init.empty()) {      
      StatusCode sc = m_Chi2MuIDTool-> muonCandidate(*mother, mtrack_partial,
                                                     isMuonCandidate,ids_init);
      if (!sc.isFailure()) {
        std::vector<LHCb::LHCbID>::const_iterator id;
        for(id = mtrack_partial.lhcbIDs().begin() ; 
            id !=  mtrack_partial.lhcbIDs().end() ; id++){
          if (msgLevel(MSG::DEBUG) ) debug()<< "id is muon? "<<id->isMuon()<<endmsg;
          if (id->isMuon()) {
            if (msgLevel(MSG::DEBUG) ) debug()<< "id station  " 
                                              << id->muonID().station()<<endmsg;
          }
          
          if (msgLevel(MSG::DEBUG) ) debug()<< "id channelID="<< id->channelID()<<endmsg;
        }
        
        StatusCode sc2 = m_Chi2MuIDTool->muonQuality(mtrack_partial,Quality);
        if (!sc2.isFailure()) {
          if (msgLevel(MSG::DEBUG) ) debug()<<"\t Quality="<< Quality<<endmsg;
          mtrack->addInfo(300,Quality);
          mtrack->setChi2PerDoF(Quality);
        }
        else {
          if (msgLevel(MSG::DEBUG) ) debug()<<"Error when preparing track to fit"<<endmsg;
        }
      }
    }
  }
  
  mtrack->addInfo(300,Quality);  
  mtrack->addInfo(301,mupid.PreSelMomentum());
  mtrack->addInfo(302,mupid.InAcceptance());
  mtrack->addInfo(303,mupid.IsMuonLoose());
  mtrack->addInfo(304,mupid.IsMuon());
  
  double Dist=-1.;
  double DLL=-1.;
  double NShared=-1;
  if (mupid.IsMuonLoose()) {
    Dist    = m_dist_out;
    DLL     = mupid.MuonLLMu()-mupid.MuonLLBg();
    NShared = mupid.nShared();
  }
  if (msgLevel(MSG::DEBUG) ) debug()<< "makeMuonTrack:: Dist, DLL, NShared: "<< Dist<<" "<<DLL<<" "<<NShared<<endmsg;
  
  mtrack->addInfo(305,Dist);
  mtrack->addInfo(306,DLL);
  mtrack->addInfo(307,NShared);
  
  double CLQuality=-1.;
  double CLArrival=-1.;
  mtrack->addInfo(308,CLQuality); 
  mtrack->addInfo(309,CLArrival);

  //add mother track to ancestors
  mtrack->addToAncestors(*mother) ;
    
  
  return mtrack; 
}

