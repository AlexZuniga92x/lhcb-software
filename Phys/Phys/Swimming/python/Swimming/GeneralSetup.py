#some more general options which come before
#the gaudi.initialize step
from LHCbKernel.Configuration import *
from Configurables import DataOnDemandSvc,CondDB
from Configurables import DstConf, CaloDstUnPackConf
from Configurables import Gaudi__IODataManager as IODataManager
CaloDstUnPackConf ( Enable = True )
DstConf           ( EnableUnpack = True )
CondDB().IgnoreHeartBeat = True
IODataManager().UseGFAL=False
importOptions("$STDOPTS/DecodeRawEvent.py")
appConf = ApplicationMgr()
appConf.HistogramPersistency = 'ROOT'
appConf.ExtSvc += ['DataOnDemandSvc']
EventSelector().PrintFreq = -1
EventSelector().OutputLevel = 6 
