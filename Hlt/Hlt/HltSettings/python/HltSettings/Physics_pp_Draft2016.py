from GaudiKernel.SystemOfUnits import GeV, MeV, mm
import importlib

def __get_conf__(folder, suffix):
    conf = folder + suffix
    module = importlib.import_module("HltSettings.{0}.{1}".format(folder, conf))
    return getattr(module, conf)()

def __update_conf__( current, extra ) :
    for (conf,d) in extra.iteritems() :
        if conf not in current :
            current[conf] = d
            continue
        cur = current[conf]
        for (k,v) in d.iteritems() :
            if k not in cur :
                cur[k] = v
                continue
            if cur[k] == v : continue
            print 'Warning: potential collision detected: %s -- %s' % (conf,k)
            print 'current: %s' % cur[k]
            print 'request: %s' % v
            if type(cur[k])==dict :
                cur[k].update( v )
            else :
                cur[k] = v
            print 'result: %s' % cur[k]

class Physics_pp_Draft2016( object ):
    """
    Settings 25ns physics in October 2015

    WARNING :: DO NOT EDIT WITHOUT PERMISSION OF THE HLT GROUP

    @author R. Aaij
    @date 2014-12-10
    """

    __all__ = ( 'ActiveHlt1Lines', 'ActiveHlt2Lines', 'Thresholds', 'L0TCK' )

    def verifyType(self,ref) :
        # verify self.ActiveLines is still consistent with
        # our types self.ActiveHlt2Lines and self.ActiveHlt1Lines...
        # so we can force that classes which inherit from us
        # and overrule either ActiveHlt.Lines also overrule
        # HltType...
        if ( self.ActiveHlt1Lines() != ref.ActiveHlt1Lines(self)  or
             self.ActiveHlt2Lines() != ref.ActiveHlt2Lines(self) ) :
            raise RuntimeError( 'Must update HltType when modifying ActiveHlt.Lines()' )

    def L0TCK(self) :
        return '0x0050'

    def HltType(self) :
        self.verifyType( Physics_pp_Draft2016 )
        return          'Physics_pp_Draft2016'

    def SubDirs(self):
        return {'August2015' : ['RareStrange',
                                'TriMuon','Phi'],
                'Draft2016': ['CcDiHadron', 'DPS', 'Bc2JpsiX','B2HH','B2Kpi0','BHad','CharmHad','DiElectron','Exotica',
                              'Jets','LFV','LowMult','PID','Radiative','RareCharm','EW',
                              'Topo','TrackEffDiMuon','DisplVertices','XcMuXForTau','Technical','TrackEff',
                              'DiMuon', 'SingleMuon','Majorana','RareStrange']}

    def Thresholds(self) :
        """
        Returns a dictionary of cuts
        """

        from Hlt1Lines.Hlt1TrackLines           import Hlt1TrackLinesConf
        from Hlt1Lines.Hlt1MuonLines            import Hlt1MuonLinesConf
        from Hlt1Lines.Hlt1ElectronLines        import Hlt1ElectronLinesConf
        from Hlt1Lines.Hlt1L0Lines              import Hlt1L0LinesConf
        from Hlt1Lines.Hlt1MBLines              import Hlt1MBLinesConf
        from Hlt1Lines.Hlt1CommissioningLines   import Hlt1CommissioningLinesConf
        from Hlt1Lines.Hlt1BeamGasLines         import Hlt1BeamGasLinesConf
        from Hlt1Lines.Hlt1MVALines             import Hlt1MVALinesConf
        from Hlt1Lines.Hlt1CalibTrackingLines   import Hlt1CalibTrackingLinesConf
        from Hlt1Lines.Hlt1LowMultLines         import Hlt1LowMultLinesConf
        from Hlt1Lines.Hlt1CalibRICHMirrorLines import Hlt1CalibRICHMirrorLinesConf

        thresholds = { Hlt1TrackLinesConf :   {  'AllL0_Velo_NHits'  : 9
                                               , 'AllL0_Velo_Qcut'   : 3
                                               , 'AllL0_TrNTHits'    : 16
                                               , 'AllL0_PT'          : 1300.
                                               , 'AllL0_P'           : 6000.
                                               , 'AllL0_IPChi2'      : 13.
                                               , 'AllL0_TrChi2'      : 3.
                                               , 'AllL0_GEC'         : 'Loose'
                                               , 'Muon_TrNTHits'     : 0 #OFF
                                               , 'Muon_Velo_NHits'   : 0 #OFF
                                               , 'Muon_Velo_Qcut'    : 999 #OFF
                                               , 'Muon_PT'           : 910.
                                               , 'Muon_P'            : 6000.
                                               , 'Muon_IPChi2'       : 10.
                                               , 'Muon_TrChi2'       : 3.
                                               , 'Muon_GEC'          : 'Loose'
                                               , 'MuonNoSPD_TrNTHits'     : 0 #OFF
                                               , 'MuonNoSPD_Velo_NHits'   : 0 #OFF
                                               , 'MuonNoSPD_Velo_Qcut'    : 999 #OFF
                                               , 'MuonNoSPD_PT'           : 910.
                                               , 'MuonNoSPD_P'            : 6000.
                                               , 'MuonNoSPD_IPChi2'       : 10.
                                               , 'MuonNoSPD_TrChi2'       : 3.
                                               , 'MuonNoSPD_GEC'          : 'Loose'
                                               , 'Photon_PT'         : 1200.
                                               , 'Photon_P'          : 6000.
                                               , 'Photon_IPChi2'     : 13.
                                               , 'Photon_TrChi2'     : 4.
                                               , 'Photon_GEC'        : 'Loose'
                                               , 'L0Channels'        : {'AllL0'  : 'L0_DECISION_PHYSICS',
                                                                        'Muon'   : ('Muon', 'DiMuon'),
                                                                        'MuonNoSPD'   : ['MuonNoSPD' ],
                                                                        'Photon' : ("Photon", "Electron")}
                                               }
                     , Hlt1MVALinesConf :     {'DoTiming'                     : False,
                                               'TrackMVA' :    {'TrChi2'      :     2.5,
                                                                'MinPT'       :  1000.  * MeV,
                                                                'MaxPT'       : 25000.  * MeV,
                                                                'MinIPChi2'   :     7.4,
                                                                'Param1'      :     1.0,
                                                                'Param2'      :     1.0,
                                                                'Param3'      :     1.6,
                                                                'GEC'         : 'Loose'},
                                               'TwoTrackMVA' : {'P'           :  5000. * MeV,
                                                                'PT'          :   500. * MeV,
                                                                'TrChi2'      :     2.5,
                                                                'IPChi2'      :     4.,
                                                                'MinMCOR'     :  1000. * MeV,
                                                                'MaxMCOR'     :   1e9  * MeV,
                                                                'MinETA'      :     2.,
                                                                'MaxETA'      :     5.,
                                                                'MinDirA'     :     0.,
                                                                'V0PT'        :  2000. * MeV,
                                                                'VxChi2'      :    10.,
                                                                'Threshold'   :     0.95,
                                                                'MvaVars'     : {'chi2'   : 'VFASPF(VCHI2)',
                                                                                 'fdchi2' : 'BPVVDCHI2',
                                                                                 'sumpt'  : 'SUMTREE(PT, ISBASIC, 0.0)',
                                                                                 'nlt16'  : 'NINTREE(ISBASIC & (BPVIPCHI2() < 16))'},
                                                                'Classifier'  : {'Type'   : 'MatrixNet',
                                                                                 'File'   : '$PARAMFILESROOT/data/Hlt1TwoTrackMVA.mx'},
                                                                'GEC'         : 'Loose'},
                                               'L0Channels'  : {'TrackMVA'    : 'L0_DECISION_PHYSICS',
                                                                'TwoTrackMVA' : 'L0_DECISION_PHYSICS'},
                                               'Priorities'  : {'TrackMVA'    : 1,
                                                                'TwoTrackMVA' : 2}
                                               }
                     , Hlt1ElectronLinesConf : { 'SingleElectronNoIP_P'          : 20000
                                               , 'SingleElectronNoIP_PT'         : 10000
                                               , 'SingleElectronNoIP_TrChi2'     :     3
                                               , 'SingleElectronNoIP_TrNTHits'   :     0 #OFF
                                               , 'SingleElectronNoIP_Velo_NHits' :     0 #OFF
                                               , 'SingleElectronNoIP_Velo_Qcut'  :   999 #OFF
                                               , 'SingleElectronNoIP_GEC'        : 'Loose'
                                               , 'L0Channels': { 'SingleElectronNoIP' : ( 'Electron', ) }
                                               }
                     , Hlt1MuonLinesConf :     { 'SingleMuonHighPT_P'       : 6000
                                               , 'SingleMuonHighPT_PT'      : 4340
                                               , 'SingleMuonHighPT_TrChi2'  :    3.
                                               , 'SingleMuonHighPT_GEC'     : 'Loose'
                                               , 'SingleMuonNoIP_P'         : 6000
                                               , 'SingleMuonNoIP_PT'        : 1400
                                               , 'SingleMuonNoIP_TrChi2'    :    3.
                                               , 'SingleMuonNoIP_GEC'       : 'Loose'
                                               , 'DiMuonLowMass_VxDOCA'     :  0.2
                                               , 'DiMuonLowMass_VxChi2'     :   25
                                               , 'DiMuonLowMass_P'          : 3000
                                               , 'DiMuonLowMass_PT'         :    0
                                               , 'DiMuonLowMass_TrChi2'     :    4
                                               , 'DiMuonLowMass_M'          :    0.
                                               , 'DiMuonLowMass_IPChi2'     :    4.
                                               , 'DiMuonLowMass_GEC'        : 'Loose'
                                               , 'DiMuonHighMass_VxDOCA'    :  0.2
                                               , 'DiMuonHighMass_VxChi2'    :   25
                                               , 'DiMuonHighMass_P'         : 6000
                                               , 'DiMuonHighMass_PT'        :  300
                                               , 'DiMuonHighMass_TrChi2'    :    3
                                               , 'DiMuonHighMass_M'         : 2700
                                               , 'DiMuonHighMass_GEC'       : 'Loose'
                                               , 'DiMuonNoIP_VxDOCA'    :  0.1
                                               , 'DiMuonNoIP_VxChi2'    :    9.
                                               , 'DiMuonNoIP_P'         : 20000.
                                               , 'DiMuonNoIP_PT'        : 1000.
                                               , 'DiMuonNoIP_TrChi2'    :  3.
                                               , 'DiMuonNoIP_M'         :  0.
                                               , 'DiMuonNoIP_GEC'       : 'Loose'
                                               , 'DiMuonNoL0_VxDOCA'     :  0.2
                                               , 'DiMuonNoL0_VxChi2'     :   25.
                                               , 'DiMuonNoL0_P'          : 3000.
                                               , 'DiMuonNoL0_PT'         :   80.
                                               , 'DiMuonNoL0_TrChi2'     :    4.
                                               , 'DiMuonNoL0_M'          :    0.
                                               , 'DiMuonNoL0_IPChi2'     :    9.
                                               , 'DiMuonNoL0_GEC'        : 'Tight'
                                               , 'MultiMuonNoL0_P'          : 6000
                                               , 'MultiMuonNoL0_PT'         :  500
                                               , 'MultiMuonNoL0_TrChi2'     :    3.
                                               , 'MultiMuonNoL0_GT'         :    2.5
                                               , 'MultiMuonNoL0_GEC'        : 'Loose'
                                               , 'MultiDiMuonNoIP_VxDOCA'     :  0.1
                                               , 'MultiDiMuonNoIP_VxChi2'     :    9.
                                               , 'MultiDiMuonNoIP_P'          : 10000.
                                               , 'MultiDiMuonNoIP_PT'         :  500.
                                               , 'MultiDiMuonNoIP_TrChi2'     :    3.
                                               , 'MultiDiMuonNoIP_M'          :    0.
                                               , 'MultiDiMuonNoIP_MSS'        : 200000.
                                               , 'MultiDiMuonNoIP_IPChi2'     :    -1
                                               , 'MultiDiMuonNoIP_NMinDiMu'   :    1.
                                               , 'MultiDiMuonNoIP_GEC'        : 'Loose'
                                               ,'L0Channels'               : {'SingleMuonHighPT' : ( 'Muon', 'MuonEW'),
                                                                              'SingleMuonNoIP'   : ( 'Muon', ),
                                                                              'DiMuonLowMass'    : ( 'Muon', 'DiMuon' ),
                                                                              'DiMuonHighMass'   : ( 'Muon', 'DiMuon' ),
                                                                              'DiMuonNoIP'   : ( 'Muon', 'DiMuon' ),
                                                                              'MultiDiMuonNoIP'   : ( 'Muon', 'DiMuon' ),
                                                                              'DiMuonNoL0'       : None,
                                                                              'MultiMuonNoL0'    : None
                                                                             }
                                               , 'Prescale'                 : { 'Hlt1NoPVPassThrough': 1.0,
                                                                                'Hlt1SingleMuonNoIP' : 0.01,
                                                                                'Hlt1DiMuonNoL0'  :  1.0,
                                                                                'Hlt1MultiMuonNoL0'  : 0.01
                                                                                }
                                               }
                       , Hlt1L0LinesConf :     {  'Postscale' : { 'Hlt1L0AnyRateLimited'       : 'RATE(1)'
                                                                , 'Hlt1L0AnyNoSPDRateLimited'  : 'RATE(1)'
                                                                }
                                               ,  'Prescale' : { 'Hlt1L0HighSumETJet' : 1
                                                               , 'Hlt1L0AnyNoSPD'     : 0.01
                                                               }
                                               }
                       , Hlt1BeamGasLinesConf : {
                                         'Prescale'  : { 'Hlt1BeamGasCrossingForcedRecoFullZ': 0.001 }
                                       , 'Postscale' : { 'Hlt1BeamGasNoBeamBeam1'            : 'RATE(0.5)'
                                                       , 'Hlt1BeamGasNoBeamBeam2'            : 'RATE(0.5)'
                                                       , 'Hlt1BeamGasBeam1'                  : 1.
                                                       , 'Hlt1BeamGasBeam2'                  : 1.
                                                       , 'Hlt1BeamGasCrossingEnhancedBeam1'  : 'RATE(0)'
                                                       , 'Hlt1BeamGasCrossingEnhancedBeam2'  : 'RATE(0)'
                                                       , 'Hlt1BeamGasCrossingForcedReco'     : 'RATE(0.5)'
                                                       , 'Hlt1BeamGasCrossingForcedRecoFullZ': 'RATE(0.5)'
                                                       , 'Hlt1BeamGasCrossingParasitic'      : 'RATE(1)'
                                                       , 'Hlt1BeamGasHighRhoVertices'        : 'RATE(4)'
                                                       }
                                                }
                       , Hlt1CalibTrackingLinesConf :  { 'ParticlePT'            : 600     # MeV
                                                        ,'ParticleP'             : 4000    # MeV
                                                        ,'TrackCHI2DOF'          : 2       # dimensionless
                                                        ,'CombMaxDaughtPT'       : 900     # MeV 900
                                                        ,'CombAPT'               : 1800    # MeV 1200
                                                        ,'CombDOCA'              : 0.1     # mm
                                                        ,'CombVCHI2DOF'          : 10      # dimensionless
                                                        ,'CombVCHI2DOFLoose'     : 15      # dimensionless
                                                        ,'CombDIRA'              : 0.99    # dimensionless
                                                        ,'CombTAU'               : 0.25    # ps
                                                        ,'D0MassWinLoose'        : 100     # MeV
                                                        ,'D0MassWin'             : 60      # MeV
                                                        ,'B0MassWinLoose'        : 200     # MeV
                                                        ,'B0MassWin'             : 150     # MeV
                                                        ,'D0DetachedDaughtsIPCHI2': 9      # dimensionless
                                                        ,'D0DetachedIPCHI2'       : 9      # dimensionless
                                                        ,'BsPhiGammaMassMinLoose': 3350    # MeV
                                                        ,'BsPhiGammaMassMaxLoose': 6900    # MeV
                                                        ,'BsPhiGammaMassMin'     : 3850    # MeV
                                                        ,'BsPhiGammaMassMax'     : 6400    # MeV
                                                        ,'PhiMassWinLoose'       : 50      # MeV
                                                        ,'PhiMassWin'            : 30      # MeV
                                                        ,'PhiMassWinTight'       : 20      # MeV
                                                        ,'PhiPT'                 : 1800    # MeV
                                                        ,'PhiPTLoose'            : 800     # MeV
                                                        ,'PhiSumPT'              : 3000    # MeV
                                                        ,'PhiIPCHI2'             : 16      # dimensionless
                                                        ,'B0SUMPT'               : 4000    # MeV
                                                        ,'B0PT'                  : 1000    # MeV
                                                        ,'GAMMA_PT_MIN'          : 2000    # MeV
                                                        ,'Velo_Qcut'             : 999     # OFF
                                                        ,'TrNTHits'              : 0       # OFF
                                                        ,'ValidateTT'            : False
                                                        ,'Prescale'  : { 'Hlt1CalibTrackingKPi': 0.2 }
                                                       }
                       , Hlt1CommissioningLinesConf : { 'Postscale' : { 'Hlt1ErrorEvent'   : 'RATE(0.01)'

                                                                        }
                                                        }
                       # micro bias lines switched off for high mu physics running
                       , Hlt1MBLinesConf :     { 'Prescale' : { 'Hlt1MBMicroBiasVelo'                : 0
                                                              , 'Hlt1MBMicroBiasTStation'            : 0
                                                              , 'Hlt1MBMicroBiasVeloRateLimited'     : 0
                                                              , 'Hlt1MBMicroBiasTStationRateLimited' : 0 }
                                               , 'MaxNoBiasRate' : 1000000.
                                               }
                       , Hlt1CalibRICHMirrorLinesConf : { 'Prescale' : { 'Hlt1CalibHighPTLowMultTrks'     : 0.0001,
                                                                         'Hlt1CalibRICHMirrorRICH1'       : 0.281,
                                                                         'Hlt1CalibRICHMirrorRICH2'       : 1.0}
                                                          , 'DoTiming' : False
                                                          , 'R2L_PT'       : 500. * MeV
                                                          , 'R2L_P'        : 40000. * MeV
                                                          , 'R2L_MinETA'   : 2.65
                                                          , 'R2L_MaxETA'   : 2.80
                                                          , 'R2L_Phis'     : [(-2.59, -2.49), (-0.65, -0.55), (0.55, 0.65), (2.49, 2.59)]
                                                          , 'R2L_TrChi2'   : 2.
                                                          , 'R2L_MinTr'    : 0.5
                                                          , 'R2L_GEC'      : 'Loose'
                                                          , 'R1L_PT'       : 500. * MeV
                                                          , 'R1L_P'        : 20000. * MeV
                                                          , 'R1L_MinETA'   : 1.6
                                                          , 'R1L_MaxETA'   : 2.04
                                                          , 'R1L_Phis'     : [(-2.65, -2.30 ), (-0.80, -0.50), (0.50, 0.80), (2.30, 2.65)]
                                                          , 'R1L_TrChi2'   : 2.
                                                          , 'R1L_MinTr'    : 0.5
                                                          , 'R1L_GEC'      : 'Loose'
                                                          , 'LM_PT'    : 500. * MeV
                                                          , 'LM_P'     : 1000. * MeV
                                                          , 'LM_TrChi2': 2.
                                                          , 'LM_MinTr' : 1
                                                          , 'LM_MaxTr' : 40
                                                          , 'LM_GEC'   : 'Loose'
                                                          }
                       , Hlt1LowMultLinesConf : { 'Prescale' : {"Hlt1LowMultPassThrough" : 0.01}
                                                , 'SpdMult'  :   100.   # dimensionless, Spd Multiplicity cut
                                                , 'VeloCut_Hadrons_MinVelo'     :     2    # dimensionless
                                                , 'VeloCut_Hadrons_MaxVelo'     :     8    # dimensionless
                                                , 'VeloCut_Hadrons_SpdMult'     :    20    # dimensionless
                                                , 'VeloCut_Leptons_MinVelo'     :     0    # dimensionless
                                                , 'VeloCut_Leptons_MaxVelo'     :   100    # dimensionless
                                                , 'MaxVeloCut_MaxVelo'          :     8    # dimensionless
                                                , 'MaxNVelo'                    :  1000    # dimensionless,
                                                , 'MinNVelo'                    :     2    # dimensionless,
                                                , 'TrChi2'                      :     5.   # dimensionless,
                                                , 'PT'                          :   500.   # MeV
                                                , 'P'                           :     0.   # MeV
                                                , 'LowMultLineL0Dependency'     :             "( L0_CHANNEL_RE('.*lowMult') & ~(L0_CHANNEL_RE('Photon,lowMult')) )"
                                                , 'LowMultVeloCut_HadronsLineL0Dependency'  : "( L0_CHANNEL_RE('DiHadron,lowMult') )"
                                                , 'LowMultVeloCut_LeptonsLineL0Dependency'  : "( L0_CHANNEL_RE('Muon,lowMult') | L0_CHANNEL_RE('DiMuon,lowMult') | L0_CHANNEL_RE('Electron,lowMult') )"
                                                , 'LowMultMaxVeloCutLineL0Dependency'       : "( L0_CHANNEL_RE('Photon,lowMult') | L0_CHANNEL_RE('DiEM,lowMult') )"
                                                , 'LowMultPassThroughLineL0Dependency'      : "L0_CHANNEL_RE('.*lowMult')"
                                                , 'NoBiasTriggerType'                       : 'LumiTrigger' # dimensionless
                                                , 'NoBiasBeamCrossingTypeVeto'              : 'BeamCrossing' # dimensionless
                                                }
                       }

        # HLT2 thresholds from individual files
        sds = self.SubDirs()
        for monthyear, subdirs in self.SubDirs().iteritems():
            for subdir in subdirs:
                conf = __get_conf__(subdir, "_25ns_%s" %(monthyear))
                __update_conf__(thresholds, conf.Thresholds())

        return thresholds

    def ActiveHlt2Lines(self) :
        """
        Returns a list of active lines
        """

        hlt2 = []
        sds = self.SubDirs()
        for monthyear, subdirs in self.SubDirs().iteritems():
            for subdir in subdirs:
                conf = __get_conf__(subdir, "_25ns_%s" %(monthyear))
                hlt2.extend(conf.ActiveHlt2Lines())

        return hlt2

    def ActiveHlt1Lines(self) :
        """
        Returns a list of active lines
        """
        lines =  [ 'Hlt1TrackMuon'
                 , 'Hlt1TrackMuonNoSPD'
                 , 'Hlt1SingleMuonHighPT'
                 , 'Hlt1SingleMuonNoIP', 'Hlt1DiMuonNoL0', 'Hlt1DiMuonLowMass', 'Hlt1DiMuonHighMass'
                 , 'Hlt1MultiMuonNoL0', 'Hlt1DiMuonNoIP', 'Hlt1MultiDiMuonNoIP'
                 , 'Hlt1B2GammaGamma'
                 , 'Hlt1B2PhiPhi_LTUNB'
                 , 'Hlt1SingleElectronNoIP'
                 , 'Hlt1TrackMVA', 'Hlt1TwoTrackMVA'
                 , 'Hlt1CalibTrackingKPi' , 'Hlt1CalibTrackingKK' , 'Hlt1CalibTrackingPiPi'
                 , 'Hlt1CalibHighPTLowMultTrks', 'Hlt1CalibTrackingKPiDetached'
                 , 'Hlt1CalibMuonAlignJpsi'
                 , 'Hlt1B2HH_LTUNB_KPi' , 'Hlt1B2HH_LTUNB_KK' , 'Hlt1B2HH_LTUNB_PiPi'
                 , 'Hlt1IncPhi'
                 , 'Hlt1B2PhiGamma_LTUNB'
                 , 'Hlt1CalibRICHMirrorRICH1'
                 , 'Hlt1CalibRICHMirrorRICH2'
                 , 'Hlt1DiProton'
                 , 'Hlt1DiProtonLowMult'
                 , 'Hlt1LowMult'
                 , 'Hlt1LowMultVeloCut_Hadrons'
                 , 'Hlt1LowMultVeloCut_Leptons'
                 , 'Hlt1LowMultMaxVeloCut'
                 , 'Hlt1LowMultPassThrough'
                 , 'Hlt1NoBiasNonBeamBeam'
                   , 'Hlt1Bottomonium2PhiPhi'
                   , 'Hlt1Bottomonium2KstarKstar']
                   
                   
                   

        from Hlt1TechnicalLines import Hlt1TechnicalLines
        lines.extend( Hlt1TechnicalLines().ActiveHlt1Lines() )

        return lines
