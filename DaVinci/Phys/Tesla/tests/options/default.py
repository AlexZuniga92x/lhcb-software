# Options file for the QMTest to check that Tesla produces no errors

from Configurables import Tesla 
from Gaudi.Configuration import *

from Configurables import RecombineRawEvent, DecodeRawEvent
RecombineRawEvent()
DecodeRawEvent().DataOnDemand = True

from Configurables import ConfigTarFileAccessSvc
ConfigTarFileAccessSvc().File='../../options/config.tar'

prefix = "Tesla"

Tesla().TriggerLine = "Hlt2IncPhi"
Tesla().ReportVersion = 2
Tesla().OutputPrefix = prefix
Tesla().OutputLevel = 4
Tesla().EvtMax = -1

from GaudiConf.IOHelper import IOHelper
IOHelper().inputFiles( [ "PFN:root://castorlhcb.cern.ch//castor/cern.ch/user/s/sbenson/TempTestFiles/RemadeSel_phiphi_1k_Tutorial_0044.dst" ] ) 
