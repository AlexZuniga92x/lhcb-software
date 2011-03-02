from Configurables import HltANNSvc 


# quick hack to make sure Hlt2Global is OK...
Hlt2ID = HltANNSvc().Hlt2SelectionID
if "Hlt2Global" not in Hlt2ID : Hlt2ID.update( {  "Hlt2Global" : 2 } )
Hlt2ID.update(  { 'Hlt2PileUpDecision'                    : 65534 } )

HltANNSvc().Hlt1SelectionID = {
    # Global
    "Hlt1Global"                            :    1,
    # L0 (prescaled) Hlt1L0.*Decision
    "Hlt1L0MuonDecision"                    :   10,
    "Hlt1L0MuonNoGlobDecision"              :   11,
    "Hlt1L0DiMuonDecision"                  :   12,
    "Hlt1L0Muon,lowMultDecision"            :   13,
    "Hlt1L0DiMuon,lowMultDecision"          :   14,
    "Hlt1L0ElectronDecision"                :   15,
    "Hlt1L0PhotonDecision"                  :   16,
    "Hlt1L0HadronDecision"                  :   17,
    "Hlt1L0GlobalPi0Decision"               :   18,
    "Hlt1L0LocalPi0Decision"                :   19,
    "Hlt1L0PUDecision"                      :   20,
    "Hlt1L0PU20Decision"                    :   21,
    "Hlt1L0SPDDecision"                     :   22,
    "Hlt1L0SPD40Decision"                   :   23,
    "Hlt1L0B1gasDecision"                   :   24,
    "Hlt1L0B2gasDecision"                   :   25,
    "Hlt1L0MUON,minbiasDecision"            :   26,
    # Misc..
    "Hlt1IncidentDecision"                  :   30,
    "Hlt1Tell1ErrorDecision"                :   31,
    "Hlt1ErrorEventDecision"                :   32,
    "Hlt1NZSVeloDecision"                   :   33,
    # LU Hlt1Lumi.*Decision
    "Hlt1LumiDecision"                      :   40, 
    "Hlt1LumiLowNoBeamDecision"             :   41, 
    "Hlt1LumiLowBeamCrossingDecision"       :   42, 
    "Hlt1LumiLowBeam1Decision"              :   43, 
    "Hlt1LumiLowBeam2Decision"              :   44,
    "Hlt1LumiNoBeamDecision"                :   45, 
    "Hlt1LumiBeamCrossingDecision"          :   46, 
    "Hlt1LumiBeam1Decision"                 :   47, 
    "Hlt1LumiBeam2Decision"                 :   48,
    "Hlt1LumiLowDecision"                   :   49,
    # BG  Hlt1BeamGas.*Decision
    "Hlt1BeamGasNoBeamBeam1Decision"        :   50,
    "Hlt1BeamGasNoBeamBeam2Decision"        :   51,
    "Hlt1BeamGasBeam1Decision"              :   52,
    "Hlt1BeamGasBeam2Decision"              :   53,
    "Hlt1BeamGasCrossingEnhancedBeam1Decision": 54,
    "Hlt1BeamGasCrossingEnhancedBeam2Decision": 55,
    "Hlt1BeamGasCrossingParasiticDecision"  :   56,
    "Hlt1BeamGasCrossingForcedRecoDecision" :   57,
    # VE Hlt1Velo.*Decision
    #"Hlt1VeloClosingDecision"               :   60,
    "Hlt1VeloASideDecision"                 :   61,
    "Hlt1VeloCSideDecision"                 :   62,
    # PA
    "Hlt1ODINTechnicalDecision"             :   70,
    "Hlt1ODINPhysicsDecision"               :   71,
    "Hlt1L0ForcedDecision"                  :   72,
    "Hlt1L0AnyDecision"                     :   73,
    "Hlt1L0AnyRateLimitedDecision"          :   74,
    # XP
    "Hlt1XPressDecision"                    :   80,
    "Hlt1AlignVeloDecision"                 :   81,
    #  more lumi:
    "Hlt1LumiLowNoBeamRateLimitedDecision"  :   90,
    "Hlt1LumiLowBeam2RateLimitedDecision"   :   91,
    "Hlt1LumiLowBeam1RateLimitedDecision"   :   92,
    "Hlt1LumiLowBeamCrossingRateLimitedDecision"  :   93,
    "Hlt1LumiMidBeamCrossingDecision"       :   94,
    # MU Hlt1(Single|Di)Muon.*Decision
    "Hlt1SingleMuonNoIPL0Decision"          :  100,
    "Hlt1SingleMuonNoIPGECDecision"         :  101,
    "Hlt1NoPVPassThroughDecision"           :  102,
    "Hlt1SingleMuonIPCL0Decision"           :  105,
    "Hlt1SingleMuonIPCGECDecision"          :  106,
    "Hlt1DiMuonNoIPL0DiDecision"            :  110,
    "Hlt1DiMuonNoIP2L0Decision"             :  111,
    "Hlt1DiMuonNoIPL0SegDecision"           :  112,
    "Hlt1DiMuonNoIPL0GECDecision"           :  113,
    "Hlt1DiMuonNoIPGECSegDecision"          :  114,
    "Hlt1DiMuonIPCL0DiDecision"             :  120,
    "Hlt1DiMuonIPC2L0Decision"              :  121,
    "Hlt1DiMuonIPCL0SegDecision"            :  122,
    "Hlt1DiMuonIPCL0GRCDecision"            :  123,
    "Hlt1DiMuonIPCGECSegDecision"           :  124,
    "Hlt1DiMuonNoPVL0DiDecision"            :  130,
    "Hlt1DiMuonNoPV2L0Decision"             :  131,
    "Hlt1DiMuonNoPVL0SegDecision"           :  132,
    "Hlt1DiMuonIPCL0GECDecision"            :  133,
    # HA Hl1(Single,Di)Hadron.*Decision
    "Hlt1SingleHadronDecision"              :  200,
    "Hlt1DiHadronDecision"                  :  201,
    "Hlt1DiHadronLTUnbiasedDecision"        :  202,
    "Hlt1TrackAllL0Decision"                :  203,
    "Hlt1TrackMuonDecision"                 :  204,
    "Hlt1TrackPhotonDecision"               :  205,  
    # PH
    "Hlt1PhotonDecision"                    :  300,
    "Hlt1PhoFromEleDecision"                :  301,
    "Hlt1PhotonFromEleDecision"             :  302,
    "Hlt1PhotonTrackDecision"               :  303,
    "Hlt1PhotonFromEleTrackDecision"        :  304,
    "Hlt1PhotonDiTrackDecision"             :  305,
    "Hlt1PhotonFromEleDiTrackDecision"      :  306,
    "Hlt1PhotonDiTrackFromEleDecision"      :  307,
    "Hlt1PhotonTrackFromEleDecision"        :  308,
    # EL
    "Hlt1SingleElectronDecision"            :  400,
    "Hlt1ElectronFromPi0TrackNoIPDecision"  :  401,
    "Hlt1ElectronFromPi0TrackWithIPDecision":  402,
    "Hlt1ElectronTrackDecision"             :  403,
    "Hlt1ElectronTrackNoIPDecision"         :  404,
    "Hlt1ElectronTrackWithIPDecision"       :  405,
    "Hlt1SingleElectronFromPi0Decision"     :  406,
    "Hlt1SingleElectronNoIPDecision"        :  407,
    "Hlt1SingleElectronWithIPDecision"      :  408,

    # CO
    "Hlt1CosmicOTDecision"                  :  480,
    # MiniBias
    "Hlt1MBMiniBiasNoBeamDecision"          :  500, 
    "Hlt1MBMiniBiasBeam1Decision"           :  501, 
    "Hlt1MBMiniBiasBeam2Decision"           :  502,
    "Hlt1MBMiniBiasBeamCrossingDecision"    :  503, 
    # NoBias
    "Hlt1MBNoBiasNoBeamDecision"            :  504, 
    "Hlt1MBNoBiasBeam1Decision"             :  505, 
    "Hlt1MBNoBiasBeam2Decision"             :  506,
    "Hlt1MBNoBiasBeamCrossingDecision"      :  507, 
    #  MicroBias
    'Hlt1MBMicroBiasRZVeloDecision'         :  508,
    'Hlt1MBMicroBiasTStationDecision'       :  509,
    'Hlt1MBMiniBiasDecision'                :  510,
    'Hlt1MBNoBiasDecision'                  :  511,


    # Autogenerated Decisions start at 1K
    'Hlt1L0CALODecision'                    : 1000,
    'Hlt1L0MUONDecision'                    : 1001,
    #
    # intermediate selections: 10000 -- 20000
    # 'All' L0 candidates ; 
    'Hlt1L0AllMuonCandidates'               :10000,
    'Hlt1L0AllElectronCandidates'           :10010,
    'Hlt1L0AllPhotonCandidates'             :10011,
    'Hlt1L0AllHadronCandidates'             :10012,
    'Hlt1L0AllLocalPi0Candidates'           :10013,
    'Hlt1L0AllGlobalPi0Candidates'          :10014,
    #
    'RZVelo'                                :10100,
    'Forward1'                              :10102,
    'PV3D'                                  :10103,
    'VeloOpen'                              :10104,
    'PV3DOpen'                              :10105,
    'RZVeloBW'                              :10106,
    'VeloCandidatesDiMuon'                  :10107,
    'VeloCandidatesTrackAllL0'              :10108,
    'VeloCandidatesTrackMuon'               :10109,
    'VeloCandidatesTrackPhoton'             :10110,
    'VeloCandidatesDiMuonHighMass'          :10111,
    'VeloCandidatesDiMuonLowMass'           :10112,
    'VeloCandidatesSingleMuonHighPT'        :10113,
    'VeloCandidatesSingleMuonNoIP'          :10114,
    'VeloCandidatesDiProtonLowMult'         :10115,

    #
    # 11K - 20K : added automatically by configuration...
    # 50K+ : used by Hlt2
}

#/ generated by parsing the logfile, of a job _without_ explicitly specified 
#/ InfoID, with:
#/ awk 'BEGIN{n="foo"}/autogenerated: InfoID/{if (substr($6,0,length(n))!=n) {n=$6; i =(1+int(i/100))*100} ; print sprintf( "    %-40s: %4d,","\""$6"\"",++i)}' logfile

##
## InfoID below 1000 is reserved for the tracking reconstruction group (JAH 020409)
## 
HltANNSvc().InfoID = {
    "FitIP"                                 : 1001,
    "FitTrack"                              : 1003,
    "FitIP_PV3D"                            : 1004,
    "AntiEleConf"                           : 1101,
    "Calo3DChi2"                            : 1301,
    "Chi2Mu"                                : 1311,
    "Chi2OverN"                             : 1312,
    "DOCA"                                  : 1401,
    "DeltaP"                                : 1501,
    "DoShareM3"                             : 1601,
    "Ecal3DChi2"                            : 1801,
    "Ecal3DChi2_Hlt1L0ElectronDecision"     : 1802,
    "FitChi2OverNdf"                        : 1901,
    "FitMuChi2"                             : 1902,
    "HltTrackFit"                           : 2001,
    "HltRadCorTool"                         : 2091,
    "HltRadCorTool/HltRadCorTool"           : 2092,
    "IP"                                    : 2101,
    "IP_PV3D"                               : 2103,
    "IsMuon"                                : 2201,
    "IsPhoton"                              : 2301,
    "L0ET"                                  : 2501,
    "MatchIDsFraction"                      : 2601,
    "PT"                                    : 2701,
    "PT0"                                   : 2702,
    "PatForwardTool"                        : 2801,
    "PatForwardTool/PatForwardTool"         : 2802,
    "SumPT"                                 : 3001,
    "Tf::PatVeloSpaceTool"                  : 3101,
    "VeloCalo3DChi2"                        : 3201,
    "VertexAngle"                           : 3301,
    "VertexDimuonMass"                      : 3401,
    "VertexDiElectronMass"                  : 3402,
    "VertexDz"                              : 3501,
    "VertexDz_PV3D"                         : 3503,
    "VertexMatchIDsFraction"                : 3601,
    "VertexMinIP"                           : 3701,
    "VertexMinIP_PV3D"                      : 3703,
    "VertexMinPT"                           : 3801,
    "VertexNumberOfASideTracks"             : 3901,
    "VertexNumberOfCSideTracks"             : 4001,
    "VertexTrack1PT"                        : 4150,
    "VertexTrack2PT"                        : 4151,
    "chi2_PatMatch"                         : 4201,
    "FitVertexAngle"                        : 5101,
    "FitVertexDOCA"                         : 5201,
    "FitVertexDimuonMass"                   : 5301,
    "FitVertexMaxChi2OverNdf"               : 5501,
    "FitVertexMaxMuChi2"                    : 5502,
    "FitVertexMinIP"                        : 5601,
    "FitVertexMinIP_PV3D"                   : 5603,
    "FitVertexMinIP_PV3DOpen"               : 5604,
    "FitVertexMinPT"                        : 5701,
    "FitVertexTrack1Chi2OverNdf"            : 5811,
    "FitVertexTrack1IP"                     : 5821,
    "FitVertexTrack2IP"                     : 5822,
    "FitVertexTrack1IP_PV3D"                : 5825,
    "FitVertexTrack2IP_PV3D"                : 5826,
    "FitVertexTrack1PT"                     : 5831,
    "FitVertexTrack2PT"                     : 5832,
    "FitVertexTrack2Chi2OverNdf"            : 5841,
    "HltGuidedForward"                      : 5901,
    "HltGuidedForward/HltGuidedForward"     : 5902,
    "Velo"                                       : 6298,
    "PatForwardTool/LooseForward"                : 6299,
    "PatForwardTool/TightForward"                : 6300,
    "MatchVeloMuon"                              : 6400,
    "IsMuonTool"                                 : 6401,
    'Hlt2Topo2BodyBBDTResponse'                  : 6502,
    'Hlt2Topo3BodyBBDTResponse'                  : 6303,
    'Hlt2Topo4BodyBBDTResponse'                  : 6304
}
