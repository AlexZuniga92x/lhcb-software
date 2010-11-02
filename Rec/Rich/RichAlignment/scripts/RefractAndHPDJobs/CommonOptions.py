from Brunel.Configuration import *
from Configurables import Brunel, LHCbApp

importOptions("$APPCONFIGOPTS/Brunel/earlyData.py")
importOptions("$APPCONFIGOPTS/Brunel/DataType-2010.py")

LHCbApp().DDDBtag   = "head-20101003"
LHCbApp().CondDBtag = "head-20101010"

Brunel().OutputType = "None"

# Only tracking and RICH reco
Brunel().RecoSequence = ["Decoding","VELO","TT","IT","OT","Tr","Vertex","RICH"]

from Configurables import RichRecSysConf
rConf = RichRecSysConf("RichOfflineRec")

# Disable Aerogel (Not being calibrated)
#rConf.Radiators = [ "Rich1Gas","Rich2Gas" ]

# Photon selection options
rConf.photonConfig().SelectionMode = "Wide"

# No PID or ring finding, for speed
rConf.TracklessRingAlgs = []
rConf.PidConfig = "None"

# Only RICH Monitoring
from Configurables import RecMoniConf
RecMoniConf().MoniSequence = ["RICH"]

# Tweek the RICH Monitoring histograms
from Configurables import RichRecQCConf
rMoni = RichRecQCConf("OfflineRichMoni")
rMoni.removeMonitors ( ["HPDIFBMonitoring","PidMonitoring","TracklessRingAngles"] )
# Open up the CK res plot range, for the Wide photon selection
rMoni.CKThetaResRange = [ 0.03, 0.0075, 0.004 ]

# Alignment histogram options
from Configurables import RichAlignmentConf
conf = RichAlignmentConf("OfflineRichMoni_RichAlignmentConf")
conf.Histograms = "Expert"
conf.HPDList = [ [], [0], [0] ]

# Save all histograms from image analysis
from Configurables import Rich__Mon__RichHPDImageSummary
imageSummary = Rich__Mon__RichHPDImageSummary("RichHPDImageSummary")
imageSummary.Keep2DHistograms = True
