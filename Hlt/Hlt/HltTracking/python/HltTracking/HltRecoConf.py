#!/usr/bin/env gaudirun.py
# =============================================================================
# $Id: $
# =============================================================================
## @file
#  Configuration of Hlt Reconstruction Algorithms
#  @author Sebastian Neubert, sebastian.neubert@cern.ch
#  @date 2014-11-24
# =============================================================================
"""
 Parameters for the HLT reconstruction algorithms
 Note: the actual definition of the tracking sequences can be found in
 HltSharedTracking.py
 Hlt1Tracking.py
 and
 Hlt2Tracking.py
 Line-specific parameters should go into the respectiv HltLine configurables
"""
# =============================================================================
__author__  = "Sebastian Neubert, sebastian.neubert@cern.ch"
__version__ = ""
# =============================================================================

from Gaudi.Configuration import * 
from LHCbKernel.Configuration import *

from GaudiKernel.SystemOfUnits import MeV, mm, m

class HltRecoConf(LHCbConfigurableUser):
   __slots__ = { "Forward_HPT_MinPt"            :  500. * MeV
                 ,"Forward_HPT_MinP"            : 3000. * MeV
                 ,"Forward_LPT_MinPt"           :   80. * MeV  # was 200
                 ,"Forward_LPT_MinP"            : 1000. * MeV # was 3000
                 ,"Forward_MaxOTHits"           : 15000
                 ,"MatchVeloMuon_MinP"          : 6000. * MeV
                 ,"GoodTrCHI2PDOF"              : 3.0  # This TrCHI2PDOF is used in the sequence to mark hits of good tracks.
                 ,"MaxTrCHI2PDOF"               : 3.0  # This TrCHI2PDOF is used in the making of protoparticles.
                 ,"VeloSelectionCut"            : "(~TrBACKWARD) & ( TrNVELOMISS < 100 )"
                 ,"FitVelo"                     : True
                 ,"OfflineSeeding"              : True
                 ,"OfflineRich"                 : True
                 ,"AddGhostProb"                : True
                 ,"InitFits"                    : True
                 ,"MoreOfflineLikeFit"          : True   # This runs a fit in HLT1 and HLT2 which uses the default offline fit
                                                         # with a simplified material description
                 ,"FitIterationsInHltFit"       : 1      # Increases the number of fit iterations in the current Hlt fit.
                                                         # Default is 1.
                 }

   def getConfiguredForward(Name) :
       
       pass

   def __apply_configuration__(self):
        """
        Apply Hlt configuration
        """
        log.debug("Hlt configuration")
        import GaudiKernel.ProcessJobOptions
        GaudiKernel.ProcessJobOptions.PrintOff()


MaxOTHits = 15000 #--> move to CommonForwardOptions, this one is used e.g. in Hlt1GECs.py or Hlt1BeamGasLines.py

MatchVeloMuonOptions = {"MaxChi2DoFX" : 10.,
                        "XWindow" : 200.,
                        "YWindow" : 200.  }
MatchVeloTTMuonOptions = MatchVeloMuonOptions

VeloTTToolOptions = {"minMomentum" : 0.0,
                     "minPT" : 100.0 ,
                     "PassTracks" : True,
                     "PassHoleSize" : 40 * mm,  # Update to new default from 45 mm.
                     }
                          
VeloTTOptions = { }

CommonPVOptions = {"UseBeamSpotCut" : True,
                   "BeamSpotRCut" : 0.2 * mm,
                   "TrackErrorScaleFactor" : 1.,
                   "ResolverBound" : 1 * mm,
                   "MinTracks" : 4.,
                   "maxTrackChi2" : 12.}

CommonForwardOptions = { "MaxOTHits" : 150000 , #Why do we have this three times?
                         "MaxITHits" : 999999 ,
                         "MaxNVelo" : 999999 }

#Sascha: Rename this to tool,
CommonForwardTrackingOptions = { "MaxChi2" : 40.,
                                 "MaxChi2Track" : 40.,
                                 "MinHits" : 12,  
                                 "MinOTHits" : 14  }

#Sascha: Rename this to tool and hlt1
ForwardTrackingOptions_MomEstimate = { "UseMomentumEstimate" : True,
                                       "UseWrongSignWindow" : True,
                                       "WrongSignPT" : 2000.,
                                       "Preselection" : True,
                                       "PreselectionPT" : 400.,
                                       "AddTTClusterName" : "PatAddTTCoord",
                                       "SecondLoop" : False}

ComplementForwardToolOptions = { "AddTTClusterName" : "PatAddTTCoord"
                                 ,"SecondLoop" : True} # to be decided

OnlineSeedingToolOptions = { "NDblOTHitsInXSearch" : 2,
                             "MinMomentum" : 1000.}

OfflineSeedingToolOptions = { "NDblOTHitsInXSearch" : 0,
                              "MinMomentum" : 500.}


DownstreamOptions = { "MinMomentum": 0.,
                      "MinPt": 0.}

#
# The default RICH options
#
MaxChi2 = HltRecoConf().getProp("MaxTrCHI2PDOF")
HltRichDefaultHypos			= ["pion","kaon"]
HltRichDefaultRadiators			= ["Rich1Gas","Rich2Gas"] 
HltRichDefaultTrackCuts     = { tt : { "Chi2Cut" : [0.,MaxChi2], "PCut" : [2,100], "PtCut" : [0.8,100]} for tt in ['Forward','Match']}


OfflineRichDefaultHypos	       = ["pion","kaon","proton","belowThreshold"]
OfflineRichDefaultRadiators    = ["Rich1Gas","Rich2Gas"]
OfflineRichDefaultTrackCuts    = { tt : {'Chi2Cut' : [0,3.0], 'PCut' : [0,1e9], 'PtCut' : [0,1e9] } for tt in ['Forward','Match'] }



## CommonForwardTrackingOptions_EarlyData = { "MaxChi2" : 40., 
##                                  "MaxChi2Track" : 40, 
##                                  "MinHits" : 12,  
##                                  "MinOTHits" : 14,
##                                  "MinXPlanes" : 4,
##                                  "MinPlanes" : 8,
##                                  "MaxSpreadX" : 1.5,
##                                  "MaxSpreadY" : 3.0  }
## CommonSeedingTrackingOptions_EarlyData = { "OTNHitsLowThresh" : 12,
##                                            "MinTotalPlanes" : 7,
##                                            "MaxMisses" : 2,
##                                            "MaxTrackChi2LowMult" : 10,
##                                            "MaxFinalTrackChi2" : 20,
##                                            "MaxFinalChi2" : 30,
##                                            "MaxTrackChi2" : 40,
##                                            "MaxChi2HitIT" : 10,
##                                            "MaxChi2HitOT" : 30}    
## CommonDownstreamTrackingOptions_EarlyData = { "xPredTol2" : 20.0,
##                                               "TolMatch" : 1.5,
##                                               "TolUV" : 2.0,
##                                               "maxWindowSize" : 10.0,
##                                               "MaxChisq" : 20.0,
##                                               "MaxDistance" : 0.3,
##                                               "deltaP" : 2.0,
##                                               "errorZMagnet" : 30.0 } 



   
   
