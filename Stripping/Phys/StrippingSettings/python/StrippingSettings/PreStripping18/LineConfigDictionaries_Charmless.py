##
## B-> hh, pp 
##
Hb2Charged2Body = {
    'WGs'         : ['Charmless'],
    'BUILDERTYPE' : 'Hb2Charged2BodyLines',
    'CONFIG'      : {
           'PrescaleB2Charged2Body'   : 1,      'PrescaleB2PPbar' : 1, 'PrescaleBs2KK_NoIPCutOnB' : 1,  
           'MinPTB2Charged2Body'      : 1100,    'MinPTB2PPbar' : 900,
           'MinIPB2Charged2Body'      : 0.12,
           'MinIPChi2B2Charged2Body'  : 100,     'MinIPChi2B2PPbar' : 9,
           'TrChi2'                   : 5,
           'PIDppi'                   : 0,
           'PIDpk'                    : -2, 
           'MaxPTB2Charged2Body'      : 2500,   'MaxPTB2PPbar' : 2100,
           'MaxIPB2Charged2Body'      : 0.25,
           'MaxIPChi2B2Charged2Body'  : 200,    'MaxIPChi2B2PPbar' : 25, 
           'CombMassLow'              : 4800,
           'CombMassHigh'             : 5800,   'CombMassWindow' : 200,
           'DOCA'                     : 0.08,   'VertexChi2B2PPbar' : 16, 
           'BPT'                      : 1200,   'BPTB2PPbar' : 1000,
           'BIP'                      : 0.1,
           'BIPChi2B2Charged2Body'    : 12,     'BIPChi2B2PPbar' : 36, 
           'BDIRA'                    : 0.9995,
           'BTAU'                     : 0.0006,
           'MassLow'                  : 4800,
           'MassHigh'                 : 5800,
           'PIDKBs2KK_NoIPCutOnB'     : 0.
           },
    'STREAMS'     : [ 'Bhadron' ]
    }

##
## B->hh, lifetime  unbiased
##
B2hhLTUnbiased = {
    'WGs'         : ['Charmless'],
    'BUILDERTYPE' : 'StrippingB2hhLTUnbiasedConf' ,
    'CONFIG'      : {'TrackChi2'               :    5.0
                     , 'DaughterPtMin'         :    1.5 # GeV
                     , 'DaughterPtMax'         :    2.0 # GeV
                     , 'DaughterPMin'          :   10.0 # GeV
                     , 'DaughterPIDKMax'       :    0.1
                     , 'BMassMin'              :    5.0 # GeV
                     , 'BMassMax'              :    6.0 # GeV
                     , 'DOCA'                  :    0.07
                     , 'VertexChi2'            :   25.0
                     , 'PrescaleLoose'         :    0.005
                     , 'PostscaleLoose'        :    1.0
                     , 'PrescaleNB'            :    0.15
                     , 'PostscaleNB'           :    1.0
                     , 'NetCut'                :    0.9
                     , 'BLifetime'             : -999.0
                    },
    'STREAMS'    : [ 'Bhadron' ]
    }

B2hhLTUnbiasedDetached= {
    'WGs'         : ['Charmless'], 
    'BUILDERTYPE' : 'StrippingB2hhLTUnbiasedConf' ,
    'CONFIG'      : {'TrackChi2'               :    5.0
                     , 'DaughterPtMin'         :    1.5 # GeV
                     , 'DaughterPtMax'         :    2.0 # GeV
                     , 'DaughterPMin'          :   10.0 # GeV
                     , 'DaughterPIDKMax'       :    0.1
                     , 'BMassMin'              :    5.0 # GeV
                     , 'BMassMax'              :    6.0 # GeV
                     , 'DOCA'                  :    0.07
                     , 'VertexChi2'            :   25.0
                     , 'PrescaleLoose'         :    0.005
                     , 'PostscaleLoose'        :    1.0
                     , 'PrescaleNB'            :    1.0
                     , 'PostscaleNB'           :    1.0
                     , 'NetCut'                :    0.9
                     , 'BLifetime'             :    0.3
                   },
    'STREAMS'    : [ 'Bhadron' ]
    }

##
## B->hhh
##
B2hhh = {
    'WGs'         : ['Charmless'],
    'BUILDERTYPE' : 'Bu2hhhBuilder',
    'CONFIG'      : { 'MaxTrSIZE'            :  200., # long tracks!
                      '_h_PT'                : 100. ,
                      '_h_P'                 : 1500. ,
                      '_h_IPCHI2'            : 1. ,
                      '_h_TRCHI2DOF'         : 3.0 ,
                      '_3h_DOCA'             : .2 ,
                      '_3h_PTmax'            : 1500 ,
                      '_3h_DIRA'             : .99998 ,# !
                      '_3h_FDCHI2'           : 500. ,
                      '_3h_PVDOCAmin'        : 3.0 ,
                      '_3h_CHI2'             : 12.0 ,
                      '_3h_IPCHI2'           : 10. , # 8.!
                      '_3h_PT'               : 1000. ,
                      '_3h_PTsum'            : 4500.  ,# 4500?
                      '_3h_Psum'             : 20000.  ,
                      '_3h_PVIPCHI2sum'      : 500. ,
                      '_3h_TRKCHIDOFmin'     : 3.,
                      '_3h_CORRMmax'         : 7000. ,
                      '_3h_CORRMmin'         : 4000. ,
                      '_3hKKK_Mmax'          : 6300. , # 6200?
                      '_3hKKK_Mmin'          : 5050. , # 5050!
                      '_3hpph_deltaMmax'     :400, # 300!
                      '_3hpph_deltaMmin'     :200,
                      'KKK_inclLinePrescale' : 1. ,
                      'KKK_inclLinePostscale': 1 ,
                      'pph_inclLinePrescale' : 1 ,
                      'pph_inclLinePostscale': 1
                    },
    'STREAMS'     : [ 'Bhadron' ]
    }

##
## B->hpi pi0
##
B2HHPi0 = {
      'WGs'         : [ 'Charmless' ] ,
      'BUILDERTYPE' : 'StrippingB2HHPi0Conf',
      'CONFIG'      : { 'PiMinPT'              : 500       # MeV
                       ,'PiMinP'               : 5000      # MeV
                       ,'PiMinTrackProb'       : 0.000001  # unitless
                       ,'PiMinIPChi2'          : 25        # unitless
                       ,'Pi0MinPT_M'           : 2500      # MeV
                       ,'Pi0MinPT_R'           : 1500      # MeV
                       ,'ResPi0MinMM'          : 100       # MeV  # no effect if bellow 105
                       ,'ResPi0MaxMM'          : 170       # MeV  # no effect if above  165
                       ,'ResPi0MinGamCL'       : 0.2       # unitless
                       ,'BMinM'                : 4200      # MeV
                       ,'BMaxM'                : 6400      # MeV
                       ,'BMinPT_M'             : 3000      # MeV
                       ,'BMinPT_R'             : 2500      # MeV
                       ,'BMinVtxProb'          : 0.001     # unitless
                       ,'BMaxIPChi2'           : 9         # unitless
                       ,'BMinDIRA'             : 0.99995   # unitless
                       ,'BMinVVDChi2'          : 64        # unitless
                       ,'MergedLinePrescale'   : 1.        # unitless
                       ,'MergedLinePostscale'  : 1.        # unitless
                       ,'ResolvedLinePrescale' : 1.        # unitless
                       ,'ResolvedLinePostscale': 1.        # unitless
                      },
      'STREAMS'     : [ 'Bhadron' ]
    }

##
## B->Kshh
##
B2KShh = {
    'WGs'         : ['Charmless'],
    'BUILDERTYPE' : 'B2KShhConf',
    'CONFIG' : {  'Trk_Chi2'                : 4.0,
                  'KS_DD_MassWindow'        : 30.0,
                  'KS_DD_VtxChi2'           : 12.0,
                  'KS_DD_FDChi2'            : 50.0,
                  'KS_DD_Pmin'              : 6000.0,
                  'KS_LL_MassWindow'        : 20.0,
                  'KS_LL_VtxChi2'           : 12.0,
                  'KS_LL_FDChi2'            : 80.0,
                  'B_Mlow'                  : 500.0,
                  'B_Mhigh'                 : 587.0,
                  'B_APTmin'                : 1000.0,
                  'B_PTmin'                 : 1500.0,
                  'BDaug_MedPT_PT'          : 800.0,
                  'BDaug_MaxPT_IP'          : 0.05,
                  'BDaug_DD_maxDocaChi2'    : 5.0,
                  'BDaug_LL_maxDocaChi2'    : 5.0,
                  'BDaug_DD_PTsum'          : 3000.0,
                  'BDaug_LL_PTsum'          : 3000.0,
                  'B_VtxChi2'               : 12.0,
                  'B_Dira'                  : 0.9999,
                  'B_DD_IPCHI2wrtPV'        : 8.0,
                  'B_LL_IPCHI2wrtPV'        : 8.0,
                  'B_FDwrtPV'               : 1.0,
                  'B_DD_FDChi2'             : 50.0,
                  'B_LL_FDChi2'             : 50.0,
                  'GEC_MaxTracks'           : 250,
                  'Prescale'                : 1.0,
                  'Postscale'               : 1.0
                 },
    'STREAMS' : [ 'Bhadron' ]
    }

# B2KPhiPhi from BetaS (M.Chrzaszcz)
B2KPhiPhi = {
    'WGs'               : ['Charmless'],
    'BUILDERTYPE'       : 'B2KPhiPhiConf',
    'CONFIG' : {
    'Prescale'                : 1.0 ,
    'Postscale'               : 1.0 ,

    #K parameters
    'K_PT'                    : 300.,
    'K_IPCHI2'                : 6.,
    'K_TRCHI2'                : 5.0,

    #phi parameters         
    'phi_VDZ'                 : 0.,
    'phi_PT'                  : 300.,
    'phi_IPCHI2'              : 6.,
    'phi_VCHI2_VDOF'          : 25.,
    'phi_MM_max'              : 1070.,
    'phi_MM_min'              : 980.,

    #B parameters
    'B_ADAMASS'              : 500.,
    'B_DIRA'                 : 0.999,
    'B_IPCHI2'               : 12.25,
    'B_VCHI2_VDOF'           : 25.,
    'B_LTCHI2'               : 64.,
    'B_VDZ'                  : 0.
    },
    'STREAMS' : [ 'Bhadron' ]
    }


# Quasi two-body (Fred Blanc)
BetaSQ2B = {
    'WGs'               : ['Charmless'],
    'BUILDERTYPE'       : 'B2Quasi2BodyConf',
    'CONFIG' : { 'Q2BPrescale'     : 1.,
                 'Q2BResMinPT'     : 600.,
                 'Q2BResMinP'      : 10.,
                 'Q2BResMaxMass'   : 1000.,
                 'Q2BResVtxChiDOF' : 9.,
                 'Q2BBMinM4pi'     : 3800.,
                 'Q2BBMinM3pi'     : 4400.,
                 'Q2BBMaxM'        : 5700.,
                 'Q2BBMinCorrM4pi' : 0.,
                 'Q2BBMinCorrM3pi' : 0.,
                 'Q2BBMaxCorrM'    : 6000.,
                 'Q2BBVtxChi2DOF'  : 9.
                 },
    'STREAMS' : [ 'Bhadron' ]
    }

# Bs2PhiKst from BetaS (Cibran Santamarina)
BetaSBs2PhiKst = {
        'WGs'           : ['Charmless'],
        'BUILDERTYPE'   : 'StrippingBs2PhiKstConf',
        'CONFIG'        : {
               "KaonPT"                : 500.0 # MeV
         ,     "KaonIPCHI2"            : 9.    # adimensional
         ,     "KaonPIDK"              : 0.    # adimensional
         ,     "PionPT"                : 500.0 # MeV
         ,     "PionIPCHI2"            : 9.    # adimensional
         ,     "PionPIDK"              : 10.   # adimensional
         ,     "PhiVCHI2"              : 9.    # adimensional
         ,     "PhiPT"                 : 900.0 # MeV
         ,     "PhiMassWin"            : 25.0  # MeV
         ,     "KstarVCHI2"            : 9.0   # adimensional
         ,     "KstarPT"               : 900.0 # MeV
         ,     "KstarMassWin"          : 150.0 # MeV
         ,     "BMassWin"              : 500.0 # MeV
         ,     "BVCHI2"                : 15.0  # adimensional
         ,     "BDOCA"                 : 0.3   # mm
        },
        'STREAMS' : [ 'Bhadron' ]
        }

# Bs2KstKst from BetaS (Paula Alvarez)
BetaSBs2KstKst = {
    'WGs'               : ['Charmless'],
    'BUILDERTYPE'       : 'StrippingBs2KstKstConf',
    'CONFIG'    : {
                "KaonPT"        : 500.0 # MeV
         ,      "KaonIPCHI2"        : 9.     # adimensional
         ,      "PionPT"                : 500.0 # MeV
         ,      "PionIPCHI2"            : 9.    # adimensional
         ,      "PionPIDK"              :10. #adimensional
         ,      "KstarVCHI2"        : 9.0   # adimensional
         ,      "KstarPT"        : 900.0 # MeV
         ,      "KaonPIDK"              : -5.0  # adimensional
         ,      "KstarMassWin"        : 150.0 # MeV
         ,      "BMassWin"        : 500.0 # MeV
         ,      "BVCHI2"        : 15.0    # adimensional
         ,      "BDOCA"                 : 0.3   # mm
         ,      "BIPCHI2"               : 25    # adimensional
         },
    'STREAMS' : [ 'Bhadron' ]
    }

# Bs2K0stK0st from BetaS (Paula Alvarez)
BetaSBs2K0stK0st = {
    'WGs'               : ['Charmless'],
    'BUILDERTYPE'       : 'StrippingBs2Kst_0Kst_0Conf',
    'CONFIG'    : {
             "KaonPT"                 : 500.0         #MeV
          ,  "KaonIPCHI2"             : 9.            # adimensional
          ,  "PionPT"                 : 500.0         # MeV
          ,  "PionIPCHI2"             : 9.            # adimensional
          ,  "PionPIDK"               : 0.            #adimensional
          ,  "KstarVCHI2"             : 9.0           # adimensional
          ,  "KstarPT"                : 900.0         # MeV
          ,  "KaonPIDK"               : 2.0           # adimensional
          ,  "KstarMassWin"           : 680.0         # MeV
          ,  "KstarAPT"               : 800.0         # MeV
          ,  "BMassWin"               : 500.0         # MeV
          ,  "BVCHI2"                 : 15.0          # adimensional
          ,  "BDOCA"                  : 0.3           # mm
          ,  "BIPCHI2"                : 25.           # adimensional
          ,  "BFDistanceCHI2"         : 81.           # adimensional
          ,  "SumPT"                  : 5000          # MeV 
         },
    'STREAMS' : [ 'Bhadron' ]
    }

# Bs2Q2Body4pi from BetaS (Sean Benson)
BetaSBs2Q2Body4pi = {
    'WGs'       :       ['Charmless'],
    'BUILDERTYPE'       : 'Bs2Q2Body4piConf',
    'CONFIG'  : {'Q2BPrescale'     : 1.,
                 'Q2BResMinPT'     : 900.,
                 'Q2BResMinP'      : 1.,
                 'Q2BResMaxMass'   : 1100.,
                 'Q2BResVtxChiDOF' : 9.,
                 'Q2BBMinM'        : 4500.,
                 'Q2BBMaxM'        : 5700.,
                 'Q2BBVtxChi2DOF'  : 9.,
                 'Q2BIPCHI2' : 25
                 },
    'STREAMS' : [ 'Bhadron' ]
    }

# Bs2Ph2PhiPhi from BetaS (Sebastian Schleich)
BetaSBs2PhiPhi  = {
    'WGs'               : ['Charmless'],
    'BUILDERTYPE'       : 'StrippingBs2PhiPhiConf',
    'CONFIG'   : {'KaonPT'              : 450      # MeV/c
                , 'KaonIPCHI2'          : 3.5      # adimensional
                , 'PhiPT'               : 0        # MeV/c
                , 'PhiPTsq'             : 2.0      # GeV*GeV/cc
                , 'PhiVertexCHI2pDOF'   : 25       # adimensional
                , 'PhiMassWindow'       : 25       # MeV/cc
                , 'PhiMassMax'          : 1090     # MeV/cc
                , 'BsVertexCHI2pDOF'    : 25       # adimensional
                , 'BsMassWindow'        : 300      # MeV/cc
                , 'WidePrescale'        : 0.05     # adimensional
                  },
    'STREAMS' : [ 'Bhadron' ]
    } 

