"""
 Configurable to configure HLT monitoring

 @author R. Aaij
 @date 19-06-2015
"""
# =============================================================================
__author__  = "R. Aaij roel.aaij@cern.ch"
# =============================================================================
import types
from Gaudi.Configuration import *
from LHCbKernel.Configuration import *
from Configurables import GaudiSequencer as Sequence

def _recurse(c,fun) :
    fun(c)
    for p in [ 'Members','Filter0','Filter1' ] :
        if not hasattr(c,p) : continue
        x = getattr(c,p)
        if list is not type(x) : x = [ x ]
        for i in x : _recurse(i,fun)

def _enableMonitoring(c) :
    if c.getType() in ['FilterDesktop','CombineParticles' ] :
        c.Monitor = True
        c.HistoProduce = True

def _disableHistograms(c,filter = lambda x : True) :
    if 'HistoProduce' in c.getDefaultProperties() and filter(c):
        c.HistoProduce = False
    for p in [ 'Members','Filter0','Filter1' ] :
        if not hasattr(c,p) : continue
        x = getattr(c,p)
        if list is not type(x) : x = [ x ]
        for i in x : _disableHistograms(i,filter)


class HltMonitoringConf(LHCbConfigurableUser):
    __used_configurables__ = []

    __slots__ = {"EnableAlgMonitoring"      : False,  # Enable monitoring for some algorithms
                 "EnableL0Monitor"          : True,
                 "EnableGlobalMonitor"      : True,
                 "EnableTrackMonitor"       : True,
                 "EnableMassMonitor"        : True,
                 'Hlt1TrackMonitorPrescale' : 0.001,
                 'Hlt1TrackMonitorGEC'      : "Loose",
                 "HistogrammingLevel"       : "Line",  # One of 'None', 'Line' or 'NotLine'
                 "OutputFile"               : "",
                 "MonitorSequence"          : None
                }

    def __globalMonitor(self, stage):
        from Configurables import HltGlobalMonitor
        globalMon = HltGlobalMonitor(stage + "GlobalMonitor")

        from DAQSys.Decoders import DecoderDB
        decRepLoc = DecoderDB["HltDecReportsDecoder/%sDecReportsDecoder" % stage].listOutputs()[0]
        globalMon.HltDecReports = decRepLoc
        globalMon.Stage = stage

        from HltConf.Configuration import onlinePV
        globalMon.VertexLocations = onlinePV()
        return globalMon

    def __groupLines( self, lines , mapping ) :
        import re
        groups = {}
        taken = []
        for pos in range(len(mapping)):
            (name, pattern) = mapping[pos]
            expr = re.compile(pattern)
            groups[ name ] = [ i for i in lines if expr.match(i) and i not in taken ]
            taken += groups[ name ]
        #prune empty groups
        return dict( (k,v) for k,v in groups.iteritems() if v )


    def __l0_monitoring(self, stage):

        from Configurables import HltL0GlobalMonitor
        from DAQSys.Decoders import DecoderDB
        from DAQSys.DecoderClass import decodersForBank

        l0decoder = decodersForBank( DecoderDB, 'L0DU' )
        assert len(l0decoder)
        l0decoder = l0decoder[0].setup()

        decRepLoc = DecoderDB["HltDecReportsDecoder/%sDecReportsDecoder" % stage].listOutputs()[0]
        monitor = HltL0GlobalMonitor(stage + 'L0GlobalMonitor', DecReports = decRepLoc,
                                     HltDecName = stage + "Global")

        if self.getProp("EnableL0Monitor"):
            return [l0decoder, monitor]
        else:
            return []

    def __hlt1_monitoring(self, lines1):
        monSeq = Sequence("Hlt1MonitorSequence", IgnoreFilterPassed = True)

        l0Mon = self.__l0_monitoring("Hlt1")
        monSeq.Members += l0Mon

        # Tell the monitoring what it should expect..
        # the keys are the Labels for the Histograms in the GUI
        # the values are the Pattern Rules to for the Decisions contributing

        ## Global monitor
        from Configurables import HltGlobalMonitor
        globalMon = self.__globalMonitor("Hlt1")
        globalMon.DecToGroup = self.__groupLines( [ i.decision() for i in lines1 ],
                                                  [ ("L0"         , "Hlt1L0.*Decision"),
                                                    ("LumiBeamGas", "Hlt1(Lumi|BeamGas).*Decision"),
                                                    ("SingleMuon" , "Hlt1(Single|Track)Muon.*Decision"),
                                                    ("DiMuon"     , "Hlt1DiMuon.*Decision"),
                                                    ("TrackMVA"   , "Hlt1(Two)?TrackMVADecision"),
                                                    ("ECAL"       , "Hlt1.*(Electron|Photon).*Decision"),
                                                    ("LowMult"    , "Hlt1LowMult.*Decision"),
                                                    ("Beauty"     , "Hlt1B2.*Decision"),
                                                    ("Commissioning", "Hlt1(ODIN.*|Tell1Error|Incident)Decision"),
                                                    ("MinBias"    , "Hlt1MB.*Decision"),
                                                    ("Calibration", "Hlt1CalibTracking.*"),
                                                    ("Global"     , ".*Global.*"),
                                                    ("Other"      , ".*") # add a 'catch all' term to pick up all remaining decisions...
                                                  ]
                                                  )
        if self.getProp("EnableGlobalMonitor"):
            monSeq.Members.append(globalMon)

        ## Mass monitor
        from Configurables import HltMassMonitor
        from DAQSys.Decoders import DecoderDB
        massMon = HltMassMonitor("Hlt1MassMonitor")
        massMon.DecReportsLocation = DecoderDB["HltDecReportsDecoder/Hlt1DecReportsDecoder"].listOutputs()[0]
        massMon.SelReportsLocation = DecoderDB["HltSelReportsDecoder/Hlt1SelReportsDecoder"].listOutputs()[0]
        massMon.Decisions  = {"Jpsi"         : "Hlt1DiMuonHighMassDecision",
                              "JpsiAlign"    : "Hlt1CalibMuonAlignJpsiDecision",
                              "D0->Kpi(det)" : 'Hlt1CalibTrackingKPiDetachedDecision',
                              "D0->Kpi"      : 'Hlt1CalibTrackingKPiDecision',
                              "D0->KK"       : 'Hlt1CalibTrackingKKDecision',
                              "D0->pipi"     : "Hlt1CalibTrackingPiPiDecision",
                              "phi->KK"      : "Hlt1IncPhiDecision"}
        massMon.DecisionStructure = {"Jpsi" : [105.658,105.658]}
        massMon.Histograms = {"Jpsi"         : [ 3010, 3190, 90 ],
                              "JpsiAlign"    : [ 3010, 3190, 90 ],
                              "D0->Kpi(det)" : [ 1815, 1915, 50 ],
                              "D0->Kpi"      : [ 1815, 1915, 50 ],
                              "D0->KK"       : [ 1815, 1915, 50 ],
                              "D0->pipi"     : [ 1815, 1915, 50 ],
                              "phi->KK"      : [ 1000, 1040, 80 ]}
        if self.getProp("EnableMassMonitor"):
            monSeq.Members.append(massMon)

        # Setup the track monitoring
        from Configurables        import Hlt1TrackMonitor
        import HltTracking
        from HltTracking.HltSharedTracking import MinimalVelo, VeloTTTracking, HltHPTTracking
        from Configurables import DeterministicPrescaler
        trackMon = Hlt1TrackMonitor("Hlt1TrackMonitor")
        trackMon.VeloTrackLocation = MinimalVelo.outputSelection()
        trackMon.VeloTTTrackLocation = VeloTTTracking.outputSelection()
        trackMon.ForwardTrackLocation = HltHPTTracking.outputSelection()

        # This is not so nice but currently unavoidable
        from HltTracking.Hlt1TrackNames import Hlt1TrackLoc
        from HltTracking.HltTrackNames import HltDefaultFitSuffix
        trackMon.FittedTrackLocation = Hlt1TrackLoc["FitTrack"]

        from Hlt1Lines.Hlt1GECs import Hlt1GECUnit
        gecUnit = Hlt1GECUnit( self.getProp("Hlt1TrackMonitorGEC") )
        prescaler = DeterministicPrescaler("Hlt1TrackMonitorPrescaler", AcceptFraction = self.getProp("Hlt1TrackMonitorPrescale") )
        trackMonSeq = Sequence('Hlt1TrackMonitorSequence',
                               Members = [ gecUnit, prescaler ] + HltHPTTracking.members() + [ trackMon ])

        if self.getProp("EnableTrackMonitor"):
            monSeq.Members.append(trackMonSeq)

        return monSeq

    def __hlt2_monitoring(self, lines2):
        monSeq = Sequence("Hlt2MonitorSequence", IgnoreFilterPassed = True)

        l0Mon = self.__l0_monitoring("Hlt2")
        monSeq.Members += l0Mon

        # Tell the monitoring what it should expect..
        # the keys are the Labels for the Histograms in the GUI
        # the values are the Pattern Rules to for the Decisions contributing

        from Configurables import HltGlobalMonitor
        globalMon = self.__globalMonitor("Hlt2")
        globalMon.DecToGroup = self.__groupLines( [ i.decision() for i in lines2 ],
                                 [("B2HH",          "Hlt2B2K{,2}Pi{,2}.*Decision"),
                                  ("B2Kpi0",        "Hlt2B2K0?[pP]i0?.*Decision"),
                                  ("Bc2JpsiX",      "Hlt2Bc2JpsiX.*Decision"),
                                  ("CaloTest",      "Hlt2CaloTest.*Decision"),
                                  ("CcDiHadron",    "Hlt2CcDiHadron.*Decision"),
                                  ("CharmHad",      "Hlt2CharmHad.*Decision"),
                                  ("Commissioning", "Hlt2Commissioning.*Decision"),
                                  ("DPS",           "Hlt2DPS.*Decision"),
                                  ("DiMuon",        "Hlt2DiMuon.*Decision"),
                                  ("DisplVertices", "Hlt2DisplVertices.*Decision"),
                                  ("EW",            "Hlt2EW.*Decision"),
                                  ("LowMult",       "Hlt2LowMult.*Decision"),
                                  ("PID",           "Hlt2PID.*Decision"),
                                  ("Phi",           "Hlt2Phi.*Decision"),
                                  ("Radiative",     "Hlt2Radiative.*Decision"),
                                  ("RareCharm",     "Hlt2RareCharm.*Decision"),
                                  ("RareStrange",   "Hlt2RareStrange.*Decision"),
                                  ("RecoTest",      "Hlt2RecoTest.*Decision"),
                                  ("SingleMuon",    "Hlt2SingleMuon.*Decision"),
                                  ("Topo",          "Hlt2Topo.*Decision"),
                                  ("TrackEff",      "Hlt2TrackEff.*Decision"),
                                  ("TrackEffDiMuon","Hlt2TrackEffDiMuon.*Decision"),
                                  ("TriMuon",       "Hlt2TriMuon.*Decision"),
                                  ("XcMuXForTau",   "Hlt2XcMuXForTau.*Decision"),
                                  ("Global",        ".*Global.*"),
                                  ("Other",         ".*") # add a 'catch all' term to pick up all remaining decisions...
                                 ]
                                 )
        if self.getProp("EnableGlobalMonitor"):
            monSeq.Members.append(globalMon)

        from Configurables import HltMassMonitor
        massMon = HltMassMonitor("Hlt2MassMonitor")
        from DAQSys.Decoders import DecoderDB
        massMon.DecReportsLocation = DecoderDB["HltDecReportsDecoder/Hlt2DecReportsDecoder"].listOutputs()[0]
        massMon.SelReportsLocation = DecoderDB["HltSelReportsDecoder/Hlt2SelReportsDecoder"].listOutputs()[0]
        massMon.Decisions  = {"Jpsi"                : "Hlt2DiMuonJPsiDecision",
                              "Psi2S"               : "Hlt2DiMuonPsi2STurboDecision",
                              "D+->Kpipi"           : "Hlt2CharmHadDpToKmPipPipTurboDecision",
                              "Ds+->KKpi"           : "Hlt2CharmHadDspToKmKpPipTurboDecision",
                              "Lambdac->pKpi"       : "Hlt2CharmHadLcpToPpKmPipTurboDecision",
                              "Omega->Lambda(LL)K"  : "Hlt2CharmHadOmm2LamKm_LLLDecision",
                              "Omega->Lambda(DD)K"  : "Hlt2CharmHadOmm2LamKm_DDLDecision",
                              "Xi->Lambda(LL)pi"    : "Hlt2CharmHadXim2LambPim_LLLDecision",
                              "Xi->Lambda(DD)pi"    : "Hlt2CharmHadXim2LambPim_DDLDecision",
                              "D*->(D0->KK)pi"      : "Hlt2CharmHadDstp2D0Pip_D02KmKpTurboDecision",
                              "D*->(D0->Kpi)pi"     : "Hlt2CharmHadDstp2D0Pip_D02KmPipTurboDecision",
                              "D*->(D0->Kpipipi)pi" : "Hlt2CharmHadDstp2D0Pip_D02KmPimPipPipTurboDecision",
                              "Xc->(D0->Kpi)pi"     : "Hlt2CharmHadSpec_D0ToKPi_PiTurboDecision",
                              "Xc->(D+->Kpipi)pi"   : "Hlt2CharmHadSpec_DpPiTurboDecision",
                              "D0->Kpi"             : "Hlt2RareCharmD02KPiDecision",
                              "phi->KK"             : "Hlt2IncPhiDecision"
                              }
        massMon.Histograms = {"Jpsi"                : [3005, 3186, 50],
                              "Psi2S"               : [3600, 3770, 50],
                              "D+->Kpipi"           : [1820.,1920., 100],
                              "Ds+->KKpi"           : [1920., 2020., 100],
                              "Lambdac->pKpi"       : [2235., 2335., 100],
                              "Omega->Lambda(LL)K"  : [1640., 1705., 65],
                              "Omega->Lambda(DD)K"  : [1640., 1705., 65],
                              "Xi->Lambda(LL)pi"    : [1290., 1355., 65],
                              "Xi->Lambda(DD)pi"    : [1290., 1355., 65],
                              "D*->(D0->KK)pi"      : [1990., 2040., 100],
                              "D*->(D0->Kpi)pi"     : [1990.,2040., 100],
                              "D*->(D0->Kpipipi)pi" : [1990.,2040., 100],
                              "Xc->(D0->Kpi)pi"     : [1975.,2975., 200],
                              "Xc->(D+->Kpipi)pi"   : [1995.,2995., 200],
                              "D0->Kpi"             : [1815.,1915., 100],
                              "phi->KK"             : [1000.,1040., 80]
                              }
        if self.getProp("EnableMassMonitor"):
            monSeq.Members.append(massMon)

        return monSeq

    def __rateMonitoring(self):

        hlt1_rates = {  0 : '( ODIN_BXTYP == LHCb.ODIN.Beam1 ) | ( ODIN_BXTYP == LHCb.ODIN.BeamCrossing )'
                     ,  1 : '( ODIN_BXTYP == LHCb.ODIN.Beam2 ) | ( ODIN_BXTYP == LHCb.ODIN.BeamCrossing )'
                     ,  3 : 'ODIN_TRUE'
                     ,  4 : 'ODIN_TRGTYP == LHCb.ODIN.LumiTrigger'
                     ,  5 : 'jbit( ODIN_EVTTYP, 2)' # ODIN NOBIAS
                     ,  6 : 'jbit( ODIN_EVTTYP, 14)' # ODIN LEADING BUNCH
                     ,  8 : 'L0_DECISION_PHYSICS'
                     ,  9 : "L0_CHANNEL_RE('B?gas')"
                     , 10 : "|".join( [ "L0_CHANNEL('%s')" % chan for chan in [ 'CALO','MUON,minbias' ] if chan in L0Channels() ] )
                     , 11 : "|".join( [ "L0_CHANNEL('%s')" % chan for chan in [ 'Electron','Photon','Hadron','Muon','DiMuon',
                                                                                'Muon,lowMult','DiMuon,lowMult','Electron,lowMult',
                                                                                'Photon,lowMult','DiEM,lowMult','DiHadron,lowMult'] if chan in L0Channels() ] )
                     , 12 : "L0_CHANNEL('CALO')" if 'CALO' in L0Channels() else "" # note: need to take into account prescale in L0...
                     , 13 : "L0_CHANNEL( 'Hadron' )" if 'Hadron' in L0Channels() else ""
                     , 14 : "L0_CHANNEL_RE('Electron|Photon')"
                     , 15 : "L0_CHANNEL_RE('Muon|DiMuon')"
                     , 16 : "L0_CHANNEL_RE('.*NoSPD')"
                     , 17 : "L0_CHANNEL_RE('.*,lowMult')"
                     , 18 : "L0_CHANNEL('DiMuon')" if 'DiMuon' in L0Channels() else ""
                     , 19 : "L0_CHANNEL( 'Hadron|SumEt' )" if ('Hadron' in L0Channels() and 'SumEt' in L0Channels()) else ""
                     , 32 : "HLT_PASS('Hlt1Global')"
                     , 38 : "HLT_PASS('Hlt1ODINTechnicalDecision')"
                     , 39 : "HLT_PASS_SUBSTR('Hlt1L0')"
                     , 41 : "HLT_PASS_RE('Hlt1(Single|Track)Muon.*Decision')"
                     , 42 : "HLT_PASS_RE('Hlt1.*DiMuon.*Decision')"
                     , 43 : "HLT_PASS_RE('Hlt1.*MuonNoL0.*Decision')"
                     , 44 : "HLT_PASS_RE('Hlt1.*Electron.*Decision')"
                     , 45 : "HLT_PASS_RE('Hlt1.*Gamma.*Decision')"
                     , 47 : "HLT_PASS_RE('Hlt1MBMicroBias.*Decision')"
                     , 49 : "HLT_PASS_RE('Hlt1.*MVA.*Decision')"
                     , 50 : "HLT_PASS('Hlt1LumiLowBeamCrossingDecision')"
                     , 51 : "HLT_PASS('Hlt1LumiMidBeamCrossingDecision')"
                     , 60 : "HLT_PASS('Hlt1TrackAllL0Decision')"
                     }


        hlt2_rates = { 64 : "HLT_PASS('Hlt2Global')"
                        , 65 : "HLT_PASS('Hlt2DebugEventDecision')"
                        , 66 : "HLT_PASS_RE('Hlt2(?!Transparent).*Decision')"
                        , 67 : "HLT_PASS_RE('Hlt2.*SingleMuon.*Decision')"
                        , 68 : "HLT_PASS_RE('Hlt2.*DiMuon.*Decision')"
                        , 69 : "HLT_PASS_RE('Hlt2.*DY.*Decision')"
                        , 70 : "HLT_PASS_RE('Hlt2.*Topo.*Decision')"
                        , 71 : "HLT_PASS_RE('Hlt2.*CharmHad.*Decision')"
                        , 72 : "HLT_PASS_RE('Hlt2.*IncPhi.*Decision')"
                        , 73 : "HLT_PASS_RE('Hlt2.*Gamma.*Decision')"
                        , 74 : "HLT_PASS_RE('Hlt2.*TriMuon.*Decision')"
                        , 75 : "HLT_PASS_RE('Hlt2.*RareCharm.*Decision')"
                        , 76 : "HLT_PASS_RE('Hlt2.*DisplVertices.*Decision')"
                        , 77 : "HLT_PASS_RE('Hlt2(?!Forward)(?!DebugEvent)(?!Lumi)(?!Transparent)(?!PassThrough).*Decision')"
                        , 78 : "HLT_PASS_RE('Hlt2.*Muon.*Decision')"
                        , 79 : "HLT_PASS_RE('Hlt2.*Incl.*HHX.*Decision')"
                        , 80 : "HLT_PASS_RE('Hlt2.*Electron.*Decision')"
                        , 81 : "HLT_PASS_RE('Hlt2Topo.*2Body.*Decision')"
                        , 82 : "HLT_PASS_RE('Hlt2Topo.*3Body.*Decision')"
                        , 83 : "HLT_PASS_RE('Hlt2Topo.*4Body.*Decision')"
                        , 84 : "HLT_PASS_RE('Hlt2TopoMu[234]Body.*Decision')"
                        , 85 : "HLT_PASS_RE('Hlt2TopoE[234]Body.*Decision')"
                        , 86 : "HLT_PASS_RE('Hlt2Topo[234]Body.*Decision')"}

    
    def __configureOutput(self):
        histoFile = self.getProp("OutputFile")
        if not histoFile:
            return

        from Configurables import HistogramPersistencySvc, RootHistCnv__PersSvc
        HistogramPersistencySvc().OutputFile = histoFile
        RootHistCnv__PersSvc().OutputFile = histoFile

    def configureHltMonitoring(self, lines1, lines2):
        """
        HLT Monitoring configuration
        """
        ## Only do things here that need to know the list of instantiated HLT
        ## lines. This function is called from the postConfigAction of
        ## HltConf.Configuration.

        monSeq = self.getProp("MonitorSequence")
        monSeq.IgnoreFilterPassed = True  # enforce execution of all sub-sequences
        if lines1:
            hlt1Mon = self.__hlt1_monitoring(lines1)
            monSeq.Members += [hlt1Mon]
        if lines2:
            hlt2Mon = self.__hlt2_monitoring(lines2)
            monSeq.Members += [hlt2Mon]

        # Disable production of histograms for all (most) of the algorithms
        from HltLine.HltLine     import hlt1Lines, hlt2Lines
        if   self.getProp('HistogrammingLevel') == 'None' :
            for i in hlt1Lines()+hlt2Lines() : _disableHistograms( i.configurable() )
        elif self.getProp('HistogrammingLevel') == 'Line' :
            for i in hlt1Lines()+hlt2Lines() : _disableHistograms( i.configurable(), lambda x: x.getType()!='Hlt::Line' )
        elif self.getProp('HistogrammingLevel') == 'NotLine' :
            for i in hlt1Lines()+hlt2Lines() : _disableHistograms( i.configurable(), lambda x: x.getType()=='Hlt::Line' )
        else:
            ValueError("HltMonitoringConf: HistogrammingLevel property must be set to 'None', 'Line' or 'NotLine'.")

        # Enable production of histograms for some algorithms, see _enableMonitoring()
        if self.getProp('EnableAlgMonitoring') :
            for i in hlt1Lines()+hlt2Lines() : _recurse( i.configurable(),_enableMonitoring )

    def __apply_configuration__(self):
        ## Only do things here that do not need to know the list of instantiated
        ## HLT lines.
        if not self.getProp("MonitorSequence"):
            raise ValueError("HltMonitoringConf: MonitorSequence property must be set.")
        self.__configureOutput()
