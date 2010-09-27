#include "TaggerKaonSameTool.h"

TaggerKaonSameTool::TaggerKaonSameTool() {

  declareProperty( "KaonSame_Pt_cut", m_Pt_cut_kaonS = 0.45 *GeV );
  declareProperty( "KaonSame_P_cut",  m_P_cut_kaonS  = 4.0 *GeV );
  declareProperty( "KaonSame_IP_cut", m_IP_cut_kaonS = 3.0 );
  declareProperty( "KaonSame_Phi_cut",m_phicut_kaonS = 1.1 );
  declareProperty( "KaonSame_Eta_cut",m_etacut_kaonS = 1.0 );
  declareProperty( "KaonSame_dQ_cut", m_dQcut_kaonS  = 1.6 *GeV);
  declareProperty( "KaonS_LCS_cut",   m_lcs_cut      = 5.0 );

  declareProperty( "KaonSPID_kS_cut", m_KaonSPID_kS_cut   =  1.0 );
  declareProperty( "KaonSPID_kpS_cut",m_KaonSPID_kpS_cut  = -1.0 );

  declareProperty( "ProbMin_kaonS",   m_ProbMin_kaonS  = 0. ); //no cut

  NNetTool_MLP nnet;
  tkaonS = new Tagger();
  hcut_kS_ippu= new TH1F("hcut_kS_ippu","hcut_kS_ippu",100, 0, 20);
  hcut_kS_N   = new TH1F("hcut_kS_N",   "hcut_kS_N",   100, 0, 100);
  hcut_kS_pidk= new TH1F("hcut_kS_pidk","hcut_kS_pidk",100, -10, 90);
  hcut_kS_pidp= new TH1F("hcut_kS_pidp","hcut_kS_pidp",100, -10, 90);
  hcut_kS_pt  = new TH1F("hcut_kS_pt",  "hcut_kS_pt",  100, 0, 5);
  hcut_kS_p   = new TH1F("hcut_kS_p",   "hcut_kS_p",   100, 0, 100);
  hcut_kS_lcs = new TH1F("hcut_kS_lcs", "hcut_kS_lcs", 100, 0, 5);
  hcut_kS_IPs = new TH1F("hcut_kS_IPs", "hcut_kS_IPs", 100, 0, 20);
  hcut_kS_dphi= new TH1F("hcut_kS_dphi","hcut_kS_dphi",100, 0, 3);
  hcut_kS_deta= new TH1F("hcut_kS_deta","hcut_kS_deta",100, 0, 3);
  hcut_kS_dq  = new TH1F("hcut_kS_dq",  "hcut_kS_dq",  100, 0, 3);

}

///////////////////////////////////////////////
Tagger* TaggerKaonSameTool::tag(Event& event) {
  tkaonS->reset();
  
  TLorentzVector ptotB = event.signalB()->momentum();

  //select kaonS sameside tagger(s)
  //if more than one satisfies cuts, take the highest Pt one
  Particle* ikaonS=NULL;
  double ptmaxkS = -99.0, ncand=0;
  double save_dphi=0, save_dQ=0, save_IPsig=0;

  Particles::iterator ipart;
  Particles vtags = event.particles();
  for (ipart=vtags.begin(); ipart!=vtags.end(); ++ipart) {

    if(!checkPIDhypo(Particle::kaon_same, *ipart)) continue;
   
    double Pt = (*ipart)->pt();
    if( Pt < m_Pt_cut_kaonS )  continue;

    double P  = (*ipart)->p();
    if( P < m_P_cut_kaonS )  continue;

    if( (*ipart)->LCS() > m_lcs_cut ) continue;

    //calculate signed IP wrt RecVert
    double IPsig = (*ipart)->IPs();
    if(fabs(IPsig) > m_IP_cut_kaonS) continue;

    double deta  = fabs(log(tan(ptotB.Theta()/2.)/tan(asin(Pt/P)/2.)));
    if(deta > m_etacut_kaonS) continue;

    double dphi  = fabs((*ipart)->momentum().Phi() - ptotB.Phi()); 
    if(dphi>3.1416) dphi=6.2832-dphi;
    if(dphi > m_phicut_kaonS) continue;

    TLorentzVector pm  = (*ipart)->momentum();
    double E = sqrt(pm.P() * pm.P()+ 493.677*MeV * 493.677*MeV);
    TLorentzVector pmK ( pm.Px(),pm.Py(),pm.Pz(), E);
    double dQ= (ptotB+pmK).M() - ptotB.M();

    if(dQ > m_dQcut_kaonS ) continue;
 
    ncand++;
 
    hcut_kS_ippu->Fill((*ipart)->IPPU());
    hcut_kS_N   ->Fill(event.multiplicity());
    hcut_kS_pidk->Fill((*ipart)->PIDk());
    hcut_kS_pidp->Fill((*ipart)->PIDp());
    hcut_kS_pt  ->Fill(Pt);
    hcut_kS_p   ->Fill(P);
    hcut_kS_lcs ->Fill((*ipart)->LCS());
    hcut_kS_IPs ->Fill(IPsig);
    hcut_kS_dphi->Fill(dphi);
    hcut_kS_deta->Fill(deta);
    hcut_kS_dq  ->Fill(dQ);

   if( Pt > ptmaxkS ) { 
      ikaonS  = (*ipart);
      ptmaxkS = Pt;
      save_dphi=dphi;
      save_dQ=dQ;
      save_IPsig=IPsig;
    }

  } 
  if( !ncand ) return tkaonS;

  double ang = asin(ikaonS->pt()/ikaonS->p());
  double deta= log(tan(ptotB.Theta()/2))-log(tan(ang/2));

  std::vector<double> NNinputs(10);
  NNinputs.at(0) = vtags.size();
  NNinputs.at(1) = event.signalB()->pt();
  NNinputs.at(2) = ikaonS->p()/GeV;
  NNinputs.at(3) = ikaonS->pt()/GeV;
  NNinputs.at(4) = save_IPsig;
  NNinputs.at(5) = deta;
  NNinputs.at(6) = save_dphi;
  NNinputs.at(7) = save_dQ/GeV;
  NNinputs.at(8) = event.pileup();
  NNinputs.at(9) = ncand;

  double pn = nnet.MLPkS( NNinputs );

  if( pn < m_ProbMin_kaonS ) return tkaonS;

  ikaonS->setID(321*ikaonS->charge());

  tkaonS->setOmega( 1-pn );
  tkaonS->setRawNNetOutput( nnet.m_rnet_kS );
  tkaonS->setDecision(ikaonS->charge()>0 ? 1: -1);
  tkaonS->setType( Tagger::SS_Kaon ); 
  tkaonS->addToTaggerParts(ikaonS);

  return tkaonS;
}
