"""
Configuration file for 'Standard Loose Lambda_c+'
"""
__author__  = "Liming Zhang"
__date__ = '09/12/2009'
__version__ = "version $Revision: 1.1 $"
# =============================================================================
__all__ = (
    'StdLooseLambdac2PKPi' ,
    'locations'
    )
# =============================================================================
from Gaudi.Configuration import *
from Configurables       import CombineParticles
from CommonParticles.Utils import *

## ============================================================================
## create the algorithm 
StdLooseLambdac2PKPi = CombineParticles ( 'StdLooseLambdac2PKPi' )

StdLooseLambdac2PKPi.InputLocations = [ "StdLooseKaons", "StdLoosePions", "StdLooseProtons" ]

StdLooseLambdac2PKPi.DecayDescriptor = "[Lambda_c+ -> K- p+ pi+]cc" 
StdLooseLambdac2PKPi.DaughtersCuts = {
    "K+"  : "((PT >250*MeV) & (P > 2*GeV) & (MIPCHI2DV(PRIMARY) > 4.))", 
    "pi+" : "((PT >250*MeV) & (P > 2*GeV) & (MIPCHI2DV(PRIMARY) > 4.))",
    "p+"  : "((PT >250*MeV) & (P > 2*GeV) & (MIPCHI2DV(PRIMARY) > 4.))"
    }

StdLooseLambdac2PKPi.CombinationCut = "(ADAMASS('Lambda_c+')<150*MeV) & (APT>1.*GeV) & (ADOCACHI2CUT(50, ''))"
StdLooseLambdac2PKPi.MotherCut = "(VFASPF(VCHI2) < 30 )" 

## configure Data-On-Demand service 
locations = updateDoD ( StdLooseLambdac2PKPi )

## ============================================================================
if '__main__' == __name__ :

    print __doc__
    print __author__
    print __version__
    print locationsDoD ( locations ) 

# =============================================================================
# The END 
# =============================================================================
