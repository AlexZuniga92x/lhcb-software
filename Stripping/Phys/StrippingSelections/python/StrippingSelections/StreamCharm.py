#
#   Charm stripping selection
#
#   all physics channels with hadronic final states from charm decay
#
#   @author A. Poluektov
#   @date 2010-01-21
# 

from Gaudi.Configuration import *
from StrippingConf.StrippingStream import StrippingStream

from StrippingSelections.StrippingD2KS0h_KS02PiPi import StrippingD2KS0h_KS02PiPiConf
from StrippingSelections import StrippingD2hhh
from StrippingSelections import StrippingD2hhhXS
from StrippingSelections import StrippingD5h
from StrippingSelections import StrippingDstarD2KShh
from StrippingSelections import StrippingDstarD2KShhRobust
from StrippingSelections import StrippingDstar_D2KPiPi0
from StrippingSelections import StrippingDstarPromptWithD02HHNoPt
from StrippingSelections import StrippingD02MuMu

#from StrippingSelections import StrippingDstarD02KKpipiRobust
#from StrippingSelections import StrippingDstarD02KKpipiRegular
from StrippingSelections import StrippingDstarD02KKmumuRobust
#from StrippingSelections import StrippingDstarD02KKmumuRegular

from StrippingSelections.StrippingDstarD02KKmumuRegular import StrippingDstarD02KKmumuRegularConf
from StrippingSelections.StrippingDstarD02KKpipiRegular import StrippingDstarD02KKpipiRegularConf

#from StrippingSelections.StrippingDstarPromptWithD02K3Pi import StrippingDstarPromptWithD02K3PiConf
#from StrippingSelections.StrippingD02K3PiForCrossSection import StrippingD02K3PiForCrossSectionConf

from StrippingSelections.StrippingDstarPromptWithD02HH import StrippingDstarPromptWithD02HHConf
#from StrippingSelections.StrippingDstarVeryLooseWithD02HH import StrippingDstarVeryLooseWithD02HHConf
from StrippingSelections.StrippingDstarD2KShh          import StrippingDstarD2KShhConf
from StrippingSelections.StrippingDstarD02xx import  StrippingDstarD02xxConf
#from StrippingSelections.StrippingDstarD02xxForCross import  StrippingDstarD02xxForCrossConf
from StrippingSelections.StrippingCcbar2Ppbar import StrippingCcbar2PpbarConf

from StrippingSelections.StrippingBd2KstarMuMu import StrippingBd2KstarMuMuConf
from StrippingSelections.StrippingLambdac      import StrippingLambdacConf

from StrippingSelections.StrippingDs2piPhiNoPt import StrippingDs2piPhiConf

from StrippingSelections.StrippingD02K3Pi import StrippingD02K3PiForXSectionConf 

from StrippingSelections.StrippingD2hh import StrippingD2hhConf
from StrippingSelections.StrippingJpsippForD0MuMu import StrippingJpsippForD0MuMuConf
stream = StrippingStream("Charm")

stream.appendLines( [
          StrippingD2KS0h_KS02PiPiConf().D2KS0h_KS02PiPi()
        , StrippingD2hhh.lineD2PPP_A_NoPID_Bkg
#        , StrippingD2hhh.lineD2PPP_B_NoPID_Bkg
        , StrippingD2hhh.lineD2PPP_A_NoPID_Sig
#        , StrippingD2hhh.lineD2PPP_B_NoPID_Sig
        , StrippingD2hhh.lineD2KPP_A_NoPID_Bkg
#        , StrippingD2hhh.lineD2KPP_B_NoPID_Bkg
        , StrippingD2hhh.lineD2KPP_A_LoosePID_Bkg
        , StrippingD2hhh.lineD2KPP_B_LoosePID_Bkg
        , StrippingD2hhh.lineD2KPP_A_NoPID_Sig
#        , StrippingD2hhh.lineD2KPP_B_NoPID_Sig
        , StrippingD2hhh.lineD2KPP_A_LoosePID_Sig
        , StrippingD2hhh.lineD2KPP_B_LoosePID_Sig
        , StrippingD2hhh.lineD2KKP_B_NoPID_Bkg
        , StrippingD2hhh.lineD2KKP_A_LoosePID_Bkg
        , StrippingD2hhh.lineD2KKP_B_LoosePID_Bkg
        , StrippingD2hhh.lineD2KKP_B_NoPID_Sig
        , StrippingD2hhh.lineD2KKP_A_LoosePID_Sig
        , StrippingD2hhh.lineD2KKP_B_LoosePID_Sig
        , StrippingD2hhh.lineD2KKK_A_LoosePID_Bkg
        , StrippingD2hhh.lineD2KKK_B_LoosePID_Bkg
        , StrippingD2hhh.lineD2KKK_A_LoosePID_Sig
        , StrippingD2hhh.lineD2KKK_B_LoosePID_Sig
        , StrippingD2hhh.lineD2KKP_A_NoPID_Sig
        , StrippingD2hhh.lineD2KKP_A_NoPID_Bkg
#        , StrippingD2hhh.lineDs2KPP_A_LoosePID_Sig
#        , StrippingD2hhh.lineDs2KPP_B_LoosePID_Sig
#        , StrippingD2hhh.lineDs2KPP_A_LoosePID_Bkg
#        , StrippingD2hhh.lineDs2KPP_B_LoosePID_Bkg
#        , StrippingD2hhh.lineD2KPP_DCS_A_LoosePID_Sig
#        , StrippingD2hhh.lineD2KPP_DCS_B_LoosePID_Sig
#        , StrippingD2hhh.lineD2KPP_DCS_A_LoosePID_Bkg
#        , StrippingD2hhh.lineD2KPP_DCS_B_LoosePID_Bkg
        , StrippingD2hhh.lineD2HHH
#        , StrippingD2hhhXS.lineD2PPP_B_NoPID_BkgXS
#        , StrippingD2hhhXS.lineD2PPP_B_NoPID_SigXS
#        , StrippingD2hhhXS.lineD2KPP_B_NoPID_BkgXS
      #  , StrippingD2hhhXS.lineD2KPP_B_LoosePID_BkgXS
#        , StrippingD2hhhXS.lineD2KPP_B_NoPID_SigXS
     #   , StrippingD2hhhXS.lineD2KPP_B_LoosePID_SigXS
#        , StrippingD2hhhXS.lineD2KKP_B_NoPID_BkgXS
     #   , StrippingD2hhhXS.lineD2KKP_B_LoosePID_BkgXS
#        , StrippingD2hhhXS.lineD2KKP_B_NoPID_SigXS
     #   , StrippingD2hhhXS.lineD2KKP_B_LoosePID_SigXS
#        , StrippingD2hhhXS.lineD2KKK_B_LoosePID_BkgXS
#        , StrippingD2hhhXS.lineD2KKK_B_LoosePID_SigXS
#        , StrippingD2hhhXS.lineDs2KPP_B_LoosePID_SigXS
#        , StrippingD2hhhXS.lineDs2KPP_B_LoosePID_BkgXS
#        , StrippingD2hhhXS.lineD2KPP_DCS_B_LoosePID_SigXS
#        , StrippingD2hhhXS.lineD2KPP_DCS_B_LoosePID_BkgXS
#        , StrippingD5h.lineD2KKPPP_B_NoPID_Bkg 
#        , StrippingD5h.lineD2KKPPP_B_LoosePID_Bkg 
#        , StrippingD5h.lineD2KKPPP_B_NoPID_Sig    
#        , StrippingD5h.lineD2KKPPP_B_LoosePID_Sig
#        , StrippingD5h.lineD2KPPPP_B_NoPID_Bkg 
#        , StrippingD5h.lineD2KPPPP_B_LoosePID_Bkg 
#        , StrippingD5h.lineD2KPPPP_B_NoPID_Sig    
#        , StrippingD5h.lineD2KPPPP_B_LoosePID_Sig 
        , StrippingDstar_D2KPiPi0.lineRS
        , StrippingDstar_D2KPiPi0.lineWS
#        , StrippingDstarVeryLooseWithD02HHConf().line()
        , StrippingBd2KstarMuMuConf().line_for_DKpipipi()
        , StrippingDstarPromptWithD02HHNoPt.lineDstarPromptWithD02HHNoPt
        , StrippingDstarPromptWithD02HHNoPt.lineD02HHPromptNoPt
        , StrippingD02MuMu.lineD02MuMu
        , StrippingD02MuMu.lineDstarWithD02MuMu
        , StrippingD02MuMu.lineD02HHForD02MuMu
        , StrippingD02MuMu.lineDstarWithD02HHForD02MuMu
#        , StrippingDstarD02KKpipiRobust.line1
#        , StrippingDstarD02KKpipiRegular.line1
        , StrippingDstarD02KKmumuRobust.line1

#        , StrippingDstarD02KKmumuRegular.line1
          ,StrippingDs2piPhiConf().line() 
    ] )
stream.appendLines( [ StrippingJpsippForD0MuMuConf().ForD0mumu_Line() ] )
stream.appendLines( StrippingDstarD02KKmumuRegularConf().lines )
stream.appendLines( StrippingDstarD02KKpipiRegularConf().lines )

stream.appendLines( StrippingDstarPromptWithD02HHConf().linesDstarOnly() )
stream.appendLines( StrippingDstarD2KShhConf().MakeLines() )
stream.appendLines( StrippingDstarD02xxConf().lines() )
#stream.appendLines( StrippingDstarD02xxForCrossConf().lines() )
# Charmonium -> p pbar
stream.appendLines( [ StrippingCcbar2PpbarConf().Nominal_Line() ] )
stream.appendLines( [ StrippingCcbar2PpbarConf().Exclusive_Line() ] )
#stream.appendLines( StrippingDstarPromptWithD02K3PiConf().lines() )
#stream.appendLines( StrippingD02K3PiForCrossSectionConf().lines() )
D02K3PiForXSection = StrippingD02K3PiForXSectionConf(
  'D02K3PiForCrossSection'
  , config = StrippingD02K3PiForXSectionConf._default_config)

stream.appendLines( StrippingLambdacConf().lines() )

from StrippingSelections.StrippingPromptCharm import StrippingPromptCharmConf
_conf = { "D0Prescale"    : 1.00 ,
          "DplusPrescale" : 1.00 }
Lines = StrippingPromptCharmConf( config = _conf )
stream.appendLines ( Lines.lines() )

from StrippingSelections.StrippingD2hh import StrippingD2hhConf
stream.appendLines( StrippingD2hhConf().lines() )

from StrippingSelections.StrippingD2hhLTUnbiased import StrippingD2hhLTUnbiasedConf
stream.appendLines( StrippingD2hhLTUnbiasedConf().lines() )

# Calubration line for bbar cross section in Bd->D*munu    from S.Vecchi 04-10-2010
from StrippingSelections import StrippingDstarVeryLooseWithD02Kpi  
confDstarVeryLooseWithD02Kpi = StrippingDstarVeryLooseWithD02Kpi.DstarVeryLooseWithD02KpiAllLinesConf(StrippingDstarVeryLooseWithD02Kpi.confdict)
stream.appendLines( confDstarVeryLooseWithD02Kpi.Lines ) 

from StrippingSelections import StrippingDForBSemi
confDforBSemi = StrippingDForBSemi.DforBSemiLinesConf("confDforBSemi",StrippingDForBSemi.confdict)
stream.appendLines(confDforBSemi.lines())

from StrippingSelections import StrippingD0ForBXX
confD0forBXX = StrippingD0ForBXX.D0forBXXLinesConf(StrippingD0ForBXX.name,StrippingD0ForBXX.confdict)
stream.appendLines(  confD0forBXX.lines() )

