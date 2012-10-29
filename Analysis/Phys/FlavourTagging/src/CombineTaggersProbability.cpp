#include "CombineTaggersProbability.h"

//-----------------------------------------------------------------------------
// Implementation file for class : CombineTaggersProbability v2.
//
// 2011-01-18 : Marco Musy & Marc Grabalosa 
//-----------------------------------------------------------------------------

using namespace LHCb ;
using namespace Gaudi::Units;

// Declaration of the Algorithm Factory
DECLARE_TOOL_FACTORY( CombineTaggersProbability );

//=============================================================================
CombineTaggersProbability::CombineTaggersProbability( const std::string& type,
                                                      const std::string& name,
                                                      const IInterface* parent ):
  GaudiTool ( type, name, parent ) { 
  declareInterface<ICombineTaggersTool>(this);

  declareProperty( "OmegaMaxBin", m_omegamaxbin  = 0.38 );
  declareProperty( "OmegaScale",  m_omegascale  = 0.07 );

  declareProperty( "ProbMin",     m_ProbMin     = 0.);
  declareProperty( "ProbMin_OS",  m_ProbMin_OS  = 0.);
  // Tuning Moriond 2012
  //declareProperty( "P0_Cal_OS",   m_P0_Cal_OS   = 0.392); 
  //declareProperty( "P1_Cal_OS",   m_P1_Cal_OS   = 0.921); 
  //declareProperty( "Eta_Cal_OS",  m_Eta_Cal_OS  = 0.363);
  // Tuning 1fb-1 paper
  declareProperty( "P0_Cal_OS",   m_P0_Cal_OS   = 0.392); 
  declareProperty( "P1_Cal_OS",   m_P1_Cal_OS   = 0.953); 
  declareProperty( "Eta_Cal_OS",  m_Eta_Cal_OS  = 0.362);

}
CombineTaggersProbability::~CombineTaggersProbability(){}
StatusCode CombineTaggersProbability::initialize() { 
  warning() << "Comb OS calib ctt: P0_Cal "<<m_P0_Cal_OS<<", P1_Cal "<<m_P1_Cal_OS<<endreq;
  return StatusCode::SUCCESS; }

StatusCode CombineTaggersProbability::finalize()   { return StatusCode::SUCCESS; }

//=============================================================================
int CombineTaggersProbability::combineTaggers(FlavourTag& theTag, 
                                              std::vector<Tagger*>& vtg , int signalType){
  if( vtg.empty() ) return 0;

  double tagdecision=0;
  double pnsum_a= 0.50;  //hypothesis of truetag=+1
  double pnsum_b= 0.50;  //hypothesis of truetag=-1
  int vtgsize = vtg.size();
  for( int i = 0; i != vtgsize; i++ ) { //multiply all probabilities
    if(! vtg.at(i)) continue;
    if( vtg.at(i)->type() == (Tagger::SS_Pion) || vtg.at(i)->type() == (Tagger::SS_Kaon) ) continue;  // Just combine the prob of OS
    double mtag = vtg.at(i)->decision();
    if(!mtag) continue;
    double pn   = 1-(vtg.at(i))->omega(); //probability of 'right'
    pnsum_a *= ((1-mtag)/2 + mtag*pn ); // p
    pnsum_b *= ((1+mtag)/2 - mtag*pn ); //(1-p)
  }
  if(pnsum_a > pnsum_b) tagdecision = +1;
  if(pnsum_a < pnsum_b) tagdecision = -1;
  //normalise probability to the only two possible flavours:
  double pnsum = std::max(pnsum_a,pnsum_b) /(pnsum_a + pnsum_b);

  //Calibration (w=1-pn) w' = p0 + p1(w-eta)
  debug() << " Before pn="<< pnsum <<" w="<<1-pnsum<<endreq;
  pnsum = 1 - m_P0_Cal_OS - m_P1_Cal_OS * ( (1-pnsum)-m_Eta_Cal_OS);
  debug() << " OS pn="<< pnsum <<" w="<<1-pnsum<<endreq;

  //throw away poorly significant tags
  if(pnsum < m_ProbMin_OS) {
    pnsum = 0.50;
    tagdecision = 0;
  }
  verbose() << "Final OS 1-w = " << pnsum <<endreq;
    
  //sort decision into categories ------------------
  //cat=1 will be least reliable, cat=5 most reliable
  //ProbMin is a small offset to adjust for range of pnsum
  int category = 0;
  double omega = fabs(1-pnsum); 
  if(      omega > m_omegamaxbin                ) category=1;
  else if( omega > m_omegamaxbin-m_omegascale   ) category=2;
  else if( omega > m_omegamaxbin-m_omegascale*2 ) category=3;
  else if( omega > m_omegamaxbin-m_omegascale*3 ) category=4;
  else                                            category=5;
  if( !tagdecision ) category=0;

  ///fill FlavourTag object
  if(      tagdecision ==  1 ) theTag.setDecisionOS( FlavourTag::bbar );
  else if( tagdecision == -1 ) theTag.setDecisionOS( FlavourTag::b );
  else theTag.setDecisionOS( FlavourTag::none );
  theTag.setCategoryOS( category );
  theTag.setOmegaOS( 1-pnsum );

  //###################################
  //Now fill SS+OS combination
  double SSeOS_tagdecision=tagdecision;
  double SSeOS_pnsum_a= ((1-tagdecision)/2 + tagdecision*pnsum ); //hypothesis of truetag=+1
  double SSeOS_pnsum_b= ((1+tagdecision)/2 - tagdecision*pnsum );;  //hypothesis of truetag=-1
  for( int i = 0; i != vtgsize; i++ ) { //multiply all probabilities
    if(! vtg.at(i)) continue;
    if( (signalType == 1 && vtg.at(i)->type() == (Tagger::SS_Pion)) || 
        (signalType == 2 && vtg.at(i)->type() == (Tagger::SS_Kaon)) ) {
      double mtagss = vtg.at(i)->decision();
      if(!mtagss) continue;
      double pnss   = 1-(vtg.at(i))->omega(); //probability of 'right'
      SSeOS_pnsum_a *= ((1-mtagss)/2 + mtagss*pnss ); // p
      SSeOS_pnsum_b *= ((1+mtagss)/2 - mtagss*pnss ); //(1-p)
    }
  }    
  if(SSeOS_pnsum_a > SSeOS_pnsum_b) SSeOS_tagdecision = +1;
  if(SSeOS_pnsum_a < SSeOS_pnsum_b) SSeOS_tagdecision = -1;
  //normalise probability to the only two possible flavours:
  double SSeOS_pnsum = std::max(SSeOS_pnsum_a,SSeOS_pnsum_b) /(SSeOS_pnsum_a + SSeOS_pnsum_b);
    
  //throw away poorly significant tags
  if(SSeOS_pnsum < m_ProbMin) {
    SSeOS_pnsum = 0.50;
    SSeOS_tagdecision = 0;
  }
  verbose() << "Final 1-w = " << SSeOS_pnsum <<endreq;
    
  //sort decision into categories ------------------
  //cat=1 will be least reliable, cat=5 most reliable
  //ProbMin is a small offset to adjust for range of pnsum
  int SSeOS_category = 0;
  double SSeOS_omega = fabs(1-SSeOS_pnsum); 
  if(      SSeOS_omega > m_omegamaxbin                ) SSeOS_category=1;
  else if( SSeOS_omega > m_omegamaxbin-m_omegascale   ) SSeOS_category=2;
  else if( SSeOS_omega > m_omegamaxbin-m_omegascale*2 ) SSeOS_category=3;
  else if( SSeOS_omega > m_omegamaxbin-m_omegascale*3 ) SSeOS_category=4;
  else                                            SSeOS_category=5;
  if( !SSeOS_tagdecision ) SSeOS_category=0;

  ///fill FlavourTag object
  if(      SSeOS_tagdecision ==  1 ) theTag.setDecision( FlavourTag::bbar );
  else if( SSeOS_tagdecision == -1 ) theTag.setDecision( FlavourTag::b );
  else theTag.setDecision( FlavourTag::none );
  theTag.setCategory( SSeOS_category );
  theTag.setOmega( 1-SSeOS_pnsum );

  //fill in taggers info into FlavourTag object
  for(int j=0; j!=vtgsize; ++j) {
    int itag = vtg.at(j)->decision();
    if(itag) theTag.addTagger(*(vtg.at(j)));
  }

  return category;
}
//=============================================================================
