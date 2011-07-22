// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 
// from LHCb
#include "Relations/RelationWeighted1D.h"
#include "CaloUtils/CaloParticle.h"
// local
#include "Calo2MCTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : Calo2MCTool
//
// 2009-07-27 : Olivier Deschamps
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( Calo2MCTool )

//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
Calo2MCTool::Calo2MCTool( const std::string& type,
                          const std::string& name,
                          const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<ICalo2MCTool>(this);

  declareProperty ( "Hypo2MCTable"    , m_hypo2MCLoc ) ;
  declareProperty ( "Cluster2MCTable" , m_cluster2MCLoc ) ;
  declareProperty ( "Digit2MCTable"   , m_digit2MCLoc = "Relations/" + LHCb::CaloDigitLocation::Default ) ;
  declareProperty ( "Cluster2Digits"  , m_cluster2Digit = false  ); // delegate cluster->digit->MC
  declareProperty ( "Hypo2Cluster"    , m_hypo2Cluster  = true   ); // Direct hypo->MC relation table does yet not exists
  declareProperty ( "Merged2Split"    , m_merged2Split  = false  ); // expert usage (merged->split->cluster DOES NOT WORK so far)
  declareProperty ( "DigitStatusFilter"  , m_sFilter    = LHCb::CaloDigitStatus::UseForEnergy       ) ; // for ..->digit->MC
  // cluster2MC table location depends on context()

  std::string out( context() );
  std::transform( context().begin() , context().end() , out.begin () , ::toupper ) ;
  if( out == "HLT" ){
    m_cluster2MCLoc = "Relations/" + LHCb::CaloClusterLocation::DefaultHlt;
  }else{
    m_cluster2MCLoc = "Relations/" + LHCb::CaloClusterLocation::Default;
  }


}
//=============================================================================
// Destructor
//=============================================================================
Calo2MCTool::~Calo2MCTool() {} 

//=============================================================================


StatusCode Calo2MCTool::initialize(){
  StatusCode sc = GaudiTool::initialize();
  if (sc.isFailure()) return Error("Failed to initialize", sc);
  m_ppsvc = svc<LHCb::IParticlePropertySvc>( "LHCb::ParticlePropertySvc", true);
  if( m_ppsvc == NULL )return StatusCode::FAILURE;

  // incidentSvc
  IIncidentSvc* inc = incSvc() ;
  if ( 0 != inc )inc -> addListener  ( this , IncidentType::BeginEvent ) ;

  //
  if( !m_hypo2Cluster ){
    warning() << "Hypo->MC relation table not yet exists  - use Hypo->Cluster->MC relation" << endmsg;
    m_hypo2Cluster = true;
  }
  
  // init
  m_digit   = NULL; 
  m_cluster = NULL;
  m_hypo    = NULL;
  m_proto   = NULL;
  m_part    = NULL;
  m_digit2MC= NULL;
  m_cluster2MC=NULL;
  m_category = 0 ;
  m_depth    = -1 ;
  //m_hypo2MC = NULL;
  // get fragment tool and propagate properties
  m_tool = tool<ICalo2MCTool>("Calo2MCTool/CaloFragment2MCTool"); // CAUTION : THE TOOL CANNOT BE PRIVATE !!
  m_tool->_setProperty ( "Hypo2MCTable"    , m_hypo2MCLoc    ) ;
  m_tool->_setProperty ( "Cluster2MCTable" , m_cluster2MCLoc ) ;
  m_tool->_setProperty ( "Digit2MCTable"   , m_digit2MCLoc   ) ;
  m_tool->_setProperty ( "Cluster2Digits"  , Gaudi::Utils::toString( m_cluster2Digit ) );
  m_tool->_setProperty ( "Hypo2Cluster"    , Gaudi::Utils::toString( m_hypo2Cluster  ) ); 
  m_tool->_setProperty ( "Merged2Split"    , Gaudi::Utils::toString( m_merged2Split  ) );
  m_tool->_setProperty ( "DigitStatusFilter"  , Gaudi::Utils::toString( m_sFilter    ) ) ;
  

  m_sum = 0.;
  m_maxMC  = NULL ;
  m_bestMC = NULL ;

  // 
  clear();
  return StatusCode::SUCCESS;
}




/*-------------------------- from Part to MC  ------------------------*/
// associate single particle
ICalo2MCTool* Calo2MCTool::from(const LHCb::Particle*   part  ){
  if( part == m_part)return this; // process only if needed    
  
  clear();
  m_depth = 4; // particle level
  if( part == NULL )return this;

  // check the particle is full calorimetric (using caloParticle functionalities)
  LHCb::CaloParticle cPart = LHCb::CaloParticle( (LHCb::Particle*) part );
  if( !cPart.isCalo() ){
    Warning("Cannot associate non-pure calorimetric particle to MC").ignore();
    return this;
  }


  // register final state particles 
  m_parts = cPart.caloEndTree();
  m_part = (LHCb::Particle*) part;
  if( m_parts.size() == 0)m_parts.push_back( part );



  // particle->protoparticle cascade (mandatory)
  m_depth=3; // protoparticle level
  for(LHCb::Particle::ConstVector::const_iterator ip = m_parts.begin() ; m_parts.end() != ip ; ip ++ ){
    const LHCb::Particle* fs = * ip;
    const LHCb::ProtoParticle* proto = fs->proto();
    if( NULL == proto){
      Warning("ProtoParticle point to NULL (should not)").ignore();
      continue;
    }

    addProto( proto , fs );
  }
 StatusCode sc = process();
  if ( sc.isFailure() )Warning("Processing Calo2MCTool from Particle failed").ignore();
  return this;
}


/*-------------------------- from Proto to MC  ------------------------*/
// Accumulate protos->hypo()s [->clusters]
void Calo2MCTool::addProto(const LHCb::ProtoParticle* proto, const LHCb::Particle* parent ){ 
  if( NULL == proto)return;

  // register proto if not yet done
  bool ok = true;
  for(std::vector<const LHCb::ProtoParticle*>::const_iterator ip = m_protos.begin() ; ip != m_protos.end() ; ip++){
    if( *ip == proto){ok = false;break;}
  }
  if(ok)m_protos.push_back( proto ) ;
  if(!ok){
    if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
      debug() << "ProtoParticle appears twice in the same decay chain" << endmsg;
    counter("ProtoParticle appears twice in the same decay chain" ) += 1.;
  }
  

  // proto->hypo cascade (mandatory)
  m_depth = 2; // hypo level
  const SmartRefVector<LHCb::CaloHypo> hypos = proto->calo();  
  if( hypos.empty() )return;
  
  // special treatment for Bremstrahlung electrons 
  bool brem = false ;
  bool charged = false;
  if( NULL != proto-> track() ){   /* charged part->proto */
    charged = true;
    if( NULL != parent)
      if( fabs(parent->momentum().P() - proto->track()->firstState().p())> 1E-4 ){ /* bremstrahlung corrected particle */
      brem = true;
    }
  }
  for(SmartRefVector<LHCb::CaloHypo>::const_iterator it = hypos.begin(); it != hypos.end() ; ++it){
    const LHCb::CaloHypo* hypo = *it;
    if ( !charged 
         || (charged && hypo->hypothesis() == LHCb::CaloHypo::EmCharged )
         || (charged && brem && hypo->hypothesis() == LHCb::CaloHypo::Photon ) )addHypo( hypo );
  }
}

// associate single protoParticle
ICalo2MCTool* Calo2MCTool::from(const LHCb::ProtoParticle*   proto  ){
  if( proto == m_proto)return this; // process only if needed
  clear();
  m_depth = 3; // proto level
  if( proto == NULL )return this;
  m_proto = (LHCb::ProtoParticle*) proto;
  addProto( proto );
  StatusCode sc = process();
  if ( sc.isFailure() )Warning("Processing Calo2MCTool from ProtoParticle failed").ignore();
  return this;
}


/*-------------------------- from Hypo to MC  ------------------------*/
// Accumulate hypos [->clusters]
void Calo2MCTool::addHypo(const LHCb::CaloHypo* hypo){ 
  if( hypo == NULL )return;

  // register hypo if not yet done
  bool ok = true;
  for(std::vector<const LHCb::CaloHypo*>::const_iterator ih = m_hypos.begin() ; ih != m_hypos.end() ; ih++){
    if( *ih == hypo){ok = false;break;}
  }
  if(ok)m_hypos.push_back( hypo ) ;
  if(!ok){
    if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
      debug() << "CaloHypo appears twice in the same decay chain" << endmsg;
    counter("CaloHypo appears twice in the same decay chain" )+=1;
  }
  
  // hypo->cluster cascade if requested
  if( m_hypo2Cluster ){
    m_depth = 1; // cluster level
    const SmartRefVector<LHCb::CaloCluster> clusters = hypo->clusters();

    if( clusters.empty() )return;
    const LHCb::CaloCluster* cluster =
      ( clusters.size() > 1 && hypo->hypothesis() == LHCb::CaloHypo::PhotonFromMergedPi0 ) ?
      *(clusters.begin() + 1 ) : *(clusters.begin()); //@ToDO use cluster::Type (when defined)
    if( NULL == cluster )return ;
    if( clusters.size() !=1 && hypo->hypothesis() != LHCb::CaloHypo::PhotonFromMergedPi0 )counter("ill-defined CaloHypo") += 1;
    addCluster( cluster );
  }
  return;
}
// associate single hypo
ICalo2MCTool* Calo2MCTool::from(const LHCb::CaloHypo*   hypo  ){
  if( hypo == m_hypo)return this; // process only if needed
  clear();
  m_depth = 2; // hypo level
  if( hypo == NULL )return this;
  m_hypo = (LHCb::CaloHypo*) hypo;
  // special case for MergedPi0
  if( hypo->hypothesis() == LHCb::CaloHypo::Pi0Merged && m_merged2Split){
    SmartRefVector<LHCb::CaloHypo> hyps = hypo->hypos();
    if( hyps.empty() )return this;
    LHCb::CaloHypo* hyp1 = *(hyps.begin());
    LHCb::CaloHypo* hyp2 = *(hyps.begin()+1);
    addHypo( hyp1 ); // splitPhoton1
    addHypo( hyp2 ); // splitPhoton2
  }
  else{
    addHypo( hypo ); // mother CaloHypo
  }
  StatusCode sc = process();
  if ( sc.isFailure() )Warning("Processing Calo2MCTool from CaloHypo failed").ignore();
  return this;
}

/*-------------------------- from Cluster to MC  ------------------------*/
// Accumulate clusters [->digits]
void Calo2MCTool::addCluster(const LHCb::CaloCluster*   cluster  ){
  if( cluster == NULL )return;

  // register cluster (if not yet done)
  bool ok = true;
  for(std::vector<const LHCb::CaloCluster*>::const_iterator ic = m_clusters.begin() ; ic != m_clusters.end() ; ic++){
    if( *ic == cluster){ ok = false; break;}
  }
  
  if(ok) m_clusters.push_back( cluster ) ;
  if(!ok){
    if( UNLIKELY( msgLevel(MSG::DEBUG) ) ) 
      debug() << "Warning : CaloCluster appears twice in the same decay chain" << endmsg;
    counter( "Warning : CaloCluster appears twice in the same decay chain" )+=1;
  }
  
  // cluster->digit cascade if requested
  if(m_cluster2Digit){
    m_depth = 0;    
    const std::vector<LHCb::CaloClusterEntry>& entries = cluster->entries() ;
    for( std::vector<LHCb::CaloClusterEntry>::const_iterator ie = entries.begin();ie!=entries.end();++ie){
      LHCb::CaloClusterEntry entry = *ie;
      if ( m_sFilter >= 0 && ( entry.status() & m_sFilter) == 0 ){ continue ; }
      LHCb::CaloDigit* digit = entry.digit();
      if( NULL == digit )continue;
    // check digit is not yet registered 
      ok = true;
      for(std::vector<const LHCb::CaloDigit*>::const_iterator id = m_digits.begin() ; id != m_digits.end() ; id++){
        if( *id == digit){ ok = false; break;}
      }
      if(ok)m_digits.push_back( digit );
    }
  } 
  return;
}
// associate single cluster
ICalo2MCTool* Calo2MCTool::from(const LHCb::CaloCluster*   cluster  ){
  if( cluster == m_cluster)return this; // process only if needed
  clear();
  m_depth = 1;// cluster level
  if( cluster == NULL )return this;
  m_cluster = (LHCb::CaloCluster*) cluster;
  //
  addCluster( cluster );
  //
  StatusCode sc = process();
  if ( sc.isFailure() )Warning("Processing Calo2MCTool from CaloCluster failed").ignore();
  return this; 
}  

/*-------------------------- from Digit to MC  ------------------------*/
// associate single digit
ICalo2MCTool* Calo2MCTool::from(const LHCb::CaloDigit*   digit  ){
  if( digit == m_digit)return this; // process only if needed
  clear();
  m_depth = 0; // digit level
  if( digit == NULL )return this;
  m_digit = (LHCb::CaloDigit*) digit;
  m_digits.push_back( digit ) ;
  StatusCode sc = process();
  if ( sc.isFailure() )Warning("Processing Calo2MCTool from CaloDigit failed").ignore();
  return this; 
}  

/*-------------------------- Generic processing ------------------------*/
StatusCode Calo2MCTool::process(){

  m_sum = 0.;

  if( m_depth == 2 ){
    return Error("DIRECT HYPO->MC RELATION IS NOT YET ALLOWED",StatusCode::FAILURE);
    /*  
        // HYPO -> MC TABLE PRODUCTION DOES NO EXISITS
        if( exist<LHCb::Calo2MC::IHypoTable>(m_hypo2MCLoc ) )
        m_hypo2MC  = get<LHCb::Calo2MC::IHypoTable>( m_hypo2MCLoc );
        if( 0 == m_hypo2MC ) 
        return Warning(" Hypo <-> MC relation table not found at " + m_hypo2MCLoc , StatusCode::FAILURE);
        // 2 - built (particle,weight) map
        for( std::vector<const LHCb::CaloHypo*>::const_iterator ih = m_hypos.begin() ; ih != m_hypos.end() ; ih++){
        const LHCb::CaloHypo* hypo = *ih;
        if( hypo == NULL )continue;
        LHCb::Calo2MC::IHypoTable::Range hypo2MCRange = m_hypo2MC->relations( hypo );
        for( LHCb::Calo2MC::IHypoTable::Range::const_iterator ir = hypo2MCRange.begin();hypo2MCRange.end()!=ir;ir++){
        const LHCb::MCParticle* mcpart = ir->to();
        m_mcMap[mcpart] += ir->weight();
        }
        m_sum += hypo->e();
        }
    */
  } 
  else if( m_depth == 1 ){
    // process clusters matching
    // 1 - get cluster->MC table
    //if( 0 == m_cluster2MC )
    if( exist<LHCb::Calo2MC::IClusterTable>(m_cluster2MCLoc ) )
      m_cluster2MC  = get<LHCb::Calo2MC::IClusterTable>( m_cluster2MCLoc );
    if( 0 == m_cluster2MC ) 
      return Warning(" Cluster <-> MC relation table not found at " + m_cluster2MCLoc , StatusCode::FAILURE);
    
    // 2 - built (particle,weight) map
    for( std::vector<const LHCb::CaloCluster*>::const_iterator ic = m_clusters.begin() ; ic != m_clusters.end() ; ic++){
      const LHCb::CaloCluster* cluster = *ic;
      if( cluster == NULL )continue;
      LHCb::Calo2MC::IClusterTable::Range cluster2MCRange = m_cluster2MC->relations( cluster );
      for( LHCb::Calo2MC::IClusterTable::Range::const_iterator ir = cluster2MCRange.begin();cluster2MCRange.end()!=ir;ir++){
        const LHCb::MCParticle* mcpart = ir->to();
        m_mcMap[mcpart] += ir->weight();
      }
      m_sum += cluster->e();
    }  
  }
  else if( m_depth == 0 ){
    // process digits matching    
    // 1 - get digit->MC table
    //if( 0 == m_digit2MC )
    if( exist<LHCb::Calo2MC::IDigitTable>(m_digit2MCLoc ) )
      m_digit2MC  = get<LHCb::Calo2MC::DigitTable>( m_digit2MCLoc );
    if( 0 == m_digit2MC ) 
      return Warning(" Digit <-> MC relation table not found at " + m_digit2MCLoc , StatusCode::FAILURE);
    // 2 - built (particle,weight) map
    m_mcMap.clear();
    for( std::vector<const LHCb::CaloDigit*>::const_iterator id = m_digits.begin() ; id != m_digits.end() ; id++){
      const LHCb::CaloDigit* digit = *id;
      LHCb::Calo2MC::IDigitTable::Range digit2MCRange = m_digit2MC->relations( digit );
      for( LHCb::Calo2MC::IDigitTable::Range::const_iterator ir = digit2MCRange.begin();digit2MCRange.end()!=ir;ir++){
        const LHCb::MCParticle* mcpart = ir->to();
        m_mcMap[mcpart] += ir->weight();
      }
      m_sum += digit->e(); // frac ??
    }
  }else{
    counter("Undefined calo chain level") += 1;
    return StatusCode::SUCCESS;
  }
  mcDigest();
  verbose() << " Processing Calo2MCTool " << std::endl << descriptor() << endmsg;
  return StatusCode::SUCCESS;
}

void Calo2MCTool::clear(){
  m_mcMap.clear();
  m_treeMap.clear();
  m_digits.clear();
  m_clusters.clear();
  m_hypos.clear();
  m_protos.clear();
  m_parts.clear();
  m_nFrag = 0;
  m_sum = 0.;
  m_maxMC  = NULL ;
  m_bestMC = NULL ;
  m_digit   = NULL; 
  m_cluster = NULL;
  m_hypo    = NULL;
  m_proto   = NULL;
  m_part    = NULL;
  m_digit2MC= NULL;
  m_cluster2MC=NULL;
  m_category = 0 ;
  m_depth    = -1 ;
}
  

void Calo2MCTool::mcDigest(){

  double mcMax = 0.;
  double mcBest = 0.;
  const LHCb::MCParticle* empty = NULL;
  m_maxMC  = empty ;
  m_bestMC = empty ;



  if( m_sum <= 0 )return;
  // loop over contributing particle :
  for( CaloMCTools::CaloMCMap::const_iterator imap = m_mcMap.begin() ; imap != m_mcMap.end() ; imap++){
    const LHCb::MCParticle* mcPart = imap->first;
    double w  = weight( mcPart );
    double q  = quality( mcPart );
    double m = mcPart->momentum().M();    
    // the most contributing MCParticle (with smallest mass when several MCPart with same weight)
    if( w >= mcMax ){
      bool ok = true;
      if( m_maxMC != NULL &&  w == mcMax && m > m_maxMC->momentum().M() )ok= false;
      if(ok){
        mcMax = w;
        m_maxMC = mcPart ;
      }      
    }
    // the best matching MCParticle
    if( q >= mcBest ){
      mcBest = q;
      m_bestMC = mcPart;
    }
    

  } // end loop over MCParticles
  // build MC tree
  // 1- get related MC particle (seed) without any descendant listed in the related mcParticles 
  std::vector<const LHCb::MCParticle*> seeds;
  for( CaloMCTools::CaloMCMap::const_iterator imap = m_mcMap.begin() ; imap != m_mcMap.end() ; imap++){
    const LHCb::MCParticle* mcPart = imap->first;
    const SmartRefVector<LHCb::MCVertex>& vertices = mcPart->endVertices();
    int nProd = 0;
    for( SmartRefVector<LHCb::MCVertex>::const_iterator iv = vertices.begin();iv!=vertices.end();iv++){
      const  SmartRefVector<LHCb::MCParticle> products = (*iv)->products();
      for( SmartRefVector<LHCb::MCParticle>::const_iterator ip = products.begin();ip!=products.end();ip++){
        CaloMCTools::CaloMCMap::iterator it = m_mcMap.find( *ip );
        if( it != m_mcMap.end() )nProd += 1 ;
      }      
    }
    if( nProd == 0) seeds.push_back( mcPart );
  }
  // 2- build the seed upstream tree
  for( std::vector<const LHCb::MCParticle*>::const_iterator is = seeds.begin() ; is!= seeds.end() ; is++){
    const LHCb::MCParticle* seed = *is;
    std::vector<const LHCb::MCParticle*> tree;
    std::string sTree ;
    mcTree( seed , tree, sTree);
    std::stringstream frac("");
    std::stringstream qual("");
    frac << format(" %6.1f %% from : ", weight( seed ) *100. );
    qual << " ( " << format(" %6.1f %% of the MC particle energy contributing",
                            (weight( seed ) == 0)?0: quality(seed) /weight( seed )*100.) << " )";
    sTree = frac.str() + sTree + qual.str(); 
    m_treeMap[ sTree ] = tree;
  }  
}



void Calo2MCTool::mcTree(const LHCb::MCParticle* part, std::vector<const LHCb::MCParticle*>& tree , std::string& sTree){
  if( NULL == part)return;
  tree.push_back( part );
  const LHCb::ParticleProperty* prop = m_ppsvc->find( part->particleID() );
  std::string p = ( NULL != prop) ? prop->name() : "??";
  sTree = p + sTree;
  if( part->mother() != NULL){
    sTree = " -> " + sTree;
    mcTree( part->mother() , tree, sTree );
  }
}

double Calo2MCTool::weight(const LHCb::MCParticle* part){
  if( part == NULL )return 0.;
  if( m_sum <= 0 )return 0.;
  return m_mcMap[part]/m_sum;
}

double Calo2MCTool::quality(const LHCb::MCParticle* part){
  if( part == NULL )return 0.;
  return (part->momentum().E() != 0) ? weight(part) * m_mcMap[part]/part->momentum().E() : 0.;
}


std::string Calo2MCTool::descriptor(){
  std::stringstream ss("");
  ss  << std::endl;
  ss  << "     ---------- Calo MC contribution " ;
  if( NULL != m_part  ) ss << "to particle (pid = " << m_part->particleID().pid() << ")" << std::endl;
  if( m_parts.size() > 1 ) ss << " -> to " << m_parts.size() << " particle(s) -------- " << std::endl;
  if( !m_protos.empty() ) ss << "to " << m_protos.size() << " protoParticle(s) -------- " << std::endl;
  if( !m_hypos.empty() ) ss << "to " << m_hypos.size() << " hypo(s) -------- " << std::endl;
  if( !m_digits.empty() ) ss << "to " << m_digits.size() << " digit(s) -------- " << std::endl;
  if( !m_clusters.empty() ) ss << "to " << m_clusters.size() << " cluster(s) ------- " << std::endl;
  ss  << "     ---- Total calo energy deposit : " << m_sum << " MeV " << std::endl;
  for( std::map<std::string,std::vector<const LHCb::MCParticle*> >::iterator im = m_treeMap.begin() ; m_treeMap.end()!=im;im++){
    ss  << "        -- " << (*im).first << std::endl; 
  }
  
  if( bestMC() != NULL ){
    const LHCb::ParticleProperty* prop = m_ppsvc->find( bestMC()->particleID() );
    std::string p = ( NULL != prop) ? prop->name() : "??";
    ss << "      --> Best matching MCParticle : [" << p << "] ==  (Quality/Weight : " 
       << quality( bestMC() ) <<" / " << weight( bestMC() )<< ")" << std::endl;
  }
  
  if( maxMC() != NULL ){
    const LHCb::ParticleProperty* prop = m_ppsvc->find( maxMC()->particleID() );
    std::string p = ( NULL != prop) ? prop->name() : "??";
    ss << "      --> Maximum weight MCParticle : [" << p << "] == (Quality/Weight : " 
       << quality( maxMC() ) <<" / " << weight( maxMC() )<<")" <<  std::endl;
  }
  
  ss << "      -------------------------------- "; 
  return ss.str();
}


const LHCb::MCParticle* Calo2MCTool::findMCOrBest(LHCb::ParticleID id, double threshold ){
  const LHCb::MCParticle* found = findMC(id,threshold);
  if( NULL != found )return found;
  return bestMC();
}
const LHCb::MCParticle* Calo2MCTool::findMCOrBest(std::string name, double threshold ){
  const LHCb::MCParticle* found = findMC(name,threshold);
  if( NULL != found )return found;
  return bestMC();
}
const LHCb::MCParticle* Calo2MCTool::findMC(std::string name, double threshold ){
  const LHCb::ParticleProperty* prop = m_ppsvc->find( name );
  if ( prop == NULL)return NULL;
  return findMC( prop->particleID() , threshold);
}
const LHCb::MCParticle* Calo2MCTool::findMC(LHCb::ParticleID id, double threshold ){
  double t = threshold;
  LHCb::MCParticle* best  = NULL;
  for( CaloMCTools::CaloMCMap::const_iterator imap = m_mcMap.begin() ; imap != m_mcMap.end() ; imap++){
    const LHCb::MCParticle* mcPart = imap->first;
    if( mcPart->particleID().abspid() != id.abspid() )continue;
    double q = quality( mcPart );
    if( q < t )continue;
    t = q;
    best = (LHCb::MCParticle*) mcPart;
  }
  return best;
}
const LHCb::MCParticle* Calo2MCTool::bestMC(){
  return m_bestMC;
}
const LHCb::MCParticle* Calo2MCTool::maxMC(){
  return m_maxMC;
}
CaloMCTools::CaloMCMap  Calo2MCTool::mcMap(){
  return m_mcMap;  
}


//int Calo2MCTool::mcCategory(){
//}


ICalo2MCTool* Calo2MCTool::fragment(unsigned int i){
  m_nFrag = 0;
  /* CaloHypo level */
  if( m_depth == 2 ){
    m_nFrag = m_hypos.size();
    if( i >= m_nFrag)return NULL;
    if(m_nFrag == 1)return this;
    return m_tool->from( m_hypos[i] );
  /* CaloCluster level */
  }else if( m_depth == 1 ){
    m_nFrag = m_clusters.size();
    if( i >= m_nFrag)return NULL;
    if(m_nFrag == 1)return this;
    return m_tool->from( m_clusters[i] );
  }
  /* CaloDigit level */
  else if( m_depth == 0 ){
    m_nFrag = m_digits.size();
    if( i >= m_nFrag)return NULL;
    if(m_nFrag == 1)return this;
    return m_tool->from( m_digits[i] );
  }
  return NULL;
}


