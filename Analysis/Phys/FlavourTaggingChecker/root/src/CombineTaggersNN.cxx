#include "CombineTaggersNN.h"
#include "taggingutils.h"
#include "globals.h"

CombineTaggersNN::CombineTaggersNN() {

  declareProperty( "OmegaMaxBin", m_omegamaxbin = 0.375 );
  declareProperty( "OmegaScale",  m_omegascale  = 0.06 );
  declareProperty( "ProbMin_NN",  m_ProbMin     = 0.53 );

  declareProperty( "P0_NN",   m_P0_NN   = 1.01714 );
  declareProperty( "P1_NN",   m_P1_NN   = -1.01022 );
  declareProperty( "P2_NN",   m_P2_NN   = 0 );

  theTag = new FlavourTag();

}
CombineTaggersNN::~CombineTaggersNN() { 
  delete theTag;
}

/////////////////////////////////////////////////////////////
FlavourTag* CombineTaggersNN::combineTaggers( Taggers& vtg ) {
  theTag->reset();

  theTag->setTaggers(vtg);

  if( vtg.empty() ) return theTag;

  double tagdecision=0;
  int vtgsize = vtg.size();
  int abstagsum = 0;
  pmu=pe=pk=pss=pvtx=0.5; //NN variables (.h)
  int mutag, etag, ktag, sstag, vtxtag;
  mutag=etag=ktag=sstag=vtxtag=0;
  double pnsum=0.5;
  double probNN=0;

  //obtain pn(omega) and tagdec for each tagger
  for( int i = 0; i != vtgsize; i++ ) { //multiply all probabilities
    if (!vtg.at(i)->decision()) continue;
    int type = vtg.at(i)->type();
    if (type==2) {
      pmu = 1-(vtg.at(i))->omega(); //probability of 'right' mu
      mutag = vtg.at(i)->decision(); 
      debug()<<"muon -> pmu: "<<pmu<<", mutag:"<<mutag<<endreq;
    }
    else if (type==3) {
      pe = 1-(vtg.at(i))->omega(); //probability of 'right' e
      etag = vtg.at(i)->decision(); 
      debug()<<"ele -> pe: "<<pe<<", etag:"<<etag<<endreq;
    }
    else if (type==4) {
      pk = 1-(vtg.at(i))->omega(); //probability of 'right' k
      ktag = vtg.at(i)->decision(); 
      debug()<<"kaon -> pk: "<<pk<<", ktag:"<<ktag<<endreq;
    }
    else if (type==5) {
      pss = 1-(vtg.at(i))->omega(); //probability of 'right' kaonSS
      sstag = vtg.at(i)->decision(); 
      debug()<<"kaonSS -> pss: "<<pss<<", sstag:"<<sstag<<endreq;
    }
    else if (type==6){
      pss = 1-(vtg.at(i))->omega(); //probability of 'right' pionSS
      sstag = vtg.at(i)->decision(); 
      debug()<<"pionSS -> pss: "<<pss<<", sstag:"<<sstag<<endreq;
    }
    else if (type==10){
      pvtx = 1-(vtg.at(i))->omega(); //probability of 'right' vtx
      vtxtag = vtg.at(i)->decision(); 
      debug()<<"pvtx: "<<pvtx<<", vtxtag:"<<vtxtag<<endreq;
    }
    abstagsum = abs(mutag)+abs(etag)+abs(ktag)+abs(sstag)+abs(vtxtag);
  }

  //only one tagger
  if (abstagsum==1){
    pnsum=pmu+pe+pk+pss+pvtx-2;//sum all taggers minus 2(4 taggers inactive)
    tagdecision=mutag+etag+ktag+sstag+vtxtag;
  }

  //more than one tagger
  if (abstagsum>1){
    //ch p[i] for tagdec=-1
    if (mutag<0) pmu=1-pmu;
    if (etag<0)  pe =1-pe;
    if (ktag<0)  pk =1-pk;
    if (sstag<0) pss=1-pss;
    if (vtxtag<0)pvtx=1-pvtx;

    NNcomb net;
    probNN = net.Value(0, pmu,pe,pk,pss,pvtx);
    double probPlus = m_P0_NN + m_P1_NN*(probNN) + m_P2_NN*(probNN)*(probNN);//tuning NN

    if (probPlus>1) probPlus=1;
    if (probPlus<0) probPlus=0;
    if (probNN>0.) {tagdecision = +1, pnsum=probPlus;}
    if (probNN<0.) {tagdecision = -1, pnsum=1-probPlus;}
    //swap negative to possitive
    if(pnsum < 0.5) {
      pnsum = 1-pnsum;
      tagdecision = -1*tagdecision;
    }
    debug() << "probNN: " << probNN <<", probPlus: " << probPlus 
            <<", pnsum: "<<pnsum<<", tagdecision: "<<tagdecision<<endreq;
  }
  //throw away poorly significant tags
  if(pnsum < m_ProbMin) {
    pnsum = 0.50;
    tagdecision = 0;
  }
  
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

  //info
  debug() << "Final 1-w = " << pnsum <<endreq;
  debug() << "TagDecision = " << tagdecision <<endreq;
  debug() << "Category = " << category <<endreq;
  debug() << "Omega = " << omega <<endreq;

  ///fill FlavourTag object
  if(      tagdecision ==  1 ) theTag->setDecision( FlavourTag::bbar );
  else if( tagdecision == -1 ) theTag->setDecision( FlavourTag::b );
  else theTag->setDecision( FlavourTag::none );
  theTag->setCategory( category );
  theTag->setOmega( 1-pnsum );

  //fill in taggers info into FlavourTag object
  for(int j=0; j!=vtgsize; ++j) {
    int itag = vtg.at(j)->decision();
    if(itag) theTag->addTagger(vtg.at(j));
  }

  return theTag;
}
