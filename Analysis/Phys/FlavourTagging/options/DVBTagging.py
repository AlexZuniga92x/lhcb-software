########################################################################
#
# Example Options for BTagging algorithm
#
# @author Marc Grabalosa
# @date 2010-02-20
#
########################################################################
from Gaudi.Configuration import *
from Configurables import GaudiSequencer, DaVinci

######################################################################\
##
# CheatedSelection.

from Configurables import CheatedSelection, PhysDesktop

cheatsel = CheatedSelection("CheatedSelection")
cheatsel.InputLocations = [ "Phys/TaggingPions" ]
cheatsel.AssociatorInputData = [ "Phys/CheatedSelection/Particles" ]
cheatsel.OutputLevel = 4


########################################################################
#
# Flavour tagging. 
#
from Configurables import BTagging, BTaggingTool, BTaggingChecker, TriggerTisTos, TaggingUtils, MCMatchObjP2MCRelator, BackgroundCategory

location = "Phys/CheatedSelection"

tag = BTagging("BTagging")
tag.InputLocations = [ location ]

tag.OutputLevel    = 3
tag.addTool( PhysDesktop )
tag.PhysDesktop.OutputLevel = 4
tag.addTool( TriggerTisTos )
tag.TriggerTisTos.OutputLevel = 4
tag.addTool( TaggingUtils )
tag.TaggingUtils.OutputLevel = 4
tag.addTool( MCMatchObjP2MCRelator )
tag.MCMatchObjP2MCRelator.OutputLevel = 4
tag.addTool( BackgroundCategory )
tag.BackgroundCategory.OutputLevel = 4

tag.addTool( BTaggingTool )
tag.BTaggingTool.ChoosePVCriterium = "PVbyIP" #needed by CheatedSel
#tag.BTaggingTool.CombineTaggersName = "CombineTaggersNN" #combine taggers with NN
tag.BTaggingTool.OutputLevel    = 3

#Import cuts for MC2010
importOptions('$FLAVOURTAGGINGOPTS/MC2010_cuts.py')
#Import cuts for Stripping9
#importOptions('$FLAVOURTAGGINGOPTS/Stripping9_cuts.py')
#Import cuts for Stripping10
#importOptions('$FLAVOURTAGGINGOPTS/Stripping10_cuts.py')


######################################################################\
##
# Flavour tagging Checker:

tagcheck = BTaggingChecker("BTaggingChecker")
tagcheck.InputLocations = [ location ]
tagcheck.TagsLocation = location+"/FlavourTags"
tagcheck.OutputLevel = 4

########################################################################
#
# Standard configuration
#
MessageSvc().Format = "% F%30W%S%7W%R%T %0W%M"

DaVinci().EvtMax     = 200                         # Number of events
DaVinci().SkipEvents = 0                           # Events to skip
DaVinci().PrintFreq  = 1
DaVinci().DataType   = "2010" 
DaVinci().Simulation = True

DaVinci().DDDBtag    = "head-20100119"
DaVinci().CondDBtag  = "sim-20100222-vc-md100"

########################################################################

DaVinci().MoniSequence = [  cheatsel,
                            tag,
                            tagcheck
                         ]  # The algorithms

########################################################################
#/releases/DAVINCI/DAVINCI_v24r7/DaVinciSys/tests/options/DVTestTagging.py
#
# example data file (Bs2DsPi)
#

EventSelector().Input = [ "DATAFILE='castor://castorlhcb.cern.ch:9002//castor/cern.ch/grid/lhcb/MC/2010/DST/00005967/0000/00005967_00000001_1.dst?svcClass=lhcbdata&castorVersion=2' TYP='POOL_ROOTTREE' OPT='READ'" ]

