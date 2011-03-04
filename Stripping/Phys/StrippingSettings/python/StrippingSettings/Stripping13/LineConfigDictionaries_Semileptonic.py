"""
Line config dictionaries for Semileptonic WG.

Stripping13 definitions.
Dictionaries have the name of the line builder instance.
"""

Bd2DstarMuNu = {
    'BUILDERTYPE' : 'Bd2DstarMuNuAllLinesConf',
    'CONFIG' :
    { 'Loose' : { 'Prescale'    : 0.5 ,
                  'Postscale'   : 1.0 ,
                  'MuPT'        : 700, # 800  # MeV
                  'MuTrPID'     :-20,  # -5   
                  'MuIPChi2'    :  0,  # 4    
                  'KPT'         : 350, # 350  # MeV
                  'KIPChi2'     :  0,  # 4    
                  'KTrPID'      : -5,  #-5    
                  #pion parameters
                  'PiPT'        : 350, # MeV
                  'PiIPChi2'    :  0, # 4    
                  #D0-resonance parameters
                  'D0MassW'     : 60, # 40    # MeV
                  'D0_BPVVDCHI2': 50,
                  #Dstar-resonance parameters
                  'Dstar_PT'     : 1250, # 1250 # MeV
                  'Dstar_VCHI2'  : 25, # 20 # 
                  'DstarMassW'   : 80, # 50 # MeV
                  #Bd-resonance parameters
                  'B0d_VCHI2'    : 10, # 10 # 
                  'B0dD0_DeltaZVtx' : -10, # -2.5  # mm
                  'B0dPV_DeltaZVtx' : -100, # 0.5  # mm
                  },
      'LooseNoPresc' : { 'Prescale'    : 1. ,
                         'Postscale'   : 1.0 ,
                         #muon paramters # loose # Tight
                         'MuPT'        : 700, # 800  # MeV
                         'MuTrPID'     :-20,  # -5   
                         'MuIPChi2'    :  0,  # 4    
                         #kaon parameters
                         'KPT'         : 350, # 350  # MeV
                         'KIPChi2'     :  0,  # 4    
                         'KTrPID'      : -5,  #-5    
                         #pion parameters
                         'PiPT'        : 350, # MeV
                         'PiIPChi2'    :  0, # 4    
                         #D0-resonance parameters
                         'D0MassW'     : 60, # 40    # MeV
                         'D0_BPVVDCHI2': 50,
                         #Dstar-resonance parameters
                         'Dstar_PT'     : 1250, # 1250 # MeV
                         'Dstar_VCHI2'  : 25, # 20 # 
                         'DstarMassW'   : 80, # 50 # MeV
                         #Bd-resonance parameters
                         'B0d_VCHI2'    : 10, # 10 # 
                         'B0dD0_DeltaZVtx' : -10, # -2.5  # mm
                                   'B0dPV_DeltaZVtx' : -100, # 0.5  # mm
                         },
      'Tight'   : {'Prescale'    : 1.0 ,
                   'Postscale'   : 1.0 ,
                   #muon paramters # 
                   'MuPT'        : 800, # MeV
                   'MuTrPID'     :-5,
                   'MuIPChi2'    : 4,
                   #kaon parameters
                   'KPT'         : 350, # MeV
                   'KIPChi2'     : 4,
                   'KTrPID'      :-5, 
                   #pion parameters
                             'PiPT'        : 350, # MeV
                   'PiIPChi2'    : 4,
                   #D0-resonance parameters
                   'D0MassW'     : 40, # MeV
                   'D0_BPVVDCHI2': 50,
                   #Dstar-resonance parameters
                   'Dstar_PT'     : 1250, #  MeV
                   'Dstar_VCHI2'  : 20, 
                   'DstarMassW'   : 50, 
                   #Bd-resonance parameters
                   'B0d_VCHI2'    : 10, 
                   'B0dD0_DeltaZVtx' : -2.5, # mm
                   'B0dPV_DeltaZVtx' : 0.5 #  mm
                             }
      },
    'STREAMS' : ['Semileptonic'] ,
    'WGs'    : ['Semileptonic'] 
    
    }

B0q2DplusMuX = {
    'BUILDERTYPE' : 'B0q2DplusMuXAllLinesConf',
    'CONFIG' : { 'Presel' : { 'Prescale'    : 0.02 ,
                              'Postscale'   : 1.0 ,
                              'MuPT'        : 500, #MeV
                              'MuPidPi'     : -5.,
                              'MuPidK'      : -10.,
                              'MuTrChi2'    : 9,
                              'KPT'         : 300, #MeV
                              'KIP'         : 0.03, #mm
                              'KPidPi'      : -5,
                              'KPidMu'      : -40,
                              'KPidP'       : -40,
                              'KTrChi2'     : 16,
                              'KIPChi2'     : 4,
                              'PiPidK'      : -40,
                              'PiIP'        : 0.01, #mm
                              'PiPidMu'     : -5,
                              'PiTrChi2'    : 16,
                              'PiIPChi2'    : 4,
                              'DPT'         : 1000, #MeV
                              'D_APT'       : 1000, #MeV
                              'D_VPCHI2'    : 0.000250,
                              'D_BPVVDCHI2' : 49,
                              'B_VPCHI2'    : 0.000250,
                              'B_BPVDIRA'   : 0.997
                              },
    
                 'Tuned'   : { 'Prescale'    : 1.0 ,
                               'Postscale'   : 1.0 ,
                               'MuPT'        : 500, #MeV
                               'MuPidPi'     : -1.,
                               'MuPidK'      : -5,
                               'MuTrChi2'    : 5,
                               'KPT'         : 400, #MeV
                               'KIP'         : 0.04, #mm
                               'KPidPi'      : 0,
                               'KPidMu'      : 5,
                               'KPidP'       : -10,
                               'KTrChi2'     : 5,
                               'KIPChi2'     : 4,
                               'PiPidK'      : -10,
                               'PiIP'        : 0.04, #mm
                               'PiPidMu'     : -5,
                               'PiTrChi2'    : 10,
                               'PiIPChi2'    : 7,
                               'DPT'         : 1500, #MeV
                               'D_APT'       : 1200, #MeV
                               'D_VPCHI2'    : 0.010,
                               'D_BPVVDCHI2' : 144,
                               'B_VPCHI2'    : 0.010,
                               'B_BPVDIRA'   : 0.9980
                               },
    
                 'MC09'   : { 'Prescale'    : 1.0 ,
                              'Postscale'   : 1.0 ,
                              'MuPT'        : 600, #MeV
                              'MuPidPi'     : 0.,
                              'MuPidK'      : -10,
                              'MuTrChi2'    : 2,
                              'KPT'         : 400, #MeV
                              'KIP'         : 0.03, #mm
                              'KPidPi'      : 7,
                              'KPidMu'      : 10,
                              'KPidP'       : -10,
                              'KTrChi2'     : 3,
                              'KIPChi2'     : 4,
                              'PiPidK'      : -5,
                              'PiIP'        : 0.01, #mm
                              'PiPidMu'     : -2,
                              'PiTrChi2'    : 9,
                              'PiIPChi2'    : 7,
                              'DPT'         : 1500, #MeV
                              'D_APT'       : 1200, #MeV
                              'D_VPCHI2'    : 0.0150,
                              'D_BPVVDCHI2' : 144,
                              'B_VPCHI2'    : 0.0150,
                              'B_BPVDIRA'   : 0.9980
                              }
                 
                 },
    'STREAMS' : [ 'Semileptonic' ],
    'WGs'    : ['Semileptonic'] 
    }

Bd2DstarTauNu = {
    'BUILDERTYPE' : 'Bd2DstarTauNuAllLinesConf',
    'CONFIG' : { 'Loose' : { 'Prescale'    : 0.1 ,
                             'Postscale'   : 1.0 ,
                             #tau: single pi cuts
                             'SinglePiPT'  : 150 ,
                             'SinglePiIPChi2' : 0,
                             #tau: di pi cuts
                             'DiPiPT'      : 600,
                             'DiPiLowM'    : 600,
                             'DiPiHighM'   : 2000,
                             'DiPiVxChi2'  : 12,
                             'DiPiDira'    : 0.99,
                             'DiPiIPChi2'  : 0,
                             #tau: di pi combi cut
                             'DiPiMaxIPChi2' : 20,
                             #tau: tri pi cuts
                             'TriPiPT'     : 800,
                             'TriPiLowM'   : 1000,
                             'TriPiHighM'  : 2400,
                             'TriPiVxChi2' : 12,
                             'TriPiDira'   : 0.99,
                             'TriPiIPChi2' : 0,
                             #Bd cuts
                             'BdVxChi2'    : 0,
                             'BdDira'      : 0.99,
                             'VxOrder'     : 0,
                             #Bd combi cuts
                             'BdLowDM'     : -2279,
                             'BdHighDM'    : 500,
                             #kaon parameters
                             'KPT'         : 350, # 350  # MeV
                             'KTrChi2'     :  8,  # 10   
                             'KIPChi2'     :  0,  # 4 already in StdLooseD0
                             'KTrPID'      : -5,  #-5
                             'KIP'         :  0,
                             #pion parameters
                             'PiPT'        : 350, # MeV
                             'PiTrChi2'    :  8, # 10   
                             'PiIPChi2'    :  0, # 4 already in StdLooseD0
                             'PiIP'        :  0,
                             #D0-resonance parameters
                             'D0MassW'     : 60, # 40    # MeV
                             'D0_BPVVDCHI2': 50,
                             #Dstar-resonance parameters
                             'slowPiTrChi2' : 8, # 10 
                             'Dstar_PT'     : 1250, # 1250 # MeV
                             'Dstar_VCHI2'  : 25, # 20 # 
                             'DstarMassW'   : 80, # 50 # MeV
                             #Bd-resonance parameters
                             'B0d_VCHI2'    : 10, # 10 # 
                             'B0dD0_DeltaZVtx' : -10, # -2.5  # mm
                             'B0dPV_DeltaZVtx' : -100, # 0.5  # mm
                             },
                 'Tight'   : {'Prescale'    : 1.0 ,
                              'Postscale'   : 1.0 ,
                              #tau: single pi cuts
                              'SinglePiPT'  : 150 ,
                              'SinglePiIPChi2' : 4,
                              #tau: di pi cuts
                              'DiPiPT'      : 600,
                              'DiPiLowM'    : 750,
                              'DiPiHighM'   : 1500,
                              'DiPiVxChi2'  : 9,
                              'DiPiDira'    : 0.99,
                              'DiPiIPChi2'  : 5,
                              #tau: di pi combi cut
                              'DiPiMaxIPChi2' : 20,
                              #tau: tri pi cuts
                              'TriPiPT'     : 800,
                              'TriPiLowM'   : 1200,
                              'TriPiHighM'  : 1780,
                              'TriPiVxChi2' : 9,
                              'TriPiDira'   : 0.99,
                              'TriPiIPChi2' : 5,
                              #Bd cuts
                              'BdVxChi2'    : 0,
                              'BdDira'      : 0.99,
                              'VxOrder'     : 0,
                              #Bd combi cuts
                              'BdLowDM'     : -2279,
                              'BdHighDM'    : 0,
                              #kaon parameters
                              'KPT'         : 350, # MeV
                              'KTrChi2'     : 8,
                              'KIPChi2'     : 4,
                              'KTrPID'      :-5,
                              'KIP'         : 0.04,
                              #pion parameters
                              'PiPT'        : 350, # MeV
                              'PiTrChi2'    : 8,
                              'PiIPChi2'    : 4,
                              'PiIP'        : 0.04,
                              #D0-resonance parameters
                              'D0MassW'     : 40, # MeV
                              'D0_BPVVDCHI2': 50,
                              #Dstar-resonance parameters
                              'slowPiTrChi2' : 8, 
                              'Dstar_PT'     : 1250, #  MeV
                              'Dstar_VCHI2'  : 20, 
                              'DstarMassW'   : 50, 
                              #Bd-resonance parameters
                              'B0d_VCHI2'    : 10, 
                              'B0dD0_DeltaZVtx' : -2.5, # mm
                              'B0dPV_DeltaZVtx' : 0.5 #  mm
                              }
                 },
    'STREAM' : ['Bhadron'],
    'WGs'    : ['Semileptonic'] 
    }

B2XuMuNu = {
    'BUILDERTYPE' : 'B2XuMuNuBuilder',
    'CONFIG' :  {    "MuonTRCHI2"          : 6.   ,#adimensional
                     "MuonP"               : 3000. ,#MeV
                     "MuonPT"              : 1000. ,#MeV
                     "MuonPIDK"            : 0.    ,#adimensional
                     "MuonPIDmu"           : 3.    ,#adimensional
                     "MuonPIDp"            : 0.    ,#adimensional
                     #        "MuonMINIP"           : 0.05  ,#mm
                     "MuonMINIPCHI2"       : 12     ,#adminensional
                     #Xu
                     #Pi Channel
                     "PionTRCHI2"          : 6.   ,#adimensional
                     "PionP"               : 3000. ,#MeV
                     "PionPT"              : 800.  ,#MeV
                     "PionPIDK"            : 0.    ,#adimensional
                     "PionPIDmu"           : 0.    ,#adimensional
                     "PionPIDp"            : 0.    ,#adimensional
                     #        "PionMINIP"           : 0.05  ,#mm
                     "PionMINIPCHI2"       : 16     ,#adminensional
                     #K channel
                     "KaonTRCHI2"          : 6.   ,#adimensional
                     "KaonP"               : 3000. ,#MeV
                     "KaonPT"              : 800.  ,#MeV
                     "KaonPIDK"            : 5.    ,#adimensional // JUST CHANGED THIS
                     "KaonPIDmu"           : 0.    ,#adimensional
                     "KaonPIDp"            : 0.    ,#adimensional
                     #        "KaonMINIP"          : 0.05  ,#mm
                     "KaonMINIPCHI2"       : 16     ,#adminensional
                     #Rho channel
                     "RhoMassWindow"       : 190.  ,#MeV
                     "RhoVCHI2DOF"         : 6    ,#adimensional
                     "RhoPT"               : 500.  ,#MeV
                     "RhoLeadingPionPT"    : 800.  ,#MeV
                     #        "RhoMINIP"           : 0.05  ,#mm 
                     "RhoMINIPCHI2"        : 4     ,#adimensional
                     "RhoChPionPT"         : 300.  ,#MeV
                     "RhoChPionTRCHI2"     : 10.  ,#MeV
                     "RhoChPionPIDK"       : -10.   ,#adminensional
                     #        "RhoChPionMINIP"     : 0.05  ,#mm
                     "RhoChPionMINIPCHI2"  : 9     ,#adimensional
                     #K* channel
                     "KSMassWindow"        : 300.  ,#MeV
                     "KSVCHI2DOF"          : 6    ,#adimensional
                     "KSPT"                : 400.  ,#MeV
                     #        "KSMINIP"             : 0.05  ,#mm 
                     "KSMINIPCHI2"         : 4     ,#adimensional
                     "KstarMassWindow"     : 100.  ,#MeV
                     "KstarVCHI2DOF"       : 6    ,#adimensional
                     "KstarPT"             : 400.  ,#MeV
                     #        "KstarMINIP"          : 0.05  ,#mm 
                     "KstarMINIPCHI2"      : 4     ,#adimensional
                     "KstarChPionPT"       : 100.  ,#MeV
                     "KstarChPionTRCHI2"   : 10.  ,#MeV
                     "KstarChPionPIDK"     : -10.   ,#adminensional
                     #        "KstarChPionMINIP"    : 0.05  ,#mm
                     "KstarChPionMINIPCHI2": 9     ,#adimensional
                     "KstarLeadingPionPT"  : 800.  ,#MeV
                     #Xu
                     "PiMuMassLowLoose"    : 1230. ,#MeV
                     "KMuMassLowTight"     : 1370. ,#MeV
                     "RhoMuMassLowTight"   : 1360. ,#MeV
                     "KstarMuMassLowTight" : 1400. ,#MeV
                     #B
                     "BVCHI2DOF"           : 4     ,#adminensional
                     "BDIRA"               : 0.99,  #adminensional
                     "BFDCHI2LOW"             : 15,   #adimensional
                     "BFDCHI2HIGH"             : 225,   #adimensional
                     "XMuMassUpper"        : 5500 # MeV
                     },
    'STREAMS' : ['Semileptonic'],
    'WGs'    : ['Semileptonic'] 
    }

B2DMuNuX = {
    'BUILDERTYPE' : 'B2DMuNuXAllLinesConf',
    'CONFIG' :  { "MINIPCHI2"      : 9.0    # adimensiional
                  ,"TRCHI2"        : 5.0    # adimensiional
                  ,"KaonPIDK"      : 4.0    # adimensiional
                  ,"PionPIDK"      : 10.0   # adimensiional
                  ,"MuonIPCHI2"    : 4.00   # adimensiional
                  ,"MuonPT"        : 800.0  # MeV
                  ,"KPiPT"         : 300.0  # MeV
                  ,"DsDIRA"        : 0.99   # adimensiional
                  ,"DsFDCHI2"      : 100.0  # adimensiional
                  ,"DsMassWin"     : 80.0   # MeV
                  ,"DsAMassWin"    : 100.0  # MeV
                  ,"DsIP"          : 7.4    #mm
                  ,"DsVCHI2DOF"    : 6.0    # adimensiional
                  ,"PIDmu"         : -0.0   # adimensiional
                  ,"BDIRA"         : 0.999  #adimensiional
                  ,"BVCHI2DOF"     : 6.0    # adimensiional
                  ,"DZ"            : 0.0    #mm
                  ,"DDocaChi2Max"  : 20     #adimensiional
                  },
    'STREAM' : ['Semileptonic'] ,
    'WGs'    : ['Semileptonic'] 
    }


DForBSemi = {
    'BUILDERTYPE'	: 'DforBSemiLinesConf',
    'CONFIG'	: {'DDocaChi2Max': 20,
                   'DpPreScale': 0.040000000000000001,
                   'DsAMassWin': 100.0,
                   'DsDIRA': 0.98999999999999999,
                   'DsFDCHI2': 100.0,
                   'DsIP': 7.4000000000000004,
                   'DsMassWin': 80.0,
                   'DsPreScale': 0.040000000000000001,
                   'DsVCHI2DOF': 6.0,
                   'KaonPIDK': 4.0,
                   'LcPreScale': 0.080000000000000002,
                   'MINIPCHI2': 9.0,
                   'PT': 300.0,
                   'PionPIDK': 10.0,
                   'TRCHI2': 5.0},
    'STREAM' : ['CharmControl'],
    'WGs'    : ['Semileptonic'] 
}



D0ForBXX = {
    'BUILDERTYPE'	: 'D0forBXXLinesConf',
    'CONFIG'	: {'D0AMassWin': 100.0,
                   'D0DIRA': 0.99990000000000001,
                   'D0DIRAK3Pi': 0.99990000000000001,
                   'D0IP': 0.20000000000000001,
                   'D0K3PiIP': 0.20000000000000001,
                   'D0MassWin': 80.0,
                   'D0VCHI2DOF': 6.0,
                   'DDocaChi2Max': 20,
                   'FDCHI2': 100.0,
                   'KaonPIDK': 4.0,
                   'MINIPCHI2': 9.0,
                   'PT': 300.0,
                   'PionPIDK': 10.0,
                   'PreScale': 0.029999999999999999,
                   'TRCHI2': 5.0},
    'STREAM' : ['CharmControl'],
    'WGs'    : ['Semileptonic'] 
    }
