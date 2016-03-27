"""Hlt2 Tracking and PID.

Based on code by P. Koppenburg.
"""

from Gaudi.Configuration import *
from LHCbKernel.Configuration import *
from HltTrackNames import TrackName, Hlt2TrackRoot, Hlt2TrackLoc, Hlt1TrackLoc, HltSharedTrackLoc

from HltTrackNames import Hlt2ChargedProtoParticleSuffix, Hlt2TrackingRecognizedTrackTypes
from HltTrackNames import Hlt2TrackingRecognizedFitTypes
from HltTrackNames import _baseTrackLocation, _baseProtoPLocation
from HltTrackNames import HltMuonTracksName, HltAllMuonTracksName
from HltTrackNames import HltMuonIDSuffix, HltRICHIDSuffix, HltCALOIDSuffix, HltSharedPIDPrefix
from HltTrackNames import (HltNoPIDSuffix, HltAllPIDsSuffix, HltCaloProtosSuffix, HltMuonProtosSuffix,
                           HltRichProtosSuffix, HltAllPIDsProtosSuffix, HltCaloAndMuonProtosSuffix)

from HltTrackNames import HltDefaultFitSuffix
from HltTrackNames import HltGlobalTrackLocation
from HltTrackNames import Hlt2ChargedProtoParticleSuffix, Hlt2NeutralProtoParticleSuffix

from HltRecoConf import HltRichDefaultHypos, HltRichDefaultRadiators
from HltRecoConf import HltRichDefaultTrackCuts
from HltRecoConf import OfflineRichDefaultHypos, OfflineRichDefaultRadiators
from HltRecoConf import OfflineRichDefaultTrackCuts, OfflineRichDefaultDownTrackCuts

from Configurables import CaloProcessor, RichRecSysConf, TrackSelector, TrackSys
from Configurables import ChargedProtoANNPIDConf

__author__ = "V. Gligorov (vladimir.gligorov@cern.ch), S. Stahl (sascha.stahl@cern.ch)"

#################################################################################################
#
# Hlt2 Tracking
#
class Hlt2Tracking(LHCbConfigurableUser):
    # python configurables that I configure
    __used_configurables__ = [
        (CaloProcessor, None),
        (RichRecSysConf, None),
        TrackSys,
        (ChargedProtoANNPIDConf, None),
    ]

    __slots__ = { "DataType"                        : '2012' # datatype  2009, MC09, DC06...
                , "EarlyDataTracking"               : False
                , "Hlt2Tracks"                      : "Long" # type of HLT2 tracks
                , "Prefix"                          : "Hlt2"                 # Why should we need this? It's never changed
                , "FastFitType"                     : HltDefaultFitSuffix
                , "DoSeeding"                       : False  # TODO: Remove not used anymore
                , "DoCleanups"                      : True   # Intended default True. Development options
                , "RestartForward"                  : False  # Intended default False. Development options
                , "CreateBestTracks"                : True  # Intended default True. Development options
                , "UseTrackBestTrackCreator"        : True  # Intended default True. Development options
                                                            # This one disables separate fitting of Hlt2 Forward, Match
                                                            # and Downstream tracks. Leave DoCleanups true for Hlt1 track filtering
                , "NewDownstreamAlg"                : True  # TODO: Remove this option after some grace period
                , "FitTTracks"                      : True
                , "FindV0s"                         : True
                , "RichHypos"                       : HltRichDefaultHypos
                , "RichRadiators"                   : HltRichDefaultRadiators
                , "RichTrackCuts"                   : HltRichDefaultTrackCuts
                , "RichTracklessRingAlgs"           : []
                , "RichFindClusters"                : False
                , "RichPidConfig"                   : "FastGlobal"
                # Flip these three for offline RICH
                , "RichOverrideSafety"              : True
                , "ExtraRichName"                   : ""
                  #, "TrackCuts"                       : HltDefaultTrackCuts # take from HltRecoConf
                , "Hlt2ForwardMaxVelo"              : 0
                # TODO : make these variables, not slots
                , "__hlt2ChargedProtosSeq__"        : 0
                , "__hlt2ChargedCaloProtosSeq__"    : 0
                , "__hlt2ChargedRichProtosSeq__"    : 0
                , "__hlt2ChargedMuonProtosSeq__"    : 0
                , "__hlt2ChargedMuonWithCaloProtosSeq__": 0
                , "__hlt2ChargedAllPIDsProtosSeq__": 0
                , "__hlt2NeutralProtosSeq__"        : 0
                , "__hlt2PhotonsFromL0Seq__"        : 0
                , "__hlt2Pi0FromL0Seq__"            : 0
                , "__hlt2ElectronsFromL0Seq__"      : 0
                , "__hlt2VeloTrackingSeq__"         : 0
                , "__hlt2ForwardTrackingSeq__"      : 0
                , "__hlt2SeedTrackingSeq__"         : 0
                , "__hlt2MatchTrackingSeq__"        : 0
                , "__hlt2DownstreamTrackingSeq__"   : 0
                , "__hlt2TrackingSeq__"             : 0
                , "__hlt2StagedFastFitSeq__"        : 0
                , "__hlt2MuonIDSeq__"               : 0
                , "__hlt2RICHIDSeq__"               : 0
                , "__hlt2CALOIDSeq__"               : 0
                , "__caloProcessor"                 : 0
                , "__allTracks"                     : 0
                }


    #############################################################################################
    #############################################################################################
    #
    # Externally visible methods to return the created bindMembers objects
    #
    #############################################################################################
    #########################################################################################
    #
    # No Charged Protos
    #
    def hlt2ChargedProtos(self, suffix) :
        """
        Charged No PID protoparticles
        """
        # Fill the sequence
        return self.__getSequence("__hlt2ChargedProtosSeq__", self.__hlt2ChargedProtos, suffix)
    #############################################################################################
    #
    # Charged ProtoParticles with no PID information
    #
    def hlt2ChargedNoPIDsProtos(self):
        """
        Charged protoparticles from Calo (=electrons)
        """
        return self.hlt2ChargedProtos(HltNoPIDSuffix)
    #############################################################################################
    #
    # Charged ProtoParticles with CALO ID
    #
    def hlt2ChargedCaloProtos(self):
        """
        Charged protoparticles from Calo (=electrons)
        """
        return self.__getSequence("__hlt2ChargedCaloProtosSeq__", self.__hlt2ChargedCaloProtos)
    #############################################################################################
    #
    # Charged ProtoParticles with RICH ID
    #
    def hlt2ChargedRichProtos(self):
        """
        Charged protoparticles using RICH (=pions,protons,kaons)
        """
        return self.__getSequence("__hlt2ChargedRichProtosSeq__", self.__hlt2ChargedRichProtos)
    #############################################################################################
    #
    # Charged ProtoParticles with Muon ID
    #
    def hlt2ChargedMuonProtos(self):
        """
        Charged protoparticles using Muon (=Muons)
        """
        return self.__getSequence("__hlt2ChargedMuonProtosSeq__", self.__hlt2ChargedMuonProtos)

    #############################################################################################
    #
    # Charged ProtoParticles with Muon and CALO ID
    #
    def hlt2ChargedMuonWithCaloProtos(self):
        """
        Charged protoparticles using Muon (=Muons)
        """
        return self.__getSequence("__hlt2ChargedMuonWithCaloProtosSeq__", self.__hlt2ChargedMuonWithCaloProtos)
    #############################################################################################
    #
    # Charged ProtoParticles with all PID information
    #
    def hlt2ChargedAllPIDsProtos(self):
        """
        Charged protoparticles all PID information
        """
        return self.__getSequence("__hlt2ChargedAllPIDsProtosSeq__", self.__hlt2ChargedAllPIDsProtos)
    #############################################################################################
    #
    # Neutral ProtoParticles
    #
    def hlt2NeutralProtos(self):
        """
        Neutral protoparticles
        """
        return self.__getSequence("__hlt2NeutralProtosSeq__", self.__getCALOSeq, "Neutral")
    #############################################################################################
    #
    # Photons from L0 Candidates
    #
    def hlt2PhotonsFromL0(self):
        """
        Neutral protoparticles
        """
        return self.__getSequence("__hlt2PhotonsFromL0Seq__", self.__getNewCALOSeq, 'photon')
    #############################################################################################
    #
    # Electrons from L0 Candidates
    #
    def hlt2ElectronsFromL0(self):
        """
        Charged protoparticles
        """
        return self.__getSequence("__hlt2ElectronsFromL0Seq__", self.__hlt2ElectronsFromL0)
    #############################################################################################
    #
    # Pi0 from L0 Candidates
    #
    def hlt2Pi0FromL0(self):
        """
        Neutral protoparticles
        """
        return self.__getSequence("__hlt2Pi0FromL0Seq__", self.__getNewCALOSeq, 'pi0')
    #############################################################################################
    #
    # Velo tracking for the PV making sequence
    #
    def hlt2VeloTracking(self):
        """
        Velo tracks
        """
        return self.__getSequence("__hlt2VeloTrackingSeq__", self.__hlt2VeloTracking)
    #############################################################################################
    #
    # Forward tracking
    #
    def hlt2ForwardTracking(self):
        """
        Forward tracks
        """
        return self.__getSequence("__hlt2ForwardTrackingSeq__", self.__hlt2ForwardTracking)
    #############################################################################################
    #
    # Seed tracking
    #
    def hlt2SeedTracking(self):
        """
        Seed tracks
        """
        return self.__getSequence("__hlt2SeedTrackingSeq__", self.__hlt2SeedTracking)
    #############################################################################################
    #
    # Match tracking
    #
    def hlt2MatchTracking(self):
        """
        Match tracks
        """
        return self.__getSequence("__hlt2MatchTrackingSeq__", self.__hlt2MatchTracking)
    #############################################################################################
    #
    # Downstream tracking
    #
    def hlt2DownstreamTracking(self):
        """
        Downstream tracks
        """
        return self.__getSequence("__hlt2DownstreamTrackingSeq__", self.__hlt2DownstreamTracking)
    #############################################################################################
    #
    # Full tracking sequence
    #
    def hlt2Tracking(self):
        """
        Velo tracks
        """
        return self.__getSequence("__hlt2TrackingSeq__", self.__hlt2Tracking)
    #############################################################################################
    #
    # Full tracking sequence
    #
    def hlt2StagedFastFit(self):
        """
        Fitted Tracks
        """
        return self.__getSequence("__hlt2StagedFastFitSeq__", self.__hlt2StagedFastFit)
    #############################################################################################
    #
    # The Muon ID sequence: makes the tracks and Muon IDs them
    #
    def hlt2MuonID(self):
        """
        Muon PID
        """
        return self.__getSequence("__hlt2MuonIDSeq__", self.__hlt2MuonID)
    #############################################################################################
    #
    # The RICH ID sequence: makes the tracks and RICH IDs them
    #
    def hlt2RICHID(self):
        """
        RICH PID
        """
        return self.__getSequence("__hlt2RICHIDSeq__", self.__hlt2RICHID)
    #############################################################################################
    #
    # The CALO ID: makes the tracks and CALO IDs them
    #
    def hlt2CALOID(self):
        """
        CALO PID
        """
        return self.__getSequence("__hlt2CALOIDSeq__", self.__hlt2CALOID)
    #############################################################################################
    #
    # Track preparation
    #
    # This will automatically do the required tracking and then, if required
    # do the specified fast-fit as well; this is the only one of the tracking sequences
    # whose output is used in the protoparticle making
    #
    def hlt2PrepareTracks(self):
        """
        The staged fast fit
        """
        return self.hlt2StagedFastFit()
    #############################################################################################
    #
    # Return all track locations that are filled
    #
    def allTrackLocations(self):
        """
        All track locations
        """
        return self.__allTracks
    #############################################################################################
    #
    # The apply configuration method, makes the bindMembers objects which can then
    # be exported, and also checks that the configuration is not inconsistent
    #
    #############################################################################################
    #############################################################################################
    def __apply_configuration__(self) :
        log.debug('############################################################')
        log.debug('## INFO You have configured an instance of the Hlt2 tracking')
        log.debug('## INFO ----------------------------------------------------')
        log.debug('## INFO The data type is %s'     % self.getProp("DataType"       ))
        log.debug('## INFO Early data tuning? %s'   % self.getProp("EarlyDataTracking"))
        log.debug('## INFO Tracks to make are %s'   % self.getProp("Hlt2Tracks"     ))
        #log.debug('## INFO The prefix is %s'        % self.getProp("Prefix"         ))
        log.debug('## INFO The fit type is %s'      % self.getProp("FastFitType"    ))
        log.debug('## INFO Seeding? = %s'           % self.getProp("DoSeeding"      ))
        log.debug('## INFO Rich hypos = %s'         % self.getProp("RichHypos"      ))
        log.debug('## INFO Rich radiators = %s'     % self.getProp("RichRadiators"  ))
        log.debug('#################################%##########################')
        #
        # First of all check that I have been called with a sensible set of options
        #
        outputOfHlt2Tracking = self.__trackType()
        if (outputOfHlt2Tracking == 'Unknown') :
            self.__fatalErrorUnknownTrackType()
            return []
        #
        if (self.getProp("FastFitType") not in Hlt2TrackingRecognizedFitTypes) :
            self.__fatalErrorUnknownTrackFitType()
            return []
        if (self.getProp("UseTrackBestTrackCreator") and not self.getProp("CreateBestTracks")):
            log.fatal( '#############################################################')
            log.fatal( 'Set CreateBestTracks to true if UseTrackBestTrackCreator true')
            return []

    #############################################################################################
    #############################################################################################
    #
    # Actual implementation of the functions
    #
    #############################################################################################
    #
    # Start with the naming conventions
    #
    # The track type for the derived tracks
    # This function checks that the track asked for by the Hlt2Tracking instance
    # is in the recognised track types, and returns "Unknown" or the correct
    # suffix based on the configuration of the Hlt2Tracking.
    #
    def __getSequence(self, prop, fun, *args):
        if args:
            if self.isPropertySet(prop):
                k = tuple(args)
                p = self.getProp(prop)
                if k not in p:
                    p[k] = fun(*args)
                    self.setProp(prop, p)
                return p[k]
            else:
                r = fun(*args)
                self.setProp(prop, {tuple(args) : r})
                return r
        else:
            if not self.isPropertySet(prop):
                self.setProp(prop, fun())
            return self.getProp(prop)

    def __trackType(self) :
        if ( self.getProp("Hlt2Tracks") not in Hlt2TrackingRecognizedTrackTypes) :
            return "Unknown"
        elif ( self.getProp("Hlt2Tracks") == "Long") :
            if self.getProp("DoSeeding") :
                return "Long"
            else :
                self.__warningAskLongGetForward()
                return "Long"
        elif ( self.getProp("Hlt2Tracks") == "Forward") :
            if self.getProp("DoSeeding") :
                self.__warningAskForwardGetLong()
                return "Long"
            else :
                return "Long"
        elif ( self.getProp("Hlt2Tracks") == "Downstream") :
            return "Downstream"

    def trackType(self) :
        return self.__trackType()
    #
    # Now the "long" track location, for the tracks which will be used to
    # make particles, protoparticles, etc. baseTrack/ProtoPLocation live in
    # HltTrackNames.py   WE NEED TO REWORK THIS SHIT. GOD DAMMIT
    #

    def __trackLocationByType(self, type):
        thisTrackLocation     = self.getProp("FastFitType") + "/" + \
                                TrackName[type]
        return Hlt2TrackRoot + thisTrackLocation

    def __trackLocation(self):
        return self.__trackLocationByType(self.__trackType())

    def trackLocation(self):
        return self.__trackLocation();


    ## For protos, the format is e.g. Hlt2/ProtoP/Unfitted/Forward/Charged #
    def __protosLocation(self,protosType):
        thisProtosLocation = self.getProp("FastFitType") + "/" + \
                             self.__trackType() + "/" + \
                             protosType
        return _baseProtoPLocation(self.getProp("Prefix"), thisProtosLocation)
    #
    # The hadron protos need subdirectories depending on the type of PID requested
    #
    def __chargedRichProtosSuffix(self):
        baseSuffix = HltRichProtosSuffix+"/"
        baseSuffix += HltRICHIDSuffix + "/" + self.__richIDSuffix()
        return baseSuffix

    #
    # The trackified Muon ID location
    #
    def _trackifiedMuonIDLocation(self) :
        return self.__hltBasePIDLocation() + "/"+ HltMuonTracksName

    #
    # The trackified AllMuon ID location
    #
    def _trackifiedAllMuonIDLocation(self) :
        return self.__hltBasePIDLocation() + "/"+ HltAllMuonTracksName


    #
    # The PID objects themselves
    #
    def __hltBasePIDLocation(self) :
        return self.__trackLocation() + "/" + HltSharedPIDPrefix
    #
    def __muonIDLocation(self) :
        #muonBase = self.__hltBasePIDLocation() + "/" + HltMuonIDSuffix
        # This is now the same for all Hlt2Tracking instances
        muonBase = self.getProp("Prefix")+"/"+HltSharedPIDPrefix+"/"+HltMuonIDSuffix
        return muonBase

    #
    def __richIDLocation(self) :
        return self.__hltBasePIDLocation() + "/" + HltRICHIDSuffix + "/" + self.__richIDSuffix()
    #
    def __richIDSuffix(self) :
        baseSuffix = self.getProp("ExtraRichName")
        for hypo in self.getProp("RichHypos") :
            baseSuffix += hypo
        baseSuffix += "/"
        for radiator in self.getProp("RichRadiators") :
            baseSuffix += radiator
        return baseSuffix
    #
    def __caloIDLocation(self) :
        # This is now the same for all Hlt2Tracking instances
        #caloBase =  self.__hltBasePIDLocation() + "/" + HltCALOIDSuffix
        caloBase = self.getProp("Prefix")+"/"+HltSharedPIDPrefix+"/"+HltCALOIDSuffix
        return caloBase
    def __caloL0IDLocation(self) :
        caloBase =  self.__hltBasePIDLocation() + "/L0Calo" + HltCALOIDSuffix
        return caloBase
    #
    # The prefixes for the various tools and algorithms used
    #
    def __trackingAlgosAndToolsPrefix(self) :
        return self.getProp("Prefix") + self.__trackType()
    #
    def __trackfitAlgosAndToolsPrefix(self) :
        return self.getProp("Prefix") + self.getProp("FastFitType") + self.__trackType()
    #
    def __pidAlgosAndToolsPrefix(self) :
        return self.name()
    # Explicitly tell users about the new long/forward naming convention: long tracks are the
    # forward+match tracks (seeding required), forward tracks are just PatForward, no seeding.
    # Since we are not bastards, if someone asks for forward tracks and turns the seeding on we
    # will give them long tracks, but in this case we should print a warning to tell them what
    # happened.
    #
    def __warningAskForwardGetLong(self) :
        log.warning( '##################################################################################')
        log.warning( '## WARNING You asked for Forward tracks with seeding on. I will assume that you   ')
        log.warning( '## WARNING would like both forward and match tracks, and I will put the combined  ')
        log.warning( '## WARNING output of these into the long track container. You can still retrieve  ')
        log.warning( '## WARNING the forward tracks alone by asking for the forward track container.    ')
        log.warning( '##################################################################################')
    #
    def __warningAskLongGetForward(self) :
        log.warning( '##################################################################################')
        log.warning( '## WARNING You asked for long tracks with seeding off. I will assume that you     ')
        log.warning( '## WARNING would like only forward tracks, which will be placed in the forward    ')
        log.warning( '## WARNING forward track container. The long track container will be left empty.  ')
        log.warning( '##################################################################################')
    #
    # In case something went wrong when specifying the options, warn me before you die
    #
    def __fatalErrorUnknownTrackType(self) :
        log.fatal( '##################################################################################')
        log.fatal( '## FATAL You specified an unknown track type %s for the Hlt2 Reconstruction' % self.getProp("Hlt2Tracks") )
        log.fatal( '## FATAL I will now die, you need to make Hlt2Tracking aware of this track type!  ')
        log.fatal( '##################################################################################')
    #
    def __fatalErrorUnknownTrackFitType(self) :
        log.fatal( '################################################################################')
        log.fatal( '## FATAL You specified an unknown fit type %s for the Hlt2 Reconstruction' % self.getProp("FastFitType") )
        log.fatal( '## FATAL I will now die, you need to make Hlt2Tracking aware of this fit type!  ')
        log.fatal( '################################################################################')

    #########################################################################################
    #
    # Electron Protos
    #
    def __hlt2ChargedCaloProtos( self ) :
        """
        Charged Calo protoparticles = electrons
        """

        #
        # The charged protoparticles and their output location
        #
        allTracks                   = self.hlt2Tracking()
        tracks                      = self.hlt2StagedFastFit()
        chargedProtos               = self.hlt2ChargedProtos(HltCaloProtosSuffix)
        chargedProtosOutputLocation = chargedProtos.outputSelection()

        doCaloReco              = self.hlt2CALOID()

        #
        # Add the Calo info to the DLL
        #
        from Configurables import ( ChargedProtoParticleAddEcalInfo,
                                    ChargedProtoParticleAddBremInfo,
                                    ChargedProtoParticleAddHcalInfo,
                                    ChargedProtoParticleAddPrsInfo,
                                    ChargedProtoParticleAddSpdInfo
                                    )
        from Configurables import GaudiSequencer
        addCaloInfo = GaudiSequencer( self.__pidAlgosAndToolsPrefix() + HltCaloProtosSuffix + "Sequence")
        caloPidLocation = self.__caloIDLocation()
        prefix = self.__pidAlgosAndToolsPrefix()
        suffix =  HltCaloProtosSuffix
        ecal = ChargedProtoParticleAddEcalInfo(prefix+"ChargedProtoPAddEcal"+suffix)
        brem = ChargedProtoParticleAddBremInfo(prefix+"ChargedProtoPAddBrem"+suffix)
        hcal = ChargedProtoParticleAddHcalInfo(prefix+"ChargedProtoPAddHcal"+suffix)
        prs  = ChargedProtoParticleAddPrsInfo (prefix+"ChargedProtoPAddPrs"+suffix )
        spd  = ChargedProtoParticleAddSpdInfo (prefix+"ChargedProtoPAddSpd"+suffix )
        for alg in (ecal,brem,hcal,prs,spd):
            alg.setProp("ProtoParticleLocation",chargedProtosOutputLocation)
            alg.setProp("Context",caloPidLocation)
            addCaloInfo.Members += [ alg  ]
        # For the charged we need a combined DLL
        from Configurables import ChargedProtoCombineDLLsAlg
        combine_name                    = self.__pidAlgosAndToolsPrefix()+"ChargedCaloProtoPCombDLLs"
        combine                         = ChargedProtoCombineDLLsAlg(combine_name)
        combine.ProtoParticleLocation   = chargedProtosOutputLocation

        sequenceToReturn  = [ allTracks, tracks, chargedProtos]
        sequenceToReturn += [ doCaloReco ]
        sequenceToReturn += [ addCaloInfo ]
        sequenceToReturn += [ combine ]

        from HltLine.HltLine import bindMembers
        # Build the bindMembers
        bm_name         = self.__pidAlgosAndToolsPrefix()+"ChargedCaloProtosSeq"
        bm_members      = sequenceToReturn
        bm_output       = chargedProtosOutputLocation

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

    #########################################################################################
    #
    # Muons with CALO ID added, these seem to be obsolete and probably don't work
    #
    def __hlt2ChargedMuonWithCaloProtos(self) :
        """
        Muons which have the CALO PID information filled for them
        """
        #
        # The charged protoparticles and their output location
        #
        allTracks                   = self.hlt2Tracking()
        tracks                      = self.hlt2StagedFastFit()
        chargedProtos               = self.hlt2ChargedProtos(HltCaloAndMuonProtosSuffix)
        chargedProtosOutputLocation = chargedProtos.outputSelection()
        doCaloReco              = self.hlt2CALOID()

        #
        # Add the Calo info to the DLL
        # TODO: This is now used several times, create an own function
        from Configurables import ( ChargedProtoParticleAddEcalInfo,
                                    ChargedProtoParticleAddBremInfo,
                                    ChargedProtoParticleAddHcalInfo,
                                    ChargedProtoParticleAddPrsInfo,
                                    ChargedProtoParticleAddSpdInfo
                                    )
        from Configurables import GaudiSequencer
        addCaloInfo = GaudiSequencer( self.__pidAlgosAndToolsPrefix() + HltCaloAndMuonProtosSuffix + "Sequence")
        caloPidLocation = self.__caloIDLocation()
        prefix = self.__pidAlgosAndToolsPrefix()
        suffix =   HltCaloAndMuonProtosSuffix
        ecal = ChargedProtoParticleAddEcalInfo(prefix+"ChargedProtoPAddEcal"+suffix)
        brem = ChargedProtoParticleAddBremInfo(prefix+"ChargedProtoPAddBrem"+suffix)
        hcal = ChargedProtoParticleAddHcalInfo(prefix+"ChargedProtoPAddHcal"+suffix)
        prs  = ChargedProtoParticleAddPrsInfo (prefix+"ChargedProtoPAddPrs"+suffix )
        spd  = ChargedProtoParticleAddSpdInfo (prefix+"ChargedProtoPAddSpd"+suffix )
        for alg in (ecal,brem,hcal,prs,spd):
            alg.setProp("ProtoParticleLocation",chargedProtosOutputLocation)
            alg.setProp("Context",caloPidLocation)
            addCaloInfo.Members += [ alg  ]

        from Configurables import ChargedProtoParticleAddMuonInfo

        # Now add the muon information
        # The muon ID
        muonID                          = self.hlt2MuonID()
        muon_name                       = self.__pidAlgosAndToolsPrefix()+HltCaloAndMuonProtosSuffix
        muon                            = ChargedProtoParticleAddMuonInfo(muon_name)
        muon.ProtoParticleLocation      = chargedProtosOutputLocation
        muon.InputMuonPIDLocation       = muonID.outputSelection()


        # For the charged we need a combined DLL
        from Configurables import ChargedProtoCombineDLLsAlg
        combine_name                    = self.__pidAlgosAndToolsPrefix()+HltCaloAndMuonProtosSuffix+ "CombDLLs"
        combine                         = ChargedProtoCombineDLLsAlg(combine_name)
        combine.ProtoParticleLocation   = chargedProtosOutputLocation

        sequenceToReturn  = [ allTracks, tracks, chargedProtos]
        sequenceToReturn += [ doCaloReco, addCaloInfo]
        sequenceToReturn += [ muonID, muon ]
        sequenceToReturn += [ combine ]

        from HltLine.HltLine import bindMembers

        # Build the bindMembers
        bm_name         = self.__pidAlgosAndToolsPrefix()+"MwCSeq"
        bm_members      = sequenceToReturn
        bm_output       = chargedProtosOutputLocation

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)


    #########################################################################################
    #
    # Muon Protos
    #
    def __hlt2ChargedMuonProtos(self):
        """
        Charged muon protoparticles
        Requires chargedProtos and muon ID
        """
        from Configurables import ( ChargedProtoParticleAddMuonInfo,
                                    ChargedProtoCombineDLLsAlg )
        #The different add PID algorithms
        #
        # The charged protoparticles and their output location
        chargedProtos                   = self.hlt2ChargedProtos(HltMuonProtosSuffix)
        chargedProtosOutputLocation     = chargedProtos.outputSelection()
        #
        muon_name           = self.__pidAlgosAndToolsPrefix()+"ChargedProtoPAddMuon"

        muon                = ChargedProtoParticleAddMuonInfo(muon_name)
        muon.ProtoParticleLocation  = chargedProtosOutputLocation
        # Get the MuonID from the MuonID sequence
        muonID                      = self.hlt2MuonID()
        muon.InputMuonPIDLocation   = muonID.outputSelection()

        from HltLine.HltLine import bindMembers
        # Build the bindMembers

        bm_name         = self.__pidAlgosAndToolsPrefix()+"ChargedMuonProtosSeq"

        bm_members      = [ muonID, chargedProtos, muon ]
        bm_output       = chargedProtosOutputLocation

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)
    #########################################################################################
    #
    # Charged hadron ProtoParticles
    # Does not necessarily use RICH or CALO but can do if so configured
    #
    def __hlt2ChargedRichProtos(self):
        """
        Charged hadron protoparticles = pion, kaon, proton
        If RICH is on, then requires rich (this is off by default)
        """
        from Configurables import ( ChargedProtoParticleAddRichInfo,
                                    ChargedProtoCombineDLLsAlg)
        #The different add PID algorithms
        #
        # The charged protoparticles and their output location
        #
        chargedProtos                   = self.hlt2ChargedProtos(self.__chargedRichProtosSuffix())
        chargedProtosOutputLocation     = chargedProtos.outputSelection()
        #
        # Now set up the RICH sequence
        #
        doRICHReco              = self.hlt2RICHID()
        #
        # Add the RICH info to the DLL
        #
        richDLL_name            = self.__pidAlgosAndToolsPrefix()+"ChargedHadronProtoPAddRich"
        richDLL                 = ChargedProtoParticleAddRichInfo(richDLL_name)
        richDLL.InputRichPIDLocation    = doRICHReco.outputSelection()
        richDLL.ProtoParticleLocation   = chargedProtosOutputLocation
        #
        # What are we returning?
        #
        sequenceToReturn = []
        sequenceToReturn += [chargedProtos]
        sequenceToReturn += [doRICHReco]
        sequenceToReturn += [richDLL]
        #
        # The combined DLL is needed for cutting on later
        #
        from Configurables import ChargedProtoCombineDLLsAlg
        combine_name                    = self.__pidAlgosAndToolsPrefix()+"RichCombDLLs"
        combine                         = ChargedProtoCombineDLLsAlg(combine_name)
        combine.ProtoParticleLocation   = chargedProtosOutputLocation

        sequenceToReturn += [combine]

        from HltLine.HltLine import bindMembers
        # Build the bindMembers
        bm_name         = self.__pidAlgosAndToolsPrefix()+"ChargedRichProtosSeq"
        bm_members      = sequenceToReturn
        bm_output       = chargedProtosOutputLocation

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

    #########################################################################################
    #
    # Charged ProtoParticles with all pid information
    #
    #
    def __hlt2ChargedAllPIDsProtos(self):
        """
        Charged hadron protoparticles = pion, kaon, proton
        If RICH is on, then requires rich (this is off by default)
        """
        #
        # The charged protoparticles and their output location
        #
        allTracks                   = self.hlt2Tracking()
        tracks                      = self.hlt2StagedFastFit()
        chargedProtos               = self.hlt2ChargedProtos(HltAllPIDsProtosSuffix)
        chargedProtosOutputLocation = chargedProtos.outputSelection()

        from Configurables import ( ChargedProtoParticleAddRichInfo,ChargedProtoParticleAddMuonInfo,
                                    ChargedProtoCombineDLLsAlg)

        #
        # Set up the pid sequence
        #
        doRICHReco              = self.hlt2RICHID()
        muonID                  = self.hlt2MuonID()
        doCaloReco              = self.hlt2CALOID()

        #
        # Add the rich info to the DLL
        #
        richDLL_name                    = self.__pidAlgosAndToolsPrefix()+"ChargedHadronProtoPAddRich"+HltAllPIDsSuffix
        richDLL                         = ChargedProtoParticleAddRichInfo(richDLL_name)
        richDLL.InputRichPIDLocation    = doRICHReco.outputSelection()
        richDLL.ProtoParticleLocation   = chargedProtosOutputLocation
        #
        # Add the muon info to the DLL
        #
        muon_name                   = self.__pidAlgosAndToolsPrefix()+"ChargedProtoPAddMuon"+HltAllPIDsSuffix
        muon                        = ChargedProtoParticleAddMuonInfo(muon_name)
        muon.ProtoParticleLocation  = chargedProtosOutputLocation
        muon.InputMuonPIDLocation   = muonID.outputSelection()

        #
        # Add the Calo info to the DLL
        #
        from Configurables import ( ChargedProtoParticleAddEcalInfo,
                                    ChargedProtoParticleAddBremInfo,
                                    ChargedProtoParticleAddHcalInfo,
                                    ChargedProtoParticleAddPrsInfo,
                                    ChargedProtoParticleAddSpdInfo
                                    )
        from Configurables import GaudiSequencer
        addCaloInfo = GaudiSequencer( self.__pidAlgosAndToolsPrefix() + HltAllPIDsSuffix + "Sequence")
        caloPidLocation = self.__caloIDLocation()
        prefix = self.__pidAlgosAndToolsPrefix()
        suffix =  HltAllPIDsSuffix
        ecal = ChargedProtoParticleAddEcalInfo(prefix+"ChargedProtoPAddEcal"+suffix)
        brem = ChargedProtoParticleAddBremInfo(prefix+"ChargedProtoPAddBrem"+suffix)
        hcal = ChargedProtoParticleAddHcalInfo(prefix+"ChargedProtoPAddHcal"+suffix)
        prs  = ChargedProtoParticleAddPrsInfo (prefix+"ChargedProtoPAddPrs"+suffix )
        spd  = ChargedProtoParticleAddSpdInfo (prefix+"ChargedProtoPAddSpd"+suffix )
        for alg in (ecal,brem,hcal,prs,spd):
            alg.setProp("ProtoParticleLocation",chargedProtosOutputLocation)
            alg.setProp("Context",caloPidLocation)
            addCaloInfo.Members += [ alg  ]


        # Combine the DLLs
        from Configurables import ChargedProtoCombineDLLsAlg
        combine_name                    = self.__pidAlgosAndToolsPrefix()+"CombDLLs"+HltAllPIDsSuffix
        combine                         = ChargedProtoCombineDLLsAlg(combine_name)
        combine.ProtoParticleLocation   = chargedProtosOutputLocation


        #
        # What are we returning?
        #
        sequenceToReturn  = [ allTracks, tracks, chargedProtos]
        sequenceToReturn += [ doRICHReco, muonID, doCaloReco ]
        sequenceToReturn += [ richDLL, muon , addCaloInfo ]
        sequenceToReturn += [combine]

        # Calculate ProbNN variables
        from Configurables import HltRecoConf
        if HltRecoConf().getProp("CalculateProbNN") == True:
            probNNSeqName         = self._instanceName(ChargedProtoANNPIDConf)
            probNNSeq             = GaudiSequencer(probNNSeqName+"Seq")
            annconf = ChargedProtoANNPIDConf(probNNSeqName)
            annconf.DataType = self.getProp( "DataType" )
            annconf.TrackTypes              = [ self.__trackType() ]
            annconf.RecoSequencer = probNNSeq
            annconf.ProtoParticlesLocation = chargedProtosOutputLocation
            sequenceToReturn += [probNNSeq]

        from HltLine.HltLine import bindMembers
        # Build the bindMembers
        bm_name          = self.__pidAlgosAndToolsPrefix()+"ChargedAllPIDsProtosSeq"
        bm_members       = sequenceToReturn
        bm_output        = chargedProtosOutputLocation
        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

    #########################################################################################
    #
    # Charged ProtoParticles
    #
    def __hlt2ChargedProtos(self,outputLocation):
        """
        Charged protoparticles

        Requires tracks, fitted if necessary
        """
        from Configurables import ChargedProtoParticleMaker
        from HltLine.HltLine import bindMembers
        # Build the bindMembers
        chargedProtosOutputLocation = self.__protosLocation(Hlt2ChargedProtoParticleSuffix) + "/" + outputLocation
        charged_name = self.__pidAlgosAndToolsPrefix()+outputLocation.replace("/", "")+'ChargedProtoPAlg'
        charged      = ChargedProtoParticleMaker(charged_name)
        charged.TrackSelectorType = "TrackSelector"
        charged.addTool(TrackSelector, name="TrackSelector")
        ts=charged.TrackSelector
        from Configurables import HltRecoConf
        ts.setProp("MinChi2Cut", 0.)
        ts.setProp("MaxChi2Cut", HltRecoConf().getProp("MaxTrCHI2PDOF"))
        if HltRecoConf().getProp("ApplyGHOSTPROBCut") == True:
            ts.setProp("MinGhostProbCut", 0.)
            ts.setProp("MaxGhostProbCut", HltRecoConf().getProp("MaxTrGHOSTPROB"))

        if self.__trackType() == "Long":
            ts.TrackTypes = ["Long"]
        elif self.__trackType() == "Downstream":
            ts.TrackTypes = ["Downstream"]

        # Need to allow for fitted tracks
        # This is now done inside the staged fast fit based on the fastFitType passed
        tracks          = self.hlt2StagedFastFit()
        charged.Inputs  = [tracks.outputSelection()]
        charged.Output  = chargedProtosOutputLocation

        bm_name         = self.__pidAlgosAndToolsPrefix() + outputLocation.replace("/", "") + "ChargedProtosSeq"
        bm_members      = [tracks, charged]
        bm_output       = chargedProtosOutputLocation
        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)


    #########################################################################################
    #
    # Photons built from L0
    #
    def __hlt2ElectronsFromL0(self):
        """
        Photons coming from L0
        """
        # Configure CaloElectron
        caloElectronsFromL0 = self.__getNewCALOSeq('electron')
        from Configurables import ChargedProtoCombineDLLsAlg
        combine_name                    = self.__pidAlgosAndToolsPrefix()+"LowEtChargedCaloProtoPCombDLLs"
        combine                         = ChargedProtoCombineDLLsAlg(combine_name)
        combine.ProtoParticleLocation   = caloElectronsFromL0.outputSelection()

        from HltLine.HltLine import bindMembers
        # Build the bindMembers
        bm_name         = self.__pidAlgosAndToolsPrefix()+"LowEtChargedCaloProtosSeq"
        bm_members      = [caloElectronsFromL0, combine]
        bm_output       = caloElectronsFromL0.outputSelection()

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

    #########################################################################################
    #
    # MuonID
    #
    def __hlt2MuonID(self) :
        """
        Muon ID options
        Requires tracks
        """

        from MuonID import ConfiguredMuonIDs
        from Configurables import MuonRec, MuonIDAlgLite
        from HltLine.HltLine import bindMembers

        cm = ConfiguredMuonIDs.ConfiguredMuonIDs(data=self.getProp("DataType"))

        #HltMuonIDAlg_name   = self.__pidAlgosAndToolsPrefix()+"MuonIDAlgLite"
        HltMuonIDAlg = cm.configureMuonIDAlgLite(self.getProp("Prefix") + "MuonIDAlgLite")

        # Configure moun ID tools explicitly, would be better if the ConfiguredMuonIDs class
        # provided a comprehensive method. All tools are public, but should configure
        # everywhere, where they are used to be safe.
        import Configurables
        for tool, fun in (("CommonMuonTool" ,"IsMuonTool"),
                     ("DLLMuonTool", "DLLMuonTool"),
                     ("MakeMuonTool", "MakeMuonTool")):
            tool = getattr(Configurables, tool)()
            getattr(cm, "configure" + fun)(tool)

        #The tracks to use
        #tracks              = self.__hlt2StagedFastFit()
        # We want all tracks
        tracks              = self.hlt2Tracking()
        #Enforce naming conventions
        HltMuonIDAlg.TracksLocations        = tracks.outputSelection()
        HltMuonIDAlg.MuonIDLocation         = self.__muonIDLocation() #output
        HltMuonIDAlg.MuonTrackLocation      = self._trackifiedMuonIDLocation()
        #HltMuonIDAlg.MuonTrackLocationAll   = self._trackifiedAllMuonIDLocation()
        # CRJ : Disable FindQuality in HLT since it increases CPU time for MuonID by
        #       a factor 3-4
        # TODO: Do we want to keep this? Which difference does it make.
        #HltMuonIDAlg.FindQuality            = False

        # Build the bindMembers
        bm_name         = self.__pidAlgosAndToolsPrefix()+"MuonIDSeq"
        bm_members      = [ tracks, MuonRec(), HltMuonIDAlg ]
        bm_output       = HltMuonIDAlg.MuonIDLocation

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

    #########################################################################################
    #
    # Calo ID
    #
    def __hlt2CALOID(self) :
        """
        Calo ID
        Does calo reconstruction and Calo ID
        """
        # the sequence to run
        myCALOProcessorPIDSeq   = self.__getCALOSeq("PID")

        return myCALOProcessorPIDSeq
    #########################################################################################
    #
    # RICH ID
    #
    def __hlt2RICHID(self) :
        """
        Set up the sequence for doing the RICH PID on the tracks
        """
        from Configurables import HltRecoConf
        if HltRecoConf().getProp("OfflineRich"):
            if (self.__trackType() == "Long"):
                self.setProp("RichTrackCuts", OfflineRichDefaultTrackCuts)
            if (self.__trackType() == "Downstream"):
                self.setProp("RichTrackCuts", OfflineRichDefaultDownTrackCuts)
            self.setProp("RichHypos",  OfflineRichDefaultHypos)
            self.setProp("RichPidConfig", "FullGlobal")
            self.setProp("RichTracklessRingAlgs", [])
            self.setProp("RichFindClusters", False)

        from HltLine.HltLine import bindMembers
        from Configurables import RichRecSysConf
        #configure the rich reco
        # first give it a nice name, in this case constructed
        # from the name of our configurable and the RichRecoSys...
        richSeqName         = self._instanceName(RichRecSysConf)
        richSeq             = GaudiSequencer(richSeqName+"Seq")
        # richSeq.MeasureTime = True
        # The RICH COnfigurable
        # note, the context MUST contain the string HLT
        # in whatever case
        richConf            = RichRecSysConf(richSeqName)
        richConf.DataType   = self.getProp( "DataType" )
        richConf.Context    = richSeqName
        richConf.RichPIDLocation    = self.__richIDLocation()+self.__trackType()
        richConf.PidConfig          = self.getProp("RichPidConfig")
        richConf.CheckProcStatus    = False
        richConf.InitPixels         = True
        richConf.InitTracks         = True
        richConf.InitPhotons        = True
        richConf.TracklessRingAlgs  = self.getProp("RichTracklessRingAlgs")
        richConf.Particles          = self.getProp("RichHypos")
        richConf.pixelConfig().FindClusters = self.getProp("RichFindClusters")
        # Set cuts on which tracks enter the RICH reco
        richConf.trackConfig().TrackCuts = self.getProp("RichTrackCuts")
        richConf.gpidConfig().TrackCuts  = self.getProp("RichTrackCuts")
        richConf.Radiators          = self.getProp("RichRadiators")
        # Set the sequence to run the RICH PID in
        richConf.setProp("RecoSequencer",richSeq)
        # The input location of the tracks
        tracks                      = self.hlt2StagedFastFit()
        richConf.trackConfig().InputTracksLocation = tracks.outputSelection()

        # Workaround for Rich dictionary loading.
        #import cppyy
        #cppyy.gbl.LHCb.RichRecSegment

        from HltLine.HltLine import bindMembers
        # Build the bindMembers
        bm_name         = self.__pidAlgosAndToolsPrefix()+"RICHIDSeq"
        bm_members      = [tracks, richSeq]
        bm_output       = richConf.RichPIDLocation

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)
    #########################################################################################
    #
    # Staged fast fit
    #
    def __hlt2StagedFastFit(self) :
        """
        The staged fast-fit. Currently set to
        be bidirectional and with smoothing to allow PID information
        to be used; special cases need to be justified separately
        """
        from HltLine.HltLine import bindMembers

        #Make the original tracks in case this has not been run already
        tracks                  = self.hlt2Tracking()

        #And my output location is?
        hlt2StagedFastFitOutputLocation = self.__trackLocation()

        from Configurables import TrackEventFitter, TrackMasterFitter
        bm_members=[]
        if ((self.getProp("DoCleanups") and self.__trackType() == "Long") or self.getProp("CreateBestTracks")):
            # We have done everything, only the location of the current tracking configuration is propagated
            hlt2StagedFastFitOutputLocation = self.__trackLocation()
            bm_members      = [ tracks ]
        else:
            Hlt2StagedFastFit_name      = self.__trackfitAlgosAndToolsPrefix()+'StagedFastFit'
            Hlt2StagedFastFit           = TrackEventFitter(Hlt2StagedFastFit_name)
            Hlt2StagedFastFit.TracksInContainer    = tracks.outputSelection()
            Hlt2StagedFastFit.TracksOutContainer    = hlt2StagedFastFitOutputLocation
            Hlt2StagedFastFit.addTool(TrackMasterFitter, name = 'Fitter')
            from TrackFitter.ConfiguredFitters import ConfiguredHltFitter
            fitter = ConfiguredHltFitter( getattr(Hlt2StagedFastFit,'Fitter'))
            bm_members      = [tracks, Hlt2StagedFastFit]

        # Build the bindMembers
        bm_name         = self.__trackfitAlgosAndToolsPrefix()+"FastFitSeq"
        bm_output       = hlt2StagedFastFitOutputLocation
        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

    #########################################################################################
    #
    # Track reconstruction
    # TODO: Separate long and downstream tracks?
    def __hlt2Tracking(self) :
        """
        Track reconstruction
        """
        from HltLine.HltLine import bindMembers
        #
        # Some "inevitable" special stuff here
        # TODO: fix this description
        # If we ask for long tracks and no seeding, the final output will be
        # the forward tracking, otherwise the final output is the long
        # (forward + match) tracking.
        #
        hlt2TrackingOutput      = Hlt2TrackLoc[self.__trackType()]
        if self.getProp("CreateBestTracks"):
            hlt2TrackingOutput      = Hlt2TrackLoc["Best"]
        # Finally make the sequence
        # TODO: fix this description
        # The sequence called depends on the track type, so far we recognise two track types
        # for Long tracks the options doSeeding, doCloneKilling, etc. are meaningful
        # for SeedTT tracks the seeding is mandatory and the CloneKilling is irrelevant
        # This part gets done in either case
        trackRecoSequence = []
        if (self.__trackType() == "Long" or self.getProp("CreateBestTracks")) :
            # Do the forward, seeding + matching
            trackRecoSequence        =    [self.hlt2ForwardTracking()]
            trackRecoSequence       +=    [self.hlt2MatchTracking()]
            # The copy sequence, which merges the forward and
            # match tracks into the long track container
            hlt2TracksToMerge    =    []
            hlt2TracksToMerge   +=    [ self.hlt2ForwardTracking().outputSelection()]
            hlt2TracksToMerge   +=    [ self.hlt2MatchTracking().outputSelection()  ]
            if self.getProp("FitTTracks"):
                hlt2TracksToMerge   +=    [ self.hlt2SeedTracking().outputSelection() ]
            if  self.getProp("CreateBestTracks"):
                trackRecoSequence       +=    [self.hlt2DownstreamTracking()]
                hlt2TracksToMerge   +=    [ self.hlt2DownstreamTracking().outputSelection()  ]
            from Configurables import TrackEventCloneKiller, TrackCloneFinder
            #cloneKiller = TrackEventCloneKiller( self.__trackingAlgosAndToolsPrefix()+'FastCloneKiller'
            if self.getProp("UseTrackBestTrackCreator"):
                from Configurables import TrackBestTrackCreator
                bestTrackCreator = TrackBestTrackCreator( self.getProp("Prefix") + "TrackBestTrackCreator" )
                from Configurables import TrackMasterFitter
                from TrackFitter.ConfiguredFitters import ConfiguredHltFitter, ConfiguredMasterFitter
                bestTrackCreator.addTool(TrackMasterFitter, name="Fitter")
                from HltSharedTracking import ConfigureFitter
                ConfigureFitter( getattr(bestTrackCreator,"Fitter") )
                from Configurables import HltRecoConf
                bestTrackCreator.MaxChi2DoF = HltRecoConf().getProp("MaxTrCHI2PDOF")
                bestTrackCreator.StateInitTool.VeloFitterName = "FastVeloFitLHCbIDs"
                bestTrackCreator.StateInitTool.UseFastMomentumEstimate = True
                bestTrackCreator.DoNotRefit = True
                bestTrackCreator.TracksInContainers =  hlt2TracksToMerge
                bestTrackCreator.SplitByType = True
                bestTrackCreator.TracksOutContainer = ""
                bestTrackCreator.TracksOutContainers["Long"] = self.__trackLocationByType("Long")
                bestTrackCreator.TracksOutContainers["Downstream"] = self.__trackLocationByType("Downstream")
                if self.getProp("FitTTracks"):
                    bestTrackCreator.TracksOutContainers["Ttrack"] = self.__trackLocationByType("Ttrack")
                if HltRecoConf().getProp("ApplyGHOSTPROBCutInTBTC"):
                    bestTrackCreator.AddGhostProb = True
                    bestTrackCreator.MaxGhostProb = HltRecoConf().getProp("MaxTrGHOSTPROB")
                hlt2TrackingOutput = [ self.__trackLocationByType("Long"), self.__trackLocationByType("Downstream") ]
                if self.getProp("FitTTracks"):
                    hlt2TrackingOutput += [ self.__trackLocationByType("Ttrack") ]
                trackRecoSequence        +=      [bestTrackCreator]
                if self.getProp("FindV0s"):
                    from HltTracking.HltPVs import PV3D
                    pvs = PV3D("Hlt2")
                    from Configurables import TrackV0Finder
                    from Configurables import GaudiSequencer
                    v0Sequence = GaudiSequencer( self.getProp("Prefix") + "TrackV0FinderSeq")
                    v0Sequence.IgnoreFilterPassed = True
                    trackV0Finder = TrackV0Finder( self.getProp("Prefix") +"TrackV0Finder")
                    trackV0Finder.TrackContainers = [self.__trackLocationByType("Long"),self.__trackLocationByType("Downstream")]
                    trackV0Finder.PVContainer = pvs.output
                    trackV0Finder.V0Container = self.__trackLocationByType("Long")+"V0Vertices"
                    v0Sequence.Members = pvs.members() + [ trackV0Finder]
                    trackRecoSequence += [ v0Sequence ]

                from Configurables import HltRecoConf
                if HltRecoConf().getProp("AddGhostProb") and not HltRecoConf().getProp("ApplyGHOSTPROBCutInTBTC"):
                    from Configurables import TrackAddNNGhostId
                    addNNGhostIdLong = TrackAddNNGhostId( self.getProp("Prefix") + "LongTrackAddNNGhostID",GhostIdTool="Run2GhostId")
                    addNNGhostIdLong.inputLocation = self.__trackLocationByType("Long")
                    addNNGhostIdDownstream = TrackAddNNGhostId( self.getProp("Prefix") + "DownstreamTrackAddNNGhostID",GhostIdTool="Run2GhostId")
                    addNNGhostIdDownstream.inputLocation = self.__trackLocationByType("Downstream")
                    trackRecoSequence += [ addNNGhostIdLong, addNNGhostIdDownstream ]

            else:
                cloneKiller = TrackEventCloneKiller( self.getProp("Prefix") +'FastCloneKiller'
                                                     , TracksInContainers         = hlt2TracksToMerge
            	                                     , TracksOutContainer         = hlt2TrackingOutput
            	                                     , SkipSameContainerTracks    = False
            	                                     , CopyTracks                 = True)
                cloneKiller.addTool(TrackCloneFinder, name = 'CloneFinderTool')
                cloneKiller.CloneFinderTool.RestrictedSearch = True
                trackRecoSequence += [cloneKiller]

        elif (self.__trackType() == "Downstream" ) :
            trackRecoSequence = [self.hlt2DownstreamTracking()]

        # Set this property for outside access to it
        self.__allTracks = hlt2TrackingOutput

        # Build the bindMembers
        bm_name    = self.__trackingAlgosAndToolsPrefix()+"TrackingSeq"
        bm_members = trackRecoSequence
        bm_output  = hlt2TrackingOutput
        bm         = bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

        return bm

    #########################################################################################
    #
    # Hlt2 Velo Reconstruction
    #
    def __hlt2VeloTracking(self):
        """
        Velo track reconstruction for Hlt2
        """
        #HltSharedTracking decides which Velo sequence is used
        from HltSharedTracking        import RevivedVelo, MinimalVelo
        from Configurables  import Hlt2Conf
        from HltLine.HltLine    import bindMembers

        veloTracksOutputLocation = HltSharedTrackLoc["Velo"]

        Velo = RevivedVelo

        ## for debugging:
        #from Configurables import DumpTracks
        #veloDumper = DumpTracks('VeloDumper',TracksLocation = veloTracksOutputLocation )
        #veloDumper.StatPrint = True

        # Build the bindMembers
        bm_name         = self.getProp("Prefix")+"VeloTracking"
        bm_members      =  Velo.members()
        bm_output       = veloTracksOutputLocation

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)


    #########################################################################################
    #
    # Hlt2 Forward Reconstruction
    #
    def __hlt2ForwardTracking(self) :
        """
        Forward track reconstruction for Hlt2
        """
        from Configurables    import PatForward, Hlt2Conf
        from Configurables      import PatForwardTool , DumpTracks, TrackListRefiner
        from HltLine.HltLine    import bindMembers
        from HltTracking.HltSharedTracking import ( RevivedForward, ConfiguredForwardComplement, ConfiguredHltEventFitter,
                                                    ConfiguredGoodTrackFilter )
        from Configurables import HltRecoConf

        # Run Forward only on unused hits if wanted
        # Fit Hlt1 tracks for filtering
        VetoTracksLocation = []
        if self.getProp("DoCleanups"):
            # Do some filtering of bad tracks before marking hits
            fitHlt1ForwardTracks = ConfiguredHltEventFitter(name = self.getProp("Prefix") + 'FitHlt1Tracks',
                                                            TracksInContainer = RevivedForward.outputSelection(),
                                                            TracksOutContainer = RevivedForward.outputSelection()+HltDefaultFitSuffix)
            filterHlt1ForwardTracks = ConfiguredGoodTrackFilter( self.getProp("Prefix") + 'FilterHlt1ForwardTracks',
                                                                 InputLocation = fitHlt1ForwardTracks.TracksOutContainer )
            VetoTracksLocation = [ filterHlt1ForwardTracks.outputLocation ]
        else:
            VetoTracksLocation = [  RevivedForward.outputSelection() ]
        if self.getProp("RestartForward"):
            VetoTracksLocation = [  ]

        forwardComplementOutputLocation = Hlt2TrackLoc["ComplementForward"]
        recoForward = ConfiguredForwardComplement(name = self.getProp("Prefix") + "ForwardComplement"
                                                  , InputTracksName = HltSharedTrackLoc["Velo"]
                                                  , OutputTracksName = forwardComplementOutputLocation
                                                  , VetoSeedLocations = VetoTracksLocation
                                                  , MinMomentum = HltRecoConf().getProp("Forward_LPT_MinP")
                                                  , MinPt = HltRecoConf().getProp("Forward_LPT_MinPt"))

        if self.getProp("DoCleanups"):
            if not self.getProp("UseTrackBestTrackCreator"):
                fitHlt2ForwardTracks = ConfiguredHltEventFitter(name = self.getProp("Prefix") + 'FitHlt2ForwardTracks',
                                                                TracksInContainer = recoForward.OutputTracksName)
                filterHlt2ForwardTracks  = ConfiguredGoodTrackFilter( self.getProp("Prefix") + 'FilterHlt2ForwardTracks',
                                                                      InputLocation = recoForward.OutputTracksName )
                forwardLocations = [ filterHlt1ForwardTracks.outputLocation, filterHlt2ForwardTracks.outputLocation ]
            else:
                # Do the fit of Hlt2 tracks later when clone killing
                forwardLocations = [ filterHlt1ForwardTracks.outputLocation, forwardComplementOutputLocation ]
        else:
            # If we do not clean up hits, we do not need to fit tracks, can do that later
            forwardLocations = [ HltSharedTrackLoc["ForwardHPT"], forwardComplementOutputLocation ]
        if self.getProp("RestartForward"):
            forwardLocations = [ forwardTracksOutputLocation,forwardComplementOutputLocation ]

        from Configurables import TrackListMerger
        # We don't need a CloneKiller here, as by construction no clones are created in the two forward instances.
        # And even if we had clones, the same hits are marked for the seeding.
        # If we restart the Forward, we simply copy
        forwardMerger = TrackListMerger( self.getProp("Prefix") + 'ForwardMerger',
                                        inputLocations=forwardLocations ,
                                        outputLocation=Hlt2TrackLoc["Forward"] )


        # Build the sequences according to whether or not we reuse tracks from HLT1
        # Build the bindMembers
        bm_name         = self.getProp("Prefix")+"ForwardTracking"
        bm_members =  self.hlt2VeloTracking().members() + RevivedForward.members()
        if self.getProp("DoCleanups"):
            bm_members +=  [ fitHlt1ForwardTracks, filterHlt1ForwardTracks ]
            bm_members += [ recoForward ]
            if not self.getProp("UseTrackBestTrackCreator"):
                bm_members +=  [ fitHlt2ForwardTracks, filterHlt2ForwardTracks ]
        else:
            bm_members +=  [ recoForward ]
        bm_members +=  [ forwardMerger ]
        bm_output       = forwardMerger.outputLocation
        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

    #########################################################################################
    #
    # Hlt2 Seeding for forward,downstream reconstruction
    #
    def __hlt2SeedTracking(self):
        """
        Seeding in the trackers for later use in Match Forward reconstruction
        or in downstream tracking.
        """
        from Configurables    import PatSeeding
        from Configurables      import PatSeedingTool

        from HltLine.HltLine    import bindMembers

        # We depend on the forward tracking
        fwdtracks = self.hlt2ForwardTracking()
        # Now our output location
        seedTrackOutputLocation    = Hlt2TrackLoc["Seeding"]

        #### Seeding
        vetoTrackLocations = [ fwdtracks.outputSelection() ]
        from Configurables import HltRecoConf
        if HltRecoConf().getProp("OfflineSeeding"):
            vetoTrackLocations = None
        from HltTracking.HltSharedTracking import ConfiguredPatSeeding
        recoSeeding = ConfiguredPatSeeding(self.getProp("Prefix")+'Seeding',
                                           OutputTracksName = seedTrackOutputLocation,
                                           VetoTrackLocations = vetoTrackLocations )

        # Build the bindMembers
        bm_name         = self.getProp("Prefix")+"SeedTracking"
        bm_members      = self.hlt2VeloTracking().members() + self.__hlt2TrackerDecoding().members() + \
                          self.hlt2ForwardTracking().members() + [recoSeeding]
        bm_output       = seedTrackOutputLocation

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)
    #########################################################################################
    #
    # Hlt2 Match Forward Reconstruction
    #
    def __hlt2MatchTracking(self):
        """
        Forward track reconstruction for Hlt2 using seeding
        """
        from Configurables    import PatMatch
        from HltLine.HltLine    import bindMembers
        fwdtracks = self.hlt2ForwardTracking()
        matchTrackOutputLocation = Hlt2TrackLoc["Match"]
        # TODO: We can restrict the input of the matching by vetoing already used velo tracks.
        #### Matching
        recoMatch         = PatMatch(self.getProp("Prefix")+'Match'
                                     , VeloInput = self.hlt2VeloTracking().outputSelection()
                                     , SeedInput = self.hlt2SeedTracking().outputSelection()
                                     , MatchOutput = matchTrackOutputLocation)
        from Configurables import PatMatchTool
        from Configurables import HltRecoConf
        recoMatch.addTool(PatMatchTool, name="PatMatchTool")
        # We depend on the forward tracking
        if HltRecoConf().getProp("OfflineSeeding"):
            recoMatch.PatMatchTool.VeloVetoTracksName = []
        else:
            recoMatch.PatMatchTool.VeloVetoTracksName = [ fwdtracks.outputSelection() ]
            recoMatch.PatMatchTool.MinMomentum = HltRecoConf().getProp("Forward_LPT_MinP")
            recoMatch.PatMatchTool.MinPt = HltRecoConf().getProp("Forward_LPT_MinPt")


        # Build the bindMembers
        bm_name         = self.getProp("Prefix")+"MatchTracking"
        bm_members      = self.hlt2VeloTracking().members() + self.hlt2ForwardTracking().members() +self.hlt2SeedTracking().members()
        if self.getProp("DoCleanups") and not self.getProp("UseTrackBestTrackCreator"):
            from HltTracking.HltSharedTracking import ( ConfiguredHltEventFitter, ConfiguredGoodTrackFilter )
            fitHlt2MatchTracks = ConfiguredHltEventFitter(name = self.getProp("Prefix") + 'FitHlt2MatchTracks',
                                                          TracksInContainer = matchTrackOutputLocation)
            filterHlt2MatchTracks = ConfiguredGoodTrackFilter( self.getProp("Prefix") + 'FilterHlt2MatchTracks',
                                                               InputLocation = matchTrackOutputLocation )
            matchTrackOutputLocation = filterHlt2MatchTracks.outputLocation
            bm_members += [ recoMatch, fitHlt2MatchTracks, filterHlt2MatchTracks ]
        else:
            bm_members += [ recoMatch ]
        if self.getProp("EarlyDataTracking") :
            # Do something special in case of early data
            # For the moment just a dummy setting
            pass

        bm_output       = matchTrackOutputLocation

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)
    #########################################################################################
    #
    # Hlt2 Downstream Forward Reconstruction
    #
    def __hlt2DownstreamTracking(self):
        """
        Downstream track reconstruction for Hlt2 using seeding
        """
        from Configurables    import PatDownstream, PatLongLivedTracking
        from HltLine.HltLine    import bindMembers

        downstreamTrackOutputLocation    = Hlt2TrackLoc["Downstream"]

        fwdtracks   = self.hlt2ForwardTracking()
        matchtracks = self.hlt2MatchTracking()

        ### Downstream tracking


        if self.getProp("NewDownstreamAlg"):
            PatDownstream                 = PatLongLivedTracking(self.getProp("Prefix")+'PatLongLivedTracking')
        else:
            PatDownstream                 = PatDownstream(self.getProp("Prefix")+'PatDownstream')
        PatDownstream.InputLocation   = self.hlt2SeedTracking().outputSelection()
        PatDownstream.OutputLocation  = downstreamTrackOutputLocation
        PatDownstream.ForwardLocation = fwdtracks.outputSelection()
        PatDownstream.MatchLocation   = matchtracks.outputSelection()
        #Set to true to remove used seeds and tt hits
        PatDownstream.RemoveUsed     = False
        PatDownstream.RemoveAll      = True
        from HltTracking.HltRecoConf import DownstreamOptions
        PatDownstream.MinMomentum = DownstreamOptions["MinMomentum"]
        PatDownstream.MinPt = DownstreamOptions["MinPt"]

        bm_members      = self.hlt2MatchTracking().members() + [PatDownstream]
        #TODO: If we merge them, we don't need to fit the Downstream tracks which are obviously clones of good long tracks.
        #DONE: use TrackBestTrackCreator. TODO: Clean this up, once the reconstruction is fixed forever
        if self.getProp("CreateBestTracks") and not self.getProp("UseTrackBestTrackCreator"):
            from HltTracking.HltSharedTracking import ( ConfiguredHltEventFitter, ConfiguredGoodTrackFilter )
            fitHlt2DownstreamTracks = ConfiguredHltEventFitter(name = self.getProp("Prefix") + 'FitHlt2DownstreamTracks',
                                                               TracksInContainer = downstreamTrackOutputLocation)
            filterHlt2DownstreamTracks = ConfiguredGoodTrackFilter( self.getProp("Prefix") + 'FilterHlt2DownstreamTracks',
                                                                    InputLocation = downstreamTrackOutputLocation )

            from Configurables import CreateFastTrackCollection
            recoCopy = CreateFastTrackCollection(self.getProp("Prefix")+'CopyDownstreamTracks'
                                                 , InputLocations = [ filterHlt2DownstreamTracks.outputLocation ]
                                                 , OutputLocation = filterHlt2DownstreamTracks.outputLocation + HltDefaultFitSuffix
                                                 , SlowContainer  = True)
            #downstreamTrackOutputLocation = recoCopy.OutputLocation
            downstreamTrackOutputLocation = downstreamTrackOutputLocation
            #downstreamTrackOutputLocation = filterHlt2DownstreamTracks.outputLocation
            #bm_members      += [ fitHlt2DownstreamTracks,filterHlt2DownstreamTracks, recoCopy ]
            bm_members      += [ fitHlt2DownstreamTracks ]

        # Build the bindMembers
        bm_name         = self.getProp("Prefix")+"DownstreamTracking"
        bm_output       = downstreamTrackOutputLocation

        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)
    #########################################################################################
    #
    # Hlt2 tracker decoding
    #
    def __hlt2TrackerDecoding(self):
        """
        Decode the ST for Hlt2
        """
        from HltLine.HltDecodeRaw     import DecodeTT, DecodeIT
        from HltLine.HltLine        import bindMembers
        return bindMembers(self.getProp("Prefix")+"DecodeSTSeq", DecodeTT.members() + DecodeIT.members())
    #########################################################################################
    #
    # Helper function to set up the CALO processor and return the correct sequence
    #
    def __getNewCALOSeq(self, mode ):
        from HltLine.HltLine    import bindMembers
        # Load tracks
        tracks        = self.hlt2StagedFastFit()

        #neutralProtosLowEtOutputLocation = self.__protosLocation(Hlt2LowEtNeutralProtoParticleSuffix)
        #neutralProtosHighEtOutputLocation = self.__protosLocation(Hlt2HighEtNeutralProtoParticleSuffix)

        #outputCALOPID            = self.__caloIDLocation()
        caloName = "HLT2CaloLines"+ mode.capitalize() + self.__trackType()
        # Create configurable
        from Configurables import CaloLines
        caloLines = CaloLines(caloName)
        bm_name    = self.__pidAlgosAndToolsPrefix() + mode.capitalize() + self.__trackType()
        bm_members = [tracks]
        bm_output = ''
        if mode.lower() == 'photon':
            caloLines.HighPhoton = True
            caloLines.LowPhoton = False
            caloLines.LowElectron = False
            bm_name  += 'PhotonsFromL0'
            bm_output = '/Event/'+caloName+'HighPhoton/ProtoP/Neutrals'
        elif mode.lower() == 'electron':
            caloLines.HighPhoton = False
            caloLines.LowPhoton = False
            caloLines.LowElectron = True
            from Configurables import ChargedProtoParticleMaker
            bm_name  += 'ElectronsFromL0'
            bm_output = '/Event/' + caloName + 'LowElectron/ProtoP/Charged'
            chargedName = self.__pidAlgosAndToolsPrefix() + bm_output.replace("/", "") + 'ChargedProtoPAlg'
            charged     = ChargedProtoParticleMaker(chargedName)
            charged.TrackSelectorType = "TrackSelector"
            charged.addTool(TrackSelector, name="TrackSelector")
            ts=charged.TrackSelector
            from Configurables import HltRecoConf
            ts.setProp("MinChi2Cut", 0.)
            ts.setProp("MaxChi2Cut", HltRecoConf().getProp("MaxTrCHI2PDOF"))
            if HltRecoConf().getProp("ApplyGHOSTPROBCut") == True:
                ts.setProp("MinGhostProbCut", 0.)
                ts.setProp("MaxGhostProbCut", HltRecoConf().getProp("MaxTrGHOSTPROB"))

            if self.__trackType() == "Long":
                ts.TrackTypes = ["Long"]
            elif self.__trackType() == "Downstream":
                ts.TrackTypes = ["Downstream"]

            charged.Inputs = [tracks.outputSelection()]
            charged.Output = bm_output
            bm_members += [charged]
        elif mode.lower() == 'pi0':
            caloLines.HighPhoton = False
            caloLines.LowPhoton = True
            caloLines.LowElectron = False
            bm_name  += 'Pi0FromL0'
            bm_output = '/Event/'+caloName+'LowPhoton/ProtoP/Neutrals'
        seq = caloLines.sequence(tracks=[tracks.outputSelection()])
        bm_members += [seq]
        return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

    def caloProcessor(self):
        if not hasattr(self, '__caloProcessor') or not self.__caloProcessor:
            caloProcessorName = "SharedCaloProcessor"
            self.__caloProcessor = CaloProcessor(caloProcessorName)
            self.__caloProcessor.Context = self.__caloIDLocation()
        return self.__caloProcessor

    #########################################################################################
    #
    # Helper function to set up the CALO processor and return the correct sequence
    #
    def __getCALOSeq(self,mode):
        """
        Defines the CALO processor and, depending on the mode,
        returns the PID, charged proto, or neutral proto sequences
        """
        # We need all tracks: Long, Downstream (and possibly more in the future)
        tracks = self.hlt2Tracking()
        trackLocations = tracks.outputSelection()
        chargedProtos               = self.hlt2ChargedProtos(HltCaloProtosSuffix)
        chargedProtosOutputLocation = chargedProtos.outputSelection()
        neutralProtosOutputLocation = self.__protosLocation(Hlt2NeutralProtoParticleSuffix)
        outputCALOPID            = self.__caloIDLocation()

        from HltLine.HltLine    import bindMembers

        # One single CaloProcessor for all Hlt2Tracking instances
        myCALOProcessor         = self.caloProcessor()
        # Do the reconstruction and the PID but do not make or update the
        # protoparticles here!
        myCALOProcessor.CaloReco     = True
        myCALOProcessor.CaloPIDs     = True
        myCALOProcessor.CaloPIDTrTypes = [3,5]
        #
        # Check if we are making neutrals or not
        #
        myCALOProcessor.SkipNeutrals    = False
        myCALOProcessor.SkipCharged     = False

        # The sequences are given the track and protoparticle locations when initializing
        myPIDSeq         = myCALOProcessor.caloSequence(  trackLocations  )
        myChargedSeq     = myCALOProcessor.chargedProtoSequence(     trackLocations ,    chargedProtosOutputLocation    )
        myNeutralSeq     = myCALOProcessor.neutralProtoSequence(     trackLocations ,    neutralProtosOutputLocation    )

        bm_name = self.__pidAlgosAndToolsPrefix()
        if ( mode == "PID" ) :
            bm_name         += "CALOPIDSeq"
            bm_members      = [tracks, myPIDSeq]
            bm_output       = outputCALOPID
            return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

        if ( mode == "Neutral" ) :
            bm_name         += "CaloNeutralSeq"
            bm_members      = [tracks, myPIDSeq, myNeutralSeq]
            bm_output       = myCALOProcessor.NeutralProtoLocation
            return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)

        if ( mode == "Charged" ) :
            bm_name         += "CaloChargedSeq"
            bm_members      = [tracks, myPIDSeq, chargedProtos, myChargedSeq]
            bm_output       = chargedProtosOutputLocation
            return bindMembers(bm_name, bm_members).setOutputSelection(bm_output)
