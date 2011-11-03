from Configurables import Swimming
from Gaudi.Configuration import *

Swimming().DataType = '2011'
Swimming().EvtMax = 100
Swimming().Simulation = False
Swimming().DDDBtag = "head-20110302"
Swimming().CondDBtag = "head-20110321"
Swimming().Persistency = 'ROOT'
Swimming().InputType = 'DST'
Swimming().OutputFile = 'SwimTrigDST.dst'
Swimming().SwimStripping = False
Swimming().TCK = '0x006D0032'
Swimming().Hlt1Triggers = ["Hlt1TrackAllL0Decision"]#, "Hlt1TrackMuonDecision"]
Swimming().Hlt2Triggers = ["Hlt2CharmHadD02HH_D02KPiDecision"]#, "Hlt2DiMuonDetachedJPsiDecision"]
Swimming().OffCands = "/Event/CharmCompleteEvent/Phys/D2hhPromptD2KPiLine"
Swimming().TransformName = '2011_WithBeamSpotFilter'
Swimming().SelectMethod = 'random'
Swimming().OutputType = 'DST'
Swimming().UseFileStager = False
Swimming().Input = ["DATAFILE='PFN:castor:/castor/cern.ch/grid/lhcb/LHCb/Collision11/CHARMCOMPLETEEVENT.DST/00012586/0000/00012586_00000584_1.charmcompleteevent.dst'"]

from Configurables import FileStagerSvc
FileStagerSvc().OutputLevel = 1

from Configurables import CondDB
CondDB(UseOracle = True)

# Configure Moore to swim the trigger
from Swimming.Configuration import ConfigureMoore
ConfigureMoore()

# The custom event loop
from Gaudi.Configuration import setCustomMainLoop
from Swimming.EventLoop import SwimmingEventLoop
setCustomMainLoop(SwimmingEventLoop)
