from Gaudi.Configuration import *
from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser

class Hlt2CharmHadD2HHHLinesConf(HltLinesConfigurableUser) :
    __slots__ = {
                 ## 3Body
                    'TrkPt_3Body'                 : 250.0    # in MeV
                  , 'TrkP_3Body'                  : 2000.0   # in MeV
                  , 'TrkPVIPChi2_3Body'           : 2.0      # unitless
                  , 'TrkChi2_3Body'               : 5.0      # unitless
                  , 'PairMinDoca_3Body'      : 0.10     # in mm
                  , 'VtxPVDispChi2_3Body'         : 100.0    # unitless
                  , 'VtxChi2_3Body'               : 20.0     # unitless
                  , 'DIPChi2_3Body'               : 25.0     # unitless
                  , 'DSumPt_3Body'                : 2000.0   # sum pT   
                  , 'MCOR_MAX_3Body'                : 3500.    # MeV
                  ## 2-body Input for 3Body
                  , 'TrkPt_2BodyFor3Body'         : 500.0    # in MeV
                  , 'TrkP_2BodyFor3Body'          : 5000.0   # in MeV
                  , 'TrkPVIPChi2_2BodyFor3Body'   : 7.0      # unitless
                  , 'TrkChi2_2BodyFor3Body'       : 5.0      # unitless
                  , 'Doca_2BodyFor3Body'          : 0.085     # in mm
                  , 'VtxPVDispChi2_2BodyFor3Body' : 40.      # unitless  
                  , 'VtxPVDisp_2BodyFor3Body'     : 3.0      # in mm
                  , 'DSumPt_2BodyFor3Body'        : 1400.0   # in MeV
                  , 'MCOR_MAX_2BodyFor3Body'        : 3500.    # MeV
                  # prescales
                  , 'Prescale'                  : {
                        'Hlt2CharmHadD2HHHWideMass'    : 0.1
                        }
                  , 'HltANNSvcID'  : {
                          'Hlt2CharmHad2BodyForD2HHHDecision' : 50910
                        , 'Hlt2CharmHadD2HHHDecision'         : 50911
                        , 'Hlt2CharmHadD2HHHWideMassDecision' : 50912
                        }
                }

    def __InPartFilterLowIP(self, name, inputContainers) : 
        from HltLine.HltLine import Hlt2Member, bindMembers
        from Configurables import FilterDesktop, CombineParticles
        from HltTracking.HltPVs import PV3D

        incuts = """ (TRCHI2DOF< %(TrkChi2_2BodyFor3Body)s )
                    & (MIPCHI2DV(PRIMARY)> %(TrkPVIPChi2_3Body)s )""" % self.getProps()

        filter = Hlt2Member( FilterDesktop
                            , 'Filter'
                            , Inputs = inputContainers
                            , Code = incuts
                           )

        ## Require the PV3D reconstruction before our cut on IP.
        filterSeq = bindMembers( name, [ PV3D()] + inputContainers + [filter ] )

        return filterSeq

    def __InPartFilter2BodyFor3Body(self, name, inputContainers) :  
        from HltLine.HltLine import Hlt2Member, bindMembers
        from Configurables import FilterDesktop, CombineParticles
        from HltTracking.HltPVs import PV3D

        incuts = "(PT> %(TrkPt_2BodyFor3Body)s *MeV)" \
                 "& (P> %(TrkP_2BodyFor3Body)s *MeV)" \
                 "& (MIPCHI2DV(PRIMARY)> %(TrkPVIPChi2_2BodyFor3Body)s )" % self.getProps()


        filter = Hlt2Member( FilterDesktop
                            , 'Filter'
                            , Inputs = inputContainers
                            , Code = incuts
                           )
        ## Require the PV3D reconstruction before our cut on IP.
        filterSeq = bindMembers( name, [ PV3D()] + inputContainers + [filter ] )

        return filterSeq

    def __InPartFilter3Body(self, name, inputContainers) :  
        from HltLine.HltLine import Hlt2Member, bindMembers
        from Configurables import FilterDesktop, CombineParticles
        from HltTracking.HltPVs import PV3D

        incuts = "(TRCHI2DOF< %(TrkChi2_3Body)s )" \
                 "& (PT> %(TrkPt_3Body)s *MeV)" \
                 "& (P> %(TrkP_3Body)s *MeV)" \
                 "& (MIPCHI2DV(PRIMARY)> %(TrkPVIPChi2_3Body)s )" % self.getProps()


        filter = Hlt2Member( FilterDesktop
                            , 'Filter'
                            , Inputs = inputContainers
                            , Code = incuts
                           )

        ## Require the PV3D reconstruction before our cut on IP.
        filterSeq = bindMembers( name, [ PV3D()] + inputContainers + [filter ] )

        return filterSeq

    def __3BodyCombine(self, name, inputSeq, decayDesc) :
        from HltLine.HltLine import Hlt2Member, bindMembers
        from Configurables import FilterDesktop, CombineParticles
        from HltTracking.HltPVs import PV3D

        combcuts = "(AM<2100*MeV)" \
                   "& ((APT1+APT2+APT3) > %(DSumPt_3Body)s) " \
                   "& (AMINDOCA('LoKi::TrgDistanceCalculator') < %(PairMinDoca_3Body)s)" \
                   "& (AALLSAMEBPV)" % self.getProps()
        mothercuts = "(VFASPF(VCHI2PDOF) < %(VtxChi2_3Body)s)" \
                     "& (abs(CHILD(1,SUMQ) + CHILD(2,Q))==1)" \
                     "& (BPVVDCHI2> %(VtxPVDispChi2_3Body)s )" \
                     "& (BPVCORRM < %(MCOR_MAX_3Body)s*MeV)" \
                     "& (BPVIPCHI2() < %(DIPChi2_3Body)s)" % self.getProps()
        combineCharm3Body = Hlt2Member( CombineParticles
                          , "Combine_Stage2"
                          , DecayDescriptors = decayDesc
                          , Inputs = inputSeq 
                          , CombinationCut = combcuts
                          , MotherCut = mothercuts
                          )
        return bindMembers(name, [PV3D()] + inputSeq + [combineCharm3Body])

    def __3BodyFilter(self, name, inputSeq, extracode = None) :

        from HltLine.HltLine import Hlt2Member, bindMembers
        from Configurables import FilterDesktop, CombineParticles

        if extracode :
            filter = Hlt2Member( FilterDesktop
                                 , 'Filter'
                                 , Inputs = inputSeq
                                 , Code = extracode
                                 )
        else :
            filter = Hlt2Member( FilterDesktop
                                 , 'Filter'
                                 , Inputs = inputSeq
                                 , Code = 'ALL'
                                 )

        return bindMembers(name, inputSeq + [filter])
 
    def __apply_configuration__(self) :
        from HltLine.HltLine import Hlt2Line
        from HltLine.HltLine import Hlt2Member, bindMembers
        from Configurables import HltANNSvc
        from Configurables import CombineParticles
        from Configurables import FilterDesktop
        from Hlt2SharedParticles.TrackFittedBasicParticles import BiKalmanFittedPions,BiKalmanFittedSecondLoopPions, BiKalmanFittedKaons, BiKalmanFittedSecondLoopKaons
        from HltTracking.HltPVs import PV3D
        
        # Filter pions and kaons with LowIP Cut
        pionsLowIP = self.__InPartFilterLowIP('CharmInputPionsLowIP', [ BiKalmanFittedPions] )
        kaonsLowIP = self.__InPartFilterLowIP('CharmInputKaonsLowIP', [ BiKalmanFittedKaons] )

         
        # Tighten IP cut
        pions2BodyFor3Body = self.__InPartFilter2BodyFor3Body('CharmInputPions2BodyFor3Body', [ pionsLowIP ] )
        kaons2BodyFor3Body = self.__InPartFilter2BodyFor3Body('CharmInputKaons2BodyFor3Body', [ kaonsLowIP ] )
 
        twoBodyCombCut = "(AM<2100*MeV)" \
                         "& ((APT1+APT2)> %(DSumPt_2BodyFor3Body)s)" \
                         "& (AMINDOCA('LoKi::TrgDistanceCalculator') < %(Doca_2BodyFor3Body)s )" \
                         "& (AALLSAMEBPV)" % self.getProps()
        twoBodyMotherCut = "(BPVVD> %(VtxPVDisp_2BodyFor3Body)s )" \
                           "& (BPVCORRM < %(MCOR_MAX_2BodyFor3Body)s*MeV)" \
                           "& (BPVVDCHI2> %(VtxPVDispChi2_2BodyFor3Body)s )" % self.getProps()
        #First stage - Combine 2 Body with pt > 500MeV        
        Charm2BodyCombine = Hlt2Member( CombineParticles
                          , "Combine_Stage1"
                          , DecayDescriptors = ["K*(892)0 -> pi+ pi+" , "K*(892)0 -> pi+ pi-"
                          , "K*(892)0 -> pi- pi-" , "K*(892)0 -> K+ K+"
                          , "K*(892)0 -> K+ K-"   , "K*(892)0 -> K- K-"
                          , "K*(892)0 -> K+ pi-"  , "K*(892)0 -> pi+ K-"
                          , "K*(892)0 -> K+ pi+"  , "K*(892)0 -> K- pi-" ]
                          , CombinationCut = twoBodyCombCut 
                          , MotherCut = twoBodyMotherCut
                          , Inputs = [ pions2BodyFor3Body , kaons2BodyFor3Body ])

        Hlt2Charm2BodyFor3Body =  bindMembers('CharmHadD2HHH', [ pions2BodyFor3Body , kaons2BodyFor3Body, Charm2BodyCombine ])
                          
        #Second Stage - picks up a low pt track too

        # Filter low PT pions and kaons 

        pionsFor3Body = self.__InPartFilter3Body('CharmInputPions3Body', [ BiKalmanFittedSecondLoopPions] )
        kaonsFor3Body = self.__InPartFilter3Body('CharmInputKaons3Body', [ BiKalmanFittedSecondLoopKaons] )
 
        # Make 3Body 
        Charm3BodyCombine = self.__3BodyCombine (  name = 'CharmHadD2HHH'
                                                  , inputSeq = [ Hlt2Charm2BodyFor3Body, pionsFor3Body, kaonsFor3Body, kaonsLowIP, pionsLowIP ]
                                                  , decayDesc = ["D+ -> K*(892)0 pi+", "D+ -> K*(892)0 pi-"
                                                                ,"D+ -> K*(892)0 K+",  "D+ -> K*(892)0 K-" ] 
                                                 )   
        # 3Body line
        Hlt2Charm3Body = self.__3BodyFilter ( name = 'CharmHadD2HHH', inputSeq = [Charm3BodyCombine], extracode = "in_range(1800*MeV, M, 2040*MeV)")
        # 3Body WideMass line - with prescale
        Hlt2Charm3BodyWideMass = self.__3BodyFilter (name = 'CharmHadD2HHHWideMass', inputSeq = [Charm3BodyCombine], extracode = "in_range(1700*MeV, M, 2100*MeV)")

        ###########################################################################
        # Define the Hlt2 Lines
        #
        # Note: for the 2-loop approach you just need to explicitly add the second loop pions inbetween the two stages above 
        ##########################################################################
        line = Hlt2Line('CharmHad2BodyForD2HHH', prescale = self.prescale
                        , algos = [ PV3D(), pions2BodyFor3Body , kaons2BodyFor3Body, Hlt2Charm2BodyFor3Body]
                        , postscale = self.postscale
                        )
        decName = "Hlt2CharmHad2BodyForD2HHHDecision"
        annSvcID = self._scale(decName,'HltANNSvcID')
        HltANNSvc().Hlt2SelectionID.update( { decName : annSvcID } )

        line = Hlt2Line('CharmHadD2HHH', prescale = self.prescale
                        , algos = [ PV3D(), pions2BodyFor3Body , kaons2BodyFor3Body, Hlt2Charm2BodyFor3Body, pionsFor3Body, kaonsFor3Body, kaonsLowIP, pionsLowIP, Hlt2Charm3Body]
                        , postscale = self.postscale
                        )
        decName = "Hlt2CharmHadD2HHHDecision"
        annSvcID = self._scale(decName,'HltANNSvcID')
        HltANNSvc().Hlt2SelectionID.update( { decName : annSvcID } )

        line = Hlt2Line('CharmHadD2HHHWideMass', prescale = self.prescale
                        , algos = [ PV3D(), pions2BodyFor3Body , kaons2BodyFor3Body, Hlt2Charm2BodyFor3Body, pionsFor3Body, kaonsFor3Body, kaonsLowIP, pionsLowIP, Hlt2Charm3BodyWideMass]
                        , postscale = self.postscale
                        )
        decName = "Hlt2CharmHadD2HHHWideMassDecision"
        annSvcID = self._scale(decName,'HltANNSvcID')
        HltANNSvc().Hlt2SelectionID.update( { decName : annSvcID } )
