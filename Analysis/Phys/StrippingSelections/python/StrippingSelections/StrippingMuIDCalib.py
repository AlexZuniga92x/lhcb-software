## #####################################################################
# A stripping selection for muon identification calibration trough
# the study of J/psi(1S) -> mu+ mu- and Lambda to p pi decays
#
# @authors S.Furcas, G.Lanfranchi, M.Palutan, A.Sarti 
# @date 2009-July-30
# 
## #####################################################################
from Gaudi.Configuration import *
from Configurables import GaudiSequencer, CombineParticles, FilterDesktop
from StrippingConf.StrippingLine import StrippingLine, StrippingMember


# Create Jpsi -> mumu candidates out of no pid muons
## ############################################################
#Muons common cut
mucocut = '(0.5<PPINFO(LHCb.ProtoParticle.InAccMuon,-1)) & (P>3*GeV) & (PT>800*MeV) & (TRCHI2DOF<10) & (ISLONG)'
#Tag and probe cuts: 
#   TAG:: IsMuon and P>6Gev and Pt>1.5 GeV
#   PROBE:: Ecal (-10,1000) and Hcal (1000,4000) energy filtered

tag1cuts = " (CHILDCUT(ISMUON,1)) & (CHILDCUT((P>6*GeV),1)) & (CHILDCUT((PT>1.3*GeV),1)) "
tag2cuts = " (CHILDCUT(ISMUON,2)) & (CHILDCUT((P>6*GeV),2)) & (CHILDCUT((PT>1.3*GeV),2)) "

#tag1cuts = " (CHILDCUT(ISMUON,1)) & (CHILDCUT((P>6*GeV),1)) "
#tag2cuts = " (CHILDCUT(ISMUON,2)) & (CHILDCUT((P>6*GeV),2)) "

probe2cuts = " ( (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)<1000*MeV),2)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)<4000*MeV),2)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)>-10*MeV),2)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)>-10*MeV),2)) ) "
probe1cuts = " ( (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)<1000*MeV),1)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)<4000*MeV),1)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)>-10*MeV),1)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)>-10*MeV),1)) )  "

child1cuts = tag1cuts + " & " + probe2cuts

child2cuts = tag2cuts + " & " + probe1cuts

JpsiCombine = StrippingMember( CombineParticles
                               , 'Combine'
                               , InputLocations = [ 'StdNoPIDsMuons' ]
                               , DecayDescriptor = 'J/psi(1S) -> mu+ mu-'
                               , DaughtersCuts = { 'mu+' : mucocut , 
                                                   'mu-' : mucocut }
                               , CombinationCut = "(ADAMASS('J/psi(1S)')<300*MeV)"
                               , MotherCut = "(VFASPF(VCHI2/VDOF)<20) & ( " + child1cuts + " | " + child2cuts + " ) "
                               )

# Define the line
## ############################################################
jp_line = StrippingLine('JpsiNoPID'
                        , prescale = 1.
                        , algos = [ JpsiCombine ]
                        )


# Create b -> Jpsi -> mumu candidates out of no pid muons
## ############################################################
#Muons common cut
mucocut = '(0.5<PPINFO(LHCb.ProtoParticle.InAccMuon,-1)) & (P>3*GeV) & (PT>800*MeV) & (TRCHI2DOF<3) & (ISLONG)'
#Tag and probe cuts: 
#   TAG:: IsMuon and P>6Gev and Pt>1.5 GeV
#   PROBE:: Ecal (-10,1000) and Hcal (1000,4000) energy filtered
tag1cuts = " (CHILDCUT(ISMUON,1)) & (CHILDCUT((P>6*GeV),1)) & (CHILDCUT((PT>1.5*GeV),1)) "
tag2cuts = " (CHILDCUT(ISMUON,2)) & (CHILDCUT((P>6*GeV),2)) & (CHILDCUT((PT>1.5*GeV),2)) "

probe2cuts = " ( (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)<1000*MeV),2)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)<4000*MeV),2)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)>-10*MeV),2)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)>1000*MeV),2)) ) "
probe1cuts = " ( (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)<1000*MeV),1)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)<4000*MeV),1)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)>-10*MeV),1)) & (CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)>1000*MeV),1)) )  "

child1cuts = tag1cuts + " & " + probe2cuts

child2cuts = tag2cuts + " & " + probe1cuts

hlt1cuts = "  (CHILDCUT((MIPDV(PRIMARY)>0.08),1)) | (CHILDCUT((PT>6*GeV),1)) | ((CHILDCUT((MIPDV(PRIMARY)>0.025),1)) & (CHILDCUT((MIPDV(PRIMARY)>0.05),2))) "
hlt2cuts = "  (CHILDCUT((MIPDV(PRIMARY)>0.08),2)) | (CHILDCUT((PT>6*GeV),2)) | ((CHILDCUT((MIPDV(PRIMARY)>0.025),2)) & (CHILDCUT((MIPDV(PRIMARY)>0.05),1))) "


JpsiFromBCombine = StrippingMember( CombineParticles
                                    , 'FromBCombine'
                                    , InputLocations = [ 'StdNoPIDsMuons' ]
                                    , DecayDescriptor = 'J/psi(1S) -> mu+ mu-'
                                    , DaughtersCuts = { 'mu+' : mucocut , 
                                                        'mu-' : mucocut }
                                    , CombinationCut = "(ADAMASS('J/psi(1S)')<300*MeV)"
                                    , MotherCut = "(VFASPF(VCHI2/VDOF)<6) & ( ( " + child1cuts + " & ( " + hlt1cuts + " ) ) | (" 
                                                                                  + child2cuts + " & ( " + hlt2cuts + " ) ) ) "
                                    )

# Define the line
## ############################################################
b_line = StrippingLine('JpsiFromBNoPID'
                        , prescale = 1.
                        , algos = [ JpsiFromBCombine ]
                        )


