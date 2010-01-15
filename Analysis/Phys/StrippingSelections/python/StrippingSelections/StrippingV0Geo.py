 ########################################################################
#
# Options for V0 Reconstruction algorithm
#
########################################################################
from os import environ
import GaudiKernel.SystemOfUnits as Units
from Gaudi.Configuration import *
from Configurables import GaudiSequencer
from Configurables import DaVinci, CombineParticles
import CommonParticles.StdNoPIDsPions
import CommonParticles.StdNoPIDsProtons
import CommonParticles.StdNoPIDsDownPions
import CommonParticles.StdNoPIDsDownProtons
from StrippingConf.StrippingLine import StrippingLine, StrippingMember
from CommonParticles.Utils import *



LambdaAllCombineGeo = StrippingMember(
    CombineParticles
    , 'LambdaCombineAllGeo'    
    , InputLocations  = [ "StdNoPIDsDownPions", "StdNoPIDsDownProtons","StdNoPIDsPions", "StdNoPIDsProtons" ]
    , DecayDescriptor = "[Lambda0 -> p+ pi-]cc"
    , CombinationCut  = "AM < 1.5 * GeV "
    , MotherCut = """
    ( ADMASS('Lambda0') < 20*MeV ) & 
    ( ((CHILD(MIPDV(PRIMARY), 1) * CHILD(MIPDV(PRIMARY), 2)) / MIPDV(PRIMARY))> 1*mm ) &
    ( VFASPF ( VCHI2 ) < 100 )
    """
    )

##########################################################
KsAllCombineGeo = StrippingMember(
    CombineParticles
    , 'KsCombineAllGeo'    
    ,  InputLocations  = [ "StdNoPIDsDownPions","StdNoPIDsPions" ]
    ,  DecayDescriptor = " KS0 -> pi+ pi- "
    ,  CombinationCut  = " AM < 1*GeV "                                    
    , MotherCut = """
    ( ADMASS ('KS0') < 100 * MeV ) & 
    (((CHILD(MIPDV(PRIMARY), 1) * CHILD(MIPDV(PRIMARY), 2)) / MIPDV(PRIMARY))> 1*mm) &
    ( VFASPF ( VCHI2 ) < 100 )
    """
    )

##########################################################################
line_lambda_all= StrippingLine(
    'LambdaAllGeo'
    , prescale = 1
    , algos = [  LambdaAllCombineGeo ])


line_KS_all= StrippingLine(
    'KSAllGeo'
    , prescale = 1
    , algos = [  KsAllCombineGeo ])

############################################################################
