#!/usr/bin/env python
# =============================================================================
# $Id: PurgedLists.py
# =============================================================================
## @file  FlavourTagging/PurgedLists.py
#  remove signal B daughters from particle lists to be used for charm candidates
# =============================================================================
"""
configuration file
"""
__author__  = "Riccardo Cenci, Jack Wimberley"
__version__ = "CVS tag $Name: not supported by cvs2svn $, version $Revision: 1.0 $"
# =============================================================================

# =============================================================================
__all__ = (
    'CharmPreambulo',
    'Tag_StdD02KPi',
    'Tag_StdD02KPiPiPi',
    'Tag_StdD02KsPiPi',
    'Tag_StdDstar2D0Pi2KsPiPi',
    'Tag_StdD02KPiPi0',
    'Tag_StdD02KPipart',
    'Tag_StdD02Kepart',
    'Tag_StdD02Kmupart',
    'Tag_StdDp2KPipart',
    'Tag_StdDp2Kepart',
    'Tag_StdDp2Kmupart',
    'Tag_StdDp2KPiPi',
    'Tag_StdDp2KsPi',
    'Tag_StdLambdaC2PKPi',
    'Tag_StdLambdaC2LambdaPi',
    'Tag_StdLambdaC2PKs',
    'locations'
    )
# =============================================================================
from Gaudi.Configuration import *


elist = ["Phys/StdAllNoPIDsElectrons/Particles"]
mulist = ["Phys/StdAllNoPIDsMuons/Particles"]
pilist = ["Phys/StdAllNoPIDsPions/Particles"]
prolist = ["Phys/StdAllNoPIDsProtons/Particles"]
pizlist = ["Phys/StdLooseMergedPi0/Particles", "Phys/StdLooseResolvedPi0/Particles"]
klist = ["Phys/StdAllNoPIDsKaons/Particles"]
kslist = ["Phys/StdLooseKsLL/Particles", "Phys/StdLooseKsLD/Particles", "Phys/StdLooseKsDD/Particles"]

## pikslist = ["Phys/StdNoPIDsDownPions/Particles", bcandsel]

##########################################################################################

CharmPreambulo = [
    "from LoKiPhysMC.decorators import *" ,
    "from LoKiPhysMC.functions import mcMatch",
    "from GaudiKernel.SystemOfUnits import *"
    ]

# all tracks have TRCHI2NDOF < 3, (TRCHI2DOF<4.0) & (PT>100*MeV) & (P>1000*MeV) & (MIPCHI2DV(PRIMARY)>4.0) & (TRGHP<0.3)"
piSelection = "(PT>100*MeV) & (P>1000*MeV) & (BPVIPCHI2()>4.0)"
piksSelection = "(PT>100*MeV) & (P>1000*MeV) & (BPVIPCHI2()>4.0)"
pi0Selection = "(ABSID==111) & (PT>500*MeV) & (P>1000*MeV)"

eSelection = "(PT>100*MeV) & (P>1000*MeV) & (BPVIPCHI2()>4.0) & (PROBNNe > 0.05)" 
muSelection = "(PT>100*MeV) & (P>1000*MeV) & (BPVIPCHI2()>4.0) & (PROBNNmu > 0.05)"
kSelection = "(PT>100*MeV) & (P>1000*MeV) & (BPVIPCHI2()>4.0) & (PROBNNK > 0.05)"
proSelection = "(PT>100*MeV) & (P>1000*MeV) & (BPVIPCHI2()>4.0) & (PROBNNp > 0.05)"

ksSelection = "(PT>100*MeV) & (P>1000*MeV) & (BPVVDCHI2>1000)"
lam0Selection = "(PT>100*MeV) & (P>1000*MeV) & (BPVVDCHI2>1000)"

dSelection = "(VFASPF(VCHI2/VDOF)<10) & (BPVVDCHI2>25) & (BPVDIRA>0.99)"
dStarSelection = "(VFASPF(VCHI2/VDOF)<25) & (BPVVDCHI2>25) & (BPVDIRA>0.99)"

lambdaCSelection = "(VFASPF(VCHI2/VDOF)<10) & (BPVVDCHI2>25) & (BPVDIRA>0.99)"
lambdaSelection = "(VFASPF(VCHI2/VDOF)<10) & (BPVVDCHI2>25)"

##########################################################################################

from Configurables       import CombineParticles

from CommonParticles.Utils import *

from PhysSelPython.Wrappers import Selection


##########################################################################################
# D0 -> K Pi #
Tag_StdD02KPi = CombineParticles ( 'Tag_StdD02KPi' )

Tag_StdD02KPi.Preambulo = CharmPreambulo

Tag_StdD02KPi.Inputs = []
Tag_StdD02KPi.Inputs.extend(pilist)
Tag_StdD02KPi.Inputs.extend(klist)

Tag_StdD02KPi.DecayDescriptors = ["[D0 -> K- pi+]cc"]

Tag_StdD02KPi.DaughtersCuts = {
    "K+" : kSelection
    , "pi+" : piSelection
    }

Tag_StdD02KPi.CombinationCut = "(ADAMASS('D0')<100*MeV) & (ADOCAMAX('')<0.5*mm)"
Tag_StdD02KPi.MotherCut = "(ADMASS('D0')<50*MeV)" + " & " + dSelection

Tag_StdD02KPi.Output = "Phys/" + Tag_StdD02KPi.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdD02KPi )


##########################################################################################
# D0 -> K Pi Pi Pi
#
Tag_StdD02KPiPiPi = CombineParticles ("Tag_StdD02KPiPiPi")

Tag_StdD02KPiPiPi.Preambulo = CharmPreambulo

Tag_StdD02KPiPiPi.Inputs = []
Tag_StdD02KPiPiPi.Inputs.extend(pilist)
Tag_StdD02KPiPiPi.Inputs.extend(klist)

Tag_StdD02KPiPiPi.DecayDescriptors = ["[D0 -> K- pi+ pi+ pi-]cc"]

Tag_StdD02KPiPiPi.DaughtersCuts = {
    "K+" : kSelection
    , "pi+" : piSelection
    , "pi-" : piSelection
    }

Tag_StdD02KPiPiPi.CombinationCut = "(ADAMASS('D0')<100*MeV) & (ADOCAMAX('')<0.5*mm)"
Tag_StdD02KPiPiPi.MotherCut = "(ADMASS('D0')<50*MeV)" + " & " + dSelection

Tag_StdD02KPiPiPi.Output = "Phys/" + Tag_StdD02KPiPiPi.name() + "/Particles"

## configure Data-On-Demand service
locations.update( updateDoD ( Tag_StdD02KPiPiPi ) )


##########################################################################################
# D0 -> Ks Pi Pi
#
Tag_StdD02KsPiPi = CombineParticles("Tag_StdD02KsPiPi")

Tag_StdD02KsPiPi.Preambulo = CharmPreambulo

Tag_StdD02KsPiPi.Inputs = []
Tag_StdD02KsPiPi.Inputs.extend(pilist)
Tag_StdD02KsPiPi.Inputs.extend(kslist)

Tag_StdD02KsPiPi.DecayDescriptors = ["[D0 -> KS0 pi+ pi-]cc"]

Tag_StdD02KsPiPi.DaughtersCuts = {
    "KS0" : ksSelection
    , "pi+" : piSelection 
    , "pi-" : piSelection 
    }

Tag_StdD02KsPiPi.CombinationCut = "(ADAMASS('D0')<100*MeV) & (ADOCAMAX('')<0.5*mm)"
Tag_StdD02KsPiPi.MotherCut = "(ADMASS('D0')<50*MeV)"  + " & " + dSelection

Tag_StdD02KsPiPi.Output = "Phys/" + Tag_StdD02KsPiPi.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdD02KsPiPi )


## ##########################################################################################
Tag_StdDstar2D0Pi2KsPiPi = CombineParticles ( 'Tag_StdDstar2D0Pi2KsPiPi' )

Tag_StdDstar2D0Pi2KsPiPi.Preambulo = CharmPreambulo

Tag_StdDstar2D0Pi2KsPiPi.Inputs = []
Tag_StdDstar2D0Pi2KsPiPi.Inputs.extend(pilist)
Tag_StdDstar2D0Pi2KsPiPi.Inputs.extend([Tag_StdD02KsPiPi.Output])

Tag_StdDstar2D0Pi2KsPiPi.DecayDescriptors = ["[D*(2010)+ -> D0 pi+]cc"] 

Tag_StdDstar2D0Pi2KsPiPi.CombinationCut = "(ADAMASS('D*(2010)+')<50*MeV) & (ADOCAMAX('')<0.5*mm)"
Tag_StdDstar2D0Pi2KsPiPi.MotherCut = "(M-MAXTREE('D0'==ABSID,M)<165.5)" + " & " + dStarSelection

Tag_StdDstar2D0Pi2KsPiPi.Output = "Phys/" + Tag_StdDstar2D0Pi2KsPiPi.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdDstar2D0Pi2KsPiPi )


##########################################################################################
# D0 -> K Pi Pi0
#
Tag_StdD02KPiPi0 = CombineParticles("Tag_StdD02KPiPi0")

Tag_StdD02KPiPi0.Preambulo = CharmPreambulo

Tag_StdD02KPiPi0.Inputs = []
Tag_StdD02KPiPi0.Inputs.extend(pilist)
Tag_StdD02KPiPi0.Inputs.extend(pizlist)
Tag_StdD02KPiPi0.Inputs.extend(klist)

Tag_StdD02KPiPi0.DecayDescriptors = ["[D0 -> K- pi+ pi0]cc"]

Tag_StdD02KPiPi0.DaughtersCuts = {
    "K+" : kSelection
    , "pi+" : piSelection
    , "pi0" : pi0Selection
    }

Tag_StdD02KPiPi0.CombinationCut = "(ADAMASS('D0')<100*MeV) & (ADOCAMAX('')<0.5*mm)"
Tag_StdD02KPiPi0.MotherCut = "(ADMASS('D0')<50*MeV)" + " & " + dSelection

Tag_StdD02KPiPi0.Output = "Phys/" + Tag_StdD02KPiPi0.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdD02KPiPi0 )


##########################################################################################
# D0 -> K Pi partial
#
Tag_StdD02KPipart = CombineParticles("Tag_StdD02KPipart")

Tag_StdD02KPipart.Preambulo = CharmPreambulo

Tag_StdD02KPipart.Inputs = []
Tag_StdD02KPipart.Inputs.extend(pilist)
Tag_StdD02KPipart.Inputs.extend(klist)

Tag_StdD02KPipart.DecayDescriptors = ["[D0 -> K- pi+]cc"]

Tag_StdD02KPipart.DaughtersCuts = {
    "K+" : kSelection + " & (MIPCHI2DV(PRIMARY)>10.0)",
    "pi+" : piSelection + " & (MIPCHI2DV(PRIMARY)>10.0)"
    }

Tag_StdD02KPipart.CombinationCut = "( in_range(1450*MeV,AWM('K-','pi+'),1850*MeV) | in_range(842*MeV,AWM('K-','pi+'),955*MeV) )"
Tag_StdD02KPipart.MotherCut = "( (ADMASS('K*(892)0')<50*MeV) | ( (DMASS('D0')>-400*MeV) & (DMASS('D0')<0*MeV) ) )" + " & " + dSelection

Tag_StdD02KPipart.Output = "Phys/" + Tag_StdD02KPipart.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdD02KPipart )





##########################################################################################
# D0 -> K e X 
#
Tag_StdD02Kepart = CombineParticles("Tag_StdD02Kepart")

Tag_StdD02Kepart.Preambulo = CharmPreambulo

Tag_StdD02Kepart.Inputs = []
Tag_StdD02Kepart.Inputs.extend(elist)
Tag_StdD02Kepart.Inputs.extend(klist)

Tag_StdD02Kepart.DecayDescriptors = ["[D0 -> K- e+]cc"]

Tag_StdD02Kepart.DaughtersCuts = {
    "K+" : kSelection + " & (MIPCHI2DV(PRIMARY)>10.0)",
    "e+" : eSelection + " & (MIPCHI2DV(PRIMARY)>10.0)"
    }

Tag_StdD02Kepart.CombinationCut = "( in_range(1450*MeV,AWM('K-','e+'),1850*MeV) | in_range(842*MeV,AWM('K-','e+'),955*MeV) )"
Tag_StdD02Kepart.MotherCut = "( (ADMASS('K*(892)0')<50*MeV) | (DMASS('D0')>-400*MeV) & (DMASS('D0')<0*MeV) )" + " & " + dSelection

Tag_StdD02Kepart.Output = "Phys/" + Tag_StdD02Kepart.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdD02Kepart )



##########################################################################################
# D0 -> K mu X 
#
Tag_StdD02Kmupart = CombineParticles("Tag_StdD02Kmupart")

Tag_StdD02Kmupart.Preambulo = CharmPreambulo

Tag_StdD02Kmupart.Inputs = []
Tag_StdD02Kmupart.Inputs.extend(mulist)
Tag_StdD02Kmupart.Inputs.extend(klist)

Tag_StdD02Kmupart.DecayDescriptors = ["[D0 -> K- mu+]cc"]

Tag_StdD02Kmupart.DaughtersCuts = {
    "K+" : kSelection + " & (MIPCHI2DV(PRIMARY)>10.0)",
    "mu+" : muSelection + " & (MIPCHI2DV(PRIMARY)>10.0)"
    }

Tag_StdD02Kmupart.CombinationCut = "( in_range(1450*MeV,AWM('K-','mu+'),1850*MeV) | in_range(842*MeV,AWM('K-','mu+'),955*MeV))"
Tag_StdD02Kmupart.MotherCut = "( (ADMASS('K*(892)0')<50*MeV) | (DMASS('D0')>-400*MeV) & (DMASS('D0')<0*MeV) )" + " & " + dSelection

Tag_StdD02Kmupart.Output = "Phys/" + Tag_StdD02Kmupart.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdD02Kmupart )



##########################################################################################
# D+ -> K Pi partial
#
Tag_StdDp2KPipart = CombineParticles("Tag_StdDp2KPipart")

Tag_StdDp2KPipart.Preambulo = CharmPreambulo

Tag_StdDp2KPipart.Inputs = []
Tag_StdDp2KPipart.Inputs.extend(pilist)
Tag_StdDp2KPipart.Inputs.extend(klist)

Tag_StdDp2KPipart.DecayDescriptors = ["[D+ -> K- pi+]cc"]

Tag_StdDp2KPipart.DaughtersCuts = {
    "K-" : kSelection + " & (MIPCHI2DV(PRIMARY)>10.0)"
    , "pi+" : piSelection + " & (MIPCHI2DV(PRIMARY)>10.0)"
    }

Tag_StdDp2KPipart.CombinationCut = "( in_range(1450*MeV,AWM('K-','pi+'),1850*MeV) | in_range(842*MeV,AWM('K-','pi+'),955*MeV))"
Tag_StdDp2KPipart.MotherCut = "( (ADMASS('K*(892)0')<50*MeV) | ( (DMASS('D+')>-400*MeV) & (DMASS('D+')<0*MeV) ) )" + " & " + dSelection

Tag_StdDp2KPipart.Output = "Phys/" + Tag_StdDp2KPipart.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdDp2KPipart )


##########################################################################################
# D+ -> K e X 
#
Tag_StdDp2Kepart = CombineParticles("Tag_StdDp2Kepart")

Tag_StdDp2Kepart.Preambulo = CharmPreambulo

Tag_StdDp2Kepart.Inputs = []
Tag_StdDp2Kepart.Inputs.extend(elist)
Tag_StdDp2Kepart.Inputs.extend(klist)

Tag_StdDp2Kepart.DecayDescriptors = ["[D+ -> K- e+]cc"]

Tag_StdDp2Kepart.DaughtersCuts = {
    "K+" : kSelection + " & (MIPCHI2DV(PRIMARY)>10.0)",
    "e+" : eSelection + " & (MIPCHI2DV(PRIMARY)>10.0)"
    }

Tag_StdDp2Kepart.CombinationCut = "( in_range(1450*MeV,AWM('K-','e+'),1850*MeV) | in_range(842*MeV,AWM('K-','e+'),955*MeV) )"
Tag_StdDp2Kepart.MotherCut = "( (ADMASS('K*(892)0')<50*MeV) | (DMASS('D+')>-400*MeV) & (DMASS('D+')<0*MeV) )" + " & " + dSelection

Tag_StdDp2Kepart.Output = "Phys/" + Tag_StdDp2Kepart.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdDp2Kepart )



##########################################################################################
# D+ -> K mu X 
#
Tag_StdDp2Kmupart = CombineParticles("Tag_StdDp2Kmupart")

Tag_StdDp2Kmupart.Preambulo = CharmPreambulo

Tag_StdDp2Kmupart.Inputs = []
Tag_StdDp2Kmupart.Inputs.extend(mulist)
Tag_StdDp2Kmupart.Inputs.extend(klist)

Tag_StdDp2Kmupart.DecayDescriptors = ["[D+ -> K- mu+]cc"]

Tag_StdDp2Kmupart.DaughtersCuts = {
    "K+" : kSelection + " & (MIPCHI2DV(PRIMARY)>10.0)",
    "mu+" : muSelection + " & (MIPCHI2DV(PRIMARY)>10.0)"
    }

Tag_StdDp2Kmupart.CombinationCut = "( in_range(1450*MeV,AWM('K-','mu+'),1850*MeV) | in_range(842*MeV,AWM('K-','mu+'),955*MeV))"
Tag_StdDp2Kmupart.MotherCut = "( (ADMASS('K*(892)0')<50*MeV) | (DMASS('D+')>-400*MeV) & (DMASS('D+')<0*MeV) )" + " & " + dSelection

Tag_StdDp2Kmupart.Output = "Phys/" + Tag_StdDp2Kmupart.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdDp2Kmupart )


##########################################################################################
# D+ -> K Pi Pi
#
Tag_StdDp2KPiPi = CombineParticles ( 'Tag_StdDp2KPiPi' )

Tag_StdDp2KPiPi.Preambulo = CharmPreambulo

Tag_StdDp2KPiPi.Inputs = []
Tag_StdDp2KPiPi.Inputs.extend(pilist)
Tag_StdDp2KPiPi.Inputs.extend(klist)

Tag_StdDp2KPiPi.DecayDescriptors = ["[D+ -> K- pi+ pi+]cc"]
Tag_StdDp2KPiPi.DaughtersCuts = {
    "K-"  : kSelection
    , "pi+" : piSelection
    }

Tag_StdDp2KPiPi.Output = "Phys/" + Tag_StdDp2KPiPi.name() + "/Particles"

Tag_StdDp2KPiPi.CombinationCut = "(ADAMASS('D+')<100*MeV)"
Tag_StdDp2KPiPi.MotherCut = "(ADMASS('D+')<50*MeV)" + " & " + dSelection

## Configure Data-On-Demand service
locations = updateDoD ( Tag_StdDp2KPiPi )


## ##########################################################################################
# Dp -> Ks Pi
#
Tag_StdDp2KsPi = CombineParticles("Tag_StdDp2KsPi")

Tag_StdDp2KsPi.Preambulo = CharmPreambulo

Tag_StdDp2KsPi.Inputs = []
Tag_StdDp2KsPi.Inputs.extend(pilist)
Tag_StdDp2KsPi.Inputs.extend(kslist)

Tag_StdDp2KsPi.DecayDescriptors = ["[D+ -> KS0 pi+]cc"]

Tag_StdDp2KsPi.DaughtersCuts = {
    "KS0" : ksSelection
    , "pi+" : piSelection
    }

Tag_StdDp2KsPi.CombinationCut = "(ADAMASS('D+')<100*MeV)"
Tag_StdDp2KsPi.MotherCut = "(ADMASS('D+')<50*MeV)" + " & " + dSelection

Tag_StdDp2KsPi.Output = "Phys/" + Tag_StdDp2KsPi.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdDp2KsPi )

## ##########################################################################################
# Lambda_c -> P K Pi
#

Tag_StdLambdaC2PKPi = CombineParticles("Tag_StdLambdaC2PKPi")

Tag_StdLambdaC2PKPi.Preambulo = CharmPreambulo

Tag_StdLambdaC2PKPi.Inputs = []
Tag_StdLambdaC2PKPi.Inputs.extend(pilist)
Tag_StdLambdaC2PKPi.Inputs.extend(prolist)
Tag_StdLambdaC2PKPi.Inputs.extend(klist)

Tag_StdLambdaC2PKPi.DecayDescriptors = ["[Lambda_c+ -> p+ K- pi+]cc"]

Tag_StdLambdaC2PKPi.DaughtersCuts = {
    "pi+" : piSelection
    , "K-" : kSelection
    , "p+" : proSelection
    }

Tag_StdLambdaC2PKPi.CombinationCut = "(ADAMASS('Lambda_c+')<110*MeV) & (ADOCAMAX('')<0.5*mm)"
Tag_StdLambdaC2PKPi.MotherCut = "(ADMASS('Lambda_c+')<100*MeV)" + " & " + lambdaCSelection

Tag_StdLambdaC2PKPi.Output = "Phys/" + Tag_StdLambdaC2PKPi.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdLambdaC2PKPi )


## ##########################################################################################
# Lambda_c -> Lambda Pi
#

Tag_Lambda = CombineParticles("Tag_Lambda")
Tag_Lambda.Preambulo = CharmPreambulo
Tag_Lambda.Inputs = []
Tag_Lambda.Inputs.extend(pilist)
Tag_Lambda.Inputs.extend(prolist)
Tag_Lambda.DecayDescriptors = ["[Lambda0 -> p+ pi-]cc"]
Tag_Lambda.DaughtersCuts = {
    "pi-" : piSelection
    , "p+" : proSelection
    }

Tag_Lambda.CombinationCut = "(ADAMASS('Lambda0')<800*MeV) & (ADOCAMAX('') < 0.5*mm)"
Tag_Lambda.MotherCut = "(ADMASS('Lambda0')<150*MeV)" + " & " + lambdaSelection

lambdaloc = "Phys/" + Tag_Lambda.name() + "/Particles"
Tag_Lambda.Output = lambdaloc
lambdalist = [lambdaloc]

Tag_StdLambdaC2LambdaPi = CombineParticles("Tag_StdLambdaC2LambdaPi")

Tag_StdLambdaC2LambdaPi.Preambulo = CharmPreambulo

Tag_StdLambdaC2LambdaPi.Inputs = []
Tag_StdLambdaC2LambdaPi.Inputs.extend(pilist)
Tag_StdLambdaC2LambdaPi.Inputs.extend(lambdalist)

Tag_StdLambdaC2LambdaPi.DecayDescriptors = ["[Lambda_c+ -> Lambda0 pi+]cc"]

Tag_StdLambdaC2LambdaPi.DaughtersCuts = {
    "Lambda0" : lam0Selection
    , "pi+" : piSelection
    }

Tag_StdLambdaC2LambdaPi.CombinationCut = "(ADAMASS('Lambda_c+')<3000*MeV) & (ADOCAMAX('') < 0.5*mm)"
Tag_StdLambdaC2LambdaPi.MotherCut = "(ADMASS('Lambda_c+')<200*MeV)" + " & " + lambdaCSelection

Tag_StdLambdaC2LambdaPi.Output = "Phys/" + Tag_StdLambdaC2LambdaPi.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdLambdaC2LambdaPi )



## ##########################################################################################
# Lambda_c -> P Ks
#

Tag_StdLambdaC2PKs = CombineParticles("Tag_StdLambdaC2PKs")

Tag_StdLambdaC2PKs.Preambulo = CharmPreambulo

Tag_StdLambdaC2PKs.Inputs = []
Tag_StdLambdaC2PKs.Inputs.extend(prolist)
Tag_StdLambdaC2PKs.Inputs.extend(kslist)

Tag_StdLambdaC2PKs.DecayDescriptors = ["[Lambda_c+ -> p+ KS0]cc"]

Tag_StdLambdaC2PKs.DaughtersCuts = {
    "KS0" : ksSelection
    , "p+" : proSelection
    }

Tag_StdLambdaC2PKs.CombinationCut = "(ADAMASS('Lambda_c+')<110*MeV) & (ADOCAMAX('') < 0.5*mm)"
Tag_StdLambdaC2PKs.MotherCut = "(ADMASS('Lambda_c+')<100*MeV)" + " & " + lambdaCSelection

Tag_StdLambdaC2PKs.Output = "Phys/" + Tag_StdLambdaC2PKs.name() + "/Particles"

## configure Data-On-Demand service
locations = updateDoD ( Tag_StdLambdaC2PKs )

## ============================================================================
if '__main__' == __name__ :

    print __doc__
    print __author__
    print __version__
    print locationsDoD ( locations )

# =============================================================================
# The END
# =============================================================================
  
