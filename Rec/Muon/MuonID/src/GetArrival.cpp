// $Id: GetArrival.cpp,v 1.2 2009-07-06 08:13:41 cattanem Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 

// local
#include "GetArrival.h"
#include "Interpolator.h"
//#include <algorithm>

//boost
#include <boost/assign/list_of.hpp>

//-----------------------------------------------------------------------------
// Implementation file for class : GetArrival
//
// 2008-07-02 : Jose Angel Hernando Morata
//              Xabier Cid Vidal
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( GetArrival );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
GetArrival::GetArrival( const std::string& type,
                        const std::string& name,
                        const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<IGetArrival>(this);

  //remove smallest amongst all probabilities for 4 stations
  declareProperty("removeSmallest", m_removeSmallest=false);
  debug()<< "removeSmallest="<<m_removeSmallest<<endmsg;

  declareProperty("useFunct", m_useFunct=false);
  //use fitted functions instead of histograms
  debug()<< "useFunct="<<m_useFunct<<endmsg;
    
  //moms for all 4 stations
  declareProperty("moms", m_moms = boost::assign::list_of (-1.));
  debug()<< "moms="<<m_moms<<endmsg;

  //probs corresponding to those moms in each station
  declareProperty("probs", m_probs);
  debug()<< "probs="<<m_probs<<endmsg;
  
  //parameter alpha in fitted curves
  declareProperty("alpha", m_alpha = boost::assign::list_of (-1.));
  debug()<< "alpha="<<m_alpha<<endmsg;
  
  ////parameter beta in fitted curves
  declareProperty("beta", m_beta = boost::assign::list_of (-1.));
  debug()<< "beta="<<m_beta<<endmsg;

  ////eff of muon chambers
  declareProperty("eff", m_eff=.99);
  debug()<< "eff="<<m_eff<<endmsg;

  ////number of min hits for cls arr
  declareProperty("MinHits",m_minhits=2);
  debug()<<"MinHits="<<m_minhits;

}

// from m_probs and m_moms fill the m_vprobs
StatusCode GetArrival::initialize() {
  
  m_init.setCode(1);
  if (m_useFunct) {
    
    for (std::vector<double>::const_iterator it=m_beta.begin();
         it!=m_beta.end();++it) {
      if ((*it)==-1) {
        m_init.setCode(401);
        return Error( "BETAS NOT INITIALIZED", m_init);
      }
    }
    for (std::vector<double>::const_iterator it=m_alpha.begin();
         it!=m_alpha.end();++it) {
      if ((*it)==-1) {
        m_init.setCode(401);
        return Error( "ALPHAS NOT INITIALIZED", m_init);
      }
    }
  }
  
  else{  
    //check if tool has been initialized
    for (std::vector<double>::const_iterator it=m_moms.begin();
         it!=m_moms.end();++it)
    {
      if ((*it)==-1) 
      {
        debug() << "ARRIVAL TOOL NOT INITIALIZED"<<endmsg;
        m_init.setCode(401);
        return StatusCode::SUCCESS;
      }
      
    }
    
    
    //number of points per each station
    m_npoints = m_moms.size();
    debug() << " npoints: " << m_npoints << endmsg;
    if ((m_probs.size()%m_npoints)!=0){
      m_init.setCode(402);
      return Error( "INPUT VALUES WRONG SIZE PER STATION", m_init);
    }
    
    //build vector for each station. For station 0, initialization to 0
    for(int i=0;i<5;i++) {
      
      m_vprobs.push_back(std::vector<double>());
      
      if (i==0) {
        std::vector<double> partial_st0(m_npoints);
        m_functprobs.push_back(Uniformer(m_moms,partial_st0));
        continue;
      }
      
      debug()<<"ST="<<i<<endmsg;
      
      for (int j=0;j<m_npoints;j++){
        //int g_ind=i*m_npoints+j;
        int g_ind=(i-1)*m_npoints+j;
        m_vprobs[i].push_back(m_probs[g_ind]);
      }
      
      //from vector build uniformer to interpolate between different moms.
      debug() << " m_vprobs "<< i << m_vprobs[i]<<endmsg;
      m_functprobs.push_back(Uniformer(m_moms,m_vprobs[i]));
      
    }
  }
  
  std::vector<std::vector<int> > types_st;

  types_st.push_back(boost::assign::list_of (1)(1)(1)(1));
  types_st.push_back(boost::assign::list_of (1)(1)(1)(0));
  types_st.push_back(boost::assign::list_of (1)(1)(0)(1));
  types_st.push_back(boost::assign::list_of (1)(0)(1)(1));
  types_st.push_back(boost::assign::list_of (0)(1)(1)(1));
  types_st.push_back(boost::assign::list_of (1)(1)(0)(0));
  types_st.push_back(boost::assign::list_of (0)(0)(1)(1));
  types_st.push_back(boost::assign::list_of (0)(1)(1)(0));
  types_st.push_back(boost::assign::list_of (1)(0)(0)(1));
  types_st.push_back(boost::assign::list_of (1)(0)(1)(0));
  types_st.push_back(boost::assign::list_of (0)(1)(0)(1));
  types_st.push_back(boost::assign::list_of (1)(0)(0)(0));
  types_st.push_back(boost::assign::list_of (0)(1)(0)(0));
  types_st.push_back(boost::assign::list_of (0)(0)(1)(0));
  types_st.push_back(boost::assign::list_of (0)(0)(0)(1));
  types_st.push_back(boost::assign::list_of (0)(0)(0)(0));

  
  for (std::vector<std::vector<int> >::const_iterator it=types_st.begin();
       it!=types_st.end();++it) {                        
    std::vector<int> type_st=*it;         
    if (countArray(type_st,4,1)>=m_minhits) m_types_st.push_back(type_st);  
  }

  debug()<<"m_types_st="<<m_types_st<<endmsg;
  
  return StatusCode::SUCCESS;
  
}
 
//find prob for one single station according to mom
double GetArrival::findProbStation(const int st, const double mom) {       
  double val,val1;
  val1=pow(m_alpha[st]*(mom-m_beta[st]), st);
  //calculate function value for station st
  if (m_useFunct) 
  { 
    if (mom>m_beta[st]) val=val1/(1+val1);
      else val=.01;
  }
  
  //calculate value from histogram
  else val=m_functprobs[st].getYvalue(mom);
  return 0.99*val; 
  }


//find arrival probs for all 4 stations
double GetArrival::findProbAllStations(std::vector<int> sts, const double mom)
{
  double prob = 1.;
  std::vector<double> all_probs;
  sort(sts.begin(),sts.end());

  //find prob per each station
  for (int st=1;st<5;st++) {
    double prob_st=findProbStation(st,mom);
    if (!(binary_search(sts.begin(),sts.end(),st))) prob_st=1-prob_st;    

    debug()<<"ST="<<st<<",prob="<<prob_st<<endmsg;
    all_probs.push_back(prob_st);
  }

  //sort all probs according to value
  sort(all_probs.begin(),all_probs.end());
  debug()<<"all_probs sorted="<<all_probs<<endmsg;

  //if removeSmallest, remove smallest
  if (m_removeSmallest) all_probs.erase(all_probs.begin());
  
  debug()<<"after removal (if removeSmallest)"<<all_probs<<endmsg;

  //calculate prod for all stored values
  for (std::vector<double>::const_iterator it=all_probs.begin();
      it!=all_probs.end();++it)
      {                        
         double p=*it;         
         prob*=p; 
      }
  
  debug()<<"all_prob="<<prob<<endmsg;
  if (prob==0) return 1e-6; 

  return prob;
}


bool GetArrival::stInStations(const int myst,const std::vector<int>& stations)
{
  
  for (std::vector<int>::const_iterator it = stations.begin();
         it != stations.end(); ++it) 
    {
      int ist = *it;
      if (ist==myst) return true;
    }
  return false;
}


//get array with stations from track's lhcbIDs
StatusCode GetArrival::getStationsFromTrack(const LHCb::Track& mutrack, std::vector<int>& sts, const bool opt)
{
  std::vector<int> sts_init;
  std::vector< LHCb::LHCbID > lhcbids = mutrack.lhcbIDs();
  for (std::vector<LHCb::LHCbID>::const_iterator it=lhcbids.begin();
       it!=lhcbids.end();++it)
  {                        
    if (!it->isMuon()) continue;
    int st=(*it).muonID().station();
    debug()<<"added station "<<st<<endmsg;
    if (!stInStations(st,sts_init)) sts_init.push_back(st);
  }
  
  if (opt){
    for (int ist=1;ist<5;ist++){
      if (stInStations(ist,sts_init)) sts.push_back(1);
      else sts.push_back(0);
    }
  }
  else sts=sts_init;
  
  if (sts.size()<1) return StatusCode::FAILURE;
  return StatusCode::SUCCESS;
}


//get array prob directly from track
StatusCode GetArrival::getArrivalFromTrack(const LHCb::Track& mutrack,double& parr)
{
  if (m_init.isFailure()){
    parr=0.;
    return m_init;  
  }

  StatusCode sc;
  if (mutrack.lhcbIDs().size()<1) 
  {
    debug()<<"NO LHCbIDs ON TRACK. IMPOSSIBLE TO CALCULATE QUALITY"<<endmsg;
    parr=0.;
    sc.setCode(410);
    return sc;
  }

  
  std::vector<int> type_st;
  sc = getStationsFromTrack(mutrack,type_st);
  if (sc.isFailure())
  {
    debug()<<"COULD NOT RETRIEVE STS FROM LHCbIDs"<<endmsg;
    sc.setCode(411);
    return sc;
  }



  debug()<<"sts="<<type_st<<endmsg;
  parr= probTypeSt(mutrack.p(),type_st);
  debug()<<"prob="<<endmsg;
  return StatusCode::SUCCESS;
}




double GetArrival::probTypeSt(const double p, const std::vector<int>& type_st) {
  
  // returns the probability of getting a given type_st [1,1,1,1] (all stations for example)

  double p4 = probTypeStStation(p,type_st,4);
  double p3 = 0.;
  double p2 = 0.;
  double p1 = 0.;
  double p0 = 0.;
  
  if (!type_st[3]){
    p3 = probTypeStStation(p,type_st,3);
    if (!type_st[2]){
      p2 = probTypeStStation(p,type_st,2);
      if (!type_st[1]){
        p1 = probTypeStStation(p,type_st,1);
        if (!type_st[0]) p0 = probTypeStStation(p,type_st,0);
      }
    }  
  }
  
  double pp = p4 + p3 + p2 + p1 + p0;
  
  debug()<< "@probTypeSt: "<<type_st<<","<<p4<<","<<p3<<","<<p2<<","<<p1<<","<<p0<<","<<pp<<endmsg;
  return pp;
}



int GetArrival::countArray(const std::vector<int>& type_st, const int up_to, const int el){
  int out=0;
  for (int i=0;i<up_to;i++){
    if (type_st[i]==el) out+=1;
  }
  return out;
}



double GetArrival::probTypeStStation(const double p, const std::vector<int>& type_st,const int station){
      
  //returns the probability of a given type_st, assuming that a track reach that station (3 or 4)
  double P4 = findProbStation(4, p);
  double P3 = findProbStation(3, p);
  double P2 = findProbStation(2, p);
  double P1 = findProbStation(1, p);

  double PP = (1-P4)*(1-P3)*(1-P2)*(1-P1);
  if (station == 4) PP = P4;
  else if (station == 3) PP = (1-P4)*P3;
  else if (station == 2) PP = (1-P4)*(1-P3)*P2;
  else if (station == 1) PP = (1-P4)*(1-P3)*(1-P2)*P1;

  int m = 0;
  if (station == 4) m = 4;
  else if (station == 3) m = 3;
  else if (station == 2) m = 2;
  else if (station == 1) m = 1;
  

  int nholes = 0;
  if (station == 4) nholes = countArray(type_st,4,0);
  else if (station == 3) nholes = countArray(type_st,3,0);
  else if (station == 2) nholes = countArray(type_st,2,0);
  else if (station == 1) nholes = countArray(type_st,1,0);

  double val = PP*pow(m_eff,m-nholes)*pow((1-m_eff),nholes);
  
  debug()
    <<"@probTypeStStation: "<<p<<","<<type_st<<","<<station
    <<" \np4, p3, p2, p1, pp ="<<P4<<","<<P3<<","<<P2<<","<<P1<<","<<PP
    <<" \neff, (1-eff) "<<","<<pow(m_eff,m-nholes)<<","<<pow((1-m_eff),nholes)
    <<" \nval "<<val<<endmsg;
  return val;
  
}



StatusCode GetArrival::clArrival(const LHCb::Track& muTrack, double& clarr){
  
  if (m_init.isFailure()) 
  {
    clarr=0.;
    return m_init;
  }
  
  StatusCode sc;
  if (muTrack.lhcbIDs().size()<1) 
  {
    debug()<<"NO LHCbIDs ON TRACK. IMPOSSIBLE TO CALCULATE CL"<<endmsg;
    clarr=0.;
    sc.setCode(410);
    return sc;
  }
  
  
  std::vector<int> type_st;
  sc = getStationsFromTrack(muTrack,type_st);
  if (sc.isFailure())
  {
    clarr=0.;
    debug()<<"COULD NOT RETRIEVE STS FROM LHCbIDs"<<endmsg;
    sc.setCode(411);
    return sc;
  }
  
  double p = muTrack.p();
  return clArrival(p, type_st, clarr);
}




StatusCode GetArrival::clArrival(const double p,const std::vector<int>& type_st, double& clarr)
{
  
  StatusCode sc;
  sc.setCode(412);
  
  if (countArray(type_st,4,1)<m_minhits) {
    debug()<<"number of hits less than min"<<endmsg;
    return sc;
  }
  
  //return CL of a muon having as this type_st or a worse type_st of this
  std::vector<double> vals;
  for (std::vector<std::vector<int> >::const_iterator it=m_types_st.begin();
       it!=m_types_st.end();++it){
    vals.push_back(probTypeSt(p,*it));
  }
  
  double  tot= 0.;
  for (std::vector<double>::const_iterator it=vals.begin();
       it!=vals.end();++it) tot += *it;
  
  double  sval = probTypeSt(p,type_st);
  double  stot = 0.;
  for (std::vector<double>::const_iterator it=vals.begin();
       it!=vals.end();++it) {
    if (fabs(*it)<sval+1.e-5) stot += *it;
  }
  if (tot > ((double) 0.)) clarr = stot/tot;
  else {
    debug()<<"tot=0, division by 0"<<endmsg;
    return sc;
  }
  
  for (unsigned int i=0; i<m_types_st.size();i++){
    debug() << " probTypeSt " << m_types_st[i]<<","<<vals[i]<<endmsg;
  }
  
  debug()
    << " currenttype_st "<<type_st<<","<<sval
    << " tot,stot,cls "<<tot<<","<<stot<<","<<clarr<<endmsg;
  
  return StatusCode::SUCCESS;
}
