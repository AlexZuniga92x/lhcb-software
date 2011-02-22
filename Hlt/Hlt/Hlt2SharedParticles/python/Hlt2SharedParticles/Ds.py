### @file
#
#  Standard Ds
#
#  @author P. Koppenburg Patrick.Koppenburg@cern.ch
#  @date 2008-07-15
#
##
from Gaudi.Configuration import *
from Configurables import CombineParticles
from Hlt2SharedParticles.GoodParticles import GoodKaons, GoodPions
from HltLine.HltLine import bindMembers

__all__ = ( 'Ds2KKPi' )

Hlt2SharedDs2KKPi = CombineParticles("Hlt2SharedDs2KKPi")
Hlt2SharedDs2KKPi.Inputs = [ GoodKaons.outputSelection(), GoodPions.outputSelection() ]
Hlt2SharedDs2KKPi.Output = 'Hlt2/Hlt2SharedDs2KKPi/Particles'
Hlt2SharedDs2KKPi.DecayDescriptor = "[D_s- -> K+ K- pi-]cc" 

Hlt2SharedDs2KKPi.DaughtersCuts = {"K+"  : "ALL", "pi+" : "ALL"}
# need a large window to have the Ds* in as well.
Hlt2SharedDs2KKPi.CombinationCut = "(ADAMASS('D_s-')<200*MeV) & (APT>1*GeV)"
Hlt2SharedDs2KKPi.MotherCut = "(VFASPF(VCHI2PDOF)<6.67) & (MIPDV(PRIMARY)>1.0*mm)"

Ds2KKPi = bindMembers( None, [ GoodKaons, GoodPions, Hlt2SharedDs2KKPi ] )
