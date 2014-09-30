# A module to hold the hardcoded names of tracks/protoparticles in the Hlt
# and rules for deriving containers from them  
__author__  = "V. Gligorov vladimir.gligorov@cern.ch"
########################################################################
# Globals
########################################################################
HltGlobalEventPrefix			= ""
########################################################################
# These "globals" define that Tracks go into .../Track/... and
# protoparticles into .../ProtoP/...
HltGlobalTrackLocation 			= "Track"
HltGlobalProtoPLocation			= "ProtoP"
#
########################################################################
# The rules for generating track and proto particle containers
# These rules apply to HLT2 TODO: add rule for HLT1 usable from Hlt1Units
########################################################################
# For tracks, the format is e.g. Hlt2/Track/Unfitted/Forward 
#
# First of all we have the "base" track and protoparticle
# location; this just defines that tracks and protoparticles go into
# some_prefix_you_choose/Track/... and some_refix_you_choose/ProtoP/... 
#
def _baseTrackLocation(prefix,tracks) :
    return prefix + "/" + HltGlobalTrackLocation + "/" + tracks 
#
def _baseProtoPLocation(prefix,protos) :
    return prefix + "/" + HltGlobalProtoPLocation + "/" + protos

########################################################################
# Tracks
########################################################################
# prefixes where to put the tracks (these go into the prefix field of 
# _trackLocation and _protosLocation)
#
HltSharedTracksPrefix 		= HltGlobalEventPrefix + "Hlt"
Hlt1TracksPrefix 			= HltGlobalEventPrefix + "Hlt1"
Hlt2TracksPrefix 			= HltGlobalEventPrefix + "Hlt2"

Hlt1TrackRoot               = Hlt1TracksPrefix + "/" + HltGlobalTrackLocation
Hlt2TrackRoot               = Hlt2TracksPrefix + "/" + HltGlobalTrackLocation
HltSharedTrackRoot          = HltSharedTracksPrefix + "/" + HltGlobalTrackLocation

#
# names of track types (these go into the tracks field of _trackLocation 
# and _protosLocation)
#


HltSharedRZVeloTracksName               = "RZVelo"

TrackName = { "Velo"         : "Velo"          # full Velo recoonstruction
              ,"VeloTTHPT"   : "VeloTTHPT"     # VeloTT for high pt and p thresholds (HLT1-like)
              ,"VeloTTComp"  : "VeloTTComp"    # VeloTT complement (HLT2 relaxed pt & p cuts) 
              ,"VeloTT"      : "VeloTT"        # Full VeloTT (HLT2)
              ,"ForwardHPT"  : "ForwardHPT"    # Forward for high pt and p thresholds (HLT1-like)
              ,"ForwardComp" : "ForwardComp"   # Forward complement
              ,"ForwardCompLPT"  : "ForwardCompLPT"    # low PT complement (aka secondLoop)
              ,"Forward"     : "Forward"       # Full Forward
              ,"Seeding"     : "Seeding"       # Seeding
              ,"ForwardSecondLoop" : "ForwardSecondLoop"
              ,"Match"       : "Match"
              ,"Long"        : "Long"
              ,"Downstream"  : "SeedTT"
              ,"MuonTT"      : "MuonTT"		#tag track for tracking efficiency from Muon and TT hits
              ,"VeloMuon"    : "VeloMuon"	#tag track for tracking efficiency from Velo and Muon hits
              ,"FullDownstream"  : "FullDownstream"	#tag track for tracking efficiency, needs full downstream reco.
              }    

# The prefix says where this track has been produced
Hlt1TrackLoc = { name : _baseTrackLocation(Hlt1TracksPrefix,loc) for name,loc in TrackName.iteritems() }
Hlt2TrackLoc = { name : _baseTrackLocation(Hlt2TracksPrefix,loc) for name,loc in TrackName.iteritems() }
HltSharedTrackLoc = { name : _baseTrackLocation(HltSharedTracksPrefix,loc) for name,loc in TrackName.iteritems() }

# check if the Decoders are writing to the correct locations
from DAQSys.Decoders import DecoderDB
DecoderLocations = DecoderDB["HltTrackReportsDecoder"].listOutputs()
for loc in DecoderLocations :
    if not loc in Hlt1TrackLoc.values() + HltSharedTrackLoc.values()  : 
        print "TrackReports location: " + loc + " not found in track locations. Go synchronize HltTrackNames.py and HltDAQ."
        raise Exception("TrackReports location not found in TrackNames") 
    #else : print "Checked TrackReports location "+loc
    #endif#endloop    

#HltSharedVeloTracksName               = "Velo"
#HltSharedVeloTTTracksName               = "VeloTT"
#HltSharedForwardTracksName               = "Forward"
#
#HltSharedVeloLocation = _baseTrackLocation(HltSharedTracksPrefix,HltSharedVeloTracksName)
#HltSharedVeloTTLocation = _baseTrackLocation(HltSharedTracksPrefix,HltSharedVeloTTTracksName)
#HltSharedForwardLocation = _baseTrackLocation(HltSharedTracksPrefix,HltSharedForwardTracksName)

#
#Hlt1SeedingTracksName                   = "Seeding"

#Hlt1ForwardPestimateTracksName          = "PestiForward"

#
#Hlt2VeloTracksName 			= "Velo"
#Hlt2ForwardTracksName 			= "Forward"
#Hlt2ForwardSecondLoopTracksName = "ForwardSecondLoop"
#Hlt2MatchTracksName 			= "Match"
#Hlt2LongTracksName 			= "Long"
#Hlt2SeedingTracksName 			= Hlt1SeedingTracksName
#Hlt2DownstreamTracksName 		= "SeedTT"
# The next two are "trackified" PID objects
HltMuonTracksName			= "MuonSegments"
HltAllMuonTracksName			= "AllMuonSegments"
#
# types of track fit (including no fit!) (these go into the fastFitType 
# field of _trackLocation and _protosLocation)
#
HltUnfittedTracksSuffix			= "Unfitted"
HltBiDirectionalKalmanFitSuffix 	= "BiKalmanFitted"
HltUniDirectionalKalmanFitSuffix 	= "UniKalmanFitted"  
#
# The recognised track types for the Hlt2 Tracking
#
Hlt2TrackingRecognizedTrackTypes 	= [	"Forward",
                                        "Long", 
                                        "Downstream" ]
#
# The recognised fit types for the Hlt2 Tracking
# 
Hlt2TrackingRecognizedFitTypes		= [	HltUnfittedTracksSuffix,
						HltBiDirectionalKalmanFitSuffix,
						HltUniDirectionalKalmanFitSuffix
					  ]
#
Hlt2TrackingRecognizedFitTypesForRichID = [	HltBiDirectionalKalmanFitSuffix
					  ]	
#




########################################################################
# ProtoParticles
########################################################################
# The suffix (this goes into the type field of _protosLocation)
#
Hlt2ChargedProtoParticleSuffix 		= "Charged"
Hlt2NeutralProtoParticleSuffix 		= "Neutrals" 
#
########################################################################
# PID
########################################################################
# We want to generate PID containers for different track types the
# same way in which we generate tracks, meaning using the same rules,
# because we need different PID for different tracks; they should not 
# overwrite each other!
# 
HltSharedPIDPrefix			= "PID"
HltNoPIDSuffix				= "NoPID"
HltMuonIDSuffix				= "Muon"
HltRICHIDSuffix				= "RICH"
HltCALOIDSuffix				= "CALO"
#
# The default RICH options
#
MaxChi2 = 3.

HltRichDefaultHypos			= ["pion","kaon"]
HltRichDefaultRadiators			= ["Rich1Gas","Rich2Gas"] 
HltRichDefaultTrackCuts     = {"Forward" :{ "Chi2Cut" : [0,MaxChi2], "PCut" : [2,100], "PtCut" : [0.8,100]},
                               "Match" :{ "Chi2Cut" : [0.,MaxChi2], "PCut" : [2,100], "PtCut" : [0.8,100]}}
#
# The default track cuts applied to every track
#

HltDefaultTrackCuts = {"Chi2Cut" : [0.,MaxChi2] }

# And the subdirectories. These are necessary so that different algorithms
# using e.g. different options for the RICH reco (radiators, hypotheses) 
# don't clash with each other 
#
HltCaloProtosSuffix			= "WithCaloID"
HltMuonProtosSuffix			= "WithMuonID"
HltRichProtosSuffix			= "WithRichID"

#
__all__ = (	
		#
		# The strings
		#
		HltGlobalTrackLocation,
		HltGlobalProtoPLocation,
		# 
		HltSharedTracksPrefix, 
           	Hlt1TracksPrefix, 
           	Hlt2TracksPrefix, 
#        HltSharedVeloLocation,
#        HltSharedVeloTTLocation,
#        HltSharedForwardLocation,
		#
        TrackName,
        Hlt1TrackRoot,
        Hlt2TrackRoot,
        HltSharedTrackRoot,
        Hlt1TrackLoc,
        Hlt2TrackLoc,
        HltSharedTrackLoc,
        #
		HltSharedRZVeloTracksName,
#Hlt2ForwardTracksName, 
#		Hlt2ForwardSecondLoopTracksName,
#        Hlt2MatchTracksName, 
#		Hlt2LongTracksName, 
#		Hlt2SeedingTracksName, 
#		Hlt2DownstreamTracksName,
		HltMuonTracksName,
		HltAllMuonTracksName,
		#
		HltUnfittedTracksSuffix, 
		HltBiDirectionalKalmanFitSuffix, 
		HltUniDirectionalKalmanFitSuffix,
		#
		Hlt2TrackingRecognizedTrackTypes,
		Hlt2TrackingRecognizedFitTypes, 
		Hlt2TrackingRecognizedFitTypesForRichID,
		#
		Hlt2ChargedProtoParticleSuffix, 
		Hlt2NeutralProtoParticleSuffix,
		#	
		HltSharedPIDPrefix,
		HltNoPIDSuffix, 
		HltMuonIDSuffix,   
		HltRICHIDSuffix,   
		HltCALOIDSuffix,
		#
		HltRichDefaultHypos,
		HltRichDefaultRadiators,
		#
		HltCaloProtosSuffix,               
		HltMuonProtosSuffix,                     
		HltRichProtosSuffix,                   
		#
		# The functions
		#
		_baseTrackLocation,
		_baseProtoPLocation

	  ) 
