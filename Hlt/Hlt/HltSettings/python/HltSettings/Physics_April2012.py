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

class Physics_April2012( object ):
    """
    Settings draft for 2012 running.
    
    WARNING :: DO NOT EDIT WITHOUT PERMISSION OF THE AUTHORS
    
    @author P.Koppenburg, G. Raven, J. Albrecht, R. Aaij
    @date 2011-04-29
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
        return '0x0039'

    def HltType(self) :
        self.verifyType( Physics_April2012 ) 
        return          'Physics_April2012'
    
    def Thresholds(self) :
        """
        Returns a dictionary of cuts
        """
        from Hlt1Lines.Hlt1TrackLines          import Hlt1TrackLinesConf
        from Hlt1Lines.Hlt1MuonLines           import Hlt1MuonLinesConf
        from Hlt1Lines.Hlt1ElectronLines       import Hlt1ElectronLinesConf
        from Hlt1Lines.Hlt1L0Lines             import Hlt1L0LinesConf
        from Hlt1Lines.Hlt1MBLines             import Hlt1MBLinesConf
        from Hlt1Lines.Hlt1CommissioningLines  import Hlt1CommissioningLinesConf
        from Hlt1Lines.Hlt1DisplVertexLines    import Hlt1DisplVertexLinesConf
        from Hlt2Lines.Hlt2CommissioningLines  import Hlt2CommissioningLinesConf
        from Hlt1Lines.Hlt1BeamGasLines        import Hlt1BeamGasLinesConf
        from Hlt2Lines.Hlt2diphotonDiMuonLines import Hlt2diphotonDiMuonLinesConf
        from Hlt2Lines.Hlt2InclusiveDiProtonLines import Hlt2InclusiveDiProtonLinesConf

        thresholds = { Hlt1TrackLinesConf :    { 'AllL0_PT'      :  1350
                                               , 'AllL0_P'       : 1000
                                               , 'AllL0_IP'      :     0.100
                                               , 'AllL0_IPChi2'  :    16
                                               , 'AllL0_TrChi2'  :     2.25
                                               , 'AllL0_GEC'     : 'Loose'
                                               , 'AllL0_Velo_NHits' : 7 
                                               , 'AllL0_Velo_Qcut' : 3  
                                               , 'AllL0_ValidateTT' : False 
                                               , 'Muon_PT'       :  1000 
                                               , 'Muon_P'        :  1000 
                                               , 'Muon_IP'       :     0.100
                                               , 'Muon_IPChi2'   :    16
                                               , 'Muon_TrChi2'   :     5.0  
                                               , 'Muon_GEC'      : 'Loose'
                                               , 'Muon_ValidateTT' : False
                                               , 'Photon_PT'     :  1200
                                               , 'Photon_P'      :  1000
                                               , 'Photon_IP'     :     0.100
                                               , 'Photon_IPChi2' :    16
                                               , 'Photon_TrChi2' :     2.25
                                               , 'Photon_L0Channels' : 'PhotonHi,ElectronHi' 
                                               , 'Photon_GEC'        : 'Loose'
                                               , 'Photon_ValidateTT' : False
                                               }
                     , Hlt1ElectronLinesConf : { 'SingleElectronNoIP_P'          : 20000
                                               , 'SingleElectronNoIP_PT'         : 10000
                                               , 'SingleElectronNoIP_TrChi2'     :     5
                                               , 'SingleElectronNoIP_TrNTHits'   :     0 #OFF
                                               , 'SingleElectronNoIP_Velo_NHits' :     0 #OFF
                                               , 'SingleElectronNoIP_Velo_Qcut'  :   999 #OFF
                                               , 'SingleElectronNoIP_GEC'        : 'Loose'
                                               , 'L0Channels': { 'SingleElectronNoIP' : ( 'Electron', ) }
                                               }
                     , Hlt1MuonLinesConf :     { 'SingleMuonHighPT_P'        : 8000
                                               , 'SingleMuonHighPT_PT'      : 4800
                                               , 'SingleMuonHighPT_TrChi2'  :    5
                                               , 'SingleMuonHighPT_GEC'     : 'Loose'
                                               , 'SingleMuonNoIP_P'         : 1000
                                               , 'SingleMuonNoIP_PT'        : 1300
                                               , 'SingleMuonNoIP_TrChi2'    :    5
                                               , 'SingleMuonNoIP_GEC'       : 'Loose'
                                               , 'DiMuonLowMass_VxDOCA'     :  0.2
                                               , 'DiMuonLowMass_VxChi2'     :   25
                                               , 'DiMuonLowMass_P'          : 1000
                                               , 'DiMuonLowMass_PT'         :  500
                                               , 'DiMuonLowMass_TrChi2'     :    5
                                               , 'DiMuonLowMass_M'          :    0
                                               , 'DiMuonLowMass_IPChi2'     :    3
                                               , 'DiMuonLowMass_GEC'        : 'Loose'
                                               , 'DiMuonHighMass_VxDOCA'    :  0.2
                                               , 'DiMuonHighMass_VxChi2'    :   25
                                               , 'DiMuonHighMass_P'         : 1000
                                               , 'DiMuonHighMass_PT'        :  500
                                               , 'DiMuonHighMass_TrChi2'    :    5
                                               , 'DiMuonHighMass_M'         : 2700
                                               , 'DiMuonHighMass_GEC'       : 'Loose'
                                               , 'MultiMuonNoIP_P'          : 1000
                                               , 'MultiMuonNoIP_PT'         :  500
                                               , 'MultiMuonNoIP_TrChi2'     :    5
                                               , 'MultiMuonNoIP_GT'         :  2.5
                                               , 'MultiMuonNoIP_GEC'        : 'Loose'
                                               , 'Prescale'                 : { 'Hlt1SingleMuonNoIP' : 0.01,
                                                                                'Hlt1MultiMuonNoIP'  : 0.0 }
                                               }
                       , Hlt1L0LinesConf :     {  'Postscale' : { 'Hlt1L0AnyRateLimited'       : 'RATE(1)'
                                                                , 'Hlt1L0AnyNoSPDRateLimited'  : 'RATE(1)'
                                                                }
                                               ,  'Prescale' : { 'Hlt1L0HighSumETJet' : 1 
                                                               , 'Hlt1L0AnyNoSPD'     : 0.01
                                                               }
                                               }
                       , Hlt1BeamGasLinesConf : { 'Postscale'               : { 'Hlt1BeamGasNoBeamBeam1'           : 'RATE(0.5)'
                                                                              , 'Hlt1BeamGasNoBeamBeam2'           : 'RATE(0.5)'
                                                                              , 'Hlt1BeamGasBeam1'                 : 'RATE(2)'
                                                                              , 'Hlt1BeamGasBeam2'                 : 'RATE(2)'
                                                                              , 'Hlt1BeamGasCrossingEnhancedBeam1' : 'RATE(0)'
                                                                              , 'Hlt1BeamGasCrossingEnhancedBeam2' : 'RATE(0)'
                                                                              , 'Hlt1BeamGasCrossingForcedReco'    : 'RATE(0.5)'
                                                                              , 'Hlt1BeamGasCrossingForcedRecoFullZ':'RATE(0.5)'
                                                                              , 'Hlt1BeamGasCrossingParasitic'     : 'RATE(1)'
                                                                              , 'Hlt1BeamGasHighRhoVertices'       : 'RATE(4)'
                                                                              }
                                                }
                       , Hlt1DisplVertexLinesConf: { "VertexDisplVertex_DOCABL"    : "2.0*mm"
                                                   , "VertexDisplVertex_VELO_NSP"  : "3"
                                                   , "VertexDisplVertex_VELO_NCSP" : "2"
                                                   , "VertexDisplVertex_VELOTrChi2": "2.5"
                                                   , "VertexDisplVertex_VX_DOCA"   : "0.3*mm"
                                                   , "VertexDisplVertex_VX_CHI2"   : "1000000000.0"
                                                   , "VertexDisplVertex_VX_RHO"    : "12.0*mm"
                                                   , "VertexDisplVertex_Tr_P"      : "10.0*GeV"
                                                   , "VertexDisplVertex_Tr_PT"     : "1.7*GeV"
                                                   , "VertexDisplVertex_Tr_CHI2"   : "2.5"
                                                   , "VertexDisplVertex_GEC"       : "Loose"
                                                   }
                       #, Hlt1ProtonLinesConf        : { 'Prescale' : {  'Hlt1DiProtonLowMult'        : 0.01, 
                       #                                                 'Hlt1DiProton'               : 0.01
                       #                                                } } 
                       , Hlt1CommissioningLinesConf : { 'Postscale' : { 'Hlt1ErrorEvent'   : 'RATE(0.01)' } }
                       , Hlt2CommissioningLinesConf : { 'Postscale' : { 'Hlt2ErrorEvent'   : 'RATE(0.01)' } }
                       # micro bias lines switched off for high mu physics running              
                       , Hlt1MBLinesConf :     { 'Prescale' : { 'Hlt1MBMicroBiasVelo'                : 0
                                                              , 'Hlt1MBMicroBiasTStation'            : 0
                                                              , 'Hlt1MBMicroBiasVeloRateLimited'     : 0
                                                              , 'Hlt1MBMicroBiasTStationRateLimited' : 0 }
                                               , 'MaxNoBiasRate' : 1000000.
                                               }
                       , Hlt2diphotonDiMuonLinesConf : { 'Prescale' : { 'Hlt2LowMultHadron'     :  1.0 # for 0x0035, this is already done in L0
                                                                      , 'Hlt2LowMultPhoton'     : 0.01
                                                                      } } 

                       #, Hlt2InclusiveDiProtonLinesConf: { 'Prescale' : { 'Hlt2DiProton'           :       0.001
                       #                                                   , 'Hlt2DiProtonLowMult'  :       0.001
                       #                                               } } 

                       }

        from Muons_draft2012 import Muons_draft2012
        __update_conf__(thresholds,  Muons_draft2012().Thresholds() )

        from Electrons_July2011 import Electrons_July2011
        __update_conf__(thresholds,  Electrons_July2011().Thresholds() )

        from Hadrons_draft2012 import Hadrons_draft2012
        __update_conf__(thresholds,  Hadrons_draft2012().Thresholds() )

        from DV_draft2012 import DV_draft2012
        __update_conf__(thresholds,  DV_draft2012().Thresholds() )

        from CharmLeptonic_draft2012 import CharmLeptonic_draft2012
        __update_conf__(thresholds, CharmLeptonic_draft2012().Thresholds() )

        return thresholds
                       
    def ActiveHlt2Lines(self) :
        """
        Returns a list of active lines
        """
        hlt2 = ['Hlt2PassThrough','Hlt2Lumi','Hlt2DebugEvent','Hlt2Forward','Hlt2ErrorEvent','Hlt2Transparent',
                'Hlt2diPhotonDiMuon',
                'Hlt2LowMultMuon',
                'Hlt2LowMultHadron',
                'Hlt2LowMultPhoton',
                'Hlt2LowMultElectron',
                'Hlt2LowMultHadron_nofilter',
                'Hlt2LowMultElectron_nofilter'
                ]

        from Express_Hlt2_draft2012 import Express_Hlt2_draft2012
        hlt2.extend( Express_Hlt2_draft2012().ActiveHlt2Lines() )

        from Muons_draft2012 import Muons_draft2012
        hlt2.extend( Muons_draft2012().ActiveHlt2Lines() )

        from Electrons_July2011 import Electrons_July2011
        hlt2.extend( Electrons_July2011().ActiveHlt2Lines() )

        from Hadrons_draft2012 import Hadrons_draft2012
        hlt2.extend( Hadrons_draft2012().ActiveHlt2Lines() )
       
        from DV_draft2012 import DV_draft2012 
        hlt2.extend( DV_draft2012().ActiveHlt2Lines() )

        from CharmLeptonic_draft2012 import CharmLeptonic_draft2012
        hlt2.extend( CharmLeptonic_draft2012().ActiveHlt2Lines() )
 
        return hlt2
       
    def ActiveHlt1Lines(self) :
        """
        Returns a list of active lines
        """
        lines =  [ 'Hlt1BeamGasNoBeamBeam1', 'Hlt1BeamGasNoBeamBeam2'
                 , 'Hlt1BeamGasBeam1', 'Hlt1BeamGasBeam2'
                 , 'Hlt1BeamGasCrossingEnhancedBeam1', 'Hlt1BeamGasCrossingEnhancedBeam2'
                 , 'Hlt1BeamGasCrossingForcedReco', 'Hlt1BeamGasCrossingForcedRecoFullZ'
                 , 'Hlt1BeamGasCrossingParasitic', 'Hlt1BeamGasHighRhoVertices'
                 , 'Hlt1NoPVPassThrough'
                 , 'Hlt1CharmCalibrationNoBias'
                 , 'Hlt1TrackMuon', 'Hlt1TrackAllL0', 'Hlt1TrackPhoton'
                 , 'Hlt1TrackForwardPassThrough','Hlt1TrackForwardPassThroughLoose'
                 , 'Hlt1VertexDisplVertex'
                 , 'Hlt1SingleMuonNoIP', 'Hlt1SingleMuonHighPT'
                 , 'Hlt1SingleElectronNoIP'
                 , 'Hlt1DiMuonLowMass', 'Hlt1DiMuonHighMass'
                 , 'Hlt1DiProtonLowMult', 'Hlt1DiProton'
                 , 'Hlt1Lumi', 'Hlt1LumiMidBeamCrossing'
                 , 'Hlt1L0Any','Hlt1L0AnyNoSPD'
                 , 'Hlt1L0HighSumETJet'
                 , 'Hlt1MBNoBias' 
                 , 'Hlt1ODINTechnical', 'Hlt1Tell1Error' , 'Hlt1ErrorEvent' # , 'Hlt1Incident' 
                 , 'Hlt1MBMicroBiasVelo','Hlt1MBMicroBiasTStation'
                 , 'Hlt1VeloClosingMicroBias' ]

        lines += [ i + 'RateLimited' for i in lines
                   if i.startswith('Hlt1MBMicroBias')
                   or i.startswith('Hlt1L0Any') ]

        return lines 
