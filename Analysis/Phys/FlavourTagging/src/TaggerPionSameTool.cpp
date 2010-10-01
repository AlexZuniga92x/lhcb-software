// Include files 
#include "TaggerPionSameTool.h"

//--------------------------------------------------------------------
// Implementation file for class : TaggerPionSameTool
//
// Author: Marco Musy
//--------------------------------------------------------------------

using namespace LHCb ;
using namespace Gaudi::Units;

// Declaration of the Algorithm Factory
DECLARE_TOOL_FACTORY( TaggerPionSameTool );

//====================================================================
TaggerPionSameTool::TaggerPionSameTool( const std::string& type,
                                        const std::string& name,
                                        const IInterface* parent ) :
  GaudiTool ( type, name, parent ) {
  declareInterface<ITagger>(this);

  declareProperty( "CombTech", m_CombinationTechnique = "NNet" );
  declareProperty( "NeuralNetName",  m_NeuralNetName  = "NNetTool_MLP" );
  declareProperty( "AverageOmega",   m_AverageOmega   = 0.40 );

  declareProperty( "PionProbMin",     m_PionProbMin   = 0.53);
  declareProperty( "PionSame_Pt_cut", m_Pt_cut_pionS  = 0.75 *GeV );
  declareProperty( "PionSame_P_cut",  m_P_cut_pionS   = 0.0 *GeV );
  declareProperty( "PionSame_IPs_cut",m_IPs_cut_pionS = 3.5 );
  declareProperty( "PionS_LCS_cut",   m_lcs_cut       = 5.0 );
  declareProperty( "PionSame_dQ_cut", m_dQcut_pionS   = 2.5 *GeV);
  declareProperty( "PionSame_dQ_extra_cut", m_dQcut_extra_pionS = 1.5 *GeV);
  declareProperty( "Pion_ghost_cut",  m_ghost_cut     = -999.0);
  declareProperty( "PionSame_PIDNoK_cut", m_PionSame_PIDNoK_cut = 3.0);
  declareProperty( "PionSame_PIDNoP_cut", m_PionSame_PIDNoP_cut = 10.0);

  m_nnet = 0;
  m_util = 0;

}
TaggerPionSameTool::~TaggerPionSameTool() {}; 

//=====================================================================
StatusCode TaggerPionSameTool::initialize() { 

  m_util = tool<ITaggingUtils> ( "TaggingUtils", this );
  if( ! m_util ) {
    fatal() << "Unable to retrieve TaggingUtils tool "<< endreq;
    return StatusCode::FAILURE;
  }
  m_nnet = tool<INNetTool> ( m_NeuralNetName, this);
  if(! m_nnet) {
    fatal() << "Unable to retrieve NNetTool"<< endreq;
    return StatusCode::FAILURE;
  }
  return StatusCode::SUCCESS; 
}

//=====================================================================
Tagger TaggerPionSameTool::tag( const Particle* AXB0, const RecVertex* RecVert,
                                std::vector<const Vertex*>& allVtx, 
                                Particle::ConstVector& vtags ){
  Tagger tpionS;
  if(!RecVert) return tpionS;

  verbose()<<"--Pion SS Tagger--"<<endreq;

  Gaudi::LorentzVector ptotB = AXB0->momentum();
  double B0mass = ptotB.M();

  //select pionS sameside tagger(s)
  //if more than one satisfies cuts, take the highest Pt one
  const Particle* ipionS=0;
  double ptmaxpS = -99.0, ncand=0;
  Particle::ConstVector::const_iterator ipart, jpart;
  for( ipart = vtags.begin(); ipart != vtags.end(); ipart++ ) {

    //PID cuts to select the pion
    const ProtoParticle* proto = (*ipart)->proto();
    double PIDk= proto->info( ProtoParticle::CombDLLk,  -1000.0 );
    double PIDp= proto->info( ProtoParticle::CombDLLp,  -1000.0 );
    bool pidpass=false;
    if( PIDk==0 ) pidpass=true;
    if( PIDk!=0 ) if(PIDk < m_PionSame_PIDNoK_cut &&
                     PIDp < m_PionSame_PIDNoP_cut) pidpass=true;
    if( (*ipart)->particleID().abspid() != 211 ) continue;
    if(!pidpass) continue;
    verbose()<<" Pion PIDk="<< PIDk <<endreq;
    
    double Pt = (*ipart)->pt();
    if( Pt < m_Pt_cut_pionS )  continue;
    double P  = (*ipart)->p();
    if( P  < m_P_cut_pionS )  continue;
    verbose()<<" Pion P="<< P <<" Pt="<< Pt <<endreq;

    const Track* track = proto->track();
    double lcs = track->chi2PerDoF();
    if( lcs > m_lcs_cut ) continue;
    if( track->type() != Track::Long && 
        track->checkHistory(Track::TrackMatching)!=true ) continue;

    double tsa = track->likelihood();
    if(tsa < m_ghost_cut) continue;
    verbose() << " Pion lcs="<< lcs <<" tsa="<<tsa<<endmsg;

    double IP, IPerr;
    m_util->calcIP(*ipart, RecVert, IP, IPerr);
    if(!IPerr) continue;
    double IPsig = fabs(IP/IPerr);
    if(IPsig > m_IPs_cut_pionS)  continue;

    double dQ = (ptotB+(*ipart)->momentum()).M() - B0mass;
    verbose() << " Pion IPs="<< IPsig <<" dQ="<<dQ<<endmsg;
    if(dQ > m_dQcut_pionS ) continue;

    ncand++;

    if( Pt <= ptmaxpS ) continue;//equal sign in "<=" is used to kill duplicates

    //accept candidate
    ipionS = (*ipart);
    ptmaxpS = Pt;

  } 
  if( !ipionS ) return tpionS;

  double extra_dQ = (ptotB+ipionS->momentum()).M() - B0mass;
  if( extra_dQ > m_dQcut_extra_pionS ) return tpionS;
  verbose() << " Pion dQExtra="<< extra_dQ <<endmsg;
 
  //calculate omega
  double pn = 1-m_AverageOmega;
  if(m_CombinationTechnique == "NNet") {

    double IP, IPerr;
    double B0the= ptotB.Theta();
    double B0phi= ptotB.Phi();
    double ang = asin((ipionS->pt())/(ipionS->p()));
    double deta= log(tan(B0the/2.))-log(tan(ang/2.));
    double dphi= std::min(fabs(ipionS->momentum().Phi()-B0phi), 
                          6.283-fabs(ipionS->momentum().Phi()-B0phi));
    double dQ  = ((ptotB+ ipionS->momentum() ).M() - B0mass);
    //     debug()<<"   B0mass+pSS "<< (ptotB+ ipionS->momentum() ).M()
    // 	   <<"   B0mass "<<B0mass
    // 	   <<"        dQ"<<dQ/GeV<< endreq;

    m_util->calcIP(ipionS, RecVert, IP, IPerr);

    std::vector<double> NNinputs(10);
    NNinputs.at(0) = m_util->countTracks(vtags);
    NNinputs.at(1) = AXB0->pt()/GeV;
    NNinputs.at(2) = ipionS->p()/GeV;
    NNinputs.at(3) = ipionS->pt()/GeV;
    NNinputs.at(4) = IP/IPerr;
    NNinputs.at(5) = deta;
    NNinputs.at(6) = dphi;
    NNinputs.at(7) = dQ/GeV;
    NNinputs.at(8) = allVtx.size();
    NNinputs.at(9) = ncand;

    pn = m_nnet->MLPpS( NNinputs );
    verbose() << " Pion pn="<< pn <<endmsg;

    if( pn < m_PionProbMin ) return tpionS;
  }

  int tagdecision = ipionS->charge()>0 ? 1: -1;
  if( AXB0->particleID().hasUp() ) tagdecision = -tagdecision; //is Bu

  tpionS.setDecision( tagdecision );
  tpionS.setOmega( 1-pn );
  tpionS.setType( Tagger::SS_Pion ); 
  tpionS.addToTaggerParts(ipionS);

  return tpionS;
}
//==========================================================================
StatusCode TaggerPionSameTool::finalize() { return StatusCode::SUCCESS; }

