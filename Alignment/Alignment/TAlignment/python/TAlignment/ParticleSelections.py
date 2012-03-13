##################################################################
# Data structure to hold a particles selection for the alignment.
# This still needs to be moved to a better location.
##################################################################

class ParticleSelection:
    #__slots__ = {
    #    "_name" : ""
    #    ,"_location" : "" # particle location in the TES
    #    ,"_algorithm" : None
    #    }
    def __init__( self, Name, Location, Algorithm = None ) :
        self._name = Name
        self._location = Location
        self._algorithm = Algorithm

    def algorithm(self) : return self._algorithm
    def location(self) : return self._location
    def name(self) : return self._name

##################################################################
# Creates a selection object from a location in the TES ona dst,
# making sure to refit the particles. It would make sense to add some
# cuts here as well.
##################################################################
def configuredParticleListFromDST( ParticleLocation ) :
    # create a sequence to refit and monitor
    name = ParticleLocation
    name.replace("Event","")
    name.replace("/Phys","")
    name.replace("/Particle","")
    name.replace("/","_")
    fitter = TrackParticleRefitter( name + "Refitter", 
                                    Fitter = TrackMasterFitter,
                                    InputLocation = ParticleLocation )
    configuredMasterFitter( fitter.Fitter, SimplifiedMaterial = True )
    seq = GaudiSequencer(name + "Seq")
    seq.append( fitter )
    sel = ParticleSelection()
    sel.algorithm = seq
    sel.location = ParticleLocation
    return sel


def protoParticlesFromHLTSelSequence( Name, HltDecision,
                                      ApplyRichID = False,
                                      ApplyMuonID = False):
    # create a sequence that
    # * revives tracks from an HLT sequence
    # * fits them
    # * runs rich or muon ID on demand
    # tracks will be stored at location 'Rec/Track/' + Name
    # protoparticles will be stored at location ????
    trackListName =  'Rec/Track/' + Name
    from Configurables import GaudiSequencer
    seq = GaudiSequencer(Name + "Seq")
    # revive only particles used for trigger
    from Configurables import HltTrackConverter
    hltTrackConv = HltTrackConverter(Name + "HltTrackConv")
    hltTrackConv.ReadHltLinesFrom1stEvent = HltDecision
    hltTrackConv.TrackDestignation = 'Rec/Track/' + Name + 'All'
    seq.Members += [ HltSelReportsDecoder(), hltTrackConv ]
    # now fit those tracks and apply a selection
    from TAlignment.Utils import configuredFitAndHitAdderSequence
    fitseq = configuredFitAndHitAdderSequence( Name + 'Fit',
                                               trackListName + 'All',
                                               trackListName )
    seq.Members += [ fitseq ]
    # next step, prepare the RICH sequence, if we need it.
    #if ApplyRichID:
    return seq

##################################################################
# Create a selection based on HLT D0->Kpi
##################################################################
def defaultHLTD0Selection():

    # this still needs to be worked out
    from Configurables import Escher
    Escher().RecoSequence = ["Hlt","Decoding","AlignTr","Vertex","RICH" ]
    Escher().MoniSequence = ["Tr","OT"]

    # if the Escher hlt filter is not set, set it here
    if not hasattr(Escher(),"HltFilterCode") or not Escher().HltFilterCode :
        #Code = "HLT_PASS_RE( 'Hlt2CharmHadD02HH_D02KPiDecision' )"
        Escher().HltFilterCode = "HLT_PASS_RE( 'Hlt2ExpressDStar2D0PiDecision' )"# | HLT_PASS_RE( 'Hlt2ExpressBeamHalo' )"
    
    # revive only particles used for trigger
    from Configurables import GaudiSequencer
    trackseq = GaudiSequencer("RecoAlignTrSeq")
    from Configurables import HltTrackConverter, HltSelReportsDecoder
    hltTrackConv = HltTrackConverter("HltTrackConv")
    hltTrackConv.HltLinesToUse = ['Hlt2ExpressDStar2D0PiDecision']
    hltTrackConv.TrackDestignation = 'Rec/Track/AllBest'
    trackseq.Members += [ HltSelReportsDecoder(), hltTrackConv ]#, trkContCopy]

    # create a sequence that fits the tracks and does the hit-adding
    from TAlignment.Utils import configuredFitAndHitAdderSequence
    fitseq =  configuredFitAndHitAdderSequence( Name = 'HltD0', 
                                                InputLocation = hltTrackConv.TrackDestignation,
                                                OutputLocation = 'Rec/Track/Best' )
    trackseq.Members.append( fitseq )
    # now make sure that there are at least 2 tracks left
    from Configurables import LoKi__VoidFilter as LokiFilter 
    trackseq.Members.append( LokiFilter ( 'BestTrackFilter' ,
                                          Code = "1 < CONTAINS ( 'Rec/Track/Best' )" ) )
    
     # Tweak a little bit RICH
    from Configurables import Escher,RichRecSysConf,RecSysConf
    richSeqName         = Escher()._instanceName(RichRecSysConf)
    richSeq             = GaudiSequencer(richSeqName+"Seq")
    richSeq.MeasureTime = True
    richConf            = RichRecSysConf(RecSysConf().richRecConfName)
    richConf.DataType   = Escher().getProp( "DataType" )
    richConf.Context    = "HLT"
    #enable 'fast' PID settings
    richConf.PidConfig          = "FastGlobal"
    # only consider the Pion and Kaon hypos (instead of the full list. This means you will
    # only have DLL(K-pi) available, but if that is all you need its OK).
    richConf.Particles    = [ "pion","kaon" ]
    #turn off trackless ring finding
    richConf.TracklessRingAlgs  = []
    #richConf.trackConfig().InputTracksLocation = '/Rec/Track/PidTracks'
    # Set the sequence to run the RICH PID in
    richConf.setProp("RecoSequencer",richSeq)

    # now create the D0->K-Pi+ candidates
    from Configurables import FilterDesktop
    from Configurables import ChargedProtoParticleMaker, ChargedProtoParticleAddRichInfo, ChargedProtoCombineDLLsAlg

    # take as much as possible from CommonParticles
    from CommonParticles.StdAllLooseKaons import StdAllLooseKaons
    from CommonParticles.StdAllLoosePions import StdAllLoosePions
    from CommonParticles.StdLooseKaons import StdLooseKaons
    from CommonParticles.StdLoosePions import StdLoosePions
    from CommonParticles.StdLooseD02HH import StdLooseD02KPi
    # remove cuts that require a PV
    StdLooseD02KPi.MotherCut = "((VFASPF(VCHI2)<10) & (ADMASS('D0')<100*MeV))"
    StdLooseKaons.Code = "ALL"
    StdLoosePions.Code = "ALL"

    # tighten the mass window for candidates used in alignment. also
    # add tight PID cuts basically toensure that we don't swap the
    # kaon and pion.
    AlignD02KPi = FilterDesktop("AlignD02KPi",
                                Inputs = ["Phys/StdLooseD02KPi"], 
                                Code = "(ADMASS('D0') < 20.*MeV) & (VFASPF(VCHI2) < 9.)"\
                                    " & (MINTREE('K+'==ABSID, PIDK) > 0)"\
                                    " & (MINTREE('pi+'==ABSID, PIDK) < 0)" )
    # create the sequence that we pass to the alignment 
    recoD0Seq= GaudiSequencer("RecoD0Seq")
    recoD0Seq.Members = [ 
        ChargedProtoParticleMaker('ChargedProtoPMaker'),
        ChargedProtoParticleAddRichInfo('ChargedProtoPAddRich'),
        ChargedProtoCombineDLLsAlg('ChargedProtoPCombDLLs'),
        #StdAllLoosePions, StdAllLooseKaons,
        #StdLoosePions, StdLooseKaons,
        #StdLooseD02KPi,
        AlignD02KPi]

    # Add monitors
    from Configurables import TrackParticleMonitor
    recoD0Seq.Members += [ 
        TrackParticleMonitor('StdLooseD02KPiMonitor', 
                             InputLocation = '/Event/Phys/StdLooseD02KPi/Particles',
                             MinMass = 1810, MaxMass = 1930),
        TrackParticleMonitor('AlignD02KPiMonitor', 
                             InputLocation = '/Event/Phys/AlignD02KPi/Particles',
                             MinMass = 1810, MaxMass = 1930)
        ]
    
    sel = ParticleSelection( Name = 'D02KPi',
                             Location = '/Event/Phys/AlignD02KPi/Particles',
                             Algorithm = recoD0Seq )
    return sel

##################################################################
# Create a selection based on HLT J/psi->mumu
##################################################################
def defaultHLTJPsiSelection():

    # this still needs to be worked out
    from Configurables import Escher
    Escher().RecoSequence = ["Hlt","Decoding","AlignTr","Vertex","MUON" ]
    Escher().MoniSequence = ["Tr","OT"]

    # if the Escher hlt filter is not set, set it here
    if not hasattr(Escher(),"HltFilterCode") or not Escher().HltFilterCode :
        #Code = "HLT_PASS_RE( 'Hlt2DiMuonJPsi.*Decision' )"
        Escher().HltFilterCode = "HLT_PASS_RE( 'Hlt2.*JPsi.*Decision' )"

    # revive only particles used for trigger 
    from Configurables import GaudiSequencer
    trackseq = GaudiSequencer("RecoAlignTrSeq")
    from Configurables import HltTrackConverter, HltSelReportsDecoder
    hltTrackConv = HltTrackConverter("HltTrackConv")
    #hltTrackConv.ReadHltLinesFrom1stEvent = True
    hltTrackConv.HltLinesToUse = ['Hlt2ExpressJPsiDecision',
                                  'Hlt2DiMuonDetachedJPsiDecision',
                                  'Hlt2DiMuonJPsiDecision',
                                  'Hlt2DiMuonJPsiHighPTDecision']
    hltTrackConv.TrackDestignation = 'Rec/Track/AllBest'
    trackseq.Members += [ HltSelReportsDecoder(), hltTrackConv ]#, trkContCopy]

    # create a sequence that fits the tracks and does the hit-adding
    from TAlignment.Utils import configuredFitAndHitAdderSequence
    trackseq.Members.append(configuredFitAndHitAdderSequence(  Name = 'HltJpsi',
                                                               InputLocation = hltTrackConv.TrackDestignation,
                                                               OutputLocation = 'Rec/Track/Best' ))
    
    # now make sure that there are at least 2 tracks left
    from Configurables import LoKi__VoidFilter as LokiFilter 
    trackseq.Members.append( LokiFilter ( 'BestTrackFilter' ,
                                          Code = "1 < CONTAINS ( 'Rec/Track/Best' )" ) )
                             
    # Now create the J/psi candidates
    from Configurables import CombineParticles, FilterDesktop
    from CommonParticles.StdAllLooseMuons import StdAllLooseMuons
    from CommonParticles.StdLooseJpsi2MuMu import StdLooseJpsi2MuMu
    
    ## tighten the mass window for candidates used in alignment
    AlignJpsi2MuMu = FilterDesktop("AlignJpsi2MuMu",
                                   Inputs = ["Phys/StdLooseJpsi2MuMu"],
                                   Code = "(ADMASS('J/psi(1S)') < 25.*MeV) & (VFASPF(VCHI2) < 9.)")
    
    from Configurables import ChargedProtoParticleMaker, ChargedProtoParticleAddMuonInfo, ChargedProtoCombineDLLsAlg
    from Configurables import TrackParticleMonitor
    recoJpsiSeq= GaudiSequencer("RecoJpsiSeq")
    recoJpsiSeq.Members = [ 
        ChargedProtoParticleMaker('ChargedProtoPMaker'),
        ChargedProtoParticleAddMuonInfo('ChargedProtoPAddMuon'),
        ChargedProtoCombineDLLsAlg('ChargedProtoPCombDLLs'),
        StdAllLooseMuons,  # we could also get this from the DoD
        StdLooseJpsi2MuMu, # we could also get this from the DoD
        AlignJpsi2MuMu,
        TrackParticleMonitor('StdLooseJpsi2MuMuMonitor', 
                             InputLocation = '/Event/Phys/StdLooseJpsi2MuMu/Particles',
                             MinMass = 3000, MaxMass = 3190),
        TrackParticleMonitor('AlignJpsi2MuMuMonitor', 
                             InputLocation = '/Event/Phys/AlignJpsi2MuMu/Particles',
                             MinMass = 3000, MaxMass = 3190),
        ]

    sel = ParticleSelection( Name = 'Jpsi2MuMu',
                             Location = '/Event/Phys/AlignJpsi2MuMu/Particles',
                             Algorithm = recoJpsiSeq )
    return sel

