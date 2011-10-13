// $Id: $
// Include files

// from Gaudi
#include "GaudiKernel/AlgFactory.h"
#include "TrackInterfaces/ITrackFitter.h"

// local
#include "TrackSmeared.h"
#include <boost/foreach.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/assign/list_of.hpp>
#include <TRandom3.h>


//-----------------------------------------------------------------------------
// Implementation file for class : TrackSmeared
//
// 2010-08-27 : Sascha Stahl
//-----------------------------------------------------------------------------

// Declaration of the Algorithm Factory
DECLARE_ALGORITHM_FACTORY( TrackSmeared );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
TrackSmeared::TrackSmeared( const std::string& name,
                            ISvcLocator* pSvcLocator)
  : DVAlgorithm ( name , pSvcLocator )
{
  declareProperty( "InputLocation",m_trackLocation = LHCb::TrackLocation::Default);
  declareProperty( "OutputLocation",m_outputLocation = "Smeared");
  declareProperty( "ProtoLocation",m_protoLocation = LHCb::ProtoParticleLocation::Charged);
  declareProperty( "Seed",m_seed = 4357);
  declareProperty( "xpar1_mc",m_xpar1_mc = 7.6);
  declareProperty( "xpar2_mc",m_xpar2_mc = 24.8);
  declareProperty( "xpar1_data",m_xpar1_data = 9.2 );
  declareProperty( "xpar2_data",m_xpar2_data = 28.3 );
  declareProperty( "ypar1_mc",m_ypar1_mc = 8.0);
  declareProperty( "ypar2_mc",m_ypar2_mc = 24.4);
  declareProperty( "ypar1_data",m_ypar1_data = 9.0 );
  declareProperty( "ypar2_data",m_ypar2_data = 27.6 );
  declareProperty( "makePlots",m_plots = false );
  declareProperty( "smear",m_smear = true);
  declareProperty( "Scale",m_scale = 1.0);
  declareProperty( "usePhi",m_usePhi = true );
  declareProperty( "smearBest", m_smearBest = false);
  declareProperty( "smearProto", m_smearProto = false );
  declareProperty( "smearCopied", m_smearCopied = false );
  declareProperty( "paramsx_data",m_paramsx_data=
                   boost::assign::list_of< std::pair<double,double> >
                   (7.39663,25.375)(11.9523,30.8468)(12.1657,31.0318)(6.80381,26.2239)
                   (6.31307,26.1924)(12.0715,30.4908)(13.968,29.4501)(8.58082,24.9248) );
  declareProperty( "paramsy_data",m_paramsy_data=
                   boost::assign::list_of< std::pair<double,double> >
                   (11.3659,23.9497)(9.61021,29.9749)(7.62483,31.652)(9.04329,26.1887) 
                   (10.1667,24.9057)(10.4476,28.463)(7.64104,30.8264)(12.5512,23.7025) );
  declareProperty( "paramsx_mc",m_paramsx_mc=
                   boost::assign::list_of< std::pair<double,double> >
                   (5.94179,19.298)(10.4385,29.5275)(9.38674,30.6793)(6.45098,19.2148)
                   (7.05916,18.8357)(9.35444,29.9623)(8.79601,30.6255)(6.60782,18.9007) );
  declareProperty( "paramsy_mc",m_paramsy_mc=
                   boost::assign::list_of< std::pair<double,double> >
                   (9.85327,19.1644)(6.61456,28.2261)(6.21135,29.2543)(10.0675,19.4316)
                   (9.74202,19.0929)(6.13729,28.6578)(5.31868,29.8177)(10.6302,18.7859) );
  m_funcsy_mc.reserve(m_paramsy_mc.size());
  m_funcsx_mc.reserve(m_paramsx_mc.size());
  m_funcsy_data.reserve(m_paramsy_data.size());
  m_funcsx_data.reserve(m_paramsx_data.size());
  

}

//=============================================================================
// Destructor
//=============================================================================
TrackSmeared::~TrackSmeared() {}

//=============================================================================
// Initialization
//=============================================================================
StatusCode TrackSmeared::initialize() {
  StatusCode sc = DVAlgorithm::initialize();
  if ( sc.isFailure() ) return sc;

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Initialize" << endmsg;
  info()<< "==> Initialize" << endmsg;
  info()<< "++++++++++++++++" << endmsg;
  info()<< "smear: "<< m_smear << endmsg;
  info()<< "scale factor: "<< m_scale << endmsg;
  info()<< "Random seed: "<< m_seed << endmsg;
  info()<< "use Phi dependency: " << m_usePhi << endmsg;
  info()<< "smear Tracks in Best Container: "<< m_smearBest << endmsg;
  info()<< "smear Proto particles: "<< m_smearProto << endmsg;
  info()<< "smear copied tracks: "<< m_smearCopied << endmsg;
  info()<< "++++++++++++++++" << endmsg;

  //m_trackFitter = tool<ITrackFitter>("TrackInitFit");

  m_funcx_data = new TF1("funcx_data","[0]/1000.0+[1]*x",0.0,1.0);
  m_funcx_data->SetParameter(0,m_xpar1_data);
  m_funcx_data->SetParameter(1,m_xpar2_data);

  m_funcx_mc = new TF1("funcx_mc","[0]/1000.0+[1]*x",0.0,1.0);
  m_funcx_mc->SetParameter(0,m_xpar1_mc);
  m_funcx_mc->SetParameter(1,m_xpar2_mc);

  m_funcy_data = new TF1("funcy_data","[0]/1000.0+[1]*x",0.0,1.0);
  m_funcy_data->SetParameter(0,m_ypar1_data);
  m_funcy_data->SetParameter(1,m_ypar2_data);

  m_funcy_mc = new TF1("funcy_mc","[0]/1000.0+[1]*x",0.0,1.0);
  m_funcy_mc->SetParameter(0,m_ypar1_mc);
  m_funcy_mc->SetParameter(1,m_ypar2_mc);

  m_rnd=new TRandom3();
  m_rnd->SetSeed(m_seed);
  if(m_paramsx_data.size()!= m_paramsy_data.size() || m_paramsx_mc.size()!= m_paramsy_mc.size() || 
     m_paramsx_mc.size()!= m_paramsx_data.size() || m_paramsy_mc.size()!= m_paramsy_data.size() ||
     m_paramsx_data.size()!= m_paramsy_mc.size() || m_paramsx_mc.size()!= m_paramsy_data.size()  )
    return StatusCode::FAILURE;
  for(unsigned int i = 0; i< m_paramsx_data.size(); i++){
    m_funcsx_mc.push_back(new TF1(Form("funcx%d_mc",i),"[0]/1000.0+[1]*x",0.0,1.0));
    m_funcsx_mc[i]->SetParameter(0,m_paramsx_mc[i].first);
    m_funcsx_mc[i]->SetParameter(1,m_paramsx_mc[i].second);
  
    m_funcsy_mc.push_back( new TF1(Form("funcy%d_mc",i),"[0]/1000.0+[1]*x",0.0,1.0));
    m_funcsy_mc[i]->SetParameter(0,m_paramsy_mc[i].first);
    m_funcsy_mc[i]->SetParameter(1,m_paramsy_mc[i].second);
  
    m_funcsx_data.push_back( new TF1(Form("funcx%d_data",i),"[0]/1000.0+[1]*x",0.0,1.0));
    m_funcsx_data[i]->SetParameter(0,m_paramsx_data[i].first);
    m_funcsx_data[i]->SetParameter(1,m_paramsx_data[i].second);
    

    m_funcsy_data.push_back( new TF1(Form("funcy%d_data",i),"[0]/1000.0+[1]*x",0.0,1.0));
    m_funcsy_data[i]->SetParameter(0,m_paramsy_data[i].first);
    m_funcsy_data[i]->SetParameter(1,m_paramsy_data[i].second);
    
  }
    

  return StatusCode::SUCCESS;
}

double TrackSmeared::getSigmax( LHCb::State* state)
{
  double pt = state->pt();
  double inversept = pt > 0 ? 1.0/pt : 0.0;
  double sigma2 = m_funcx_data->Eval(inversept)*m_funcx_data->Eval(inversept)-
    m_funcx_mc->Eval(inversept)*m_funcx_mc->Eval(inversept);
  return sigma2>0 ? m_scale*sqrt(sigma2) : 0.0;

}

double TrackSmeared::getSigmay( LHCb::State* state)
{
  double pt = state->pt();
  double inversept = pt > 0 ? 1.0/pt : 0.0;
  double sigma2 = m_funcy_data->Eval(inversept)*m_funcy_data->Eval(inversept)-
    m_funcy_mc->Eval(inversept)*m_funcy_mc->Eval(inversept);
  return sigma2>0 ? m_scale*sqrt(sigma2) : 0.0;
}

double TrackSmeared::getSigma( LHCb::State* state,  std::vector<TF1*> funcs_data, std::vector<TF1*> funcs_mc)
{
  double pt = state->pt();
  double phi = state->momentum().phi() >= 0.0 ?  state->momentum().phi() :  state->momentum().phi() + 2.0*M_PI;
  double inversept = pt > 0 ? 1.0/pt : 0.0;
  int phiBin = (int)(phi/(2*M_PI)*funcs_data.size());
  //debug() << "phi " << state->momentum().phi() << '\t' << phi << " bin " << phiBin << " vector sizes" << funcs_data.size() << '\t' << funcs_mc.size() << endmsg;
  //debug() << phi - 2.0*M_PI << endmsg;
  TF1* func_data = funcs_data[phiBin];
  TF1* func_mc = funcs_mc[phiBin];
  double sigma2 = func_data->Eval(inversept)*func_data->Eval(inversept)-
    func_mc->Eval(inversept)*func_mc->Eval(inversept);
  return sigma2>0 ? m_scale*sqrt(sigma2) : 0.0;


}


void TrackSmeared::smearStates(  const std::vector< LHCb::State * > & states )
{

  //double pt = states[0]->pt();
  //double inversept = pt > 0 ? 1.0/pt : 0.0;
  const double tx = states[0]->tx();
  const double alpha = atan(tx);
  const double ty = states[0]->ty();
  const double gamma = atan(ty);
  //const double z = states[0]->z();
  //const double phi = states[0]->momentum().phi();
  double xrand = 0.0;
  double yrand = 0.0;
  //double zrand = 0.0;
  debug()<<"smear: "<<m_smear<<endmsg;
  double sigmax = m_usePhi ? getSigma(states[0],m_funcsx_data,m_funcsx_mc) : getSigmax(states[0]);
  double sigmay = m_usePhi ? getSigma(states[0],m_funcsy_data,m_funcsy_mc) : getSigmay(states[0]);

  if (m_smear)
  {
    xrand = m_rnd->Gaus(0.0,sigmax)/(cos(alpha)*cos(alpha));
    yrand = m_rnd->Gaus(0.0,sigmay)/(cos(gamma)*cos(gamma));
    debug()<<"rnd: "<<xrand<<" "<<yrand<<endmsg;
  }
  BOOST_FOREACH(LHCb::State* st ,states){
    const double x = st->x();
    const double y = st->y();
    st->setX(x+xrand);
    st->setY(y+yrand);
  }


}



StatusCode TrackSmeared::smearProto(){
  if  (!exist<LHCb::ProtoParticle::Container>(LHCb::ProtoParticleLocation::Charged)) return StatusCode::FAILURE;
  LHCb::ProtoParticles* pp = get< LHCb::ProtoParticles >(LHCb::ProtoParticleLocation::Charged) ;
  LHCb::ProtoParticles* newProtos = new LHCb::ProtoParticles();
  LHCb::Tracks* newTracks = new LHCb::Tracks();
  
  BOOST_FOREACH(const LHCb::ProtoParticle* proto, *pp){
    // copy protoparticle
    LHCb::ProtoParticle* smearedProto = proto->clone();
    debug()<<"######"<<endmsg;
    debug()<<"proto before p:"<<proto->track()->states()[0]->p()<<".x: "<< proto->track()->states()[0]->x()<<endmsg;
    // copy track associated to protoparicle
    LHCb::Track* smearedtrack = new LHCb::Track();
    smearedtrack->copy(*(smearedProto->track()));
    const std::vector< LHCb::State * > & states = smearedtrack->states();
  
    debug()<<"smeared track before p:"<<smearedtrack->states()[0]->p()<<".x: "<< smearedtrack->states()[0]->x()<<endmsg;
    smearStates(states);
    // save smeared track and set to protoparticle
    debug()<<"smeared track after p:"<<smearedtrack->states()[0]->p()<<".x: "<< smearedtrack->states()[0]->x()<<endmsg;
    smearedProto->setTrack(smearedtrack);
    debug()<<"proto after p:"<<smearedProto->track()->states()[0]->p()<<".x: "<< smearedProto->track()->states()[0]->x()<<endmsg;
    debug()<<"######"<<endmsg;
    newProtos->add(smearedProto);
    newTracks->add(smearedtrack);
    
  }
  put(newTracks, "Rec/Track/"+m_outputLocation);
  put(newProtos, "Rec/ProtoP/"+m_outputLocation);
  setFilterPassed(true);  // Mandatory. Set to true if event is accepted.
  return StatusCode::SUCCESS;
  
}

StatusCode TrackSmeared::smearBest(){
  if (m_trackLocation!=LHCb::TrackLocation::Default){
    debug()<<"You know what you are doing? Smearing a different container "<<endmsg;
  }
  
  if  (!exist<LHCb::Track::Range>(m_trackLocation))  return StatusCode::FAILURE;
  LHCb::Track::Range tracks = get<LHCb::Track::Range>(m_trackLocation) ;
  // loop over Best Track Container
  BOOST_FOREACH(const LHCb::Track* tr ,tracks){
    if (!tr->type() || (LHCb::Track::Long != tr->type() && LHCb::Track::Upstream!= tr->type()) ){
      debug()<< "No Long or Upstream track! Not smeared!" <<endmsg;
    } else {
      debug()<<"Ok smear the track!"<<endmsg;
      // smear state
      const std::vector< LHCb::State * > & states = tr->states();
      if (states.size()>0){
        debug()<<"old x position "<<states[0]->x()<<endmsg;
        debug()<<"old y position "<<states[0]->y()<<endmsg;
        debug()<<"old z position "<<states[0]->z()<<endmsg;
        smearStates(states);
        debug()<<"new x position "<<states[0]->x()<<endmsg;
        debug()<<"new y position "<<states[0]->y()<<endmsg;
        debug()<<"new z position "<<states[0]->z()<<endmsg;
      }
    }
  }
  setFilterPassed(true);  // Mandatory. Set to true if event is accepted.
  return StatusCode::SUCCESS;
}

StatusCode TrackSmeared::smearCopied(){
  
  
  if  (!exist<LHCb::Track::Range>(m_trackLocation))  return StatusCode::FAILURE;
  if  (!exist<LHCb::RichPIDs>(LHCb::RichPIDLocation::Offline))  return StatusCode::FAILURE;
  LHCb::Track::Range tracks = get<LHCb::Track::Range>(m_trackLocation) ;
  LHCb::RichPIDs* richids  = get<LHCb::RichPIDs>(LHCb::RichPIDLocation::Offline);
  LHCb::Tracks* newTracks = new LHCb::Tracks();
  LHCb::RichPIDs* newRichIDs = new LHCb::RichPIDs();


  BOOST_FOREACH(const LHCb::Track* tr ,tracks)
  {
    LHCb::Track* smearedtrack = new LHCb::Track();
    smearedtrack->copy(*tr);

    // restore RICH information
    LHCb::RichPID* myid = 0 ;

    BOOST_FOREACH(LHCb::RichPID* pid , *richids) {
      if (pid->track() == tr) {
        myid = pid;
        break;
      }
    }
    LHCb::RichPID* neueid = myid;
    if (neueid!=0){
      neueid->setTrack(smearedtrack);
      newRichIDs->add(neueid);
    }
    
    if (!tr->type() || LHCb::Track::Long != tr->type() ){
      debug()<< "No Long track! Not smeared!" <<endmsg;
      newTracks->add(smearedtrack);
    } else {
      // smear state
      const std::vector< LHCb::State * > & states = smearedtrack->states();
      debug()<<"old x position "<<states[0]->x()<<endmsg;
      debug()<<"old y position "<<states[0]->y()<<endmsg;
      debug()<<"old z position "<<states[0]->z()<<endmsg;
      
      if (states.size()>0){
        smearStates(states);
      }
      debug()<<"new x position "<<states[0]->x()<<endmsg;
      debug()<<"new y position "<<states[0]->y()<<endmsg;
      debug()<<"new z position "<<states[0]->z()<<endmsg;
      newTracks->add(smearedtrack);
    }
  }
  
  debug() << "adding Container to TES " <<"Rec/Track/"<<m_outputLocation<< endmsg;
  put(newTracks, "Rec/Track/"+m_outputLocation);
  put(newRichIDs,"Rec/Rich/"+m_outputLocation);
  
  setFilterPassed(true);  // Mandatory. Set to true if event is accepted.
  return StatusCode::SUCCESS;
}



//=============================================================================
// Main execution
//=============================================================================
StatusCode TrackSmeared::execute() {

  debug()<<"==> Execute! "<<endmsg;
  if ((m_smearBest && m_smearCopied) || (m_smearBest && m_smearProto) || (m_smearProto && m_smearCopied)){
    always()<<"OK, if you want to do this, you have to change the following lines and maybe some output containers"<<endmsg;
    return StatusCode::FAILURE;
  }
  if (m_smearBest){
    return smearBest();
  }else if (m_smearProto){
    return smearProto();
  }else if (m_smearCopied){
    return smearCopied();
  }
  always()<<"YOU HAVE TO DECIDE!!!"<<endmsg;
  return StatusCode::FAILURE;
}

//=============================================================================
//  Finalize
//=============================================================================
StatusCode TrackSmeared::finalize() {

  if ( msgLevel(MSG::DEBUG) ) debug() << "==> Finalize" << endmsg;

  return DVAlgorithm::finalize();
}

//=============================================================================
