// $Id: B2DHSelectionCutsTool.cpp,v 1.1 2010/06/10 12:42:21 seaso Exp $
// Include files 

// from Gaudi
#include "GaudiKernel/ToolFactory.h" 

// local
#include "B2DHSelectionCutsTool.h"

//-----------------------------------------------------------------------------
// Implementation file for class : B2DHSelectionCutsTool
//
// 2010-02-18 : Sajan EASO
//-----------------------------------------------------------------------------

// Declaration of the Tool Factory
DECLARE_TOOL_FACTORY( B2DHSelectionCutsTool );


//=============================================================================
// Standard constructor, initializes variables
//=============================================================================
B2DHSelectionCutsTool::B2DHSelectionCutsTool( const std::string& type,
                                              const std::string& name,
                                              const IInterface* parent )
  : GaudiTool ( type, name , parent )
{
  declareInterface<IB2DHSelectionCutsTool>(this);

  setCommonSelectionCutValues() ;
  setBs2DsKCutValues();
  setBs2DsPiCutValues();
  resetSelectionFlags();
  
}
//=============================================================================
// Destructor
//=============================================================================
B2DHSelectionCutsTool::~B2DHSelectionCutsTool() {} 

//=============================================================================
StatusCode B2DHSelectionCutsTool::initialize() {

  StatusCode sc = StatusCode::SUCCESS; 

  m_B2DHSelectionParamTool = tool<IB2DHSelectionParamTool> ("B2DHSelectionParamTool");
  
  return sc;
  
}
void B2DHSelectionCutsTool::setAllCutFlags() 
{
     resetSelectionFlags();
    setCutFlags_Common();
    if(m_B2DHSelectionParamTool->get_current_B2DH_channel_Number() == 0) setCutFlags_Bs2DsK();
    if( m_B2DHSelectionParamTool->get_current_B2DH_channel_Number() == 1) setCutFlags_Bs2DsPi();
 
  
}

void B2DHSelectionCutsTool::setCommonSelectionCutValues() {
  
  m_zPVBsCutValue=0.0;
  m_massBsWindowCutValue=50.0;
  m_massDsWindowCutValue=21.0;
  m_massBsWideWindowCutValue=500.0;
  m_ipsBsTightCutValue=5.0;
  m_ipsBsCutValue=20.0;
  m_vtxBsChisqCutValue=40.0;
  m_vtxDsChisqCutValue=46.0;
  m_ptDsCutValue=0.0;
  m_ipsBachelorCutValue=1.0;
  m_ipsSecLowCutValue=0.0;
  m_ipsSecUpCutValue=200.0;
  m_totMomSecCutValue=2000.0;

  
  
}
void B2DHSelectionCutsTool::setBs2DsKCutValues() {

  //  m_FsBsCutValue_Bs2DsK=4.2; //DC06
  m_FsBsCutValue_Bs2DsK=6.0;  //MC09
  
  m_massDsWindowCutValue_Bs2DsK=18.0;  // MC09

  m_momBachelorUpperCutValue_Bs2DsK=150000.0;

  m_dllBachelorKaonCutValue_Bs2DsK=5.5;
  m_dllKaonMinusProtonBachelorKaonCutValue_Bs2DsK=3.0;  
  m_dllProtonBachelorLowerCut_Bs2DsK=-45.0;

  m_dllProtonSecCut_Bs2DsK = -50.0;
  m_dllKaonMinusProtonSecCut_Bs2DsK= -20.0;
  
  m_fisherDCutValue_Bs2DsK=0.75;
  

}
void B2DHSelectionCutsTool::setBs2DsPiCutValues(){

  m_FsBsCutValue_Bs2DsPi=4.0;
  m_ptBachelorCutValue_Bs2DsPi=1100.0;
  
  m_momDsUpperCutValue_Bs2DsPi=400000.0;
  m_FsDsCutValue_Bs2DsPi = 5.0;
  //  m_momBachelorUpperCutValue_Bs2DsPi=240000.0; //dc06
  m_momBachelorUpperCutValue_Bs2DsPi=200000.0;
  m_momBachelorLowerCutValue_Bs2DsPi=5000.0;
  
  m_dllKaonBachelorLowerCutValue_Bs2DsPi=-130.0;
  m_dllKaonBachelorUpperCutValue_Bs2DsPi=5.0;
  
  //  m_dllProtonBachelorCutValue_Bs2DsPi=0.0; //dc06
  m_dllProtonBachelorCutValue_Bs2DsPi=5.0; //mc09
  
  //  m_dllMuonBachelorLowerCutValue_Bs2DsPi=-15.0;
  m_dllMuonBachelorUpperCutValue_Bs2DsPi=5.0;
  
  //  m_dllProtonMinusMuonBachelorCutValue_Bs2DsPi=5.0; //dc06
  m_dllProtonMinusMuonBachelorCutValue_Bs2DsPi=10.0; //mc09

  m_dllProtonSecCutValue_Bs2DsPi=-50.0;

  m_dllKaonMinusProtonSecCutValue_Bs2DsPi=-20.0;
  m_dllKaonSec1LowerCutValue_Bs2DsPi=5.0;
  m_dllKaonSec1UpperCutValue_Bs2DsPi=100.0;
  m_dllKaonSec0LowerCutValue_Bs2DsPi=-4.0;
  m_dllKaonSec0UpperCutValue_Bs2DsPi=100.0;
  m_dllProtonSec1LowerCutValue_Bs2DsPi=-45.0;
  m_dllKaonSec2CutValue_Bs2DsPi=5.0;
  m_dllProtonSec2UpperCutValue_Bs2DsPi=20.0;
  m_dllProtonSec2LowerCutValue_Bs2DsPi=-100.0;

  m_massBsWindowCutValue_Bs2DsPi=45.0;

  m_vtxBsChisqCutValue_Bs2DsPi=10.0;
  m_vtxDsChisqCutValue_Bs2DsPi=15.0;
  m_ipsBachelorCutValue_Bs2DsPi=3.0;
  

  m_fisherDCutValue_Bs2DsPi =0.55;
  
}
void B2DHSelectionCutsTool::setCutFlags_Common(){
  m_zPVBsSel = ( m_B2DHSelectionParamTool->ZPvDiff()) > m_zPVBsCutValue;
  m_massBsWindowSel = fabs(m_B2DHSelectionParamTool->BsMassDiff()) < m_massBsWindowCutValue;
  m_massDsWindowSel = fabs( m_B2DHSelectionParamTool->DsMassDiff() ) < m_massDsWindowCutValue;

  m_massWindowSel = m_massBsWindowSel && m_massDsWindowSel;
  

  std::vector<double> aipsSec =m_B2DHSelectionParamTool-> ipsSecPart();
  

  m_IpsSel = (m_B2DHSelectionParamTool-> ipsBs() < m_ipsBsCutValue)  &&  
    (m_B2DHSelectionParamTool-> ipsBachelorPart() > m_ipsBachelorCutValue);
  
  m_IpsSecSel =  (aipsSec [0] > m_ipsSecLowCutValue) && (aipsSec [0] < m_ipsSecUpCutValue )  &&
    (aipsSec [1] > m_ipsSecLowCutValue) && (aipsSec [1] < m_ipsSecUpCutValue )  &&
    (aipsSec [2] > m_ipsSecLowCutValue) && (aipsSec [2] < m_ipsSecUpCutValue )  ;
    m_vtxChisqSel = (m_B2DHSelectionParamTool-> bvtxchisq() < m_vtxBsChisqCutValue) && 
                  (m_B2DHSelectionParamTool-> dvtxchisq() < m_vtxDsChisqCutValue);

  std::vector<double> aMomSec = m_B2DHSelectionParamTool-> secMomValue();
  
  m_ptDs_momSec_Sel  = (m_B2DHSelectionParamTool-> ptDs() > m_ptDsCutValue) && 
                      (aMomSec[0] > m_totMomSecCutValue) &&
                      (aMomSec[1] > m_totMomSecCutValue) && 
                      (aMomSec[2] > m_totMomSecCutValue);  

  m_Train_Sel =   m_zPVBsSel &&  m_IpsSel && m_vtxChisqSel && m_ptDs_momSec_Sel &&  m_IpsSecSel;  


  // info()<<" Cut settings ZPvSel ="<<  m_zPVBsSel <<"  massBsWindowSel=   "<<m_massBsWindowSel<<" massDsWindowSel= "
  //       << m_massDsWindowSel << "VertexChisqSel "<<  m_vtxChisqSel<< "  IpsSel =  "<<  m_IpsSel 
  //      << "  IpsSecSel  = "<< m_IpsSecSel
  //      <<"  ptDs_momSec_Sel= "<<m_ptDs_momSec_Sel
  //      <<   "Train_Sel= " <<  m_Train_Sel<<endmsg;
  
}
void B2DHSelectionCutsTool::setCutFlags_Bs2DsK(){

   m_auxCut_Bs_Bs2DsK = 
        (m_B2DHSelectionParamTool->  FsBs()  >  m_FsBsCutValue_Bs2DsK)  &&
        (m_B2DHSelectionParamTool-> momBachelorPart() < m_momBachelorUpperCutValue_Bs2DsK) &&
     (m_B2DHSelectionParamTool-> ipsBs() < m_ipsBsTightCutValue) ;
  
  double adllKaonMinusProtonBach = m_B2DHSelectionParamTool-> BachelorKaDll() - 
                                   m_B2DHSelectionParamTool-> BachelorPrDll();
  std::vector<double> aSecKaonDll = m_B2DHSelectionParamTool-> bKaonSecDll();
  std::vector<double> aSecProtonDll =  m_B2DHSelectionParamTool->  bProtonSecDll();
  
  m_auxCut_Dll_Bs2DsK =  
    (m_B2DHSelectionParamTool-> BachelorKaDll() > m_dllBachelorKaonCutValue_Bs2DsK) &&
     (adllKaonMinusProtonBach >  m_dllKaonMinusProtonBachelorKaonCutValue_Bs2DsK) &&
    (( aSecKaonDll[0]- aSecProtonDll[0]) >   m_dllKaonMinusProtonSecCut_Bs2DsK ) &&
    (( aSecKaonDll[1]- aSecProtonDll[1]) >   m_dllKaonMinusProtonSecCut_Bs2DsK ) &&
    ( m_B2DHSelectionParamTool -> BachelorPrDll() > m_dllProtonBachelorLowerCut_Bs2DsK ) &&
     (aSecProtonDll[0] > m_dllProtonSecCut_Bs2DsK) &&
    (aSecProtonDll[1] > m_dllProtonSecCut_Bs2DsK );
  

   m_massBsWindowSel_Bs2DsK=fabs(m_B2DHSelectionParamTool->BsMassDiff()) < m_massBsWindowCutValue;
   m_massDsWindowSel_Bs2DsK = fabs( m_B2DHSelectionParamTool->DsMassDiff() ) < m_massDsWindowCutValue_Bs2DsK;
   m_massWindowSel_Bs2DsK= m_massBsWindowSel_Bs2DsK && m_massDsWindowSel_Bs2DsK;
 

  m_fisher_Sel_Bs2DsK =  m_B2DHSelectionParamTool-> getfisherDProbValue() > m_fisherDCutValue_Bs2DsK;

  //  info()<<" Aux settings  auxCutBs2DsK= "<<m_auxCut_Bs_Bs2DsK <<"  m_auxCut_Dll_Bs2DsK=   " 
  //      <<m_auxCut_Dll_Bs2DsK  << " fisher Cut =  "<< m_fisher_Sel_Bs2DsK <<endmsg;
  

  
}
void B2DHSelectionCutsTool::setCutFlags_Bs2DsPi(){

  m_auxCut_Bs_Bs2DsPi =  ( m_B2DHSelectionParamTool-> FsBs()  >  m_FsBsCutValue_Bs2DsPi)  && 
                         (m_B2DHSelectionParamTool-> ipsBs() < m_ipsBsTightCutValue) &&
    (m_B2DHSelectionParamTool-> momBachelorPart() <  m_momBachelorUpperCutValue_Bs2DsPi) &&
    (m_B2DHSelectionParamTool-> momBachelorPart() >  m_momBachelorLowerCutValue_Bs2DsPi ) && 
    ( m_B2DHSelectionParamTool-> ptBachelorPart() >   m_ptBachelorCutValue_Bs2DsPi ) && 
    ( m_B2DHSelectionParamTool-> momDs()  < m_momDsUpperCutValue_Bs2DsPi ) &&
    ( m_B2DHSelectionParamTool->  FsDs()  > m_FsDsCutValue_Bs2DsPi ) &&
      (m_B2DHSelectionParamTool-> ipsBachelorPart() > m_ipsBachelorCutValue_Bs2DsPi);

  //  info()<< " test Fs ipsBs mombach ptbach momDs FsDs ipsBach "
  //      <<m_B2DHSelectionParamTool-> FsBs() <<"  "<<m_B2DHSelectionParamTool-> ipsBs() <<"   "
  //      <<m_B2DHSelectionParamTool-> momBachelorPart()<<"   "
  //      <<m_B2DHSelectionParamTool-> ptBachelorPart()<<"   "
  //      <<m_B2DHSelectionParamTool-> momDs()<<"   "
  //      <<m_B2DHSelectionParamTool->  FsDs()<<"   "
  //      <<m_B2DHSelectionParamTool-> ipsBachelorPart()<<"   "<<endmsg;
  // info()<<"test cuts  m_FsBsCutValue m_ipsBsTightCutValue  m_momBachelorUpperCutValue m_momBachelorLowerCutValue "
  //      <<"   m_ptBachelorCutValue  m_momDsUpperCutValue m_FsDsCutValue m_ipsBachelorCutValue "
  //      << m_FsBsCutValue_Bs2DsPi <<"   "<<m_ipsBsTightCutValue<<"   "<<m_momBachelorUpperCutValue_Bs2DsPi
  //      <<"   "<<m_momBachelorLowerCutValue_Bs2DsPi<<"   "<<m_ptBachelorCutValue_Bs2DsPi
  //      <<"   "<<m_momDsUpperCutValue_Bs2DsPi<<"   "<<m_FsDsCutValue_Bs2DsPi<<"   "
  //      <<m_ipsBachelorCutValue_Bs2DsPi<<endmsg;
  //


  double prMinusMuDll = m_B2DHSelectionParamTool->  BachelorPrDll() -
                        m_B2DHSelectionParamTool-> BachelorMuDll();
  m_auxCutDll_Bachelor_Bs2DsPi = 
       (m_B2DHSelectionParamTool->  BachelorKaDll() >  m_dllKaonBachelorLowerCutValue_Bs2DsPi) &&
       (m_B2DHSelectionParamTool-> BachelorKaDll() < m_dllKaonBachelorUpperCutValue_Bs2DsPi) &&
        (m_B2DHSelectionParamTool->  BachelorPrDll() < m_dllProtonBachelorCutValue_Bs2DsPi ) &&
        ( prMinusMuDll   <  m_dllProtonMinusMuonBachelorCutValue_Bs2DsPi) &&
        (m_B2DHSelectionParamTool->  BachelorMuDll() <   m_dllMuonBachelorUpperCutValue_Bs2DsPi);
  
  std::vector<double> akaonSecDll = m_B2DHSelectionParamTool-> bKaonSecDll();
  std::vector<double> aprotonSecDll =m_B2DHSelectionParamTool->  bProtonSecDll();
  
  int aSizeA= (int) akaonSecDll.size();
  std::vector<double>  KaonMinusPrSecDll(aSizeA ,0.0); 
  for(int j=0; j<aSizeA; ++j) {
    KaonMinusPrSecDll[j]=   akaonSecDll[j]- aprotonSecDll[j];
    
  }
  
  
  m_auxCutDll_Sec_Bs2DsPi = (KaonMinusPrSecDll[0] > m_dllKaonMinusProtonSecCutValue_Bs2DsPi) &&
                            (KaonMinusPrSecDll[1] > m_dllKaonMinusProtonSecCutValue_Bs2DsPi) &&
                            ( aprotonSecDll [0]  > m_dllProtonSecCutValue_Bs2DsPi ) &&
                            ( aprotonSecDll [0]  > m_dllProtonSecCutValue_Bs2DsPi ) &&
                            ( akaonSecDll [1] >  m_dllKaonSec1LowerCutValue_Bs2DsPi ) &&
                            (  akaonSecDll [1] <  m_dllKaonSec1UpperCutValue_Bs2DsPi) &&
                            ( akaonSecDll [0] >  m_dllKaonSec0LowerCutValue_Bs2DsPi ) &&
                            (  akaonSecDll [0] <  m_dllKaonSec0UpperCutValue_Bs2DsPi) &&
                            (  aprotonSecDll [1] > m_dllProtonSec1LowerCutValue_Bs2DsPi ) &&
                            ( akaonSecDll [2] < m_dllKaonSec2CutValue_Bs2DsPi ) &&
                            ( aprotonSecDll [2] >  m_dllProtonSec2LowerCutValue_Bs2DsPi ) &&
                            ( aprotonSecDll [2] < m_dllProtonSec2UpperCutValue_Bs2DsPi);

   m_massBsWindowSel_Bs2DsPi=fabs(m_B2DHSelectionParamTool->BsMassDiff()) < m_massBsWindowCutValue_Bs2DsPi;
   m_massDsWindowSel_Bs2DsPi = fabs( m_B2DHSelectionParamTool->DsMassDiff() ) < m_massDsWindowCutValue;
   m_massWindowSel_Bs2DsPi= m_massBsWindowSel_Bs2DsPi && m_massDsWindowSel_Bs2DsPi;

    m_vtxChisqSel_Bs2DsPi = (m_B2DHSelectionParamTool-> bvtxchisq() < m_vtxBsChisqCutValue_Bs2DsPi) && 
                  (m_B2DHSelectionParamTool-> dvtxchisq() < m_vtxDsChisqCutValue_Bs2DsPi);

  
   m_fisher_Sel_Bs2DsPi =  m_B2DHSelectionParamTool-> getfisherDProbValue() > m_fisherDCutValue_Bs2DsPi;
  
}
bool B2DHSelectionCutsTool::ApplyPreTrainSel() {
  
  return m_Train_Sel;
}
bool B2DHSelectionCutsTool::ApplyFisherSel( int channelNum) {
  bool aFlag=  m_fisher_Sel_Bs2DsK;
  if(channelNum == 1 ) aFlag = m_fisher_Sel_Bs2DsPi;
  return aFlag;
}

bool B2DHSelectionCutsTool::ApplyAuxCuts( int achannelNum) {
  bool aF1 = m_auxCut_Bs_Bs2DsK;
  if(achannelNum == 1 ) {
    //  info()<< " Aux cuts   m_auxCut_Bs_Bs2DsPi m_auxCutDll_Bachelor_Bs2DsPi m_auxCutDll_Sec_Bs2DsPi m_vtxChisqSel_Bs2DsPi "<<
    //  m_auxCut_Bs_Bs2DsPi <<"   "<<m_auxCutDll_Bachelor_Bs2DsPi<<"  "<< m_auxCutDll_Sec_Bs2DsPi << "  "
    //      <<m_vtxChisqSel_Bs2DsPi<<endmsg;
    
    aF1 =  m_auxCut_Bs_Bs2DsPi &&  m_auxCutDll_Bachelor_Bs2DsPi && m_auxCutDll_Sec_Bs2DsPi && m_vtxChisqSel_Bs2DsPi ;
    
  }
  
  return aF1;
}
bool B2DHSelectionCutsTool::ApplyMassWindowCuts(int achannelNum) { 
  bool am= m_massWindowSel_Bs2DsK;
  if(achannelNum == 1 ) am=m_massWindowSel_Bs2DsPi;
  return am;
  
}
bool B2DHSelectionCutsTool::ApplyMassWindowCuts_Bs2DsK() { 
  return m_massWindowSel_Bs2DsK;
}
bool B2DHSelectionCutsTool::ApplyMassWindowCuts_Bs2DsPi() { 
  return m_massWindowSel_Bs2DsPi;
}



void B2DHSelectionCutsTool::resetSelectionFlags() {

    m_zPVBsSel=false;

   m_massBsWindowSel=false;  
   m_massDsWindowSel =false;
   m_massWindowSel=false;
   m_IpsSel=false;
   m_IpsSecSel=false;
   m_vtxChisqSel =false;
   m_ptDs_momSec_Sel=false;
   m_Train_Sel=false;


   m_auxCut_Bs_Bs2DsK=false;  
   m_auxCut_Dll_Bs2DsK=false;
   m_fisher_Sel_Bs2DsK=false;

   m_auxCut_Bs_Bs2DsPi=false;  
   m_auxCutDll_Bachelor_Bs2DsPi =false;
   m_auxCutDll_Sec_Bs2DsPi =false;
   m_fisher_Sel_Bs2DsPi=false;  


  
}
