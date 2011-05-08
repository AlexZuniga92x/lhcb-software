# The file in which we define all known tracking
# configurations for HLT2
#
__author__  =  "V. Gligorov vladimir.gligorov@cern.ch"
from Hlt2Tracking  import Hlt2Tracking
from HltTrackNames import Hlt2TracksPrefix, Hlt2ForwardTracksName
from HltTrackNames import HltBiDirectionalKalmanFitSuffix,Hlt2DownstreamTracksName 
from HltTrackNames import Hlt2LongTracksName, HltUniDirectionalKalmanFitSuffix 
#
# Define all the instances of Hlt2Tracking
#
# FIRST THE UNFITTED TRACKS, SHOULD NOT BE USED BY SELECTIONS
# EVENTUALLY THIS SHOULD JUST BE REMOVED
#
# With bidirectional track fitting, no seeding
#
def Hlt2BiKalmanFittedForwardTracking() :
        return Hlt2Tracking("Hlt2BiKalmanFittedForwardTracking",        
                                                                                 Prefix                 = Hlt2TracksPrefix,
                                                                                 FastFitType            = HltBiDirectionalKalmanFitSuffix,
                                                                                 Hlt2Tracks             = Hlt2ForwardTracksName,
                                                                                 DoFastFit              = True,
                                                                                 DoSeeding              = False
                                                                        )
#
# With seeding and track fitting
#
def Hlt2BiKalmanFittedLongTracking() :
        return Hlt2Tracking("Hlt2BiKalmanFittedLongTracking",           
                                                                                 Prefix                 = Hlt2TracksPrefix,
                                                                                 FastFitType            = HltBiDirectionalKalmanFitSuffix,
                                                                                 Hlt2Tracks             = Hlt2LongTracksName,
                                                                                 DoFastFit              = True,
                                                                                 DoSeeding              = True,
                                                                                 DoCloneKilling         = True
                                                                        )
#
# Now the Downstream tracks with the bidirectional Kalman fit.
#
def Hlt2BiKalmanFittedDownstreamTracking() :
    return Hlt2Tracking("Hlt2BiKalmanFittedDownstreamTracking",            
                                                                                 Prefix                 = Hlt2TracksPrefix,
                                                                                 FastFitType            = HltBiDirectionalKalmanFitSuffix,
                                                                                 Hlt2Tracks             = Hlt2DownstreamTracksName,
                                                                                 DoFastFit              = True,
                                                                                 DoSeeding              = True
                                                                        )
#
# With track fitting and RICH PID for 3 hypotheses (kaon-proton-pion), but no seeding
#
def Hlt2BiKalmanFittedRichForProtonsForwardTracking() :
     return Hlt2Tracking("Hlt2BiKalmanFittedRichForProtonsForwardTracking", 
                         Prefix                 = Hlt2TracksPrefix,
                         FastFitType            = HltBiDirectionalKalmanFitSuffix,
                         Hlt2Tracks             = Hlt2ForwardTracksName,
                         DoFastFit              = True,
                         DoSeeding              = False,
                         DoCloneKilling         = False,
                         RichHypos              = ["pion","proton","kaon"],
                         RichTrackCuts          = {"Forward":{ "Chi2Cut" : [0,16], "PCut" : [2,100], "PtCut" : [1.9,100], "Likelihood" : [-999999,999999] }  }
                         )

#
# With track fitting and RICH PID for 2 hypotheses (proton-pion), but no seeding
#
def Hlt2BiKalmanFittedRichForLowPTProtonsForwardTracking() :
     return Hlt2Tracking("Hlt2BiKalmanFittedRichForLowPTProtonsForwardTracking", 
                         Prefix                 = Hlt2TracksPrefix,
                         FastFitType            = HltBiDirectionalKalmanFitSuffix,
                         Hlt2Tracks             = Hlt2ForwardTracksName,
                         DoFastFit              = True,
                         DoSeeding              = False,
                         DoCloneKilling         = False,
                         RichHypos              = ["pion","proton","kaon"],
                         RichTrackCuts          = {"Forward":{ "Chi2Cut" : [0,16], "PCut" : [2,100], "PtCut" : [0.5,100], "Likelihood" : [-999999,999999] }  }
                         )

#
def setDataTypeForTracking(trackingInstance, dataType):

    trackingInstance.setProp("DataType", dataType)
