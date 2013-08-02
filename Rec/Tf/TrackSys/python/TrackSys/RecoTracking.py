from os import environ
from Gaudi.Configuration import *
import GaudiKernel.ProcessJobOptions
from TrackSys.Configuration import *
from GaudiKernel.SystemOfUnits import mm


def RecoTracking(exclude=[]):
   '''What used to be in the options file, moved here'''
   ## Start TransportSvc, needed by track fit
   ApplicationMgr().ExtSvc.append("TransportSvc")
   
   ## --------------------------------------------------------------------
   ## Pattern Recognition and Fitting
   ## --------------------------------------------------------------------
   
   # Which algs to run ?
   trackAlgs = TrackSys().getProp("TrackPatRecAlgorithms")

   if "Velo" or "FastVelo" in trackAlgs :
      from Configurables import DecodeVeloRawBuffer
      GaudiSequencer("RecoDecodingSeq").Members += [ DecodeVeloRawBuffer("DecodeVeloClusters")]

      veloClusters = DecodeVeloRawBuffer("DecodeVeloClusters")
      veloClusters.DecodeToVeloLiteClusters = True;
      veloClusters.DecodeToVeloClusters     = True;
      globalCuts = TrackSys().getProp("GlobalCuts")
      if( "Velo" in globalCuts ) :
          veloClusters.MaxVeloClusters =  globalCuts["Velo"]
      
   from Configurables import RawBankToSTClusterAlg, RawBankToSTLiteClusterAlg
   GaudiSequencer("RecoDecodingSeq").Members += [ RawBankToSTClusterAlg("CreateTTClusters"),
                                                  RawBankToSTLiteClusterAlg("CreateTTLiteClusters") ]
   
   createITClusters = RawBankToSTClusterAlg("CreateITClusters")
   createITLiteClusters = RawBankToSTLiteClusterAlg("CreateITLiteClusters")
   createITClusters.DetType     = "IT"
   createITLiteClusters.DetType = "IT"
   
   GaudiSequencer("RecoDecodingSeq").Members += [ createITClusters, createITLiteClusters ]
   
   from Configurables import STOfflinePosition
   IT = STOfflinePosition('ITClusterPosition')
   IT.DetType = "IT"
   TT = STOfflinePosition('TTClusterPosition')
   TT.DetType = "TT"
   
   from STTools import STOfflineConf
   STOfflineConf.DefaultConf().configureTools()

   #importOptions( "$STTOOLSROOT/options/Brunel.opts" )
   
   ## Velo tracking
   if "FastVelo" in trackAlgs :
      from Configurables import FastVeloTracking
      GaudiSequencer("RecoVELOSeq").Members += [ FastVeloTracking() ]
      if TrackSys().timing() :
         FastVeloTracking().TimingMeasurement = True; 

   if "Velo" in trackAlgs :
      from Configurables import Tf__PatVeloGeneralTracking
      if TrackSys().veloOpen():
         if TrackSys().beamGas(): 
            from Configurables import Tf__PatVeloGeneric
            GaudiSequencer("RecoVELOSeq").Members += [ Tf__PatVeloGeneric("PatVeloGeneric"),
                                                       Tf__PatVeloGeneralTracking("PatVeloGeneralTracking")]
         else:
            from Configurables import Tf__PatVeloTrackTool
            GaudiSequencer("RecoVELOSeq").Members += [ Tf__PatVeloGeneralTracking("PatVeloGeneralTracking")]
            Tf__PatVeloGeneralTracking("PatVeloGeneralTracking").PointErrorMin = 2*mm;
            Tf__PatVeloGeneralTracking("PatVeloGeneralTracking").addTool(Tf__PatVeloTrackTool("PatVeloTrackTool"))
            Tf__PatVeloTrackTool("PatVeloTrackTool").highChargeFract = 0.5;
            if TrackSys().timing() :
               Tf__PatVeloGeneralTracking("PatVeloGeneralTracking").TimingMeasurement = True;
      else:
         from Configurables import ( Tf__PatVeloRTracking, Tf__PatVeloSpaceTool, Tf__PatVeloSpaceTracking )
         GaudiSequencer("RecoVELOSeq").Members += [ Tf__PatVeloRTracking("PatVeloRTracking"),
                                                    Tf__PatVeloSpaceTracking("PatVeloSpaceTracking"),
                                                    Tf__PatVeloGeneralTracking("PatVeloGeneralTracking")
                                                    ]
         Tf__PatVeloSpaceTracking("PatVeloSpaceTracking").addTool( Tf__PatVeloSpaceTool(), name="PatVeloSpaceTool" )
         Tf__PatVeloSpaceTracking("PatVeloSpaceTracking").PatVeloSpaceTool.MarkClustersUsed = True;
         if TrackSys().timing() :
            Tf__PatVeloSpaceTracking("PatVeloSpaceTracking").TimingMeasurement = True;
            Tf__PatVeloRTracking("PatVeloRTracking").TimingMeasurement = True;
            Tf__PatVeloGeneralTracking("PatVeloGeneralTracking").TimingMeasurement = True;
            
         
   ## Special OT decoder for cosmics to merge spills.
   if TrackSys().cosmics():
      from Configurables import (Tf__OTHitCreator)
      Tf__OTHitCreator('OTHitCreator').RawBankDecoder = 'OTMultiBXRawBankDecoder'
      ## note: this does not change the OTMeasurementProvider used in the fit.
      # also adapt the MasterExtrapolator in the TrackInterpolator
      from Configurables import TrackInterpolator
      TrackInterpolator().Extrapolator.ExtraSelector = 'TrackSimpleExtraSelector'
      
      
      
   ## Make sure the default extrapolator and interpolator use simplified material
   if TrackSys().simplifiedGeometry() and ('SimpleGeom' not in exclude):
      from Configurables import TrackMasterExtrapolator, TrackInterpolator
      TrackMasterExtrapolator().MaterialLocator = 'SimplifiedMaterialLocator'
      TrackInterpolator().addTool( TrackMasterExtrapolator( MaterialLocator = 'SimplifiedMaterialLocator' ), name='Extrapolator')
      
   ## Tracking sequence
   from Configurables import ProcessPhase
   track = ProcessPhase("Track");
   GaudiSequencer("RecoTrSeq").Members += [ track ]

   from Configurables import MagneticFieldSvc
   if TrackSys().fieldOff() : MagneticFieldSvc().ForcedSignedCurrentScaling = 0.
   
   if "noDrifttimes" in TrackSys().getProp("ExpertTracking"):
      from Configurables import (Tf__OTHitCreator)
      Tf__OTHitCreator("OTHitCreator").NoDriftTimes = True

   if "disableOTTimeWindow" not in TrackSys().getProp("ExpertTracking"):
      from Configurables import OTRawBankDecoder
      from GaudiKernel.SystemOfUnits import ns
      OTRawBankDecoder().TimeWindow = ( -8.0*ns, 56.0*ns )                     
      
   # Get the fitters
   from TrackFitter.ConfiguredFitters import ConfiguredFit, ConfiguredFitSeed, ConfiguredMasterFitter
   
   # Clone killer
   tracklists = []
   
   # Is this standard sequence?
   stdSeq = "fastSequence" not in TrackSys().getProp("ExpertTracking")
   
   ## Forward pattern
   if "Forward" in trackAlgs :
      track.DetectorList += [ "ForwardPat" ]
      from Configurables import PatForward
      GaudiSequencer("TrackForwardPatSeq").Members +=  [ PatForward("PatForward") ]
      from PatAlgorithms import PatAlgConf
      PatAlgConf.ForwardConf().configureAlg()
      if TrackSys().timing() :
         PatForward("PatForward").TimingMeasurement = True;    
      tracklists += ["Rec/Track/Forward"]
   
   ## Seed pattern
   if "TsaSeed" in trackAlgs and "PatSeed" in trackAlgs :
      raise RuntimeError("Cannot run both Tsa and Pat Seeding at once")

   if "TsaSeed" in trackAlgs :
      track.DetectorList += [ "SeedPat" ]
      from Configurables import Tf__Tsa__Seed, Tf__Tsa__SeedTrackCnv
      GaudiSequencer("TrackSeedPatSeq").Members += [Tf__Tsa__Seed("TsaSeed"),
                                                    Tf__Tsa__SeedTrackCnv( "TsaSeedTrackCnv" )]
      from TsaAlgorithms import TsaAlgConf
      TsaAlgConf.TsaSeedConf().configureAlg()
      if TrackSys().timing() :
         Tf__Tsa__Seed("TsaSeed").TimingMeasurement = True;
      
   if "PatSeed" in trackAlgs :
      track.DetectorList += [ "SeedPat" ]
      from Configurables import PatSeeding
      GaudiSequencer("TrackSeedPatSeq").Members += [PatSeeding("PatSeeding")]
      from PatAlgorithms import PatAlgConf
      PatAlgConf.SeedingConf().configureAlg()
      
      if TrackSys().timing() :
         PatSeeding("PatSeeding").TimingMeasurement = True;
      

      if TrackSys().cosmics() :
         from PatAlgorithms import PatAlgConf
         PatAlgConf.CosmicConf().configureAlg()
         
   if "TsaSeed" in trackAlgs or "PatSeed" in trackAlgs :
      tracklists += ["Rec/Track/Seed"]
      if "Match" in trackAlgs :
         # Fit now
         track.DetectorList += [ "SeedFit" ]
         ## Seed fit initialization
         from Configurables import TrackStateInitAlg, TrackStateInitTool
         initSeedFit = TrackStateInitAlg("InitSeedFit",
                                         TrackLocation = "Rec/Track/Seed")
         GaudiSequencer("TrackSeedFitSeq").Members += [initSeedFit]
         if "FastVelo" in trackAlgs :
            initSeedFit.StateInitTool.VeloFitterName = "FastVeloFitLHCbIDs"
         # Use small ErrorQoP fitter, needed for Match
         GaudiSequencer("TrackSeedFitSeq").Members += [ConfiguredFitSeed()]
            
   ## Match
   if "Match" in trackAlgs and "PatMatch" in trackAlgs :
      raise RuntimeError("Cannot run both TrackMatching and PatMatch at once")
   if "Match" in trackAlgs :
      track.DetectorList += [ "MatchPat" ]
      from Configurables import TrackMatchVeloSeed
      GaudiSequencer("TrackMatchPatSeq").Members += [ TrackMatchVeloSeed("TrackMatch") ]
      from TrackMatching import TrackMatchConf
      TrackMatchConf.MatchingConf().configureAlg()      
      TrackMatchVeloSeed("TrackMatch").LikCut = -99999.
      if TrackSys().timing() :
         TrackMatchVeloSeed("TrackMatch").TimingMeasurement = True;

   if "PatMatch" in trackAlgs :
      track.DetectorList += [ "MatchPat" ]
      from Configurables import PatMatch
      GaudiSequencer("TrackMatchPatSeq").Members += [ PatMatch("PatMatch") ]
   if "Match" in trackAlgs or "PatMatch" in trackAlgs :
      tracklists  += ["Rec/Track/Match"]
         
   ## Downstream
   if "Downstream" in trackAlgs :
      track.DetectorList += [ "DownstreamPat" ]
      from Configurables import PatDownstream
      GaudiSequencer("TrackDownstreamPatSeq").Members += [ PatDownstream() ];
      tracklists += ["Rec/Track/Downstream"]
      from PatAlgorithms import PatAlgConf
      #PatAlgConf.DownstreamConf().configureAlg()
      if TrackSys().timing() :
         PatDownstream("PatDownstream").TimingMeasurement = True;
      
      
   ## Velo-TT pattern
   if "VeloTT" in trackAlgs :
      track.DetectorList += ["VeloTTPat"]
      from Configurables import PatVeloTT
      GaudiSequencer("TrackVeloTTPatSeq").Members += [ PatVeloTT("PatVeloTT")] 
      from PatVeloTT import PatVeloConf
      PatVeloConf.PatVeloTTConf().configureAlg()      
      tracklists += ["Rec/Track/VeloTT"]
      if TrackSys().timing() :
         PatVeloTT("PatVeloTT").TimingMeasurement = True;
         

   ### Clean clone and fit
   track.DetectorList += ["Fit"]
   if TrackSys().getProp("OldCloneKiller"):
      from Configurables import TrackEventCloneKiller, TrackStateInitAlg, TrackContainerCopy
      cloneKiller = TrackEventCloneKiller()
      cloneKiller.TracksInContainers = tracklists
      cloneKiller.TracksOutContainer = "Rec/Track/AllBest"
      GaudiSequencer("TrackFitSeq").Members += [ cloneKiller ]
      stateInitAlg = TrackStateInitAlg("InitBestFit")
      stateInitAlg.TrackLocation = "Rec/Track/AllBest"
      if "FastVelo" in trackAlgs :
         stateInitAlg.StateInitTool.VeloFitterName = "FastVeloFitLHCbIDs"
      GaudiSequencer("TrackFitSeq").Members += [stateInitAlg]

      GaudiSequencer("TrackFitSeq").Members += [ConfiguredFit("FitBest","Rec/Track/AllBest")]      
      copyBest = TrackContainerCopy( "CopyBest" )
      copyBest.inputLocation = "Rec/Track/AllBest";
      GaudiSequencer("TrackFitSeq").Members += [ copyBest ]
                                 
      ## Velo fitting
      if "Velo" in trackAlgs or "FastVelo" in trackAlgs :
         ## Prepare the velo tracks for the fit
         track.DetectorList += [ "VeloFit"]
         from Configurables import TrackPrepareVelo
         GaudiSequencer("TrackVeloFitSeq").Members += [ TrackPrepareVelo()]
         ## Fit the velo tracks
         GaudiSequencer("TrackVeloFitSeq").Members += [ ConfiguredFit("FitVelo","Rec/Track/PreparedVelo") ]
         ## copy the velo tracks to the "best" container (except in RDST case)
         if TrackSys().getProp( "OutputType" ).upper() != "RDST":
            copyVelo = TrackContainerCopy( "CopyVelo" )
            copyVelo.inputLocation = "Rec/Track/PreparedVelo";
            GaudiSequencer("TrackVeloFitSeq").Members += [ copyVelo ]
   else:
      # complete the list of track lists
      if "Velo" in trackAlgs or "FastVelo" in trackAlgs :
         tracklists += ["Rec/Track/Velo"]
      # create the best track creator
      from Configurables import TrackBestTrackCreator
      bestTrackCreator = TrackBestTrackCreator( TracksInContainers = tracklists )
      # configure its fitter and stateinittool
      ConfiguredMasterFitter( bestTrackCreator.Fitter )
      if "FastVelo" in trackAlgs :
         bestTrackCreator.StateInitTool.VeloFitterName = "FastVeloFitLHCbIDs"
      # add to the sequence
      GaudiSequencer("TrackFitSeq").Members.append( bestTrackCreator )

   ### Change dEdx correction for simulated data
   if TrackSys().getProp("Simulation"):
      from Configurables import StateDetailedBetheBlochEnergyCorrectionTool,DetailedMaterialLocator
      if TrackSys().getProp("OldCloneKiller"):
         from Configurables import TrackEventFitter, TrackMasterFitter
         fitBest = TrackEventFitter("FitBest")
         fitBest.Fitter.MaterialLocator.addTool(StateDetailedBetheBlochEnergyCorrectionTool("GeneralDedxTool"))
         fitBest.Fitter.MaterialLocator.GeneralDedxTool.EnergyLossFactor = 0.76
      else:
         from Configurables import TrackBestTrackCreator
         fitter = TrackBestTrackCreator().Fitter
         fitter.MaterialLocator.addTool(StateDetailedBetheBlochEnergyCorrectionTool("GeneralDedxTool"))
         fitter.MaterialLocator.GeneralDedxTool.EnergyLossFactor = 0.76
      
   ## Extra track information sequence
   extraInfos = TrackSys().getProp("TrackExtraInfoAlgorithms")
   if len(extraInfos) > 0 :
      
      track.DetectorList += ["AddExtraInfo"]
      
      ## Clone finding and flagging
      if "CloneFlagging" in extraInfos :
         from Configurables import TrackBuildCloneTable, TrackCloneCleaner
         trackClones = GaudiSequencer("TrackClonesSeq")
         GaudiSequencer("TrackAddExtraInfoSeq").Members += [ trackClones ]
         if TrackSys().timing() :
            trackClones.MeasureTime = True
         cloneTable = TrackBuildCloneTable("FindTrackClones")
         cloneTable.maxDz   = 500*mm
         cloneTable.zStates = [ 0*mm, 990*mm, 9450*mm ]
         cloneTable.klCut   = 5e3
         cloneCleaner = TrackCloneCleaner("FlagTrackClones")
         cloneCleaner.CloneCut = 5e3
         trackClones.Members += [ cloneTable, cloneCleaner ]
         
      ## Add expected hit information   
      #if "ExpectedHits" in extraInfos :
      #   GaudiSequencer("TrackAddExtraInfoSeq").Members += [ TrackComputeExpectedHits() ]
      
      ## Add the likelihood information
      if "TrackLikelihood" in extraInfos and ('TrackLikelihood' not in exclude):
         from Configurables import TrackAddLikelihood, TrackLikelihood 
         trackAddLikelihood = TrackAddLikelihood()
         trackAddLikelihood.addTool( TrackLikelihood, name = "TrackMatching_likTool" )
         trackAddLikelihood.TrackMatching_likTool.otEff = 0.9
         GaudiSequencer("TrackAddExtraInfoSeq").Members += [ trackAddLikelihood ]
         
      ## ghost probability using a Neural Net
      if "GhostProbability" in extraInfos :
         from Configurables import TrackAddNNGhostId, TrackNNGhostId
         nn = TrackAddNNGhostId()
         GaudiSequencer("TrackAddExtraInfoSeq").Members += [ nn ]
         
   track.DetectorList += ["EraseExtraInformation"]
   from Configurables import TrackEraseExtraInfo
   GaudiSequencer("TrackEraseExtraInformationSeq").Members += [ TrackEraseExtraInfo() ]
   
   
   ## Muon alignment tracks
   if "MuonAlign" in trackAlgs :
      from Configurables import TrackEventFitter, AlignMuonRec
      track.DetectorList += ["MuonRec"]
      GaudiSequencer("TrackMuonRecSeq").Members += [ AlignMuonRec("AlignMuonRec"),
                                                     TrackEventFitter("MuonTrackFitter") ]
      importOptions("$TRACKSYSROOT/options/AlignMuonRec.opts")
      if TrackSys().fieldOff():
         AlignMuonRec("AlignMuonRec").BField = False;
         importOptions( "$STDOPTS/DstContentMuonAlign.opts" )

