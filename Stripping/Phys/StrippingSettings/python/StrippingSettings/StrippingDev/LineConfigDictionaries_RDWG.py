"""
Line config dictionaries for the RD WG


Dictionary names should have the same name as the corresponding
StrippingSelections file containing the line builder instance.

   e.g  StrippingPromptCharm linebuilder should be added as:

      PromptCharm = { ... }
"""

from GaudiKernel.SystemOfUnits import * 

# Bd2KstarMuMu for MVA
# J. Dickens, M. Patel 

Bd2KstarMuMu = {
    'BUILDERTYPE' : 'StrippingBdToKstarMuMuConf',
    'CONFIG' : { 
    'UseNoPIDsHadrons'          : True,
    'Prescale_BdToKstarMuMu'    : 1.0,
    'Postscale_BdToKstarMuMu'   : 1.0,
    'Prescale_BdToKstarMuMuSS'  : 1.0,
    'Postscale_BdToKstarMuMuSS' : 1.0,
    'Prescale_BuToKMuMu'        : 1.0,
    'Postscale_BuToKMuMu'       : 1.0,
    'Prescale_BuToKMuMuSS'      : 1.0,
    'Postscale_BuToKMuMuSS'     : 1.0,
    'B_Comb_MassLow'      : 4600.0,
    'B_Comb_MassHigh'     : 6000.0,
    'B_MassLow'           : 4850.0,
    'B_MassHigh'          : 5780.0,
    'B_VertexCHI2'        :    6.0,
    'B_IPCHI2'            :   16.0,
    'B_DIRA'              :   0.9999,
    'B_FlightCHI2'        :  121.0,
    'B_Dau_MaxIPCHI2'     : 9.0, 
    'Dau_VertexCHI2'      :   12.0,
    'Dau_DIRA'            :   -0.9,
    'Kstar_Comb_MassLow'  :  550.0,
    'Kstar_Comb_MassHigh' : 2200.0,
    'Kstar_MassLow'       :  600.0,
    'Kstar_MassHigh'      : 2000.0,
    'Kstar_MinIPCHI2'     :    0.0,
    'Kstar_FlightChi2'    :   9.0, 
    'Kstar_Dau_MaxIPCHI2' : 9.0, 
    'Dimu_FlightChi2'     :   9.0, 
    'Dimu_Dau_MaxIPCHI2'  : 9.0, 
    'Track_CHI2nDOF'      :    5.0,
    'Hadron_MinIPCHI2'    :    9.0, 
    'Muon_MinIPCHI2'      :    9.0,
    'Muon_IsMuon'         :   False
    },
    'WGs' : [ 'RD' ] ,
    'STREAMS' : [ 'Dimuon' ]
    }



# B2XMuMu for RD
# P. Schaak, M. Patel & T.Blake

B2XMuMu =  {
    'BUILDERTYPE' : 'B2XMuMuConf',
    'CONFIG' : {
    'BVXCHI2NDOF'        : 4.0           # dimensionless
    , 'BIPCHI2'            : 9.0           # dimensionless 
    , 'BDIRA'              : 0.999968      # dimensionless
    , 'BFDCHI2'            : 100.0         # dimensionless
    , 'KpiMINIPCHI2'       : 9.0           # dimensionless
    , 'KpiTRACKCHI2'       : 4.0           # dimensionless         
    , 'KpiVXCHI2NDOF'      : 9.0           # dimensionless       
    , 'MuonMINIPCHI2'      : 16.0           # dimensionless      
    , 'MuonTRACKCHI2'      : 4.0           # dimensionless    
    , 'MuonPID'            : 0.0           # dimensionless            
    , 'DimuonVXCHI2NDOF'   : 9.0           # dimensionless 
    , 'DimuonUPPERMASS'    : 5050.0        # MeV      
    , 'Pi0MINPT'           : 800.0         # MeV  
    , 'DplusLOWERMASS'     : 1600.0        # MeV
    , 'DplusUPPERMASS'     : 2300.0        # MeV 
    , 'KstarplusWINDOW'    : 300.0         # MeV
    },
    'WGs' : [ 'RD' ],
    'STREAMS' : [ 'Dimuon' ]
    }

# Bs2MuMuPhi
# P. Schaak

Bs2MuMuPhi = {
    'BUILDERTYPE'          : 'Bs2MuMuPhiConf' ,
    'CONFIG'               : {
    'BsIPCHI2'             : 9.0           # dimensionless
    , 'BsLT'               : 0.0002        # ns
    , 'BsVertexCHI2'       : 40.0          # dimensionless
    , 'KaonPIDK'           : 0             # dimensionless
    , 'KaonMINIPCHI2'      : 9.0           # dimensionless
    , 'MuonMINIPCHI2'      : 9.0           # dimensionless
    },
    'WGs' : [ 'RD' ] ,
    'STREAMS' : [ 'Dimuon' ]
    }


# Same-sign searches
# S. Redford & Wenbin 

B2XMuMuSS = {
    'BUILDERTYPE'   : 'B2XMuMuSSConf',
    'CONFIG'        : {
    'MuonP'         : 3000. ,    #MeV
    'MuonPT'        : 500.  ,    #MeV
    'MuonMINIPCHI2' : 4     ,    #adminensional
    
    'PionP'         : 2000. ,    #MeV
    'PionPT'        : 500.  ,    #MeV
    'PionMINIPCHI2' : 4     ,    #adminensional
    
    'KaonP'         : 2000. ,    #MeV
    'KaonPT'        : 500.  ,    #MeV
    'KaonMINIPCHI2' : 4     ,    #adminensional
    'DimuonMass'    : 0.  ,
    
    #'DimuonMass'    : 250.  ,    #MeV
    'BVCHI2DOF'     : 7    ,    #adminensional                              
    'BDIRA'         : 0.9995 ,    #adimensional
    'BIPCHI2'       : 30    ,    #adimensional
    'BMassWin'      : 400.  ,    #MeV, mass window
    
    'B2PiMuMuOSLinePrescale'  : 1 ,
    'B2PiMuMuOSLinePostscale' : 1 ,
    'B2PiMuMuSSLinePrescale'  : 1 ,
    'B2PiMuMuSSLinePostscale' : 1 ,
    'B2KMuMuOSLinePrescale'   : 1 ,
    'B2KMuMuOSLinePostscale'  : 1 ,
    'B2KMuMuSSLinePrescale'   : 1 ,
    'B2KMuMuSSLinePostscale'  : 1 
    },
    'WGs' : [ 'RD' ] ,
    'STREAMS' : [ 'Dimuon' ]
    }


# HyperCP lines
# Vanya + Andrei

HyperCP = {
    'BUILDERTYPE' : 'StrippingHyperCPXConf',
    'CONFIG' : {
    'ProtonCuts' : ' ( TRCHI2DOF < 5 ) & ( 0 < PIDp  - PIDpi ) & ( BPVIPCHI2() > 12 ) ' , 
    'MuonCuts'   : ' ( TRCHI2DOF < 5 ) & ISMUON                & ( BPVIPCHI2() > 12 ) ' ,                
    'PionCuts'   : ' ( TRCHI2DOF < 5 )                         & ( BPVIPCHI2() > 12 ) ' ,
    'SigmaCTau'  :   5 *         mm ,
    'SigmaMass'  : 250 *        MeV ,
    'DsCTau'     : 100 * micrometer ,
    'DsMass'     : 250 *        MeV ,
    'DplusCTau'  : 200 * micrometer ,
    'DplusMass'  : 250 *        MeV ,
    'PrimaryVertices' : True ,
    'Preambulo'       : [
    'chi2vx = VFASPF(VCHI2) '                    , 
    "from GaudiKernel.PhysicalConstants import c_light" , 
    "ctau   = BPVLTIME ( 9 ) * c_light "  ,
    "phi    = in_range ( 920 * MeV , AM23 , 1120 * MeV )"
    ] ,
    'SigmaPrescale' : 1.0 ,
    'DplusPrescale' : 1.0 ,
    'DsPrescale'    : 1.0 ,
    },
    'WGs' : [ 'RD' ],
    'STREAMS' : [ 'Dimuon' ]
    }

# Same-sign searches
# Wenbin 

B2SameChargeMuon = {
    'BUILDERTYPE'  : 'StrippingB2SameChargeMuonConf',
    'CONFIG'  : { 
    'LinePrescale'        :    1.   ,
    'LinePostscale'       :    1.
    },
    'WGs' : [ 'RD' ],
    'STREAMS' : [ 'Dimuon' ]
    }


# Bu2LLK ( B+ -> ee K and mu mu K )
# P. Koppenburg

Bu2LLK = {
    'BUILDERTYPE'         : 'Bu2LLKConf',
    'CONFIG'              : {
    'BFlightCHI2'         : 100       # adimentional 
    ,  'BDIRA'               : 0.9995     # adimentional    TIGHTENED
    ,  'BIPCHI2'             : 25        # adimentional  
    ,  'BVertexCHI2'         : 16        # adimentional
    ,  'DiLeptonPT'          : 0         # MeV (not used)
    ,  'DiLeptonFDCHI2'      : 16        # adimentional
    ,  'DiLeptonIPCHI2'      : 9         # adimentional
    ,  'LeptonIPCHI2'        : 16        # adimentional      TIGHTENED
    ,  'LeptonPT'            : 800       # MeV               
    ,  'KaonIPCHI2'          : 16        # adimentional      TIGHTENED
    ,  'KaonPT'              : 800       # MeV               LOOSENED
    ,  'UpperMass'           : 5500      # MeV (Higher bound of signal box)
    ,  'Bu2eeKLinePrescale'  : 1
    ,  'Bu2eeKLinePostscale' : 1
    ,  'Bu2mmKLinePrescale'  : 1
    ,  'Bu2mmKLinePostscale' : 1
    },
    'WGs'    : [ 'RD' ] ,
    'STREAMS' : [ 'Dimuon' ] 
    }

# B2MuMuMuMuLines
# J. Albrecht

B2MuMuMuMuLines = {
    'BUILDERTYPE' : 'B2MuMuMuMuLinesConf',
    'CONFIG'      : {
    'B2MuMuMuMuLinePrescale'    : 1,
    'B2MuMuMuMuLinePostscale'   : 1,
    },
    'WGs' : [ 'RD' ],
    'STREAMS' : [ 'Dimuon' ]
    }


# Stripping TriMuons
# Vanya 

TriMuon = {
    'BUILDERTYPE'  : 'StrippingTriMuonsConf',
    'CONFIG'       :  {
    'GoodMuons'    : " ( PT > 300 * MeV ) & ( TRCHI2DOF < 5  ) & ( BPVIPCHI2 () >  9 ) " ,
    'GoodMuonsForBc' : " ( BPVIPCHI2 () >  9 ) " ,
    'TightMuons'   : " ( PT > 1.9 * GeV ) & ( BPVIPCHI2 () > 25 ) " ,
    'HLT'          : " HLT_PASS_RE('(Hlt1|Hlt2).*Mu.*Decision') " , 
    '3mu-Prescale' : 1.00 , 
    'Bc-Prescale'  : 1.00 , 
    'Tau-Prescale' : 1.00 , 
    'Preambulo'    : [
    'chi2vx = VFASPF(VCHI2) '                               ,
    "from GaudiKernel.PhysicalConstants import c_light"     ,
    "ctau    =                    BPVLTIME ( ) * c_light "  ,
    "ctauBc  = PDGM('B_c+') / M * BPVLTIME ( ) * c_light "  
    ]
    },
    'WGs'    : [ 'RD' ] ,
    'STREAMS' : [ 'Dimuon' ] 
    }



# Lines for phi mu mu / f0 mu mu
# Liming Zhang

Bs2PhiMuMu = { 
    'BUILDERTYPE' : 'Bs2PhiMuMuLinesConf',
    'WGs'         : ['RD'],
    'STREAMS'     : ['Dimuon'],
    'CONFIG'      : {
    "MINIPCHI2"      : 4.00   # adimensiional
    ,"TRCHI2"        : 10.0   # adimensiional
    ,"KaonPIDK"      : 1e-10  # adimensiional
    ,"PhiPT"         : 100    # MeV
    ,"MuonMINIPCHI2" : 2.25   # adimensiional
    ,"MuonPIDmu"     : -5.0  # adimensiional
    ,"MuonTRCHI2"    : 10.0   # adimensiional
    ,"BsMassWin"     : 250.0  # MeV
    ,"BsVCHI2DOF"    : 8.0   # adimensiional
    ,"BsDIRA"        : 0.9993  # adimensiional
    ,"BsFDCHI2"      : 25.0   # adimensiional
    ,"PionPIDK"      : 10.0   # adimensiional
    ,"f0MassWin"     : 200.0  # MeV
    ,"VCHI2"         : 10.0   # adimensiional
    ,"BsIPCHI2"      : 36.0  # adimensiional
    ,"DocaChi2Max"   : 20  #mm
    }
    }

# Radiative lines
# F. Soomro, A. Puig 

B2XGamma = {
    'BUILDERTYPE' : 'StrippingB2XGammaConf',
    'CONFIG' : {'TrIPchi2Phi'          : 10.       # Dimensionless
                ,'TrIPchi2Kst'         : 10.       # Dimensionless
                ,'PhiMassWin'          : 15.       # MeV
                ,'KstMassWin'          : 100.      # MeV
                ,'KstMassWinSB'        : 200.      # MeV
                ,'BsMassWin'           : 1000.     # MeV
                ,'B0MassWin'           : 1000.     # MeV
                ,'BMassWinSB'          : 2000.     # MeV
                ,'BsDirAngle'          : 0.02      # radians
                ,'B0DirAngle'          : 0.02      # radians
                ,'BDirAngleMoni'       : 0.06      # radians
                ,'BsPVIPchi2'          : 15.       # Dimensionless
                ,'B0PVIPchi2'          : 15.       # Dimensionless
                ,'photonPT'            : 2600.     # MeV
                ,'PhiVCHI2'            : 15.       # dimensionless
                ,'KstVCHI2'            : 15.       # dimensionless
                ,'TrChi2'              : 5.        # dimensionless
                # Pre- and postscales
                ,'Bs2PhiGammaWideBMassPreScale'      : 0.1
                ,'Bs2PhiGammaWideBMassPostScale'     : 1.0
                ,'Bs2PhiGammaLooseDiraPreScale'      : 0.1
                ,'Bs2PhiGammaLooseDiraPostScale'     : 1.0
                ,'Bs2PhiGammaPreScale'               : 1.0
                ,'Bs2PhiGammaPostScale'              : 1.0
                ,'Bd2KstGammaWideBMassPreScale'      : 0.05
                ,'Bd2KstGammaWideBMassPostScale'     : 1.0
                ,'Bd2KstGammaLooseDiraPreScale'      : 0.05
                ,'Bd2KstGammaLooseDiraPostScale'     : 1.0
                ,'Bd2KstGammaWideKstMassPreScale'    : 0.05
                ,'Bd2KstGammaWideKstMassPostScale'   : 1.0
                ,'Bd2KstGammaWidePreScale'           : 0.05
                ,'Bd2KstGammaWidePostScale'          : 1.0
                ,'Bd2KstGammaPreScale'               : 1.0
                ,'Bd2KstGammaPostScale'              : 1.0
                },
    'WGs'    : ['RD'],
    'STREAMS' : [ 'Radiative' ] 
    }

# Bd2eeKstar
# J. He

Bd2eeKstar = {
    'BUILDERTYPE'             : 'Bd2eeKstarConf',
    'CONFIG'                  : {
    'LinePrescale'            :    1.   ,
    'LinePostscale'           :    1.   ,
    'ElectronPT'              :  350.   ,  # MeV
    'ElectronTrackCHI2pNDOF'  :  100.   ,
    'ElectronIPCHI2'          :    2.25 ,
    'ElectronPIDepi'          :   -2.   ,          
    'eeCombMinMass'           :    0.   ,  # MeV         
    'eeCombMaxMass'           : 1550.   ,  # MeV   
    'eeVertexCHI2'            :   15.   ,  
    'eeMinMass'               :   20.   ,  # MeV 
    'eeMaxMass'               : 1500.   ,  # MeV
    'eeFD'                    :    1.   ,  # mm
    'KaonPT'                  :  400.   ,  # MeV 
    'KaonP'                   : 3000.   ,  # MeV  
    'KaonTrackCHI2pNDOF'      :    5.   , 
    'KaonIPCHI2'              :    4.   , 
    'KaonPIDKpi'              :   -5.   , 
    'PionPT'                  :  300.   ,  # MeV
    'PionP'                   : 3000.   ,  # MeV 
    'PionTrackCHI2pNDOF'      :    5.   , 
    'PionIPCHI2'              :    4.   , 
    'PionPIDpiK'              :   10.   ,  # PIDpi-PIDK > -5, i.e., PIDK<5 
    'KstarComMassW'           :  200.   ,  # MeV  
    'KstarVertexCHI2'         :   25.   , 
    'KstarMassW'              :  130.   ,  # MeV
    'KstarIPCHI2'             :    1.   , 
    'KstarFDCHI2'             :    1.   , 
    'BComMassW'               : 1200.   ,  # MeV
    'BVertexCHI2'             :    9.   ,  # /ndf
    'BMassW'                  : 1000.   ,  # MeV  
    'BIPCHI2'                 :   64.   ,  # pointing
    'BFDCHI2'                 :    9.   , 
    'BDIRA'                   :    0.999, 
    'BIP'                     :    0.05    # mm
    },
    'WGs'    : [ 'RD' ],
    'STREAMS' : [ 'Dielectron' ]
    }


# B2JpsiKstar, Jpsi -> ee 
# J. He

Bd2JpsieeKstar = {
    'BUILDERTYPE' : 'Bd2JpsieeKstarConf',
    'CONFIG'      : {
    'LinePrescale'            :    1.   ,
    'LinePostscale'           :    1.   ,
    'ElectronPT'              :  350.   ,  # MeV
    'ElectronTrackCHI2pNDOF'  :  100.   ,
    'ElectronIPCHI2'          :    2.25 ,
    'ElectronPIDepi'          :   -2.   ,          
    'eeCombMinMass'           : 2100.   ,  # MeV         
    'eeCombMaxMass'           : 4300.   ,  # MeV   
    'eeVertexCHI2'            :   15.   ,  
    'eeMinMass'               : 2200.   ,  # MeV 
    'eeMaxMass'               : 4200.   ,  # MeV
    'eeFD'                    :    1.   ,  # mm
    'KaonPT'                  :  400.   ,  # MeV 
    'KaonP'                   : 3000.   ,  # MeV  
    'KaonTrackCHI2pNDOF'      :    5.   , 
    'KaonIPCHI2'              :    4.   , 
    'KaonPIDKpi'              :   -5.   , 
    'PionPT'                  :  300.   ,  # MeV
    'PionP'                   : 3000.   ,  # MeV 
    'PionTrackCHI2pNDOF'      :    5.   , 
    'PionIPCHI2'              :    4.   , 
    'PionPIDpiK'              :   10.   ,  # PIDpi-PIDK > -5, i.e., PIDK<5 
    'KstarComMassW'           :  200.   ,  # MeV  
    'KstarVertexCHI2'         :   25.   , 
    'KstarMassW'              :  130.   ,  # MeV
    'KstarIPCHI2'             :    1.   , 
    'KstarFDCHI2'             :    1.   , 
    'BComMassW'               : 1200.   ,  # MeV
    'BVertexCHI2'             :    9.   ,  # /ndf
    'BMassW'                  : 1000.   ,  # MeV  
    'BIPCHI2'                 :   64.   ,  # pointing
    'BFDCHI2'                 :    9.   , 
    'BDIRA'                   :    0.999, 
    'BIP'                     :    0.05    # mm
    },
    'WGs' : [ 'RD' ],
    'STREAMS' : [ 'Dielectron' ] 
    }
