// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "GaudiKernel/NTuple.h"
#include "GaudiKernel/INTupleSvc.h"
#include "Kernel/IDecayFinder.h"
#include "Kernel/IGeomDispCalculator.h"
#include "Event/EventHeader.h"
#include "Event/ProtoParticle.h"
// #include "Kernel/ILifetimeFitter.h"
#include "Kernel/IPVLocator.h"
#include "Event/TrgDecision.h"

#ifdef MCCheck
#include "MCTools/IMCDecayFinder.h"
#include "GaudiKernel/SmartDataPtr.h"
#include "MCTools/MCTrackInfo.h"
#endif
#include "Event/TrgVertex.h"

// local
#include "DecayChainNTuple.h"

//-----------------------------------------------------------------------------
// Implementation file for class : DecayChainNTuple
//
// 2004-08-01 : Luis Fernandez
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
static const  AlgFactory<DecayChainNTuple>          s_factory ;
const        IAlgFactory& DecayChainNTupleFactory = s_factory ; 


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
DecayChainNTuple::DecayChainNTuple( const std::string& name,
                                    ISvcLocator* pSvcLocator)
  : DVAlgorithm ( name , pSvcLocator )
    , m_bookedNTuple(false)
    , m_PVContainer(VertexLocation::Primary)
    , m_PVLocator()
    , m_ppSvc(0)
    , m_pDKFinder(0)
    , m_IPTool(0)
  //, m_pLifetimeFitter(0)
    , m_EDS(0)
#ifdef MCCheck
    , m_pMCDKFinder(0)
    , m_pAsctLinks(0)
#endif
    ,m_pathTrg(TrgDecisionLocation::Default)
{
  declareProperty("Decay", m_Decay = "B0 -> ^pi+ ^pi-");
#ifdef MCCheck
  declareProperty("MCDecay", m_MCDecay = "B0 -> ^pi+ ^pi-");
  declareProperty("FillMCDecay", m_FillMCDecay = false);
#endif
  declareProperty("NtupleName", m_ntupleName = "FILE1/MySelection" );
  declareProperty("UseRichPID", m_useRichPID = false);
  declareProperty("RichPIDLocation", m_richPIDLocation = "Rec/Rich/TrgPIDs" );


}
//=============================================================================
// Destructor
//=============================================================================
DecayChainNTuple::~DecayChainNTuple() {}; 

//=============================================================================
// Initialization
//=============================================================================
StatusCode DecayChainNTuple::initialize() {

  // Retrieve the ParticlePropertySvc
  StatusCode sc = service("ParticlePropertySvc", m_ppSvc);

  if( sc.isFailure() ) {
    fatal() << "Unable to locate Particle Property Service" << endreq;
    return sc;
  }

  // Retrieve the DecayFinder
  m_pDKFinder = tool<IDecayFinder>("DecayFinder", this);
  if(sc.isFailure()){
    fatal() << "Unable to retrieve DecayFinder" << endreq;
    return sc;
  }

  // Retrieve the LifetimeFitter
  /*
  sc = toolSvc()->retrieveTool("LifetimeFitter", m_pLifetimeFitter, this);
  if(sc.isFailure()){
    err() << " Unable to retrieve LifetimeFitter tool" << endreq;
    return sc;
  }
  */

  // Set the dk to be looked at
  m_pDKFinder->setDecay(m_Decay);
  info() << "Will look for the decay: "<< m_pDKFinder->decay() << endreq;

  sc = toolSvc()->retrieveTool("GeomDispCalculator", m_IPTool, this);
  if(sc.isFailure()){
    err() << " Unable to retrieve GeomDispCalculator tool" << endreq;
    return sc;
  }

  sc = toolSvc()->retrieveTool("PVLocator", m_PVLocator, this);
  if(sc.isFailure()){
    err() << " Unable to retrieve PV Locator tool" << endreq;
    return sc;
  }

  m_PVContainer = m_PVLocator->getPVLocation() ;
   
  info() << "Getting PV from " << m_PVContainer << endreq ;

  // Retrieve the data service
  sc = service("EventDataSvc", m_EDS, true);
  if( sc.isFailure() ) {
    fatal() << "Unable to locate Event Data Service"<< endreq;
    return sc;
  }

#ifdef MCCheck
  // Retrieve the MCDecayFinder
  m_pMCDKFinder = tool<IMCDecayFinder>("MCDecayFinder",this);
  if(sc.isFailure()){
    fatal() << "Unable to retrieve MCDecayFinder tool" << endreq;
    return sc;
  }

  // Set the MC dk to be looked at
  m_pMCDKFinder->setDecay(m_MCDecay);
  info() << "Will look for the MC decay: "<< m_pMCDKFinder->decay() << endreq;
  
  // Link associator
  sc = toolSvc()->retrieveTool("Particle2MCLinksAsct", "LinkAsct", m_pAsctLinks, this);
  if(sc.isFailure()){
    fatal() << "Unable to retrieve the Particle2MCLinksAsct" << endreq;
    return sc;
  }
#endif

  // Trigger
  m_dataProvider = tool<TrgDataProvider> ("TrgDataProvider");

  return StatusCode::SUCCESS;
};

//=============================================================================
// Main execution
//=============================================================================
StatusCode DecayChainNTuple::execute() {

  setFilterPassed(false);

  //---------------------------------------------
  // Retrieve informations about event
  SmartDataPtr<EventHeader> evt(eventSvc(), EventHeaderLocation::Default );

  if(evt){
    debug() << "Retrieved EVENT: " << evt->evtNum()
        << " RUN: " << evt->runNum() << endreq;
    
    m_event = evt->evtNum();
    m_run = evt->runNum();
    
  } else{
    err() << "Not able to retrieve event" << endreq;
    return StatusCode::FAILURE;
  }
  //---------------------------------------------

  // Reset counter m_n and m_mcn if MCCheck
  if(m_bookedNTuple){
    std::map<int, HandleNTuple*>::iterator it;
    for (it = m_HandleNTupleMap.begin(); 
         it != m_HandleNTupleMap.end(); 
         it++){
      it->second->clean();
#ifdef MCCheck
      if(m_FillMCDecay) it->second->mcclean();
#endif
    }
  }

  StatusCode sc = StatusCode::SUCCESS;

  //---------------------------------------------
  // Get the particles
  const ParticleVector parts = desktop()->particles();
  if (parts.size() == 0){
#ifndef MCCheck
    debug() << "No particles in desktop" << endreq;
    return StatusCode::SUCCESS ;
#endif
  } else debug() << "Particles in desktop: "
             << parts.size() << endreq;
  //---------------------------------------------

  debug() << "HOLA" << endreq;

  //---------------------------------------------
  // container holding the head of the decay
  std::vector<Particle*> mothervec;

  // start from beginning
  const Particle* imothervec = NULL;

  while(m_pDKFinder->findDecay(parts, imothervec)){
    Particle* jmothervec = const_cast<Particle*>(imothervec);
    mothervec.push_back(jmothervec);
  }
  debug() << "Found " << mothervec.size() << " reconstructed decay: " << m_pDKFinder->decay() << endreq;
  
  bool foundDK = (mothervec.size()>0);

  // Skip event if no DOI
  if(!foundDK){
    debug() << "Leaving since no DOI found" << endreq;
    return StatusCode::SUCCESS ;    
  }

  //---------------------------------------------

#ifdef MCCheck
  //---------------------------------------------
  // Get the MCparticles
  SmartDataPtr<MCParticles> kmcparts(m_EDS, MCParticleLocation::Default );
  if( !kmcparts ){
    fatal() << "Unable to find MC particles at '" << MCParticleLocation::Default << "'" << endreq;
    return false;
  }
  //---------------------------------------------

  //---------------------------------------------
  // container holding the mc head of the decay
  std::vector<MCParticle*> MCHead;
  const MCParticle* imc = NULL;

  while (m_pMCDKFinder->findDecay(kmcparts, imc)){
    MCParticle* jmc = const_cast<MCParticle*>(imc);
    MCHead.push_back(jmc);
    }
  debug() << "Found " << MCHead.size() << " true decay: " << m_pMCDKFinder->decay() << endreq;
  
  //---------------------------------------------
#endif

  //=============================================================================
  // Reconstructed decay
  //=============================================================================

  //---------------------------------------------
  // WARNING: the key must be identical when booking and filling, for both reco and true decays!!!
  // Case of identical parts: take arbitrary offset and add (500000*forthekeymother)
  // we have: int offsetmother = 9; and then offsetmother++;
  // Case of identical parts: take arbitrary offset and add (500000*forthekeydau)
  // we have: int offsetdau = 99; and then offsetdau++;

  // Book the ntuple
  if(!m_bookedNTuple){
    debug() << "NTuple not booked yet" << endreq;
    BookNTuple(mothervec);
    if (!sc.isSuccess()) return sc;
  } //!m_bookedNTuple
  //---------------------------------------------

  //---------------------------------------------
  // Write the ntuple
  WriteNTuple(mothervec);
  if (!sc.isSuccess()) return sc;
  //---------------------------------------------

#ifdef MCCheck
  //---------------------------------------------
  // Write the true decay part 
  if(m_FillMCDecay){
    WriteMCNTuple(MCHead);
    if (!sc.isSuccess()) return sc;
  }
  
  //---------------------------------------------
#endif

  //---------------------------------------------
  // Save the ntuple
  if(m_bookedNTuple && foundDK){
    NTuplePtr ntuple(ntupleSvc(), m_ntupleName);
    sc = ntuple->write();
    if (sc.isFailure()) err() << "Cannot write NTuple " << endreq;
  }
  //---------------------------------------------

  setFilterPassed(true);
  return StatusCode::SUCCESS;
};

//=============================================================================
//  Finalize
//=============================================================================
StatusCode DecayChainNTuple::finalize() {
  return StatusCode::SUCCESS;
}

//=============================================================================
//  DecayChainNTuple::HandleNTuple::HandleNTuple
//=============================================================================
DecayChainNTuple::HandleNTuple::HandleNTuple(NTuplePtr& nt, unsigned int& number, 
                                             // ILifetimeFitter *lifetimefitter,
                                             IGeomDispCalculator* iptool)
  : m_iptool(iptool)//, m_lifetimefitter(lifetimefitter)
{
  StatusCode sc;

  //------------------------------------------------------------------
  // Case of identical parts: cannot have the same name; use number of subdaughter
  std::string label;
  char tmp[2];
  sprintf(tmp, "%d", number);
  label = tmp;
  //------------------------------------------------------------------

  // Add all the items
  // index of arrays
  sc = nt->addItem("N_lab"+label,m_n,0,10000);
  sc = nt->addIndexedItem("ID_lab"+label,m_n,m_ID);

#ifdef MCCheck
  sc = nt->addIndexedItem("trueID_lab"+label,m_n,m_trueID);
#endif

  // Delta log-likelihood values (DLL)
  sc = nt->addIndexedItem("dllepi_lab"+label,m_n,m_dllepi); // dll(e-pi)
  sc = nt->addIndexedItem("dllmupi_lab"+label,m_n,m_dllmupi); // dll(mu-pi)
  sc = nt->addIndexedItem("dllkpi_lab"+label,m_n,m_dllkpi); // dll(K-pi)
  sc = nt->addIndexedItem("dllppi_lab"+label,m_n,m_dllppi); // dll(p-pi)
  
  // Global Rich PIDs
  sc = nt->addIndexedItem("globdllpi_lab"+label,m_n,m_globdllpi); // dll pion
  sc = nt->addIndexedItem("globdllk_lab"+label,m_n,m_globdllk); // dll kaon

  // momentum MeV
  sc = nt->addIndexedItem("p_lab"+label,m_n,m_p);
  // transverse momentum MeV
  sc = nt->addIndexedItem("pt_lab"+label,m_n,m_pt);
  sc = nt->addIndexedItem("mass_lab"+label,m_n,m_mass);
  // 3-momentum
  sc = nt->addIndexedItem("px_lab"+label,m_n,m_px);
  sc = nt->addIndexedItem("py_lab"+label,m_n,m_py);
  sc = nt->addIndexedItem("pz_lab"+label,m_n,m_pz);
  // sc = nt->addIndexedItem("pxvar_lab"+label,m_n,m_pxvar);
  // sc = nt->addIndexedItem("pyvar_lab"+label,m_n,m_pyvar);
  // sc = nt->addIndexedItem("pzvar_lab"+label,m_n,m_pzvar);

  // Decay vtx (and variances) if any; otherwise set to -100000
  sc = nt->addIndexedItem("vchitwo_lab"+label,m_n,m_vchitwo);
  sc = nt->addIndexedItem("vnDoF_lab"+label,m_n,m_vnDoF);
  sc = nt->addIndexedItem("vx_lab"+label,m_n,m_vx);
  sc = nt->addIndexedItem("vy_lab"+label,m_n,m_vy);
  sc = nt->addIndexedItem("vz_lab"+label,m_n,m_vz);
  // sc = nt->addIndexedItem("vxvar_lab"+label,m_n,m_vxvar);
  // sc = nt->addIndexedItem("vyvar_lab"+label,m_n,m_vyvar);
  sc = nt->addIndexedItem("vzvar_lab"+label,m_n,m_vzvar);

  // Primary vertices

  // PV w.r.t. the particle has the smallest IPS    
  sc = nt->addIndexedItem("chitwosIPSPV_lab"+label,m_n,m_chitwosIPSPV);
  sc = nt->addIndexedItem("sIPSPVx_lab"+label,m_n,m_sIPSPVx);
  sc = nt->addIndexedItem("sIPSPVy_lab"+label,m_n,m_sIPSPVy);
  sc = nt->addIndexedItem("sIPSPVz_lab"+label,m_n,m_sIPSPVz);
  // sc = nt->addIndexedItem("sIPSPVxvar_lab"+label,m_n,m_sIPSPVxvar);
  // sc = nt->addIndexedItem("sIPSPVyvar_lab"+label,m_n,m_sIPSPVyvar);
  sc = nt->addIndexedItem("sIPSPVzvar_lab"+label,m_n,m_sIPSPVzvar);
  // Minimum (smallest) IP, IPe and IPS
  sc = nt->addIndexedItem("sIP_lab"+label,m_n,m_sIP);
  // sc = nt->addIndexedItem("sIPe_lab"+label,m_n,m_sIPe);
  sc = nt->addIndexedItem("sIPS_lab"+label,m_n,m_sIPS);

  // PV w.r.t. the particle has the largest IPS    
  sc = nt->addIndexedItem("chitwolIPSPV_lab"+label,m_n,m_chitwolIPSPV);
  sc = nt->addIndexedItem("lIPSPVx_lab"+label,m_n,m_lIPSPVx);
  sc = nt->addIndexedItem("lIPSPVy_lab"+label,m_n,m_lIPSPVy);
  sc = nt->addIndexedItem("lIPSPVz_lab"+label,m_n,m_lIPSPVz);
  // sc = nt->addIndexedItem("lIPSPVxvar_lab"+label,m_n,m_lIPSPVxvar);
  // sc = nt->addIndexedItem("lIPSPVyvar_lab"+label,m_n,m_lIPSPVyvar);
  // sc = nt->addIndexedItem("lIPSPVzvar_lab"+label,m_n,m_lIPSPVzvar);
  // Maximum (largest) IP, IPe and IPS
  sc = nt->addIndexedItem("lIP_lab"+label,m_n,m_lIP);
  // sc = nt->addIndexedItem("lIPe_lab"+label,m_n,m_lIPe);
  sc = nt->addIndexedItem("lIPS_lab"+label,m_n,m_lIPS);

  // Unsigned flight distance/significance (sec-prim) if composite
  // w.r.t. the PV for which the particle has the smallest IPS
  sc = nt->addIndexedItem("FD_lab"+label,m_n,m_FD);
  sc = nt->addIndexedItem("FS_lab"+label,m_n,m_FS);

  // Cosine of the angle between the particle momentum (p)
  // and the flight distance (F) w.r.t the PV for which the 
  // IPS of the composite particle is the smallest
  sc = nt->addIndexedItem("cospF_lab"+label,m_n,m_cospF);

  // Lifetime
  // sc = nt->addIndexedItem("taufit_lab"+label,m_n,m_taufit);
  // sc = nt->addIndexedItem("taufitErr_lab"+label,m_n,m_taufitErr);
  // sc = nt->addIndexedItem("ctfitChi2_lab"+label,m_n,m_ctfitChi2);

  // State vector of the track (x,y,tx,ty,Q/P), tx = dx/dz, ty = dy/dz
  sc = nt->addIndexedItem("stateX_lab"+label,m_n,m_stateX);
  sc = nt->addIndexedItem("stateXErr_lab"+label,m_n,m_stateXErr);
  sc = nt->addIndexedItem("stateY_lab"+label,m_n,m_stateY);
  sc = nt->addIndexedItem("stateYErr_lab"+label,m_n,m_stateYErr);
  sc = nt->addIndexedItem("stateTX_lab"+label,m_n,m_stateTX);
  sc = nt->addIndexedItem("stateTXErr_lab"+label,m_n,m_stateTXErr);
  sc = nt->addIndexedItem("stateTY_lab"+label,m_n,m_stateTY);
  sc = nt->addIndexedItem("stateTYErr_lab"+label,m_n,m_stateTYErr);
  sc = nt->addIndexedItem("stateQoverP_lab"+label,m_n,m_stateQoverP);
  sc = nt->addIndexedItem("stateQoverPErr_lab"+label,m_n,m_stateQoverPErr);
  
  // sc = nt->addIndexedItem("_lab"+label,m_n,m_);

#ifdef MCCheck
  // Look if a final state is reconstructed and is signal (only meaningful for final tracks)
  sc = nt->addIndexedItem("Sig_lab"+label,m_n,m_Sig);

  // Extrapolated state vector of the associated MCParticle track (x,y,tx,ty,Q/P), tx = dx/dz, ty = dy/dz
  sc = nt->addIndexedItem("MCstateX_lab"+label,m_n,m_MCstateX);
  sc = nt->addIndexedItem("MCstateY_lab"+label,m_n,m_MCstateY);
  sc = nt->addIndexedItem("MCstateTX_lab"+label,m_n,m_MCstateTX);
  sc = nt->addIndexedItem("MCstateTY_lab"+label,m_n,m_MCstateTY);
  sc = nt->addIndexedItem("MCstateQoverP_lab"+label,m_n,m_MCstateQoverP);

#endif

#ifdef MCCheck
  // The true decay variables

  // Add all the items
  // index of arrays
  sc = nt->addItem("mcN_lab"+label,m_mcn,0,10000);
  sc = nt->addIndexedItem("mcID_lab"+label,m_mcn,m_mcID);

  // momentum MeV
  sc = nt->addIndexedItem("mcp_lab"+label,m_mcn,m_mcp);
  // transverse momentum MeV
  sc = nt->addIndexedItem("mcpt_lab"+label,m_mcn,m_mcpt);
  sc = nt->addIndexedItem("mcmass_lab"+label,m_mcn,m_mcmass);
  // 3-momentum
  sc = nt->addIndexedItem("mcpx_lab"+label,m_mcn,m_mcpx);
  sc = nt->addIndexedItem("mcpy_lab"+label,m_mcn,m_mcpy);
  sc = nt->addIndexedItem("mcpz_lab"+label,m_mcn,m_mcpz);
  // Primary vertex
  sc = nt->addIndexedItem("mcxPV_lab"+label,m_mcn,m_mcxPV);
  sc = nt->addIndexedItem("mcyPV_lab"+label,m_mcn,m_mcyPV);
  sc = nt->addIndexedItem("mczPV_lab"+label,m_mcn,m_mczPV);
  // Origin vertex
  sc = nt->addIndexedItem("mcxOriVtx_lab"+label,m_mcn,m_mcxOriVtx);
  sc = nt->addIndexedItem("mcyOriVtx_lab"+label,m_mcn,m_mcyOriVtx);
  sc = nt->addIndexedItem("mczOriVtx_lab"+label,m_mcn,m_mczOriVtx);
  // Decay vertex
  sc = nt->addIndexedItem("mcxDKVtx_lab"+label,m_mcn,m_mcxDKVtx);
  sc = nt->addIndexedItem("mcyDKVtx_lab"+label,m_mcn,m_mcyDKVtx);
  sc = nt->addIndexedItem("mczDKVtx_lab"+label,m_mcn,m_mczDKVtx);
  // True IP, w.r.t the primary vetex
  sc = nt->addIndexedItem("mcIP_lab"+label,m_mcn,m_mcIP);
  // Look if the corresponding final state is reconstructed (only meaningful for final tracks)
  sc = nt->addIndexedItem("isReco_lab"+label,m_mcn,m_isReco);
  // True lifetime (relevant for composite particles) in picoseconds
  // sc = nt->addIndexedItem("mctau_lab"+label,m_mcn,m_mctau);

#endif

}

//=============================================================================
//  DecayChainNTuple::HandleNTuple::FillNTuple
//=============================================================================
#ifndef MCCheck
void DecayChainNTuple::HandleNTuple::FillNTuple(Particle& part, VertexVector& pvs, RichPIDs* globalPIDs)
#endif

#ifdef MCCheck
  void DecayChainNTuple::HandleNTuple::FillNTuple(Particle& part, VertexVector& pvs,
                                                  bool& isSig, MCParticle* mclink, RichPIDs* globalPIDs)
#endif
{
  // std::cout << "ID = " << part.particleID().pid() << std::endl;

  // not to be outside of range ...
  if (m_n>9999) return;

  m_ID[m_n]= part.particleID().pid();

#ifdef MCCheck
  // Look true ID of the track (for PID misidentification)
  if(mclink){
    m_trueID[m_n] = mclink->particleID().pid();
  }
  else{ // not a final state, take particle ID
    m_trueID[m_n] = part.particleID().pid(); 
  }
#endif

  // Find the protoparticle from which the final state particle has been made
  ContainedObject* orig = part.origin();
  const ProtoParticle* pporig = dynamic_cast<const ProtoParticle*>(orig);

  // Find the TrgTrack from which the final state particle has been made
  const TrgTrack* TrgT = dynamic_cast<const TrgTrack*>(part.origin());

  // x,y,z-positions of:
  // - TrStateP closest to IP for protoparticle
  // - first measured hit of the track for TrgTrack
  double xTrack = 0.0;
  double yTrack = 0.0;
  double zTrack = 0.0;
  
  // set some default values
  m_dllepi[m_n] = -999.;
  m_dllmupi[m_n] = -999.;
  m_dllkpi[m_n] = -999.;
  m_dllppi[m_n] = -999.;
  
  // Global Rich PIDs
  m_globdllpi[m_n] = -999.;
  m_globdllk[m_n] = -999.; 
  
  m_stateX[m_n] = -999.;
  m_stateXErr[m_n] = -999.;
  m_stateY[m_n] = -999.;
  m_stateYErr[m_n] = -999.;
  m_stateTX[m_n] = -999.;
  m_stateTXErr[m_n] = -999.;
  m_stateTY[m_n] = -999.;
  m_stateTYErr[m_n] = -999.;
  m_stateQoverP[m_n] = -999.;
  m_stateQoverPErr[m_n] = -999.;
  
#ifdef MCCheck
  m_MCstateX[m_n] = -999.;
  m_MCstateY[m_n] = -999.;
  m_MCstateTX[m_n] = -999.;
  m_MCstateTY[m_n] = -999.;
  m_MCstateQoverP[m_n] = -999.;
#endif

  if(pporig){ // if there is a protoparticle

    // Retrieve Delta log-likelihood values
    // dll(e-pi)
    m_dllepi[m_n] = pporig->detPIDvalue( ProtoParticle::LkhPIDe );
    // dll(mu-pi)
    m_dllmupi[m_n] = pporig->detPIDvalue( ProtoParticle::LkhPIDmu );
    // dll(K-pi)
    m_dllkpi[m_n] = pporig->detPIDvalue( ProtoParticle::LkhPIDK );
    // dll(p-pi)
    m_dllppi[m_n] = pporig->detPIDvalue( ProtoParticle::LkhPIDp );

    // Error**2 = Variance = sigma**2

    // state vector TrStateP closest to IP
    const TrStateP* ClosestTrStateP = pporig->trStateP();

    if(ClosestTrStateP){
      xTrack = ClosestTrStateP->x();
      yTrack = ClosestTrStateP->y();
      zTrack = ClosestTrStateP->z();
      
      m_stateX[m_n] = ClosestTrStateP->x();
      m_stateXErr[m_n] = sqrt(ClosestTrStateP->eX2());
      m_stateY[m_n] = ClosestTrStateP->y();
      m_stateYErr[m_n] = sqrt(ClosestTrStateP->eY2());
      m_stateTX[m_n] = ClosestTrStateP->tx();
      m_stateTXErr[m_n] = sqrt(ClosestTrStateP->eTx2());
      m_stateTY[m_n] = ClosestTrStateP->ty();
      m_stateTYErr[m_n] = sqrt(ClosestTrStateP->eTy2());
      m_stateQoverP[m_n] = ClosestTrStateP->qDivP();
      m_stateQoverPErr[m_n] = sqrt(ClosestTrStateP->eQdivP2());
    }

  }
  else if (TrgT){ // the particle is a Trg track

    // Global Rich PIDs
    if(globalPIDs){
      RichPID* globalpid = globalPIDs->object(TrgT->key());    
      if(globalpid){

        /*
        std::cout << " DLLPion = "
                  << globalpid->particleDeltaLL(Rich::Pion)
                  << " DLLKaon = "
                  << globalpid->particleDeltaLL(Rich::Kaon)
                  << std::endl;
        */

        m_globdllpi[m_n] = globalpid->particleDeltaLL(Rich::Pion);
        m_globdllk[m_n] = globalpid->particleDeltaLL(Rich::Kaon);
      }
    }
    
    // state closest to track's origin
    const TrgState FirstTrgState = TrgT->firstState();

    xTrack = FirstTrgState.x();
    yTrack = FirstTrgState.y();
    zTrack = FirstTrgState.z();
    
    m_stateX[m_n] = FirstTrgState.x();
    m_stateXErr[m_n] = sqrt(FirstTrgState.errX2());
    m_stateY[m_n] = FirstTrgState.y();
    m_stateYErr[m_n] = sqrt(FirstTrgState.errY2());
    m_stateTX[m_n] = FirstTrgState.xSlope();
    m_stateTXErr[m_n] = sqrt(FirstTrgState.errTx2());
    m_stateTY[m_n] = FirstTrgState.ySlope();
    m_stateTYErr[m_n] = sqrt(FirstTrgState.errTy2());
    m_stateQoverP[m_n] = FirstTrgState.qOverP();
    m_stateQoverPErr[m_n] = sqrt(FirstTrgState.errQOverP2());

  }
  
#ifdef MCCheck
  // Compute the extrapolated state vector of the associated MCParticle (x,y,tx,ty,Q/P), tx = dx/dz, ty = dy/dz
  if(isSig){

    int Q = (mclink->particleID().pid()) > 0 ? 1 : -1;
    
    MCVertex* oVtxMCLink= mclink->originVertex();
    HepPoint3D& oVtxMCLinkPos = oVtxMCLink->position();

    double MCx0 = oVtxMCLinkPos.x();
    double MCy0 = oVtxMCLinkPos.y();
    double MCz0 = oVtxMCLinkPos.z();
    
    // std::cout << "--> Original true position [mm] x = " << MCx0
    //       << " , y = " << MCy0 << " , z = " << MCz0 << std::endl;

    // std::cout << "--> Closest to origin position of the track [mm] x = " << xTrack 
    //          << " , y = " << yTrack << " , z = " << zTrack << std::endl;

    // tx slope
    double MCstateTX = (mclink->momentum().px())/(mclink->momentum().pz());
    // ty slope
    double MCstateTY = (mclink->momentum().py())/(mclink->momentum().pz());
    
    // xz-projection    
    double MCstateX = (MCx0 + ((zTrack - MCz0)* MCstateTX));
    // yz-projection    
    double MCstateY = (MCy0 + ((zTrack - MCz0)* MCstateTY));
    
    // std::cout << "--> Extrapolated true position [mm] x = " << MCstateX
    //          << " , y = " << MCstateY << " , z = " << zTrack << std::endl;
    
    double MCstateQoverP = Q/mclink->momentum().vect().mag();
    
    m_MCstateX[m_n] = MCstateX;
    m_MCstateY[m_n] = MCstateY;
    m_MCstateTX[m_n] = MCstateTX;
    m_MCstateTY[m_n] = MCstateTY;
    m_MCstateQoverP[m_n] = MCstateQoverP;
    
  }
#endif

  // Momentum, mass, ...
  m_p[m_n]= part.p();
  m_pt[m_n]= part.pt();
  m_mass[m_n]= part.mass();
  // 3-momentum
  m_px[m_n]= part.momentum().px();
  m_py[m_n]= part.momentum().py();
  m_pz[m_n]= part.momentum().pz();
  // m_pxvar[m_n]= part.momentumErr()(1,1);
  // m_pyvar[m_n]= part.momentumErr()(2,2);
  // m_pzvar[m_n]= part.momentumErr()(3,3);
  
  // Decay vtx if any; otherwise set to -100000
  Vertex* v = part.endVertex();
  if(!v){
    m_vchitwo[m_n]=-1.;
    m_vnDoF[m_n]=-1.;
    m_vx[m_n] = -100000.;
    m_vy[m_n] = -100000.;
    m_vz[m_n] = -100000.;
    // m_vxvar[m_n] = -1.;
    // m_vyvar[m_n] = -1.;
    m_vzvar[m_n] = -1.;
  } 
  else{
    m_vchitwo[m_n] = v->chi2();
    m_vnDoF[m_n]= v->nDoF();
    m_vx[m_n] = v->position().x();
    m_vy[m_n] = v->position().y();
    m_vz[m_n] = v->position().z();
    // m_vxvar[m_n] = v->positionErr()(1,1);
    // m_vyvar[m_n] = v->positionErr()(2,2);
    m_vzvar[m_n] = v->positionErr()(3,3);
  }

  // PV w.r.t. which the particle has the smallest IPS
  Vertex* sIPSPV = 0;
  double normIPSMin = -1.;

  // PV w.r.t. which the particle has the largest IPS
  Vertex* lIPSPV = 0;
  double normIPSMax = -1.;

  Vertices::iterator ivert;
  // std::cout << "Looping on PV " << pvs.size() << std::endl  ;
  for( ivert = pvs.begin(); ivert != pvs.end(); ivert++){
    // std::cout << "In Loop" << std::endl  ;

    double ip, ipe;
    double normIPS;

    // Find the PV w.r.t. which the particle has the smallest IPS
    Vertex* sipsPV = *ivert;

    // std::cout << "IP tool" << std::endl  ;
    m_iptool->calcImpactPar(part,*sipsPV,ip,ipe);
    // std::cout << "IP tool done" << std::endl  ;
    normIPS = ip/ipe;

    if(normIPSMin<0||normIPS<normIPSMin) {
      normIPSMin=normIPS;
      sIPSPV=sipsPV;
    }

    // Find the PV w.r.t. which the particle has the largest IPS
    Vertex* lipsPV = *ivert;

    m_iptool->calcImpactPar(part,*lipsPV,ip,ipe);
    normIPS = ip/ipe;

    if(normIPSMax<0||normIPS>normIPSMax) {
      normIPSMax=normIPS;
      lIPSPV=lipsPV;
    }
  }// for ivert
  // std::cout << "Looping done" << std::endl  ;

  double ipcheck, ipecheck; // for ip filling

  // PV w.r.t. the particle has the smallest IPS    
  const HepPoint3D& sPV = sIPSPV->position();
  m_chitwosIPSPV[m_n] = sIPSPV->chi2();
  m_sIPSPVx[m_n] = sPV.x();
  m_sIPSPVy[m_n] = sPV.y();
  m_sIPSPVz[m_n] = sPV.z();
  // m_sIPSPVxvar[m_n] = sIPSPV->positionErr()(1,1);
  // m_sIPSPVyvar[m_n] = sIPSPV->positionErr()(2,2);
  m_sIPSPVzvar[m_n] = sIPSPV->positionErr()(3,3);

  m_iptool->calcImpactPar(part,*sIPSPV,ipcheck,ipecheck);

  // Minimum (smallest) IP, IPe and IPS
  m_sIP[m_n] = ipcheck;
  // m_sIPe[m_n] = ipecheck ;
  m_sIPS[m_n] = (ipcheck/ipecheck);

  // PV w.r.t. the particle has the largest IPS    
  const HepPoint3D& lPV = lIPSPV->position();
  m_chitwolIPSPV[m_n] = lIPSPV->chi2();
  m_lIPSPVx[m_n] = lPV.x();
  m_lIPSPVy[m_n] = lPV.y();
  m_lIPSPVz[m_n] = lPV.z();
  // m_lIPSPVxvar[m_n] = lIPSPV->positionErr()(1,1);
  // m_lIPSPVyvar[m_n] = lIPSPV->positionErr()(2,2);
  // m_lIPSPVzvar[m_n] = lIPSPV->positionErr()(3,3);

  // std::cout << "IP" << std::endl  ;
  m_iptool->calcImpactPar(part,*lIPSPV,ipcheck,ipecheck);

  // Maximum (largest) IP, IPe and IPS
  m_lIP[m_n] = ipcheck;
  // m_lIPe[m_n] = ipecheck ;
  m_lIPS[m_n] = (ipcheck/ipecheck);

  // Unsigned flight distance/significance (sec-prim) if composite
  // w.r.t. the PV for which the particle has the smallest IPS
  if(!v){ // no sec vtx  
    m_FD[m_n] = -999.;
    m_FS[m_n] = -999.;
  }
  else{
    double f = -1. , fe = -1.;
    m_iptool->calcVertexDis(*sIPSPV, *v, f, fe);
    double fs = fabs(f/fe);

    m_FD[m_n] = f;
    m_FS[m_n] = fs;
  }

  // Cosine of the angle between the particle momentum (p)
  // and the flight distance (F) w.r.t the PV for which the 
  // IPS of the composite particle is the smallest  
  if(!v){ // no sec vtx  
    m_cospF[m_n] = -2.;
  }
  else{
    const HepPoint3D x = v->position();
    const HepPoint3D TMP = x - sPV;
    const Hep3Vector dist(TMP.x(),TMP.y(),TMP.z());
    const Hep3Vector p(part.momentum().vect());
    double cosangle = p.dot(dist)/p.mag()/dist.mag();
    m_cospF[m_n] = cosangle;
  }

  // Lifetime
  /*
  double ct, ctErr, ctChi2;
  
  if(!v){ // no sec vtx
    m_taufit[m_n] = -999.;
    m_taufitErr[m_n] = -999.;
    m_ctfitChi2[m_n] = -999.;
  }
  else{
    m_lifetimefitter->fit(*sIPSPV, part, ct, ctErr, ctChi2);
    
    // tau in ps
    m_taufit[m_n] = 1000 * (((ct/mm) /c_light)/ns);
    m_taufitErr[m_n] = 1000 * (((ctErr/mm) /c_light)/ns) ;
    m_ctfitChi2[m_n] = ctChi2;
  }
  */

#ifdef MCCheck
  // Look if a final state is reconstructed (only meaningful for final tracks)
  m_Sig[m_n] = isSig;
#endif

  m_n++;

  // std::cout << "Done" << std::endl;
}

//=============================================================================
//  Book the ntuple
//=============================================================================
StatusCode DecayChainNTuple::BookNTuple(std::vector<Particle*>& mothervec) {
  
  verbose() << "Entering BookNTuple" << endreq;
  StatusCode sc = StatusCode::SUCCESS;
  
  // Mother (or head) of the decay
  std::vector<Particle*>::iterator imother;

  // Loop over all the mothers
  for(imother = mothervec.begin();
      imother != mothervec.end();
      imother++){

    debug() << "Mother ID = " << (*imother)->particleID().pid() << endreq;
    
    // Get all the decay members (should be flagged)
    std::vector<Particle*> Children;
    std::vector<Particle*>::iterator ichild;
    m_pDKFinder->decayMembers(*imother, Children);

    verbose() << "Number of children found is = " << Children.size() << endreq;

    NTuplePtr nt(ntupleSvc(), m_ntupleName);
    if(!nt){ // Check if already booked
      nt = ntupleSvc()->book(m_ntupleName, CLID_ColumnWiseTuple, "Decay Tree Ntuple");
      if(nt){  // ntuple sucessfully booked
          
        m_bookedNTuple = true;

        // NTuple global variables
        sc = nt->addItem("Event", m_eventNumber);
        sc = nt->addItem("Run", m_runNumber);
        sc = nt->addItem("L0Decision", m_L0Decision);
        sc = nt->addItem("L1Decision", m_L1Decision);
        sc = nt->addItem("L1Gen", m_L1Gen);
        sc = nt->addItem("L1SiMu", m_L1SiMu);
        sc = nt->addItem("L1DiMu", m_L1DiMu);
        sc = nt->addItem("L1JPsi", m_L1JPsi);
        sc = nt->addItem("L1Elec", m_L1Elec);
        sc = nt->addItem("L1Phot", m_L1Phot);

        // Part of the keys and labels
        int forthekeymother = 0;
        // just label the head with 0
        unsigned int mothernr = 0;
        int forthekeydau = 0;
        unsigned int subdaunr = 1;
          
        //---------------------------------------------
        // Book the mother
        int pidmother = (*imother)->particleID().pid();
        ParticleProperty* ppmother =  m_ppSvc->findByStdHepID(pidmother);
        info() << "Booking ParticleName (mother) " << ppmother->particle() << endreq;
          
        const std::string pnamemother = ppmother->particle();

        // Case of identical parts: take arbitrary offset and add (500000*forthekeymother)
        int offsetmother = 9;
        offsetmother++;

        forthekeymother++;

        int key =  ((offsetmother + (500000*forthekeymother)));
        HandleNTuple* add = new HandleNTuple(nt, mothernr, //m_pLifetimeFitter, 
                                             m_IPTool);
        m_HandleNTupleMap.insert(std::make_pair(key, add));

        verbose() << "Added " << pnamemother << " with key " << key
                << " and address " << add << endreq;
        mothernr++;
        //---------------------------------------------

        //---------------------------------------------
        // Book the children
        for (ichild = Children.begin() ; ichild != Children.end() ; ichild++){
          debug() << "Child has ID = " 
                  << (*ichild)->particleID().pid() << endreq;
            
          int pid = (*ichild)->particleID().pid();
          // int abspid = (*ichild)->particleID().abspid();
          ParticleProperty* pp =  m_ppSvc->findByStdHepID(pid);
          info() << "Booking Subdaughter number = " << subdaunr
                 << " , ParticleName " << pp->particle() << endreq;
            
          const std::string pname = pp->particle();
          // Case of identical parts: take arbitrary offset and add (500000*forthekeydau)
          int offsetdau = 99;
          offsetdau++;
          
          forthekeydau++;

          int keydau =  ((offsetdau + (500000*forthekeydau)));           
          HandleNTuple* adddau = new HandleNTuple(nt, subdaunr,   // m_pLifetimeFitter,
                                                  m_IPTool);
          m_HandleNTupleMap.insert(std::make_pair(keydau, adddau));
          
          verbose() << "Added " << pname << " with key " << keydau
                  << " and address " << adddau << endreq;
          subdaunr++;
        }// ichild
      }// if(nt)
      else{// did not manage to book the ntuple....
        err() << "Failed to BookNTuple" << endreq;
        return StatusCode::FAILURE;
      } //else
    } // if(!nt)
    debug() << "NTuple is booked" << endreq;

  } // imother
  
  verbose() << "Leaving BookNTuple" << endreq;
  
  return sc;
}


//=============================================================================
//  Write the ntuple
//=============================================================================
StatusCode DecayChainNTuple::WriteNTuple(std::vector<Particle*>& mothervec) {

  verbose() << "Entering WriteNTuple" << endreq;
  StatusCode sc = StatusCode::SUCCESS;

  //---------------------------------------------
  VertexVector PVs;
  sc = getPV(PVs);
  if (!sc.isSuccess()) return sc;
  if (PVs.empty()) return StatusCode::SUCCESS; // don't continue, but not an error
  //---------------------------------------------

#ifdef MCCheck
  //---------------------------------------------
  // Get the MCparticles
  SmartDataPtr<MCParticles> kmcparts(m_EDS, MCParticleLocation::Default );
  if( !kmcparts ){
    fatal() << "Unable to find MC particles at '" << MCParticleLocation::Default << "'" << endreq;
    return false;
  }
  //---------------------------------------------

  //---------------------------------------------
  std::vector<MCParticle*> MCHead;
  const MCParticle* imc = NULL;
  while (m_pMCDKFinder->findDecay(kmcparts, imc)){
    MCParticle* jmc = const_cast<MCParticle*>(imc);
    MCHead.push_back(jmc);
  }
  //---------------------------------------------  
#endif

  //---------------------------------------------  
  RichPIDs* globalPIDs = NULL;
  if (m_useRichPID){
    globalPIDs = get<RichPIDs>( m_richPIDLocation );
    if(globalPIDs){
      debug() << "Found " << globalPIDs->size() << " RichPIDs at " << m_richPIDLocation << endreq;
    }
  }
  //---------------------------------------------  

  //---------------------------------------------  
  // Trigger information
  SmartDataPtr<TrgDecision> trg(eventSvc(), m_pathTrg);
  if( 0 == trg ) {
    err() << "Unable to find Trigger at " << m_pathTrg << endreq;
    return StatusCode::FAILURE;
  }
  bool L0Decision = false;
  if (trg){
    L0Decision = trg->L0();
    debug() << "L0 decision: " << L0Decision << endreq;
  }

  // L1 Trigger informations
  bool L1Decision = false;
  bool L1Gen      = false;
  bool L1SiMu     = false;
  bool L1DiMu     = false;
  bool L1JPsi     = false;
  bool L1Elec     = false;
  bool L1Phot     = false;
  
  if(m_dataProvider){
    L1Score* score = m_dataProvider->l1Score();
    if(score){
      L1Decision = score->decision();
      L1Gen      = score->decisionGen();
      L1SiMu     = score->decisionMu();
      L1DiMu     = score->decisionDiMu();
      L1JPsi     = score->decisionDiMuJPsi();
      L1Elec     = score->decisionElec();
      L1Phot     = score->decisionPhot();
 
      debug() << "L1 trigger summary: "        << endreq;
      debug() << " Generic:     " << L1Gen      << endreq;
      debug() << " Single Muon: " << L1SiMu     << endreq;
      debug() << " Dimuon:      " << L1DiMu     << endreq;
      debug() << " JPsi:        " << L1JPsi     << endreq;
      debug() << " Electron:    " << L1Elec     << endreq;
      debug() << " Photon:      " << L1Phot     << endreq;
      debug() << "----------------------------" << endreq;
      debug() << "Total:       " << L1Decision << endreq;
    }
  }
  //---------------------------------------------  

  //---------------------------------------------  
  // Fill NTuple global variables
  m_eventNumber = m_event;
  m_runNumber = m_run;
  m_L0Decision = long(L0Decision);
  m_L1Decision = long(L1Decision);
  m_L1Gen = long(L1Gen);
  m_L1SiMu = long(L1SiMu);
  m_L1DiMu = long(L1DiMu);
  m_L1JPsi = long(L1JPsi);
  m_L1Elec = long(L1Elec);
  m_L1Phot = long(L1Phot);

  //---------------------------------------------  

  //---------------------------------------------
  // Mother (or head) of the decay
  std::vector<Particle*>::iterator imother;

  // Loop over all the mothers
  for(imother = mothervec.begin();
      imother != mothervec.end();
      imother++){

    debug() << "Mother ID = " << (*imother)->particleID().pid() << endreq;
    
    // Get all the decay members (should be flagged)
    std::vector<Particle*> Children;
    std::vector<Particle*>::iterator ichild;
    m_pDKFinder->decayMembers(*imother, Children);

    verbose() << "Number of children found is = " << Children.size() << endreq;

    //------------------------------------------------------------------------------------------
    // Filling the NTuple
    //------------------------------------------------------------------------------------------

    // Part of the keys and labels
    int forthekeymother = 0;
    int forthekeydau = 0;
    unsigned int subdaunr = 1;
      
    //---------------------------------------------
    // Fill the mother
    int pidmother = (*imother)->particleID().pid();
    ParticleProperty* ppmother =  m_ppSvc->findByStdHepID(pidmother);
    debug() << "Filling ParticleName (mother) " << ppmother->particle() << endreq;

    // Case of identical parts: take arbitrary offset and add (500000*forthekeymother)
    int offsetmother = 9;
    offsetmother++;

    forthekeymother++;

    std::map<int, HandleNTuple*>::iterator jkeymother = m_HandleNTupleMap.find((offsetmother + (500000*forthekeymother)));

#ifndef MCCheck
    if (jkeymother != m_HandleNTupleMap.end()){
      jkeymother->second->FillNTuple(**imother, PVs, globalPIDs);
    }
#endif

#ifdef MCCheck
    // Look if a final state is reconstructed and is signal (only meaningful for final tracks)
    bool isSig = false;
    MCParticle* mclink = 0;
    
    if (jkeymother != m_HandleNTupleMap.end()){
      jkeymother->second->FillNTuple(**imother, PVs, isSig, mclink, globalPIDs);
    }
#endif

    //---------------------------------------------

    //---------------------------------------------
    // Fill the children
    for (ichild = Children.begin() ; ichild != Children.end() ; ichild++){
      
      int pid = (*ichild)->particleID().pid();
      // int abspid = (*ichild)->particleID().abspid();
      ParticleProperty* pp =  m_ppSvc->findByStdHepID(pid);
      debug() << "Filling child number = " << subdaunr
              << " , ParticleName " << pp->particle() << endreq;

      //---------------------------------------------
#ifdef MCCheck
      // Look if a final state is reconstructed and is signal (only meaningful for final tracks)
      isSig = false;

      verbose() << "Looking for link association for particle with ID: "
             << (*ichild)->particleID().pid() << " " << (*ichild)->momentum() << endreq;

      // Corresponding MC particle
      mclink = m_pAsctLinks->associatedFrom(*ichild);

      // Check if the associated MCParticle belongs to the DOI
      isSig = isSignal(mclink, MCHead);

      debug() << "Is the particle reconstructed and signal (1: yes, 0: false)? ==> " << isSig << endreq;

#endif
      //---------------------------------------------
      // Case of identical parts: take arbitrary offset and add (5000000*forthekeydau)
      int offsetdau = 99;
      offsetdau++;

      forthekeydau++;

      std::map<int, HandleNTuple*>::iterator jkeydau = m_HandleNTupleMap.find((offsetdau + (500000*forthekeydau)));
      
#ifndef MCCheck
      if (jkeydau != m_HandleNTupleMap.end()){
        jkeydau->second->FillNTuple(**ichild, PVs, globalPIDs);
      }
#endif

#ifdef MCCheck
      if (jkeydau != m_HandleNTupleMap.end()){
        jkeydau->second->FillNTuple(**ichild, PVs, isSig, mclink, globalPIDs);
      }
#endif

      subdaunr++;
    }// ichild
  } // imother

  verbose() << "Leaving WriteNTuple" << endreq;
  
  return sc;
}

//=============================================================================
//  getPV
//=============================================================================
StatusCode DecayChainNTuple::getPV(VertexVector& PVs) {
  
  verbose() << "Entering getPV" << endreq;
  StatusCode sc = StatusCode::SUCCESS;
 
  Vertices* vertices = get<Vertices>(m_PVContainer);
 
  if ( !vertices ) {
    err() << "Could not find primary vertex location " <<  m_PVContainer << endreq;
    return StatusCode::FAILURE; 
  }

  debug() << "Number of primary vertices  = "
          << vertices->size() << endreq;
  
  Vertices::iterator ivert;
  for( ivert = vertices->begin(); ivert != vertices->end(); ivert++){
    debug() << "Primary vertex coordinates = ( "
           << (*ivert)->position().x()
           << " , " << (*ivert)->position().y()
           << " , " << (*ivert)->position().z() << " ) and chi2 = " 
            << (*ivert)->chi2() << " with nDoF " << (*ivert)->nDoF() << endreq;
    PVs.push_back((*ivert));
  }

  verbose() << "Leaving getPV" << endreq;

  if ( PVs.empty()) warning() << "No primary vertex found at " << m_PVContainer << endreq ;
 
  return sc;
}

#ifdef MCCheck
//=============================================================================
//  isSignal: looks if the associated MCParticle belongs to signal decay
//=============================================================================
bool DecayChainNTuple::isSignal(MCParticle* MC, std::vector<MCParticle*>& MCHead){

  verbose() << "Entering isSignal" << endreq;

  bool issig = false;
  
  if(MC != NULL){
    verbose() << "There is a link " << endreq;
    std::vector<MCParticle*>::const_iterator imchead;

    // For each true decay
    for(imchead = MCHead.begin(); imchead != MCHead.end(); ++imchead){

      // list of the true decay daughters
      std::vector<MCParticle*> mclist;
      const MCParticle* mc = *imchead;
      // final states must be flagged
      m_pMCDKFinder->decayMembers(mc, mclist);

      // loop over list of true decay daughters
      std::vector<MCParticle*>::iterator mcdaughter ; 
      for (mcdaughter= mclist.begin() ; mcdaughter != mclist.end(); ++mcdaughter){
        
        // Check if the mc associated particle is in the list of mc parts of the true decay
        if(MC == *mcdaughter){
          issig = true;
        } // if mc associated particle is in the true list
      } // for each true daughter
    } // for each true decay
  } // if there is a link associated MCParticle
  
  verbose() << "Leaving isSignal" << endreq;

  return issig;
}
#endif


#ifdef MCCheck
//=============================================================================
//  Write true decay part
//=============================================================================
StatusCode DecayChainNTuple::WriteMCNTuple(std::vector<MCParticle*>& MCHead) {

  verbose() << "Entering WriteMCNTuple" << endreq;
  StatusCode sc = StatusCode::SUCCESS;

  //---------------------------------------------
  // Mother (or head) of the decay
  std::vector<MCParticle*>::iterator imcmother;

  // Loop over all the mothers
  for(imcmother = MCHead.begin();
      imcmother != MCHead.end();
      imcmother++){

    debug() << "Truth: Mother ID = " << (*imcmother)->particleID().pid() << endreq;

    // The MC primary vertex is the origin vertex of the B, but not for prompt J/Psi
    // Get the MC PV coordinates from the Collision
    // -> check the MCVertexType: 1 = ppCollision, 2 = decay

    MCVertex* headOriVtx = (*imcmother)->originVertex();

    debug() << "Head origin vtx [mm] x = "
              << headOriVtx->position().x()
              << " , y = " << headOriVtx->position().y()
              << " , z = " << headOriVtx->position().z()
              << endreq;

    MCVertex::MCVertexType MCPVType = headOriVtx->type();

    if(MCPVType == 1){
      debug() << "The head originates from a MC primary vertex" << endreq;
    }
    else{
      debug() << "The head does not come from a MC primary vertex" << endreq;
    }

    Collision* headCollision = headOriVtx->collision();
    HepPoint3D& MCPVPosition = headCollision->primVtxPosition();
 
    debug() << "Head collision vtx [mm] x = "
            << MCPVPosition.x()
            << " , y = " << MCPVPosition.y()
            << " , z = " << MCPVPosition.z()
            << endreq;
 
    // Get all the decay members (should be flagged)
    std::vector<MCParticle*> MCChildren;
    std::vector<MCParticle*>::iterator imcchild;
    m_pMCDKFinder->decayMembers(*imcmother, MCChildren);

    debug() << "Truth: Number of children found is = " << MCChildren.size() << endreq;

    //------------------------------------------------------------------------------------------
    // Filling the NTuple
    //------------------------------------------------------------------------------------------

    // Part of the keys and labels
    int forthekeymother = 0;
    int forthekeydau = 0;
    unsigned int subdaunr = 1;
   
    //---------------------------------------------
    // Fill the mother
    int pidmother = (*imcmother)->particleID().pid();
    // int abspidmother = (*imcmother)->particleID().abspid();
    ParticleProperty* ppmother =  m_ppSvc->findByStdHepID(pidmother);
    debug() << "Truth: Filling ParticleName (mother) " << ppmother->particle() << endreq;

    // Case of identical parts: take arbitrary offset and add (500000*forthekeymother)
    int offsetmother = 9;
    offsetmother++;

    forthekeymother++;

    std::map<int, HandleNTuple*>::iterator jkeymother = m_HandleNTupleMap.find((offsetmother + (500000*forthekeymother)));

    // Look if the corresponding final state is reconstructed (only meaningful for final tracks)
    bool isReco = false;

    if (jkeymother != m_HandleNTupleMap.end()){
      jkeymother->second->FillMCNTuple(**imcmother, MCPVPosition, isReco);
    }

    //---------------------------------------------

    //---------------------------------------------
    // Fill the children
    for (imcchild = MCChildren.begin() ; imcchild != MCChildren.end() ; imcchild++){
   
      int pid = (*imcchild)->particleID().pid();
      // int abspid = (*imcchild)->particleID().abspid();
      ParticleProperty* pp =  m_ppSvc->findByStdHepID(pid);
      debug() << "Truth: Filling child number = " << subdaunr
              << " , ParticleName " << pp->particle() << endreq;

      //---------------------------------------------
      // Look if the corresponding final state is reconstructed (only meaningful for final tracks)
      Particle* reco = 0;
      isReco = false;
   
      verbose() << "Looking for link association for MC particle with ID: "
             << (*imcchild)->particleID().pid() << " " << (*imcchild)->momentum() << endreq;

      // Corresponding particle
      reco = m_pAsctLinks->associatedTo(*imcchild);
   
      if(reco){
        isReco = true;
      }

      debug() << "Is the MC particle reconstructed (1: yes, 0: false)? ==> " << isReco << endreq;

      // Check if the track is reconstructible
      MCTrackInfo trInfo(eventSvc(), msgSvc());
      verbose() << "MCTrackInfo = " << trInfo.fullInfo(*imcchild) << endreq;

      debug() << "MCTrackInfo: hasVelo = " << trInfo.hasVelo(*imcchild) << endreq;
      debug() << "MCTrackInfo: hasTT = " << trInfo.hasTT(*imcchild) << endreq;
      debug() << "MCTrackInfo: hasT = " << trInfo.hasT(*imcchild) << endreq;

      //---------------------------------------------
      // Case of identical parts: take arbitrary offset and add (500000*forthekeydau)
      int offsetdau = 99;
      offsetdau++;

      forthekeydau++;

      std::map<int, HandleNTuple*>::iterator jkeydau = m_HandleNTupleMap.find((offsetdau + (500000*forthekeydau)));

      if (jkeydau != m_HandleNTupleMap.end()){
        jkeydau->second->FillMCNTuple(**imcchild, MCPVPosition, isReco);
      }

      subdaunr++;
    }// imcchild
  } // imcmother

  verbose() << "Leaving WriteMCNTuple" << endreq;

  return sc;
}
#endif

#ifdef MCCheck
//=============================================================================
//  DecayChainNTuple::HandleNTuple::FillMCNTuple
//=============================================================================
void DecayChainNTuple::HandleNTuple::FillMCNTuple(MCParticle& mcpart, HepPoint3D& MCPVPosition,
                                                    bool& isReco)
{
  // std::cout << "MCID = " << mcpart.particleID().pid() << std::endl;

  // not to be outside of range ...
  if (m_mcn>9999) return;

  m_mcID[m_mcn]= mcpart.particleID().pid();

  // Momentum, mass, ...
  m_mcp[m_mcn]= mcpart.momentum().vect().mag();
  m_mcpt[m_mcn]= mcpart.pt();
  m_mcmass[m_mcn]= mcpart.momentum().m();
  // 3-momentum
  m_mcpx[m_mcn]= mcpart.momentum().px();
  m_mcpy[m_mcn]= mcpart.momentum().py();
  m_mcpz[m_mcn]= mcpart.momentum().pz();

  // Primary vertex
  m_mcxPV[m_mcn] = MCPVPosition.x();
  m_mcyPV[m_mcn] = MCPVPosition.y();
  m_mczPV[m_mcn] = MCPVPosition.z();

  // Origin vertex
  m_mcxOriVtx[m_mcn] = mcpart.originVertex()->position().x();
  m_mcyOriVtx[m_mcn] = mcpart.originVertex()->position().y();
  m_mczOriVtx[m_mcn] = mcpart.originVertex()->position().z();

  // Decay vertex
  SmartRefVector<MCVertex> MCDKVtx = mcpart.endVertices();
  SmartRefVector<MCVertex>::const_iterator ivert = MCDKVtx.begin();

  m_mcxDKVtx[m_mcn] = (*ivert)->position().x();
  m_mcyDKVtx[m_mcn] = (*ivert)->position().y();
  m_mczDKVtx[m_mcn] = (*ivert)->position().z();

  // True IP, w.r.t the primary vertex
  MCVertex* mcpartOrigin = mcpart.originVertex();
  HepVector3D mcpartDirection = mcpart.momentum().vect().unit();
  HepVector3D distance = mcpartOrigin->position() - MCPVPosition;
  HepVector3D ipVector = mcpartDirection.cross(distance.cross(mcpartDirection));
  double mcip = ipVector.mag();
  m_mcIP[m_mcn] = mcip;

  // Look if the corresponding final state is reconstructed (only meaningful for final tracks)
  m_isReco[m_mcn] = isReco;

  // True lifetime (relevant for composite particles) in picoseconds
  // ctau = m * L dot p / p2
  // m_mctau[m_mcn] = (1/(picosecond*c_light)) * (mcpart.momentum().m())* (mcpart.momentum().vect())*
  //  ((*ivert)->position() - mcpart.originVertex()->position())/ mcpart.momentum().vect().mag2();

  m_mcn++;

  // std::cout << "Done" << std::endl;
}
#endif
