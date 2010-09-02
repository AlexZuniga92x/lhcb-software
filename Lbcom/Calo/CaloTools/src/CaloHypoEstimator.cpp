// $Id: $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 

// local
#include "CaloHypoEstimator.h"
#include "Event/ProtoParticle.h"
#include "GaudiKernel/IRegistry.h"
//-----------------------------------------------------------------------------
// Implementation file for class : CaloHypoEstimator
//
// 2010-08-18 : Olivier Deschamps
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( CaloHypoEstimator );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
CaloHypoEstimator::CaloHypoEstimator( const std::string& type,
                                      const std::string& name,
                                      const IInterface* parent )
  : GaudiTool ( type, name , parent ),
    m_status(true)
{
  declareInterface<ICaloHypoEstimator>(this);


  declareProperty("Extrapolation", m_extrapol = true);  
  declareProperty("AddSeed"      , m_seed = false);  
  declareProperty("AddNeighbors" , m_neig = false);  
  declareProperty( "ClusterMatchLocation"   , m_cmLoc );
  declareProperty( "ElectronMatchLocation"  , m_emLoc );
  declareProperty( "BremMatchLocation"      , m_bmLoc );
  declareProperty( "NeutralIDLocations"     , m_pidLoc ) ;
  declareProperty( "SkipNeutralID"          , m_skipN=false);
  declareProperty( "SkipChargedID"          , m_skipC=false);
  declareProperty( "SkipClusterMatch"       , m_skipCl=false);
  m_cmLoc= LHCb::CaloAlgUtils::CaloIdLocation("ClusterMatch", context());
  m_emLoc= LHCb::CaloAlgUtils::CaloIdLocation("ElectronMatch", context());
  m_bmLoc= LHCb::CaloAlgUtils::CaloIdLocation("BremMatch", context());
  using namespace LHCb::CaloIdLocation;
  m_pidLoc["Photon"]    = LHCb::CaloAlgUtils::PathFromContext( context() , PhotonID );
  m_pidLoc["Pi0Merged"]  = LHCb::CaloAlgUtils::PathFromContext( context() , MergedID );
  m_pidLoc["PhotonFromMergedPi0"]  = LHCb::CaloAlgUtils::PathFromContext( context() , PhotonFromMergedID );
}

StatusCode CaloHypoEstimator::finalize(){
  IIncidentSvc* inc = incSvc() ;
  if ( 0 != inc ) { inc -> removeListener  ( this ) ; }
  return GaudiTool::finalize();
}

StatusCode CaloHypoEstimator::initialize() {
  StatusCode sc = GaudiTool::initialize(); // must be executed first

  IIncidentSvc* inc = incSvc() ;
  if ( 0 != inc )inc -> addListener  ( this , IncidentType::BeginEvent ) ;
  m_toCalo = tool<ICaloHypo2Calo> ("CaloHypo2Calo", "CaloHypo2Calo", this); 
  std::string seed = m_seed ? "true" : "false";
  std::string line = m_extrapol ? "true" : "false";
  std::string neig = m_neig ? "true" : "false";
  hypo2Calo()->_setProperty("Seed", seed).ignore();
  hypo2Calo()->_setProperty("PhotonLine", line).ignore();
  hypo2Calo()->_setProperty("AddNeighbors", neig).ignore();
  m_electron = tool<ICaloElectron>("CaloElectron","CaloElectron",this);

  clean();
  m_status = true;
  return sc;
}

//=============================================================================
// Destructor
//=============================================================================
CaloHypoEstimator::~CaloHypoEstimator() {} 

//=============================================================================


// ------------
double CaloHypoEstimator::data(const LHCb::CaloCluster* cluster,CaloDataType::DataType type, double def ){  
  double val = def;
  if ( cluster != m_cluster || NULL == cluster){
    clean();
    m_status = estimator(cluster);
    if( !m_status ){
      counter("Cluster estimation failed") += 1;
      clean();
    }
  }
  caloDataType::iterator it = m_data.find( type );
  if( it != m_data.end() )val = m_data[type];  
  return val;
  
}

// ------------
double CaloHypoEstimator::data(const LHCb::CaloHypo* hypo,CaloDataType::DataType type, double def){
  double val = def;
  if ( hypo != m_hypo || NULL == hypo ){
    clean();
    m_status = estimator(hypo);
    if( !m_status){
      counter("Hypo estimation failed") += 1;
      clean();
    }
  }
  caloDataType::iterator it = m_data.find( type );
  if( it != m_data.end() )val = m_data[type];  
  return val;  
}


// ------------ FROM HYPO
bool CaloHypoEstimator::estimator(const LHCb::CaloHypo* hypo){
  using namespace CaloDataType;
  if( NULL == hypo)return false;
  m_hypo = (LHCb::CaloHypo*) hypo;

  LHCb::CaloMomentum mom(hypo);
  m_data[HypoE] = mom.e();
  m_data[HypoEt] = mom.pt();
  m_data[HypoM] = mom.mass(); // for mergedPi0 hypothesis only (->for all clusters - toDo in CaloMergedPi0Alg)
  m_data[ToPrsE] = m_toCalo->energy(*hypo, "Prs");
  m_data[ToPrsM] = m_toCalo->multiplicity(*hypo, "Prs");
  m_data[ToSpdM] = m_toCalo->multiplicity(*hypo, "Spd");    
  // using extra-digits
  const LHCb::CaloHypo::Digits& digits = hypo->digits();
  LHCb::CaloDataFunctor::IsFromCalo<const LHCb::CaloDigit* > isSpd ( DeCalorimeterLocation::Spd );
  LHCb::CaloDataFunctor::IsFromCalo<const LHCb::CaloDigit* > isPrs ( DeCalorimeterLocation::Prs );
  m_data[HypoSpdM]= std::count_if ( digits.begin(),digits.end(),isSpd );
  m_data[HypoPrsM]= std::count_if ( digits.begin(),digits.end(),isPrs );
  double sumPrs = 0;
  for( LHCb::CaloHypo::Digits::const_iterator id = digits.begin(); id != digits.end() ; ++id){ 
    if ( 0 != *id && isPrs ( *id ) ) { sumPrs += (*id)->e(); } 
  }
  m_data[HypoPrsE]=sumPrs;
  
  // electron matching
  if( !m_skipC ){
    double chi2e = 9999;
    double trajL = 9999;
    const LHCb::Track* etrack = NULL;
    if (exist<LHCb::Calo2Track::ITrHypoTable2D> (m_emLoc)) {
      LHCb::Calo2Track::ITrHypoTable2D* etable = get<LHCb::Calo2Track::ITrHypoTable2D> (m_emLoc);
      const LHCb::Calo2Track::ITrHypoTable2D::InverseType::Range range = etable -> inverse()->relations(hypo);
      if ( !range.empty() ){
        chi2e= range.front().weight();   
        etrack = range.front();
        if( NULL != etrack ){
          LHCb::ProtoParticle* dummy = new LHCb::ProtoParticle();
          dummy->setTrack( etrack );
          dummy->addToCalo ( hypo );
          //CaloElectron->caloTrajectory must be after addToCalo
          if ( m_electron->set(dummy) )
            trajL =m_electron->caloTrajectoryL(CaloPlane::ShowerMax,"hypo");
          delete dummy;
        }
      } 
    }else
      counter( "Missing "+ m_emLoc) += 1;

    m_track[CaloMatchType::ElectronMatch]=etrack;
    m_data[ElectronMatch] = chi2e;
    m_data[TrajectoryL]= trajL;

    // brem matching
    double chi2b = 9999;
    const LHCb::Track* btrack = NULL;
    if (exist<LHCb::Calo2Track::ITrHypoTable2D> (m_bmLoc)) {
      LHCb::Calo2Track::ITrHypoTable2D* btable = get<LHCb::Calo2Track::ITrHypoTable2D> (m_bmLoc);
      const LHCb::Calo2Track::ITrHypoTable2D::InverseType::Range range = btable -> inverse()->relations(hypo);
      if ( !range.empty() ){
        chi2b= range.front().weight(); 
        btrack = range.front();
      }
    }else
      counter( "Missing "+ m_bmLoc) += 1; 

    m_track[CaloMatchType::BremMatch]=btrack;
    m_data[BremMatch] = chi2b;
  }
  
  // NeutralID (warning : protect against infinite loop from CaloPhotonIDAlg using this tools with DoD active)
  if( !m_skipN ){
    for( std::map<std::string,std::string>::iterator l = m_pidLoc.begin(); m_pidLoc.end()!=l; ++l){
      std::string loc = l->second;
      std::string  hypothesis =  l->first;
      if ( exist<LHCb::Calo2Track::IHypoEvalTable>( loc ) )
        m_idTable[hypothesis] = get<LHCb::Calo2Track::IHypoEvalTable> ( loc ) ;
      else
        counter( "Missing "+ loc) += 1; 
    }
    std::ostringstream type("");
    type << hypo->hypothesis();
    std::string hypothesis = type.str();
    const LHCb::Calo2Track::IHypoEvalTable* idTable =  NULL ;
    std::map<std::string, LHCb::Calo2Track::IHypoEvalTable*>::iterator it = m_idTable.find(hypothesis);
    if( it != m_idTable.end())idTable = it->second;
    if( NULL != idTable ){
      const LHCb::Calo2Track::IHypoEvalTable::Range range = idTable->relations( hypo ) ;
      if ( !range.empty()  )m_data[NeutralID]= range.front().to();
    }  
  }
  // link 2 cluster :
  const LHCb::CaloCluster* cluster = LHCb::CaloAlgUtils::ClusterFromHypo( hypo );
  m_clusters[CaloClusterType::SplitOrMain]=cluster;
  m_clusters[CaloClusterType::Main]=  LHCb::CaloAlgUtils::ClusterFromHypo( hypo , false); // get the main cluster
  if( 0 != cluster )return estimator( cluster , hypo);
  return false;
}

// ------------ FROM CLUSTER
bool CaloHypoEstimator::estimator(const LHCb::CaloCluster* cluster, const LHCb::CaloHypo* fromHypo){
  using namespace CaloDataType;
  if( NULL ==cluster)return false;
  m_cluster = (LHCb::CaloCluster*) cluster;
  

  if (cluster->entries().empty()) {
    counter("Empty cluster") += 1;
    return false;
  }
  LHCb::CaloCluster::Entries::const_iterator iseed = 
    LHCb::ClusterFunctors::locateDigit(cluster->entries().begin(),cluster->entries().end(), LHCb::CaloDigitStatus::SeedCell);

  if (iseed != cluster->entries().end()) {

    const LHCb::CaloDigit* seed = iseed->digit();
    if (0 == seed) {
      counter("Seed points to NULL") += 1;
      return false;
    }

    double eEcal = cluster->e();
    m_data[ClusterE]  = eEcal;
    m_data[E1]  = seed->e();
    caloDataType::iterator it = m_data.find( HypoE );
    double eHypo = ( it != m_data.end() ) ? m_data[HypoE] : 0;
    m_data[E19] = m_data[ClusterE] > 0. ? (seed->e()) / m_data[ClusterE] : -1.;
    m_data[E1Hypo] = eHypo > 0. ? (seed->e()) / eHypo : -1.;
    LHCb::CaloCellID sid = seed->cellID();
    m_data[CellID] = (double) sid.all();
    m_data[Spread] = cluster->position().spread()(1,1)+cluster->position().spread()(0,0);
    // E4
    std::vector<double> e4s;
    e4s.reserve(4);
    e4s.push_back(0);
    e4s.push_back(0);
    e4s.push_back(0);
    e4s.push_back(0);
    for( LHCb::CaloCluster::Entries::const_iterator ie = cluster->entries().begin() ; cluster->entries().end() != ie ; ++ie){
      const LHCb::CaloDigit* dig = (*ie).digit();
      if( NULL == dig)continue;
      LHCb::CaloCellID id = dig->cellID();
      if( id.area() != sid.area() || abs((int)id.col() - (int)sid.col()) > 1 ||  abs((int)id.row() - (int)sid.row()) > 1)continue;
      if(id.col() <= sid.col() && id.row() >= sid.row() )e4s[0] += dig->e()*(*ie).fraction();
      if(id.col() >= sid.col() && id.row() >= sid.row() )e4s[1] += dig->e()*(*ie).fraction();
      if(id.col() >= sid.col() && id.row() <= sid.row() )e4s[2] += dig->e()*(*ie).fraction();
      if(id.col() <= sid.col() && id.row() <= sid.row() )e4s[3] += dig->e()*(*ie).fraction();    
    }
    
    double e4max = 0;
    for( std::vector<double>::iterator ih = e4s.begin();e4s.end() != ih;++ih){
      if( *ih >= e4max)e4max=*ih;
    }
    m_data[E4]  = e4max;
    m_data[E49] = (m_data[ClusterE]>0) ? e4max /m_data[ClusterE] : 0.;

    
    
    if( NULL == fromHypo){
      // hypo and cluster parameters may produce small difference according to m_toCalo setting (due to extrapolation)
      m_data[ToPrsE] = m_toCalo->energy(*cluster, "Prs");
      m_data[ToPrsM] = m_toCalo->multiplicity(*cluster, "Prs");
      m_data[ToSpdM] = m_toCalo->multiplicity(*cluster, "Spd");    
    }
    double eHcal = m_toCalo->energy(*cluster, "Hcal");
    m_data[ToHcalE]= eHcal;
    m_data[Hcal2Ecal] = (eEcal > 0) ? eHcal/eEcal : 0.;
  }

  // cluster-match chi2
  if( !m_skipCl ){
    const LHCb::CaloCluster* clus = cluster ;
    // special trick for split cluster : use full cluster matching
    if (NULL != fromHypo && fromHypo->hypothesis() == LHCb::CaloHypo::PhotonFromMergedPi0)
      clus = LHCb::CaloAlgUtils::ClusterFromHypo( fromHypo , false); // get the main cluster
    double chi2 = 9999;
    const LHCb::Track* ctrack = NULL;
    if (exist<LHCb::Calo2Track::IClusTrTable> (m_cmLoc)) {
      LHCb::Calo2Track::IClusTrTable* ctable = get<LHCb::Calo2Track::IClusTrTable> (m_cmLoc);
      const LHCb::Calo2Track::IClusTrTable::Range range = ctable -> relations(clus);
      if ( !range.empty() ){
        chi2= range.front().weight();
        ctrack = range.front();
      }
    }else
      counter( "Missing "+ m_cmLoc) += 1;

    m_track[CaloMatchType::ClusterMatch]=ctrack;
    m_data[ClusterMatch] = chi2;
  }
  return true;
}



void CaloHypoEstimator::clean(){
  m_data.clear();
  m_cluster=NULL;
  m_hypo=NULL;
  m_track.clear();
  return;
}
