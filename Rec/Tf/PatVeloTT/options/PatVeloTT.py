from TrackSys.Configuration import *
from TrackFitter.ConfiguredFitters import *

from Configurables import ( PatVeloTT, PatVeloTTTool,
                            TrackMasterFitter, TrackMasterExtrapolator,
                            TrackKalmanFilter, TrackSimpleExtraSelector )

patVTT = PatVeloTT("PatVeloTT")
patVTT.InputUsedTracksNames = ["Rec/Track/Forward","Rec/Track/Match"]

patVTT.addTool(TrackMasterFitter,"Fitter")
ConfiguredMasterFitter(patVTT.Fitter)
patVTT.Fitter.NumberFitIterations = 1
patVTT.Fitter.MaxNumberOutliers = 1
patVTT.Fitter.Extrapolator.ExtraSelector = "TrackSimpleExtraSelector"
patVTT.Fitter.StateAtBeamLine = False

patVTT.addTool(PatVeloTTTool, name="PatVeloTTTool")
if TrackSys().fieldOff():
   patVTT.PatVeloTTTool.minMomentum = 5000.
   patVTT.PatVeloTTTool.maxPseudoChi2 = 256
   patVTT.maxChi2 = 256.
   patVTT.PatVeloTTTool.DxGroupFactor = 0.0
   patVTT.fitTracks = False
else:
   patVTT.PatVeloTTTool.minMomentum = 800.
   patVTT.PatVeloTTTool.maxPseudoChi2 = 10000.
   patVTT.maxChi2 = 5.
