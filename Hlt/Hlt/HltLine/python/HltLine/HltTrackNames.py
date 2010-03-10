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
# Tracks
########################################################################
# prefixes where to put the tracks (these go into the prefix field of 
# _trackLocation and _protosLocation)
#
HltSharedTracksPrefix 			= HltGlobalEventPrefix + "Hlt"
Hlt1TracksPrefix 			= HltGlobalEventPrefix + "Hlt1"
Hlt2TracksPrefix 			= HltGlobalEventPrefix + "Hlt2"
#
# names of track types (these go into the tracks field of _trackLocation 
# and _protosLocation)
#
HltSharedRZVeloTracksName               = "RZVelo"
#
Hlt1SeedingTracksName                   = "Seeding"
#
Hlt2VeloTracksName 			= "Velo"
Hlt2ForwardTracksName 			= "Forward"
Hlt2MatchTracksName 			= "Match"
Hlt2LongTracksName 			= "Long"
Hlt2SeedingTracksName 			= Hlt1SeedingTracksName
Hlt2DownstreamTracksName 		= "SeedTT"
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
Hlt2TrackingRecognizedTrackTypes 	= [	Hlt2ForwardTracksName, 
						Hlt2LongTracksName, 
						Hlt2DownstreamTracksName
					  ]
#
# The recognised fit types for the Hlt2 Tracking
# 
Hlt2TrackingRecognizedFitTypes		= [	HltUnfittedTracksSuffix,
						HltBiDirectionalKalmanFitSuffix,
						HltUniDirectionalKalmanFitSuffix
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
HltMuonIDSuffix				= "Muon"
HltRICHIDSuffix				= "RICH"
HltCALOIDSuffix				= "CALO"
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
		#
		HltSharedRZVeloTracksName,
		Hlt2ForwardTracksName, 
		Hlt2MatchTracksName, 
		Hlt2LongTracksName, 
		Hlt2SeedingTracksName, 
		Hlt2DownstreamTracksName,
		HltMuonTracksName,
		HltAllMuonTracksName,
		#
		HltUnfittedTracksSuffix, 
		HltBiDirectionalKalmanFitSuffix, 
		HltUniDirectionalKalmanFitSuffix,
		#
		Hlt2TrackingRecognizedTrackTypes,
		Hlt2TrackingRecognizedFitTypes, 
		#
		Hlt2ChargedProtoParticleSuffix, 
		Hlt2NeutralProtoParticleSuffix,
		#	
		HltSharedPIDPrefix,
		HltMuonIDSuffix,   
		HltRICHIDSuffix,   
		HltCALOIDSuffix,
		#
		# The functions
		#
		_baseTrackLocation,
		_baseProtoPLocation
	  ) 
