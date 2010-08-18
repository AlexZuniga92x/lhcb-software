
from Brunel.Configuration import *
from Configurables import Brunel, LHCbApp

importOptions("$APPCONFIGOPTS/Brunel/earlyData.py")
importOptions("$APPCONFIGOPTS/Brunel/DataType-2010.py")
#importOptions("$APPCONFIGOPTS/UseOracle.py")

LHCbApp().DDDBtag   = "head-20100518"
LHCbApp().CondDBtag = "head-20100730"

Brunel().OutputType = "None"

Brunel().EvtMax    = 1000
Brunel().PrintFreq = 100

# Only tracking and RICH reco
Brunel().RecoSequence    = ["Decoding","VELO","TT","IT","OT","Tr","Vertex","RICH"]

from Configurables import RichRecSysConf
rConf = RichRecSysConf("RichOfflineRec")

# Disable Aerogel (Not being calibrated)
rConf.Radiators = [ "Rich1Gas","Rich2Gas" ]

# No PID or ring finding
rConf.TracklessRingAlgs = []
rConf.PidConfig = "None"

# Only RICH Monitoring
from Configurables import RecMoniConf
RecMoniConf().MoniSequence = ["RICH"]

from Configurables import RichRecQCConf
rMoni = RichRecQCConf("OfflineRichMoni")
rMoni.removeMonitors ( ["HPDIFBMonitoring","PidMonitoring","TracklessRingAngles"] )

from Configurables import RichAlignmentConf
conf = RichAlignmentConf("OfflineRichMoni_RichAlignmentConf")
conf.Histograms = "Expert"

importOptions("/usera/jonesc/MyDataOptions/Collisions10/Raw/RUN_75718_RealData_90000000_RAW_PFNs.py")

#from Configurables import CondDB, CondDBAccessSvc
#CondDB().addLayer(CondDBAccessSvc("RichCKCond",ConnectionString="sqlite_file:NewRichCKRefIndexCalib.db/LHCBCOND",DefaultTAG="HEAD"))
