"""HLT Afterburner configuration."""
import types
from Gaudi.Configuration import *
from LHCbKernel.Configuration import *
from Configurables import GaudiSequencer as Sequence
from Configurables import ChargedProtoANNPIDConf
from HltTracking.Hlt2TrackingConfigurations import Hlt2BiKalmanFittedForwardTracking
from HltTracking.Hlt2TrackingConfigurations import Hlt2BiKalmanFittedDownstreamTracking

from HltPersistReco import HltPersistRecoConf

from ThresholdUtils import importLineConfigurables
import Hlt2Lines

__author__ = "R. Aaij roel.aaij@cern.ch"


class HltAfterburnerConf(LHCbConfigurableUser):
    # python configurables to be applied before me
    __queried_configurables__ = [
        Hlt2BiKalmanFittedForwardTracking,
        Hlt2BiKalmanFittedDownstreamTracking,
    ] + importLineConfigurables(Hlt2Lines)
    # We need the dependency on the Hlt2 lines above as we need to inspect
    # all of them for the PersistReco flag in order to create the filter

    # python configurables that I configure
    __used_configurables__ = [
        HltPersistRecoConf,
        ( ChargedProtoANNPIDConf, None )
    ]

    __slots__ = {"Sequence"                : None,
                 "EnableHltRecSummary"     : True,
                 "RecSummaryLocation"      : "Hlt2/RecSummary",
                 "AddAdditionalTrackInfos" : True,
                 "AddPIDToDownstream"      : True,
                 "Hlt2DownstreamFilter"    : [ "Hlt2CharmHadDp2KS0KS0KpTurbo",
                                               "Hlt2CharmHadDp2KS0KS0PipTurbo",
                                               "Hlt2CharmHadLcp2LamKmKpPip_Lam2PpPimTurbo",
                                               "Hlt2CharmHadLcp2LamKmPipPip_Lam2PpPimTurbo",
                                               "Hlt2CharmHadDsp2KS0KS0KpTurbo",
                                               "Hlt2CharmHadDsp2KS0KS0PipTurbo",
                                               "Hlt2CharmHadLcp2KS0KS0PpTurbo",
                                               "Hlt2CharmHadXim2LamPim_DDDTurbo",
                                               "Hlt2CharmHadXim2LamPim_DDLTurbo",
                                               "Hlt2CharmHadOmm2LamKm_DDDTurbo",
                                               "Hlt2CharmHadOmm2LamKm_DDLTurbo",
                                               "Hlt2CharmHadDstp2D0Pip_D02KS0KmKp_KS0DDTurbo",
                                               "Hlt2CharmHadDstp2D0Pip_D02KS0KmKp_KS0DD_LTUNBTurbo",
                                               "Hlt2CharmHadDstp2D0Pip_D02KS0KmPip_KS0DDTurbo",
                                               "Hlt2CharmHadDstp2D0Pip_D02KS0KmPip_KS0DD_LTUNBTurbo",
                                               "Hlt2CharmHadDstp2D0Pip_D02KS0KpPim_KS0DDTurbo",
                                               "Hlt2CharmHadDstp2D0Pip_D02KS0KpPim_KS0DD_LTUNBTurbo",
                                               "Hlt2CharmHadDstp2D0Pip_D02KS0PimPip_KS0DDTurbo",
                                               "Hlt2CharmHadDstp2D0Pip_D02KS0PimPip_KS0DD_LTUNBTurbo",
                                               "Hlt2CharmHadDstp2D0Pip_D02KS0KS0_KS0DDTurbo",
                                               "Hlt2CharmHadDstp2D0Pip_D02KS0KS0_KS0LL_KS0DDTurbo",
                                               "Hlt2CharmHadDp2KS0Pip_KS0DDTurbo",
                                               "Hlt2CharmHadDp2KS0Kp_KS0DDTurbo",
                                               "Hlt2CharmHadDp2KS0KmKpPip_KS0DDTurbo",
                                               "Hlt2CharmHadDp2KS0KmPipPip_KS0DDTurbo",
                                               "Hlt2CharmHadDp2KS0KpKpPim_KS0DDTurbo",
                                               "Hlt2CharmHadDp2KS0KpPimPip_KS0DDTurbo",
                                               "Hlt2CharmHadDp2KS0PimPipPip_KS0DDTurbo",
                                               "Hlt2CharmHadDsp2KS0KmKpPip_KS0DDTurbo",
                                               "Hlt2CharmHadDsp2KS0KmPipPip_KS0DDTurbo",
                                               "Hlt2CharmHadDsp2KS0KpKpPim_KS0DDTurbo",
                                               "Hlt2CharmHadDsp2KS0KpPimPip_KS0DDTurbo",
                                               "Hlt2CharmHadDsp2KS0PimPipPip_KS0DDTurbo",
                                               "Hlt2CharmHadLcp2LamKp_LamDDTurbo",
                                               "Hlt2CharmHadLcp2LamPip_LamDDTurbo"
                                               ],
                 "Hlt2Filter"              : "HLT_PASS_RE('Hlt2(?!Forward)(?!DebugEvent)(?!Lumi)(?!Transparent)(?!PassThrough).*Decision')"
                }

    def _persistRecoLines(self):
        from HltLine.HltLine import hlt2Lines
        return [line for line in hlt2Lines() if line.persistReco()]

    def _persistRecoFilterCode(self, lines):
        decisions = [line.decision() for line in lines]
        # code = ' | '.join(["HLT_TURBOPASS('{}')".format(i) for i in decisions])
        # if not code: code = 'HLT_NONE'
        # There is no HLT_TURBOPASS functor, so need to use regexps:
        code = "HLT_TURBOPASS_RE('^({})$')".format('|'.join(decisions))
        return code

    def _filterCode(self, lines):
        code = "HLT_TURBOPASS_RE('^({})Decision$')".format('|'.join(lines))
        return code

    def _persistRecoSeq(self):
        lines = self._persistRecoLines()
        code = self._persistRecoFilterCode(lines)
        print '# List of requested PersistReco lines: {}'.format(
            [line.name() for line in lines])

        from DAQSys.Decoders import DecoderDB
        decoder = DecoderDB["HltDecReportsDecoder/Hlt2DecReportsDecoder"]
        from Configurables import LoKi__HDRFilter as HltFilter
        lineFilter = HltFilter(
            "Hlt2PersistRecoLineFilter",
            Code=code,
            Location=decoder.listOutputs()[0]
        )
        seq = Sequence("HltPersistReco")
        HltPersistRecoConf().Sequence = seq
        return Sequence("HltPersistRecoFilterSequence", Members=[lineFilter, seq])

###################################################################################
#
# Main configuration
#
    def __apply_configuration__(self):
        """
        HLT Afterburner configuration
        """
        from HltTracking.Hlt2TrackingConfigurations import Hlt2BiKalmanFittedForwardTracking
        from HltTracking.Hlt2TrackingConfigurations import Hlt2BiKalmanFittedDownstreamTracking

        Afterburner = self.getProp("Sequence") if self.isPropertySet("Sequence") else None
        if not Afterburner:
            return
        AfterburnerFilterSeq = Sequence("HltAfterburnerFilterSequence" )
        Afterburner.Members += [AfterburnerFilterSeq]
        if self.getProp("Hlt2Filter"):
            from DAQSys.Decoders import DecoderDB
            decoder = DecoderDB["HltDecReportsDecoder/Hlt2DecReportsDecoder"]
            from Configurables import LoKi__HDRFilter   as HDRFilter
            hlt2Filter = HDRFilter('HltAfterburnerHlt2Filter', Code = self.getProp("Hlt2Filter"),
                                   Location = decoder.listOutputs()[0])
            AfterburnerFilterSeq.Members += [hlt2Filter]
        AfterburnerSeq = Sequence("HltAfterburnerSequence", IgnoreFilterPassed = True)
        AfterburnerFilterSeq.Members += [ AfterburnerSeq ]
        if self.getProp("EnableHltRecSummary"):
            from Configurables import RecSummaryAlg
            seq = Sequence("RecSummarySequence")
            
            tracks = Hlt2BiKalmanFittedForwardTracking().hlt2PrepareTracks()
            tracksDown = Hlt2BiKalmanFittedDownstreamTracking().hlt2PrepareTracks()
            muonID = Hlt2BiKalmanFittedForwardTracking().hlt2MuonID()

            from HltLine.HltDecodeRaw import DecodeVELO, DecodeIT, DecodeTT, DecodeSPD, DecodeMUON
            decoders = {"Velo"   : (DecodeVELO, "VeloLiteClustersLocation"),
                        "TT"     : (DecodeTT,   "clusterLocation"),
                        "IT"     : (DecodeIT,   "clusterLocation"),
                        "SPD"    : (DecodeSPD,  "DigitsContainer"),
                        'Muon'   : (DecodeMUON, "OutputLocation"),
                        'MuonTr' : (muonID,     "MuonTrackLocation")}
            decoders = {k : (bm.members(), bm.members()[-1].getProp(loc)) for (k, (bm, loc)) in decoders.iteritems()}

            from HltTracking.HltPVs import PV3D
            PVs = PV3D("Hlt2")
            from HltTracking.HltTrackNames import Hlt2TrackLoc
            recSeq = Sequence("RecSummaryRecoSequence", IgnoreFilterPassed = True)
            from itertools import chain
            from Hlt2Lines.Utilities.Utilities import uniqueEverseen
            recSeq.Members = list(uniqueEverseen(chain.from_iterable([dec[0] for dec in decoders.itervalues()]
                                                                     + [tracks, tracksDown, muonID, PVs])))
            summary = RecSummaryAlg('Hlt2RecSummary', SummaryLocation = self.getProp("RecSummaryLocation"),
                                    HltSplitTracks = True,
                                    SplitLongTracksLocation = tracks.outputSelection(),
                                    SplitDownTracksLocation = tracksDown.outputSelection(),
                                    PVsLocation = PVs.output,
                                    VeloClustersLocation = decoders['Velo'][1],
                                    ITClustersLocation = decoders['IT'][1],
                                    TTClustersLocation = decoders['TT'][1],
                                    SpdDigitsLocation  = decoders['SPD'][1],
                                    MuonCoordsLocation = decoders['Muon'][1],
                                    MuonTracksLocation = decoders['MuonTr'][1])
            seq.Members = [recSeq, summary]
            AfterburnerSeq.Members += [seq]

        if self.getProp("AddAdditionalTrackInfos"):
            from GaudiKernel.SystemOfUnits import mm
            from Configurables import LoKi__VoidFilter as Filter
            trackLocations = [ Hlt2BiKalmanFittedForwardTracking().hlt2PrepareTracks().outputSelection(),
                               Hlt2BiKalmanFittedDownstreamTracking().hlt2PrepareTracks().outputSelection() ]
            infoSeq = Sequence("TrackInfoSequence", IgnoreFilterPassed = True)
            # I don't want to pull in reconstruction if not run before, then there should be also no candidates needing this information
            # This is anyhow done by the RecSummary above
            members = [Hlt2BiKalmanFittedForwardTracking().hlt2PrepareTracks().members() + Hlt2BiKalmanFittedDownstreamTracking().hlt2PrepareTracks().members()]
            infoSeq.Members += list(uniqueEverseen(chain.from_iterable(members)))
            prefix = "Hlt2"
            trackClones = Sequence(prefix + "TrackClonesSeq")
            #checkTracks =  Filter(prefix+"CheckTrackLoc",Code = "EXISTS('%(trackLocLong)s') & EXISTS('%(trackLocDown)s')" % {"trackLocLong" : trackLocations[0], "trackLocDown" : trackLocations[1]})
            #trackClones.Members += [checkTracks]
            from Configurables import TrackBuildCloneTable, TrackCloneCleaner
            cloneTable = TrackBuildCloneTable(prefix + "FindTrackClones")
            cloneTable.maxDz   = 500*mm
            cloneTable.zStates = [ 0*mm, 990*mm, 9450*mm ]
            cloneTable.klCut   = 5e3
            cloneTable.inputLocations = trackLocations
            cloneTable.outputLocation = trackLocations[0]+"Downstream" + "Clones"
            cloneCleaner = TrackCloneCleaner(prefix + "FlagTrackClones")
            cloneCleaner.CloneCut = 5e3
            cloneCleaner.inputLocations = trackLocations
            cloneCleaner.linkerLocation = cloneTable.outputLocation
            trackClones.Members += [ cloneTable, cloneCleaner ]

            infoSeq.Members += [trackClones]

            AfterburnerSeq.Members += [infoSeq]

            # Add VeloCharge to protoparticles for dedx
            veloChargeSeq = Sequence("VeloChargeSequence")
            from Configurables import ChargedProtoParticleAddVeloInfo
            protoLocation = Hlt2BiKalmanFittedForwardTracking().hlt2ChargedAllPIDsProtos().outputSelection()
            checkProto =  Filter("CheckProtoParticles",Code = "EXISTS('%(protoLoc)s')"% {"protoLoc" : protoLocation})
            addVeloCharge = ChargedProtoParticleAddVeloInfo("Hlt2AddVeloCharge")
            addVeloCharge.ProtoParticleLocation = protoLocation
            decodeVeloFullClusters = DecoderDB["DecodeVeloRawBuffer/createVeloClusters"].setup()
            veloChargeSeq.Members +=  [checkProto, decodeVeloFullClusters, addVeloCharge]
            AfterburnerSeq.Members += [veloChargeSeq]

        persistRecoLines = self._persistRecoLines()
        if self.getProp("AddPIDToDownstream") and (len(persistRecoLines)>0 or len(self.getProp("Hlt2DownstreamFilter"))>0):
            from Configurables import LoKi__HDRFilter   as HDRFilter
            if (len(persistRecoLines)>0 and len(self.getProp("Hlt2DownstreamFilter"))>0):
                code = self._persistRecoFilterCode(persistRecoLines) + " | " + self._filterCode(self.getProp("Hlt2DownstreamFilter"))
            elif len(persistRecoLines)>0:
                code = self._persistRecoFilterCode(persistRecoLines)
            elif len(self.getProp("Hlt2DownstreamFilter"))>0:
                code = self._filterCode(self.getProp("Hlt2DownstreamFilter"))
            # Activate Downstream RICH for all PersistReco lines
            from DAQSys.Decoders import DecoderDB
            decoder = DecoderDB["HltDecReportsDecoder/Hlt2DecReportsDecoder"]
            hlt2DownstreamFilter = HDRFilter('DownstreamHlt2Filter', Code =  code ,
                                   Location = decoder.listOutputs()[0])
            downstreamPIDSequence = Sequence( "Hlt2AfterburnerDownstreamPIDSeq")
            downstreamPIDSequence.Members += [ hlt2DownstreamFilter ]
            downstreamTracking = Hlt2BiKalmanFittedDownstreamTracking()
            tracksDown = downstreamTracking.hlt2PrepareTracks()
            protosDown = downstreamTracking.hlt2ChargedNoPIDsProtos()
            chargedProtosOutputLocation =  protosDown.outputSelection()
            richPid = downstreamTracking.hlt2RICHID()
            downstreamPIDSequence.Members += list(uniqueEverseen(chain.from_iterable([ tracksDown, protosDown, richPid ])))
            from Configurables import ChargedProtoParticleAddRichInfo,ChargedProtoParticleAddMuonInfo
            downstreamRichDLL_name            = "Hlt2AfterburnerDownstreamProtoPAddRich"
            downstreamRichDLL                 = ChargedProtoParticleAddRichInfo(downstreamRichDLL_name)
            downstreamRichDLL.InputRichPIDLocation    = richPid.outputSelection()
            downstreamRichDLL.ProtoParticleLocation   = chargedProtosOutputLocation
            # Add the muon info to the DLL
            downstreamMuon_name                   = "Hlt2AfterburnerDownstreamProtoPAddMuon"
            downstreamMuon                        = ChargedProtoParticleAddMuonInfo(downstreamMuon_name)
            downstreamMuon.ProtoParticleLocation  = chargedProtosOutputLocation
            downstreamMuon.InputMuonPIDLocation   = downstreamTracking.hlt2MuonID().outputSelection()
            # Add the Calo info to the DLL
            #
            from Configurables import ( ChargedProtoParticleAddEcalInfo,
                                        ChargedProtoParticleAddBremInfo,
                                        ChargedProtoParticleAddHcalInfo,
                                        ChargedProtoParticleAddPrsInfo,
                                        ChargedProtoParticleAddSpdInfo
                                        )
            caloPidLocation = downstreamTracking.hlt2CALOID().outputSelection()
            downstreamEcal = ChargedProtoParticleAddEcalInfo("HltAfterburnerDownstreamProtoPAddEcal")
            downstreamBrem = ChargedProtoParticleAddBremInfo("HltAfterburnerDownstreamProtoPAddBrem")
            downstreamHcal = ChargedProtoParticleAddHcalInfo("HltAfterburnerDownstreamProtoPAddHcal")
            downstreamPrs  = ChargedProtoParticleAddPrsInfo ("HltAfterburnerDownstreamProtoPAddPrs")
            downstreamSpd  = ChargedProtoParticleAddSpdInfo ("HltAfterburnerDownstreamProtoPAddSpd")
            for alg in (downstreamEcal,downstreamBrem,downstreamHcal,downstreamPrs,downstreamSpd):
                alg.setProp("ProtoParticleLocation",chargedProtosOutputLocation)
                alg.setProp("Context",caloPidLocation)
                downstreamPIDSequence.Members += [ alg  ]


            from Configurables import ChargedProtoCombineDLLsAlg, ChargedProtoANNPIDConf
            downstreamCombine_name                    = "Hlt2AfterburnerDownstreamRichCombDLLs"
            downstreamCombine                         = ChargedProtoCombineDLLsAlg(downstreamCombine_name)
            downstreamCombine.ProtoParticleLocation   = downstreamTracking.hlt2ChargedNoPIDsProtos().outputSelection()
            from Hlt2Lines.Utilities.Utilities import uniqueEverseen
            downstreamPIDSequence.Members += [ downstreamMuon,downstreamRichDLL,downstreamCombine ]
            probNNDownSeqName = self._instanceName(ChargedProtoANNPIDConf)

            probNNDownSeq             = GaudiSequencer(probNNDownSeqName+"Seq")
            annconfDown = ChargedProtoANNPIDConf(probNNDownSeqName)
            annconfDown.DataType = downstreamTracking.DataType
            annconfDown.TrackTypes = [ "Downstream" ]
            annconfDown.RecoSequencer = probNNDownSeq
            annconfDown.ProtoParticlesLocation = downstreamTracking.hlt2ChargedNoPIDsProtos().outputSelection()
            downstreamPIDSequence.Members += [ probNNDownSeq ]
            AfterburnerSeq.Members += [ downstreamPIDSequence ]

        # Configure and add the persist reco
        AfterburnerSeq.Members += [self._persistRecoSeq()]
