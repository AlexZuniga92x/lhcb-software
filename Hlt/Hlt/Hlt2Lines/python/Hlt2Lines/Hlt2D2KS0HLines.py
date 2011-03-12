__author__  = [ 'Sam Gregson' ]


from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser
from GaudiKernel.SystemOfUnits import MeV

## Create a python class that inherits from HltLinesConfigurableUser
class Hlt2CharmHadD2KS0HLinesConf(HltLinesConfigurableUser) :

   # define some values so that they are not hard-coded
    __slots__ = {
                  ## KS0 daughter pion cuts
                    'KS0DaugTrackChi2' : 5.0
                  , 'KS0DaugMIPChi2'   : 90.0
                  ## KS0 mother cuts
                  , 'KS0VertexChi2'    : 15.0
                  , 'KS0PT'            : 800.0 * MeV

                  ## Bachelor pion cuts
                  , 'BachPionP'         : 4500.0 * MeV
                  , 'BachPionPT'        : 450.0 * MeV
                  , 'BachPionTrackChi2' : 5.0
                  , 'BachPionMIPChi2'   : 30.0

                  ## Bachelor kaon cuts
                  , 'BachKaonP'         : 4500.0 * MeV
                  , 'BachKaonPT'        : 450. * MeV
                  , 'BachKaonTrackChi2' : 5.0
                  , 'BachKaonMIPChi2'   : 30.0

                  ## D meson cuts
                  ## Combo cuts
                  , 'DMesonComboLowMass'   : 1760.0 * MeV
                  , 'DMesonComboHighMass'  : 2080.0 * MeV
                  ## Mother cuts
                  , 'DMesonMotherLowMass'    : 1770.0 * MeV
                  , 'DMesonMotherHighMass'   : 2070.0 * MeV
                  , 'DMesonMotherVertexChi2' : 15.0
                  , 'DMesonMotherMIPChi2'    : 25.0
                  , 'DMesonMotherPT'         : 1800.0 * MeV
                  , 'TisTosParticleTaggerSpecs': { "Hlt1Track.*Decision%TOS":0 }
                  , 'name_prefix'              : 'CharmHadD2KS0H'
                  ## prescales
                  , 'Prescale'                  : {
                        'Hlt2CharmHadD2KS0H_D2KS0Pi'    : 1.0
                       , 'Hlt2CharmHadD2KS0H_D2KS0K'    : 1.0 
                        }
                  , 'HltANNSvcID'  : {
                       'Hlt2CharmHadD2KS0H_D2KS0PiDecision' : 50913
                       ,'Hlt2CharmHadD2KS0H_D2KS0KDecision' : 50914
                        }
                }


    def __KS0LLFilter(self, name, inputContainers) : 
        """Use this section to create a KS0 LL filter
           - DD not possible due to seeding issues 
        """
        from HltLine.HltLine import Hlt2Member, bindMembers           
        from Configurables import FilterDesktop, CombineParticles      
        from HltTracking.HltPVs import PV3D

        incuts = "CHILDCUT((TRCHI2DOF < %(KS0DaugTrackChi2)s),1)" \
                 "& CHILDCUT((TRCHI2DOF < %(KS0DaugTrackChi2)s),2)" \
                 "& (VFASPF(VCHI2PDOF) < %(KS0VertexChi2)s)" \
                 "& (PT > %(KS0PT)s)" \
                 "& CHILDCUT((MIPCHI2DV(PRIMARY) > %(KS0DaugMIPChi2)s),1)" \
                 "& CHILDCUT((MIPCHI2DV(PRIMARY) > %(KS0DaugMIPChi2)s),2)" % self.getProps()


        # Define the HLT2 member with it's necessary inputs
        filter = Hlt2Member( FilterDesktop        
                            , 'Filter'
                            , Inputs = inputContainers
                            , Code = incuts
                           )

        ## Require the PV3D reconstruction before our cut on IP 
        filterSeq = bindMembers( name, [ PV3D()] + inputContainers + [filter ] )      
        return filterSeq


    def __BachelorPionFilter(self, name, inputContainers) : 
        """Use this section to create a bachelor filter
        """
        from HltLine.HltLine import Hlt2Member, bindMembers           
        from Configurables import FilterDesktop, CombineParticles      
        from HltTracking.HltPVs import PV3D                           

        Bachelor_Cuts_Pion = "(TRCHI2DOF < %(BachPionTrackChi2)s)" \
                             "& (PT > %(BachPionPT)s)" \
                             "& (P > %(BachPionP)s)" \
                             "& (MIPCHI2DV(PRIMARY) > %(BachPionMIPChi2)s)"  % self.getProps()


        # Define the HLT2 member with it's necessary inputs
        filter = Hlt2Member( FilterDesktop
                            , 'Filter'
                            , Inputs = inputContainers
                            , Code = Bachelor_Cuts_Pion
                           )

        ## Require the PV3D reconstruction before our cut on IP
        filterSeq = bindMembers( name, [ PV3D()] + inputContainers + [filter ] )      
        return filterSeq


    def __BachelorKaonFilter(self, name, inputContainers) : 
        from HltLine.HltLine import Hlt2Member, bindMembers           
        from Configurables import FilterDesktop, CombineParticles      
        from HltTracking.HltPVs import PV3D                           

        Bachelor_Cuts_Kaon = "(TRCHI2DOF < %(BachKaonTrackChi2)s)" \
                             "& (PT > %(BachKaonPT)s)" \
                             "& (P > %(BachKaonP)s)" \
                             "& (MIPCHI2DV(PRIMARY) > %(BachKaonMIPChi2)s)"  % self.getProps()


        # Define the HLT2 member with it's necessary inputs
        filter = Hlt2Member( FilterDesktop
                            , 'Filter'
                            , Inputs = inputContainers
                            , Code = Bachelor_Cuts_Kaon
                           )

        ## Require the PV3D reconstruction before our cut on IP
        filterSeq = bindMembers( name, [ PV3D()] + inputContainers + [filter ] )      
        return filterSeq
    
    

    def __D2KS0hCombine(self, name, inputSeq, decayDesc) :
        """Use this section to define the D->KS0h decays/combine particles
        """
        from HltLine.HltLine import Hlt2Member, bindMembers
        from Configurables import FilterDesktop, CombineParticles
        from HltTracking.HltPVs import PV3D

        combcuts = "in_range(%(DMesonComboLowMass)s, AM, %(DMesonComboHighMass)s)"  % self.getProps()


        mothercuts = "in_range(%(DMesonMotherLowMass)s, MM, %(DMesonMotherHighMass)s)" \
                     "& (VFASPF(VCHI2PDOF) < %(DMesonMotherVertexChi2)s)" \
                     "& (PT > %(DMesonMotherPT)s)" \
                     "& (MIPCHI2DV(PRIMARY) < %(DMesonMotherMIPChi2)s)"  % self.getProps()

        # Daughter cuts defined in bachelor pion/kaon filter      
        
        combineCharmD2KS0h= Hlt2Member( CombineParticles
                          , "Combine_Stage"
                          , DecayDescriptors = decayDesc
                          , Inputs = inputSeq 
                          , CombinationCut = combcuts
                          , MotherCut = mothercuts
                          )
        return bindMembers(name, [PV3D()] + inputSeq + [combineCharmD2KS0h])


    def __filterHlt1TOS(self, name, input) : # {
        from HltLine.HltLine import bindMembers
        from Configurables import TisTosParticleTagger

        filterTOS = TisTosParticleTagger('Hlt2'+name+'Hlt1TOSFilter')
        filterTOS.TisTosSpecs = self.getProp('TisTosParticleTaggerSpecs')
        filterTOS.Inputs = [ input.outputSelection() ]

        return bindMembers(name, [ input, filterTOS ])
    # }

 
    def __apply_configuration__(self) :
        from HltLine.HltLine import Hlt2Line
        from HltLine.HltLine import Hlt2Member, bindMembers
        from Configurables import HltANNSvc
        from Configurables import CombineParticles
        from Configurables import FilterDesktop
        from Hlt2SharedParticles.TrackFittedBasicParticles import BiKalmanFittedPions, BiKalmanFittedKaons
        from Hlt2SharedParticles.Ks import KsLL as KsData 
        from HltTracking.HltPVs import PV3D

        ## Start applying the member functions we have been defining above

        # Stage 1
        # Filter the KS0s and the bachelor pions and kaons
       
        # Filter the StdLooseLL KS0s
        ksName = self.getProp('name_prefix') + '_KS0LL'
        KS0LLForD2KS0h = self.__KS0LLFilter(ksName, [ KsData ] )

        # Filter the bachelor pions and kaons 
        pionName = self.getProp('name_prefix') + 'BachelorPions'
        kaonName = self.getProp('name_prefix') + 'BachelorKaons'

        pionsBachelorForD2KS0h = self.__BachelorPionFilter(pionName, [ BiKalmanFittedPions] )
        kaonsBachelorForD2KS0h = self.__BachelorKaonFilter(kaonName, [ BiKalmanFittedKaons] )
       
        # Stage 2
        # Stage 2a - bachelor hadron = pion

        # Build the D(s)

        # Make D2KS0Pi
        kSPiName = self.getProp('name_prefix') + '_D2KS0Pi'
        hlt2CharmD2KS0PiComb = self.__D2KS0hCombine(  name = kSPiName 
                                                  , inputSeq = [ KS0LLForD2KS0h, pionsBachelorForD2KS0h]           
                                                  , decayDesc =  ['[D+ -> KS0 pi+]cc'] 
                                                 )   

        # Stage 2b - bachelor hadron = kaon
        Hlt2CharmD2KS0Pi = self.__filterHlt1TOS(kSPiName, hlt2CharmD2KS0PiComb)

        # Build the D(s)

        # Make D2KS0K
        kSKName  = self.getProp('name_prefix') + '_D2KS0K'
        hlt2CharmD2KS0KComb = self.__D2KS0hCombine(  name = kSKName
                                                  , inputSeq = [ KS0LLForD2KS0h, kaonsBachelorForD2KS0h]           
                                                  , decayDesc =  ['[D+ -> KS0 K+]cc'] 
                                                 )   

        Hlt2CharmD2KS0K = self.__filterHlt1TOS(kSKName, hlt2CharmD2KS0KComb)


        ###########################################################################
        # Define the Hlt2 Lines for D(s)->KS0h - seperate pion and kaon lines
        ##########################################################################

        ### D(s)->KS0Pi line

        line = Hlt2Line(kSPiName, prescale = self.prescale
                        , algos = [ PV3D(), KS0LLForD2KS0h, pionsBachelorForD2KS0h, Hlt2CharmD2KS0Pi]
                        # All the necessary algorithms
                        , postscale = self.postscale
                        )
        decName = 'Hlt2' + kSPiName + 'Decision'
        annSvcID = self._scale(decName,'HltANNSvcID')
        HltANNSvc().Hlt2SelectionID.update( { decName : annSvcID } )
        
        ### D(s)->KS0K line

        line2 = Hlt2Line(kSKName, prescale = self.prescale
                        , algos = [ PV3D(), KS0LLForD2KS0h, kaonsBachelorForD2KS0h, Hlt2CharmD2KS0K]
                        # All the necessary algorithms
                        , postscale = self.postscale
                        )
        decName2 = 'Hlt2' + kSKName + 'Decision'
        annSvcID2 = self._scale(decName2,'HltANNSvcID')
        HltANNSvc().Hlt2SelectionID.update( { decName2 : annSvcID2 } )
