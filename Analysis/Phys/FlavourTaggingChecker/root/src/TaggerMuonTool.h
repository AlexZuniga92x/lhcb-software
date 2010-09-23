#ifndef TaggerMuonTool_H
#define TaggerMuonTool_H 1

#include <iostream>
#include <vector>
#include <TLorentzVector.h>
#include "Tagger.h"
#include "Event.h"
#include "GenericTool.h"
#include "NNetTool_MLP.h"

#include "TH1F.h"

using namespace std;

/** @class TaggerMuonTool
 *
 *  Tool to retrieve a tagger by using muons 
 *   from the opposite B meson in the event
 *
 *  @author Marco Musy
 *  @date   2010-09-07
 */

class TaggerMuonTool : public GenericTool {

 public:

  TaggerMuonTool( );
  ~TaggerMuonTool( ){ delete tmu; } ///< Destructor

  Tagger* tag( Event& );

 private:
  double m_P_cut_muon;
  double m_Pt_cut_muon;
  double m_lcs_cut_muon ;
  double m_ProbMin_muon;
 
  NNetTool_MLP nnet;
  Tagger* tmu;

  TH1F* hcut_mu_pid;
  TH1F* hcut_mu_pt;
  TH1F* hcut_mu_p;
  TH1F* hcut_mu_lcs;
  TH1F* hcut_mu_ippu;
  TH1F* hcut_mu_N ;
  
 
};

#endif 
