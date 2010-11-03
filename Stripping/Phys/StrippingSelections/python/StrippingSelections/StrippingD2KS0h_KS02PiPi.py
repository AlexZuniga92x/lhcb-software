

__author__ = 'Sam Gregson'
__date__ = '14/07/2010'
__version__ = '$Revision: 1.2 $'

'''
D->KS0(PiPi)Pi stripping selection using LoKi::Hybrid and python
configurables.
'''

from Gaudi.Configuration import *
from LHCbKernel.Configuration import *
from PhysSelPython.Wrappers import Selection, SelectionSequence, DataOnDemand, MergedSelection
from Configurables import FilterDesktop, CombineParticles, OfflineVertexFitter
import GaudiKernel.SystemOfUnits as Units

## Table for altering cut values - not used in first iteration

class StrippingD2KS0h_KS02PiPiConf(LHCbConfigurableUser):
    __slots__ = { 
        'BFlightCHI2_LL'        : 144.  
        ,  'BDIRA_LL'           : 0.99985     
        ,  'BIPCHI2_LL'         : 16.         
        ,  'BVertexCHI2_LL'     : 9.
        ,  'BachKIPCHI2_LL'     : 9.
        ,  'BachKPt_LL'         : 0.9  
        ,  'DVertexCHI2_LL'     : 6.25     
        ,  'DdaughterPiIPCHI2_LL' : 9.
        ,  'KSFlightCHI2_LL'    : 4.
        ,  'KSVertexCHI2_LL'    : 20.25
        ,  'KSdaughterPiIPCHI2_LL' : 4.
        ,  'BFlightCHI2_DD'     : 81.
        ,  'BDIRA_DD'           : 0.9998    
        ,  'BIPCHI2_DD'         : 25.         
        ,  'BVertexCHI2_DD'     : 12.25
        ,  'BachKIPCHI2_DD'     : 4.
        ,  'BachKPt_DD'         : 0.4  
        ,  'DVertexCHI2_DD'     : 16.        
        ,  'DdaughterPiIPCHI2_DD' : 4.
        ,  'KSFlightCHI2_DD'    : 4.
        ,  'KSVertexCHI2_DD'    : 20.25
        ,  'KSdaughterPiIPCHI2_DD' : 4.
        }

    _propertyDocDct = {
        'BFlightCHI2_LL'     : """ LL B flight distance chi2 cut """   
        , 'BDIRA_LL'            : """ LL B cos angle cut         """
        ,  'BIPCHI2_LL'         : """ LL B IP chi2 cut         """       
        ,  'BVertexCHI2_LL'     : """ LL B vertex chi2 cut         """
        ,  'BachKIPCHI2_LL'     : """ LL Bachelor kaon IP chi2 cut         """
        ,  'BachKPt_LL'         : """ LL Bachelor kaon Pt cut         """
        ,  'DVertexCHI2_LL'     : """ LL D0 vertex chi2 cut         """
        ,  'DdaughterPiIPCHI2_LL'     : """ LL D daughter pion IP chi2 cut         """
        ,  'KSFlightCHI2_LL'    : """ LL KS flight distance chi2 cut     """
        ,  'KSVertexCHI2_LL'    : """ LL KS vertex chi2 cut     """
        ,  'KSdaughterPiIPCHI2_LL'    : """ LL KS daughter pion IP chi2 cut         """
        ,  'BFlightCHI2_DD'     : """ DD B flight distance chi2 cut """   
        ,  'BDIRA_DD'           : """ DD B cos angle cut         """
        ,  'BIPCHI2_DD'         : """ DD B IP chi2 cut         """       
        ,  'BVertexCHI2_DD'     : """ DD B vertex chi2 cut         """
        ,  'BachKIPCHI2_DD'     : """ DD Bachelor kaon IP chi2 cut         """
        ,  'BachKPt_DD'         : """ DD Bachelor kaon Pt cut         """
        ,  'DVertexCHI2_DD'     : """ DD D0 vertex chi2 cut         """
        ,  'DdaughterPiIPCHI2_DD'     : """ DD D daughter pion IP chi2 cut         """
        ,  'KSFlightCHI2_DD'    : """ DD KS flight distance chi2 cut     """
        ,  'KSVertexCHI2_DD'    : """ DD KS vertex chi2 cut     """
        ,  'KSdaughterPiIPCHI2_DD'    : """ DD KS daughter pion IP chi2 cut         """


        }

    ###############################################
    # Create StrippingLines with these selections #
    ###############################################

        
    def D2KS0h_KS02PiPi( self ) :
        from StrippingConf.StrippingLine import StrippingLine, StrippingMember
        DForD2KS0h_KS02PiPi = self.DForD2KS0h_KS02PiPiAlg()
        return StrippingLine('D2KS0h_KS02PiPi_stripping_line'
                             , prescale = 1
                             , algos = [ DForD2KS0h_KS02PiPi ]
                             , postscale = 1
                             )

        ##############################################
        # KS0 - LL and DD 
        ##############################################
    def KS0ForD2KS0h_KS02PiPiAlg(self):
        from Configurables import CombineParticles
        import GaudiKernel.SystemOfUnits as Units

        # Outline the KS0 daughter pion cuts
        #KS0_DaughterCut = "(MIPCHI2DV(PRIMARY) > 9) & CHILDCUT((MIPCHI2DV(PRIMARY)>100),1) & CHILDCUT((MIPCHI2DV(PRIMARY)>100),2)"
        KS0_DaughterCut = "(ADMASS('KS0')<20*MeV) & (MIPCHI2DV(PRIMARY) > 9) & (VFASPF(VCHI2/VDOF) < 10) & CHILDCUT((PT>0.2*GeV),1) & CHILDCUT((PT >0.2*GeV),2) & CHILDCUT((TRCHI2DOF < 4),1) & CHILDCUT((TRCHI2DOF < 4),2) &  CHILDCUT((MIPCHI2DV(PRIMARY)>100),1) & CHILDCUT((MIPCHI2DV(PRIMARY)>100),2)"
      
        KS0ForD2KS0h_KS02PiPi = FilterDesktop("StripKS0ForD2KS0h_KS02PiPi")
        #KS0ForD2KS0h_KS02PiPi.InputLocations = [ "StdLooseKsLL", "StdLooseKsDD"]
        
        # Apply the cuts on the KS0 daughter pions and KS0
        KS0ForD2KS0h_KS02PiPi.Code =  KS0_DaughterCut 

        _stdKsLL = DataOnDemand( Location = "Phys/StdLooseKsLL" )
        _stdKsDD = DataOnDemand( Location = "Phys/StdLooseKsDD" )

	_KS = MergedSelection( "MergedKS0ForD2KS0h_KS02PiPi", RequiredSelections = [_stdKsLL, _stdKsDD] )

        _sel = Selection( "SelKS0ForD2KS0h_KS02PiPi",
                          RequiredSelections = [_KS], Algorithm = KS0ForD2KS0h_KS02PiPi )
        
        return _sel 
        
        ############################################
        # D meson - LL and DD
        ############################################
    def DForD2KS0h_KS02PiPiAlg(self):
        from Configurables import CombineParticles
        import GaudiKernel.SystemOfUnits as Units
        
        #D_DaughterCut = { "pi+" : "(P > 2*GeV) & (TRCHI2DOF < 10) & PPCUT(PP_RICHTHRES_PI) & (MIPCHI2DV(PRIMARY) > 49)" }
        #D_ComboCut = "(AM> 1380*MeV) & (AM < 2220*MeV) & (AMAXDOCA('') < 1.0*mm)" 
        #D_MotherCut = "(VFASPF(VCHI2/VDOF) < 20) & (MM> 1400*MeV) & (MM < 2200*MeV) & (MIPCHI2DV(PRIMARY) < 4)"

        D_DaughterCut = { "pi+" : "(P > 5*GeV) & (PT > 0.5*GeV) & (TRCHI2DOF < 3)  & (MIPCHI2DV(PRIMARY) > 49)" }
        D_ComboCut = "(AM> 1580*MeV) & (AM < 2090*MeV) & (AMAXDOCA('LoKi::DistanceCalculator') < 0.6*mm)"
        D_MotherCut = "(VFASPF(VCHI2/VDOF) < 10) & (MM> 1600*MeV) & (MM < 2070*MeV) & (MIPCHI2DV(PRIMARY) < 50)"


        
        _stdPions = DataOnDemand( Location = "Phys/StdLoosePions" )
        
        DForD2KS0h_KS02PiPi = CombineParticles("StripDForD2KS0h_KS02PiPi")
        DForD2KS0h_KS02PiPi.DecayDescriptor = "[D+ -> KS0 pi+]cc"
        #   DForD2KS0h_KS02PiPi.InputLocations =  ["StripKS0ForD2KS0h_KS02PiPi",
     #"StdLoosePions"
      #                                            ]
      
        DForD2KS0h_KS02PiPi.Preambulo = [ "from LoKiProtoParticles.decorators import *"]

      
        DForD2KS0h_KS02PiPi.DaughtersCuts = D_DaughterCut
        DForD2KS0h_KS02PiPi.CombinationCut = D_ComboCut
        DForD2KS0h_KS02PiPi.MotherCut = D_MotherCut
        
        
        _sel = Selection( "SelDForD2KS0h_KS02PiPi",
                          RequiredSelections = [ _stdPions, self.KS0ForD2KS0h_KS02PiPiAlg() ], Algorithm = DForD2KS0h_KS02PiPi )
        
        return _sel 


##(AMAXDOCA('') < 0.6*mm)
## (PIDK - PIDpi > -0.1) & (PIDK - PIDpi < 0.1)
##  Bachelor_Cut = { "pi+" : "(P > 2*GeV) & (TRCHI2DOF < 10) &
##PPCUT(PP_RICHTRES_PI) & (MIPCHI2DV(PRIMARY) > 49)" }
##myAlg.Preambulo = [ "from LoKiProtoParticles.decorators import *"]
