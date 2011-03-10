__author__  = [ 'Sandra Amato', 'Carlo Gobel', 'Patrick Spradlin' ]
__date__    = '$Date: $'
__version__ = '$Revision: $'


from Gaudi.Configuration import *
from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser
from GaudiKernel.SystemOfUnits import MeV, GeV, mm

from Hlt2Lines.Hlt2CharmHadD2HHHLines     import Hlt2CharmHadD2HHHLinesConf
from Hlt2Lines.Hlt2CharmHadD02HHHHLines   import Hlt2CharmHadD02HHHHLinesConf

## Must inherit from HltLinesConfigurableUser for slots to be updated by
## HltSettings, even though this configurable does not directly define a
## line.
class Hlt2CharmHadTwoBodyForMultiBodyConf(HltLinesConfigurableUser) : # {
    """
    Defines a configurable to create a 2-body reconstruction that is used
    as a first stage for the D -> 3H and D -> 4H lines prior to the SecondLoop
    track reconstruction.

    NOTE:  Any Hlt2Line configurable that uses this MUST be on the list of
    __used_configurables__.
    """

    ## Must be configured before every configurable that uses it.
    __used_configurables__ = [   Hlt2CharmHadD2HHHLinesConf
                               , Hlt2CharmHadD02HHHHLinesConf ]

    __slots__ = {
                  ## 2-body Input for 3Body
                    'Trk_PT_MIN'                :  500.0 * MeV
                  , 'Trk_P_MIN'                 : 5000.0 * MeV
                  , 'Trk_MIPCHI2DV_MIN'         :   10.0      # unitless
                  , 'Trk_TRCHI2DOF_MAX'         :    3.0      # unitless
                  , 'Pair_AMINDOCA_MAX'         :    0.1 * mm
                  , 'Pair_BPVVDCHI2_MIN'        :   40.0      # unitless  
                  , 'Pair_BPVVD_MIN'            :    3.0 * mm
                  , 'Pair_SumAPT_MIN'           : 2000.0 * MeV
                  , 'Pair_BPVCORRM_MAX'         : 3500.0 * MeV
                  , 'name_prefix'              : 'CharmHadTwoBodyForMultiBody'
                  ##
                  , '__hlt2TwoBodySeq__'       : None
                }


    ## Sequence will be empty if called before configured.
    ## This is why it is imperative that client configurables be on the
    ##   __used_configurables__ list.
    def twoBodySequence(self) : # {
        return self.getProp("__hlt2TwoBodySeq__")
    # }


    def __apply_configuration__(self) : # {
        from HltLine.HltLine import Hlt2Member, bindMembers
        from Configurables import CombineParticles
        from Hlt2SharedParticles.TrackFittedBasicParticles import BiKalmanFittedPions, BiKalmanFittedKaons
        from HltTracking.HltPVs import PV3D
        

        twoBodyDaughterCut = "(TRCHI2DOF< %(Trk_TRCHI2DOF_MAX)s )" \
                             "& (PT> %(Trk_PT_MIN)s)" \
                             "& (P> %(Trk_P_MIN)s)" \
                             "& (MIPCHI2DV(PRIMARY)> %(Trk_MIPCHI2DV_MIN)s )" % self.getProps()
 
        twoBodyCombCut = "(AM<2100*MeV)" \
                         "& ((APT1+APT2)> %(Pair_SumAPT_MIN)s)" \
                         "& (AMINDOCA('LoKi::TrgDistanceCalculator') < %(Pair_AMINDOCA_MAX)s )" \
                         "& (AALLSAMEBPV)" % self.getProps()

        twoBodyMotherCut = "(BPVVD> %(Pair_BPVVD_MIN)s )" \
                           "& (BPVCORRM < %(Pair_BPVCORRM_MAX)s)" \
                           "& (BPVVDCHI2> %(Pair_BPVVDCHI2_MIN)s )" % self.getProps()
        #First stage - Combine 2 Body with pt > 500MeV        
        Charm2BodyCombine = Hlt2Member( CombineParticles
                          , "Combine"
                          , DecayDescriptors = ["K*(892)+ -> pi+ pi+" , "K*(892)0 -> pi+ pi-"
                          , "K*(892)- -> pi- pi-" , "K*(892)+ -> K+ K+"
                          , "K*(892)0 -> K+ K-"   , "K*(892)- -> K- K-"
                          , "K*(892)0 -> K+ pi-"  , "K*(892)0 -> K- pi+"
                          , "K*(892)+ -> K+ pi+"  , "K*(892)- -> K- pi-" ]
                          , DaughtersCuts = { "pi+" : twoBodyDaughterCut,
                                              "K+"  : twoBodyDaughterCut }
                          , CombinationCut = twoBodyCombCut 
                          , MotherCut = twoBodyMotherCut
                          , Inputs = [ BiKalmanFittedPions, BiKalmanFittedKaons ])

        twoBodySeq = bindMembers(self.getProp('name_prefix'), [ BiKalmanFittedPions, BiKalmanFittedKaons, PV3D(), Charm2BodyCombine ])
 
        self.setProp('__hlt2TwoBodySeq__', twoBodySeq)
    # }

# }
