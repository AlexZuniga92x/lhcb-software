class Debug_320Vis_300L0_NoLines_Hlt1_Aug09 :
    """
    Threshold settings for Hlt1 for 2010 conditions, no Hlt1 lines included for debugging of individual lines
    
    WARNING :: DO NOT EDIT WITHOUT PERMISSION OF THE AUTHORS
    
    @author A. Perez-Calero
    @date 2009-11-26
    """
    
    __all__ = ( 'ActiveHlt1Lines', 'ActiveHlt2Lines', 'Thresholds','L0TCK' )

    def verifyType(self,ref) :
        # verify self.ActiveLines is still consistent with
        # our types self.ActiveHlt2Lines and self.ActiveHlt1Lines...
        # so we can force that classes which inherit from us
        # and overrule either ActiveHlt.Lines also overrule
        # HltType...
        if ( self.ActiveHlt1Lines() != ref.ActiveHlt1Lines(self)  or
             self.ActiveHlt2Lines() != ref.ActiveHlt2Lines(self) ) :
            raise RuntimeError('Must update HltType when modifying ActiveHlt.Lines()')

    
    def L0TCK(self) :
        return '0xFF68'

    def HltType(self) :
        self.verifyType( Debug_320Vis_300L0_NoLines_Hlt1_Aug09 )
        return          'Debug_320Vis_300L0_NoLines_Hlt1_Aug09'

    def Thresholds(self) :
        """
        Returns a dictionary of cuts
        """
        from Hlt1Lines.Hlt1CommissioningLines  import Hlt1CommissioningLinesConf
        from Hlt1Lines.Hlt1MuonLines           import Hlt1MuonLinesConf
        from Hlt1Lines.Hlt1HadronLines         import Hlt1HadronLinesConf
        from Hlt1Lines.Hlt1ElectronLines       import Hlt1ElectronLinesConf
        from Hlt1Lines.Hlt1PhotonLines         import Hlt1PhotonLinesConf
        from Hlt1Lines.Hlt1VeloLines           import Hlt1VeloLinesConf
        from Hlt2Lines.Hlt2CommissioningLines  import Hlt2CommissioningLinesConf
        
        return { Hlt1HadronLinesConf : {  'SingleHadron_PTCut' : 4800
                                         , 'HadSingle_IPChi2Cut' : 25.0
                                         , 'HadMain_PTCut' : 250
                                         , 'HadCompanion_DZCut' : 2.0
                                         , 'HadDi_VDChi2Cut' : 50.0
                                         , 'HadCompanion_PTCut' : 750
                                         , 'HadCompanion_PointingCut' : 0.39
                                         , 'HadMain_TrackFitChi2Cut' :  10.
                                        } 
                 , Hlt1PhotonLinesConf : { 'Pho_EtCut'         : 2800.0
                                          , 'Pho_IsPho'        : -0.082 
                                          , 'Track_IPCut'      : 0.15
                                          , 'Track_PtCut'      : 1250.
                                          , 'DiTrack_PtCut'    : 650
                                          }
                 , Hlt1ElectronLinesConf : { 'EleIP_PtCut'             : 2820.0
                                            , 'EleIP_IPCut'            : 0.13 
                                            , 'EleIPCompanion_PtCut'   : 2750
                                            , 'EleCompanion_PtCut'     : 1000
                                            }
                 , Hlt1MuonLinesConf : {  'DiMuon_SumPtCut' : 120.0
                                          , 'Muon_PtCut' : 1000
                                          , 'MuonIP_PtCut' : 1000
                                          , 'Muon_IPMinCut' : 0.05
                                          , 'DiMuon_DOCACut' : 1.0
                                          , 'DiMuon_MassCut' : 2500
                                          , 'DiMuonIP_MassCut' : 300
                                          , 'DiMuon_IPCut' : 0.08
                                          #, 'Prescale' : { 'Hlt1MuTrack4JPsi' : 0. }
                                          , 'MuTrackMuPt' : 500
                                          , 'MuTrackMuIP' : 0.07
                                          , 'MuTrackTrPt' : 550
                                          , 'MuTrackTrIP' : 0.1
                                          , 'MuTrackDoca' : 0.2
                                          , 'MuTrackDZ' : 2.0
                                          , 'MuTrackDimuMass' : 1000
                                          , 'MuTrackPoint' : 0.4
                                          , 'MuTrackTrChi2' : 10
                                          }
                 , Hlt2CommissioningLinesConf : { 'Prescale' : { 'Hlt2PassThrough'  : 1.
                                                                 , 'Hlt2Forward'      : 0. # first needs a check that /Hlt/Track/Long actually exists..
                                                                 , 'Hlt2DebugEvent'   : 0.0001
                                                                 }              }
                 
                 }
    
    def ActiveHlt2Lines(self) :
        """
        Returns a list of active lines
        """
        return [ 'Hlt2PassThrough', 'Hlt2Transparent','Hlt2DebugEvent']
    
    
    def ActiveHlt1Lines(self) :
        """
        Returns a list of active lines
        """
        return []
    
    
