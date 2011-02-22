### @file
#
#  Standard K*
#
#  @author P. Koppenburg Patrick.Koppenburg@cern.ch
#  @date 2008-07-15
#
##
from Gaudi.Configuration import *
from Configurables import CombineParticles
from Hlt2SharedParticles.GoodParticles import GoodKaons, GoodPions
from Hlt2SharedParticles.TrackFittedBasicParticles import  BiKalmanFittedKaons, BiKalmanFittedPions
from HltLine.HltLine import bindMembers

__all__ = ( 'Kstar2KPi' )

Hlt2SharedKstar2KPi = CombineParticles("Hlt2SharedKstar2KPi")
Hlt2SharedKstar2KPi.Inputs = [ BiKalmanFittedKaons.outputSelection(), BiKalmanFittedPions.outputSelection() ]
Hlt2SharedKstar2KPi.Output = 'Hlt2/Hlt2SharedKstar2KPi/Particles'
Hlt2SharedKstar2KPi.DecayDescriptor = "[K*(892)0 -> K+ pi-]cc" 
Hlt2SharedKstar2KPi.DaughtersCuts = { "pi+" : "ALL", "K+" : "ALL" } 
Hlt2SharedKstar2KPi.CombinationCut = "(ADAMASS('K*(892)0')<300)"
Hlt2SharedKstar2KPi.MotherCut = "(VFASPF(VCHI2PDOF)<25)"

Kstar2KPi = bindMembers( None, [ BiKalmanFittedKaons, BiKalmanFittedPions, Hlt2SharedKstar2KPi ] )
