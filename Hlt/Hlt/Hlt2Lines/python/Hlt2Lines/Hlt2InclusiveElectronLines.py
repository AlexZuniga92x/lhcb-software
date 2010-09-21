from Gaudi.Configuration import *
from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser

class Hlt2InclusiveElectronLinesConf(HltLinesConfigurableUser) :
    
    __slots__ = { 'Prescale'             : { 'Hlt2SingleHighPTElectron'            : 1.0
                                            ,'Hlt2DYeh1'                           : 1.0
                                            ,'Hlt2DYeh2'                           : 1.0
                                            ,'Hlt2SingleHighPTElectronUnfitted'    : 1.0
                                            ,'Hlt2DYeh1Unfitted'                   : 1.0
                                            ,'Hlt2DYeh2Unfitted'                   : 1.0

                                            ,'Hlt2SingleElectron'         :  0.01
                                            ,'Hlt2ElectronPlusTrack'      :  0.01
                                            ,'Hlt2SingleTFElectron'       :  1.0
                                            ,'Hlt2TFElectronPlusTrack'    :  1.0
                                             }
                                             
                 
                  ,'SingleHighPTElectronPT'        : 20000      # MeV
                  ,'SingleHighPTElectronIP'        :   0.1      # mm
                  ,'SingleHighPTElectronTkChi2'    :    20     
                  
                  ,'DYeh1ePt'                 : 4000      # MeV
                  ,'DYeh1trPt'                : 4000      # MeV                  
                  ,'DYeh1MinMass'             :10000      # MeV                  
                  ,'DYeh2ePt'                 : 8000      # MeV
                  ,'DYeh2trPt'                : 8000      # MeV                  
                  ,'DYeh2MinMass'             :20000      # MeV                  
                  ,'DYehDphiMin'             :     0      # radian                 
                  ,'DYehVertexChi2'          :    20      #

                  
                  ,'SingleElectron_L0Req'   :  "L0_CHANNEL('Electron')"
                  ,'SingleElectron_Hlt1Req' :  "HLT_PASS_RE('Hlt1(Track|.*Electron).*Decision')"   
                  ,'SingleElectron_PT'      :  3000.    # MeV
                  ,'SingleElectron_IP'      :     0.2   # mm
                  ,'SingleElectron_PIDe'    :     4.  
                  ,'SingleElectron_IPCHI2'  :    -1.    

                  ,'ElectronPlusTrack_L0Req'        :  "L0_CHANNEL('Electron')"
                  ,'ElectronPlusTrack_Hlt1Req'      :  "HLT_PASS_RE('Hlt1(Track|.*Electron).*Decision')"          
                  ,'ElectronPlusTrack_ElecPT'       : 1500.   # MeV
                  ,'ElectronPlusTrack_ElecIP'       :    0.2  # mm
                  ,'ElectronPlusTrack_ElecPIDe'     :    4. 
                  ,'ElectronPlusTrack_ElecIPCHI2'   :   -1.
                  ,'ElectronPlusTrack_TrackPT'      : 1500.   # MeV
                  ,'ElectronPlusTrack_TrackIP'      :    0.2  # mm
                  ,'ElectronPlusTrack_TrackIPCHI2'  :   -1.
                  ,'ElectronPlusTrack_VtxCHI2'      :   25.

                  
                  ,'SingleTFElectron_L0Req'   :  "L0_CHANNEL('Electron')"
                  ,'SingleTFElectron_Hlt1Req' :  "HLT_PASS_RE('Hlt1(Track|.*Electron).*Decision')"   
                  ,'SingleTFElectron_PT'      :  3000.    # MeV
                  ,'SingleTFElectron_IP'      :     0.2   # mm
                  ,'SingleTFElectron_PIDe'    :     4.  
                  ,'SingleTFElectron_IPCHI2'  :    -1.    

                  ,'TFElectronPlusTrack_L0Req'        :  "L0_CHANNEL('Electron')"
                  ,'TFElectronPlusTrack_Hlt1Req'      :  "HLT_PASS_RE('Hlt1(Track|.*Electron).*Decision')"                     
                  ,'TFElectronPlusTrack_ElecPT'       : 1500.   # MeV
                  ,'TFElectronPlusTrack_ElecIP'       :    0.2  # mm
                  ,'TFElectronPlusTrack_ElecIPCHI2'   :   -1.
                  ,'TFElectronPlusTrack_ElecPIDe'     :    4.
                  ,'TFElectronPlusTrack_TrackPT'      : 1500.   # MeV
                  ,'TFElectronPlusTrack_TrackIP'      :    0.2  # mm
                  ,'TFElectronPlusTrack_TrackIPCHI2'  :   -1.
                  ,'TFElectronPlusTrack_VtxCHI2'      :   25.
                  
                  ,'HltANNSvcID'  : {
                                      'SingleElectron'         :  51300
                                     ,'ElectronPlusTrack'      :  51310
                                     ,'SingleTFElectron'       :  51350
                                     ,'TFElectronPlusTrack'    :  51360
                                     
                                      }
                  }
    
    
    def __apply_configuration__(self) :
        self.__makeHlt2SingleElectronForDYLines()
        self.__makeHlt2SingleElectronLines()
        self.__makeHlt2ElectronPlusTrackLines()
        self.__makeHlt2SingleTFElectronLines()
        self.__makeHlt2TFElectronPlusTrackLines()       

        
    def __makeHlt2SingleElectronForDYLines(self):
        # Hlt2 Inclusive single Electron and Electron+track selections 
        #
        #  @author D.R.Ward
        #  @date 2010-04-20
        #
        
        from HltLine.HltLine import Hlt2Line
        from HltLine.HltLine import Hlt2Member
        from HltLine.HltLine import bindMembers
        from HltTracking.HltPVs import PV3D
        from Configurables import HltANNSvc
        from Configurables import CombineParticles, PhysDesktop
        from Configurables import FilterDesktop
        from Hlt2SharedParticles.BasicParticles import Electrons, NoCutsPions
        from Hlt2SharedParticles.TrackFittedBasicParticles import BiKalmanFittedElectrons

        #some string definitions... 


#        TKQuality =  " & (MIPDV(PRIMARY)<"+str(self.getProp('SingleHighPTElectronIP'))+"*mm) & (TRCHI2DOF<"+str(self.getProp('SingleHighPTElectronTkChi2'))+")"                                
        TKQuality =  " & (TRCHI2DOF<"+str(self.getProp('SingleHighPTElectronTkChi2'))+")"                                

        ############################################################################
        #    Selection for a large PT single electron:  
        ############################################################################
        
        Hlt2SelSingleHighPTElectron = Hlt2Member(   FilterDesktop
                                                    , "Filter"
                                                    , Code = " (PT>"+str(self.getProp('SingleHighPTElectronPT'))+"*MeV) " + TKQuality
                                                    , InputLocations  = [BiKalmanFittedElectrons]
                                                    )
        line = Hlt2Line( 'SingleHighPTElectron'
                         , prescale = self.prescale
                         , L0DU = "L0_CHANNEL('Electron')"
                         , HLT = "HLT_PASS_RE('Hlt1(Track|.*Electron).*Decision')"
                         , algos = [ BiKalmanFittedElectrons, Hlt2SelSingleHighPTElectron]
                         , postscale = self.postscale
                         )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2SingleHighPTElectronDecision" : 50460 } )
        
        
        ############################################################################
        #    Selection for inclusive decays with high mass Drell-Yan ->e+track
        ############################################################################

        combine_DY1 = Hlt2Member( CombineParticles
                                  , "Combine"
                                  , DecayDescriptors = ["[Z0 -> e+ pi-]cc"] 
                                  , DaughtersCuts = { "e+" : "(PT>"+str(self.getProp('DYeh1ePt'))+"*MeV)" + TKQuality ,     
                                                      "pi-" : "(PT>"+str(self.getProp('DYeh1trPt'))+"*MeV)" + TKQuality }
                                  , CombinationCut = "(AM >"+str(self.getProp('DYeh1MinMass'))+"*MeV)"\
                                  " & (abs(ACHILD(PHI,1)-ACHILD(PHI,2))>"+str(self.getProp('DYehDphiMin'))+")"
                                  , MotherCut = "(VFASPF(VCHI2PDOF)<"+str(self.getProp('DYehVertexChi2'))+")"
                                  , InputLocations  = [ BiKalmanFittedElectrons , NoCutsPions ]
                                  )
        line1 = Hlt2Line( 'DYeh1'
                          , prescale = self.prescale
                          , L0DU = "L0_CHANNEL('Electron')"
                          , HLT = "HLT_PASS_RE('Hlt1(Track|.*Electron).*Decision')"
                          , algos = [BiKalmanFittedElectrons, NoCutsPions, combine_DY1]
                          , postscale = self.postscale
                          )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DYeh1Decision" : 50461 } )

        combine_DY2 = Hlt2Member( CombineParticles
                                  , "Combine"
                                  , DecayDescriptors = ["[Z0 -> e+ pi-]cc"] 
                                  , DaughtersCuts = { "e+" : "(PT>"+str(self.getProp('DYeh2ePt'))+"*MeV)" + TKQuality ,     
                                                      "pi-" : "(PT>"+str(self.getProp('DYeh2trPt'))+"*MeV)" + TKQuality }
                                  , CombinationCut = "(AM>"+str(self.getProp('DYeh2MinMass'))+"*MeV)"\
                                  " & (abs(ACHILD(PHI,1)-ACHILD(PHI,2))>"+str(self.getProp('DYehDphiMin'))+")"
                                  , MotherCut = "(VFASPF(VCHI2PDOF)<"+str(self.getProp('DYehVertexChi2'))+")"
                                  , InputLocations  = [ BiKalmanFittedElectrons , NoCutsPions ]
                                  )
        line2 = Hlt2Line( 'DYeh2'
                          , prescale = self.prescale
                          , L0DU = "L0_CHANNEL('Electron')"
                          , HLT = "HLT_PASS_RE('Hlt1(Track|.*Electron).*Decision')"
                          , algos = [BiKalmanFittedElectrons, NoCutsPions, combine_DY2]
                          , postscale = self.postscale
                          )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DYeh2Decision" : 50462 } )

 
        ############################################################################
        #    Now similar lines for unfitted electron tracks
        ############################################################################
        
        ############################################################################
        #    Selection for a large PT single electron:  
        ############################################################################
        
        Hlt2SelSingleHighPTElectronU = Hlt2Member(   FilterDesktop
                                                     , "Filter"
                                                     , Code = " (PT>"+str(self.getProp('SingleHighPTElectronPT'))+"*MeV) " + TKQuality
                                                     , InputLocations  = [Electrons]
                                                     )
        lineU = Hlt2Line( 'SingleHighPTElectronUnfitted'
                         , prescale = self.prescale 
                         , algos = [ Electrons, Hlt2SelSingleHighPTElectronU]
                         , postscale = self.postscale
                         )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2SingleHighPTElectronUnfittedDecision" : 50450 } )
        
        
        ############################################################################
        #    Selections for inclusive decays with high mass Drell-Yan ->e+track
        ############################################################################

        combine_DY1U = Hlt2Member( CombineParticles
                                  , "Combine"
                                  , DecayDescriptors = ["[Z0 -> e+ pi-]cc"] 
                                  , DaughtersCuts = { "e+" : "(PT>"+str(self.getProp('DYeh1ePt'))+"*MeV)" + TKQuality ,     
                                                      "pi-" : "(PT>"+str(self.getProp('DYeh1trPt'))+"*MeV)" + TKQuality }
                                  , CombinationCut = "(AM >"+str(self.getProp('DYeh1MinMass'))+"*MeV)"\
                                  " & (abs(ACHILD(PHI,1)-ACHILD(PHI,2))>"+str(self.getProp('DYehDphiMin'))+")"
                                  , MotherCut = "(VFASPF(VCHI2PDOF)<"+str(self.getProp('DYehVertexChi2'))+")"
                                  , InputLocations  = [ Electrons , NoCutsPions ]
                                  )
        line1U = Hlt2Line( 'DYeh1Unfitted'
                         , prescale = self.prescale 
                         , algos = [Electrons, NoCutsPions, combine_DY1U]
                         , postscale = self.postscale
                         )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DYeh1UnfittedDecision" : 50451 } )

        combine_DY2U = Hlt2Member( CombineParticles
                                  , "Combine"
                                  , DecayDescriptors = ["[Z0 -> e+ pi-]cc"] 
                                  , DaughtersCuts = { "e+" : "(PT>"+str(self.getProp('DYeh2ePt'))+"*MeV)" + TKQuality ,     
                                                      "pi-" : "(PT>"+str(self.getProp('DYeh2trPt'))+"*MeV)" + TKQuality }
                                  , CombinationCut = "(AM>"+str(self.getProp('DYeh2MinMass'))+"*MeV)"\
                                  " & (abs(ACHILD(PHI,1)-ACHILD(PHI,2))>"+str(self.getProp('DYehDphiMin'))+")"
                                  , MotherCut = "(VFASPF(VCHI2PDOF)<"+str(self.getProp('DYehVertexChi2'))+")"
                                  , InputLocations  = [ Electrons , NoCutsPions ]
                                  )
        line2U = Hlt2Line( 'DYeh2Unfitted'
                         , prescale = self.prescale 
                         , algos = [Electrons, NoCutsPions, combine_DY2U]
                         , postscale = self.postscale
                         )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DYeh2UnfittedDecision" : 50452 } )

 

    def __makeHlt2SingleElectronLines(self):

        #
        # Single Electron line with PT, IP, PIDe cuts using unfitted track
        #
        # Author: Jibo.He@cern.ch
        # 12 May 2010
        # 
        from HltLine.HltLine import Hlt2Line, Hlt2Member
        from Configurables import HltANNSvc
        from Configurables import FilterDesktop

        from Hlt2SharedParticles.BasicParticles import Electrons
        from HltTracking.HltPVs import PV3D

        HltANNSvc().Hlt2SelectionID.update( { "Hlt2SingleElectronDecision" :
                                              self.getProp('HltANNSvcID')['SingleElectron'] } )

        """
        #------------------------
        # L0 & Hlt1 Requirements
        #------------------------ 
        """
        L0Req   = self.getProp("SingleElectron_L0Req")
        Hlt1Req = self.getProp("SingleElectron_Hlt1Req")
        
        if not L0Req:
            L0Req = None

        if not Hlt1Req:
            Hlt1Req= None



        FilterSingleElectron = Hlt2Member( FilterDesktop # type
                                           , "FilterSingleElectron" 
                                           , Code ="( PT > %(SingleElectron_PT)s *MeV)" \
                                           " & ( MIPDV(PRIMARY) > %(SingleElectron_IP)s *mm)" \
                                           " & ( PIDe > %(SingleElectron_PIDe)s )" \
                                           " & ( MIPCHI2DV(PRIMARY) > %(SingleElectron_IPCHI2)s )" %self.getProps() 
                                           , InputLocations = [ Electrons ]
                                           )

        SingleElectronLine = Hlt2Line("SingleElectron"
                                      , prescale = self.prescale
                                      , L0DU = L0Req
                                      , HLT  = Hlt1Req
                                      , algos = [ PV3D()
                                                  , Electrons
                                                  , FilterSingleElectron ]
                                      , postscale = self.postscale
                                      )


    def __makeHlt2ElectronPlusTrackLines(self):

        #
        # Electron + Track line with PT, IP, PIDe cuts using unfitted track
        #
        # Author: Jibo.He@cern.ch
        # 12 May 2010
        #
        
        from HltLine.HltLine import Hlt2Line, Hlt2Member
        from Configurables import HltANNSvc
        from Configurables import CombineParticles

        from Hlt2SharedParticles.BasicParticles import Electrons, NoCutsKaons
        from HltTracking.HltPVs import PV3D

        HltANNSvc().Hlt2SelectionID.update( { "Hlt2ElectronPlusTrackDecision" :
                                              self.getProp('HltANNSvcID')['ElectronPlusTrack'] } )

        """
        #------------------------
        # L0 & Hlt1 Requirements
        #------------------------ 
        """
        L0Req   = self.getProp("ElectronPlusTrack_L0Req")
        Hlt1Req = self.getProp("ElectronPlusTrack_Hlt1Req")
        
        if not L0Req:
            L0Req = None
            
        if not Hlt1Req:
            Hlt1Req= None 
        
        
        ElectronCut = "(PT > %(ElectronPlusTrack_ElecPT)s *MeV) & (PIDe > %(ElectronPlusTrack_ElecPIDe)s ) & (MIPDV(PRIMARY) > %(ElectronPlusTrack_ElecIP)s *mm) & (MIPCHI2DV(PRIMARY) > %(ElectronPlusTrack_ElecIPCHI2)s )" %self.getProps()
        
        TrackCut = "(PT > %(ElectronPlusTrack_TrackPT)s *MeV) & (MIPDV(PRIMARY) > %(ElectronPlusTrack_TrackIP)s *mm) & (MIPCHI2DV(PRIMARY) > %(ElectronPlusTrack_TrackIPCHI2)s )" %self.getProps()

        CombCut = "AALL"
        
        MomCut = "(VFASPF(VCHI2PDOF)< %(ElectronPlusTrack_VtxCHI2)s )" %self.getProps()
        
        CombineElectronTrack = Hlt2Member( CombineParticles # type
                                           , "CombineElectronTrack"
                                           , DecayDescriptors = [ "[ J/psi(1S) -> e+ K- ]cc",
                                                                  "[ J/psi(1S) -> e+ K+ ]cc" ]  
                                           , DaughtersCuts = { "e+" : ElectronCut  
                                                               ,"K-": TrackCut  }
                                           , CombinationCut = CombCut
                                           , MotherCut = MomCut
                                           , InputLocations = [ Electrons, NoCutsKaons ]
                                           )

        ElectronPlusTrackLine = Hlt2Line("ElectronPlusTrack"
                                         , prescale = self.prescale
                                         , L0DU = L0Req
                                         , HLT  = Hlt1Req
                                         , algos = [ PV3D()
                                                     , Electrons
                                                     , NoCutsKaons
                                                     , CombineElectronTrack ]
                                         , postscale = self.postscale
                                         )
        

    def __makeHlt2SingleTFElectronLines(self):

        #
        # Single Electron line with PT, IP, PIDe cuts using fitted track
        #
        # Author: Jibo.He@cern.ch
        # 12 May 2010
        #
        
        from HltLine.HltLine import Hlt2Line, Hlt2Member
        from Configurables import HltANNSvc
        from Configurables import FilterDesktop

        from Hlt2SharedParticles.TrackFittedBasicParticles import BiKalmanFittedElectrons
        from HltTracking.HltPVs import PV3D
        
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2SingleTFElectronDecision" :
                                              self.getProp('HltANNSvcID')['SingleTFElectron'] } )

        """
        #------------------------
        # L0 & Hlt1 Requirements
        #------------------------ 
        """
        L0Req   = self.getProp("SingleTFElectron_L0Req")
        Hlt1Req = self.getProp("SingleTFElectron_Hlt1Req")
        
        if not L0Req:
            L0Req = None

        if not Hlt1Req:
            Hlt1Req= None 
        
        
        FilterSingleTFElectron = Hlt2Member( FilterDesktop # type
                                             , "FilterSingleTFElectron" 
                                             , Code ="(PT > %(SingleTFElectron_PT)s *MeV)" \
                                             " & (PIDe > %(SingleTFElectron_PIDe)s )" \
                                             " & (MIPDV(PRIMARY) > %(SingleTFElectron_IP)s *mm)" \
                                             " & (MIPCHI2DV(PRIMARY) > %(SingleTFElectron_IPCHI2)s )" %self.getProps() 
                                             , InputLocations = [ BiKalmanFittedElectrons ]
                                             )

        SingleTFElectronLine = Hlt2Line("SingleTFElectron"
                                        , prescale = self.prescale
                                        , L0DU = L0Req
                                        , HLT = Hlt1Req
                                        , algos = [ PV3D()
                                                    , BiKalmanFittedElectrons
                                                    , FilterSingleTFElectron ]
                                        , postscale = self.postscale
                                        )
        

        
    def __makeHlt2TFElectronPlusTrackLines(self):

        #
        # Electron + Track line with PT, IP, PIDe cuts using fitted track
        #
        # Author: Jibo.He@cern.ch
        # 12 May 2010
        #
        from HltLine.HltLine import Hlt2Line, Hlt2Member
        from Configurables import HltANNSvc
        from Configurables import CombineParticles

        from Hlt2SharedParticles.TrackFittedBasicParticles import BiKalmanFittedElectrons, BiKalmanFittedKaons
        from HltTracking.HltPVs import PV3D

        HltANNSvc().Hlt2SelectionID.update( { "Hlt2TFElectronPlusTrackDecision" :
                                              self.getProp('HltANNSvcID')['TFElectronPlusTrack'] } )

        """
        #------------------------
        # L0 & Hlt1 Requirements
        #------------------------ 
        """
        L0Req   = self.getProp("TFElectronPlusTrack_L0Req")
        Hlt1Req = self.getProp("TFElectronPlusTrack_Hlt1Req")
        
        if not L0Req:
            L0Req = None
            
        if not Hlt1Req:
            Hlt1Req= None 
        

        ElectronCut = "(PT > %(TFElectronPlusTrack_ElecPT)s *MeV) & (PIDe > %(TFElectronPlusTrack_ElecPIDe)s ) & (MIPDV(PRIMARY) > %(TFElectronPlusTrack_ElecIP)s *mm) & (MIPCHI2DV(PRIMARY) > %(TFElectronPlusTrack_ElecIPCHI2)s )" %self.getProps()
        
        TrackCut = "(PT > %(TFElectronPlusTrack_TrackPT)s *MeV) & (MIPDV(PRIMARY) > %(TFElectronPlusTrack_TrackIP)s *mm) & (MIPCHI2DV(PRIMARY) > %(TFElectronPlusTrack_TrackIPCHI2)s )" %self.getProps()
        
        CombCut = "AALL"
        
        MomCut = "(VFASPF(VCHI2PDOF)< %(TFElectronPlusTrack_VtxCHI2)s )" %self.getProps()
        
        CombineTFElectronTrack = Hlt2Member( CombineParticles # type
                                             , "CombineTFElectronTrack"
                                             , DecayDescriptors = [ "[ J/psi(1S) -> e+ K- ]cc",
                                                                    "[ J/psi(1S) -> e+ K+ ]cc" ]
                                             , DaughtersCuts = { "e+" : ElectronCut  
                                                                 ,"K-": TrackCut  }
                                             , CombinationCut = CombCut
                                             , MotherCut = MomCut
                                             , InputLocations = [ BiKalmanFittedElectrons, BiKalmanFittedKaons ]
                                             )
        
        TFElectronPlusTrackLine = Hlt2Line("TFElectronPlusTrack"
                                           , prescale = self.prescale
                                           , L0DU = L0Req
                                           , HLT  = Hlt1Req 
                                           , algos = [ PV3D()
                                                       , BiKalmanFittedElectrons
                                                       , BiKalmanFittedKaons
                                                       , CombineTFElectronTrack ]
                                           , postscale = self.postscale
                                           )        
        
