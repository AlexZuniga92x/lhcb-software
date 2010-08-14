########################################################################
#
# Options for pre-selecting B->hh' and Bs->mu+ mu- events
#
# @author Angelo Carbone
# @date 2010-05-21
#
########################################################################                   

from Configurables import GaudiSequencer, CombineParticles
from StrippingConf.StrippingLine import StrippingLine

PreselB2Charged2Body = CombineParticles("PreselB2Charged2Body")
PreselB2Charged2Body.InputLocations  = ["StdLoosePions"]
PreselB2Charged2Body.DecayDescriptor = "B0 -> pi+ pi-"
PreselB2Charged2Body.CombinationCut = "(AMAXDOCA('') < 0.08*mm)"
PreselB2Charged2Body.DaughtersCuts = { "pi+" : "(PT>700*MeV) & (MIPDV(PRIMARY)>0.08) & (TRCHI2DOF<5)"}
PreselB2Charged2Body.MotherCut = "(M>4800*MeV) & (M<5800*MeV) & (BPVIP()<0.06) & (BPVVDSIGN>1.8) & (INTREE ( (ABSID=='pi+') & (PT>2400*MeV))) & (INTREE ( (ABSID=='pi+') & (MIPDV(PRIMARY)>0.2)))"




# Create the line for this selection
line = StrippingLine('B2Charged2Body'
                     , prescale = 1
                     , algos = [ PreselB2Charged2Body ]
                     )

