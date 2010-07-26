__author__ = 'Serezha.Barsuk@cern.ch, Jibo.He@cern.ch'
__date__ = '12/04/2010'
__version__ = '$Revision: 1.3 $'

"""
 Charmonium -> p pbar, including eta(1S) -> psi(2S)
"""

from Gaudi.Configuration import *
from LHCbKernel.Configuration import *

class StrippingCcbar2PpbarConf(LHCbConfigurableUser):
    __slots__ = {   'ProtonPT'         :  1950.   # MeV
                  , 'ProtonPIDppi'     :     2.   # CombDLL(p-pi)
                  , 'ProtonPIDpK'      :     2.   # CombDLL(p-K)
                  , 'CombUpperMass'    :  3950.   # MeV, before Vtx fit
                  , 'CombLowerMass'    :  2750.   # MeV, before Vtx fit
                  , 'UpperMass'        :  3850.   # MeV, after Vtx fit
                  , 'LowerMass'        :  2850.   # MeV, after Vtx fit
                  , 'VtxCHI2'          :    16.   # dimensionless
                  # Loose selection
                  , 'ProtonPTLoose'         :  1600.   # MeV
                  , 'ProtonPIDppiLoose'     :    -5.   # CombDLL(p-pi)
                  , 'ProtonPIDpKLoose'      :    -5.   # CombDLL(p-K)
                  , 'CombUpperMassLoose'    :  4050.   # MeV, before Vtx fit
                  , 'CombLowerMassLoose'    :  2650.   # MeV, before Vtx fit
                  , 'UpperMassLoose'        :  3950.   # MeV, after Vtx fit
                  , 'LowerMassLoose'        :  2750.   # MeV, after Vtx fit
                  , 'VtxCHI2Loose'          :    36.   # dimensionless 
                  }

    def Nominal_Line(self):
        from StrippingConf.StrippingLine import StrippingLine, StrippingMember
        StripCcbar2Ppbar = self.combineCcbar()
        return StrippingLine('Ccbar2Ppbar', prescale = 1,  algos = [ StripCcbar2Ppbar ] )


    def Loose_Line(self):
        from StrippingConf.StrippingLine import StrippingLine, StrippingMember
        StripCcbar2PpbarLoose = self.combineCcbarLoose()
        return StrippingLine('Ccbar2PpbarLoose', prescale = 0.01,  algos = [ StripCcbar2PpbarLoose ] )
   
    
    
    def combineCcbar(self):
        from Configurables import CombineParticles
        
        #---------------------------
        # Selections 
        #---------------------------
        ProtonCut = "(PT> %(ProtonPT)s *MeV) & ((PIDp-PIDpi) > %(ProtonPIDppi)s) & ((PIDp-PIDK) > %(ProtonPIDpK)s)" % self.getProps()
        CombCut = "(%(CombLowerMass)s *MeV < AM) & (AM < %(CombUpperMass)s *MeV)" % self.getProps()
        MomCut = "(VFASPF(VCHI2/VDOF)< %(VtxCHI2)s) & (%(LowerMass)s *MeV < MM) & (MM < %(UpperMass)s *MeV)" % self.getProps()
        
        StripCcbar2Ppbar = CombineParticles("StripCcbar2Ppbar")
        StripCcbar2Ppbar.InputLocations = [ "StdLooseProtons" ]
        StripCcbar2Ppbar.DecayDescriptor = "J/psi(1S) -> p+ p~-" 
        StripCcbar2Ppbar.DaughtersCuts = { "p+": ProtonCut }
        StripCcbar2Ppbar.CombinationCut = CombCut
        StripCcbar2Ppbar.MotherCut = MomCut
        return StripCcbar2Ppbar


    def combineCcbarLoose(self):
        from Configurables import CombineParticles
        
        #---------------------------
        # Loose Selections 
        #---------------------------
        ProtonLooseCut = "(PT> %(ProtonPTLoose)s *MeV) & ((PIDp-PIDpi) > %(ProtonPIDppiLoose)s) & ((PIDp-PIDK) > %(ProtonPIDpKLoose)s)" % self.getProps()
        CombLooseCut = "(%(CombLowerMassLoose)s *MeV < AM) & (AM < %(CombUpperMassLoose)s *MeV)" % self.getProps()
        MomLooseCut = "(VFASPF(VCHI2/VDOF)< %(VtxCHI2Loose)s) & (%(LowerMassLoose)s *MeV < MM) & (MM < %(UpperMassLoose)s *MeV)" % self.getProps()
        
        StripCcbar2PpbarLoose = CombineParticles("StripCcbar2PpbarLoose")
        StripCcbar2PpbarLoose.InputLocations = [ "StdLooseProtons" ]
        StripCcbar2PpbarLoose.DecayDescriptor = "J/psi(1S) -> p+ p~-" 
        StripCcbar2PpbarLoose.DaughtersCuts = { "p+": ProtonLooseCut }
        StripCcbar2PpbarLoose.CombinationCut = CombLooseCut
        StripCcbar2PpbarLoose.MotherCut = MomLooseCut
        return StripCcbar2PpbarLoose  
    
    def getProps(self):
        """
        From HltLinesConfigurableUser
        @todo Should be shared between Hlt and stripping
        Copied from StrippingBu2LLK
        """
        d = dict()
        for (k,v) in self.getDefaultProperties().iteritems() :
            d[k] = getattr(self,k) if hasattr(self,k) else v
        return d
