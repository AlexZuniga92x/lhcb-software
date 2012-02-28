from GaudiKernel.SystemOfUnits import *

MicroDSTNeuroBayesMuMu = {
    'BUILDERTYPE' : 'StrippingNeuroBayesMuMuConf',
    'CONFIG' : {'trackChi2'         :    5.0
              , 'MuMuMassMin'       :    0.0
              , 'MuMuMassMax'       :   12.0
              , 'MuMuMassHighMin'   :    8.5
              , 'MuMuMassHighMax'   :  130.0
              , 'JPsiMassMin'       :    3.04 # GeV
              , 'JPsiMassMax'       :    3.14 # GeV
              , 'vertexChi2'        :   25.0
              , 'NBCutJPsi'         :    0.7
              , 'NBCutMuMu'         :    0.95
              , 'NBCutMuMuHigh'     :   -0.7
              , 'ExpertiseJPsi'     : 'Muon/mumu_net_noip.nb'
              , 'ExpertiseMuMu'     : 'Muon/mumu_net_full.nb'
              , 'ExpertiseMuMuHigh' : 'Muon/mumuhighmass_full.nb'
              , 'NBVersionJPsi'     : "TuneSep2010"
              , 'NBVersionMuMu'     : "TuneSep2010"
              , 'NBVersionMuMuHigh' : "TuneSep2010"
              , 'PrescaleMuMu'      : 1.0
              , 'PostscaleMuMu'     : 1.0
              , 'PrescaleMuMuHigh'  : 1.0
              , 'PostscaleMuMuHigh' : 1.0
              , 'PrescaleJPsi'      : 1.0
              , 'PostscaleJPsi'     : 1.0
              },
    'STREAMS' : [ 'Leptonic' ],
    'WGs'    : [ 'BHQ' ]
    }


ChiCJPsiGammaConversion = {
    'BUILDERTYPE'  : 'StrippingChiCJPsiGammaConversionConf',
    'CONFIG'       : {    'trackChi2'               :    5.0
                          , 'JPsiMassMin'           :    3.04 # GeV
                          , 'JPsiMassMax'           :    3.14 # GeV
                          , 'JPsiVertexChi2'        :   25.0
                          , 'NBCutJPsi'             :    0.7
                          , 'ExpertiseJPsi'         : 'Muon/mumu_net_noip.nb'
                          , 'NBVersionJPsi'         : "TuneSep2010"
                          , 'eDLLe'                 :  -5.0
                          , 'GammaEEMass'           :   7.0 #MeV
                          , 'GammaEEChi2'           :   2.0
                          , 'GammaEETau'            :   5.0  #ps
                          , 'mMinChiCRaw'           :   2.9 #GeV
                          , 'mMaxChiCRaw'           :   4.1 #GeV
                          , 'mMinChiCFit'           :   3.0 #GeV
                          , 'mMaxChiCFit'           :   4.0 #GeV
                          , 'PrescaleChiC'          :   1.0
                          , 'PostscaleChiC'         :   1.0
                          , 'PrescaleChiCWS'        :   0.5
                          , 'PostscaleChiCWS'       :   1.0
                          },
    'STREAMS' : [ 'Leptonic' ],
    'WGs'    : [ 'BHQ' ]
    }

FullDSTDiMuon = {
    'BUILDERTYPE'       :       'DiMuonConf',
    'CONFIG'    : {
        'MicroDST'                                 :  False   ,
               
        # DiMuon line
        'DiMuon_Prescale'                          :     1.   ,
        'DiMuon_Postscale'                         :     1.   ,
        'DiMuon_checkPV'                           : False    ,  
       
        'DiMuon_MuonPT'                            :   650.   ,  # MeV
        'DiMuon_MuonP'                             : -8000.   ,  # MeV, no cut now
        'DiMuon_MuonTRCHI2DOF'                     :     5.   ,
        'DiMuon_MinMass'                           :  3000.   ,  # MeV
        'DiMuon_MaxMass'                           :  4000.   ,  # MeV
        'DiMuon_VCHI2PDOF'                         :    20.   ,
        'DiMuon_PT'                                :  3000.   ,  # MeV, no cut now

        # DiMuon Same Sign line
        'DiMuonSameSign_Prescale'                  :     0.05  ,
        'DiMuonSameSign_Postscale'                 :     1.   ,
        'DiMuonSameSign_checkPV'                   : False    ,

        # DiMuonPrescaled line
        'DiMuonPrescaled_Prescale'                 :     0.1  ,
        'DiMuonPrescaled_Postscale'                :     1.   ,
        'DiMuonPrescaled_checkPV'                  : False    ,
       
        'DiMuonPrescaled_MuonPT'                   :   650.   ,  # MeV
        'DiMuonPrescaled_MuonP'                    : -8000.   ,  # MeV, no cut now
        'DiMuonPrescaled_MuonTRCHI2DOF'            :     5.   ,
        'DiMuonPrescaled_MinMass'                  :  3000.   ,  # MeV
        'DiMuonPrescaled_MaxMass'                  :  4000.   ,  # MeV
        'DiMuonPrescaled_VCHI2PDOF'                :    20.   ,
        'DiMuonPrescaled_PT'                       : -1000.   ,  # MeV, no cut now
       
        # DiMuonExclusive line
        'DiMuonExclusive_Prescale'                 :     0.   ,
        'DiMuonExclusive_Postscale'                :     1.   ,
        'DiMuonExclusive_checkPV'                  :  True    ,
       
        'DiMuonExclusive_MuonPT'                   :   650.   ,  # MeV
        'DiMuonExclusive_MuonP'                    : -8000.   ,  # MeV, no cut now
        'DiMuonExclusive_MuonTRCHI2DOF'            :     5.   ,
        'DiMuonExclusive_MinMass'                  :  2900.   ,
        'DiMuonExclusive_VCHI2PDOF'                :    20.   ,
        'DiMuonExclusive_PT'                       : -1000.   ,  # MeV, no cut now
        'DiMuonExclusive_DZ'                       :    -1.   ,  # mm, upstream of any PV

        # DiMuonNoPV line
        'DiMuonNoPV_Prescale'                      :     1.   ,
        'DiMuonNoPV_Postscale'                     :     1.   ,
       
        # DiMuon High Mass line
        'DiMuonHighMass_Prescale'                  :     1.   ,
        'DiMuonHighMass_Postscale'                 :     1.   ,
        'DiMuonHighMass_checkPV'                   : False    ,
       
        'DiMuonHighMass_MuonPT'                    :   700.   , # MeV
        'DiMuonHighMass_MuonP'                     :  8000.   , # MeV
        'DiMuonHighMass_MuonTRCHI2DOF'             :     5.   ,
        'DiMuonHighMass_MinMass'                   :  8500.   , # MeV
        'DiMuonHighMass_VCHI2PDOF'                 :    20.   ,
        'DiMuonHighMass_PT'                        : -1000.   , # MeV, no cut now

        # DiMuon High Mass Same Sign line
        'DiMuonHighMassSameSign_Prescale'          :     0.2  ,
        'DiMuonHighMassSameSign_Postscale'         :     1.   ,
        'DiMuonHighMassSameSign_checkPV'           : False    ,

        # DiMuon Low Mass line
        'DiMuonLowMass_Prescale'                   :     0.   ,
        'DiMuonLowMass_Postscale'                  :     1.   ,
        'DiMuonLowMass_checkPV'                    : False    ,
       
        'DiMuonLowMass_MuonPT'                     :   650.   ,  # MeV
        'DiMuonLowMass_MuonP'                      : -8000.   ,  # MeV, no cut now
        'DiMuonLowMass_MuonTRCHI2DOF'              :     5.   ,
        'DiMuonLowMass_MinMass'                    :   500.   ,  # MeV
        'DiMuonLowMass_VCHI2PDOF'                  :    20.   ,      
        'DiMuonLowMass_PT'                         : -1000.   ,  # MeV, no cut now

        # Jpsi2MuMu line
        'Jpsi2MuMu_Prescale'                       :     1.   ,
        'Jpsi2MuMu_Postscale'                      :     1.   ,
        'Jpsi2MuMu_checkPV'                        : False    ,
       
        'Jpsi2MuMu_MuonPT'                         :   650.   ,  # MeV
        'Jpsi2MuMu_MuonP'                          : 10000.   ,  # MeV
        'Jpsi2MuMu_MuonPIDmu'                      :     0.   ,
        'Jpsi2MuMu_MuonTRCHI2DOF'                  :     5.   ,
        'Jpsi2MuMu_MinMass'                        :  3010.   ,  # MeV
        'Jpsi2MuMu_MaxMass'                        :  3170.   ,  # MeV
        'Jpsi2MuMu_VCHI2PDOF'                      :    20.   ,
        'Jpsi2MuMu_PT'                             :  3000.   ,  # MeV

        # Psi2MuMu line
        'Psi2MuMu_Prescale'                        :     1.   ,
        'Psi2MuMu_Postscale'                       :     1.   ,
        'Psi2MuMu_checkPV'                         : False    ,

        'Psi2MuMu_ParticleName'                    : "'psi(2S)'", # Particle Name, like "'psi(2S)'"  
        'Psi2MuMu_MuonPT'                          :  1000.   ,  # MeV
        'Psi2MuMu_MuonP'                           : 10000.   ,  # MeV
        'Psi2MuMu_MuonPIDmu'                       :     0.   ,
        'Psi2MuMu_MuonTRCHI2DOF'                   :     5.   ,
        'Psi2MuMu_MassWindow'                      :   100.   ,  # MeV
        'Psi2MuMu_VCHI2PDOF'                       :    20.   ,
        'Psi2MuMu_PT'                              :  3000.   ,   # MeV
       
        # DiMuonDetached line
        'DiMuonDetached_Prescale'                 :     0.   ,
        'DiMuonDetached_Postscale'                :     1.   ,
       
        'DiMuonDetached_MuonPT'                   :   500.   ,  # MeV
        'DiMuonDetached_MuonP'                    : -8000.   ,  # MeV, no cut now
        'DiMuonDetached_MuonPIDmu'                :    -5.   ,
        'DiMuonDetached_MuonTRCHI2DOF'            :     5.   ,
        'DiMuonDetached_MinMass'                  :  2950.   ,
        'DiMuonDetached_VCHI2PDOF'                :    20.   ,
        'DiMuonDetached_PT'                       : -1000.   ,  # MeV, no cut now
        'DiMuonDetached_DLS'                      :     5.   ,  # mm, upstream of any PV
       
        # Jpsi2MuMuDetached line
        'Jpsi2MuMuDetached_Prescale'                       :     1.   ,
        'Jpsi2MuMuDetached_Postscale'                      :     1.   ,
       
        'Jpsi2MuMuDetached_MuonPT'                         :   550.   ,  # MeV
        'Jpsi2MuMuDetached_MuonP'                          : -8000.   ,  # MeV, no cut now
        'Jpsi2MuMuDetached_MuonPIDmu'                      :     0.   ,
        'Jpsi2MuMuDetached_MuonTRCHI2DOF'                  :     5.   ,
        'Jpsi2MuMuDetached_MinMass'                        :  2996.916,  # MeV
        'Jpsi2MuMuDetached_MaxMass'                        :  3196.916,  # MeV
        'Jpsi2MuMuDetached_VCHI2PDOF'                      :    20.   ,
        'Jpsi2MuMuDetached_PT'                             : -1000.   ,  # MeV
        'Jpsi2MuMuDetached_DLS'                            :     3.   ,

        # Psi2MuMuDetachedDetached line
        'Psi2MuMuDetached_Prescale'                        :     1.   ,
        'Psi2MuMuDetached_Postscale'                       :     1.   ,

        'Psi2MuMuDetached_ParticleName'                    : "'psi(2S)'", # Particle Name, like "'psi(2S)'"  
        'Psi2MuMuDetached_MuonPT'                          :   550.   ,  # MeV
        'Psi2MuMuDetached_MuonP'                           : -8000.   ,  # MeV, no cut now
        'Psi2MuMuDetached_MuonPIDmu'                       :     0.   ,
        'Psi2MuMuDetached_MuonTRCHI2DOF'                   :     5.   ,
        'Psi2MuMuDetached_MassWindow'                      :   100.   ,  # MeV
        'Psi2MuMuDetached_VCHI2PDOF'                       :    20.   ,
        'Psi2MuMuDetached_PT'                              : -1000.   ,   # MeV, no cut now
        'Psi2MuMuDetached_DLS'                             :     5.  
        },
    'STREAMS' : [ 'Dimuon' ],
    'WGs'    : [ 'BHQ' ]
    }

MicroDSTDiMuon = {
    'BUILDERTYPE' : 'DiMuonConf',
    'CONFIG' : {
        'MicroDST'                                 :   True   ,
    
        # DiMuon line
        'DiMuon_Prescale'                          :     1.   ,
        'DiMuon_Postscale'                         :     1.   ,
        'DiMuon_checkPV'                           :  False   ,  
        
        'DiMuon_MuonPT'                            :   650.   ,  # MeV
        'DiMuon_MuonP'                             : -8000.   ,  # MeV, no cut now 
        'DiMuon_MuonTRCHI2DOF'                     :     5.   , 
        'DiMuon_MinMass'                           :  3000.   ,  # MeV
        'DiMuon_MaxMass'                           :  4000.   ,  # MeV
        'DiMuon_VCHI2PDOF'                         :    20.   , 
        'DiMuon_PT'                                :  3000.   ,  # MeV

        # DiMuon Same Sign line
        'DiMuonSameSign_Prescale'                  :     0.05 ,
        'DiMuonSameSign_Postscale'                 :     1.   ,
        'DiMuonSameSign_checkPV'                   :  False   ,

        # DiMuonPrescaled line
        'DiMuonPrescaled_Prescale'                 :     0.1  ,
        'DiMuonPrescaled_Postscale'                :     1.   ,
        'DiMuonPrescaled_checkPV'                  : False    ,
        
        'DiMuonPrescaled_MuonPT'                   :   650.   ,  # MeV
        'DiMuonPrescaled_MuonP'                    : -8000.   ,  # MeV, no cut now 
        'DiMuonPrescaled_MuonTRCHI2DOF'            :     5.   , 
        'DiMuonPrescaled_MinMass'                  :  3000.   ,  # MeV
        'DiMuonPrescaled_MaxMass'                  :  4000.   ,  # MeV
        'DiMuonPrescaled_VCHI2PDOF'                :    20.   , 
        'DiMuonPrescaled_PT'                       : -1000.   ,  # MeV, no cut now 

        # DiMuonExclusive line
        'DiMuonExclusive_Prescale'                 :     1.   ,
        'DiMuonExclusive_Postscale'                :     1.   ,
        'DiMuonExclusive_checkPV'                  :  True    ,
        
        'DiMuonExclusive_MuonPT'                   :   650.   ,  # MeV
        'DiMuonExclusive_MuonP'                    : -8000.   ,  # MeV, no cut now 
        'DiMuonExclusive_MuonTRCHI2DOF'            :     5.   , 
        'DiMuonExclusive_MinMass'                  :  2900.   ,
        'DiMuonExclusive_VCHI2PDOF'                :    20.   ,
        'DiMuonExclusive_PT'                       : -1000.   ,  # MeV, no cut now 
        'DiMuonExclusive_DZ'                       :    -1.   ,  # mm, upstream of any PV

        # DiMuonNoPV line
        'DiMuonNoPV_Prescale'                      :     1.   ,
        'DiMuonNoPV_Postscale'                     :     1.   ,
               
        # DiMuon High Mass line
        'DiMuonHighMass_Prescale'                  :     1.   ,
        'DiMuonHighMass_Postscale'                 :     1.   ,
        'DiMuonHighMass_checkPV'                   :  True    ,
        
        'DiMuonHighMass_MuonPT'                    :   650.   , # MeV
        'DiMuonHighMass_MuonP'                     : -8000.   , 
        'DiMuonHighMass_MuonTRCHI2DOF'             :     5.   , 
        'DiMuonHighMass_MinMass'                   :  8000.   , # MeV
        'DiMuonHighMass_VCHI2PDOF'                 :    20.   ,
        'DiMuonHighMass_PT'                        : -1000.   , # MeV, no cut now 

        # DiMuon High Mass Same Sign line
        'DiMuonHighMassSameSign_Prescale'          :     1.   ,
        'DiMuonHighMassSameSign_Postscale'         :     1.   ,
        'DiMuonHighMassSameSign_checkPV'           :  True    ,

        # DiMuon Low Mass line
        'DiMuonLowMass_Prescale'                   :     1.   ,
        'DiMuonLowMass_Postscale'                  :     1.   ,
        'DiMuonLowMass_checkPV'                    :  True    , 
        
        'DiMuonLowMass_MuonPT'                     :   650.   ,  # MeV
        'DiMuonLowMass_MuonP'                      : -8000.   ,  # MeV, no cut now
        'DiMuonLowMass_MuonTRCHI2DOF'              :     5.   , 
        'DiMuonLowMass_MinMass'                    :   500.   ,  # MeV
        'DiMuonLowMass_VCHI2PDOF'                  :    20.   ,       
        'DiMuonLowMass_PT'                         : -1000.   ,  # MeV, no cut now

        # Jpsi2MuMu line
        'Jpsi2MuMu_Prescale'                       :     1.   ,
        'Jpsi2MuMu_Postscale'                      :     1.   ,
        'Jpsi2MuMu_checkPV'                        :  True    ,
        
        'Jpsi2MuMu_MuonPT'                         :   650.   ,  # MeV
        'Jpsi2MuMu_MuonP'                          : -8000.   ,  # MeV, no cut now
        'Jpsi2MuMu_MuonPIDmu'                      :     0.   ,  
        'Jpsi2MuMu_MuonTRCHI2DOF'                  :     5.   , 
        'Jpsi2MuMu_MinMass'                        :  3010.   ,  # MeV
        'Jpsi2MuMu_MaxMass'                        :  3170.   ,  # MeV
        'Jpsi2MuMu_VCHI2PDOF'                      :    20.   ,
        'Jpsi2MuMu_PT'                             :  3000.0  ,  # MeV

        # Psi2MuMu line
        'Psi2MuMu_Prescale'                        :     1.   ,
        'Psi2MuMu_Postscale'                       :     1.   ,
        'Psi2MuMu_checkPV'                         :  True    ,

        'Psi2MuMu_ParticleName'                    : "'psi(2S)'", # Particle Name, like "'psi(2S)'"   
        'Psi2MuMu_MuonPT'                          :  1000.   ,  # MeV
        'Psi2MuMu_MuonP'                           :  8000.   ,  # MeV
        'Psi2MuMu_MuonPIDmu'                           :     0.   , 
        'Psi2MuMu_MuonTRCHI2DOF'                   :     5.   , 
        'Psi2MuMu_MassWindow'                      :   120.   ,  # MeV
        'Psi2MuMu_VCHI2PDOF'                       :    20.   ,
        'Psi2MuMu_PT'                              :  2000.   ,  # MeV

        # DiMuonDetached line
        'DiMuonDetached_Prescale'                 :     1.   ,
        'DiMuonDetached_Postscale'                :     1.   ,
        
        'DiMuonDetached_MuonPT'                   :   500.   ,  # MeV
        'DiMuonDetached_MuonP'                    : -8000.   ,  # MeV, no cut now
        'DiMuonDetached_MuonPIDmu'                :    -5.   ,
        'DiMuonDetached_MuonTRCHI2DOF'            :     5.   , 
        'DiMuonDetached_MinMass'                  :  2950.   ,
        'DiMuonDetached_VCHI2PDOF'                :    20.   ,
        'DiMuonDetached_PT'                       : -1000.   ,  # MeV, no cut now 
        'DiMuonDetached_DLS'                      :     5.   ,  # mm, upstream of any PV
        
        # Jpsi2MuMuDetached line
        'Jpsi2MuMuDetached_Prescale'                       :     1.   ,
        'Jpsi2MuMuDetached_Postscale'                      :     1.   ,
        
        'Jpsi2MuMuDetached_MuonPT'                         :   500.   ,  # MeV
        'Jpsi2MuMuDetached_MuonP'                          : -8000.   ,  # MeV, no cut now
        'Jpsi2MuMuDetached_MuonPIDmu'                      :    -5.   , 
        'Jpsi2MuMuDetached_MuonTRCHI2DOF'                  :     5.   , 
        'Jpsi2MuMuDetached_MinMass'                        :  2976.916,  # MeV
        'Jpsi2MuMuDetached_MaxMass'                        :  3216.916,  # MeV
        'Jpsi2MuMuDetached_VCHI2PDOF'                      :    20.   ,
        'Jpsi2MuMuDetached_PT'                             : -1000.   ,  # MeV
        'Jpsi2MuMuDetached_DLS'                            :     3.   ,

        # Psi2MuMuDetachedDetached line
        'Psi2MuMuDetached_Prescale'                        :     1.   ,
        'Psi2MuMuDetached_Postscale'                       :     1.   ,

        'Psi2MuMuDetached_ParticleName'                    : "'psi(2S)'", # Particle Name, like "'psi(2S)'"   
        'Psi2MuMuDetached_MuonPT'                          :   500.   ,  # MeV
        'Psi2MuMuDetached_MuonP'                           : -8000.   ,  # MeV, no cut now
        'Psi2MuMuDetached_MuonPIDmu'                       :    -5.   , 
        'Psi2MuMuDetached_MuonTRCHI2DOF'                   :     5.   , 
        'Psi2MuMuDetached_MassWindow'                      :   120.   ,  # MeV
        'Psi2MuMuDetached_VCHI2PDOF'                       :    20.   ,
        'Psi2MuMuDetached_PT'                              : -1000.   ,   # MeV, no cut now
        'Psi2MuMuDetached_DLS'                             :     5.
        }, 
    'STREAMS' : [ 'Leptonic' ] ,
    'WGs'    : [ 'BHQ' ]
    }

X2psi = {
    'BUILDERTYPE' : 'X2psiPiPiConf',
    'CONFIG' : {
    'MuonPT'           :     0.9 , # GeV
    'MuonP'            :     8.  , # GeV
    'MuonPIDmu'        :    -5.  , 
    'MuonTRCHI2DOF'    :     5.  ,
    'JpsiPT'           :     3.  , # GeV
    'JpsiMassCuts'     :  "((in_range(3.04*GeV,MM,3.14*GeV)) | (in_range(3.635*GeV,MM,3.735*GeV)))",
    'JpsiVCHI2PDOF'    :    20.  ,
    'PionCuts'         :  "(PT>0.5*GeV) & (TRCHI2DOF<5)" ,
    'KaonCuts'         :  "(PT>0.5*GeV) & (TRCHI2DOF<5)" ,
    'CombMinMass'      :     3.5 , # GeV, before Vtx fit
    'CombMaxMass'      :     5.1 , # GeV, before Vtx fit
    'XVCHI2PDOF'       :    16.  ,
    'MinMass'          :     3.6 , # GeV, after Vtx fit
    'MaxMass'          :     5.0 , # GeV, after Vtx fit
    'SSPrescale'       :     0.2 
    },
    'STREAMS' : [ 'Leptonic' ] ,
    'WGs'    : [ 'BHQ' ]
  }

X2psiGamma = {
    'BUILDERTYPE' : 'X2psiGammaConf',
    'CONFIG' : {
    'MuonPT'           :     0.7 , # GeV
    'MuonP'            :     0.  , # GeV
    'MuonPIDmu'        :    -5.  , 
    'MuonTRCHI2DOF'    :     5.  ,
    'JpsiPT'           :     3.  , # GeV
    'JpsiMassCuts'     :  "((in_range(3.04*GeV,MM,3.14*GeV)) | (in_range(3.635*GeV,MM,3.735*GeV)))",
    'JpsiVCHI2PDOF'    :    20.  ,
    'GammaCuts'        :  "(PT>0.6*GeV) & (P>5*GeV) & (CL>0.5)" ,
    'CombMinMass'      :     3.0 , # GeV, no Vtx fit
    'CombMaxMass'      :     5.0   # GeV, no Vtx fit
    },
    'STREAMS' : [ 'Leptonic' ] ,
    'WGs'    : [ 'BHQ' ]
  }

B2psiX = {
    'BUILDERTYPE' : 'PsiX_BQ_Conf',
    'CONFIG' : {
    ## PV-requiremens
    #
    'CheckPV'   : True ,
    #
    ## c*tau cut for B-hadrons 
    #
    'CTAU'      : 100 * micrometer , 
    'CTAU_BC'   :  50 * micrometer , 
    'CTAU_Kst'  : 150 * micrometer , 
    #
    ## muon selection for  psi(') -> mu+ mu-
    #
    'MuonCut'   : """
    ISMUON &
    ( PT            >  550 * MeV ) &
    ( PIDmu - PIDpi >    0       ) &
    ( CLONEDIST     > 5000       )     
    """ , 
    #
    ## tight (K,pi) for 5(K,pi), 6(K.pi)-decays
    #
    'TightPi' : ' ( P > 3.2 * GeV ) & HASRICH & ( PIDpi - PIDK  > 0 ) ' , 
    'TightK'  : ' ( P > 3.2 * GeV ) & HASRICH & ( PIDK  - PIDpi > 0 ) ' ,
    #
    ## useful shortcuts:
    #
    'Preambulo' : [
    ## shortcut for chi2 of vertex fit 
    'chi2vx = VFASPF(VCHI2) '                                  , 
    ## shortcut for the c*tau
    "from GaudiKernel.PhysicalConstants import c_light"        , 
    ## use the embedded cut for chi2(LifetimeFit)<16
    "ctau      = BPVLTIME ( 25 ) * c_light "                   ,
    "ctau_16   = BPVLTIME ( 16 ) * c_light "                   ,
    "ctau_9    = BPVLTIME (  9 ) * c_light "                   ,
    "APT23     = LoKi.AParticles.TransverseMomentum ( 2 , 3 )" ,
    ## Combination mass-cut for neutral beauty particles 
    "mb0_acut  = in_range ( 5.100 * GeV , AM , 5.550 * GeV ) " ,
    "mbc_acut  = in_range ( 5.100 * GeV , AM , 6.550 * GeV ) " ,
    ## mass-cut for beauty particles 
    "mb0_cut   = in_range ( 5.150 * GeV ,  M , 5.500 * GeV ) " ,
    "mbc_cut   = in_range ( 5.150 * GeV ,  M , 6.500 * GeV ) " ,
    ] ,
    #     ## Prescales 
    #     'B2PsiPiPrescale'   : 1.0 ,
    'B2PsiKPrescale'    : 1.0 ,
    #
    'B2Psi2PiPrescale'  : 1.0 ,
    'B2PsiKPiPrescale'  : 1.0 ,
    'B2Psi2KPrescale'   : 1.0 ,
    #
    'B2Psi3KPrescale'   : 1.0 ,
    'B2Psi3PiPrescale'  : 1.0 ,
    'B2Psi3KPiPrescale' : 1.0 ,
    #
    'B2Psi4PiPrescale'  : 1.0 ,
    'B2Psi4KPiPrescale' : 1.0 ,
    'B2Psi4KPrescale'   : 1.0 ,
    #
    'B2Psi5PiPrescale'  : 1.0 ,
    'B2Psi5KPiPrescale' : 1.0 ,
    #
    'B2Psi6PiPrescale'  : 1.0 ,
    'B2Psi6KPiPrescale' : 1.0 ,
    #     'B2PsiKstPrescale'  : 1.0 
    # =========================================================================    
    },
    'STREAMS' : [ 'Leptonic' ] ,
    'WGs'    : [ 'BHQ' ]
    }

B2psiX0 = {
    'BUILDERTYPE' : 'PsiX0Conf',
    'CONFIG' : {
        ## PV-requiremens
    #
    'CheckPV'   : True ,
    #
    ## the transverse momentum of X0
    #
    'X0PT'      : 2.0 * GeV        ,
    #
    ## c*tau cut for B-candidates
    #
    'CTAU'      : 100 * micrometer ,
    #
    ## photon selection for eta' -> rho gamma
    #
    'GammaCut'  : ' ( PT > 250 * MeV ) ' ,
    #
    ## pi0 seclection for  eta/omega -> pi+ pi- pi0 ,
    #
    'Pi0Cut'    : """
    ( 250    * MeV < MINTREE ( 'gamma' == ID , PT ) ) 
    """ ,
    #
    ## eta> gamma gamma seelction for eta' -> pi+ pi- eta
    #
    'EtaCut'    :"""
    ( 250    * MeV < MINTREE ( 'gamma' == ID , PT ) ) 
    """ ,
    #
    ## muon selection for  psi(') -> mu+ mu-
    #
    'MuonCut'   : """
    ISMUON &
    ( PT  > 550 * MeV   ) &
    ( PIDmu - PIDpi > 0 ) &
    ( CLONEDIST     > 0 )     
    """ , 
    #
    ## useful shortcuts:
    #
    'Preambulo' : [
    ## shortcut for chi2 of vertex fit 
    'chi2vx = VFASPF(VCHI2) '                                 , 
    ## shortcut for the c*tau
    "from GaudiKernel.PhysicalConstants import c_light"       , 
    ## use the embedded cut for chi2(LifetimeFit)<16
    "ctau      = BPVLTIME ( 25 ) * c_light "                  ,
    ## Combination mass-cut for beauty particles 
    "mb_acut   = in_range ( 4.95 * GeV , AM , 5.65 * GeV ) "  ,
    ## mass-cut for beauty particles 
    "mb_cut    = in_range ( 5.00 * GeV , M  , 5.60 * GeV ) "  ,
    ] ,
    # =========================================================================
    ## Prescales 
    # =========================================================================
    'Eta2ggPrescale'       : 1.0 ,
    'Eta23piPrescale'      : 1.0 ,
    'Etap2rhogPrescale'    : 1.0 ,
    'Etap2pipietaPrescale' : 1.0 ,
    'OmegaPrescale'        : 1.0 
    # =========================================================================     
    },
    'STREAMS' : [ 'Leptonic' ] ,
    'WGs'    : [ 'BHQ' ]
    }


Bc2JpsiMu = {
    'BUILDERTYPE' : 'Bc2JpsiMuXConf',
    'CONFIG' : {
    'LinePrescale'        :    1.   ,
    'LinePostscale'       :    1.   ,
   
    'MuonTRCHI2DOF'       :    5.   ,  # adimentional
    'MuonPT'              : 1400.   ,  # MeV
    'MuonP'               :   -5.   ,  # MeV, not applied now
    'MuMuParticleName'    : "'J/psi(1S)'", # Particle Name, like "'psi(2S)'"
    'MuMuMassWindow'      :  150.   ,  # MeV, 10 sigma, may decreased to 150
    'MuMuVtxCHI2'         :    9.   ,  # adimentional
    'MuMuPT'              :  -10.   ,  # MeV, not applied
   
    'MuonBcTRCHI2DOF'     :    5.   ,  # adimentional    
    'MuonBcPT'            : 2500.   ,  # MeV
    'MuonBcP'             :   -5.   ,  # MeV, not applied now
    'BcUpperMass'         : 6400.   ,  # MeV, Upper limit for partial rec.
    'BcLowerMass'         : 3200.   ,  # MeV, Lower limit for partial rec.
    'BcVtxCHI2'           :    9.   ,  # adimentional
    'BcPT'                : 6000.      # MeV, May incrase up to 5000 MeV if needed      
    },
    'STREAMS' : [ 'Leptonic' ] ,
    'WGs'    : [ 'BHQ' ]
  }

Bc2JpsiH = {
    'BUILDERTYPE'       : 'Bc2JpsiHConf',
    'CONFIG'    : {
    'LinePrescale'        :    1.   ,
    'LinePostscale'       :    1.   ,
   
    'MuonTRCHI2DOF'       :    5.   ,  # adimentional
    'MuonPT'              : 1200.   ,  # MeV
    'MuonP'               :   -5.   ,  # MeV, not applied now
    'MuMuParticleName'    : "'J/psi(1S)'", # Particle Name, like "'psi(2S)'"
    'MuMuMassWindow'      :   80.   ,  # MeV, 10 sigma, may decreased to 150
    'MuMuVtxCHI2'         :    9.   ,  # adimentional
    'MuMuPT'              : 2000.   ,  # MeV
   
    'PionTRCHI2DOF'       :    5.   ,  # adimentional    
    'PionPT'              : 2000.   ,  # MeV
    'PionP'               :   -5.   ,  # MeV, not applied now
    'BcMassWindow'        :  400.   ,  # MeV, mass window
    'BcVtxCHI2'           :    9.   ,  # adimentional
    'BcPT'                : 6000.   ,  # MeV, May incrase up to 5000 MeV if needed

    'LifetimeCut'         :   ""    
    },
    'STREAMS' : [ 'Leptonic' ],
    'WGs'    : [ 'BHQ' ]
    }

Bc2JpsiHDetached = {
    'BUILDERTYPE'   : 'Bc2JpsiHConf',
    'CONFIG'    : {
    'LinePrescale'        :    1.   ,
    'LinePostscale'       :    1.   ,
   
    'MuonTRCHI2DOF'       :    5.   ,  # adimentional
    'MuonPT'              :  900.   ,  # MeV
    'MuonP'               :   -5.   ,  # MeV, not applied now
    'MuMuParticleName'    : "'J/psi(1S)'", # Particle Name, like "'psi(2S)'"
    'MuMuMassWindow'      :  100.   ,  # MeV, 10 sigma, may decreased to 150
    'MuMuVtxCHI2'         :    9.   ,  # adimentional
    'MuMuPT'              :  -10.   ,  # MeV, not applied
   
    'PionTRCHI2DOF'       :    5.   ,  # adimentional    
    'PionPT'              : 1400.   ,  # MeV
    'PionP'               :   -5.   ,  # MeV, not applied now
    'BcMassWindow'        :  400.   ,  # MeV, mass window
    'BcVtxCHI2'           :    9.   ,  # adimentional
    'BcPT'                : 4000.   ,  # MeV, May incrase up to 5000 MeV if needed      

    'LifetimeCut'         : " & (BPVLTIME()>0.1*ps)"
    },
    'STREAMS'   : [ 'Dimuon' ],
    'WGs'    : [ 'BHQ' ]
}

BuToKX3872 = {
    'BUILDERTYPE'       : 'StrippingBu2KX3872Conf',
    'CONFIG' : {
     'Prescale_BuToKX3872'    : 1.0,
    'Postscale_BuToKX3872'   : 1.0,

    'Prescale_BuToKX3872Loose'    : 1.0,
    'Postscale_BuToKX3872Loose'   : 1.0,

    'Prescale_BuToKPsi2S'    : 1.0,
    'Postscale_BuToKPsi2S'   : 1.0,

    'Prescale_BuToKPsi2SLoose'    : 1.0,
    'Postscale_BuToKPsi2SLoose'   : 1.0,

    # B cuts
    'Bu_Comb_MassWindow'         : 450.0,
    'Bu_Comb_MassWindowLoose'    : 500.0,
    'Bu_MassWindow'              : 400.0,
    'Bu_MassWindowLoose'         : 400.0,
    'Bu_VertexCHI2'              :   4.0,
    'Bu_VertexCHI2Loose'         :   5.0,
    'Bu_IPCHI2'                  :  15.0,
    'Bu_IPCHI2Loose'             :  20.0,
    'Bu_FlightCHI2'              :  40.0,
    'Bu_FlightCHI2Loose'         :  30.0,
    'Bu_DIRA'                    :   0.9995,
    'Bu_DIRALoose'               :   0.9995,

    # X3872 / Psi(2S) cuts
    'X3872_Comb_MassWindow'      : 180.0,
    'X3872_Comb_MassWindowLoose' : 220.0,
    'X3872_MassWindow'           : 150.0,
    'X3872_MassWindowLoose'      : 190.0,
    'X3872_VertexCHI2'           :   8.0,
    'X3872_VertexCHI2Loose'      :  10.0,

    # Track cuts
    'Track_CHI2nDOF'       : 4.0,

    # Kaon cuts
    'Kaon_MinIPCHI2'       : 6.0,
    'Kaon_MinIPCHI2Loose'  : 5.0,
    'Kaon_PT'              : 200.0,
    'Kaon_PTLoose'         : 150.0,

    # Pion cuts
    'Pion_MinIPCHI2'       : 6.0,
    'Pion_MinIPCHI2Loose'  : 5.0,

    # JPsi cuts
    'JPsi_MassWindow'       :  70.0,
    'JPsi_MassWindowLoose'  :  70.0,
    'JPsi_VertexCHI2'       :  10.0,
    'JPsi_VertexCHI2Loose'  :  10.0,

    # Muon cuts
    'Muon_MinIPCHI2'        :   3.0,
    'Muon_MinIPCHI2Loose'   :   2.0,
    'Muon_PT'               : 500.0,
    'Muon_IsMuon'           : True
     },
    'STREAMS' : [ 'Leptonic' ],
    'WGs'    : [ 'BHQ' ]
    }

HeavyBaryon = {
    'BUILDERTYPE'  : 'HeavyBaryonsConf',
    'CONFIG'       : {    'TRCHI2DOF'             :       4.,
                          'PionPIDK'              :       5.,
                          'KaonPIDK'              :      -5.,
                          'DLSForLongLived'       :       5.,
                          'XiMassWindow'          :      50.,
                          'XistarMassWindow'      :      150.,
                          'OmegaMassWindow'       :      500.,
                          "XibminusMassWindow"    :      300.,
                          "XibzeroMassWindow"     :      500.,
                          "OmegabminusMassWindow" :      500.    
                          },
    'STREAMS' : [ 'Dimuon' ],
    'WGs'    : ['BHQ']
    }

FullDSTDiElectron = {
    'BUILDERTYPE'	: 'DiElectronConf',
    'CONFIG'	: {
        'MicroDST'                                      :  False  ,
        
        # DiElectron line
        'DiElectron_Prescale'                           :     1.  ,
        'DiElectron_Postscale'                          :     1.  ,
        'DiElectron_checkPV'                            : False   , 
        
        'DiElectron_ElectronPT'                         :  1200.  ,  # MeV
        'DiElectron_ElectronP'                          : -8000.  ,  # MeV, no cut now
        'DiElectron_ElectronPIDe'                       :     5.  ,  
        'DiElectron_ElectronTRCHI2DOF'                  :     5.  , 
        'DiElectron_MinMass'                            :  2000.  ,  # MeV
        'DiElectron_VCHI2PDOF'                          :    16.  , 
        'DiElectron_PT'                                 :  2000.  ,  # MeV
        'DiElectron_TisTosSpecs'                        : { "Hlt1Global%TIS" : 0, "Hlt2Global%TIS" : 0 },

        # DiElectronLowMass line
        'DiElectronLowMass_Prescale'                    :     0.04,
        'DiElectronLowMass_Postscale'                   :     1.  ,
        'DiElectronLowMass_checkPV'                     : False   ,   
        
        'DiElectronLowMass_ElectronPT'                  :  1200.  ,  # MeV
        'DiElectronLowMass_ElectronP'                   : -8000.  ,  # MeV, no cut now
        'DiElectronLowMass_ElectronPIDe'                :     5.  , 
        'DiElectronLowMass_ElectronTRCHI2DOF'           :     5.  , 
        'DiElectronLowMass_MinMass'                     :    30.  ,  # MeV
        'DiElectronLowMass_VCHI2PDOF'                   :    16.  ,       
        'DiElectronLowMass_PT'                          : -1000.  ,  # MeV, no cut now
        
        # BiasedDiElectron line
        'BiasedDiElectron_Prescale'                     :     1.  ,
        'BiasedDiElectron_Postscale'                    :     1.  ,
        
        'BiasedDiElectron_ElectronPT'                   :   500.  ,  # MeV
        'BiasedDiElectron_ElectronP'                    : -8000.  ,  # MeV, no cut now
        'BiasedDiElectron_ElectronPIDe'                 :     5.  ,
        'BiasedDiElectron_ElectronMIPCHI2'              :    16.  , 
        'BiasedDiElectron_ElectronTRCHI2DOF'            :     5.  , 
        'BiasedDiElectron_MinMass'                      :  2000.  ,  # MeV
        'BiasedDiElectron_VCHI2PDOF'                    :    16.  ,
        'BiasedDiElectron_PT'                           : -1000.  ,  # MeV, no cut now 

        # BiasedDiElectronLowMass line
        'BiasedDiElectronLowMass_Prescale'              :     0.01 ,
        'BiasedDiElectronLowMass_Postscale'             :     1.  ,
        
        'BiasedDiElectronLowMass_ElectronPT'            :   500.  ,  # MeV
        'BiasedDiElectronLowMass_ElectronP'             : -8000.  ,  # MeV
        'BiasedDiElectronLowMass_ElectronPIDe'          :     0.  ,
        'BiasedDiElectronLowMass_ElectronMIPCHI2'       :    16.  ,  
        'BiasedDiElectronLowMass_ElectronTRCHI2DOF'     :     9.  , 
        'BiasedDiElectronLowMass_MinMass'               :    30.  ,  # MeV
        'BiasedDiElectronLowMass_VCHI2PDOF'             :    16.  ,
        'BiasedDiElectronLowMass_PT'                    : -1000.  ,  # MeV, no cut now
        
        # Jpsi2ee line
        'Jpsi2ee_Prescale'                              :     1.  ,
        'Jpsi2ee_Postscale'                             :     1.  ,
        'Jpsi2ee_checkPV'                               : False   ,   
        
        'Jpsi2ee_ElectronPT'                            :  1200.  ,  # MeV
        'Jpsi2ee_ElectronP'                             : -8000.  ,  # MeV, no cut now
        'Jpsi2ee_ElectronPIDe'                          :     5.  ,
        'Jpsi2ee_ElectronTRCHI2DOF'                     :     5.  , 
        'Jpsi2ee_MinMass'                               :  2000.  ,  # MeV
        'Jpsi2ee_MaxMass'                               :  3800.  ,  # MeV
        'Jpsi2ee_VCHI2PDOF'                             :    16.  ,
        'Jpsi2ee_PT'                                    :  2000.  ,  # MeV
        'Jpsi2ee_TisTosSpecs'                           : { "Hlt1Global%TIS" : 0, "Hlt2Global%TIS" : 0 }
        },
    'STREAMS' : [ 'Radiative' ],
    'WGs'    : [ 'BHQ' ]
    }

MicroDSTDiElectron = {
    'BUILDERTYPE'	:	'DiElectronConf',
    'CONFIG'	: {
        'MicroDST'                                      :  True   ,
        
        # DiElectron line
        'DiElectron_Prescale'                           :     1.  ,
        'DiElectron_Postscale'                          :     1.  ,        
        'DiElectron_checkPV'                            :  True   ,
        
        'DiElectron_ElectronPT'                         :  1100.  ,  # MeV
        'DiElectron_ElectronP'                          : -8000.  ,  # MeV, no cut now
        'DiElectron_ElectronPIDe'                       :     2.  ,  
        'DiElectron_ElectronTRCHI2DOF'                  :     5.  , 
        'DiElectron_MinMass'                            :  2000.  ,  # MeV
        'DiElectron_VCHI2PDOF'                          :    16.  , 
        'DiElectron_PT'                                 : -1000.  ,  # MeV, no cut now 
        'DiElectron_TisTosSpecs'                        : { "Hlt1Global%TIS" : 0, "Hlt2Global%TIS" : 0 },

        # DiElectronLowMass line
        'DiElectronLowMass_Prescale'                    :     1.  ,
        'DiElectronLowMass_Postscale'                   :     1.  ,
        'DiElectronLowMass_checkPV'                     :  True   , 
        
        'DiElectronLowMass_ElectronPT'                  :   750.  ,  # MeV
        'DiElectronLowMass_ElectronP'                   : -8000.  ,  # MeV, no cut now
        'DiElectronLowMass_ElectronPIDe'                :     2.  , 
        'DiElectronLowMass_ElectronTRCHI2DOF'           :     5.  , 
        'DiElectronLowMass_MinMass'                     :    30.  ,  # MeV
        'DiElectronLowMass_VCHI2PDOF'                   :    16.  ,       
        'DiElectronLowMass_PT'                          : -1000.  ,  # MeV, no cut now

        # BiasedDiElectron line
        'BiasedDiElectron_Prescale'                     :     1.  ,
        'BiasedDiElectron_Postscale'                    :     1.  ,
        
        'BiasedDiElectron_ElectronPT'                   :   500.  ,  # MeV
        'BiasedDiElectron_ElectronP'                    : -8000.  ,  # MeV, no cut now
        'BiasedDiElectron_ElectronPIDe'                 :     2.  ,
        'BiasedDiElectron_ElectronMIPCHI2'              :     4.  , 
        'BiasedDiElectron_ElectronTRCHI2DOF'            :     5.  , 
        'BiasedDiElectron_MinMass'                      :  2000.  ,  # MeV
        'BiasedDiElectron_VCHI2PDOF'                    :    16.  ,
        'BiasedDiElectron_PT'                           : -1000.  ,  # MeV, no cut now 

        # BiasedDiElectronLowMass line
        'BiasedDiElectronLowMass_Prescale'              :     0.1  ,
        'BiasedDiElectronLowMass_Postscale'             :     1.  ,

        'BiasedDiElectronLowMass_ElectronPT'            :   500.  ,  # MeV
        'BiasedDiElectronLowMass_ElectronP'             : -8000.  ,  # MeV
        'BiasedDiElectronLowMass_ElectronPIDe'          :     0.  ,
        'BiasedDiElectronLowMass_ElectronMIPCHI2'       :     4.  ,  
        'BiasedDiElectronLowMass_ElectronTRCHI2DOF'     :     5.  , 
        'BiasedDiElectronLowMass_MinMass'               :    30.  ,  # MeV
        'BiasedDiElectronLowMass_VCHI2PDOF'             :    16.  ,
        'BiasedDiElectronLowMass_PT'                    :  -1000. ,  # MeV, no cut now
        
        # Jpsi2ee line
        'Jpsi2ee_Prescale'                              :     1.  ,
        'Jpsi2ee_Postscale'                             :     1.  ,
        'Jpsi2ee_checkPV'                               :  True   , 
        
        'Jpsi2ee_ElectronPT'                            :  1200.  ,  # MeV
        'Jpsi2ee_ElectronP'                             : -8000.  ,  # MeV, no cut now
        'Jpsi2ee_ElectronPIDe'                          :     4.  ,
        'Jpsi2ee_ElectronTRCHI2DOF'                     :     5.  , 
        'Jpsi2ee_MinMass'                               :  2000.  ,  # MeV
        'Jpsi2ee_MaxMass'                               :  3800.  ,  # MeV
        'Jpsi2ee_VCHI2PDOF'                             :    16.  ,
        'Jpsi2ee_PT'                                    : -1500.0 ,  # MeV
        'Jpsi2ee_TisTosSpecs'                           : { "Hlt1Global%TIS" : 0, "Hlt2Global%TIS" : 0 }
        },
    'STREAMS' : [ 'Leptonic' ] ,
    'WGs'    : [ 'BHQ' ]
    }



TwoBodyPrompt = {
    'BUILDERTYPE'  :  'TwoBody_promptConf',
    'CONFIG' :  { 'nbody':              2,
                  'MinBMass':        3690.0,
                  'MaxBMass':        100000.0,
                  'MinBPt':          2000.0,
                  'MaxBVertChi2DOF':   10.0,
                  'MaxBPVVDCHI2Comb' : 10.,
                  'MinBPVDIRA' : 0.999,
                  'MinNvc':             3,
                  'MinFly':1,
                  'doLm':            True,
                  'doDz':            True,
                  'doDp':            True,
                  'doDs':            True,
                  'doLc':            True,
                  'doPh':            True,
                  'doJp':            True,
                  'doDS':            True,
                  'MinSingleTrackPtPi' : 6000.0,
                  'MinSingleTrackPtK' : 4000.0,
                  'MinSingleTrackPtp' : 3500.0,  
                  'MaxSingleTrackIPChi2DV' : 5.0,   # changed 
                  'MinDPIDpSingle' : 10.0,
                  'MinDPIDKSingle' : 5.0,
                  'MinLongLivedDauPt' : 200.0,
                  'MinLongLivedDauIPChi2' : 4.0,      
                  'MinLongLivedPt' : 3000.0,
                  'MinKsPt':2500.,
                  'MaxLongLivedIPChi2' : 10.0,
                  'MaxLongLivedVertChi2DOF' : 10.0,
                  'MinLongLivedPVVDChi2' : 200.0,   # halved
                  'MaxKsDeltaM' : 25.0, 
                  'MaxLmDeltaM' : 15.0, 
                  
                  'MinDPIDK' : 5.0,
                  'MinDPIDp' : 5.0,
                  
                  'MinDPIDpK' : 5.0,
                  'MinDPIDKforD0Dplus' : 0.0,
                  'MinDDauPt' : 250.0,
                  'MinDDauIPChi2' : 7.0,      
                  'MinDPt' : 1200.0,       
                  'MaxDIPChi2' : 10.0,   # changed
                  'MaxDVertChi2DOF' : 10.0,
                  'MinDPVVDChi2' : 50.0,  # halved/2 
                  'MaxDDeltaM' : 30.0,
                  
                  'MinDDauIPChi2Lc': 2. ,   
                  'MinDPVVDChi2Lc': 5., 
                                    
                  'MinPhDauPt' : 1000.0,      
                  'MaxPhDauIPChi2' : 5.0,  # changed      
                  'MinPhPt' : 2500.0,      
                  'MaxPhVertChi2DOF' : 10.0,
                  'MaxPhDeltaM' : 20.0, 
                  
                  'MinJpDauPt' : 500.0,      
                  'MaxJpDauIPChi2' : 10.0,   # changed
                  'MinJpPt' : 1000.0,      
                  'MaxJpVertChi2DOF' : 10.0,
                  
                  'MaxDSD0VertChi2DOF' : 10.0,
                  'MinDSD0PVVDChi2' : 60.0,   # halved/2
                  'MaxDSD0DeltaM' : 30.0,
                  'MinDSPt' : 1000.0,      
                  'MaxDSDeltaM':3.0,
                  'MaxDSSlowPionIPChi2':7.,  # added   
                  'MaxDSD0IPChi2':7.,  # added
                  'MinDSD0DauPt' : 250.0, # added
                  'MinDSD0DauIPChi2' : 9.0,
                  'MinDSSlowPionDauPt' : 150.0 # added
                  },
    'STREAMS' : [ 'Bhadron' ],
    'WGs'    : [ 'BHQ' ]
    }

Ccbar2Ppbar = {
    'BUILDERTYPE'	: 'Ccbar2PpbarConf',
    'CONFIG'	: { 'LinePrescale'     :    1.   ,
                    'LinePostscale'    :    1.   ,
                    
                    'SpdMult'          :   300.  , # dimensionless, Spd Multiplicy cut 
                    'ProtonPT'         :  1950.  , # MeV
                    'ProtonP'          :    10.  , # GeV
                    'ProtonTRCHI2DOF'  :     4.  ,
                    'ProtonPIDppi'     :    20.  , # CombDLL(p-pi)
                    'ProtonPIDpK'      :    15.  , # CombDLL(p-K)
                    'ProtonIPCHI2Cut'  :    ""   ,
                    'CombMaxMass'      :  4100.  , # MeV, before Vtx fit
                    'CombMinMass'      :  2750.  , # MeV, before Vtx fit
                    'MaxMass'          :  4000.  , # MeV, after Vtx fit
                    'MinMass'          :  2800.  , # MeV, after Vtx fit
                    'VtxCHI2'          :     9.  , # dimensionless
                    'CCCut'            :  " & (PT>6*GeV)"    
                   },
    'STREAMS' : [ 'Charm' ] ,
    'WGs'    : [ 'BHQ' ]
}

Ccbar2PpbarExclusive = {
    'BUILDERTYPE'       : 'Ccbar2PpbarConf',
    'CONFIG'    : {
        'LinePrescale'     :     1.  ,
        'LinePostscale'    :     1.  ,
        
        'SpdMult'          :    20.  , # dimensionless, Spd Multiplicy cut 
        'ProtonPT'         :   550.  , # MeV
        'ProtonP'          :    -2.  , # MeV
        'ProtonTRCHI2DOF'  :     5.  ,
        'ProtonPIDppi'     :    20.  , # CombDLL(p-pi)
        'ProtonPIDpK'      :    15.  , # CombDLL(p-K)
        'ProtonIPCHI2Cut'  :    ""   ,
        'CombMaxMass'      :  1.0e+6 , # MeV, before Vtx fit
        'CombMinMass'      :     0.  , # MeV, before Vtx fit
        'MaxMass'          :  1.0e+6 , # MeV, after Vtx fit
        'MinMass'          :     0.  , # MeV, after Vtx fit
        'VtxCHI2'          :     9.  , # dimensionless
        'CCCut'            :  ""     
        },
    'STREAMS' : [ 'Charm' ] ,
    'WGs'    : [ 'BHQ' ]
    }


Ccbar2PpbarDetached = {
    'BUILDERTYPE'       : 'Ccbar2PpbarConf',
    'CONFIG'    : {
    'LinePrescale'     :     1.  ,
    'LinePostscale'    :     1.  ,
    
    'SpdMult'          :   600.  , # dimensionless, Spd Multiplicy cut 
    'ProtonPT'         :  1000.  , # MeV
    'ProtonP'          :    -2.  , # MeV
    'ProtonTRCHI2DOF'  :     5.  ,
    'ProtonPIDppi'     :    15.  , # CombDLL(p-pi)
    'ProtonPIDpK'      :    10.  , # CombDLL(p-K)
    'ProtonIPCHI2Cut'  : " & (BPVIPCHI2()>9)",
    'CombMaxMass'      :  1.0e+6 , # MeV, before Vtx fit
    'CombMinMass'      :  2650.  , # MeV, before Vtx fit
    'MaxMass'          :  1.0e+6 , # MeV, after Vtx fit
    'MinMass'          :  2700.  , # MeV, after Vtx fit
    'VtxCHI2'          :     9.  , # dimensionless
    'CCCut'            :  " & (BPVDLS>5)"     
    },
    'STREAMS' : [ 'Charm' ] ,
    'WGs'    : [ 'BHQ' ]
    }

Ccbar2PhiPhi = {
    'BUILDERTYPE'       : 'Ccbar2PhiPhiConf',
    'CONFIG'    : {
        'TRCHI2DOF'        :     5.  ,
        'KaonPIDK'         :     2.  ,
        'KaonPT'           :   500.  , # MeV
        'PhiVtxChi2'       :    16.  ,
        'PhiMassW'         :    12.  , 
        'CombMaxMass'      :  4100.  , # MeV, before Vtx fit
        'CombMinMass'      :  2750.  , # MeV, before Vtx fit
        'MaxMass'          :  4000.  , # MeV, after Vtx fit
        'MinMass'          :  2800.  , # MeV, after Vtx fit
        'Phi_TisTosSpecs'  : { "Hlt1Global%TIS" : 0, "Hlt2Global%TIS" : 0 }
        },
    'STREAMS' : [ 'Charm' ] ,
    'WGs'    : [ 'BHQ' ]
    }

Ccbar2Baryons = {
    'BUILDERTYPE'  : 'Ccbar2BaryonsConf',
    'CONFIG'       : {
    'TRCHI2DOF'        :    5.   ,
    'CombMaxMass'      :  4100.  , # MeV, before Vtx fit
    'CombMinMass'      :  2750.  , # MeV, before Vtx fit
    'MaxMass'          :  4000.  , # MeV, after Vtx fit
    'MinMass'          :  2800.    # MeV, after Vtx fit
    },
    'STREAMS' : [ 'Charm' ],
    'WGs'    : ['BHQ']
    }

CharmAssociative = {
    'BUILDERTYPE'  :'StrippingCharmAssociativeConf',
    'CONFIG'  : {
    'PhotonCuts'      : ' PT > 4.0 * GeV  '                                 , 
    'MuonCuts'        : ' ISMUON & ( PT > 650 * MeV ) & ( TRCHI2DOF < 5 ) ' , 
    #
    ## Global Event cuts 
    #
    'PrimaryVertices' : True , 
    #
    ## Technicalities:
    #
    'Preambulo'       : [
    #
    ## shortcut for chi2 of vertex fit 
    'chi2vx = VFASPF(VCHI2) '                    ,
    #
    ## shortcut for the c*tau
    "from GaudiKernel.PhysicalConstants import c_light" , 
    "ctau   = BPVLTIME ( 9 ) * c_light "  , ## use the embedded cut for chi2(LifetimeFit)<9
    #
    ## dimuons:
    "psi             = ADAMASS ( 'J/psi(1S)' ) < 150 * MeV"         ,
    "psi_prime       = ADAMASS (   'psi(2S)' ) < 150 * MeV"         ,
    "psi_tight       =  ADMASS ( 'J/psi(1S)' ) < 100 * MeV "        ,
    "psi_prime_tight =  ADMASS (   'psi(2S)' ) < 100 * MeV "        ,
    "dimuon_heavy    = M > 4.8 * GeV "                              ,
    "dimuon_tight    = psi_tight | psi_prime_tight | dimuon_heavy " ,
    ] ,
    #
    ## monitoring ?
    'Monitor'     : False ,
    #
    ## pescales 
    'DiMuonAndGammaPrescale' : 1.0 ,
    'DoubleDiMuonPrescale'   : 1.0
    },
    'STREAMS' : [ 'Leptonic' ] ,
    'WGs'     : [ 'BHQ'    ]
    }
