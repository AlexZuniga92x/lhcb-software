// Include files
// from Gaudi
#include "Event/ODIN.h" // event & run number
#include "GaudiKernel/AlgFactory.h"
#include "Event/HltDecReports.h"

#include "Event/HltSelReports.h"
#include "Event/HltVertexReports.h"
#include "Event/HltDecReports.h"
// local
#include "TeslaReportAlgo.h"

static const  Gaudi::StringKey RelInfoLocations{"RelInfoLocations"};

//-----------------------------------------------------------------------------
// Implementation file for class : TeslaReportAlgo
//
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( TeslaReportAlgo )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TeslaReportAlgo::TeslaReportAlgo( const std::string& name,
                                    ISvcLocator* pSvcLocator)
: GaudiAlgorithm ( name , pSvcLocator ), m_PV3D(true), m_refitted(false), m_dist(NULL), m_check(NULL), m_conv(NULL)
, m_hltANNSvc {NULL}
{
  declareProperty( "TriggerLines" ,         m_inputNames    = {"Hlt2CharmHadD02HH_D02KK"} );
  declareProperty( "OutputPrefix" ,         m_OutputPref   = "Tesla" );
  declareProperty( "PV" ,                   m_PV = "Offline" );
  declareProperty( "VertRepLoc" ,           m_VertRepLoc = "Hlt2/VertexReports" );
  declareProperty( "SelRepLoc" ,            m_SelRepLoc = "Hlt2/SelReports" );
  declareProperty( "PVLoc" ,                m_PVLoc = "Turbo/Primary" );
}

//=============================================================================
// Destructor
//=============================================================================
TeslaReportAlgo::~TeslaReportAlgo() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TeslaReportAlgo::initialize()
{
  turbo=false;
  StatusCode sc = GaudiAlgorithm::initialize();
  if ( sc.isFailure() ) return sc;
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;
  if ( msgLevel(MSG::DEBUG) ) {
    debug() << "Requested processing of lines: \n" << endmsg;
    for(auto line : m_inputNames) debug() << line << endmsg;
  }
  m_dist = tool<IDistanceCalculator>("LoKi::DistanceCalculator", this );
  if ( !m_dist )
  {
    return Error("Unable to retrieve the IDistanceCalculator tool");
  }
  m_check = tool<IReportCheck>("ReportCheckTool", this );
  if ( !m_check )
  {
    return Error("Unable to retrieve the ReportCheckTool");
  }
  m_conv = tool<IReportConvert>("ReportConvertTool", this );
  m_conv->setReportVersionLatest();
  if ( !m_conv )
  {
    return Error("Unable to retrieve the ReportConvertTool");
  }

  m_hltANNSvc = svc<IANNSvc>("HltANNSvc");
  m_tckANNSvc = svc<IIndexedANNSvc>("TCKANNSvc");
  return StatusCode::SUCCESS;
}

//=============================================================================
// Main execution
//=============================================================================
StatusCode TeslaReportAlgo::execute()
{
  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Execute" << endmsg;

  // get string-to-dict Rel. info. locations map
  unsigned int tck = m_check->getTCK();
  if(tck!=m_lastTCK) m_RelInfoLocationsMap.clear();
  m_lastTCK=tck;
  if(m_RelInfoLocationsMap.size()==0){
    if(tck==0){
      for( auto p: m_hltANNSvc->item_map(RelInfoLocations) ) {
        m_RelInfoLocationsMap.insert( std::pair<int,std::string>(p.second,p.first) );
      }
    } else {
      for( auto p: m_tckANNSvc->i2s(tck,RelInfoLocations) ) {
        m_RelInfoLocationsMap.insert( p );
      }
    }
  }

  // For jobs with multiple versions, need to check bank header
  m_versionNum = m_check->checkBankVersion();
  if ( msgLevel(MSG::DEBUG) ) debug() << "VersionNum = " << m_versionNum << endmsg;
  m_conv->setReportVersion(m_versionNum);

  const LHCb::HltSelReports* selReports;
  const LHCb::HltVertexReports* vtxReports;
  const LHCb::HltVertexReports::HltVertexReport* vtxRep;
  // Vertex reports
  if ( exist<LHCb::HltVertexReports>( m_VertRepLoc.c_str() ) ) { // exist --> get content
    vtxReports = getIfExists<LHCb::HltVertexReports>( m_VertRepLoc.c_str() );
    vtxRep = vtxReports->vertexReport("PV3D");
    if( vtxRep )debug() << "# of PVs in vertex reports: " << vtxRep->size() << endmsg;
    else debug() << "# of PVs in vertex reports: 0 - Could be decoding problem" << endmsg;
    if ( msgLevel(MSG::DEBUG) ){
      std::vector<std::string> vnames = vtxReports->selectionNames();
      debug() << "Available vertex reports:" << endmsg;
      for( auto vname_it : vnames ){
        debug() << vname_it << endmsg;
      }
    }
  } else{
    info() << "Vertex reports do not exist!!!" << endmsg;
    //
    return StatusCode::SUCCESS;
  }

  if ( !vtxRep ) m_PV3D=false;
  else m_PV3D=true;
  // Selection reports
  if ( exist<LHCb::HltSelReports>( m_SelRepLoc.c_str() ) ) { // exist --> get content
    selReports = get<LHCb::HltSelReports>( m_SelRepLoc.c_str() );
    if ( msgLevel(MSG::DEBUG) ){
      std::vector<std::string> selnames = selReports->selectionNames();
      debug() << "Available reports:" << endmsg;
      for( auto selname_it : selnames ){
        debug() << selname_it << endmsg;
      }
    }
  } else{
    info() << "Sel. reports do not exist!!!" << endmsg;
    //
    return StatusCode::SUCCESS;
  }

  // Bring back RecSummary
  const LHCb::HltObjectSummary* recsummaryObj = selReports->selReport("Hlt2RecSummary");
  if ( recsummaryObj && !exist<LHCb::RecSummary>( "Turbo/Rec/Summary" ) ) {
    LHCb::RecSummary* recSummary = new LHCb::RecSummary() ;
    put( recSummary , "Turbo/Rec/Summary" );
    LHCb::HltObjectSummary::Info Rec_info = recsummaryObj->substructure()[0].target()->numericalInfo();
    m_conv->RecSummaryObjectFromSummary(&Rec_info,recSummary);
    if ( msgLevel(MSG::DEBUG) ){
      debug() << "Expected RecSummary HLTOS has ID (106): "
              << recsummaryObj->substructure()[0].target()->summarizedObjectCLID() << endmsg;
      debug() << "Global subdetector properties:" << endmsg;
      debug() << "nTracks = " << recSummary->info(LHCb::RecSummary::nTracks,0) << endmsg;
      debug() << "nLongTracks = " << recSummary->info(LHCb::RecSummary::nLongTracks,0) << endmsg;
      debug() << "nDownstreamTracks = " << recSummary->info(LHCb::RecSummary::nDownstreamTracks,0) << endmsg;
      debug() << "nVeloTracks = " << recSummary->info(LHCb::RecSummary::nVeloTracks,0) << endmsg;
      debug() << "nSPDhits = " << recSummary->info(LHCb::RecSummary::nSPDhits,0) << endmsg;
      debug() << "nRich1Hits = " << recSummary->info(LHCb::RecSummary::nRich1Hits,0) << endmsg;
      debug() << "nRich2Hits = " << recSummary->info(LHCb::RecSummary::nRich2Hits,0) << endmsg;
    }
  }
  else if( !recsummaryObj && !exist<LHCb::RecSummary>( "Turbo/Rec/Summary" ) ){
    info() << "RecSummary unavailable, global dectector information will not be resurrected" << endmsg;
  }

  // first create the shared containers
  std::stringstream ss_ProtoLoc;
  ss_ProtoLoc << m_OutputPref << "Protos";
  std::stringstream ss_TrackLoc;
  ss_TrackLoc << m_OutputPref << "Tracks";
  std::stringstream ss_RPIDLoc;
  ss_RPIDLoc << m_OutputPref << "RichPIDs";
  std::stringstream ss_MPIDLoc;
  ss_MPIDLoc << m_OutputPref << "MuonPIDs";
  std::stringstream ss_CaloHyposLoc;
  ss_CaloHyposLoc << m_OutputPref << "CaloHypos";
  std::stringstream ss_CaloClustLoc;
  ss_CaloClustLoc << m_OutputPref << "CaloClusters";
  //
  LHCb::ProtoParticle::Container* cont_Proto = getOrCreate<LHCb::ProtoParticle::Container,LHCb::ProtoParticle::Container>(ss_ProtoLoc.str().c_str()) ;
  LHCb::Track::Container* cont_Track = getOrCreate<LHCb::Track::Container,LHCb::Track::Container>(ss_TrackLoc.str().c_str()) ;
  LHCb::RichPID::Container* cont_RPID = getOrCreate<LHCb::RichPID::Container,LHCb::RichPID::Container>(ss_RPIDLoc.str().c_str()) ;
  LHCb::MuonPID::Container* cont_MPID = getOrCreate<LHCb::MuonPID::Container,LHCb::MuonPID::Container>(ss_MPIDLoc.str().c_str()) ;
  LHCb::CaloHypos* cont_CaloHypo = getOrCreate<LHCb::CaloHypos,LHCb::CaloHypos>(ss_CaloHyposLoc.str().c_str());
  LHCb::CaloClusters* cont_CaloClust = getOrCreate<LHCb::CaloClusters,LHCb::CaloClusters>(ss_CaloClustLoc.str().c_str());

  for(auto line : m_inputNames){
    // Go and get the information from the Hlt
    std::stringstream HltLoc;
    m_inputName = line;
    HltLoc << m_inputName << "Decision";
    //
    const LHCb::HltObjectSummary * MotherRep = selReports->selReport(HltLoc.str().c_str());

    if(MotherRep!=0) {if ( msgLevel(MSG::DEBUG) ) debug() << "Required line has been fired" << endmsg;}
    else continue;

    // Check validity
    LHCb::HltObjectSummary firstCand = *(MotherRep->substructure()[0]->substructure()[0].target());
    if( (firstCand.numericalInfo().size()==0) && (firstCand.summarizedObjectCLID()==LHCb::Particle::classID()) ){
      warning() << "Candidate has invalid particle information" << endmsg;
      return StatusCode::SUCCESS;
    };
    // Set our output locations
    std::stringstream ss_PartLoc;
    ss_PartLoc << m_OutputPref << m_inputName << "/Particles";
    //
    std::stringstream ss_VertLoc;
    ss_VertLoc << m_OutputPref << m_inputName << "/Vertices";
    //
    std::stringstream ss_P2PVLoc;
    ss_P2PVLoc << m_OutputPref << m_inputName << "/Particle2VertexRelations";
    //
    LHCb::Particle::Container* cont_Part = new LHCb::Particle::Container() ;
    put( cont_Part , ss_PartLoc.str().c_str() );
    //
    LHCb::Vertex::Container* cont_Vert = new LHCb::Vertex::Container() ;
    put( cont_Vert , ss_VertLoc.str().c_str() );

    //
    // PV situation:
    // Need to detect if a refitted PV has been stored.
    // Logic flow:
    // - If a refitted PVs are stored, put them in lines private container
    // - If no refitted PVs => First algorithm creates shared PV container
    // - else algorithm uses the already created shared PV container

    // Detect refitted PV
    m_refitted=false;
    std::vector<LHCb::RecVertex*> refit_PVs;
    SmartRefVector <LHCb::HltObjectSummary> substructure_PVdet = MotherRep->substructure()[0]->substructure();
    if ( msgLevel(MSG::DEBUG) ) debug() << "Checking for refitted PV" << endmsg;
    for( auto child : substructure_PVdet ){
      const LHCb::HltObjectSummary * Obj = child.target();
      if ( msgLevel(MSG::DEBUG) ) debug() << " - Object has ID: " << Obj->summarizedObjectCLID() << endmsg;
      if(Obj->summarizedObjectCLID()==LHCb::RecVertex::classID()) {
        if ( msgLevel(MSG::DEBUG) ) debug() << "Refitted PV detected, resurrecting these" << endmsg;
        m_refitted=true;
        LHCb::RecVertex* refit_PV = new LHCb::RecVertex();
        const LHCb::HltObjectSummary::Info PV_info = Obj->numericalInfo();
        std::vector<std::string> vec_keys = Obj->numericalInfoKeys();
        if ( msgLevel(MSG::DEBUG) ) {
          debug() << "Saved Rec::Vertex information: " << endmsg;
          for(std::vector<std::string>::iterator it = vec_keys.begin(); it!=vec_keys.end(); it++) {
            debug() << *it << " = " << PV_info[(*it)] << endmsg;
          }
        }
        m_conv->RecVertexObjectFromSummary(&PV_info,refit_PV,true);
        // Make sure technique is known as Primary
        refit_PV->setTechnique(LHCb::RecVertex::Primary);
        refit_PVs.push_back(refit_PV);
      }
    }
    //

    if(m_refitted){
      std::stringstream ss_PVrefit;
      ss_PVrefit << m_OutputPref << m_inputName << "/_ReFitPVs";
      string sharedPVLoc = m_PVLoc;
      m_PVLoc=ss_PVrefit.str().c_str();

      // Need to deal with special case of a line refitting but asking for persist reco
      // (standard particles will expect shared PV container)
      LHCb::RecVertex::Container* cont_sharedPV = getIfExists<LHCb::RecVertex::Container>( sharedPVLoc.c_str() );
      if( !cont_sharedPV ) {
        if ( msgLevel(MSG::DEBUG) ) debug() << "Making PVs even though refitted ones will be used" << endmsg;
        cont_sharedPV = new LHCb::RecVertex::Container() ;
        put( cont_sharedPV , sharedPVLoc.c_str() );
        for( auto iv = vtxRep->begin(); iv!=vtxRep->end(); iv++){
          const LHCb::VertexBase* v = *iv;
          LHCb::RecVertex* vnew = new LHCb::RecVertex();
          // Turn the VertexBase from the vertex reports in to a RecVertex for packing
          vnew->setChi2( v->chi2() );
          vnew->setNDoF( v->nDoF() );
          vnew->setPosition( v->position() );
          vnew->setCovMatrix( v->covMatrix() );
          vnew->setExtraInfo( v->extraInfo() );
          vnew->setTechnique(LHCb::RecVertex::Primary);

          // Fill PV container
          cont_sharedPV->insert( vnew );
        }
      }
    }

    LHCb::RecVertex::Container* cont_PV = getIfExists<LHCb::RecVertex::Container>( m_PVLoc.c_str() );
    bool reusePV = true;
    if( !cont_PV ) {
      if ( msgLevel(MSG::DEBUG) ) debug() << "Making PVs" << endmsg;
      cont_PV = new LHCb::RecVertex::Container() ;
      put( cont_PV , m_PVLoc.c_str() );
      reusePV = false;
    }
    if( !reusePV && !m_refitted && m_PV3D ){
      for( auto iv = vtxRep->begin(); iv!=vtxRep->end(); iv++){
        const LHCb::VertexBase* v = *iv;
        LHCb::RecVertex* vnew = new LHCb::RecVertex();
        if ( msgLevel(MSG::DEBUG) ) {
          debug() << "Vertex chi2= " << v->chi2()<< ", ndf =" << v->nDoF()<< endmsg;
          debug() << "Vertex key= " << v->key() << endmsg;
        }
        // Turn the VertexBase from the vertex reports in to a RecVertex for packing
        vnew->setChi2( v->chi2() );
        vnew->setNDoF( v->nDoF() );
        vnew->setPosition( v->position() );
        vnew->setCovMatrix( v->covMatrix() );
        vnew->setExtraInfo( v->extraInfo() );
        vnew->setTechnique(LHCb::RecVertex::Primary);

        // Fill PV container
        cont_PV->insert( vnew );
      }
    }
    else if(m_refitted) {
      if ( msgLevel(MSG::DEBUG) ) debug() << "Inserting refitted PVs to container" << endmsg;
      for( auto pv : refit_PVs ) cont_PV->insert(pv);
    }

    //
    Particle2Vertex::Table* cont_P2PV = new Particle2Vertex::Table() ;
    put( cont_P2PV , ss_P2PVLoc.str().c_str() );

    if(MotherRep){
      if ( msgLevel(MSG::DEBUG) ){
        debug() << "Reference: " << endmsg;
        debug() << "Particle class ID = " << LHCb::Particle::classID() << endmsg;
        debug() << "Track class ID = " << LHCb::Track::classID() << endmsg;
        debug() << "RecVertex class ID = " << LHCb::RecVertex::classID() << endmsg;
        debug() << "Vertex class ID = " << LHCb::Vertex::classID() << endmsg;
        debug() << "ProtoParticle class ID = " << LHCb::ProtoParticle::classID() << endmsg;
      }
      //
      if ( msgLevel(MSG::DEBUG) ) debug() << *MotherRep << endmsg;
      std::vector<LHCb::LHCbID> LHCbIDs = MotherRep->lhcbIDs();
      if ( msgLevel(MSG::DEBUG) ) {
        debug() << "MOTHER CLASS ID = " << MotherRep->summarizedObjectCLID() << endmsg;
        debug() << "LHCbIDs.size() = " << LHCbIDs.size() << endmsg;
      }
      SmartRefVector <LHCb::HltObjectSummary> substructure = MotherRep->substructure();
      if ( msgLevel(MSG::DEBUG) ) debug() << "Number of triggered candidates = " << substructure.size() << endmsg;
      //


      int nCandidate = -1;

      // loop over the trigger candidates
      for( auto child : substructure ){
        nCandidate++;
        if ( msgLevel(MSG::DEBUG) ) {
          debug() << "Starting loop over triggered candidates" << endmsg;
          debug() << "Mother object CLASS ID = " << child.target()->summarizedObjectCLID() << endmsg;
        }
        LHCb::HltObjectSummary* Obj = child.target();

        // Check we have turbo level information
        if(m_conv->getSizeSelRepParticleLatest()<(int)Obj->numericalInfo().size()) turbo=true;

        // do we have a basic particle?
        // Assess basic by seeing if daughter particles present
        double motherID = (int)Obj->numericalInfo()["0#Particle.particleID.pid"];
        if ( msgLevel(MSG::DEBUG) ) debug() << "Found mother with ID: " << motherID << endmsg;
        bool motherBasic = true;
        if ( msgLevel(MSG::DEBUG) ) debug() << "Mother has substructure consisting of:" << endmsg;
        for( auto it : Obj->substructure() ){
          if ( msgLevel(MSG::DEBUG) ) debug() << (it).target()->summarizedObjectCLID() << endmsg;
          if((it).target()->summarizedObjectCLID()==LHCb::Particle::classID()) motherBasic=false;
        }

        // make the objects to be stored
        std::vector<ContainedObject*> newObjects_mother;
        LHCb::Particle* Part = new LHCb::Particle();
        newObjects_mother.push_back(Part);
        cont_Part->insert( Part );
        std::vector<ContainedObject*> calo_mother;
        //
        if( motherBasic == true ){
          //
          LHCb::Track* track = new LHCb::Track();
          newObjects_mother.push_back(track);
          //
          LHCb::ProtoParticle* proto = new LHCb::ProtoParticle();
          newObjects_mother.push_back(proto);
          //
          LHCb::RichPID* rich = new LHCb::RichPID();
          newObjects_mother.push_back(rich);
          //
          LHCb::MuonPID* muon = new LHCb::MuonPID();
          newObjects_mother.push_back(muon);
          //

          fillParticleInfo( newObjects_mother , Obj , motherBasic, cont_Track, &calo_mother, cont_CaloHypo, cont_CaloClust );

          // check if we already have the protoparticle in the shared container,
          // if so use that one
          bool protoClone=false;
          for(auto iproto : *cont_Proto){
            if(proto->track()){
              // charged case
              if(iproto->track()){
                if( LHCb::HashIDs::overlap(proto->track(),iproto->track()).first>0.99 ) {
                  // found the protoparticle already in the container
                  delete proto; delete track; delete rich; delete muon;
                  for( auto calo : calo_mother ){
                    if( calo->clID() == LHCb::CLID_CaloHypo && !calo->parent() ) delete dynamic_cast<LHCb::CaloHypo*>( calo );
                    if( calo->clID() == LHCb::CLID_CaloCluster && !calo->parent() ) delete dynamic_cast<LHCb::CaloCluster*>( calo );
                  }
                  proto = iproto;
                  protoClone=true;
                  break;
                }
              }
            }
            else{
              // neutral case
              if( iproto->calo().size() != proto->calo().size() ) continue;
              // if we find a hypo that is not in the proto in the container then continue
              unsigned int commonHypos=0;
              for( auto hypo : proto->calo() ){
                // check to see if the same pointer is there
                for( auto ihypo : iproto->calo() ) {
                  if(hypo==ihypo){
                    commonHypos++;
                    break;
                  }
                }
                //if( compareHypoPosition(hypo, iproto->calo())!=hypo  && hypo->parent()) commonHypos++;
              }
              // if every hypo is in a proto already in the container, then use that proto
              if( commonHypos==proto->calo().size() ){
                delete proto;
                proto=iproto;
                protoClone=true;
                break;
              }
            }
          }
          Part->setProto( proto );

          if(!protoClone){
            cont_Proto->insert( proto );
            // Make sure we don't insert zombie objects
            if(track->type()!=LHCb::Track::Types::TypeUnknown || m_SelRepLoc=="Hlt1/SelReports") { // Hlt1 is alignment usage, no zombies
              cont_Track->insert( track );
              cont_MPID->insert( muon );
              cont_RPID->insert( rich );
            }
            else{
              delete track;
              delete muon;
              delete rich;
            }

            for( auto calo : calo_mother ){
              if( calo->clID() == LHCb::CLID_CaloHypo && !calo->parent() ) cont_CaloHypo->insert( dynamic_cast<LHCb::CaloHypo*>( calo ) );
              if( calo->clID() == LHCb::CLID_CaloCluster && !calo->parent() ) cont_CaloClust->insert( dynamic_cast<LHCb::CaloCluster*>( calo ) );
            }
          }
        }
        else{

          fillParticleInfo( newObjects_mother , Obj , motherBasic, cont_Track, &calo_mother, cont_CaloHypo, cont_CaloClust );
          ProcessObject( 0, Part, Obj, cont_PV, cont_Vert, cont_Part,
              cont_Proto, cont_RPID, cont_MPID, cont_Track, cont_CaloHypo, cont_CaloClust, cont_P2PV);

        }

        if( m_PV3D || m_refitted ) {
          LHCb::RecVertex* bpv = bestPV( Part, cont_PV );
          cont_P2PV->relate( Part , bpv );
          if( msgLevel(MSG::DEBUG) ){
            debug() << "Finding related PV" << endmsg;
            if(bpv){
              debug() << "Related PV has chi^2: " << bpv->chi2() << endmsg;
            }
          }
        }
      } // candidate (loop)
    } // report exists (if)
  }
  setFilterPassed(true);  // Mandatory. Set to true if event is accepted.
  if ( msgLevel(MSG::DEBUG) ) debug() << "End of algorithm execution" << endmsg;
  return StatusCode::SUCCESS;
}

StatusCode TeslaReportAlgo::ProcessObject(int n, LHCb::Particle* Part, const LHCb::HltObjectSummary* Obj
    ,LHCb::RecVertex::Container* cont_PV
    ,LHCb::Vertex::Container* cont_Vert
    ,LHCb::Particle::Container* cont_Part
    ,LHCb::ProtoParticle::Container* cont_Proto
    ,LHCb::RichPID::Container* cont_RPID
    ,LHCb::MuonPID::Container* cont_MPID
    ,LHCb::Track::Container* cont_Track
    ,LHCb::CaloHypo::Container* cont_CaloHypo
    ,LHCb::CaloCluster::Container* cont_CaloClust
    ,Particle2Vertex::Table* cont_P2PV
    ){

  n+=1;


  LHCb::Vertex* vert_mother = new LHCb::Vertex();
  cont_Vert->insert( vert_mother );

  for( auto child : Obj->substructure() ){
    if ( msgLevel(MSG::DEBUG) )
       debug() << "Daughter CLASS ID (level " << n << ") = " << (child).target()->summarizedObjectCLID() << endmsg;

    const LHCb::HltObjectSummary * Obj_d = child.target();
    if( Obj_d->summarizedObjectCLID()==40 ) {// RelatedInfo
      AddRelInfo(Obj_d,Part);
      continue;
    }

    if( Obj_d->summarizedObjectCLID()==LHCb::RecVertex::classID() ) {
      if(n!=1) {
        warning() << "RecVertex found below the top level, probably indicates a shared stage refitted PVs and was then combined. The refitted PV will not be restored." << endmsg;
        return StatusCode::FAILURE;
      }
      else continue;
    }

    if( Obj_d->summarizedObjectCLID()==LHCb::Vertex::classID() ){
      // for a non-basic particle, the end vertex is the last
      // element of the substructure
      Part->setEndVertex( vert_mother );
      fillVertexInfo( vert_mother , Obj_d );
      if ( msgLevel(MSG::DEBUG) ) debug() << "Setting mother decay vertex" << endmsg;
      continue;
    }

    // do we have a basic particle?
    if ( msgLevel(MSG::DEBUG) ) debug() << "Daughter ID (level " << n << "): "
                                        << Obj_d->numericalInfo()["0#Particle.particleID.pid"] << endmsg;
    bool d_Basic = true;
    if ( msgLevel(MSG::DEBUG) ) debug() << "Daughter (level " << n <<") has substructure consisting of:" << endmsg;
    for( auto it : Obj_d->substructure() ){
      if ( msgLevel(MSG::DEBUG) ) debug() << (it).target()->summarizedObjectCLID() << endmsg;
      if((it).target()->summarizedObjectCLID()==LHCb::Particle::classID()) d_Basic=false;
    }

    // make the objects to be stored
    std::vector<ContainedObject*> newObjects_d;
    std::vector<ContainedObject*> calo_daughter;

    LHCb::Particle* Part_d = new LHCb::Particle();
    newObjects_d.push_back(Part_d);
    Part->addToDaughters(Part_d);
    cont_Part->insert( Part_d );

    //
    if( d_Basic == true ){
      //
      LHCb::Track* track_d = new LHCb::Track();
      newObjects_d.push_back(track_d);
      //
      LHCb::ProtoParticle* proto_d = new LHCb::ProtoParticle();
      newObjects_d.push_back(proto_d);
      //
      LHCb::RichPID* rich_d = new LHCb::RichPID();
      newObjects_d.push_back(rich_d);
      //
      LHCb::MuonPID* muon_d = new LHCb::MuonPID();
      newObjects_d.push_back(muon_d);
      //
      vert_mother->addToOutgoingParticles( Part_d );

      fillParticleInfo( newObjects_d , Obj_d , d_Basic, cont_Track, &calo_daughter, cont_CaloHypo, cont_CaloClust);

      // check if we already have the protoparticle in the shared container,
      // if so use that one
      bool protoClone=false;
      for(auto iproto : *cont_Proto){
        if(proto_d->track()){
          // charged case
          if(iproto->track()){
            if( LHCb::HashIDs::overlap(proto_d->track(),iproto->track()).first>0.99 ) {
              // found the protoparticle already in the container
              delete proto_d; delete track_d; delete rich_d; delete muon_d;
              for( auto calo : calo_daughter ){
                if( calo->clID() == LHCb::CLID_CaloHypo && !calo->parent() ) delete dynamic_cast<LHCb::CaloHypo*>( calo );
                if( calo->clID() == LHCb::CLID_CaloCluster && !calo->parent() ) delete dynamic_cast<LHCb::CaloCluster*>( calo );
              }
              proto_d = iproto;
              protoClone=true;
              break;
            }
          }
        }
        else{
          // neutral case
          if( iproto->calo().size() != proto_d->calo().size() ) continue;
          // if we find a hypo that is not in the proto in the container then continue
          unsigned int commonHypos=0;
          for( auto hypo_d : proto_d->calo() ){
            // check to see if the same pointer is there
            for( auto ihypo : iproto->calo() ) {
              if(hypo_d==ihypo){
                commonHypos++;
                break;
              }
            }
            //if( compareHypoPosition(hypo_d, iproto->calo())!=hypo_d || ) commonHypos++; 
          }
          // if every hypo is in a proto already in the container, then use that proto
          if( commonHypos==proto_d->calo().size() ){
            delete proto_d;
            proto_d=iproto;
            protoClone=true;
            break;
          }
        }
      }
      Part_d->setProto( proto_d );

      if(!protoClone){
        cont_Proto->insert( proto_d );
        // Make sure we don't insert zombie objects
        if(track_d->type()!=LHCb::Track::Types::TypeUnknown || m_SelRepLoc=="Hlt1/SelReports") { // Hlt1 is alignment usage, no zombies
          cont_Track->insert( track_d );
          cont_MPID->insert( muon_d );
          cont_RPID->insert( rich_d );
        }
        else{
          delete track_d;
          delete muon_d;
          delete rich_d;
        }

        for( auto calo : calo_daughter ){
          if( calo->clID() == LHCb::CLID_CaloHypo && !calo->parent() ) cont_CaloHypo->insert( dynamic_cast<LHCb::CaloHypo*>( calo ) );
          if( calo->clID() == LHCb::CLID_CaloCluster && !calo->parent() ) cont_CaloClust->insert( dynamic_cast<LHCb::CaloCluster*>( calo ) );
        }
      }
    }
    else {
      vert_mother->addToOutgoingParticles( Part_d );
      fillParticleInfo( newObjects_d , Obj_d , d_Basic , cont_Track , &calo_daughter, cont_CaloHypo, cont_CaloClust);
      ProcessObject( n, Part_d, Obj_d, cont_PV, cont_Vert, cont_Part,
          cont_Proto, cont_RPID, cont_MPID, cont_Track, cont_CaloHypo, cont_CaloClust, cont_P2PV);
    }

    if( m_PV3D || m_refitted ) {
      debug() << "Finding related PV" << endmsg;
      LHCb::RecVertex* bpv = bestPV( Part, cont_PV );
      cont_P2PV->relate( Part_d , bpv );
      if( msgLevel(MSG::DEBUG) ){
        if(bpv){
          debug() << "Related PV has chi^2: " << bpv->chi2() << endmsg;
        }
      }
    }
  }

  return StatusCode::SUCCESS;
}

void TeslaReportAlgo::fillParticleInfo(std::vector<ContainedObject*> vec_obj,
    const LHCb::HltObjectSummary* obj,
    bool isBasic, LHCb::Track::Container* cont_Track,
    std::vector<ContainedObject*>* calo_vector,
    LHCb::CaloHypos* cont_CaloHypo,
    LHCb::CaloClusters* cont_CaloClust){
/*
 * vector Key:
 * For basic particle:
 * 0: particle
 * 1: track
 * 2: protoparticle
 * 3: RICH PID
 * 4: Muon PID
 *
 * For non-basic:
 * 0: particle
 */

  // Set particle information
  const LHCb::HltObjectSummary::Info HLT_info = obj->numericalInfo();

  // What keys are present:
  std::vector<std::string> vec_keys = obj->numericalInfoKeys();
  if ( msgLevel(MSG::DEBUG) ) {
    debug() << "Available information" << endmsg;
    for(std::vector<std::string>::iterator it = vec_keys.begin(); it!=vec_keys.end(); it++)
       debug() << *it << " = " << HLT_info[(*it)] << endmsg;
  }
  //

  // PARTICLE *******************************************************
  // ID/Mass/Kinematics
  LHCb::Particle* part = (LHCb::Particle*)vec_obj[0];

  m_conv->ParticleObjectFromSummary(&HLT_info,part,turbo);
  if ( msgLevel(MSG::DEBUG) ) {
    debug() << "p = " << part->p() << endmsg;
    debug() << "p (par_mom) = " << part->p() << endmsg; // duplicate to make test match ref
    debug() << "pT (par_mom) = " << part->pt() << endmsg;
    debug() << "pX (par_mom) = " << part->momentum().px() << endmsg;
    debug() << "pY (par_mom) = " << part->momentum().py() << endmsg;
    debug() << "pZ (par_mom) = " << part->momentum().pz() << endmsg;
    debug() << "Particle measured mass = " << part->measuredMass() << endmsg;
    debug() << "Particle mass = " << part->momentum().M() << endmsg;
    if(turbo==true) debug() << "Particle CL = " << part->confLevel() << endmsg;
  }

  // What follows depends on whether or not the particle is basic or not
  if( isBasic == true ) {
    LHCb::Track* track = (LHCb::Track*)vec_obj[1];
    LHCb::ProtoParticle* proto = (LHCb::ProtoParticle*)vec_obj[2];
    LHCb::RichPID* rich = (LHCb::RichPID*)vec_obj[3];
    LHCb::MuonPID* muon = (LHCb::MuonPID*)vec_obj[4];

    std::vector<LHCb::LHCbID> totalIDs;
    // Get vector of substructure and extended substructure
    SmartRefVector<LHCb::HltObjectSummary> totSubstructure = obj->substructureExtended();
    for(SmartRefVector <LHCb::HltObjectSummary>::const_iterator it_basic
           = totSubstructure.begin();it_basic!=totSubstructure.end();++it_basic){

      const LHCb::HltObjectSummary* ObjBasic = it_basic->target();
      totalIDs.insert( totalIDs.end(), ObjBasic->lhcbIDs().begin(), ObjBasic->lhcbIDs().end());
      switch( ObjBasic->summarizedObjectCLID() )
      {
        case LHCb::CLID_Track:
          {
            const LHCb::HltObjectSummary::Info Track_info = ObjBasic->numericalInfo();

            LHCb::Track* muonTrack = new LHCb::Track();
            muonTrack->setFlags( (unsigned int)Track_info["10#Track.flags"] );
            if ( msgLevel(MSG::DEBUG) ) debug() << "***** TRACK TYPE TEST = " << muonTrack->type() << endmsg;
            // If we have the muon stub, attach it to the muonPID object
            if( muonTrack->type() == LHCb::Track::Types::Muon ) {
              muonTrack->setLhcbIDs( ObjBasic->lhcbIDs() );
              m_conv->TrackObjectFromSummary(&Track_info,muonTrack,turbo);
              muon->setMuonTrack( muonTrack );
              cont_Track->insert( muonTrack );
              break;
            }
            else{
              delete muonTrack;
            }
            //

            // What keys are present:
            std::vector<std::string> vec_keys_tr = ObjBasic->numericalInfoKeys();
            if ( msgLevel(MSG::DEBUG) ) {
              debug() << "Available information (track)" << endmsg;
              for(std::vector<std::string>::iterator it = vec_keys_tr.begin(); it!=vec_keys_tr.end(); it++)
                 debug() << *it << " = " << Track_info[(*it)] << endmsg;
            }
            //
            // LHCb track members:
            // double   m_chi2PerDoF ##
            // int  m_nDoF ##
            // double   m_likelihood ##
            // double   m_ghostProbability ##
            // unsigned int   m_flags
            // std::vector< LHCb::LHCbID >  m_lhcbIDs
            // std::vector< LHCb::State * >   m_states
            // LHCb::TrackFitResult *   m_fitResult
            // ExtraInfo  m_extraInfo
            // SmartRefVector< LHCb::Track >  m_ancestors
            m_conv->TrackObjectFromSummary(&Track_info,track,turbo);
            if ( msgLevel(MSG::DEBUG) ) debug() << "Track #chi^{2}/DoF = " << track->chi2PerDoF() << endmsg;

            if ( msgLevel(MSG::DEBUG) ) debug() << "Track details added" << endmsg;
            break;
          }

        case LHCb::CLID_ProtoParticle:
          {
            const LHCb::HltObjectSummary::Info Proto_info = ObjBasic->numericalInfo();
            // What keys are present:
            std::vector<std::string> vec_keys_pp = ObjBasic->numericalInfoKeys();
            if ( msgLevel(MSG::DEBUG) ) {
              for(std::vector<std::string>::iterator it = vec_keys_pp.begin(); it!=vec_keys_pp.end(); it++)
                 debug() << *it << " = " << Proto_info[(*it)] << endmsg;
              debug() << "Available information (proto)" << endmsg;
            }
            m_conv->ProtoParticleObjectFromSummary(&Proto_info,proto,turbo);
            // LHCb protoparticle members:
            // ExtraInfo   m_extraInfo (other members are a SmartRef to objects we create)
            // THIS IS A PLACE WHERE NEED TO KNOW WHAT PEOPLE NEED
            if ( msgLevel(MSG::DEBUG) ) {
              debug() << "Proto isPhoton = " << proto->info( 381,-1000) << endmsg;
              debug() << "ProtoParticle details added" << endmsg;
            }
            break;
          }

        case LHCb::CLID_RichPID:
          {
            const LHCb::HltObjectSummary::Info Rich_info = ObjBasic->numericalInfo();
            // What keys are present:
            std::vector<std::string> vec_keys_rich = ObjBasic->numericalInfoKeys();
            if ( msgLevel(MSG::DEBUG) ) {
              debug() << "Available information (rich)" << endmsg;
              for(std::vector<std::string>::iterator it = vec_keys_rich.begin(); it!=vec_keys_rich.end(); it++)
                 debug() << *it << " = " << Rich_info[(*it)] << endmsg;
            }
            //
            // LHCb RichPID  members:
            // unsigned int   m_pidResultCode
            // std::vector< float >   m_particleLLValues
            // SmartRef< LHCb::Track >  m_track
            //
            m_conv->RichPIDObjectFromSummary(&Rich_info,rich,turbo);
            if ( msgLevel(MSG::DEBUG) ) debug() << "RichPID details added" << endmsg;
            break;
          }

        case LHCb::CLID_CaloHypo:
          {
            bool alreadyPresent=false;
            LHCb::CaloHypo* hypotemp = new LHCb::CaloHypo();
            const LHCb::HltObjectSummary::Info hypo_info = ObjBasic->numericalInfo();
            // What keys are present:
            std::vector<std::string> vec_keys_hypo = ObjBasic->numericalInfoKeys();
            if ( msgLevel(MSG::DEBUG) ) {
              debug() << "Available information (hypo)" << endmsg;
              for(std::vector<std::string>::iterator it = vec_keys_hypo.begin(); it!=vec_keys_hypo.end(); it++)
                 debug() << *it << " = " << hypo_info[(*it)] << endmsg;
            }
            //
            m_conv->CaloHypoObjectFromSummary(&hypo_info,hypotemp,turbo);
            LHCb::CaloHypo* hypo = compareHypoPosition(hypotemp,cont_CaloHypo);
            debug() << "Hypo object Z: " << hypo->position()->z() << endmsg;
            if(hypo!=hypotemp) {
              if ( msgLevel(MSG::DEBUG) ) debug() << "Found already inserted hypo, using that one" << endmsg;
              alreadyPresent=true;
              delete hypotemp;
            }

            if ( msgLevel(MSG::DEBUG) ) debug() << "Added hypo info to object" << endmsg;
            calo_vector->push_back(hypo);
            SmartRefVector<LHCb::HltObjectSummary> hyposub = ObjBasic->substructureExtended();
            if ( msgLevel(MSG::DEBUG) ) debug() << "Commencing cluster addition" << endmsg;
            if ( msgLevel(MSG::DEBUG) ) debug() << "Hypo has " << hyposub.size() << " clusters" << endmsg;
            for(auto cluster : hyposub){
              if(!alreadyPresent) hypo->addToClusters(processCluster(cluster.target(),calo_vector,cont_CaloClust));
              // hypo has no ids, need to add the ones from the cluster
              totalIDs.insert( totalIDs.end(), cluster.target()->lhcbIDs().begin(), cluster.target()->lhcbIDs().end());
            }
            if ( msgLevel(MSG::DEBUG) ) debug() << "CaloHypo details added" << endmsg;
            break;
          }

        case LHCb::CLID_MuonPID:
          {
            const LHCb::HltObjectSummary::Info Muon_info = ObjBasic->numericalInfo();
            // What keys are present:
            std::vector<std::string> vec_keys_muon = ObjBasic->numericalInfoKeys();
            if ( msgLevel(MSG::DEBUG) ) {
              debug() << "Available information (muon)" << endmsg;
              for(std::vector<std::string>::iterator it = vec_keys_muon.begin(); it!=vec_keys_muon.end(); it++)
                 debug() << *it << " = " << Muon_info[(*it)] << endmsg;
            }
            //
            // LHCb MuonPID  members:
            // double   m_MuonLLMu
            // double   m_MuonLLBg
            // int  m_NShared
            // unsigned int   m_Status
            // SmartRef< LHCb::Track >
            // SmartRef< LHCb::Track >  m_muonTrack (NOT INCLUDED)
            m_conv->MuonPIDObjectFromSummary(&Muon_info,muon,turbo);
            debug() << "Muon LLMu = " << muon->MuonLLMu() << endmsg;
            debug() << "MuonPID details added" << endmsg;
            break;
          }

        case LHCb::CLID_CaloCluster:
          {
            processCluster(ObjBasic,calo_vector,cont_CaloClust);

            break;
          }
        default:
          {
            if ( msgLevel(MSG::DEBUG) )
               debug() << "No specific info for classID: " <<  ObjBasic->summarizedObjectCLID() << endmsg;
            continue;
          }
      }
    }
    // tie our pieces together and set cross class information
    if ( msgLevel(MSG::DEBUG) ) debug() << "Starting to tie pieces together" << endmsg;
    // add complete list of LHCbIDs
    track->setLhcbIDs( totalIDs );
    if ( msgLevel(MSG::DEBUG) ) debug() << "Associated LHCbIDs:" << endmsg;
    if ( msgLevel(MSG::DEBUG) ){
      for(unsigned int i=0;i<totalIDs.size();i++){
        debug()<<i<<"\t"<<totalIDs[i].detectorType()<<"\t"<<totalIDs[i].lhcbID()<<endmsg;
      }
    }

    // Only want to do this for charged particles, not neutral ones
    if ( msgLevel(MSG::DEBUG) ) debug() << "Track states vector of size: " << track->states().size() << endmsg;
    if(track->states().size()>0){
      proto->setMuonPID( muon );
      proto->setRichPID( rich );
      proto->setTrack( track );
      muon->setIDTrack( track );
      rich->setTrack( track );
      if ( msgLevel(MSG::DEBUG) ) debug() << "Pieces tied together" << endmsg;
      // If proto combined DLLs not set, set them to the RICH ones
      if(proto->info( LHCb::ProtoParticle::CombDLLe,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::CombDLLe,rich->particleDeltaLL(Rich::ParticleIDType::Electron));
      if(proto->info( LHCb::ProtoParticle::CombDLLmu,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::CombDLLmu,rich->particleDeltaLL(Rich::ParticleIDType::Muon));
      if(proto->info( LHCb::ProtoParticle::CombDLLpi,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::CombDLLpi,rich->particleDeltaLL(Rich::ParticleIDType::Pion));
      if(proto->info( LHCb::ProtoParticle::CombDLLk,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::CombDLLk,rich->particleDeltaLL(Rich::ParticleIDType::Kaon));
      if(proto->info( LHCb::ProtoParticle::CombDLLp,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::CombDLLp,rich->particleDeltaLL(Rich::ParticleIDType::Proton));
      // Set track quantities in the protoparticle
      if(proto->info( LHCb::ProtoParticle::TrackChi2PerDof,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::TrackChi2PerDof,track->chi2PerDoF());
      if(proto->info( LHCb::ProtoParticle::TrackNumDof,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::TrackNumDof,track->nDoF());
      if(proto->info( LHCb::ProtoParticle::TrackType,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::TrackType,track->type());
      if(proto->info( LHCb::ProtoParticle::TrackHistory,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::TrackHistory,track->history());
      if(proto->info( LHCb::ProtoParticle::TrackP,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::TrackP,part->p());
      if(proto->info( LHCb::ProtoParticle::TrackPt,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::TrackPt,part->pt());
      // Set rich quantities in the protoparticle
      if(proto->info( LHCb::ProtoParticle::RichDLLe,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::RichDLLe,rich->particleDeltaLL(Rich::ParticleIDType::Electron));
      if(proto->info( LHCb::ProtoParticle::RichDLLmu,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::RichDLLmu,rich->particleDeltaLL(Rich::ParticleIDType::Muon));
      if(proto->info( LHCb::ProtoParticle::RichDLLpi,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::RichDLLpi,rich->particleDeltaLL(Rich::ParticleIDType::Pion));
      if(proto->info( LHCb::ProtoParticle::RichDLLk,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::RichDLLk,rich->particleDeltaLL(Rich::ParticleIDType::Kaon));
      if(proto->info( LHCb::ProtoParticle::RichDLLp,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::RichDLLp,rich->particleDeltaLL(Rich::ParticleIDType::Proton));
      if(proto->info( LHCb::ProtoParticle::RichDLLbt,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::RichDLLbt,rich->particleDeltaLL(Rich::ParticleIDType::BelowThreshold));
      if(proto->info( LHCb::ProtoParticle::RichPIDStatus,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::RichPIDStatus,rich->pidResultCode());
      // Set muon quantities in the protoparticle
      if(proto->info( LHCb::ProtoParticle::MuonMuLL,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::MuonMuLL,muon->MuonLLMu());
      if(proto->info( LHCb::ProtoParticle::MuonBkgLL,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::MuonBkgLL,muon->MuonLLBg());
      if(proto->info( LHCb::ProtoParticle::MuonNShared,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::MuonNShared,muon->nShared());
      if(proto->info( LHCb::ProtoParticle::MuonPIDStatus,-1000)==-1000) proto->addInfo(LHCb::ProtoParticle::MuonPIDStatus,muon->Status());
      if(proto->info( LHCb::ProtoParticle::InAccMuon,-1000)==-1000) {
        if(muon->InAcceptance()==true) proto->addInfo(LHCb::ProtoParticle::InAccMuon,1.0);
        else proto->addInfo(LHCb::ProtoParticle::InAccMuon,0.0);
      }
      // Make sure we can process older data, which does
      // not know about deuterons
      if ( m_versionNum < 9 && Rich::ParticleIDType::Deuteron == rich->bestParticleID() ){
        rich->setBestParticleID( Rich::ParticleIDType::BelowThreshold );
      }
    }

    // add calorimeter hypos
    for( auto calo : *calo_vector ){
      if( calo->clID() == LHCb::CLID_CaloHypo ) proto->addToCalo( dynamic_cast<LHCb::CaloHypo*>( calo ) );
    }

    if( calo_vector->size()>0 ){
      if ( msgLevel(MSG::DEBUG) ){
        debug() << "Proto has " << proto->calo().size() << " calo hypotheses" << endmsg;
        if ( proto->calo().size() > 0 ) {
          debug() << "First hypo has " << proto->calo().front()->clusters().size() << " clusters " << endmsg;
          //debug() << "First CaloCluster has "
          //        << proto->calo().front()->clusters().front()->entries().size() << " entries" << endmsg;
        }
      }
    }
  }
}

void TeslaReportAlgo::fillVertexInfo(LHCb::Vertex* vert, const LHCb::HltObjectSummary* obj){

  // What keys are present:
  const LHCb::HltObjectSummary::Info Vert_info = obj->numericalInfo();
  std::vector<std::string> vec_keys_v = obj->numericalInfoKeys();
  if ( msgLevel(MSG::DEBUG) ) {
    debug() << "Available information (vertex)" << endmsg;
    for(std::vector<std::string>::iterator it = vec_keys_v.begin(); it!=vec_keys_v.end(); it++)
      debug() << *it << " = " << Vert_info[(*it)] << endmsg;
  }
  //
  m_conv->VertexObjectFromSummary(&Vert_info,vert,true);
  if ( msgLevel(MSG::DEBUG) ) debug() << "Decay vertex chi2 = " << vert->chi2() << endmsg;

  return;
}

LHCb::CaloDigit* TeslaReportAlgo::DigitSearchRaw(LHCb::CaloCellID id){

  // Ecal
  LHCb::CaloDigits * caloDigits = getIfExists<LHCb::CaloDigits>( LHCb::CaloDigitLocation::Ecal );
  if( caloDigits ){
    for( auto calo : *caloDigits ){
      if( calo->cellID() == id ) {
        if ( msgLevel(MSG::DEBUG) ) debug() << "Digit match found (ECAL), energy = " << calo->e() << endmsg;
        return calo;
      }
    }
  }

  // Hcal
  LHCb::CaloDigits * caloDigitsH = getIfExists<LHCb::CaloDigits>( LHCb::CaloDigitLocation::Hcal );
  if( caloDigitsH ){
    for( auto calo : *caloDigitsH ){
      if( calo->cellID() == id ) {
        if ( msgLevel(MSG::DEBUG) ) debug() << "Digit match found (HCAL), energy = " << calo->e() << endmsg;
        return calo;
      }
    }
  }

  // SPD
  LHCb::CaloDigits * caloDigitsS = getIfExists<LHCb::CaloDigits>( LHCb::CaloDigitLocation::Spd );
  if( caloDigitsS ){
    for( auto calo : *caloDigitsS ){
      if( calo->cellID() == id ) {
        if ( msgLevel(MSG::DEBUG) ) debug() << "Digit match found (SPD), energy = " << calo->e() << endmsg;
        return calo;
      }
    }
  }

  // PRS
  LHCb::CaloDigits * caloDigitsP = getIfExists<LHCb::CaloDigits>( LHCb::CaloDigitLocation::Prs );
  if( caloDigitsP ){
    for( auto calo : *caloDigitsP ){
      if( calo->cellID() == id ) {
        if ( msgLevel(MSG::DEBUG) ) debug() << "Digit match found (PRS), energy = " << calo->e() << endmsg;
        return calo;
      }
    }
  }

  // Default location
  LHCb::CaloDigits * caloDigitsD = getIfExists<LHCb::CaloDigits>( LHCb::CaloDigitLocation::Default );
  if( caloDigitsD ){
    for( auto calo : *caloDigitsD ){
      if( calo->cellID() == id ) {
        if ( msgLevel(MSG::DEBUG) ) debug() << "Digit match found, energy = " << calo->e() << endmsg;
        return calo;
      }
    }
  }
  if ( msgLevel(MSG::DEBUG) ) debug() << "No match found in raw event" << endmsg;
  return NULL;
}

LHCb::RecVertex* TeslaReportAlgo::bestPV(LHCb::Particle* part, LHCb::RecVertex::Container* cont){

  double min_var = -1.0;
  int key = 0;
  for( auto pv : *cont ){
    double ip, chi2;
    StatusCode test = m_dist->distance ( part, pv, ip, chi2 );
    if(m_PV=="Offline"){
      if( (chi2<min_var) || (min_var<0.)){
        min_var = chi2 ;
        key = (pv)->key();
      }
    }
    else if(m_PV=="Online"){
      if( (ip<min_var) || (min_var<0.)){
        min_var = ip;
        key = (pv)->key();
      }
    }
    else{
      warning() << "Choose a valid PV requirement or use the default (offline)" << endmsg;
    }
  }
  return (LHCb::RecVertex*)cont->containedObject( key );

}

void TeslaReportAlgo::AddRelInfo(const LHCb::HltObjectSummary* Obj_d, LHCb::Particle* Part){
  if ( msgLevel(MSG::DEBUG) ) debug() << "Found RelatedInfo" << endmsg;
  const LHCb::HltObjectSummary::Info loc_info = Obj_d->numericalInfo();
  int loc_enum = (int)loc_info["0#LocationID"];
  std::stringstream locRelInfo;
  locRelInfo << "/Event/" << m_OutputPref << m_inputName << "/";
  auto relInfoLoc = m_RelInfoLocationsMap.find(loc_enum);
  if (relInfoLoc==m_RelInfoLocationsMap.end()) {
    error() << "Requested to save RelatedInfo for ID " << loc_enum
            << " but it is not registered in the ANNSvc, skipping!"
            << endmsg;
  }
  else{
    locRelInfo << relInfoLoc->second;
  }
  if ( msgLevel(MSG::DEBUG) ) debug() << "Placing RelInfo at location ("
                                      << loc_enum << "): " << locRelInfo.str() << endmsg;
  if (Obj_d->substructure().size() > 0){
    const LHCb::HltObjectSummary* RelInfo_sub = Obj_d->substructure()[0].target();
    const LHCb::HltObjectSummary::Info rel_info = RelInfo_sub->numericalInfo();
    LHCb::Relation1D<LHCb::Particle,LHCb::RelatedInfoMap>* rel_RelInfo
       = new LHCb::Relation1D<LHCb::Particle,LHCb::RelatedInfoMap>();
    LHCb::RelatedInfoMap* relmap = new LHCb::RelatedInfoMap();
    m_conv->GenericMapObjectFromSummary(&rel_info,relmap);
    rel_RelInfo->relate(Part,*relmap);
    put( rel_RelInfo , locRelInfo.str() );
  }
}

LHCb::CaloCluster* TeslaReportAlgo::compareClusterPosition(LHCb::CaloCluster* calo, LHCb::CaloClusters* cont){
  auto found = std::find_if(cont->begin(),cont->end(),
      [&calo](LHCb::CaloCluster* icalo){ return std::sqrt( pow(calo->position().x()-icalo->position().x(),2.0)
        + pow(calo->position().y()-icalo->position().y(),2.0)
        + pow(calo->position().z()-icalo->position().z(),2.0)
        + pow(calo->position().e()-icalo->position().e(),2.0) )<0.01; });
  if(found!=cont->end()) return *found;
  return calo;
}
LHCb::CaloHypo* TeslaReportAlgo::compareHypoPosition(LHCb::CaloHypo* calo, LHCb::CaloHypos* cont){
  auto found = std::find_if(cont->begin(),cont->end(),
      [&calo](LHCb::CaloHypo* icalo){ return std::sqrt( pow(calo->position()->x()-icalo->position()->x(),2.0)
        + pow(calo->position()->y()-icalo->position()->y(),2.0)
        + pow(calo->position()->z()-icalo->position()->z(),2.0)
        + pow(calo->position()->e()-icalo->position()->e(),2.0) )<0.01; });
  if(found!=cont->end()) return *found;
  return calo;
}
LHCb::CaloHypo* TeslaReportAlgo::compareHypoPosition(LHCb::CaloHypo* calo, SmartRefVector<LHCb::CaloHypo> cont){
  auto found = std::find_if(cont.begin(),cont.end(),
      [&calo](LHCb::CaloHypo* icalo){ return std::sqrt( pow(calo->position()->x()-icalo->position()->x(),2.0)
        + pow(calo->position()->y()-icalo->position()->y(),2.0)
        + pow(calo->position()->z()-icalo->position()->z(),2.0)
        + pow(calo->position()->e()-icalo->position()->e(),2.0) )<0.01; });
  if(found!=cont.end()) return *found;
  return calo;
}

LHCb::CaloCluster* TeslaReportAlgo::processCluster(const LHCb::HltObjectSummary* ObjBasic, std::vector<ContainedObject*>* calo_vector, LHCb::CaloClusters* cont_CaloClust){
  debug() << "Adding CaloCluster details" << endmsg;
  // Need to make new CaloCluster and seed Digit
  LHCb::CaloCluster* calotemp = new LHCb::CaloCluster();

  std::vector<LHCb::LHCbID> ids = ObjBasic->lhcbIDs();

  if ( msgLevel(MSG::DEBUG) ) {
    debug() << "Associated CALO LHCbIDs:" << endmsg;
    for(unsigned int i=0;i<ids.size();i++){
      debug()<<i<<"\t"<<ids[i].detectorType()<<"\t"<<ids[i].lhcbID()<<endmsg;
    }
  }
  // reports save seed LHCbID as the first one
  LHCb::CaloCellID cellID = (*(ids.begin())).caloID();

  // make digits from our LHCb IDs
  std::vector<LHCb::CaloDigit*> digits;
  for( auto id : ids ){
    LHCb::CaloCellID i_cellID = id.caloID();
    LHCb::CaloDigit* i_digit = DigitSearchRaw(i_cellID);
    if(i_digit) digits.push_back(i_digit);
  }

  const LHCb::HltObjectSummary::Info Calo_info = ObjBasic->numericalInfo();
  // What keys are present:
  std::vector<std::string> vec_keys_calo = ObjBasic->numericalInfoKeys();
  if ( msgLevel(MSG::DEBUG) ) {
    debug() << "Available information (calo)" << endmsg;
    for(std::vector<std::string>::iterator it = vec_keys_calo.begin(); it!=vec_keys_calo.end(); it++)
      debug() << *it << " = " << Calo_info[(*it)] << endmsg;
  }
  m_conv->CaloClusterObjectFromSummary(&Calo_info,calotemp,turbo);
  LHCb::CaloCluster* calo = compareClusterPosition(calotemp,cont_CaloClust);
  if ( msgLevel(MSG::DEBUG) ) debug() << "Compared cluster position" << endmsg;
  if(calo!=calotemp){
    if ( msgLevel(MSG::DEBUG) ) debug() << "Found already inserted cluster, returning that one" << endmsg;
    delete calotemp;
    return calo;
  }
  calo->setSeed(cellID);

  std::vector<LHCb::CaloClusterEntry> entries;
  for( auto digit : digits ){
    if ( msgLevel(MSG::DEBUG) ) debug() << "Digit found to adding to cluster" << endmsg;
    LHCb::CaloClusterEntry entry;
    entry.setStatus( LHCb::CaloDigitStatus::UseForEnergy );
    entry.setDigit(digit);
    entries.push_back(entry);
  }
  calo->setEntries(entries);
  calo_vector->push_back(calo);

  return calo;
}
