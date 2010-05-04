//-----------------------------------------------------------------------------
/** @file TAConfig.cpp
 *
 *  Implementation file for Millepede configuration tool : TAConfig
 *
 *  CVS Log :-
 *  $Id: TAConfig.cpp,v 1.37 2010-05-04 09:47:19 jblouw Exp $
 *
 *  @author J. Blouw (johan.blouw@mpi-hd.mpg.de)
 *  @date   12/04/2007
 */
//-----------------------------------------------------------------------------

#include "GaudiKernel/SystemOfUnits.h"
#include "GaudiKernel/ToolFactory.h"

//C++
#include <algorithm>
#include <map>
#include <iostream>
#include <fstream>
// Kernel
#include "Kernel/LHCbID.h"
#include "Kernel/DifTraj.h"
#include "Kernel/AlignTraj.h"
#include "Kernel/LineTraj.h"
//#include "Kernel/SerializeStl.h"

//#include "StringConvert.h"
// Event
#include "TrackKernel/StateTraj.h"
#include "Event/Track.h"
#include "Event/OTTime.h"

#include "STDet/DeSTLayer.h"
#include "STDet/DeITLayer.h"
#include "STDet/DeTTLayer.h"
#include "STDet/DeITSector.h"
#include "STDet/DeSTDetector.h"
#include "STDet/DeITDetector.h"
#include "STDet/DeTTDetector.h"
#include "STDet/DeTTHalfModule.h"
#include "OTDet/DeOTDetector.h"
#include "OTDet/DeOTLayer.h"
#include "OTDet/DeOTQuarter.h"
#include "OTDet/DeOTStation.h"
#include "OTDet/DeOTModule.h"
#include "VeloDet/DeVelo.h"

// Conditiosn
#include "DetDesc/IAlignment.h"
#include "DetDesc/AlignmentCondition.h"
#include "DetDesc/GlobalToLocalDelta.h"
#include "DetDesc/3DTransformationFunctions.h"
//local header files
#include "TAConfig.h"

//#include "Event/STLiteCluster.h"
using namespace LHCb;
using namespace Gaudi;

//MC infos
#include "Linker/LinkedTo.h"
#include "Event/MCParticle.h"
#include "Event/MCHit.h"

DECLARE_TOOL_FACTORY( TAConfig );

// function object to check the value of a map element
template <class K, class V> class value_equals {
private:
  V value;
public:
  // constructor (initialize value to compare with)
  value_equals (const V& v)
    : value(v) {
  }
  // comparison
  bool operator() (std::pair<const K, V> elem) {
    return elem.second == value;
  }
};


//-----------------------------------------------------------------------------

TAConfig::TAConfig( const std::string& type,
                    const std::string& name,
                    const IInterface* parent ):
  GaudiTupleTool ( type, name  ,parent ),
  m_zmean_ot(0),  
  m_Sigzmean_ot(0),
  m_n_dof( 0 ),
  m_tolerance(1e-4),
  velo_detector(false),
  tt_detector(false),
  it_detector(false),
  ot_detector(false),
  m_cntfix(0),
  m_stereoAngle(0),
  m_nAlignObj(1){
  
  // Interface
  declareInterface<ITAConfigTool>(this);
  info() << "Creating TAConfig object" << endreq;
  // options
  declareProperty("MillepedeTool" , m_CentipedeTool  = "Centipede");
  declareProperty("DerivativeTool", m_derivativTool = "Derivatives");
  declareProperty("nTrackModelParameters", m_ntrack_pars = 4 );
  declareProperty("CommonXFraction", m_commonXFraction = 0.69 );
  declareProperty("Degrees_of_Freedom", m_dof );
  declareProperty("Constraint_Equations",m_constraint);
  declareProperty("PenaltyTerms", m_sigma );
  declareProperty("Chi_sq_factor", m_l_chimax = 10.0 );
  declareProperty("residual_cut", m_residual_cut = 20.0 );
  declareProperty("initial_residual_cut", m_ini_residual_cut = 10.0 );
  declareProperty("n_l_stdev", m_nstd = 10 );
  declareProperty("Chi2Scale", m_chi2Scale = 1000 );
  declareProperty("minChi2", m_minChi2= 1000 );
  declareProperty("Velo_left_modules", m_VELOmap_l);
  declareProperty("Velo_right_modules",m_VELOmap_r);
  declareProperty("Fix_alignable_objects", m_fix = true);
  //  declareProperty("FixDofs", m_fix_dofs );
  declareProperty("Fix_first_object", m_fix_first = true);
  declareProperty("Fix_last_object", m_fix_last = true);
  declareProperty("Fix_x_position", m_fix_x = true);
  declareProperty("Fix_y_position", m_fix_y = false);
  declareProperty("Fix_z_position", m_fix_z = false);
  declareProperty("Fix_alpha_rotation", m_fix_a = false);
  declareProperty("Fix_beta_rotation", m_fix_b = false);
  declareProperty("Fix_gamma_rotation", m_fix_g = false);
  //  declareProperty("OT_objects", m_OTmap );
  declareProperty("Constrain_OT", m_OTConstrain); //MD
  declareProperty("OT_system",m_otSys = false );
  declareProperty("OT_station",m_otStation = false );
  declareProperty("OT_layer",m_otLayer = false );
  declareProperty("OT_halflayer",m_otHalfLayer = false );
  declareProperty("OT_quadrant",m_otQuadrant = false );
  declareProperty("OT_module",m_otModule = false );
  declareProperty("OT_moduleShort",m_otModuleShort = false );
  //  declareProperty("IT_objects", m_ITmap );
  declareProperty("IT_system",m_itSys = false );
  declareProperty("IT_station",m_itStation = false );
  declareProperty("IT_box",m_itBox = false );
  declareProperty("IT_layer",m_itLayer = false );
  declareProperty("IT_ladder",m_itLadder = false );
  //  declareProperty("TT_objects", m_TTmap );
  declareProperty("TT_system",m_ttSys = false );
  declareProperty("TT_station",m_ttStation = false );
  declareProperty("TT_layer",m_ttLayer = false );
  declareProperty("IT_alignment_condition_location", m_itCond = "");
  declareProperty("TT_alignment_condition_location", m_ttCond = "");
  declareProperty("OT_alignment_condition_location", m_otCond = "");
  declareProperty("Outlier",m_outlier=100); 
  declareProperty("ConstrainMovement",m_OTConstrainMove); 
  declareProperty("Sim",m_Simulation = false); 
  debug() << "Millepede tool " << m_CentipedeTool << endreq;
  debug() << "Derivative tool " << m_derivativTool << endreq;
  debug() << "nTrackModelParameters " << m_ntrack_pars << endreq;
  debug() << "Degrees_of_Freedom " << m_dof << endreq;
  //MD_DOF = new bool[m_dof.size()];
}

TAConfig::~TAConfig() {
  //MD  delete[] m_DOF;
};

StatusCode TAConfig::Initialize( std::vector<std::string> &m_dets ) {
  m_detectors = m_dets;
  // Millepede tool
  m_Centipede = tool<ICentipede>( m_CentipedeTool );
  if ( m_Centipede == NULL ) {
    error() << "Error instantiating Centipede class" << endreq;
    return StatusCode::FAILURE;
  }  
  // derivatives tool
  m_derivatives = tool<IDerivatives>( "Derivatives");
  // Track extrapolator
  m_extrapolator = tool<ITrackExtrapolator>( "TrackMasterExtrapolator" );
  // - trajecory poca tool
  m_poca = tool<ITrajPoca>( "TrajPoca" );
  // magnetic field tool
  // m_bField = svc<IMagneticFieldSvc>( "MagneticFieldSvc",true );
  // Count number of degrees of freedom
  for( unsigned int i = 0; i < m_dof.size(); i++ ) 
    if ( m_dof[i] )
      m_n_dof++;
  
  m_scaleIter = m_scale;
  return StatusCode::SUCCESS;
}

StatusCode TAConfig::CacheDetElements() {
  // loop over detectors to be aligned and retrieve geometry information
  std::vector<std::string>::iterator t_i;
  info() << "Number of detectors to be aligned: " << m_detectors.end() - m_detectors.begin() << endreq;
  StatusCode sc;
  for (  t_i = m_detectors.begin(); t_i != m_detectors.end(); t_i++ ) {
    info() << "Detector " << *t_i << " will be aligned" << endreq;
    if ( *t_i == "Velo" ) {
      velo_detector = true;
      // geometry information (i.e. nominal positions...)
      m_velo = getDet<DeVelo>(DeVeloLocation::Default);
      m_develo = getDet<DetectorElement>(DeVeloLocation::Default);
      if ( m_velo == 0 ) {
        error() << "Error retrieving pointer to VeLo geometry!" << endmsg;
        return StatusCode::FAILURE;
      }
      // information from conditions db on positions...
      if ( sc.isFailure() ) {
        error() << "Error retrieving VeLo alignment conditions" << endreq;
        return sc;
      }
      // Configure alignment procedure for the VeLo. Conform S.Viret's code
      //      VELOmap.resize( m_velo->numberRightRSensors() + m_velo->numberRightPhiSensors() ) ;
      // 42 sensors... (2 X 21 stations)
      //      sc = DefineVeloMap( m_DETmap );
      if ( sc.isFailure() ) {
        error() << "Error in DefineVeloMap" << endreq;
        return sc;
      }
      info() << "Succesfully defined the velomap" << endreq;
      //      sc = ConfigVelo( m_DETmap );
      if ( sc.isFailure() ) {
        error() << "Error in ConfigVelo" << endreq;
        return sc;
      }
    }
    if ( *t_i == "TT" ) {
      info() << "Get the TT geometry info" << endreq;
      m_tt = getDet<DeSTDetector>(DeSTDetLocation::TT);
      m_dett = getDet<DetectorElement>(DeSTDetLocation::TT);
      tt_detector = true;
      sc = ConfigTT( m_DETmap );
      if ( sc.isFailure() ) {
        return sc;
      }
    }
    if ( *t_i == "IT" ) {
      info() << "Get the IT geometry info" << endreq;
      m_it = getDet<DeSTDetector>(DeSTDetLocation::IT);
      m_deit = getDet<DetectorElement>(DeSTDetLocation::IT);
      it_detector = true;
      sc = ConfigIT( m_DETmap );
      if ( sc.isFailure() ) {
      	return sc;
      }
    }
    if ( *t_i == "OT" ) {
      info() << "Get the OT geometry info" << endreq;
      m_ot = getDet<DeOTDetector>(DeOTDetectorLocation::Default);
      m_deot = getDet<DetectorElement>(DeOTDetectorLocation::Default);
      ot_detector = true;
      sc = ConfigOT( m_DETmap , m_ot);
      
      if ( sc.isFailure() ) {
        return sc;
      }
    }
    info() << "Continue initialization..." << endreq;
    info() << "Number of elements in m_DETmap : " << m_DETmap.size() << endreq;
  }
  
  // Initialize the derivative tool with the number of track parameters,
  // the number of alignable objects, and the degrees of freedom
  // to count the number of alignable objects,
  // we must only take into account entries in 
  // m_rank, which have a different rank number
  std::vector<int> sorted = m_rank;
  sort(sorted.begin(),sorted.end());
  for(unsigned int i = 1; i < sorted.size(); i++)
    if(sorted.at(i) != sorted.at(i-1)) m_nAlignObj++;
  
  info() << "Degrees of freedom: " << m_dof << endreq;
  info() << "Total number of global parameters:" <<  m_n_dof * m_nAlignObj << endreq;
  m_derivatives->initialize( m_ntrack_pars, m_nAlignObj, m_dof );
  
  //fill the m_stereoAngle vector correct
  m_stereoAngle.resize(m_nAlignObj);
  std::map<int, std::pair<double,double> >::iterator iter = m_RnkZAngle.begin();
  for(; iter != m_RnkZAngle.end(); iter++){
    m_stereoAngle.at(iter->first) =  (iter->second).second;
  }
  
  
  
  info() << "Number of elements in m_DETmap : " << m_DETmap.size() << endreq;
  //  m_totStations = m_VeloStations + m_TTStations + m_ITStations + m_OTStations;
  info() << "--> Printing ranking vector..." << endreq;
  info() << "--> Rank = " << m_rank << endreq;
  info() << "--> Finished initialization!" << endreq;
  info() << "--> Configuring Millepede..." << endreq;
  sc = ConfigMillepede();
  if ( sc.isFailure() ) {
    error() << "Error configuring Millepede" << endreq;
    return StatusCode::FAILURE;
  }
  // Ok, Millepede has been initalized, now we can 
  // do the constraints.
  // For Outer Tracker
  if ( ot_detector ) {
    if(m_OTConstrainMove.size() != 0.) sc = ConstrainMovements();
    if( m_OTConstrain.size() != 0. ) sc = ConstrainPositionsOT();
    else if(m_OTConstrain.size() == 0. ) sc = ConstrainLagrangeMultOT();
  }
  
  //zero out the vector
  for(int i=0;i<NumAlignPars();i++)
    {
      m_prevAlipar.push_back(0.);
    }
  
  //   // For Inner Tracker
  //   if ( it_detector ) sc = ConstrainPositions( constrain_it );
  //   // For Trigger Tracker
  //   if ( tt_detector ) sc = ConstrainPositions( constrain_tt );
  //   // For Velo
  //   if ( velo_detector ) sc = ConstrainPositions( constrain_velo );
  
  //   std::vector< double >::const_iterator itSh = m_shearNom.begin();
  //   for(; itSh != m_shearNom.end(); ++itSh){
  //     info() << " shearNom = " << *itSh <<  endreq;
  //     m_shearxz.push_back( (*itSh)/s_zmoy_ot );
  //   }
  //   double shearXZar[m_shearNom.size()];
  //   VectortoArray(m_shearxz,shearXZar);
  //now give values to Centipede
  //  m_Centipede->ConstF(&shearXZar[0],0.);
  return StatusCode::SUCCESS;
}

StatusCode TAConfig::ConstrainLagrangeMultOT(){
  // 1st we have to get the sigma of the z positions;
  // with that we can calculate the shearing terms
  
  //  calculate mean z position
  if(m_otHalfLayer) 
    MeanZHL();
  //get the mean for all other Objects (station,layer etc...)
  if(!m_otHalfLayer)
    MeanZ();

  m_Sigzmean_ot /= m_nAlignObj; // nominator of shearing term
  info()<<" LM ===> m_Sigzmean_ot " << m_Sigzmean_ot << endreq;
  
  for(int a =0; a < m_n_dof*m_nAlignObj; a++){
    m_consTX.push_back(0.);
    m_consTU.push_back(0.);
    m_consTV.push_back(0.);
    m_consTZ.push_back(0.);  
    m_shearXZ.push_back(0.);
    m_shearXU.push_back(0.);
    m_shearXV.push_back(0.);
    m_scaleZ.push_back(0.);
  }
  if(m_otHalfLayer){
    CalcHLLC();
  } else 
    CalcLC();

  //call Centipede
  if(m_dof[0]){
    if(m_otHalfLayer){
      //Cside
      m_Centipede->ConstF(m_consTX,0.);  
      m_Centipede->ConstF(m_shearXZ,0.);  
      m_Centipede->ConstF(m_consTU,0.);  
      m_Centipede->ConstF(m_shearXU,0.);  
      //Aside
      m_Centipede->ConstF(m_consTV,0.);  
      m_Centipede->ConstF(m_shearXV,0.);  
      m_Centipede->ConstF(m_consTZ,0.);  
      m_Centipede->ConstF(m_scaleZ,0.);  
    }
    else{
      m_Centipede->ConstF(m_consTX,0.);  
      m_Centipede->ConstF(m_shearXZ,0.);  
      m_Centipede->ConstF(m_consTU,0.);  
      m_Centipede->ConstF(m_shearXU,0.);  
      //   m_Centipede->ConstF(m_consTV,0.);  
      //m_Centipede->ConstF(m_shearXV,0.);  
      
    }
    
    
  }
  if(m_dof[2]){ 
    m_Centipede->ConstF(m_consTZ,0.);  
    m_Centipede->ConstF(m_scaleZ,0.);  
  }
  
  for(int i =0; i< m_n_dof*m_nAlignObj;i++){  
    info() <<"LM "<< i << " consTX = " << m_consTX[i] << " || consTU = " << m_consTU[i]  << " || consTZ = " << m_consTZ[i]  
           << "     shearXZ " << m_shearXZ[i] << " || shearXU = " << m_shearXU[i] << " || scaleZ = " << m_scaleZ[i] << endreq;
  }
  return StatusCode::SUCCESS;
}


void TAConfig::MeanZHL() {
  double rightZ = 0.; //right detector half
  double leftZ  = 0.; //left detector half
  double meanRight = 0;
  double meanLeft  = 0;;
  double meanRightZ[24];
  double meanLeftZ[24];
  for(int i=0;i<24;i++){
    meanRightZ[i] = 0.; 
    meanLeftZ[i] = 0.; 
  }
  std::map<int, std::pair<double,double> >::iterator iter = m_RnkZAngle.begin();
  for(; iter != m_RnkZAngle.end(); iter++){
    info() << "rank " << iter->first << " stereo " <<(iter->second).second << endreq;
    //get the right half of the detector
    if((iter->first)%2 == 0){
      rightZ += (iter->second).first; 
      meanRightZ[iter->first] += (iter->second).first/18;	
      info() << "LM right obj = " << iter->first << " zpos = " << (iter->second).first 
	     << " LM meanrightz = "  << " " << meanRightZ[iter->first]  << endreq; 
    }
    //get the left half of the detector
    if((iter->first)%2 != 0) {
      leftZ += (iter->second).first;
      meanLeftZ[iter->first] += (iter->second).first/18;
      info() << "LM left obj = " << iter->first << " zpos = " << (iter->second).first 
	     << " LM meanleft = "  << " " << meanLeftZ[iter->first]  << endreq; 
      
    }
  }
  //divide by 18 = nof modules per halflayer and nof alignable objects per HALF=12!!
  meanRight = rightZ/(9*m_nAlignObj);
  meanLeft  = leftZ/(9*m_nAlignObj);
  info() << "m_nAlignObj " << m_nAlignObj << endreq;
  info() << "LM sums: right side = " << meanRight << " left side " << meanLeft << endreq;
  // get the sigma 
  for(int rnk =0 ; rnk < m_nAlignObj; rnk++){ 
    if(rnk%2 == 0){ 
      m_Sigzmean_ot += (meanRightZ[rnk]-meanRight)*(meanRightZ[rnk]-meanRight);
      m_layerZ.push_back(meanRightZ[rnk]);
    }
    if(rnk%2 != 0) {
      m_Sigzmean_ot += (meanLeftZ[rnk]-meanLeft)*(meanLeftZ[rnk]-meanLeft);
      m_layerZ.push_back(meanLeftZ[rnk]);
    }
  }
  m_misalDetEl_Z = m_layerZ ;
  m_misalDetEl_meanZ = meanLeft;//because a layer was shifted...Aside=Cside...
}

void TAConfig::MeanZ() {
  m_zmean_ot = 0; 
  std::multimap<int, std::pair<double,double> >::iterator iter = m_RnkZAngle.begin();
  for(; iter != m_RnkZAngle.end(); iter++){
    m_zmean_ot += ((iter->second).first)/m_nAlignObj; 
    info() << " LM rank = " << iter->first << " zpos = " << (iter->second).first  << " mean z = " << m_zmean_ot<<endreq; 
  }
  info() << " LM ===> " << " m_zmean_ot = " << m_zmean_ot<<endreq; 
  
  iter  = m_RnkZAngle.begin();
  for(; iter != m_RnkZAngle.end(); iter++){
    m_Sigzmean_ot += ( (iter->second).first-m_zmean_ot)* ( (iter->second).first-m_zmean_ot); 
    m_layerZ.push_back((iter->second).first);
  }
  m_misalDetEl_Z = m_layerZ ;
  m_misalDetEl_meanZ = m_zmean_ot;
}

void TAConfig::CalcHLLC() {
  int cnt;
  for(int rnk =0; rnk< m_nAlignObj;rnk++){
    cnt = 0;
    if ( m_dof[0] ) {
      //Cside
      if(rnk%2==0){
	m_consTX.at(rnk)  = cos(m_stereoAngle[rnk]);
	m_shearXZ.at(rnk) = ((m_layerZ.at(rnk)-m_zmean_ot) / m_Sigzmean_ot)*cos(m_stereoAngle[rnk]);
	m_consTU.at(rnk)  = sin(m_stereoAngle[rnk]); // y
	m_shearXU.at(rnk) = ((m_layerZ.at(rnk)-m_zmean_ot) / m_Sigzmean_ot)*sin(m_stereoAngle[rnk]);// y          
      }
      //Cside
      if(rnk%2!=0){
	m_consTV.at(rnk)  = cos(m_stereoAngle[rnk]);
	m_shearXV.at(rnk) = ((m_layerZ.at(rnk)-m_zmean_ot) / m_Sigzmean_ot)*cos(m_stereoAngle[rnk]);
	m_consTZ.at(rnk)  = sin(m_stereoAngle[rnk]); // y
	m_scaleZ.at(rnk) = ((m_layerZ.at(rnk)-m_zmean_ot) / m_Sigzmean_ot)*sin(m_stereoAngle[rnk]);
      }
      cnt++;
    }
    if ( m_dof[2] ) {
      m_consTZ.at(rnk+cnt*m_nAlignObj) = 1.;
      m_scaleZ.at(rnk+cnt*m_nAlignObj) = (m_layerZ.at(rnk)-m_zmean_ot) / m_Sigzmean_ot;
      cnt++;
    }
    if ( m_dof[3] ) {
      m_consRa.at(rnk+cnt*m_nAlignObj) = 1.;
      cnt++;
    }
    if ( m_dof[4] ) {
      m_consRb.at(rnk+cnt*m_nAlignObj) = 1.;
      cnt++;
    }
    if ( m_dof[5] ) {
      m_consRc.at(rnk+cnt*m_nAlignObj) = 1.;
      cnt++;
    }
  }
}
void TAConfig::CalcLC() {
  int cnt;
  for(int rnk =0; rnk< m_nAlignObj;rnk++){
    cnt = 0;
    if ( m_dof[0] ) {
      m_consTX.at(rnk)  = cos(m_stereoAngle[rnk]);
      m_shearXZ.at(rnk) = ((m_layerZ.at(rnk)-m_zmean_ot) / m_Sigzmean_ot)*cos(m_stereoAngle[rnk]);
      if(m_stereoAngle[rnk]!=0.){
	m_consTU.at(rnk)  = cos(m_stereoAngle[rnk])+sin(m_stereoAngle[rnk]); // y;
	m_shearXU.at(rnk) = ((m_layerZ.at(rnk)-m_zmean_ot) / m_Sigzmean_ot)*cos(m_stereoAngle[rnk])
	  +((m_layerZ.at(rnk)-m_zmean_ot) / m_Sigzmean_ot)*sin(m_stereoAngle[rnk]);// y          ;
      }
      // search an element with value rnk
      std::map<std::string, int>::iterator pos;
      pos = find_if(m_C_pos.begin(),m_C_pos.end(),    // linear complexity
		    value_equals<std::string,int>(rnk));
      if (pos != m_C_pos.end()) {
	info() << "---> LM " << rnk << " " << pos->first << " " << pos->second << " z " <<m_layerZ[rnk] <<" m_consTX = " << m_consTX[rnk] << " m_shearXZ = " << m_shearXZ[rnk] 
	       << " stereo = " << m_stereoAngle[rnk] << endreq;
      }
      cnt++;
    }
    if ( m_dof[2] ) {
      m_consTZ.at(rnk+cnt*m_nAlignObj) = 1.;
      m_scaleZ.at(rnk+cnt*m_nAlignObj) = (m_layerZ.at(rnk)-m_zmean_ot) / m_Sigzmean_ot;
      cnt++;
    }
    if ( m_dof[3] ) {
      m_consRa.at(rnk+cnt*m_nAlignObj) = 1.;
      cnt++;
    }
    if ( m_dof[4] ) {
      m_consRb.at(rnk+cnt*m_nAlignObj) = 1.;
      cnt++;
    }
    if ( m_dof[5] ) {
      m_consRc.at(rnk+cnt*m_nAlignObj) = 1.;
      cnt++;
    }
  }
}


// StatusCode TAConfig::ConstrainPositions( std::map<std::string,int> &map ) {
//   std::string name;
//   int rank;
//   std::map<std::string,int>::iterator it = map.begin();
//   // SV uses Millepede's ParSig function a little strange.
//   // we now have to explicitly set the range within which the
//   // alignment parameters may vary.
//   //MD  for ( ; it != map.end(); it++ ) {
//   //     rank = it->second;
//   //     for ( unsigned int i = 0; i < m_dof.size(); i++ ) {
//   //       m_Centipede->ParSig(rank+i*m_nAlignObj, 20.0 );
//   //     }
//   //   }
//   //it = map.begin();
//   for ( ; it != map.end(); it++ ) {
//     name = it->first;
//     rank = it->second;
//     ConstrainPositions( rank, name, m_nAlignObj );
//   }
//   info() << "(2) Map = " << map << endreq;
//   return StatusCode::SUCCESS;
// }

StatusCode TAConfig::ConfigTT( std::vector<Gaudi::Transform3D> & TTmap ) {
  unsigned int nStations = m_tt->nStation();
  //  tt = TTmap.size();
  Gaudi::Rotation3D Rotation;
  Gaudi::XYZVector position;
  // =========================
  // First define the 'map': this is a vector with
  // positions in 3d of the alignable detector elements
  // =========================
  //m_dett->geometry()->matrixInv().GetDecomposition( Rotation, position);
  m_dett->geometry()->toLocalMatrix().GetDecomposition( Rotation, position);
  info() << "TT system name & position: " << m_dett->name() << " "
         << position.x() << ", "
         << position.y() << ", "
         << position.z() << endreq;
  if ( m_ttSys && ! m_ttStation && ! m_ttLayer ) {
    TTmap.push_back( m_dett->geometry()->toLocalMatrix() );
    AlignmentCondition *tt_cond = const_cast<AlignmentCondition*>
      (m_dett->geometry()->alignmentCondition());
    m_ALImap.push_back( tt_cond->offNominalMatrix() ); // save the alignment conditions of the tt sys
  }
  
  m_zmoy_tt = 0;
  int rank = m_rank.size();
  m_TTStations = m_dett->childIDetectorElements();
  for ( unsigned int i = 0; i < m_TTStations.size(); i++ ) {
    info() << "TT station name = " << m_TTStations[i]->name() << endreq;
    m_TTLayers = m_TTStations[i]->childIDetectorElements();
    for ( unsigned int j = 0; j < m_TTLayers.size(); j++ ) {
      info() << "TT layer name = " << m_TTLayers[j]->name() << endreq;
      CreateMap( rank, m_TTLayers[j], m_zmoy_tt );
      if ( m_fix ) {
        if ( i == 1 && m_fix_last ) {
          if ( j == 1 ) 
            constrain_tt.insert( make_pair( m_TTLayers[j]->name(), rank ) );
        } else if ( i == 0 && m_fix_first ) {
          if ( j == 0 ) 
            constrain_tt.insert( make_pair( m_TTLayers[j]->name(), rank ) );
        }
      }
      rank++;
    }
  }
  m_zmoy_tt /= nStations;
  
  for ( unsigned int i = 0; i < m_TTStations.size(); i++ ) {
    m_TTLayers = m_TTStations[i]->childIDetectorElements();
    for ( unsigned int j = 0; j < m_TTLayers.size(); j++ ) {
      (m_TTLayers[j]->geometry()->toLocalMatrix()).GetDecomposition(Rotation, position);
      s_zmoy_tt += (position.z()-m_zmoy_tt)*(position.z()-m_zmoy_tt);
    }
  }
  s_zmoy_tt /= nStations;
  return StatusCode::SUCCESS;
}

StatusCode TAConfig::ConfigOT( std::vector<Gaudi::Transform3D> &OTmap , DeOTDetector* deotdet) {
  //MD unsigned int nStations = m_ot->nStation();
  //nStations declared below...
  if ( m_otSys && ! m_otStation && ! m_otLayer && ! m_otQuadrant && ! m_otModule ) {
    OTmap.push_back( m_deot->geometry()->toGlobalMatrix() );
    AlignmentCondition *ot_cond = const_cast<AlignmentCondition*>
      (m_deot->geometry()->alignmentCondition() );
    m_ALImap.push_back( ot_cond->offNominalMatrix() );
  }
  
  m_zmean_ot = 0;
  int rank   = m_rank.size();
  int mrank  = 0;  //count ranks for m_otHalfLayer
  int nQua   = 0;  // #alignable objects per quarter
  int nLay   = 0;  // #alignable objects per layer
  int nObj   = 0;  // #alignable objects in total
  
  if(m_otStation)     {nObj = 3;}
  if(m_otLayer)       {nObj = 12;}
  if(m_otHalfLayer)   {nQua = 1; nLay = 2; mrank = -2;  nObj = 24;}
  if(m_otModule)      {nQua = 9; nLay = 18;mrank = -18; nObj = 432;}
  if(m_otModuleShort) {nObj = 432;}
  
  m_misalInput_X.resize(nObj);
  m_misalInput_Y.resize(nObj);
  m_misalInput_Z.resize(nObj);
  m_misalInput_A.resize(nObj);
  m_misalInput_B.resize(nObj);
  m_misalInput_C.resize(nObj);
  
  info() << " m_misalInput_X.resize(m_nAlignObj) = " << m_misalInput_X.size() << endreq;
  m_OTStations = m_deot->childIDetectorElements();
  for ( unsigned int i = 0; i < m_OTStations.size(); i++ ) {
    m_OTLayers = m_OTStations[i]->childIDetectorElements();
    for ( unsigned int j = 0; j < m_OTLayers.size(); j++ ) {
      mrank += nLay;
      int mhalf = (-1);   //count halflayers
      m_OTQuadrants = m_OTLayers[j]->childIDetectorElements();
      for ( unsigned int k = 0; k < m_OTQuadrants.size(); k++ ) {
	
        if(m_otModule || m_otHalfLayer){
          mhalf = mhalf*(-1); // next quadrant belongs to different half
          rank = 0;
          if(mhalf == 1)    rank = mrank;
          if(mhalf == (-1)) rank = mrank+nQua;
        }
	
	
	
        //my counting convention to sort modules in halflayers
        //         -----------------
        //        |        |        |
        //        |   Q3   |  Q2    |
        //        |        |        |
        //        |-----------------|
        //        |        |        |
        //        |   Q1   |   Q0   |
        //        |        |        |
        //         -----------------
        //        mhalf=-1 | mhalf =+1
	
        m_OTModules = m_OTQuadrants[k]->childIDetectorElements();
        for ( unsigned int l = 0; l < m_OTModules.size(); l++ ) {
          /*************************
           ***** HalfLayer**********
           *************************/
	  if ( m_otHalfLayer) {
	    
            debug() << " --> mrank " << mrank << " rank " << rank << endreq
                    << " --> call - CreateMap() - " << endreq;
            CreateMap( rank, m_OTModules[l], m_zmean_ot );
	    
            /********************************
             * in our alignment frame, the 
             * shifts are applied in the global
             * frame, the rotations in the local
             ************************************/
            const Gaudi::Transform3D misalGlobal = m_OTModules[l]->geometry()->toGlobalMatrix();
            const Gaudi::Transform3D nominGlobal = m_OTModules[l]->geometry()->toGlobalMatrixNominal();
            const Gaudi::Transform3D deltaGlobal = misalGlobal*nominGlobal.Inverse();
            const Gaudi::Transform3D deltalocal  = nominGlobal.Inverse()*deltaGlobal*nominGlobal;
            std::vector<double> rotGlobal(3,0.), shiftGlobal(3,0.);
            std::vector<double> rotlocal(3,0.), shiftlocal(3,0.);
            DetDesc::getZYXTransformParameters(deltalocal, shiftlocal, rotlocal);
            DetDesc::getZYXTransformParameters(deltaGlobal, shiftGlobal, rotGlobal);            
	    
	    
	    
            const Gaudi::Transform3D misalGlobalL = m_OTLayers[j]->geometry()->toGlobalMatrix();
            const Gaudi::Transform3D nominGlobalL = m_OTLayers[j]->geometry()->toGlobalMatrixNominal();
            const Gaudi::Transform3D deltaGlobalL = misalGlobalL*nominGlobalL.Inverse();
            const Gaudi::Transform3D deltalocalL  = nominGlobalL.Inverse()*deltaGlobalL*nominGlobalL;
            std::vector<double> rotGlobalL(3,0.), shiftGlobalL(3,0.);
            std::vector<double> rotlocalL(3,0.), shiftlocalL(3,0.);
            DetDesc::getZYXTransformParameters(deltalocalL, shiftlocalL, rotlocalL);
            DetDesc::getZYXTransformParameters(deltaGlobalL, shiftGlobalL, rotGlobalL);            
	    
	    
	    
            std::vector<DeOTLayer*> layers = deotdet->layers();
            unsigned mid = i*4+j;
            debug() <<"stat " << i << " lay " << j <<" name " << m_OTModules[l]->name() << endreq
		    << " mid = >  "<< mid<<endreq;
	    
            DeOTLayer* mlay = layers.at(mid);
            int rnk = rank;
            Gaudi::XYZPoint HlRef(0.,0.,0.);
            Gaudi::XYZPoint HlRefnom(0.,0.,0.);
            GetHalflayerReference(mlay, HlRef,HlRefnom, rnk);
            CreateHalflayerReferenceMap(rnk,HlRefnom);
	    
            debug()  << " Lay rotlocal    = " << rotlocalL  << " shiftlocal  = " <<  shiftlocalL <<endreq
		     << " Lay rotglobal   = " << rotGlobalL << " shiftglobal = " << shiftGlobalL <<  endreq;
	    debug()  << " Module name     = " << m_OTModules[l]->name() << endreq;
            debug()  << " Mod rotlocal    = " << rotlocal  << " shiftlocal  = " <<  shiftlocal <<endreq
		     << " Mod rotglobal   = " << rotGlobal << " shiftglobal = " << shiftGlobal <<  endreq
		     << "---------------------------------------------------------------"<<endreq;
            debug()  << " HL rotlocal    = " << rotlocal  << " shiftlocal  = " <<  HlRef.x()-HlRefnom.x()
		     << " " << HlRef.y()-HlRefnom.y()<< " " << HlRef.z()-HlRefnom.z()<< endreq
		     << "---------------------------------------------------------------"<<endreq;
	    
	    
	    //             ROOT::Math::Transform3D transM( ROOT::Math::XYZVector(shiftlocalL[0], shiftlocalL[1], shiftlocalL[2])) ;
	    //             ROOT::Math::Transform3D rotM( ROOT::Math::RotationX(rotlocalL[0])*ROOT::Math::RotationY(rotlocalL[1])*
	    //                                           ROOT::Math::RotationZ(rotlocalL[2])) ;
            ROOT::Math::Transform3D transM( ROOT::Math::XYZVector(0, 0, 0)) ;
            ROOT::Math::Transform3D rotM( ROOT::Math::RotationX(0)*ROOT::Math::RotationY(0)*
                                          ROOT::Math::RotationZ(0)) ;
            ROOT::Math::Transform3D myglobal = transM * rotM;
            Gaudi::Transform3D locglobal = DetDesc::localDeltaMatrix( m_OTModules[l]->geometry(), myglobal);
	    std::vector<double> locrot(3,0.), locshift(3,0.);
	    // for some reason, the call below also causes the magneticfield svc to update...
	    //m_OTModules[l]->geometry()->ownToOffNominalParams(locshift,locrot);
            const Gaudi::Transform3D misa = m_OTModules[l]->geometry()->toGlobalMatrix();
            const Gaudi::Transform3D nomi = m_OTModules[l]->geometry()->toGlobalMatrixNominal();
            const Gaudi::Transform3D delt = misa*nomi.Inverse();
            const Gaudi::Transform3D del  = nomi.Inverse()*delt*nomi;
            std::vector<double> rot(3,0.), shift(3,0.);
            DetDesc::getZYXTransformParameters(del, shift, rot);
	    //	    m_misalInput_X.at(rank) = HlRef.x()-HlRefnom.x();
	    //	    m_misalInput_Y.at(rank) = HlRef.y()-HlRefnom.y();
	    //	    m_misalInput_Z.at(rank) = HlRef.z()-HlRefnom.z();
	    m_misalInput_X.at(rank) = shiftlocal[0];
	    m_misalInput_Y.at(rank) = shiftlocal[1];
	    m_misalInput_Z.at(rank) = shiftlocal[2];
	    
	    
            m_misalInput_A.at(rank) = rotlocal[0];
            m_misalInput_B.at(rank) = rotlocal[1];
            m_misalInput_C.at(rank) = rotlocal[2];
          }//m_otHalflayer        
          /*************************
           ***** Modules  **********
           *************************/
          if ( m_otModule ) {
            CreateMap( rank, m_OTModules[l], m_zmean_ot );
	    
            /********************************
             * in our alignment frame, the 
             * shifts are applied in the global
             * frame, the rotations in the local
             ************************************/
            const Gaudi::Transform3D misalGlobal = m_OTModules[l]->geometry()->toGlobalMatrix();
            const Gaudi::Transform3D nominGlobal = m_OTModules[l]->geometry()->toGlobalMatrixNominal();
            const Gaudi::Transform3D deltaGlobal = misalGlobal*nominGlobal.Inverse();
            const Gaudi::Transform3D deltalocal  = nominGlobal.Inverse()*deltaGlobal*nominGlobal;
            std::vector<double> rotGlobal(3,0.), shiftGlobal(3,0.);
            std::vector<double> rotlocal(3,0.), shiftlocal(3,0.);
            DetDesc::getZYXTransformParameters(deltalocal, shiftlocal, rotlocal);
            DetDesc::getZYXTransformParameters(deltaGlobal, shiftGlobal, rotGlobal);            
	    
            debug() << rank << "  " <<m_OTModules[l]->name() << endreq;
            debug() << " rotlocal    = " << rotlocal  << " shiftlocal  = " <<  shiftlocal <<endreq
		    << " rotglobal   = " << rotGlobal << " shiftglobal = " << shiftGlobal <<  endreq
		    <<"---------------------------------------------------------------"<<endreq;
	    
	    /*
	      const Gaudi::Transform3D misalGlobalQ = m_OTQuadrants[k]->geometry()->toGlobalMatrix();
	      const Gaudi::Transform3D nominGlobalQ = m_OTQuadrants[k]->geometry()->toGlobalMatrixNominal();
	      const Gaudi::Transform3D deltaGlobalQ = misalGlobalQ*nominGlobalQ.Inverse();
	      const Gaudi::Transform3D deltalocalQ  = nominGlobalQ.Inverse()*deltaGlobalQ*nominGlobalQ;
	      std::vector<double> rotGlobalQ(3,0.), shiftGlobalQ(3,0.);
	      std::vector<double> rotlocalQ(3,0.), shiftlocalQ(3,0.);
	      DetDesc::getZYXTransformParameters(deltalocalQ, shiftlocalQ, rotlocalQ);
	      DetDesc::getZYXTransformParameters(deltaGlobalQ, shiftGlobalQ, rotGlobalQ);            
	      
	      const Gaudi::Transform3D misalGlobalL = m_OTLayers[j]->geometry()->toGlobalMatrix();
	      const Gaudi::Transform3D nominGlobalL = m_OTLayers[j]->geometry()->toGlobalMatrixNominal();
	      const Gaudi::Transform3D deltaGlobalL = misalGlobalL*nominGlobalL.Inverse();
	      const Gaudi::Transform3D deltalocalL  = nominGlobalL.Inverse()*deltaGlobalL*nominGlobalL;
	      std::vector<double> rotGlobalL(3,0.), shiftGlobalL(3,0.);
	      std::vector<double> rotlocalL(3,0.), shiftlocalL(3,0.);
	      DetDesc::getZYXTransformParameters(deltalocalL, shiftlocalL, rotlocalL);
	      DetDesc::getZYXTransformParameters(deltaGlobalL, shiftGlobalL, rotGlobalL);            
	      
	      
	      const Gaudi::Transform3D misalGlobalS = m_OTStations[i]->geometry()->toGlobalMatrix();
	      const Gaudi::Transform3D nominGlobalS = m_OTStations[i]->geometry()->toGlobalMatrixNominal();
	      const Gaudi::Transform3D deltaGlobalS = misalGlobalS*nominGlobalS.Inverse();
	      const Gaudi::Transform3D deltalocalS  = nominGlobalS.Inverse()*deltaGlobalS*nominGlobalS;
	      std::vector<double> rotGlobalS(3,0.), shiftGlobalS(3,0.);
	      std::vector<double> rotlocalS(3,0.), shiftlocalS(3,0.);
	      DetDesc::getZYXTransformParameters(deltalocalS, shiftlocalS, rotlocalS);
	      DetDesc::getZYXTransformParameters(deltaGlobalS, shiftGlobalS, rotGlobalS);            
	      
            */
	    
            m_misalInput_X.at(rank) = shiftlocal[0];
            m_misalInput_Y.at(rank) = shiftlocal[1];
            m_misalInput_Z.at(rank) = shiftlocal[2];
            m_misalInput_A.at(rank) = rotlocal[0];
            m_misalInput_B.at(rank) = rotlocal[1];
            m_misalInput_C.at(rank) = rotlocal[2];
	    
            //std::vector<double> rotcond(3,0.), shiftcond(3,0.);
            //const AlignmentCondition* aliCond  = m_OTModules[l]->geometry()->alignmentCondition();
            //const Gaudi::Transform3D deltacond = aliCond->offNominalMatrix();;
            // DetDesc::getZYXTransformParameters(deltacond, shiftcond, rotcond);
            //debug() << " --> M delta SHIFT (cond) = " << shiftcond << "rot = " << rotcond  <<  endreq;            
	    
	    
	    
            rank++;
          }
          /*************************************
           ***** short Modules (2.5m) **********
           *************************************/
          if ( m_otModuleShort ) {
            CreateMap( rank, m_OTModules[l], m_zmean_ot );
	    
            /********************************
             * in our alignment frame, the 
             * shifts are applied in the global
             * frame, the rotations in the local
             ************************************/
            const Gaudi::Transform3D misalGlobal = m_OTModules[l]->geometry()->toGlobalMatrix();
            const Gaudi::Transform3D nominGlobal = m_OTModules[l]->geometry()->toGlobalMatrixNominal();
            const Gaudi::Transform3D deltaGlobal = misalGlobal*nominGlobal.Inverse();
            const Gaudi::Transform3D deltalocal  = nominGlobal.Inverse()*deltaGlobal*nominGlobal;
            std::vector<double> rotGlobal(3,0.), shiftGlobal(3,0.);
            std::vector<double> rotlocal(3,0.), shiftlocal(3,0.);
            DetDesc::getZYXTransformParameters(deltalocal, shiftlocal, rotlocal);
            DetDesc::getZYXTransformParameters(deltaGlobal, shiftGlobal, rotGlobal);            
	    
            debug() << rank << "  " <<m_OTModules[l]->name() << endreq;
            debug() << " rotlocal    = " << rotlocal  << " shiftlocal  = " <<  shiftlocal <<endreq
		    << " rotglobal   = " << rotGlobal << " shiftglobal = " << shiftGlobal <<  endreq
		    <<"---------------------------------------------------------------"<<endreq;
	    
            m_misalInput_X.at(rank) = shiftlocal[0];
            m_misalInput_Y.at(rank) = shiftlocal[1];
            m_misalInput_Z.at(rank) = shiftlocal[2];
            m_misalInput_A.at(rank) = rotlocal[0];
            m_misalInput_B.at(rank) = rotlocal[1];
            m_misalInput_C.at(rank) = rotlocal[2];
            rank++;
          }
        }
        /*************************
         ***** Quarters **********
         *************************/
        if ( m_otQuadrant && ! m_otModule ) {
          CreateMap( rank, m_OTQuadrants[k], m_zmean_ot );
          info() << "De " << m_OTQuadrants[k]->name() << " has rank " << rank << endreq;
	  
          const Gaudi::Transform3D misalGlobal = m_OTQuadrants[k]->geometry()->toGlobalMatrix();
          const Gaudi::Transform3D nominGlobal = m_OTQuadrants[k]->geometry()->toGlobalMatrixNominal();
          const Gaudi::Transform3D deltaGlobal = misalGlobal*nominGlobal.Inverse();
          const Gaudi::Transform3D deltalocal  = nominGlobal.Inverse()*deltaGlobal*nominGlobal;
          std::vector<double> rotGlobal(3,0.), shiftGlobal(3,0.);
          std::vector<double> rotlocal(3,0.), shiftlocal(3,0.);
          DetDesc::getZYXTransformParameters(deltalocal, shiftlocal, rotlocal);
          DetDesc::getZYXTransformParameters(deltaGlobal, shiftGlobal, rotGlobal);            
          debug( ) << " --> delta ROT   (local)  = " << rotlocal    << " shift = " << shiftlocal <<  endreq;
          debug() << " --> delta SHIFT (global) = " << shiftGlobal << " rot   = " << rotGlobal <<  endreq;
	  
          m_misalInput_X.at(rank) = shiftlocal[0];
          m_misalInput_Y.at(rank) = shiftlocal[1];
          m_misalInput_Z.at(rank) = shiftlocal[2];
          m_misalInput_A.at(rank) = rotlocal[0];
          m_misalInput_B.at(rank) = rotlocal[1];
          m_misalInput_C.at(rank) = rotlocal[2];
	  
          rank++;
        }
      }
      /*************************
       ***** Layers   **********
       *************************/
      if ( m_otLayer && ! m_otQuadrant && ! m_otModule ) {
        //create a map with 3DTransformation* objects , i.e. 
        // 3DTransformation* objects =  from OTLayers[j]->geometry()->toGlobalMatrix()
        CreateMap( rank, m_OTLayers[j], m_zmean_ot );
	
        const Gaudi::Transform3D misalGlobal = m_OTLayers[j]->geometry()->toGlobalMatrix();
        const Gaudi::Transform3D nominGlobal = m_OTLayers[j]->geometry()->toGlobalMatrixNominal();
        const Gaudi::Transform3D deltaGlobal = misalGlobal*nominGlobal.Inverse();
        const Gaudi::Transform3D deltalocal  = nominGlobal.Inverse()*deltaGlobal*nominGlobal;
        std::vector<double> rotGlobal(3,0.), shiftGlobal(3,0.);
        std::vector<double> rotlocal(3,0.), shiftlocal(3,0.);
        DetDesc::getZYXTransformParameters(deltalocal, shiftlocal, rotlocal);
        DetDesc::getZYXTransformParameters(deltaGlobal, shiftGlobal, rotGlobal);            
	
        debug() << rank << "  " <<m_OTLayers[j]->name() << endreq;
        debug() << " rotlocal    = " << rotlocal  << " shiftlocal  = " <<  shiftlocal <<endreq
		<< " rotglobal   = " << rotGlobal << " shiftglobal = " << shiftGlobal <<  endreq
		<<"---------------------------------------------------------------"<<endreq;
	
        m_misalInput_X.at(rank) = shiftlocal[0];
        m_misalInput_Y.at(rank) = shiftlocal[1];
        m_misalInput_Z.at(rank) = shiftlocal[2];
        m_misalInput_A.at(rank) = rotlocal[0];
        m_misalInput_B.at(rank) = rotlocal[1];
        m_misalInput_C.at(rank) = rotlocal[2];
	
        rank++; // count the rank of the alignable object
	
      }
    }
    
    if ( m_otStation && ! m_otLayer && ! m_otQuadrant && ! m_otModule ) {
      CreateMap( rank, m_OTStations[i], m_zmean_ot );
      rank++;
    }
  }
  info() << "Printing all alignment pars" << endreq;
  for(int i = 0; i < rank; i++ ) {
    info() << "rank : " << i << " shift x: " << m_misalInput_X.at(i) << endreq;
    info() << "rank : " << i << " shift y: " << m_misalInput_Y.at(i) << endreq;
    info() << "rank : " << i << " shift z: " << m_misalInput_Z.at(i) << endreq;
    info() << "rank : " << i << " rot a: " << m_misalInput_A.at(i) << endreq;
    info() << "rank : " << i << " rot b: " << m_misalInput_B.at(i) << endreq;
    info() << "rank : " << i << " rot c: " << m_misalInput_C.at(i) << endreq;
  }
  info() << "--> size of rank =" <<  rank << endreq;
  
  return StatusCode::SUCCESS;
}

StatusCode TAConfig::ConfigIT( std::vector<Gaudi::Transform3D> &ITmap ) {
  unsigned int nStations = m_it->nStation();
  //  it = ITmap.size();
  Gaudi::Rotation3D Rotation;
  Gaudi::XYZVector position;
  m_deit->geometry()->toLocalMatrix().GetDecomposition( Rotation, position );
  info() << "IT system name & position: " << m_deit->name() << " "
         << position.x() << ", "
         << position.y() << ", "
         << position.z() << endreq;
  if ( m_itSys && ! m_itStation && ! m_itLayer && ! m_itBox ) {
    ITmap.push_back( m_deit->geometry()->toLocalMatrix() );
    AlignmentCondition *it_cond = const_cast<AlignmentCondition*>
      (m_deit->geometry()->alignmentCondition() );
    m_ALImap.push_back( it_cond->offNominalMatrix() );
  }
  
  m_zmoy_it = 0.0;
  int rank = m_rank.size();
  m_ITStations = m_deit->childIDetectorElements();
  for ( unsigned int i = 0; i < m_ITStations.size(); i++ ) { // Loop over stations
    // Note: there are actually 4 IT stations, since one of them is the radiation monitor...
    m_ITBoxes = m_ITStations[i]->childIDetectorElements();
    for ( unsigned int j = 0; j < m_ITBoxes.size(); j++ ) { // Loop over boxes
      m_ITLayers = m_ITBoxes[j]->childIDetectorElements();
      //      info() << "Inv Matrix of De " << m_ITBoxes[j]->name() << " = " 
      //<< m_ITBoxes[j]->geometry()->matrixInv() << endreq;
      for ( unsigned int k = 0; k < m_ITLayers.size(); k++ ) { // Loop over layers
        //	info() << "Inv Matrix of De " << m_ITLayers[k]->name() << " = " 
        //<< m_ITLayers[k]->geometry()->matrixInv() << endreq;
        info() << "Align De " << m_ITLayers[k]->name() << endreq;
        m_ITLadders = m_ITLayers[k]->childIDetectorElements();
        for ( unsigned int l = 0; l < m_ITLadders.size(); l++ ) {
          if ( m_itLadder ) {
            //	    info() << "Inv Matrix of De " << m_ITLadders[l]->name() 
            //<< " = " << m_ITLadders[l]->geometry()->matrixInv() << endreq;
            CreateMap( rank, m_ITLadders[l], m_zmoy_it );
            if ( m_fix ) {
              if ( i == 0 && m_fix_first )  // T1
                if ( k == 0 || k == 1 )
                  constrain_it.insert(make_pair(m_ITLadders[l]->name(),rank));
              if ( i == 3 && m_fix_last ) 
                if ( k == m_ITLayers.size() -1 || k == m_ITLayers.size() -2 )
                  constrain_it.insert(make_pair(m_ITLadders[l]->name(),rank));
            }
            rank++;
          }
        }
        if ( m_itLayer && ! m_itLadder && ! m_itBox && ! m_itStation ) {
          CreateMap( rank, m_ITLayers[k], m_zmoy_it );
          if ( m_fix ) {
            if ( i == 0 && m_fix_first )
              if ( k == 0 || k == 1 )
                constrain_it.insert( make_pair( m_ITLayers[k]->name(), rank ) );
            if ( i == 3 && m_fix_last )
              if ( k == m_ITLayers.size() -1 || k == m_ITLayers.size() -2 )
                constrain_it.insert( make_pair( m_ITLayers[k]->name(), rank ) );
          }
          rank++;
        }
      }
      if ( m_itBox && ! m_itLayer && ! m_itLadder && ! m_itStation) {
        CreateMap( rank, m_ITBoxes[j], m_zmoy_it );
        rank++;
      }
    }
    if ( m_itStation && (! m_itBox || ! m_itLayer) ) {
      CreateMap( rank, m_ITStations[i], m_zmoy_it );
      rank++;
    }
  }
  info() << "(1) Map = " << constrain_it << endreq;
  m_zmoy_it /= nStations;
  s_zmoy_it = 0.0;
  for ( unsigned int i = 0; i < m_ITStations.size(); i++ ) {
    m_ITBoxes = m_ITStations[i]->childIDetectorElements();
    for ( unsigned int j = 0; j < m_ITBoxes.size(); j++ ) {
      m_ITLayers = m_ITBoxes[j]->childIDetectorElements();
      for ( unsigned int k = 0; k < m_ITLayers.size(); k++ ) {
        if ( m_itLayer ) {
          (m_ITLayers[k]->geometry()->toLocalMatrix()).GetDecomposition(Rotation, position);
          s_zmoy_it += (position.z() - m_zmoy_it)*(position.z() - m_zmoy_it);
        }
      }
      if ( m_itBox && ! m_itLayer ) {
        (m_ITBoxes[j]->geometry()->toLocalMatrix()).GetDecomposition(Rotation, position);
        s_zmoy_it += (position.z() - m_zmoy_it)*(position.z() - m_zmoy_it);
      }
    }
    if ( m_itStation && ! m_itBox && ! m_itLayer ) {
      (m_ITStations[i]->geometry()->toLocalMatrix()).GetDecomposition(Rotation, position);
      s_zmoy_it += (position.z() - m_zmoy_it)*(position.z() - m_zmoy_it);
    }
  }
  s_zmoy_it /= nStations;
  return StatusCode::SUCCESS;
}

//MD useless...
// Gaudi::XYZVector TAConfig::Residual( LHCb::Track &t, LHCb::LHCbID &id ) {
//   debug() << "Into TAConfig::Residual (1)" << endreq;
//   Gaudi::XYZVector distance(-999999.9999,-9999.9999,-99999.99999);

//   // Measurement
//   Measurement& hit = t.measurement( id );

//   // State of track at z of hit
//   State &state = t.closestState( hit.z() );

//   // Set refVector in case it was not set before
//   if ( !hit.refIsSet() ) 
//     hit.setRefVector( state.stateVector() );

//   Gaudi::XYZVector bfield;
//   const Gaudi::TrackVector &refVec = hit.refVector();
//   const Gaudi::XYZPoint refPoint( refVec[0], refVec[1], hit.z() );
//   m_bField->fieldVector( refPoint, bfield );
//   const StateTraj refTraj( refVec, hit.z(), bfield );

//   // Get the measurement trajectory
//   const Trajectory& measTraj = hit.trajectory();

//   // Determine initial estimates of s1 and s2
//   double s1 = 0.0; // Assume state is already close to the minimum
//   double s2 = measTraj.arclength( refTraj.position(s1) );

//   // Determine the actual minimum with the Poca tool
//   m_poca->minimize( refTraj, s1, measTraj, s2, distance, m_tolerance );

//   return distance;
// }

void TAConfig::CreateHalflayerReferenceMap( int r,  Gaudi::XYZPoint ref) {
  m_HLRefmap.insert(std::make_pair(r,ref ));
}

void TAConfig::CreateMap( int & r,  IDetectorElement* id, double &m_zmoy ) {
  Gaudi::Rotation3D R;
  Gaudi::XYZVector T;
  info() << "Name of object = " << id->name() << " has rank = " << r << endreq;
  m_C_pos.insert(std::make_pair(id->name(),r ));
  m_rank.push_back( r );
  //info() << " fill alimap "<<endreq;
  if(!m_otQuadrant){
    debug() << "---> fill alimap " <<endreq;
    m_ALImap.push_back( id->geometry()->alignmentCondition()->offNominalMatrix() );
  }
  
  debug() << "--->  fill detmap  !" << endreq;
  m_DETmap.push_back( id->geometry()->toGlobalMatrix() );
  
  //MD store stereo angle of detector element,
  // i.e. of this rank, for later use
  IDetectorElement::IDEContainer::iterator ModIt;
  // in case of Layer, search for Quadrant 
  if(m_otLayer){  
    IDetectorElement::IDEContainer::iterator QuadIt = id->childBegin();
    info() << " 1 child name (from layer) = " << (*QuadIt)->name() << endreq;  
    ModIt = (*QuadIt)->childBegin();
    info() << " 2 child name (from layer) = " << (*ModIt)->name() << endreq;  
  }
  //we have quadrant, so next is module
  if(m_otQuadrant){
    ModIt = id->childBegin();
    info() << " child name ( from Quadrant) = " << (*ModIt)->name() << endreq;  
  }
  
  Gaudi::Rotation3D rotation;
  Gaudi::XYZVector position;
  if(m_otLayer ) 
    (*ModIt)->geometry()->toGlobalMatrixNominal().GetDecomposition( rotation, position);  
  if(m_otModule|| m_otHalfLayer || m_otModuleShort) 
    id->geometry()->toGlobalMatrixNominal().GetDecomposition( rotation, position);  
  //info() << " my rotation  = " << rotation << endreq;
  std::vector<double> rotationParams(9,0); 
  rotation.GetComponents(rotationParams.begin(),rotationParams.end());
  m_stereoAngle.push_back(rotationParams.at(3));
  id->geometry()->toGlobalMatrix().GetDecomposition( R, T );
  m_zmoy += T.z();
  double myz = T.z();
  std::pair<double, double> zstereo(myz,m_stereoAngle.back());
  //  m_RnkPosZ.insert(std::make_pair(r,myz));
  m_RnkZAngle.insert(std::make_pair(r,zstereo));
  
  std::multimap<int, std::pair<double,double> >::iterator t = m_RnkZAngle.find( r );
  if ( t != m_RnkZAngle.end() ) {
    debug() << "key of map (rank) = " << (*t).first 
            << "value of map Zpos = " << (*t).second << endreq;
  }
  //for(unsigned int i=0;i<m_stereoAngle.size();i++){
  //    debug() << i << " m_stereoAngle " << m_stereoAngle.at(i)<< endreq;}
}

StatusCode TAConfig::ConfigMillepede() {
  int num_objects = m_n_dof * m_nAlignObj;
  int nlocal      = m_ntrack_pars;
  for ( int i = 0; i < m_ntrack_pars; i++ )
    m_derLC.push_back(0.0);
  for ( int i = 0; i < num_objects; i++ )
    m_derGB.push_back(0.0);
  //  for (int j = 0; j < 6; j++) {m_DOF[j] = m_dof[j];}   // What are the parameters to align ?
  //  for (int j = 0; j < 9; j++) {m_CONSTRAINT[j] = m_sigma[j];}
  // Needed for Sebastien's Millepede initialization
  for ( unsigned int i = 0; i < m_dof.size(); i++ ) {// loop over degrees of freedom per detector
    m_SIGMA[i] = m_sigma[i];
    info() << "m_SIGMA[" << i << "] = " << m_SIGMA[i] << endreq;
  }
  //info() << "Calling InitMille with m_sigma = " << m_sigma << "..." << endreq;
  /* HD Millepede initialization call */
  m_Centipede->InitMille(num_objects, nlocal,  
                         m_l_chimax, m_nstd, 
                         m_residual_cut, m_ini_residual_cut);
  
  
  return StatusCode::SUCCESS;
}




StatusCode TAConfig::ResetGlVars() {
  debug() << "---> in ResetGlVars() ; ntrack params= "<< m_ntrack_pars  << endreq;
  // Configure the variables needed to fill the C-matrix
  int m_Nlocal = m_ntrack_pars; // number of track parameters
  // Global parameters 
  m_derGB.resize(m_n_dof*m_nAlignObj,0.);
  m_derLC.resize( m_Nlocal ,0.);
  return StatusCode::SUCCESS;
}





// StatusCode TAConfig::ConfMatrix( const double &stereo, 
//                                  const double &rmeas, 
//                                  const double &wght, 
//                                  const double &z_cor,
//                                  double local_vec[], 
//                                  double local_mat[][4] ) {
//   local_vec[0] += wght*rmeas*cos(stereo); //'x'
//   local_vec[1] += wght*rmeas*z_cor*cos(stereo); //'x'
//   local_vec[2] += wght*rmeas*sin(stereo);  //'u'
//   local_vec[3] += wght*rmeas*z_cor*sin(stereo); //'u'

//   local_mat[0][0] += wght*cos(stereo)*cos(stereo);
//   local_mat[0][1] += wght*cos(stereo)*z_cor*cos(stereo);
//   local_mat[0][2] += wght*cos(stereo)*sin(stereo);
//   local_mat[0][3] += wght*cos(stereo)*z_cor*sin(stereo);

//   local_mat[1][0] += wght*z_cor*cos(stereo)*cos(stereo);
//   local_mat[1][1] += wght*z_cor*cos(stereo)*z_cor*cos(stereo);
//   local_mat[1][2] += wght*z_cor*cos(stereo)*sin(stereo);
//   local_mat[1][3] += wght*z_cor*cos(stereo)*z_cor*sin(stereo);

//   local_mat[2][0] += wght*sin(stereo)*cos(stereo);
//   local_mat[2][1] += wght*sin(stereo)*z_cor*cos(stereo);
//   local_mat[2][2] += wght*sin(stereo)*sin(stereo);
//   local_mat[2][3] += wght*sin(stereo)*z_cor*sin(stereo);

//   local_mat[3][0] += wght*z_cor*sin(stereo)*cos(stereo);
//   local_mat[3][1] += wght*z_cor*sin(stereo)*z_cor*cos(stereo);
//   local_mat[3][2] += wght*z_cor*sin(stereo)*sin(stereo);
//   local_mat[3][3] += wght*z_cor*sin(stereo)*z_cor*sin(stereo);

//   return StatusCode::SUCCESS;
// }

StatusCode TAConfig::ConfMatrix( double rmeas, 
                                 double wght,
                                 unsigned int rank, 
                                 const double zpos,
                                 double local_vec[],
                                 double local_mat[][4],
                                 double stereo_angle
                                 ) {
  
  // first we need the derivatives computed for some fixed track parameters and some fixed alignment parameters  
  
  // get track parameters
  std::vector<double> trpar;
  if(m_ntrack_pars==4){
    trpar.push_back(m_trx0[0]);
    trpar.push_back(m_trtx[0]);
    trpar.push_back(m_try0[0]);
    trpar.push_back(m_trty[0]);
  }
  debug() << "---> CONFMAT: trpar= " << trpar[0] << " " << trpar[1] << " " << trpar[2] << " "<< trpar[3] <<endreq;
  
  m_derLC.clear();
  m_derLC.resize( m_ntrack_pars ,0.);
  Gaudi::XYZVector origin( trpar.at(0), trpar.at(2), 0.0);
  Gaudi::XYZVector slopes( trpar.at(1), trpar.at(3), 0.0);
  m_derivatives->SetLocal( m_derLC, zpos , stereo_angle);    
  
  debug() << "---> CONFMAT: m_derLC[0] = " << m_derLC[0] <<endreq
          << "---> CONFMAT: m_derLC[1] = " << m_derLC[1] <<endreq
          << "---> CONFMAT: m_derLC[2] = " << m_derLC[2] <<endreq
          << "---> CONFMAT: m_derLC[3] = " << m_derLC[3] <<endreq
          << "---> CONFMAT: rank = "<<rank <<" zpos = "<<zpos<<" stereo = "<<stereo_angle<<" weight = " << wght<<endreq
          << "---> CONFMAT: origin[0] = " << origin.x()<<" slopes[0] = " << slopes.x()<<endreq
          << "---> CONFMAT: origin[1] = " << origin.y()<<" slopes[1] = " << slopes.y()<<endreq
          << "---> CONFMAT: origin[2] = " << origin.z()<<" slopes[2] = " << slopes.z()<<endreq;
  
  
  if(m_ntrack_pars==4){
    local_vec[0] += wght*rmeas*m_derLC[0]; 
    local_vec[1] += wght*rmeas*m_derLC[1]; 
    local_vec[2] += wght*rmeas*m_derLC[2];  
    local_vec[3] += wght*rmeas*m_derLC[3]; 
    
    local_mat[0][0] += wght*m_derLC[0] * m_derLC[0];
    local_mat[0][1] += wght*m_derLC[0] * m_derLC[1];
    local_mat[0][2] += wght*m_derLC[0] * m_derLC[2];
    local_mat[0][3] += wght*m_derLC[0] * m_derLC[3];
    
    local_mat[1][0] += wght*m_derLC[1] * m_derLC[0];
    local_mat[1][1] += wght*m_derLC[1] * m_derLC[1];
    local_mat[1][2] += wght*m_derLC[1] * m_derLC[2];
    local_mat[1][3] += wght*m_derLC[1] * m_derLC[3];
    
    local_mat[2][0] += wght*m_derLC[2]*m_derLC[0];
    local_mat[2][1] += wght*m_derLC[2]*m_derLC[1];
    local_mat[2][2] += wght*m_derLC[2]*m_derLC[2];
    local_mat[2][3] += wght*m_derLC[2]*m_derLC[3];
    
    local_mat[3][0] += wght*m_derLC[3]*m_derLC[0];
    local_mat[3][1] += wght*m_derLC[3]*m_derLC[1];
    local_mat[3][2] += wght*m_derLC[3]*m_derLC[2];
    local_mat[3][3] += wght*m_derLC[3]*m_derLC[3];
  }
  
  
  return StatusCode::SUCCESS;
}


StatusCode TAConfig::ConfMatrixP( double rmeas, 
                                  double wght,
                                  unsigned int rank, 
                                  const double zpos,
                                  double local_vec[],
                                  double local_mat[][5],
                                  double stereo_angle
                                  ) {
  
  // first we need the derivatives computed for some fixed track parameters and some fixed alignment parameters  
  
  // get track parameters
  std::vector<double> trpar;
  double dRatio = -3.81831e-4;
  double mslope = m_trtx[0] + m_trQ[0]*( 2*zpos + 3*dRatio*zpos*zpos);
  
  trpar.push_back(m_trx0[0]);
  trpar.push_back(mslope);
  trpar.push_back(m_try0[0]);
  trpar.push_back(m_trty[0]);
  
  debug() << "---> CONFMAT: trpar= " << trpar[0] << " " << trpar[1] << " " << trpar[2] 
          << " "<< trpar[3] << " " << trpar[4] << endreq;
  
  m_derLC.clear();
  m_derLC.resize( m_ntrack_pars ,0.);
  Gaudi::XYZVector origin( trpar.at(0), trpar.at(2), 0.0);
  Gaudi::XYZVector slopes( trpar.at(1), trpar.at(3), 0.0);
  m_derivatives->SetLocal( m_derLC, zpos , stereo_angle );    
  
  debug() << "---> CONFMAT: m_derLC[0] = " << m_derLC[0] <<endreq
          << "---> CONFMAT: m_derLC[1] = " << m_derLC[1] <<endreq
          << "---> CONFMAT: m_derLC[2] = " << m_derLC[2] <<endreq
          << "---> CONFMAT: m_derLC[3] = " << m_derLC[3] <<endreq
          << "---> CONFMAT: m_derLC[4] = " << m_derLC[4] <<endreq
          << "---> CONFMAT: rank = "<<rank <<" zpos = "<<zpos<<" stereo = "<<stereo_angle<<" weight = " << wght<<endreq
          << "---> CONFMAT: origin[0] = " << origin.x()<<" slopes[0] = " << slopes.x()<<endreq
          << "---> CONFMAT: origin[1] = " << origin.y()<<" slopes[1] = " << slopes.y()<<endreq
          << "---> CONFMAT: origin[2] = " << origin.z()<<" slopes[2] = " << slopes.z()<<endreq;
  
  
  if(m_ntrack_pars==5){
    local_vec[0] += wght*rmeas*m_derLC[0]; 
    local_vec[1] += wght*rmeas*m_derLC[1]; 
    local_vec[2] += wght*rmeas*m_derLC[2]; 
    local_vec[3] += wght*rmeas*m_derLC[3];  
    local_vec[4] += wght*rmeas*m_derLC[4]; 
    
    local_mat[0][0] += wght*m_derLC[0] * m_derLC[0];
    local_mat[0][1] += wght*m_derLC[0] * m_derLC[1];
    local_mat[0][2] += wght*m_derLC[0] * m_derLC[2];
    local_mat[0][3] += wght*m_derLC[0] * m_derLC[3];  
    local_mat[0][4] += wght*m_derLC[0] * m_derLC[4];
    
    local_mat[1][0] += wght*m_derLC[1] * m_derLC[0];
    local_mat[1][1] += wght*m_derLC[1] * m_derLC[1];
    local_mat[1][2] += wght*m_derLC[1] * m_derLC[2];
    local_mat[1][3] += wght*m_derLC[1] * m_derLC[3];
    local_mat[1][4] += wght*m_derLC[1] * m_derLC[4];
    
    local_mat[2][0] += wght*m_derLC[2] * m_derLC[0];
    local_mat[2][1] += wght*m_derLC[2] * m_derLC[1];
    local_mat[2][2] += wght*m_derLC[2] * m_derLC[2];
    local_mat[2][3] += wght*m_derLC[2] * m_derLC[3];
    local_mat[2][4] += wght*m_derLC[2] * m_derLC[4];
    
    local_mat[3][0] += wght*m_derLC[3] * m_derLC[0];
    local_mat[3][1] += wght*m_derLC[3] * m_derLC[1];
    local_mat[3][2] += wght*m_derLC[3] * m_derLC[2];
    local_mat[3][3] += wght*m_derLC[3] * m_derLC[3];
    local_mat[3][4] += wght*m_derLC[3] * m_derLC[4];
    
    local_mat[4][0] += wght*m_derLC[4] * m_derLC[0];
    local_mat[4][1] += wght*m_derLC[4] * m_derLC[1];
    local_mat[4][2] += wght*m_derLC[4] * m_derLC[2];
    local_mat[4][3] += wght*m_derLC[4] * m_derLC[3];
    local_mat[4][4] += wght*m_derLC[4] * m_derLC[4];
    
  }
  
  
  return StatusCode::SUCCESS;
}




StatusCode TAConfig::ZeroMatrVec(double mat[][4], double vec[]) {
  for (int i=0; i< m_ntrack_pars; ++i) {// reset local params
    vec[i] = 0.0;
    for (int j=0; j< m_ntrack_pars; ++j){
      mat[i][j] = 0.0;
    }
  }
  return StatusCode::SUCCESS;
}
StatusCode TAConfig::ZeroMatrVecP(double mat[][5], double vec[]) {
  for (int i=0; i< m_ntrack_pars; ++i) {// reset local params
    vec[i] = 0.0;
    for (int j=0; j< m_ntrack_pars; ++j){
      mat[i][j] = 0.0;
    }
  }
  return StatusCode::SUCCESS;
}




int TAConfig::InvMatrix(double v[][4], double b[],const int &n) {
  int i, j, jj, k;
  double vkk;
  double eps = 0.0000000000001;
  bool *flag = new  bool[n];
  double *diag = new double[n];
  double *temp = new double[n];
  
  for (i=0; i<n; i++) {
    flag[i] = true;
    diag[i] = fabs(v[i][i]);     // save diagonal elem absolute values 	
    for (j=0; j<=i; j++) {
      v[j][i] = v[i][j] ;
    }
  }
  
  int nrank = 0;
  
  for (i=0; i<n; i++) {
    vkk = 0.0;
    k = -1;
    
    //TRACE("&&& in local inverting fct &&& " << PRINTV(i) << PRINTV(diag[i]) << PRINTV(v[i][i]) );
    
    for (j=0; j<n; j++) {// First look for the pivot, ie max unused diagonal element 
      if (flag[j] && ( (fabs(v[j][j]) > std::max(fabs(vkk),eps*diag[j])) ) ){
        vkk = v[j][j];
        k = j;
      }
    }
    
    if (k >= 0) {  // pivot found
      nrank++;
      flag[k] = false;
      vkk = 1.0/vkk;
      v[k][k] = -vkk; // Replace pivot by its inverse
      for (j=0; j<n; j++) {
        for (jj=0; jj<n; jj++) {
          if (j != k && jj != k) {// Other elements 
            //(!!! do them first as you use old v[k][j]'s !!!)
            v[j][jj] = v[j][jj] - vkk*v[j][k]*v[k][jj];
          }					
        }					
      }
      
      for (j=0; j<n; j++) {
        if (j != k) {// Pivot row or column elements 
          v[j][k] = (v[j][k])*vkk; // Column
          v[k][j] = (v[k][j])*vkk; // Line
        }
      }					
    } else { // No more pivot value (clear those elements)
      for (j=0; j<n; j++) {
        if (flag[j]) {
          b[j] = 0.0;
          for (k=0; k<n; k++) {
            v[j][k] = 0.0;
          }
        }				
      }
      break;  // No more pivots anyway, stop here
    }
  }
  
  for (j=0; j<n; j++) {
    temp[j] = 0.0;
    for (jj=0; jj<n; jj++) { // Reverse matrix elements
      v[j][jj] = -v[j][jj];
      temp[j] += v[j][jj]*b[jj];
    }					
  }
  
  for (j=0; j<n; j++)
    b[j] = temp[j];
  
  delete[] temp;
  delete[] flag;
  delete[] diag;
  return nrank;
}

int TAConfig::InvMatrixP(double v[][5], double b[],const int &n) {
  
  int i, j, jj, k;
  double vkk;
  double eps = 0.0000000000001;
  bool *flag = new  bool[n];
  double *diag = new double[n];
  double *temp = new double[n];
  
  for (i=0; i<n; i++) {
    flag[i] = true;
    diag[i] = fabs(v[i][i]);     // save diagonal elem absolute values 	
    for (j=0; j<=i; j++) {
      v[j][i] = v[i][j] ;
    }
  }
  
  int nrank = 0;
  
  for (i=0; i<n; i++) {
    vkk = 0.0;
    k = -1;
    
    //TRACE("&&& in local inverting fct &&& " << PRINTV(i) << PRINTV(diag[i]) << PRINTV(v[i][i]) );
    
    for (j=0; j<n; j++) {// First look for the pivot, ie max unused diagonal element 
      if (flag[j] && ( (fabs(v[j][j]) > std::max(fabs(vkk),eps*diag[j])) ) ){
        vkk = v[j][j];
        k = j;
      }
    }
    
    if (k >= 0) {  // pivot found
      nrank++;
      flag[k] = false;
      vkk = 1.0/vkk;
      v[k][k] = -vkk; // Replace pivot by its inverse
      for (j=0; j<n; j++) {
        for (jj=0; jj<n; jj++) {
          if (j != k && jj != k) {// Other elements 
            //(!!! do them first as you use old v[k][j]'s !!!)
            v[j][jj] = v[j][jj] - vkk*v[j][k]*v[k][jj];
          }					
        }					
      }
      
      for (j=0; j<n; j++) {
        if (j != k) {// Pivot row or column elements 
          v[j][k] = (v[j][k])*vkk; // Column
          v[k][j] = (v[k][j])*vkk; // Line
        }
      }					
    } else { // No more pivot value (clear those elements)
      for (j=0; j<n; j++) {
        if (flag[j]) {
          b[j] = 0.0;
          for (k=0; k<n; k++) {
            v[j][k] = 0.0;
          }
        }				
      }
      break;  // No more pivots anyway, stop here
    }
  }
  
  for (j=0; j<n; j++) {
    temp[j] = 0.0;
    for (jj=0; jj<n; jj++) { // Reverse matrix elements
      v[j][jj] = -v[j][jj];
      temp[j] += v[j][jj]*b[jj];
    }					
  }
  
  for (j=0; j<n; j++)
    b[j] = temp[j];
  
  delete[] temp;
  delete[] flag;
  delete[] diag;
  return nrank;
}



StatusCode TAConfig::FillMatrix( int rank, 
                                 const struct Point pred,
                                 const double meas, 
                                 const double stereo_angle, 
                                 const double zpos,
                                 double resolution                                
                                 ) {
  
  //-----------------------------
  // prepare Millepede;
  // configure the local (= track model) and global (= geometry) models:
  //-----------------------------
  
  m_derLC.clear();
  m_derGB.clear();
  m_derLC.resize( m_ntrack_pars ,0.);
  m_derGB.resize( m_n_dof*m_nAlignObj ,0.);
  //  m_Centipede->ZerLoc( m_derGB, m_derLC );
  double dRatio = -3.81831e-4;
  double mslope = m_trtx[0] + m_trQ[0]*( 2*zpos + 3*dRatio*zpos*zpos);
  std::vector<double> trt;
  
  trt.push_back(m_trx0[0]);
  trt.push_back(mslope);
  trt.push_back(m_try0[0]);
  trt.push_back(m_trty[0]);
  
  Gaudi::XYZVector origin( trt.at(0), trt.at(2), 0.0);
  Gaudi::XYZVector slopes( trt.at(1), trt.at(3), 0.0);
  //set derivatives
  m_derivatives->SetLocal( m_derLC, zpos, stereo_angle );
  m_derivatives->SetGlobal( slopes, origin, pred, m_derGB, rank, stereo_angle );
  
  debug() << " --> FILLMATRIX after SetGlobal : derGB.size = " << m_derGB.size() << " rank = " << rank << endreq
          << "     track parameter at z = "<< zpos<<" : "<< endreq;
  for(unsigned int i=0;i<trt.size();i++) debug() <<" Param "<< i <<" "<<trt.at(i) <<endreq;
  
  
  
  
  
  //fill arrays of MP       
  m_Centipede->EquLoc( m_derGB, m_derLC, meas, resolution ); 
  
  //exclude objects from global fit that where not hit
  
  return StatusCode::SUCCESS;
}
StatusCode TAConfig::CalcResidual( const LHCb::LHCbID &id, 
                                   int rank, // do we realy need the rank ?
                                   struct Point &meas,
                                   bool fitok,  // need to see if there is a local fit in progress.
                                   double & zpos , //MD
                                   double & stereo,  //MD
                                   double & Zref, //MD
                                   struct Point &locPos//MD
                                   ) {
  //  const Measurement& trMeas = track->measurement(id); // Get measurement
  
  if(id.isOT()){
    
    //    weight = 1./pow(trMeas.errMeasure(),2.);
    //weight = (1./.2)*(1./.2);
    //weight = (1./1.44)*(1./1.44);
    double stereo_angle  = 0.;
    stereo_angle         = m_ot->findLayer( id.otID() )->angle();
    stereo               = stereo_angle;
    
    
    debug() << "------------------------------" <<endreq;
    debug() << "----> CALCRES RANK =  " << rank << "     stereo = "<<stereo_angle << endreq;
    debug() << "------------------------------" <<endreq;
    
    //build previous traj
    //    LineTraj prevtraj = LineTraj(m_pointa,m_pointb);
    // info()<<" 1.   prevtraj = " << prevtraj.direction(0.) << " " << prevtraj.position(0.)<<endreq;
    //MD get another reference without measurement info
    std::auto_ptr<LHCb::Trajectory> lhcbidTraj = m_ot->findModule( id.otID() )->trajectory(id.otID());   
    Gaudi::XYZPoint idTrajPoint                = lhcbidTraj->position(0.);
    Gaudi::XYZVector idTrajDir                 = lhcbidTraj->direction(0.);    
    //    debug() << " --> trajDir = " << idTrajDir << " trajPoint = " << idTrajPoint << endreq;
    //       Gaudi::XYZVector mvec;
    //     double mwire1 = 0.;
    //     double mwire2 = 0.;
    //     m_poca->minimize((*lhcbidTraj),mwire1,prevtraj,mwire2,mvec,m_tolerance);
    //     Gaudi::XYZPoint mpointy1 = lhcbidTraj->position(mwire1);
    //     Gaudi::XYZPoint mpointy2 = prevtraj.position(mwire2);
    
    //    info() << " my new y1 = " << mpointy1 << "  y2 = " << mpointy2<< endreq;
    //meas.y = mpointy1.y();
    //give infos for next round
    //     m_pointa = lhcbidTraj->beginPoint();
    //     m_pointb = lhcbidTraj->endPoint();
    //     info() << " my starting point = " << m_pointa<< " endpoint "<< m_pointb <<endreq
    //            << " middle point " << idTrajPoint << " and dir " << idTrajDir << endreq;
    
    // get sensor where hit appeared
    DeOTLayer*   layer  = m_ot->findLayer( id.otID() );
    DeOTQuarter* quarter= m_ot->findQuarter( id.otID() );
    DeOTModule*  module = m_ot->findModule( id.otID() );
    
    
    debug() << " quarter has support n geometry tree : " << quarter->geometry()->hasSupport() <<endreq;
    
    //get otchannelid and straw and monolayer
    const OTChannelID  chID   = id.otID();
    const unsigned int straw  = chID.straw();
    bool mLayA                = module->monoLayerA(straw);
    
    //   get measurement trajectory and its direction
    //MD    const Trajectory& measTraj = trMeas.trajectory();
    Gaudi::XYZVector trajeDa =  idTrajDir;//MD measTraj.direction(0.);
    Gaudi::XYZVector trajeD;
    //    debug() << " idtrajDira = " << trajeDa << endmsg;    
    if(trajeDa.y() < 0.) //make sure all trajsdirection is upwards    //MD 15-04
      trajeD.SetXYZ(-trajeDa.x()/sqrt(trajeDa.Mag2()),
                    -trajeDa.y()/sqrt(trajeDa.Mag2()),
                    -trajeDa.z()/sqrt(trajeDa.Mag2()));
    else
      trajeD.SetXYZ(trajeDa.x()/sqrt(trajeDa.Mag2()),
                    trajeDa.y()/sqrt(trajeDa.Mag2()),
                    trajeDa.z()/sqrt(trajeDa.Mag2()));
    
    Gaudi::Rotation3D R, Rnom,rModNom,rQuNom,rQuShift;
    Gaudi::XYZVector Lay,Mod,Pnom,sModNom,sQuNom,sQuShift;
    //MD
    //     info() << "--------------------------------------------------------------" << endreq;
    //     debug() << " idtrajDir = " << trajeD << endmsg;
    //     info() << " idtraj   -->  trajPos = " << idTrajPoint << " idtrajDir = " << trajeD << endmsg;
    //     info() << " begin point of idtraj   = " << lhcbidTraj->beginPoint()    << endmsg;
    //     info() << " begin point of meastraj = " << measTraj.beginPoint()    << endmsg;
    //     info() << " end   point of idtraj   = " << lhcbidTraj->endPoint()    << endmsg;
    //     info() << " end   point of meastraj = " << measTraj.endPoint()    << endmsg;
    //     info() << " meastraj.dir = "<< measTraj.direction(0.) << " idtraj = " << lhcbidTraj->direction(0.) << endmsg;
    // Get center of Layer and Rotation matrix in LHCb coordinate frame
    layer->geometry()->toGlobalMatrix().GetDecomposition( R, Lay);
    layer->geometry()->toGlobalMatrixNominal().GetDecomposition( Rnom, Pnom);    
    
    quarter->geometry()->toGlobalMatrix().GetDecomposition( rQuShift, sQuShift);
    quarter->geometry()->toGlobalMatrixNominal().GetDecomposition( rQuNom, sQuNom);    
    
    module->geometry()->toGlobalMatrix().GetDecomposition( R, Mod);    // get center of Module including misalignments
    module->geometry()->toGlobalMatrixNominal().GetDecomposition( rModNom, sModNom);    
    
    const Gaudi::XYZPoint misalLay(Lay.x(),Lay.y(),Lay.z());
    const Gaudi::XYZPoint nomLay(Pnom.x(),Pnom.y(),Pnom.z());
    
    const Gaudi::XYZPoint misalQu(sQuShift.x(),sQuShift.y(),sQuShift.z());
    const Gaudi::XYZPoint nomQu(sQuNom.x(),sQuNom.y(),sQuNom.z());
    
    const Gaudi::XYZPoint misalMod(Mod.x(),Mod.y(),Mod.z());
    const Gaudi::XYZPoint nomMod(sModNom.x(),sModNom.y(),sModNom.z());
    
    debug() << "--> nomLay   ="<<nomLay <<endreq
            << "    misalLay ="<<misalLay << endreq
            << "--> nomQu    ="<<nomQu <<endreq
            << "    misalQu  ="<<misalQu << endreq
            << "--> nomMod   ="<<nomMod <<endreq
            << "    misalMod ="<<misalMod << endreq;
    
    
    
    
    /*********************
     ** new Mod Center ***
     ********************/
    
    std::auto_ptr< LHCb::Trajectory > mLayTraj;
    std::auto_ptr< LHCb::Trajectory > bmLayTraj;
    //get the plane of first monolayer
    if(mLayA){
      //      info() << " use module : " << module->name() << " monoB? " << !mLayA << endreq;
      mLayTraj = module->trajectoryFirstWire(0);//first monolayerRight
      bmLayTraj = module->trajectoryLastWire(0);//first monolayerLeft
      //MD 14-04    
    }
    //get the plane of the second monolayer
    //MD14-04 
    else if(!mLayA){
      mLayTraj = module->trajectoryFirstWire(1);//second monolayerRight
      bmLayTraj = module->trajectoryLastWire(1);//second monolayerLeft
    }
    
    Gaudi::XYZPoint  mLayP  = mLayTraj->position(0.);
    Gaudi::XYZPoint  bmLayP = bmLayTraj->position(0.);
    Gaudi::XYZVector dLay(mLayP.x()-bmLayP.x(),mLayP.y()-bmLayP.y(),mLayP.z()-bmLayP.z());  //dist between two trajs
    Gaudi::XYZPoint  monoMod =  mLayP-0.5*dLay;     //new center of  Monolayer
    // *******************************
    // distance between orig. module center and new monoMod is the same as for layer center and new layer center!!
    // *******************************
    Gaudi::XYZVector monoCenDist;
    monoCenDist.SetXYZ(0.,monoMod.y()-misalMod.y(),monoMod.z()-misalMod.z());
    
    
    /*********************
     ** new Lay Center***
     ********************/
    Gaudi::XYZPoint nomMonolayer(0.,0.,0.);
    Gaudi::XYZPoint nomMonoRef(0.,0.,0.);
    Gaudi::XYZPoint mPivot(0.,0.,0.);
    Gaudi::XYZPoint misalLay_mono(0.,0.,0.);
    Gaudi::XYZPoint misalMod_mono(0.,0.,0.);
    Gaudi::XYZPoint misalRef_mono(0.,0.,0.);
    double misalZpos = 0.;
      
    
    if(m_otLayer){
      nomMonoRef                    = nomLay + monoCenDist;  
      misalRef_mono                 = misalLay + monoCenDist;
      misalMod_mono                 = misalMod + monoCenDist;
      //      misalZpos = misalLay_mono.z();    //get misaligned z position of layer
      misalZpos = misalRef_mono.z();    //get misaligned z position of layer
    }
    
    if(m_otHalfLayer){
      Gaudi::XYZPoint HlRef(0.,0.,0.);
      Gaudi::XYZPoint HlRefnom(0.,0.,0.);
	GetHalflayerReference(layer, HlRef,HlRefnom, rank);
	misalRef_mono             = HlRef + monoCenDist;
	misalZpos                 = misalRef_mono.z();    //get misaligned z position of object
	plot(HlRef.x()-HlRefnom.x(),"HLdx","HLdx",-5,5,100);
	plot(HlRef.y()-HlRefnom.y(),"HLdy","HLdy",-5,5,100);
	plot(HlRef.z()-HlRefnom.z(),"HLdz","HLdz",-5,5,100);
	
      }
      
      if(m_otModule){
	mPivot                        = nomMod   + monoCenDist;
	nomMonoRef                    = nomLay + monoCenDist;  
	misalLay_mono                 = misalLay + monoCenDist;
	misalRef_mono                 = misalMod + monoCenDist;
	/**************************
	 ** module center at
	 ** y==0
	 **************************/
	double toYzero  =  (0.-misalRef_mono.y())/trajeD.y(); 
	Gaudi::XYZVector vecYzero = toYzero*trajeD;
	misalRef_mono             = misalRef_mono + vecYzero;
	misalZpos                 = misalRef_mono.z();    
	debug() << " " << endreq
		<< "--> NEW misalRef_mono " << misalRef_mono << endreq
		<< "    vecYzero          " << vecYzero << "  trajeD  " << trajeD << endreq;
      }
      if(m_otModuleShort){
	mPivot                        = nomMod   + monoCenDist;
	nomMonoRef                    = nomLay + monoCenDist;  
	misalLay_mono                 = misalLay + monoCenDist;
	misalRef_mono                 = misalMod + monoCenDist;
	misalZpos                     = misalRef_mono.z();    
      }
      
      
      /********************************************************
      ** poca LayerCenter & wire                            **
      ** -> trajs get the geometry information              **
      **    from previous run, therefore we always make the **
      **    poca to a nominal center                        **
      ********************************************************/
      Gaudi::XYZVector Mtraje(-999999.9999,-999999.9999,-999999.99999);
      double onwire=0.;
      //use false = not restircted to range of traj
      //m_poca-> minimize( (*lhcbidTraj), onwire, false, nomMonoRef, Mtraje, m_tolerance);
      m_poca-> minimize( (*lhcbidTraj), onwire, false, misalRef_mono, Mtraje, m_tolerance);
      //    m_poca-> minimize( (*lhcbidTraj), onwire, monoMod, Mtraje, m_tolerance); //module wise
      if(Mtraje.x()>0.){
	meas.x = sqrt(Mtraje.x()*Mtraje.x()+Mtraje.y()*Mtraje.y());  //sqrt(Mtraje.Mag2()); 
      }
      else{
	meas.x =-sqrt(Mtraje.x()*Mtraje.x()+Mtraje.y()*Mtraje.y());//Mtraje.Mag2());
      }
      zpos = misalZpos;//-Mtraje.z();
      
      
      double start = (misalRef_mono.y()-idTrajPoint.y())/trajeD.y(); //way to y position
      if(  idTrajPoint.y() < 0. ) start=-1*start; //traj::direction d is d> 0 for p>0;d<0 for p<0;
      Gaudi::XYZPoint startZ = lhcbidTraj->position(start);
      Zref = startZ.z()-m_meanZ;
      
      
      
      //------------------------------------------------------
      //
      //  transform the measurement into global (LHCb) system
      //
      //------------------------------------------------------
      double doca = meas.x;
      meas.x = meas.x +  misalRef_mono.x()*cos(stereo_angle) +  misalRef_mono.y()*sin(stereo_angle);
      
      debug() <<"---------------------------------" << endreq
	      << " misalRef_mono " << misalRef_mono << endreq
	      << " Mtraje        " << Mtraje << endreq;
      debug() << " ---> CALCRES    "<<endreq
	      << "      lhcbIdTra         " << lhcbidTraj->position(0.) <<  endreq
	      << "      doca              " << doca << endreq
	      << "      meas.x            " << meas.x << " Mtraje.y()/Matreje.x() = "
	      << atan(Mtraje.y()/Mtraje.x()) << endreq
	      << "      zpos              " << zpos << endreq 
	      << "      Zref              " << Zref <<endreq
	      << "      m_meanZ           " << m_meanZ <<endreq 
	      <<"              -> z =     " << zpos-m_meanZ   << endreq;
      
      
      // be sure to set meas.y = 0 , since it is just used for OTTimes
      meas.y = Mtraje.y();
      if(fitok && m_trty[0] !=0.){
	//----------------------------------
	// get the y pos of hit at z
	//----------------------------------
	//       double hity = m_try0[0]+m_trty[0]*(misalRef_mono.z()-m_meanZ);
	//       double hitx = m_trx0[0]+m_trtx[0]*(misalRef_mono.z()-m_meanZ);
	double dRatio = -3.81831e-4;
	double hity = m_try0[0]+m_trty[0]*(Zref);
	double hitx = m_trx0[0]+m_trtx[0]*(Zref) + m_trQ[0]*( Zref*Zref + dRatio*Zref*Zref*Zref);
	//-------------------------------------------
	// get the way along the wire in y direction
	//------------------------------------------
	double way = (hity-idTrajPoint.y())/trajeD.y(); //way to y position
	if(  idTrajPoint.y() < 0. ) way=-1*way; //traj::direction d is d> 0 for p>0;d<0 for p<0;
	//----------------------------------
	// get the point at yhit on the wire
	//----------------------------------
	Gaudi::XYZPoint wayP = lhcbidTraj->position(way);
	
	//----------------------------------
	// get the relative position
	//----------------------------------
	double Y_m  = hity - misalRef_mono.y();// two sides from module center and module center at + or - y
	double X_m  = hitx - misalRef_mono.x();// two sides from module center and module center at + or - x
	double Z_m  = wayP.z() - misalRef_mono.z();// two sides from module center and module center at + or - z
	locPos.x    = X_m;//hitx;
	locPos.y    = Y_m;//hity;
	
	//---------------------
	// get new Zref
	//--------------------
	Zref = wayP.z()-m_meanZ;
	
	plot2D(X_m,Y_m,"X_m_VS_Y_m","X_m_VS_Y_m",-200.,200.,-3000.,3000,200,1000);
	
	debug() << "''''''''''''''''''''''''''''''''''''''''"<<endreq
		<< " wayP                     = "  << wayP << " (along wire : "<< way << " )"<<endreq
		<< " hit x / y / z            = " << hitx<<" " <<hity<< " "<< misalRef_mono.z()-m_meanZ<< endreq
		<< " X_m,Y_m,Z_m (module frame) " << X_m << " " << Y_m<< " " << Z_m << endreq
		<< " Stereo dx                  " << Y_m*tan(stereo_angle) << endreq
		<< " Tilt   dz                  " << hity*tan(0.003601)     << endreq
		<< " new Zref                   " << Zref <<  " (old with tilt " <<  zpos-m_meanZ+(hity*tan(0.003601))<<endreq
		<< endreq
		<< ",,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,,"<<endreq;
	plot( zpos-m_meanZ+(hity*tan(0.003601))-Zref,"diffZ","diffZ",-2,2,1000);
	
	//--------------------------------------
	//straw @ y of reference/rotation point 
	//-------------------------------------
	double dY_c  =  (misalRef_mono.y()-idTrajPoint.y())/trajeD.y(); 
	//      if( m_otModule && misalRef_mono.y() < 0.) dY_c = -1*dY_c;
	if(idTrajPoint.y() < 0.) dY_c = -1*dY_c;
	Gaudi::XYZPoint yStraw_c = lhcbidTraj->position(dY_c);
	
	//locPos.x    = misalRef_mono.x()-yStraw_c.x();         
	//------------------------------------
	//
	//    for dRotZ
	//
	//------------------------------------
	
	// alt
	//       double alongY_c  =  (0.-idTrajPoint.y())/trajeD.y(); 
	//       if(idTrajPoint.y() < 0.) alongY_c = -1*alongY_c;
	//       Gaudi::XYZPoint y0Straw_c = lhcbidTraj->position(alongY_c);
	//       double dmeas_c = y0Straw_c.x() - wayP.x() - wayP.y()*tan(stereo_angle);
	
	
	//------------------------------------
	//double dY_c    = Y_m;//Mtraje.y()-Y_m;
	//double meas_c  = X_m - dY_c*tan(stereo_angle);//Mtraje.x() + dY_c*tan(stereo_angle)-X_m;
	//double gamma   = dmeas_c/dY_c ;
	
	double dX_c     = yStraw_c.x() - wayP.x() - Y_m*tan(stereo_angle);
	double gamma = dX_c/Y_m;
	
	//-------------------------------------------------------
	// get new x measurement for x layer (stereo_angle == 0) 
	// get new measurement for stereo layer
	//------------------------------------------------------
	double wayAbs = wayP.x()*cos(stereo_angle)+ wayP.y()*sin(stereo_angle);
	
	debug() << "--> GAMMA          input " << m_misalInput_C.at(rank) << endreq
		<< " gamma here  =  " << (wayAbs - meas.x)/wayP.y() <<  endreq
		<< " yStraw_c       " << yStraw_c << endreq
		<< " dY*tan(stereo_angle) " << Y_m*tan(stereo_angle) << endreq
		<< " dX_c           " << dX_c << endreq
		<< " gamma  local=  " << gamma << endreq
		<< "------------------------------------"<<endreq;
	
	//------------------------------
	// pass the new measurement
	//------------------------------
	//      if(m_otLayer) meas.x += dmeas_c;// += (meas.x-wayAbs);
	//       if(m_otLayer) meas.x += dX_c;// += (meas.x-wayAbs);
	//       if(m_otModule || m_otHalfLayer) meas.x += dX_c;//dmeas_c;
	meas.x += dX_c;//dmeas_c;
	plot(dX_c,"dmeas_c","dmeas_c",-.2,.2,100);            
	
	//------------------------------------
	//
	//    for dRotX
	//
	//------------------------------------
	//----------------------------------
	// get the z pos of hit at y, take y point 
	// from dRotZ = wayP
	//----------------------------------
	
	/* the follwing must be used if the track fit is done without using the 
	   z position of the straws, but the z of the module center
	   
	   double dY_a    = Y_m;
	   double dZ_a    = wayP.z() - yStraw_c.z() - dY_a*tan(0.0036);
	   double dmeas_a = dZ_a*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0]);
	   double alpha   = dmeas_a/((cos(stereo_angle)*dY_a*m_trtx[0]+
	   sin(stereo_angle)*dY_a*m_trty[0]));
	   
	   Gaudi::XYZPoint y0Straw_a;
	   //      if(m_otLayer){
	   double alongY_a  =  (0-idTrajPoint.y())/trajeD.y(); 
	   if(idTrajPoint.y() < 0.) alongY_a = -1*alongY_a;
	   // the straw position at y=0
	   y0Straw_a = lhcbidTraj->position(alongY_a);
	   // relative z shift
	   double A_relaZ = wayP.z()-y0Straw_a.z()-wayP.y()*tan(0.0036);
	   debug()<<"A_relaz="<<A_relaZ<<endreq
	   <<"hity   =" <<hity<<endreq;
	   
	   //calculate x misalignment due to rotation around X axis      
	   double Amisal  = A_relaZ*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0]);
	   double dalpha  =  Amisal/((cos(stereo_angle)*hity*m_trtx[0]+sin(stereo_angle)*hity*m_trty[0]));
	   
	   debug() << " ALPHA          input  "<< m_misalInput_A.at(rank)  <<"  stereo:"<< stereo_angle<<endreq
	   << " y0Straw:"<<y0Straw_a<<endreq
	   << " Amisal = " << Amisal << endreq
	   << " ===> alpha = "<< dalpha <<endreq
	   // <<"  derGB = " << (cos(stereo_angle)*dY_a*m_trtx[0]+sin(stereo_angle)*dY_a*m_trty[0])<<endreq
	   << " dY_a            " << dY_a << endreq
	   << " dZ_a            " << dZ_a << endreq
	   << " dmeas_a       = " << dmeas_a << endreq
	   << " alpha (geom)  = " << dZ_a/dY_a << endreq
	   << " alpha new     = " << alpha <<endreq
	   << "---------------------------------------------------"<< endreq;
	   
	   //      meas.x += Amisal;
	   // meas.x += dmeas_a;
	   plot(dmeas_a,"dmeas_a","dmeas_a",-.2,.2,100);      
	   
	   if(stereo_angle == 0 )
	   plot(alpha-m_misalInput_A.at(rank),"dalphanewX","dalphanewX",-0.01,0.01,1000);
	   if(stereo_angle != 0 )
	   plot(alpha-m_misalInput_A.at(rank),"dalphanewUV","dalphanewUV",-0.01,0.01,1000);
	   
	   
	   plot(dalpha-m_misalInput_A.at(rank),"dalphaold","dalphaold",-0.01,0.01,1000);
	   
	   
	   
	   
	   
	   //------------------------------------
	   //
	   //    for dRotY
	   //
	   //------------------------------------
	   //------------------------------------
	   // get z pos of wire at y=0
	   //------------------------------------
	   double alongY  =  (0-idTrajPoint.y())/trajeD.y(); 
	   if(idTrajPoint.y() < 0.) alongY = -1*alongY;
	   // the straw position at y=0
	   Gaudi::XYZPoint y0Straw = lhcbidTraj->position(alongY);
	   Gaudi::XYZVector  mLayDu,bmLayDu;
	   Gaudi::XYZVector  mLayD  = mLayTraj->direction(0.);
	   Gaudi::XYZVector  bmLayD = bmLayTraj->direction(0.);
	   if(mLayD.y() < 0.) 
	   mLayDu.SetXYZ(-mLayD.x()/sqrt(mLayD.Mag2()),-mLayD.y()/sqrt(mLayD.Mag2()),-mLayD.z()/sqrt(mLayD.Mag2()));
	   else
	   mLayDu.SetXYZ(mLayD.x()/sqrt(mLayD.Mag2()),mLayD.y()/sqrt(mLayD.Mag2()),mLayD.z()/sqrt(mLayD.Mag2()));
	   if(bmLayD.y() < 0.) 
	   bmLayDu.SetXYZ(-bmLayD.x()/sqrt(bmLayD.Mag2()),-bmLayD.y()/sqrt(bmLayD.Mag2()),-bmLayD.z()/sqrt(bmLayD.Mag2()));
	   else
	   bmLayDu.SetXYZ(bmLayD.x()/sqrt(bmLayD.Mag2()),bmLayD.y()/sqrt(bmLayD.Mag2()),bmLayD.z()/sqrt(bmLayD.Mag2()));
	   
	   double toY0  =  (0.-mLayP.y())/mLayDu.y(); 
	   double btoY0 =  (0.-bmLayP.y())/bmLayDu.y(); 
	   if(mLayP.y()  < 0.) toY0  = -1.*toY0;
	   if(bmLayP.y() < 0.) btoY0 = -1.*btoY0;
	   // the straw position at y=0
	   Gaudi::XYZPoint atY0  = mLayTraj->position(toY0);
	   Gaudi::XYZPoint batY0 = bmLayTraj->position(btoY0);
	   Gaudi::XYZVector datY0(atY0.x()-batY0.x(),atY0.y()-batY0.y(),atY0.z()-batY0.z());  //dist between two trajs
	   
	   
	   
	   //      double perpToStraw     = (0.-y0Straw.x())/datY0.x();
	   double perpToStraw     = (misalRef_mono.x()-y0Straw.x())/datY0.x();
	   Gaudi::XYZPoint ZatX0  = y0Straw+datY0*perpToStraw;
	   double dZ              = y0Straw.z()-ZatX0.z();
	   double Bmisalnew = dZ*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0]);
	   //double    beta   = Bmisalnew/ (  -sin(stereo_angle)*m_trty[0]*hitx
	   //                               -cos(stereo_angle)*m_trtx[0]*hitx);
	   double    beta   = Bmisalnew/ (  -sin(stereo_angle)*m_trty[0]*X_m
	   -cos(stereo_angle)*m_trtx[0]*X_m);
	   
	   
	   //-------------------------------
	   double diffx = misalRef_mono.x()-yStraw_c.x();//batY0.x();
	   double diffz = misalRef_mono.z()-yStraw_c.z();//-batY0.z();
	   double angle1 = fabs(diffz/diffx);
	   
	   debug() << " angle straws " << angle1 <<endreq;
	   
	   
	   //        double ndZ     = yStraw_c.z() - misalRef_mono.z();
	   //         double dmeas_b = ndZ*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0]);      
	   //         double nbeta   = dmeas_b/ (  -sin(stereo_angle)*m_trty[0]*X_m
	   //         -cos(stereo_angle)*m_trtx[0]*X_m);
	   //         double nbetageom = ndZ/X_m;
	   
	   double ndZ     = diffz;//Z_m-Y_m*tan(0.003601);//yStraw_c.z() - misalRef_mono.z();
	   double dmeas_b = ndZ*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0]);      
	   double nbeta   = dmeas_b/ (  -sin(stereo_angle)*m_trty[0]*diffx
	   -cos(stereo_angle)*m_trtx[0]*diffx);
	   double nbetageom = ndZ/X_m;
	   double percent   = ( (fabs(beta) - fabs(m_misalInput_B.at(rank)))/ fabs(m_misalInput_B.at(rank)) );
	   
	   if( percent >  0.02 
	   && fabs(m_misalInput_B.at(rank))  > 1e-4 ){
	   
	   debug() << " beta DIFF " << percent
	   << " X_M = " << X_m <<endreq;
	   if( stereo_angle == 0) plot2D(X_m,percent,"beta_X_m_VS_diffInput(percent)_X","beta_X_m_VS_diffInput(percent)_X"
	   ,-200.,200.,0.,1.3,200,100);
	   if( stereo_angle != 0) plot2D(X_m,percent,"beta_X_m_VS_diffInput(percent)_UV","beta_X_m_VS_diffInput(percent)_UV"
	   ,-200.,200.,0.,1.3,200,100);
	   
	   }
	   
	   //      meas.x += Bmisalnew;
	   plot(Bmisalnew,"dmeas_b","dmeas_b",-.2,.2,100);
	   //       meas.x += dmeas_b;
	   //       plot(dmeas_b,"dmeas_b","dmeas_b",-.2,.2,100);
	   
	   debug() << " BETA      input "<< m_misalInput_B.at(rank)  <<"  stereo:"<< stereo_angle<<endreq
	   << " diffInput(perc)" << percent << endreq
	   << " straw @ y=Ref  " << yStraw_c << endreq
	   << " ndZ            " << ndZ   << endreq
	   << " dmeas_b        " << dmeas_b << endreq
	   << " nbeta          " << nbeta << endreq
	   << " nbetageom      " << nbetageom << endreq
	   << " ZatX0          " << ZatX0 <<endreq
	   << " BmisalNew =    " << Bmisalnew << endreq
	   << " beta      =    " << beta << endreq
	   <<"---------------------------------------"<<endreq;
	   
	*/
	
	
      }
      
      
      //******** This stuff is for OTTimes **********
	  //     Gaudi::XYZPoint posRec;
	  //     Gaudi::XYZVector slRec;
	  //     m_extrapolator->position((*track),zpos,posRec);
	  //     m_extrapolator->slopes((*track),zpos,slRec);
	  //     double calTime = 0.;
	  //     double drDist  = 0.;
	  //     //get OTTimes
	  //     LHCb::OTTimes* m_ottimes = get<LHCb::OTTimes>(LHCb::OTTimeLocation::Default);
	  //     LHCb::OTTimes::const_iterator iTimes = m_ottimes->begin();
	  //     for ( ; iTimes != m_ottimes->end(); ++iTimes ) {
	  //       if (  track->isOnTrack( (*iTimes)->channel() ) &&  (*iTimes)->channel() == id.otID() ) {
	  // 	calTime =  (*iTimes)->calibratedTime();
	  // 	plot((*iTimes)->calibratedTime(),"caltime","caltime",-100,100,100);
	  // // 	DeOTDetector* m_ot1 = getDet<DeOTDetector>(DeOTDetectorLocation::Default);
	  // // 	DeOTModule* module1 = m_ot1->findModule( id.otID() );
	  // 	drDist = module->driftRadius(calTime-m_ot->propagationTime(id.otID(), posRec.x(), posRec.y()));
	  // 	plot(drDist,"drdist","drdist",-5,5,50);
	  //       }	      
	  //     }
	  //     double slangle = atan(slRec.x());
	  //     double uDist   = drDist/cos(slangle);
	  //     double help = 0.;
	  //     Gaudi::XYZVector helpV;
	  //     m_poca-> minimize( (*lhcbidTraj), help, false, posRec, helpV, m_tolerance);
	  //     if(helpV.x() < 0. ) meas.y = uDist; 
	  //     if(helpV.x() > 0. ) meas.y = -uDist; 
	  }
  else if( !id.isOT()){
    info() << " no OT id given !!! " << endmsg;
  }
  return StatusCode::SUCCESS;
  
}

// StatusCode TAConfig::CalcResidual( const LHCb::Track* track, 
//                                    const LHCb::LHCbID &id, 
//                                    int rank, // do we realy need the rank ?
//                                    struct Point &meas,
//                                    double &weight,
//                                    bool fitok,  // need to see if there is a local fit in progress.
//                                    double & zpos , //MD
//                                    double & stereo  //MD
//                                    ) {
//   //  const Measurement& trMeas = track->measurement(id); // Get measurement

//   if(id.isOT()){
//     debug() << "------------------------------" <<endreq;
//     debug() << "----> CALCRES RANK =  " << rank << endreq;
//     debug() << "------------------------------" <<endreq;

//     //    weight = 1./pow(trMeas.errMeasure(),2.);
//     //weight = (1./.2)*(1./.2);
//     weight = (1./1.44)*(1./1.44);
//     double stereo_angle  = 0.;
//     stereo_angle         = m_ot->findLayer( id.otID() )->angle();
//     stereo               = stereo_angle;

//     //build previous traj
//     //    LineTraj prevtraj = LineTraj(m_pointa,m_pointb);
//     // info()<<" 1.   prevtraj = " << prevtraj.direction(0.) << " " << prevtraj.position(0.)<<endreq;
//     //MD get another reference without measurement info
//     std::auto_ptr<LHCb::Trajectory> lhcbidTraj = m_ot->findModule( id.otID() )->trajectory(id.otID());   
//     Gaudi::XYZPoint idTrajPoint                = lhcbidTraj->position(0.);
//     Gaudi::XYZVector idTrajDir                 = lhcbidTraj->direction(0.);    
//     //    debug() << " --> trajDir = " << idTrajDir << " trajPoint = " << idTrajPoint << endreq;
//     //       Gaudi::XYZVector mvec;
//     //     double mwire1 = 0.;
//     //     double mwire2 = 0.;
//     //     m_poca->minimize((*lhcbidTraj),mwire1,prevtraj,mwire2,mvec,m_tolerance);
//     //     Gaudi::XYZPoint mpointy1 = lhcbidTraj->position(mwire1);
//     //     Gaudi::XYZPoint mpointy2 = prevtraj.position(mwire2);

//     //    info() << " my new y1 = " << mpointy1 << "  y2 = " << mpointy2<< endreq;
//     //meas.y = mpointy1.y();
//     //give infos for next round
// //     m_pointa = lhcbidTraj->beginPoint();
// //     m_pointb = lhcbidTraj->endPoint();
// //     info() << " my starting point = " << m_pointa<< " endpoint "<< m_pointb <<endreq
// //            << " middle point " << idTrajPoint << " and dir " << idTrajDir << endreq;

//     // get sensor where hit appeared
//     DeOTLayer*  layer  =  m_ot->findLayer( id.otID() );
//     DeOTModule* module = m_ot->findModule( id.otID() );

//     //get otchannelid and straw and monolayer
//     const OTChannelID  chID   = id.otID();
//     const unsigned int straw  = chID.straw();
//     bool mLayA                = module->monoLayerA(straw);

//     //   get measurement trajectory and its direction
//     //MD    const Trajectory& measTraj = trMeas.trajectory();
//     Gaudi::XYZVector trajeDa =  idTrajDir;//MD measTraj.direction(0.);
//     Gaudi::XYZVector trajeD;
//     //    debug() << " idtrajDira = " << trajeDa << endmsg;    
//     if(trajeDa.y() < 0.) //make sure all trajsdirection is upwards    //MD 15-04
//       trajeD.SetXYZ(-trajeDa.x()/sqrt(trajeDa.Mag2()),-trajeDa.y()/sqrt(trajeDa.Mag2()),-trajeDa.z()/sqrt(trajeDa.Mag2()));
//     else
//       trajeD.SetXYZ(trajeDa.x()/sqrt(trajeDa.Mag2()),trajeDa.y()/sqrt(trajeDa.Mag2()),trajeDa.z()/sqrt(trajeDa.Mag2()));

//     Gaudi::Rotation3D R, Rnom,rModNom,ownR;
//     Gaudi::XYZVector Lay,Mod,Pnom,sModNom,ownMod;
//     //MD
//     //     info() << "--------------------------------------------------------------" << endreq;
//     //     debug() << " idtrajDir = " << trajeD << endmsg;
//     //     info() << " idtraj   -->  trajPos = " << idTrajPoint << " idtrajDir = " << trajeD << endmsg;
//     //     info() << " begin point of idtraj   = " << lhcbidTraj->beginPoint()    << endmsg;
//     //     info() << " begin point of meastraj = " << measTraj.beginPoint()    << endmsg;
//     //     info() << " end   point of idtraj   = " << lhcbidTraj->endPoint()    << endmsg;
//     //     info() << " end   point of meastraj = " << measTraj.endPoint()    << endmsg;
//     //     info() << " meastraj.dir = "<< measTraj.direction(0.) << " idtraj = " << lhcbidTraj->direction(0.) << endmsg;
//     // Get center of Layer and Rotation matrix in LHCb coordinate frame
//     layer->geometry()->toGlobalMatrix().GetDecomposition( R, Lay);
//     layer->geometry()->toGlobalMatrixNominal().GetDecomposition( Rnom, Pnom);    
//     module->geometry()->toGlobalMatrix().GetDecomposition( R, Mod);    // get center of Module including misalignments
//     module->geometry()->ownToOffNominalMatrix().GetDecomposition( ownR, ownMod);    // get center
//     module->geometry()->toGlobalMatrixNominal().GetDecomposition( rModNom, sModNom);    

//     const Gaudi::XYZPoint misalLay(Lay.x(),Lay.y(),Lay.z());
//     const Gaudi::XYZPoint nomLay(Pnom.x(),Pnom.y(),Pnom.z());

//     const Gaudi::XYZPoint misalMod(Mod.x(),Mod.y(),Mod.z());
//     const Gaudi::XYZPoint nomMod(sModNom.x(),sModNom.y(),sModNom.z());
//     const Gaudi::XYZPoint own(ownMod.x(),ownMod.y(),ownMod.z());

//     if(own.x() !=0 || own.y()!=0 ||own.z()!=0){
//       info() << "--> nomLay="<<nomLay << " misalLay="<<misalLay << endreq
//              << "--> nomMod="<<nomMod << " misalMod="<<misalMod << endreq
//              << "--> nomMod="<<nomMod << " own     ="<<own << endreq;
//     }


//     /*********************
//      ** new Mod Center ***
//      ********************/

//     std::auto_ptr< LHCb::Trajectory > mLayTraj;
//     std::auto_ptr< LHCb::Trajectory > bmLayTraj;
//     //get the plane of first monolayer
//     if(mLayA){
//       //      info() << " use module : " << module->name() << " monoB? " << !mLayA << endreq;
//       mLayTraj = module->trajectoryFirstWire(0);//first monolayerRight
//       bmLayTraj = module->trajectoryLastWire(0);//first monolayerLeft
//       //MD 14-04    
//     }
//     //get the plane of the second monolayer
//     //MD14-04 
//     else if(!mLayA){
//       mLayTraj = module->trajectoryFirstWire(1);//second monolayerRight
//       bmLayTraj = module->trajectoryLastWire(1);//second monolayerLeft
//     }

//     Gaudi::XYZPoint  mLayP  = mLayTraj->position(0.);
//     Gaudi::XYZPoint  bmLayP = bmLayTraj->position(0.);
//     Gaudi::XYZVector dLay(mLayP.x()-bmLayP.x(),mLayP.y()-bmLayP.y(),mLayP.z()-bmLayP.z());  //dist between two trajs
//     Gaudi::XYZPoint  monoMod =  mLayP-0.5*dLay;     //new center of  Monolayer
//     //*******************************
//     // distance between orig. module center and new monoMod is the same as for layer center and new layer center!!
//     //*******************************
//     Gaudi::XYZVector monoCenDist;
//     monoCenDist.SetXYZ(0.,monoMod.y()-misalMod.y(),monoMod.z()-misalMod.z());

//     /*********************
//      ** new Lay Center***
//      ********************/
//     Gaudi::XYZPoint nomMonolayer(0.,0.,0.);
//     Gaudi::XYZPoint misalLay_mono(0.,0.,0.);

//     double misalZpos = 0.;


//     if(m_otLayer){
//       nomMonolayer                  = nomLay + monoCenDist;  
//       misalLay_mono                 = misalLay + monoCenDist;
//       misalZpos = misalLay_mono.z();    //get misaligned z position of layer
//     }

//     if(m_otModule || m_otHalfLayer){
//       nomMonolayer                  = nomLay + monoCenDist;//trackfit in global coor.
//       misalLay_mono                 = misalLay + monoCenDist;
//       Gaudi::XYZPoint misalMod_mono = misalMod + monoCenDist;
//       misalZpos = misalMod_mono.z();    //get misaligned z position of layer
//     }
//     zpos = misalZpos;
//     //debug() << " --->  ref point for poca between traj and ref point : " << nomMonolayer << endreq;
//     //debug() << " --->  traj      for poca between traj and ref point : " << lhcbidTraj->beginPoint() 
//     //            << " " <<  lhcbidTraj->endPoint() << endreq;

//     /********************************************************
//      ** poca LayerCenter & wire                            **
//      ** -> trajs get the geometry information              **
//      **    from previous run, therefore we always make the **
//      **    poca to a nominal center                        **
//      ********************************************************/
//     Gaudi::XYZVector Mtraje(-999999.9999,-999999.9999,-999999.99999);
//     double onwire=0.;
//     //use false = not restircted to range of traj
//     m_poca-> minimize( (*lhcbidTraj), onwire, false, nomMonolayer, Mtraje, m_tolerance);
//     //    m_poca-> minimize( (*lhcbidTraj), onwire, monoMod, Mtraje, m_tolerance); //module wise

//     if(Mtraje.x()>0.){
//       meas.x = sqrt(Mtraje.x()*Mtraje.x()+Mtraje.y()*Mtraje.y());  //sqrt(Mtraje.Mag2()); 
//     }
//     else{
//       meas.x =-sqrt(Mtraje.x()*Mtraje.x()+Mtraje.y()*Mtraje.y());//Mtraje.Mag2());
//     }
//     // debug() << " ---> CALCRES meas.x = " << meas.x << endreq;
//     // be sure to set meas.y = 0 , since it is just used for OTTimes
//     meas.y = 0.;
//     if(fitok){
//       //------------------------------------
//       //
//       //    for dRotZ
//       //
//       //------------------------------------
//       //----------------------------------
//       // get the y pos of hit at z
//       //----------------------------------
//       double hity = m_try0[0]+m_trty[0]*(nomMonolayer.z()-m_meanZ);
//       //      double hity = m_try0[0]+m_trty[0]*(zpos-m_meanZ); //misaligned z pos
//       //debug() << "--> GAMMA hity="<<hity<<" slope="<< m_trty[0]<<" off="<<m_try0[0] << endreq;    
//       //-------------------------------------------
//       // get the way along the wire in y direction
//       //------------------------------------------
//       double way = (hity-idTrajPoint.y())/trajeD.y(); //way to y position
//       if(  idTrajPoint.y() < 0. ) way=-1*way; //traj::direction d is d> 0 for p>0;d<0 for p<0;
//       Gaudi::XYZPoint begTraj =  lhcbidTraj->beginPoint();
//       //----------------------------------
//       // get the point at yhit on the wire
//       //----------------------------------
//       Gaudi::XYZPoint wayP = lhcbidTraj->position(way);
// //       //plot(wayP.y()-hity,"wayPY-hitY",-2.,2.,400);
// //       debug() <<std::setw(5)<< "--> GAMMA way(onwire)="<<way<< " wayP=" << wayP <<endreq;
// //       //-------------------------------------------------------
// //       // get new x measurement for x layer (stereo_angle == 0) 
// //       // get new measurement for stereo layer
// //       //------------------------------------------------------
// //       double wayAbs = wayP.x()*cos(stereo_angle)+ wayP.y()*sin(stereo_angle);
// //       //------------------------------
// //       // pass the new measurement
// //       //------------------------------
// //      meas.x = wayAbs;
// //      debug()<<std::setw(5) << "--> GAMMA meas.x modified = " << meas.x << " stereo = " << stereo_angle << endreq;




//       double alongY_c  =  (0-idTrajPoint.y())/trajeD.y(); 
//       if(idTrajPoint.y() < 0.) alongY_c = -1*alongY_c;
//       // the straw position at y=0
//       Gaudi::XYZPoint y0Straw_c = lhcbidTraj->position(alongY_c);
//       double Cmisal = wayP.x()-y0Straw_c.x()+hity*tan(stereo_angle);
//       debug() << "---> y0Straw"<< endreq
//               <<y0Straw_c <<endreq
//               <<" hitY"<<endreq
//               <<hity<<endreq
//               <<" wayP.x()-y0Straw_c.x()"<<endreq
//               <<wayP.x()-y0Straw_c.x() <<endreq
//               <<"hity*tan(stereo_angle) & stereo"<<endreq
//               << hity*tan(stereo_angle)<< "  &  " << stereo_angle<<endreq
//               <<" CMISAL"<<endreq;
//       debug() <<"Cmisal " << endreq 
//               <<Cmisal<<" gamma="<<Cmisal/hity <<endreq;

//       //      meas.x -= Cmisal;



//       //------------------------------------
//       //
//       //    for dRotX
//       //
//       //------------------------------------
//       //----------------------------------
//       // get the z pos of hit at y, take y point 
//       // from dRotZ = wayP
//       //----------------------------------
//       // for layer and long module alignment, the axis around which is rotated
//       // is the x-axis at y==0;
//       // for  short module alignment the rotation axis is at the middle of the 
//       // wire, i.e. at  lhcbidTraj->position(0.)=idTrajPoint;
//       Gaudi::XYZPoint y0Straw_a;
//       // for module alignment y0Straw_a.y() != 0 therefore the sign of relaZ changes
//       // for hits at same y half
//       //      if(m_otLayer){
//       double alongY_a  =  (0-idTrajPoint.y())/trajeD.y(); 
//       if(idTrajPoint.y() < 0.) alongY_a = -1*alongY_a;
//       // the straw position at y=0
//       y0Straw_a = lhcbidTraj->position(alongY_a);
//       //}
//       //       if(m_otModule){
//       //         y0Straw_a = idTrajPoint;
//       //       }
// //       info() <<"stereo:"<<endreq
// // 	     <<stereo_angle<<endreq
// // 	     << " y0Straw:"<<endreq
// // 	     <<y0Straw_a<<endreq
// // 	     <<"wayP"<<endreq      
// // 	     <<wayP<<endreq;      
//       // relative z shift
//       double A_relaZ = wayP.z()-y0Straw_a.z()-wayP.y()*tan(0.0036);
// //       info()<<"A_relaz="<<A_relaZ<<endreq
// // 	    <<" wayP.z()-y0Straw_a.z() "<< wayP.z()-y0Straw_a.z()<<endreq
// // 	    <<"-wayP.y()*tan(0.0036) "<<-wayP.y()*tan(0.0036)<<endreq;
//       //calculate x misalignment due to rotation around X axis      
//       double Amisal  = A_relaZ*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0]);
//       debug() << " Amisal = " << endreq
//               <<Amisal << " alpha = "
//<< Amisal/((cos(stereo_angle)*hity*m_trtx[0]+sin(stereo_angle)*wayP.x()*m_trty[0]))<<endreq;
//       meas.x += Amisal;

//       //ALTERNATIVE
// //       double al_a  =  (hity-idTrajPoint.y())/trajeD.y(); 
// //       if(idTrajPoint.y() < 0.) al_a = -1*al_a;
// //       //if(idTrajPoint.y() > 0  && wayP.y() < idTrajPoint.y()) relaZsign = -1.;
// //       //if(idTrajPoint.y() < 0  && wayP.y() > idTrajPoint.y()) relaZsign = -1.;
// //       Gaudi::XYZPoint strawhit = lhcbidTraj->position(al_a);

// //       Gaudi::XYZPoint locstr = module->geometry()->toLocal(strawhit);
// //       Gaudi::XYZPoint loccent = module->geometry()->toLocal(idTrajPoint);

// // //       double dYa = strawhit.y()-idTrajPoint.y();
// // //       double dZa = strawhit.z()-idTrajPoint.z();
// //       double dYa = locstr.y()-loccent.y();
// //       double dZa = locstr.z()-loccent.z();
// //       double Amisal2 = dZa*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0]);
// //       double alpha2  = dZa/dYa;

// //       debug() << "---> trajcenter " << idTrajPoint << " hity " << hity <<endreq
// //               << "direction "<<endreq
// //               << al_a <<endreq
// //               <<"strawhit "<<strawhit<< " ->loc " << locstr << endreq
// //               <<"idTrajPoint "<<idTrajPoint<< " ->loc " << loccent << endreq
// //               <<"dYa " << dYa << "  dZa "<< dZa <<endreq
// //               <<"Amisal2 "<< endreq
// //               << Amisal2 << " alpha2 = "<< alpha2 << endreq;



//       //------------------------------------
//       //
//       //    for dRotY
//       //
//       //------------------------------------
//       //------------------------------------
//       // get z pos of wire at y=0
//       //------------------------------------


//       double alongY  =  (0-idTrajPoint.y())/trajeD.y(); 
//       if(idTrajPoint.y() < 0.) alongY = -1*alongY;
//       // the straw position at y=0
//       Gaudi::XYZPoint y0Straw = lhcbidTraj->position(alongY);
//       // relative z shift
//       //       double relZ = y0Straw.z()-misalLay_mono.z();
//       //       //calculate x misalignment due to rotation around Y axis
//       //       double Bmisal = relZ*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0]);

//       Gaudi::XYZVector  mLayDu,bmLayDu;
//       Gaudi::XYZVector  mLayD  = mLayTraj->direction(0.);
//       Gaudi::XYZVector  bmLayD = bmLayTraj->direction(0.);
//       if(mLayD.y() < 0.) 
//         mLayDu.SetXYZ(-mLayD.x()/sqrt(mLayD.Mag2()),-mLayD.y()/sqrt(mLayD.Mag2()),-mLayD.z()/sqrt(mLayD.Mag2()));
//       else
//         mLayDu.SetXYZ(mLayD.x()/sqrt(mLayD.Mag2()),mLayD.y()/sqrt(mLayD.Mag2()),mLayD.z()/sqrt(mLayD.Mag2()));
//       if(bmLayD.y() < 0.) 
//         bmLayDu.SetXYZ(-bmLayD.x()/sqrt(bmLayD.Mag2()),-bmLayD.y()/sqrt(bmLayD.Mag2()),-bmLayD.z()/sqrt(bmLayD.Mag2()));
//       else
//         bmLayDu.SetXYZ(bmLayD.x()/sqrt(bmLayD.Mag2()),bmLayD.y()/sqrt(bmLayD.Mag2()),bmLayD.z()/sqrt(bmLayD.Mag2()));

//       double toY0  =  (0.-mLayP.y())/mLayDu.y(); 
//       double btoY0 =  (0.-bmLayP.y())/bmLayDu.y(); 
//       if(mLayP.y()  < 0.) toY0  = -1.*toY0;
//       if(bmLayP.y() < 0.) btoY0 = -1.*btoY0;
//       // the straw position at y=0
//       Gaudi::XYZPoint atY0  = mLayTraj->position(toY0);
//       Gaudi::XYZPoint batY0 = bmLayTraj->position(btoY0);
//       Gaudi::XYZVector datY0(atY0.x()-batY0.x(),atY0.y()-batY0.y(),atY0.z()-batY0.z());  //dist between two trajs


//       //       info() <<"---------- rnk "<<rank<<" -------------------"<<endreq;
//       //       info() <<"relZ  = "<< relZ << " hitpos = " << wayP << endreq;
//       //       info() <<"--> BETA deltameas.x = " << Bmisal  << endreq
//       //              <<"beta="
//       //              << Bmisal/div << endreq <<endreq;
//       double perpToStraw     = (0.-y0Straw.x())/datY0.x();
//       Gaudi::XYZPoint ZatX0  = y0Straw+datY0*perpToStraw;
//       double dZ              = y0Straw.z()-ZatX0.z();
//       //if(dZ >0.001) info() <<" dZ=" << dZ << " dLAY="<<datY0<<endreq;

//       //       info() << "y0Straw = " << y0Straw << " dLay*perpToStraw= "<<dLay*perpToStraw <<"-->  ZatX0 "<< ZatX0 << endreq
//       //              << "dZ= "<< dZ << " Bmisal'= " << dZ*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0])
//       //              << " --> beta'= "<< (dZ*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0]))/div 
//       //              << endreq <<"---------------------" <<endreq;
//       double Bmisalnew = dZ*(cos(stereo_angle)*m_trtx[0]+sin(stereo_angle)*m_trty[0]);
//       meas.x += Bmisalnew;


// //   if(datY0.y() > 0.001 || datY0.z() >0.001){
// // 	info() <<" datY0 = " << datY0 << " (stereo="<<stereo_angle<< endreq
// // 	       <<"from point:"<<endreq
// // 	       << "1:"<< mLayP<< "  2:"<<bmLayP<<endreq;
// // 	info() << " directioN"<<endreq
// // 	       << " 1:"<< mLayDu<<"  2:"<<bmLayDu<<endreq
// // 	       <<" wat to y0"<<endreq
// // 	       <<" 1:"<<toY0 <<"  2:"<<btoY0<<endreq;
// // 	info() <<" atY0 =" << atY0 << " batY0="<<batY0 << endreq
// // 	       <<" perpToStraw (length)"<<endreq
// // 	       << perpToStraw<<endreq
// // 	       << " datY0*perp   and y0Straw"<<endreq
// // 	       <<datY0*perpToStraw << "   " << y0Straw<<endreq
// // 	       <<" ZatX0"<< endreq
// // 	       <<ZatX0<<endreq;
//       debug() << " BmisalNew =" << Bmisalnew << endreq;

// // 	       <<"---------------------------------------"<<endreq;}
//       // info()<<std::setw(5) << "--> BETA deltameas.x = " << Bmisal  << endreq;



//     }

//     //******** This stuff is for OTTimes **********
//     //     Gaudi::XYZPoint posRec;
//     //     Gaudi::XYZVector slRec;
//     //     m_extrapolator->position((*track),zpos,posRec);
//     //     m_extrapolator->slopes((*track),zpos,slRec);
//     //     double calTime = 0.;
//     //     double drDist  = 0.;
//     //     //get OTTimes
//     //     LHCb::OTTimes* m_ottimes = get<LHCb::OTTimes>(LHCb::OTTimeLocation::Default);
//     //     LHCb::OTTimes::const_iterator iTimes = m_ottimes->begin();
//     //     for ( ; iTimes != m_ottimes->end(); ++iTimes ) {
//     //       if (  track->isOnTrack( (*iTimes)->channel() ) &&  (*iTimes)->channel() == id.otID() ) {
//     // 	calTime =  (*iTimes)->calibratedTime();
//     // 	plot((*iTimes)->calibratedTime(),"caltime","caltime",-100,100,100);
//     // // 	DeOTDetector* m_ot1 = getDet<DeOTDetector>(DeOTDetectorLocation::Default);
//     // // 	DeOTModule* module1 = m_ot1->findModule( id.otID() );
//     // 	drDist = module->driftRadius(calTime-m_ot->propagationTime(id.otID(), posRec.x(), posRec.y()));
//     // 	plot(drDist,"drdist","drdist",-5,5,50);
//     //       }	      
//     //     }
//     //     double slangle = atan(slRec.x());
//     //     double uDist   = drDist/cos(slangle);
//     //     double help = 0.;
//     //     Gaudi::XYZVector helpV;
//     //     m_poca-> minimize( (*lhcbidTraj), help, false, posRec, helpV, m_tolerance);
//     //     if(helpV.x() < 0. ) meas.y = uDist; 
//     //     if(helpV.x() > 0. ) meas.y = -uDist; 
//   }
//   else if( !id.isOT()){
//     info() << " no OT id given !!! " << endmsg;
//   }
//   return StatusCode::SUCCESS;

// }



//MD
/**************************************************
 ** Method to constrain elements using a string 
 ** Note: If you give one element the value '0', 
 ** all other values following this element must
 ** also be '0' ! 
 ** example:
 ** forbidden : S1_L1_Q0_M3
 ** allowed :   S1_L1_Q0_M0
 **************************************************/
StatusCode TAConfig::ConstrainPositionsOT() {
  
  unsigned int alipar = m_nAlignObj; //nof params to align
  const unsigned int size = m_OTConstrain.size();
  std::vector<int> cons(size,0);
  int elements[4];//[station,layer,quarter,module]
  m_cntfix = 0; //count # of fixed objects
  
  for (unsigned int i=0; i < size; i++){
    info()<< "string = " << m_OTConstrain[i] << " has " << m_OTConstrain[i].size() << " entries"<< endreq;
    
    //interpret every string
    std::string str_fix = m_OTConstrain[i];
    int dcnt =0; //count no dof
    
    for(unsigned int c=0; c < str_fix.size(); c++){
      
      if(str_fix[c] == 'X' ){
        if(m_dof[0] != 1 ) {
          error() << "--> Constrain does not match number of degree of freedom" << endreq;
          return StatusCode::FAILURE;
        }
        else cons[i] = 0;
      }
      
      if(str_fix[c] == 'Z') {
        if(m_dof[2] != 1 ) {
          error() << "--> Constrain does not match number of degree of freedom" << endreq;
          return StatusCode::FAILURE;
        }
        for(int n=0; n < 2; n++) if(m_dof[n] == 1) dcnt++;
        dcnt = alipar*dcnt;
      }
      
      if(str_fix[c] == 'A' ){
        if(m_dof[3] != 1 ) {
          error() << "--> Constrain does not match number of degree of freedom" << endreq;
          return StatusCode::FAILURE;
        }
        for(int n=0; n < 3; n++) if(m_dof[n] == 1) dcnt++;
        dcnt = alipar*dcnt;
      }
      
      if(str_fix[c] == 'B' ){
        if(m_dof[4] != 1 ) {
          error() << "--> Constrain does not match number of degree of freedom" << endreq;
          return StatusCode::FAILURE;
        }
        for(int n=0; n < 4; n++) if(m_dof[n] == 1) dcnt++;
        dcnt = alipar*dcnt;
      }
      
      if(str_fix[c] == 'C' ){
        if(m_dof[5] != 1 ) {
          error() << "--> Constrain does not match number of degree of freedom" << endreq;
          return StatusCode::FAILURE;
        }
        for(int n=0; n < 5; n++) if(m_dof[n] == 1) dcnt++;
        dcnt = alipar*dcnt;
      }
      
      if( str_fix[c] == '_' ) {
	
        //cast char
        int num = (str_fix[c+2] - '0');
        //1st entry is Station
        if(str_fix[c+1] == 'S') elements[0] = num;
        //2nd entry is Layer
        if(str_fix[c+1] == 'L') elements[1] = num;
        //3rd entry is Quarter
        if(str_fix[c+1] == 'Q') elements[2] = num;
        //4th entry is Module
        if(str_fix[c+1] == 'M') elements[3] = num;
      }
    }
    
    for(int j=0; j < 4 ; j++){
      debug() << " --> elements["<<j<<"] = "<< elements[j] << endreq; 
    }
    //general layout of detector geometry
    int modules   = 9;
    int quarters  = 4;
    int layers    = 4;
    
    //calculate dofs with respect to alignable objects
    if(m_otStation)  layers=quarters=modules=1;
    if(m_otLayer)    quarters=modules=1;
    if(m_otQuadrant) modules=1;
    if(m_otHalfLayer) {
      modules=1;
      quarters /= 2;}
    if(m_otModule) quarters /= 2;
    
    int modStation = (elements[0]-1) * layers*quarters*modules;
    int modLayer   = (elements[1]-1) * quarters*modules;
    int modQuarter = (elements[2]-1) * modules;
    int modModule  = (elements[3]-1);
    
    //since we add the number of modules, we have to set not used 
    //objects to zero 
    if(m_otStation)  modLayer=modQuarter=modModule=0;
    if(m_otLayer)    modQuarter=modModule=0;
    if(m_otQuadrant) modModule=0;
    if(m_otHalfLayer) {
      if(elements[2]%2==0) modQuarter=1;
      if(elements[2]%2!=0) modQuarter=0;
      modModule=0; 
    }
    
    
    int moduleRnk  = modStation + modLayer + modQuarter + modModule;// = rank of the selected module(if m_otModule = true)
    debug() << "  modStation " << modStation << "  modLayer " << modLayer 
            << " modQuarter " << modQuarter
            << " modModule " << modModule <<  " = rnk : " << moduleRnk << endreq;;
    
    int nObjects = 0;
    // fix a single module 
    if(elements[3] != 0 && elements[2] != 0 && elements[1] != 0){
      nObjects  = 1;
    }
    // fix all modules in this quarter
    if(elements[3] == 0 && elements[2] != 0 && elements[1] != 0){
      moduleRnk = modStation + modLayer + modQuarter;//= rank of 1st module in this quarter(if m_otModule = true)
      nObjects  = modules;
    }
    //fix all modules in this layer
    if(elements[3] == 0 && elements[2] == 0 && elements[1] != 0 ){
      moduleRnk =  modStation + modLayer;
      nObjects  =  quarters*modules;
    }
    //fix all modules in this station
    if(elements[3] == 0 && elements[2] == 0 && elements[1] == 0 ){
      moduleRnk =  modStation;
      nObjects  =  layers*quarters*modules;
    }
    
    StatusCode sc;
    //fix everything else; single or in a set
    if(nObjects != 0) {
      info() << "---> nObjects to fix = "<< nObjects << " ;fix rank nr :" << endreq;
      for(int j = 0; j< nObjects; j++){
        int fix = moduleRnk+j+dcnt;
        info()<< fix <<" ; ";	
        sc = m_Centipede->ParSig(fix, 0.0); 
        if ( sc.isFailure() ) {
          error() << "Failure in fixing position of object "<< endreq;
          return sc;
        }
      }
      info() <<endreq << "---------------------------------------"<<endreq;
      
    }
    else{
      m_cntfix++;
      info() << "Successfully fixed objects " << endreq;
    }
  }
  return StatusCode::SUCCESS;
}


StatusCode TAConfig::Rank( LHCb::LHCbID &id, int & r ) {
  std::string name;
  if ( ot_detector && id.isOT() ) {
    // Check where this hit was:
    if ( m_otModule ) {
      name = (m_ot->findModule( id.otID() ))->name();
    } else if ( m_otQuadrant ) {
      name = (m_ot->findQuarter( id.otID() ))->name();
    } else if ( m_otLayer ) {
      name = (m_ot->findLayer( id.otID() ))->name();
    } else if ( m_otStation ) {
      name = (m_ot->findStation( id.otID() ))->name();
    } else if ( m_otHalfLayer ) {//halflayer defined via modules
      name = (m_ot->findModule( id.otID() ))->name();
    } else if ( m_otModuleShort ) {//halflayer defined via modules
      name = (m_ot->findModule( id.otID() ))->name();
    }
  } else if ( it_detector && id.isIT() ) {
    if ( m_itLadder ) {
      // first find layer, then ladder
      DeITLayer * itl = dynamic_cast<DeITLayer*> (m_it->findLayer( id.stID() ));
      name = (itl->findLadder( id.stID() ))->name();
    } else if ( m_itLayer ) {
      name = (m_it->findLayer( id.stID() ))->name();
    } else if ( m_itBox ) {
      // first get station, then get box...
      DeITStation *its = dynamic_cast<DeITStation *> (m_it->findStation( id.stID() ));
      name = (its->findBox( id.stID() ))->name();
    } else if ( m_itStation ) {
      name = (m_it->findStation( id.stID() ))->name();
    }
  } else if ( tt_detector && id.isTT() ) {
    if ( m_ttLadder ) {
      name = (m_tt->findSector( id.stID() ))->name();
    } else if ( m_ttLayer ) {
      name = (m_tt->findLayer( id.stID() ))->name();
    } else if ( m_ttStation ) {
      name = (m_tt->findStation( id.stID() ))->name();
    }
  } else if ( velo_detector && id.isVelo() ) {
    // not yet done...
  }
  std::map<std::string,int>::iterator t = m_C_pos.find( name );
  if ( t != m_C_pos.end() ) {
    debug() << "key of map = " << (*t).first << endreq;
    debug() << "value of map = Rank nr. = " << (*t).second << endreq;
    r = (*t).second;
    return StatusCode::SUCCESS;
  } else
    return StatusCode::FAILURE;
  return StatusCode::FAILURE;
}


/*********************************************
 *                                           *
 *   VectortoArray                           *
 * converting vectors to arrays              *
 *                                           *
 *********************************************/
void TAConfig::VectortoArray(const std::vector<double> &the_vector, double the_array[] ){
  std::vector<double>::const_iterator the_iterator = the_vector.begin();
  unsigned int counter = 0;
  while( the_iterator < the_vector.end() ){
    the_array[counter] = *the_iterator;
    //    the_array[counter] = the_vector[counter];
    the_iterator++;
    counter++;
  }
}

/*****************************************
 * Convert array to vector...
 ****************************************/

void TAConfig::ArraytoVector( const double the_array[], std::vector<double> &the_vector , int ent) {
  the_vector.resize(ent,0.);
  for(int i=0; i< ent;++i){
    the_vector[i] = the_array[i] ;
  }
}

StatusCode TAConfig::finalize() {
  
  return GaudiTupleTool::finalize();
  
}
// Florin New procedure to set track paramdters for tracks (I do not know yet if I realy need this since track parameters    
// are propagated at this level too and could be used to set trtx and co directly)
void TAConfig::SetTrackPar(const std::vector<double>  vt) {
  
  m_trx0.clear();
  m_trtx.clear();
  m_try0.clear();
  m_trty.clear();
  m_trQ.clear();
  
  if(vt.size()==4){
    m_trx0[0] = (vt[0]);
    m_trtx[0] = (vt[1]);
    m_try0[0] = (vt[2]);
    m_trty[0] = (vt[3]);
    m_trQ[0]  = 0.;
    
    debug() << " trx0[0] = " << m_trx0[0] <<" trtx[0] = " << m_trtx[0] << " try0[0]= " << m_try0[0] << " trty[0]= " << m_trty[0] 
            <<endmsg;
  }
  if(vt.size()==5){
    m_trx0[0] = (vt[0]);
    m_trtx[0] = (vt[1]);
    m_try0[0] = (vt[2]);
    m_trty[0] = (vt[3]);
    m_trQ[0]  = (vt[4]);
    debug() << " trx0[0] = " << m_trx0[0] <<" trtx[0] = " << m_trtx[0] 
            << " try0[0]= " << m_try0[0] << " trty[0]= " << m_trty[0] 
            <<  " trQ[0] = " << m_trQ[0] <<endmsg;
  }
}

// Florin New procedure to set alignment parameters
StatusCode TAConfig::SetAlignmentPar(const std::vector<double> & estimated) {
  for(unsigned int i=0; i< estimated.size(); i++ )
    m_estimate[i]=estimated[i];
  
  //     info() << " setting up estimated alignment parameters: " << endreq;
  //      for( int a=0;a<m_estimate.size();a++){
  //        info() << a << "  new Par = " << m_estimate.at(a)<< endreq;
  //      }
  
  return StatusCode::SUCCESS;
}
void TAConfig::MakeAlignParVec() {
  //  m_estimateB4.resize(n,0.0); // initiate to null values the B4 vector and the vector.
  //  m_estimate.resize(n,0.0);
  m_estimateB4.resize(m_n_dof*m_nAlignObj);
  m_estimate.resize(m_n_dof*m_nAlignObj);
  
}
void TAConfig::MakeTrackParVec() {
  debug() << " ---> in MakeTrackParVec() " <<  endreq;
  
  m_trx0.resize(1,0.);
  m_trtx.resize(1,0.);
  m_try0.resize(1,0.);
  m_trty.resize(1,0.);
  m_trQ.resize(1,0.);
}

// **************************************************************************
//   
// PrintParameter sets up the memory,i.e. the new constants are saved in memory 
// to be used in next iteration.
// Get the new information from the DetElements.
// 
// for a stereo layer: translation[0] = translation in measurement direction !!!
// (same definition as we do the alignment)
//
// NOTE:
// What is saved in the XML file at the end of the job are the misalignments in 
// the coordinate system of the object (e.g. module)
//
// **************************************************************************** 


StatusCode TAConfig::PrintParameters( std::vector<double> &ali_par , int /*iteration*/ ) {
  // Alternative:
  debug() << " --->  alipar.size = " << ali_par.size() << endreq;
  
  for(unsigned i=0;i<ali_par.size();i++){
    info() <<i<< " param of prev iter = "<< m_prevAlipar[i] << " param of this iter = "<< ali_par[i];
    m_prevAlipar[i] += ali_par[i];      
    info() << " --> sum w param of this iter = " << m_prevAlipar[i] << endreq;
    debug() << " begin m_prevAlipar["<<i<<"] "<< m_prevAlipar[i] << endreq;
  }
  
  
  
  std::vector<double>::iterator i       = ali_par.begin();
  std::map<std::string,int>::iterator t = m_C_pos.begin();
  
  // loop over the detector elements we wanted to align.
  // check for each element which degrees of freedom were determined.
  //  int cnt_rnk  = -1;
  unsigned cnt = 0;
  for ( ; t != m_C_pos.end(); ++t ) {
    debug() << "--------------------------------------------------------------------------------------"<<endreq;
    debug() << "Parameter name = " << t->first << " || rank = " << t->second  << " ,m_nAlignObj=" << m_nAlignObj 
	    << ", cnt = " << cnt << endreq;
    
    if ( m_dof[0] ){ 
      debug() << "cnt="<<cnt<< " ali_par["<<t->second+cnt*m_nAlignObj<<"]=" << ali_par[t->second+cnt*m_nAlignObj]<<endreq; 
      cnt++;
    }
    if ( m_dof[1] ){
      debug() << "cnt="<<cnt<< " ali_par["<<t->second+cnt*m_nAlignObj<<"]=" << ali_par[t->second+cnt*m_nAlignObj]<<endreq; 
      cnt++;
    }
    if ( m_dof[2] ){
      debug() << "cnt="<<cnt<< " ali_par["<<t->second+cnt*m_nAlignObj<<"]=" << ali_par[t->second+cnt*m_nAlignObj]<<endreq; 
      cnt++; 
    }
    if ( m_dof[3] ){
      debug() << "cnt="<<cnt<< " ali_par["<<t->second+cnt*m_nAlignObj<<"]=" << ali_par[t->second+cnt*m_nAlignObj]<<endreq; 
      cnt++; 
    }
    if ( m_dof[4] ){
      debug() << "cnt="<<cnt<< " ali_par["<<t->second+cnt*m_nAlignObj<<"]=" << ali_par[t->second+cnt*m_nAlignObj]<<endreq; 
      cnt++; 
    }
    if ( m_dof[5] ){
      debug() << "cnt="<<cnt<< " ali_par["<<t->second+cnt*m_nAlignObj<<"]=" << ali_par[t->second+cnt*m_nAlignObj]<<endreq; 
      cnt++; 
    }
    cnt=0;
    
    IDetectorElement *det = getDet<IDetectorElement>(t->first);
    IGeometryInfo *geo = det->geometry();
    
    
    // get the input misalignments
    // i.e. alignment constants from previous 
    // iteration
    const Gaudi::Transform3D misalGlobal = geo->toGlobalMatrix();
    const Gaudi::Transform3D nominGlobal = geo->toGlobalMatrixNominal();
    const Gaudi::Transform3D deltaGlobal = misalGlobal*nominGlobal.Inverse();
    const Gaudi::Transform3D deltalocal  = nominGlobal.Inverse()*deltaGlobal*nominGlobal;
    std::vector<double> rotGlobal(3,0.), shiftGlobal(3,0.);
    std::vector<double> rotlocal(3,0.), shiftlocal(3,0.);
    
    DetDesc::getZYXTransformParameters(deltalocal, shiftlocal, rotlocal);
    DetDesc::getZYXTransformParameters(deltaGlobal, shiftGlobal, rotGlobal);            
    
    //       info() << "Rnk = "<<t->second << " name " << t->first << endreq
    //              << " alignment param     = " <<  ali_par[t->second+cnt*m_nAlignObj] << endreq 
    info ()<< " prev : transl.(loc) = "  <<std::setw(15) <<  shiftlocal 
	   << " prev : rotation (loc)= " <<std::setw(15) <<   rotlocal << endreq 
	   << " prev : transl.(glo) = "  <<std::setw(15) <<  shiftGlobal 
	   << " prev : rotation (glo)= " <<std::setw(15) <<   rotGlobal <<endreq
	   << "--------" << endreq; 
    
    
    Gaudi::XYZVector  nomModT;
    Gaudi::Rotation3D nomModR;
    nominGlobal.GetDecomposition(nomModR,nomModT);
    const Gaudi::XYZPoint Mod(nomModT.x(),nomModT.y(),nomModT.z());
    debug() << "--> Mod   ="<< Mod <<endreq;
    
    
    // Convert alignment parameters into
    // a vector containing translations
    // a vector containing rotations and
    // a vector with the pivot point.
    // That vector should actually be obtained from the detector element!
    std::vector<double> translation(3,0.);
    std::vector<double> rotation(3,0.);
    std::vector<double> pivot(3,0.);
    if(m_otModule){
      double thisStereo = 0.;
      std::multimap<int, std::pair<double,double> >::iterator i = m_RnkZAngle.begin();
      for(; i != m_RnkZAngle.end(); i++){
	if((i->first == t->second) ) {
	  thisStereo = (i->second).second;
	  debug() <<"t->second "<< t->second << " III rank = " << i->first << " zpos = " 
		  << (i->second).first  << " stereo "<< (i->second).second<<endreq; 
	}
      }
      pivot[0] = -Mod.y()*sin(thisStereo);
      pivot[1] = -Mod.y();
      pivot[2] = -Mod.y()*tan(0.0036);
    }
    if(m_otHalfLayer){
      Gaudi::XYZPoint mref = m_HLRefmap[t->second];// nominal
      pivot[0] = mref.x()-Mod.x();
      pivot[1] = mref.y()-Mod.y();
      pivot[2] = mref.z()-Mod.z();
    }
    
    // !! mit pivot kann man input misalignment nicht korrekt bestimmen!!      
    // Get the translation, rotation and pivot vectors from
    // the geometry object:
    //       const Gaudi::Transform3D position = geo->toLocalMatrixNominal();
    //       Gaudi::XYZVector P( 0.0, 0.0, 0.0 );
    //       Gaudi::Rotation3D R;
    //       position.GetDecomposition( R, P );
    //       pivot.push_back( P.X() );
    //       pivot.push_back( P.Y() );
    //       pivot.push_back( P.Z() );
    
    // check if we have simulation or real data
    // signs of alignment constants and input differ for simulation
    // -> below addition gives zero for new DB entries which is good for
    //    iterations using simulated data;
    // for Dz the sign is the same for sim and real data
    // -> for simulation one has to change the sign for Dz to get the zero
    //    for the addition;
    // want to calculate Input-AlignmentConstants which gives position of 'sign'
    double sign = 1.;
    if(m_Simulation) sign = -1.;
    bool ModSim = false;
    if(m_Simulation && m_otModule) ModSim = true;
    
    // fill the translation & rotation vectors
    // by looping over the number of degrees of freedom
    // since alipar has size of rank, only update new vectors 
    // for  new rank (m_Cpos may contain several entries 
    // with the same rank no.)
      // add input and reconstructed misalignments:
      // alipar is in measurement direction, got it from alignment in global system!! 
      // the layer is shifted in the global LHCb co system, i.e. it is shifted by the 
      // difference between input and output misalignment
    if ( m_dof[0] ){
      translation[0] = sign*ali_par[t->second]+shiftlocal[0]; 
      cnt++;
    }
    if ( m_dof[1] ) {
      translation[1] = ali_par[t->second+cnt*m_nAlignObj]+shiftlocal[1];
      cnt++;
    }
    if ( m_dof[2] ) {
      translation[2] = sign*ali_par[t->second+cnt*m_nAlignObj]+shiftlocal[2];
      cnt++;
    }
    if ( m_dof[3] ) {
      rotation[0] = sign*ali_par[t->second+cnt*m_nAlignObj]+rotlocal[0];
      cnt++;
    }
    if ( m_dof[4] ) {
      rotation[1] = sign*ali_par[t->second+cnt*m_nAlignObj]+rotlocal[1];
      cnt++;
    }
    if ( m_dof[5] ) {
      rotation[2] = sign*ali_par[t->second+cnt*m_nAlignObj]+rotlocal[2];
      cnt++;
    }
    cnt=0;
    //      if(!m_Simulation){ //i.e. of no input misalignments are known!!!!!!
    if( !ModSim || m_otHalfLayer){ // alignment on data (not MC!) for half-layers
      // Need to take care of the pre-existing conditions, so that they don't get lost
      // between iterations!!! 
      // Therefore, add them...
      // add input and reconstructed misalignments:
      // alipar is in measurement direction, got it from alignment in global system!! 
      // modules are shifted in the frame of its parents, therefore we have to shift 
      // them not with the difference input-output but with the output.
      if ( m_dof[0] ){
	debug() << "  m_prevAlipar[0] = " <<  m_prevAlipar[t->second+cnt*m_nAlignObj] << endreq;         
	translation[0]   = sign*ali_par[t->second+cnt*m_nAlignObj] + shiftlocal[0];//shiftlocal[0]; 
	cnt++;
      } else translation[0] = shiftlocal[0];
      if ( m_dof[1] ) {
	debug() << "  m_prevAlipar[1] = " <<  m_prevAlipar[t->second+cnt*m_nAlignObj] << endreq;         
	translation[1]    = ali_par[t->second+cnt*m_nAlignObj] + shiftlocal[1];//shiftlocal[1];
	cnt++;
      } else translation[1] = shiftlocal[1];
      if ( m_dof[2] ) {
	debug() << "  m_prevAlipar[2] = " <<  m_prevAlipar[t->second+cnt*m_nAlignObj] << endreq;
	translation[2]   = sign*ali_par[t->second+cnt*m_nAlignObj] + shiftlocal[2];//shiftlocal[2];
	cnt++;
      } else translation[2] = shiftlocal[2];
      if ( m_dof[3] ) {
	debug() << "  m_prevAlipar[3] = " <<  m_prevAlipar[t->second+cnt*m_nAlignObj] << endreq;
	rotation[0]      = sign*ali_par[t->second+cnt*m_nAlignObj] + rotlocal[0];//rotlocal[0];
	cnt++;
      } else rotation[0] = rotlocal[0];
      if ( m_dof[4] ) {
	debug() << "  m_prevAlipar[4] = " <<  m_prevAlipar[t->second+cnt*m_nAlignObj] << endreq;         
	rotation[1]      = sign*ali_par[t->second+cnt*m_nAlignObj] + rotlocal[1];//rotlocal[1];
	cnt++;
      } else rotation[1] = rotlocal[1];
      if ( m_dof[5] ) {
	debug() << "  m_prevAlipar[5] = " <<  m_prevAlipar[t->second+cnt*m_nAlignObj] << endreq;         
	rotation[2]      = sign*ali_par[t->second+cnt*m_nAlignObj] + rotlocal[2];//rotlocal[2];
	cnt++;
      } else rotation[2] = rotlocal[2];
      cnt=0;
      
    }
    
    
    
    
    info() << " final Translation vector: " << translation  
	    << " final Rotation vector   : " << rotation << endreq
	    << " pivot point             : " << pivot << endreq
	    <<"------------------------------------------------------"<<endreq;
    
    
    StatusCode sc = geo->ownToOffNominalParams(translation,
					       rotation,
					       pivot);
    if ( sc.isFailure() ) {
      error() << "Error storing translation, rotation and pivot vectors\n"
	      << "from geometry object" << endreq;
      error() << "Translation vector : " << translation << "\n"
	      << "Rotation vector    : " << rotation << "\n"
	      << "pivot vector       : " << pivot << endreq;
      return StatusCode::FAILURE;    
    }
    
    const Gaudi::Transform3D misalGlobalnew = geo->toGlobalMatrix();
    const Gaudi::Transform3D nominGlobalnew = geo->toGlobalMatrixNominal();
    const Gaudi::Transform3D deltaGlobalnew = misalGlobalnew*nominGlobalnew.Inverse();
    const Gaudi::Transform3D deltalocalnew  = nominGlobalnew.Inverse()*deltaGlobalnew*nominGlobalnew;
    std::vector<double> rotGlobalnew(3,0.), shiftGlobalnew(3,0.);
    std::vector<double> rotlocalnew(3,0.), shiftlocalnew(3,0.);
    std::vector<double> pivotnew = pivot;
    DetDesc::getZYXTransformParameters(deltalocalnew, shiftlocalnew, rotlocalnew,pivot);
    DetDesc::getZYXTransformParameters(deltaGlobalnew, shiftGlobalnew, rotGlobalnew,pivot);           
    info() << " new : pivot          " <<std::setw(15) << pivotnew << endreq
	    << " new : transl.(loc) = "  <<std::setw(15) <<  shiftlocalnew 
	    << " new : rotation (loc)= " <<std::setw(15) <<   rotlocalnew << endreq 
	    << " new : transl.(glo) = "  <<std::setw(15) <<  shiftGlobalnew 
	    << " new : rotation (glo)= " <<std::setw(15) <<   rotGlobalnew <<endreq
	    << "--------" << endreq; 
    
    
    if (sc.isFailure()) {
      std::cout << "Failed to update local delta matrix of detector element " + det->name() <<  std::endl;
      return StatusCode::FAILURE;  
    }
  }
  return StatusCode::SUCCESS;
}

void TAConfig::CheckLChi2(const double &chi2, const int &trsize,const int &nmes, bool & flagC, double mscale)
{
  double bchi2=chi2;
  bool flagb=false;
  //info()<<"start scale  " <<m_scaleIter << endreq;
  //  if(neXt==true){ m_scaleIter=sqrt(m_scaleIter);}  
  //info()<<"new scale  " <<  mscale << endreq;
  double scale=mscale; 
  if(mscale<3.0){ scale= 1;}
  
  
  double sdtdevb= m_nstd; //set in options
  m_Centipede->CheckLChi2(bchi2,trsize,nmes, scale, sdtdevb, flagb );
  
  flagC=flagb; //if true, than track passed chi2 cut
}


StatusCode TAConfig::NewMeasurement(double & meas, const int rank,
                                    const struct Point pred, 
				    //				    const double zpos,
                                    const double stereo_angle)
{
  
  //info() << " ---> called TACONFIG::new measurement " << endreq;
  m_derGB.clear();
  m_derGB.resize( m_n_dof*m_nAlignObj ,0.);
  std::vector<double> trt;
  if(m_ntrack_pars==4){
    trt.push_back(m_trx0[0]);
    trt.push_back(m_trtx[0]);
    trt.push_back(m_try0[0]);
    trt.push_back(m_trty[0]);
  }
  if(m_ntrack_pars==5){
    trt.push_back(m_trx0[0]);
    trt.push_back(m_trtx[0]);
    trt.push_back(m_try0[0]);
    trt.push_back(m_trty[0]);
    trt.push_back(m_trQ[0]);
  }
  
  Gaudi::XYZVector origin( trt.at(0), trt.at(2), 0.0);
  Gaudi::XYZVector slopes( trt.at(1), trt.at(3), 0.0);
  m_derivatives->SetGlobal( slopes, origin, pred, m_derGB, rank, stereo_angle );
  int cnt = 0;
  if(m_dof[0]){
    //info() << " ---> meas before ="<< meas << endreq;
    meas -= m_estimate[rank]*m_derGB[rank];
    //info() << " ---> meas after ="<< meas << endreq;
    plot(m_estimate[rank],"x estimate","x estimate",-10.,10,100);
    cnt++;
  }
  if(m_dof[1]){
    meas -= m_estimate[rank+cnt*m_nAlignObj]*m_derGB[rank+cnt*m_nAlignObj];  
    plot(m_estimate[rank+cnt*m_nAlignObj],"y estimate","y estimate",-10.,10,100);
    cnt++;
  }
  if(m_dof[2]){
    meas -= m_estimate[rank+cnt*m_nAlignObj]*m_derGB[rank+cnt*m_nAlignObj]; 
    plot(m_estimate[rank+cnt*m_nAlignObj],"z estimate","z estimate",-10.,10,100);
    cnt++;
  }
  if(m_dof[3]){
    meas -= m_estimate[rank+cnt*m_nAlignObj]*m_derGB[rank+cnt*m_nAlignObj];  
    plot(m_estimate[rank+cnt*m_nAlignObj],"a estimate","a estimate",-0.01,0.01,100);
    cnt++;
  }
  if(m_dof[4]){
    meas -= m_estimate[rank+cnt*m_nAlignObj]*m_derGB[rank+cnt*m_nAlignObj];  
    plot(m_estimate[rank+cnt*m_nAlignObj],"b estimate","b estimate",-0.01,0.01,100);   
    cnt++;
  }
  if(m_dof[5]){
    //    info() << " ---> meas before ="<< meas << endreq;
    meas -= m_estimate[rank+cnt*m_nAlignObj]*m_derGB[rank+cnt*m_nAlignObj];  
    //info() << " ---> meas after ="<< meas << endreq;
    plot(m_estimate[rank+cnt*m_nAlignObj],"c estimate","c estimate",-0.01,0.01,500);   
    //info() << "---> rank " << rank << " estmate["<<rank+cnt*m_nAlignObj<<"] ="<< m_estimate[rank+cnt*m_nAlignObj]
    // <<endreq;
    //   << endreq  << "cnt = " << cnt << endreq;
  }
  
  
  return StatusCode::SUCCESS;
  
}
StatusCode TAConfig::ConstrainMovements(){ 
  int  cnt       = 0;
  double vdof[6] = {0.,0.,0.,0.,0.,0.};//constrain value per parameter (dof)
  int size       = m_OTConstrainMove.size();
  for (int i=0; i < size; i++){
    info()<< "--> Constrain Movements string = " << m_OTConstrainMove[i] << endreq;
    //interpret every string
    std::string str_fix = m_OTConstrainMove[i];
    for(unsigned int c=0; c < str_fix.size(); c++){
      
      if(str_fix[c] == 'X' ) {
        vdof[0] = -1;
        if( !m_dof[0]) {
          error() << " DoF X is not selected for alignment " << endreq;
          return StatusCode::FAILURE;
        }
      }
      if(str_fix[c] == 'Y' ) {
        vdof[1] = -1;
        if( !m_dof[1]) {
          error() << " DoF Y is not selected for alignment " << endreq;
          return StatusCode::FAILURE;
        }
      }
      if(str_fix[c] == 'Z' ) {
        vdof[2] = -1;
        if( !m_dof[2]) {
          error() << " DoF Z is not selected for alignment " << endreq;
          return StatusCode::FAILURE;
        }
      }
      if(str_fix[c] == 'A' ) {
        vdof[3] = -1;
        if( !m_dof[3]) {
          error() << " DoF A is not selected for alignment " << endreq;
          return StatusCode::FAILURE;
        }
      }
      if(str_fix[c] == 'B' ) {
        vdof[4] = -1;
        if( !m_dof[4]) {
          error() << " DoF B is not selected for alignment " << endreq;
          return StatusCode::FAILURE;
        }
      }
      if(str_fix[c] == 'C' ) {
        vdof[5] = -1;
        if( !m_dof[5]) {
          error() << " DoF C is not selected for alignment " << endreq;
          return StatusCode::FAILURE;
        }
      }
      //get the value of the constrain
      if(str_fix[c] == '=' ){
        std::vector<char> vec;
        for(unsigned int i=c+1; i < str_fix.size(); i++){
          vec.push_back(str_fix[i]);
        }
        char value[vec.size()];  
        //put the char from the vector into a char[]
        for(unsigned a=0; a<vec.size(); a++){
          value[a] = vec.at(a);
          debug() << " value " << a << " " << value[a] << " size of vc " << vec.size() << endreq;
        }
        std::string val = value;
        for(int k=0;k<6;k++){
          if(vdof[k] == -1) vdof[k] = atof(val.c_str()); 
          debug() << k << " vdof  = "<< vdof[k]  << endreq;
        }
        vec.clear(); 
      }
    }// one string
  }//# strings 
  
  
  
  for( int i=0; i< m_nAlignObj; i++){        
    if(m_dof[0]){
      if( vdof[0] != 0.){
        info() << "--> X call ParSig["<<i<<"] = "<<vdof[0]<<endreq;
        m_Centipede->ParSig(i,vdof[0]);     
      }
      cnt++;
    }
    if(m_dof[1] ){
      if(vdof[1] != 0.){
        info() << "--> call ParSig["<<i+cnt*m_nAlignObj<<"] = "<<vdof[1]<<endreq;
        m_Centipede->ParSig(i+cnt*m_nAlignObj,vdof[1]);     
      }
      cnt++;
    }
    if(m_dof[2]){
      if( vdof[2] != 0.){
        info() << "--> Z call ParSig["<<i+cnt*m_nAlignObj<<"] = "<<vdof[2]<<endreq;     
        m_Centipede->ParSig(i+cnt*m_nAlignObj,vdof[2]);     
      }
      cnt++;
    }
    if(m_dof[3]){
      if( vdof[3] != 0.){
        info() << "--> A call ParSig["<<i+cnt*m_nAlignObj<<"] = "<<vdof[3]<<endreq;
        m_Centipede->ParSig(i+cnt*m_nAlignObj,vdof[3]);     
      }
      cnt++;
    }
    if(m_dof[4]){
      if(vdof[4] != 0.){
        info() << "--> B call ParSig["<<i+cnt*m_nAlignObj<<"] = "<<vdof[4]<<endreq;
        m_Centipede->ParSig(i+cnt*m_nAlignObj,vdof[4]);     
      }
      cnt++;
    }
    if(m_dof[5]){
      if(vdof[5] != 0.){
        info() << "--> C call ParSig["<<i+cnt*m_nAlignObj<<"] = "<<vdof[5]<<endreq;
        m_Centipede->ParSig(i+cnt*m_nAlignObj,vdof[5]);     
      }
    }
    cnt = 0;
  }
  return StatusCode::SUCCESS;
  
}

StatusCode TAConfig::GetHalflayerReference(DeOTLayer* lay, Gaudi::XYZPoint& ref, Gaudi::XYZPoint& refnom,int rank)
{
  double sumX[4]={0.,0.,0.,0.}; double meanX[4]={0.,0.,0.,0.};
  double sumY[4]={0.,0.,0.,0.}; double meanY[4]={0.,0.,0.,0.};
  double sumZ[4]={0.,0.,0.,0.}; double meanZ[4]={0.,0.,0.,0.};
  double sumXnom[4]={0.,0.,0.,0.}; double meanXnom[4]={0.,0.,0.,0.};
  double sumYnom[4]={0.,0.,0.,0.}; double meanYnom[4]={0.,0.,0.,0.};
  double sumZnom[4]={0.,0.,0.,0.}; double meanZnom[4]={0.,0.,0.,0.};
  
  //get the quarters of the layer
  std::vector<DeOTQuarter*>  quart = lay->quarters();
  for(unsigned k=0; k< quart.size();k++){
    //Cside
    if(rank%2 == 0 && k%2 ==0){
      
      //get the modules
      std::vector<DeOTModule*>  mod = quart.at(k)->modules();
      for(unsigned l=0; l< mod.size();l++){ //neglect small modules at beampipe   
	
        Gaudi::Rotation3D R,Rnom;
        Gaudi::XYZVector  T,Tnom;
        mod.at(l)->geometry()->toGlobalMatrix().GetDecomposition( R, T);    // get center of Module including misalignments
        const Gaudi::XYZPoint misal(T.x(),T.y(),T.z());
        mod.at(l)->geometry()->toGlobalMatrixNominal().GetDecomposition( Rnom, Tnom);    // get center of Module no misalignments
        const Gaudi::XYZPoint nom(Tnom.x(),Tnom.y(),Tnom.z());
        // debug() << "    misal ="<<misal << endreq;
        sumX[k] += T.x();
        sumY[k] += T.y();
        sumZ[k] += T.z();
        sumXnom[k] += Tnom.x();
        sumYnom[k] += Tnom.y();
        sumZnom[k] += Tnom.z();
	
      }
      meanX[k] = sumX[k]/9;
      meanY[k] = sumY[k]/9;
      meanZ[k] = sumZ[k]/9;
      meanXnom[k] = sumXnom[k]/9;
      meanYnom[k] = sumYnom[k]/9;
      meanZnom[k] = sumZnom[k]/9;
      
      debug() << rank << " Quarter "<< quart.at(k)->name() << endreq
              << " misal mean X,Y,Z "<< meanX[k]<<" " << meanY[k]<<" "<<meanZ[k]<<endreq
              << " nom   mean X,Y,Z "<< meanXnom[k]<<" " << meanYnom[k]<<" "<<meanZnom[k]<<endreq;
      
    }
    //Aside
    if(rank%2 != 0 && k%2 !=0){
      //get the modules
      std::vector<DeOTModule*>  mod = quart.at(k)->modules();
      for(unsigned l=0; l< mod.size();l++){ //neglect small modules at beampipe   
	
        Gaudi::Rotation3D R,Rnom;
        Gaudi::XYZVector  T,Tnom;
        mod.at(l)->geometry()->toGlobalMatrix().GetDecomposition( R, T);    // get center of Module including misalignments
        const Gaudi::XYZPoint misal(T.x(),T.y(),T.z());
        mod.at(l)->geometry()->toGlobalMatrixNominal().GetDecomposition( Rnom, Tnom);    // get center of Module no misalignments
        const Gaudi::XYZPoint nom(Tnom.x(),Tnom.y(),Tnom.z());
        // debug() << "    misal ="<<misal << endreq;
        sumX[k] += T.x();
        sumY[k] += T.y();
        sumZ[k] += T.z();
        sumXnom[k] += Tnom.x();
        sumYnom[k] += Tnom.y();
        sumZnom[k] += Tnom.z();
	
      }
      meanX[k] = sumX[k]/9;
      meanY[k] = sumY[k]/9;
      meanZ[k] = sumZ[k]/9;
      meanXnom[k] = sumXnom[k]/9;
      meanYnom[k] = sumYnom[k]/9;
      meanZnom[k] = sumZnom[k]/9;
      
      debug() << rank << " Quarter "<< quart.at(k)->name() << endreq
              << " misal mean X,Y,Z "<< meanX[k]<<" " << meanY[k]<<" "<<meanZ[k]<<endreq
              << " nom   mean X,Y,Z "<< meanXnom[k]<<" " << meanYnom[k]<<" "<<meanZnom[k]<<endreq;
      
    } 
  }
  
  double hlX=0.;
  double hlY=0.;
  double hlZ=0.;
  double hlXnom=0.;
  double hlYnom=0.;
  double hlZnom=0.;
  
  for(int i=0;i<4;i++){
    hlX += meanX[i];
    hlY += meanY[i];
    hlZ += meanZ[i];
    hlXnom += meanXnom[i];
    hlYnom += meanYnom[i];
    hlZnom += meanZnom[i];
  }
  
  //  debug() << " mean of both quarters : " << hlX/2 << " " << hlY/2<<" "<<hlZ/2 <<endreq;
  
  ref.SetX(hlX/2);
  ref.SetY(hlY/2);
  ref.SetZ(hlZ/2);
  
  refnom.SetX(hlXnom/2);
  refnom.SetY(hlYnom/2);
  refnom.SetZ(hlZnom/2);
  
  return StatusCode::SUCCESS;
  
  
}
