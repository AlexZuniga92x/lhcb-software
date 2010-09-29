// Include files 
#include "BTaggingTool.h"

//--------------------------------------------------------------------------
// Implementation file for class : BTaggingTool
//
// Author: Marco Musy
//--------------------------------------------------------------------------

using namespace LHCb ;
using namespace Gaudi::Units;

// Declaration of the Algorithm Factory
DECLARE_TOOL_FACTORY( BTaggingTool );

//=========================================================================
BTaggingTool::BTaggingTool( const std::string& type,
                            const std::string& name,
                            const IInterface* parent ) :
  GaudiTool ( type, name, parent ) {

  declareInterface<IBTaggingTool>(this);

  declareProperty( "CombineTaggersName", m_CombineTaggersName = "CombineTaggersProbability" ); //"CombineTaggersNN"
  //declareProperty( "CombineTaggersName", m_CombineTaggersName = "CombineTaggersNN" );
  declareProperty( "UseVtxChargeWithoutOS", m_UseVtxOnlyWithoutOS = false );

  declareProperty( "ChoosePVCriterium",     m_ChoosePV    = "PVbyIPs");
  declareProperty( "UseReFitPV",            m_UseReFitPV  = false );

  declareProperty( "IPPU_cut",     m_IPPU_cut    = 4.0 );
  declareProperty( "thetaMin_cut", m_thetaMin    = 0.012 ); 
  declareProperty( "distphi_cut",  m_distphi_cut = 0.005 );

  declareProperty( "EnableMuonTagger",    m_EnableMuon    = true );
  declareProperty( "EnableElectronTagger",m_EnableElectron= true );
  declareProperty( "EnableKaonOSTagger",  m_EnableKaonOS  = true );
  declareProperty( "EnableKaonSSTagger",  m_EnableKaonSS  = true );
  declareProperty( "EnablePionTagger",    m_EnablePionSS  = true );
  declareProperty( "EnableVertexChargeTagger",m_EnableVertexCharge= true);
  declareProperty( "EnableJetSameTagger", m_EnableJetSame = false );
  declareProperty( "TaggerLocation", m_taggerLocation = "Phys/TaggingParticles" );
  m_util    = 0;
  m_descend = 0;
  m_pvReFitter = 0;
  m_combine = 0;
  m_taggerMu=m_taggerEle=m_taggerKaon=0;
  m_taggerKaonS=m_taggerPionS=m_taggerVtx=0 ;
}
BTaggingTool::~BTaggingTool() {}; 

//==========================================================================
StatusCode BTaggingTool::initialize() {

  m_util = tool<ITaggingUtils> ( "TaggingUtils", this );
  if( ! m_util ) {
    fatal() << "Unable to retrieve TaggingUtils tool "<< endreq;
    return StatusCode::FAILURE;
  }
  m_descend = tool<IParticleDescendants> ( "ParticleDescendants", this );
  if( ! m_descend ) {
    fatal() << "Unable to retrieve ParticleDescendants tool "<< endreq;
    return StatusCode::FAILURE;
  }
  m_taggerMu = tool<ITagger> ("TaggerMuonTool", this);
  if(! m_taggerMu) {
    fatal() << "Unable to retrieve TaggerMuonTool"<< endreq;
    return StatusCode::FAILURE;
  }
  m_taggerEle = tool<ITagger> ("TaggerElectronTool", this);
  if(! m_taggerEle) {
    fatal() << "Unable to retrieve TaggerElectronTool"<< endreq;
    return StatusCode::FAILURE;
  }
  m_taggerKaon = tool<ITagger> ("TaggerKaonOppositeTool", this);
  if(! m_taggerKaon) {
    fatal() << "Unable to retrieve TaggerKaonOppositeTool"<< endreq;
    return StatusCode::FAILURE;
  }
  m_taggerKaonS = tool<ITagger> ("TaggerKaonSameTool", this);
  if(! m_taggerKaonS) {
    fatal() << "Unable to retrieve TaggerKaonSameTool"<< endreq;
    return StatusCode::FAILURE;
  }
  m_taggerPionS = tool<ITagger> ("TaggerPionSameTool", this);
  if(! m_taggerPionS) {
    fatal() << "Unable to retrieve TaggerPionSameTool"<< endreq;
    return StatusCode::FAILURE;
  }
  m_taggerVtxCh= tool<ITagger> ("TaggerVertexChargeTool", this);
  if(! m_taggerVtxCh) {
    fatal() << "Unable to retrieve TaggerVertexChargeTool"<< endreq;
    return StatusCode::FAILURE;
  }
  m_taggerJetS = tool<ITagger> ("TaggerJetSameTool", this);
  if(! m_taggerJetS) {
    fatal() << "Unable to retrieve TaggerJetSameTool"<< endreq;
    return StatusCode::FAILURE;
  }
  m_combine = tool<ICombineTaggersTool> (m_CombineTaggersName, this);
  if(! m_combine) {
    fatal() << "Unable to retrieve "<< m_CombineTaggersName << endreq;
    return StatusCode::FAILURE;
  }
  if(m_ChoosePV =="RefitPV") {
    m_pvReFitter = tool<IPVReFitter>("AdaptivePVReFitter", this );
    if(! m_pvReFitter) {
      fatal() << "Unable to retrieve AdaptivePVReFitter" << endreq;
      return StatusCode::FAILURE;
    }
  }

  return StatusCode::SUCCESS;
}

//==========================================================================
StatusCode BTaggingTool::tag( FlavourTag& theTag, const Particle* AXB, 
                              const RecVertex* RecVert,
                              Particle::ConstVector& vtags ) {

  theTag.setDecision( FlavourTag::none );
  if ( ! AXB ) {
    err() << "No B hypothesis to tag!" << endreq;
    return StatusCode::FAILURE;
  }

  //Load vertices and particles
  const Particle::Range  parts=get<Particle::Range>(m_taggerLocation+"/Particles");
  const RecVertex::Range verts=get<RecVertex::Range>(RecVertexLocation::Primary);
  if(msgLevel(MSG::DEBUG)) debug()<<" Nr Vertices: "<< verts.size() 
                                  <<" Nr Particles: "<<parts.size()<<endreq;

  Vertex::ConstVector allVtx;
  RecVertex::Range::const_iterator iv;
  for(iv=verts.begin(); iv!=verts.end(); iv++) {
    if(msgLevel(MSG::DEBUG)) debug()<<"PV found at z="<<(*iv)->position().z()/mm<<endreq;
    allVtx.push_back(AXB->endVertex());
  }

  //////////////////////////////////////////////////////////////////////////////
  //choose RecVert PV and pileup vertices taking into account refitting ///////
  RecVertex RefitRecVert(0);
  const RecVertex::ConstVector PileUpVtx= choosePrimary(AXB, verts, RecVert, RefitRecVert);

  if( msgLevel(MSG::DEBUG) ) {
    debug()<<"--> RecVert z=" << RecVert->position().z()/mm <<"  "<<m_ChoosePV<<endreq;
    if(m_ChoosePV=="RefitPV")
      debug() <<"-->     refitRecVert z=" << RefitRecVert.position().z()/mm <<endreq;
    for(RecVertex::ConstVector::const_iterator iv=PileUpVtx.begin();
        iv!=PileUpVtx.end(); iv++) {
      debug()<<"--> PileUpPV at z="<<(*iv)->position().z()/mm<<endreq;
    }
  }
  
  ////////////////////////////////////////////////////////
  //loop over Particles, preselect candidates ///////////
  theTag.setTaggedB( AXB );
  if(vtags.empty()) vtags = chooseCandidates(AXB, parts, PileUpVtx);
  

  //AXB is the signal B from selection
  bool isBd = false; if( AXB->particleID().hasDown() )   isBd = true;
  bool isBs = false; if( AXB->particleID().hasStrange()) isBs = true;
  bool isBu = false; if( AXB->particleID().hasUp() )     isBu = true;

  ///Choose Taggers ------------------------------------------------------ 
  if (msgLevel(MSG::DEBUG)) debug() <<"evaluate taggers" <<endreq;
  Tagger muon, elec, kaon, kaonS, pionS, vtxCh, jetS;

  if(m_EnableMuon)     muon = m_taggerMu   -> tag(AXB, RecVert, allVtx, vtags);
  if(m_EnableElectron) elec = m_taggerEle  -> tag(AXB, RecVert, allVtx, vtags);
  if(m_EnableKaonOS)   kaon = m_taggerKaon -> tag(AXB, RecVert, allVtx, vtags);
  if(m_EnableKaonSS) if(isBs)  
    kaonS= m_taggerKaonS-> tag(AXB, RecVert, allVtx, vtags);
  if(m_EnablePionSS) if(isBd || isBu)
    pionS= m_taggerPionS-> tag(AXB, RecVert, allVtx, vtags);
  if(m_EnableJetSame)  jetS = m_taggerJetS -> tag(AXB, RecVert, allVtx, vtags);
  if(m_EnableVertexCharge){ 
    Particle::ConstVector vtagsPlusOS(0);
    Particle::ConstVector::const_iterator i, j;
    for ( i=vtags.begin(); i!=vtags.end(); i++ ){
      const ProtoParticle* iproto = (*i)->proto();
      for ( j=i+1; j!=vtags.end(); j++) if(iproto==(*j)->proto()) continue;
      if( m_UseVtxOnlyWithoutOS ) {
        if(muon.type()!=0) 
          if(muon.taggerParts().at(0)->proto() == iproto ) 
            vtagsPlusOS.push_back(*i);
        if(elec.type()!=0) 
          if(elec.taggerParts().at(0)->proto() == iproto ) 
            vtagsPlusOS.push_back(*i);
        if(kaon.type()!=0)
          if(kaon.taggerParts().at(0)->proto() == iproto ) 
            vtagsPlusOS.push_back(*i);
      }
      vtagsPlusOS.push_back(*i);
    }
    vtxCh = m_taggerVtxCh -> tag(AXB, RecVert, allVtx, vtagsPlusOS);
  }

  std::vector<Tagger*> taggers;
  taggers.push_back(&muon);
  taggers.push_back(&elec);
  taggers.push_back(&kaon);
  if( isBs ) taggers.push_back(&kaonS);
  if( isBu || isBd ) taggers.push_back(&pionS);
  taggers.push_back(&vtxCh);

  //----------------------------------------------------------------------
  //Now combine the individual tagger decisions into 
  //one final B flavour tagging decision. Such decision 
  //can be made and categorised in two ways: the "PID" 
  //approach is based on the particle type, while the 
  //"Prob" approach is based on the wrong tag fraction

  if (msgLevel(MSG::DEBUG)) debug() <<"combine taggers "<< taggers.size() <<endreq;
  m_combine -> combineTaggers( theTag, taggers );

  //now only using OS taggers
  std::vector<Tagger*> taggersOS;
  taggersOS.push_back(&muon);
  taggersOS.push_back(&elec);
  taggersOS.push_back(&kaon);
  taggersOS.push_back(&vtxCh);
  FlavourTag tmp_theTagOS = theTag;
  m_combine -> combineTaggers( tmp_theTagOS, taggersOS );
  ///fill FlavourTag object
  theTag.setDecisionOS( tmp_theTagOS.decision() );
  theTag.setCategoryOS( tmp_theTagOS.category() );
  theTag.setOmegaOS   ( tmp_theTagOS.omega() );


  ///OUTPUT to Logfile ---------------------------------------------------
  int sameside = kaonS.decision();
  if(!sameside) sameside = pionS.decision();

  RecHeader* evt = get<RecHeader> (RecHeaderLocation::Default);
  if (msgLevel(MSG::DEBUG)) debug() << "BTAGGING TAG   " 
                                    << std::setw(9) << evt->runNumber()
                                    << std::setw(9) << evt->evtNumber()
                                    << std::setw(5) << theTag.decision()
                                    << std::setw(3) << theTag.category()
                                    << std::setw(5) << theTag.decisionOS()
                                    << std::setw(3) << theTag.categoryOS()
                                    << std::setw(5) << muon.decision()
                                    << std::setw(3) << elec.decision()
                                    << std::setw(3) << kaon.decision()
                                    << std::setw(3) << sameside
                                    << std::setw(3) << vtxCh.decision()
                                    << endmsg;

  return StatusCode::SUCCESS;
}
//==========================================================================
StatusCode BTaggingTool::tag( FlavourTag& theTag, const Particle* AXB ) {
  Particle::ConstVector p(0);
  return tag( theTag, AXB, 0, p ) ;
}
//==========================================================================
StatusCode BTaggingTool::tag( FlavourTag& theTag, const Particle* AXB, 
                              const RecVertex* RecVert ) {
  Particle::ConstVector p(0);
  return tag( theTag, AXB, RecVert, p );
}

///=========================================================================
const RecVertex::ConstVector 
BTaggingTool::choosePrimary(const Particle* AXB,
                            const RecVertex::Range& verts,
                            const RecVertex*& RecVert,
                            RecVertex& RefitRecVert){
  bool user_recvert = false;
  const RecVertex* RecVert_save=0;
  if( RecVert ) { //PV was given by the user
    debug()<<"Will use the PV given by the user."<<endreq;
    RecVert_save = RecVert;
    user_recvert = true;
  }

  RecVertex::ConstVector PileUpVtx(0); //will contain all the other primary vtx's

  double kdmin = 1000000;
  RecVertex::Range::const_iterator iv;
  for(iv=verts.begin(); iv!=verts.end(); iv++){
    RecVertex newPV(**iv);
    double var, ip, iperr;
    if(m_ChoosePV=="RefitPV") { 
      Particle newPart(*AXB);
      StatusCode sc = m_pvReFitter->remove(&newPart, &newPV);     
      if(!sc) { err()<<"ReFitter fails!"<<endreq; continue; }
      m_util->calcIP(AXB, &newPV, ip, iperr); 
      var=fabs(ip); 
    } else if(m_ChoosePV=="PVbyIP") { //cheated sel needs this
      m_util->calcIP(AXB, *iv, ip, iperr);
      var=fabs(ip); 	
    } else if(m_ChoosePV=="PVbyIPs") { 
      m_util->calcIP(AXB, *iv, ip, iperr);
      if(!iperr){
        err()<<"IPerror zero or nan, skip vertex: "<<iperr<<endreq;
        continue;
      }
      var=fabs(ip/iperr); 	
    } else {
      err()<<"Invalid option ChoosePVCriterium: "<<m_ChoosePV<<endreq;
      return PileUpVtx;
    }
    if( var < kdmin ) {
      kdmin = var;
      RecVert = (*iv);
      if(m_ChoosePV=="RefitPV") RefitRecVert = newPV;
    }    
    if( ! RecVert ) {
      err() <<"No Reconstructed Vertex! Skip." <<endreq;
      return PileUpVtx;
    }    
  }

  //build a vector of pileup vertices --------------------------
  for(iv=verts.begin(); iv!=verts.end(); iv++){
    if( (*iv) == RecVert ) continue;
    PileUpVtx.push_back(*iv);
  }
  //UseReFitPV means that it will use the refitted pV for the ip calculation 
  //of taggers and SV building. Do not move this line above PileUpVtx building
  if( !user_recvert ) if( m_UseReFitPV ) if(m_ChoosePV=="RefitPV")
    RecVert = (&RefitRecVert);
  //force to use supplied PV from user
  if( user_recvert ) RecVert = RecVert_save;
  if( ! RecVert ) {
    err() <<"No Reconstructed Vertex!! Skip." <<endreq;
    return PileUpVtx;
  }    

  return PileUpVtx;
}

//=============================================================================
const Particle::ConstVector 
BTaggingTool::chooseCandidates(const Particle* AXB,
                               const Particle::Range& parts,
                               const RecVertex::ConstVector& PileUpVtx) {
  double distphi;
  Particle::ConstVector vtags(0);
  Particle::Range::const_iterator ip;
  Particle::ConstVector axdaugh = m_descend->descendants( AXB );
  axdaugh.push_back( AXB );
  for ( ip = parts.begin(); ip != parts.end(); ip++ ){

    const ProtoParticle* proto = (*ip)->proto();
    if( (*ip)->p()/GeV < 2.0 ) continue;               
    if( (*ip)->momentum().theta() < m_thetaMin ) continue;
    if( (*ip)->charge() == 0 ) continue;               
    if( !proto )               continue;
    if( !proto->track() ) continue;
    if( proto->track()->type() < 3 )   continue; 
    if( proto->track()->type() > 4 )   continue; 
    //    if( (proto->track()->type()!=3) and (proto->track()->type()!=4) and  (proto->track()->type()!=7) )   continue; 
    if( (*ip)->p()/GeV  > 200 ) continue;
    if( (*ip)->pt()/GeV >  10 ) continue;
    if( m_util->isinTree( *ip, axdaugh, distphi ) ) continue ;//exclude signal
    if( distphi < m_distphi_cut ) continue;

    //calculate the min IP wrt all pileup vtxs
    double ippu, ippuerr;
    m_util->calcIP( *ip, PileUpVtx, ippu, ippuerr );
    //eliminate from vtags all parts coming from a pileup vtx
    if(ippuerr) if( ippu/ippuerr<m_IPPU_cut ) continue; //preselection

    bool dup=false;
    //if((*ip)->particleID().abspid()==211) { 
    Particle::ConstVector::const_iterator ik;
    for ( ik = ip+1; ik != parts.end(); ik++) {
      if((*ik)->proto() == proto) { 
        dup=true; 
        break; 
      }
    }
    //}
    if(dup) continue; 
 
    //////////////////////////////////
    vtags.push_back(*ip);          // store tagger candidate
    ////////////////////////////////

    if (msgLevel(MSG::DEBUG)) 
      debug() <<"part ID="<<(*ip)->particleID().pid()
              <<" p="<<(*ip)->p()/GeV
              <<" pt="<<(*ip)->pt()/GeV
              <<" PIDm="<<(*ip)->proto()->info( ProtoParticle::CombDLLmu, 0)
              <<" PIDe="<<(*ip)->proto()->info( ProtoParticle::CombDLLe, 0)
              <<" PIDk="<<(*ip)->proto()->info( ProtoParticle::CombDLLk, 0)
              <<endreq;
  }
  return vtags;
}

//=========================================================================
StatusCode BTaggingTool::finalize() { return StatusCode::SUCCESS; }


