# Hlt2 Inclusive Phi selections 06/19/09
#
# Magnus Lieng (magnus.lieng@cern.ch)
# Kim Vervink (Kim.Vervink@cern.ch)
#
# Inclusive Phi line usign three steps:
#  - Robust cuts. (IPS, PT, P, DOCA, MM)
#  - Track fitting: error based cuts. (SIPS, PT, P, VCHI2, MM)
#  - RICH PID calculated. (RICHPID(K-Pi))
#
# At each step kaons candidated are created from tracks. These kaons are
# combined to create Phi candidates. 

from Gaudi.Configuration import *
from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser
from HltLine.HltLine import Hlt2Line
from HltLine.HltLine import Hlt2Member
from Configurables import RichRecSysConf

class Hlt2InclusivePhiLinesConf(HltLinesConfigurableUser) :

    __used_configurables__ = [RichRecSysConf]
    
    __slots__ = {  'KaonPT'             : 800      # MeV
                  ,'KaonIP'             : 0.05     # mm
                  ,'PhiMassWin'         : 12       # MeV
                  ,'PhiMassWinSB'       : 30       # MeV
                  ,'PhiPT'              : 1800     # MeV
                  ,'PhiDOCA'            : 0.2      # mm
                  ,'TFKaonPT'           : 0        # MeV
                  ,'TFKaonIPS'          : 6        # dimensionless
                  ,'TFPhiMassWin'       : 12       # MeV
                  ,'TFPhiMassWinSB'     : 30       # MeV
                  ,'TFPhiPT'            : 1800     # MeV
                  ,'TFPhiVCHI2'         : 20       # dimensionless
                  ,'TFKaonRichPID'      : 0        # dimensionless
                  ,'Prescale'           : {'Hlt2IncPhiSidebands'  : 0.1
                                          ,'Hlt2IncPhi'           : 1.0
                                          ,'Hlt2IncPhiRobust'     : 0.001
                                          ,'Hlt2IncPhiTF'         : 0.001
                                          }
                  ,'Postscale'          : {'Hlt2IncPhiSidebands'  : 1.0
                                          ,'Hlt2IncPhi'           : 1.0
                                          ,'Hlt2IncPhiRobust'     : 1.0
                                          ,'Hlt2IncPhiTF'         : 1.0
                                          }
                  ,'IncludeLines'       : {'IncPhi'           : True
                                          ,'IncPhiSidebands'  : True
                                          ,'IncPhiRobust'     : True
                                          ,'IncPhiTF'         : True
                                          }
                  ,'HltANNSvcID'        : {'IncPhi'           : 50000
                                          ,'IncPhiSidebands'  : 50003
                                          ,'IncPhiRobust'     : 50001
                                          ,'IncPhiTrackFit'   : 50002
                                          }
                  }
    
    
    def __apply_configuration__(self) :
        from Configurables import HltANNSvc
        from Configurables import CombineParticles, PhysDesktop
        from Configurables import FilterDesktop
        from Hlt2SharedParticles.GoodParticles import GoodKaons

        # Some string definitions... 
        decayDesc = ["phi(1020) -> K+ K-"]

        # Track fitting and Rich algs setup
        importOptions("$HLTCONFROOT/options/Hlt2TrackFitForIncPhi.py")

        ############################################################################
        #    Inclusive Phi selection, robust cuts
        ############################################################################
        
        kaonPtCut = "(PT>"+str(self.getProp('KaonPT'))+"*MeV)"
        kaonIpCut = "(MIPDV(PRIMARY)>"+str(self.getProp('KaonIP'))+")"
        phiMassCut = "(ADAMASS('phi(1020)')<"+str(self.getProp('PhiMassWin'))+"*MeV)"
        phiMassCutSB = "(ADAMASS('phi(1020)')<"+str(self.getProp('PhiMassWin'))+"*MeV)"
        phiDocaCut = "(AMINDOCA('LoKi::TrgDistanceCalculator')<"+str(self.getProp('PhiDOCA'))+")"
        phiPtCut = "(PT>"+str(self.getProp('PhiPT'))+"*MeV)"
        Hlt2InclusivePhi = Hlt2Member( CombineParticles
                                       , "Combine"
                                       , DecayDescriptors = decayDesc
                                       , DaughtersCuts = { "K+" : kaonPtCut+" & "+kaonIpCut }
                                       , CombinationCut =  phiMassCut+" & "+phiDocaCut
                                       , MotherCut = phiPtCut
                                       , InputLocations  = [ GoodKaons ]
                                       )
        Hlt2InclusivePhiSB = Hlt2Member( CombineParticles
                                         , "CombineSB"
                                         , DecayDescriptors = decayDesc
                                         , DaughtersCuts = { "K+" : kaonPtCut+" & "+kaonIpCut }
                                         , CombinationCut =  phiMassCutSB+" & "+phiDocaCut
                                         , MotherCut = phiPtCut
                                         , InputLocations  = [ GoodKaons ]
                                         )
        
        
        ############################################################################
        #    Inclusive Phi selection, Track fitted
        ############################################################################
        
        TfKaonPtCut = "(PT>"+str(self.getProp('TFKaonPT'))+"*MeV)"
        TfKaonIpsCut = "(MIPCHI2DV(PRIMARY)>"+str(self.getProp('TFKaonIPS'))+")"
        TfPhiMassCut = "(ADAMASS('phi(1020)')<"+str(self.getProp('TFPhiMassWin'))+"*MeV)"
        TfPhiMassCutSB = "(ADAMASS('phi(1020)')<"+str(self.getProp('TFPhiMassWin'))+"*MeV)"
        TfPhiVchi2Cut = "(VFASPF(VCHI2/VDOF)<"+str(self.getProp('TFPhiVCHI2'))+")"
        TfPhiPtCut = "(PT>"+str(self.getProp('TFPhiPT'))+"*MeV)"
        Hlt2InclusivePhiTF = Hlt2Member( CombineParticles
                                         , "TFCombine"
                                         , DecayDescriptors = decayDesc
                                         , DaughtersCuts = { "K+" : TfKaonPtCut+" & "+TfKaonIpsCut }
                                         , CombinationCut = TfPhiMassCut 
                                         , MotherCut = TfPhiVchi2Cut+" & "+TfPhiPtCut
                                         , InputLocations  = [ "Hlt2IncPhiTFKaons" ]
                                         )
        Hlt2InclusivePhiTFSB = Hlt2Member( CombineParticles
                                           , "TFCombineSB"
                                           , DecayDescriptors = decayDesc
                                           , DaughtersCuts = { "K+" : TfKaonPtCut+" & "+TfKaonIpsCut }
                                           , CombinationCut = TfPhiMassCutSB 
                                           , MotherCut = TfPhiVchi2Cut+" & "+TfPhiPtCut
                                           , InputLocations  = [ "Hlt2IncPhiTFKaons" ]
                                           )
        
        ############################################################################
        #    Inclusive Phi selection, RICH PID
        ############################################################################
        
        TfKaonRichPidTf = "(PIDK>"+str(self.getProp('TFKaonRichPID'))+")"
        Hlt2InclusivePhiRich = Hlt2Member( CombineParticles
                                           , "RichCombine"
                                           , DecayDescriptors = decayDesc
                                           , DaughtersCuts = { "K+" : TfKaonPtCut+" & "+TfKaonIpsCut+" & "+TfKaonRichPidTf }
                                           , CombinationCut = TfPhiMassCut
                                           , MotherCut = TfPhiVchi2Cut+" & "+TfPhiPtCut
                                           , InputLocations  = [ "Hlt2IncPhiRichPIDsKaons" ]
                                           )
        Hlt2InclusivePhiRichSB = Hlt2Member( CombineParticles
                                             , "RichCombineSB"
                                             , DecayDescriptors = decayDesc
                                             , DaughtersCuts = { "K+" : TfKaonPtCut+" & "+TfKaonIpsCut+" & "+TfKaonRichPidTf }
                                             , CombinationCut = TfPhiMassCutSB
                                             , MotherCut = TfPhiVchi2Cut+" & "+TfPhiPtCut
                                             , InputLocations  = [ "Hlt2IncPhiRichPIDsKaons" ]
                                             )
        
        
        

        ############################################################################
        #    Inclusive Phi complete line
        ############################################################################

        if self.getProp('IncludeLines')['IncPhi']:
          line = Hlt2Line('IncPhi'
                          , prescale = self.prescale
                          , algos = [ GoodKaons, 
                                      Hlt2InclusivePhi, 
                                      GaudiSequencer("Hlt2IncPhiTFParticlesSeq"),
                                      Hlt2InclusivePhiTF,
                                      GaudiSequencer("Hlt2IncPhiRichParticlesSeq"),
                                      Hlt2InclusivePhiRich]
                          , postscale = self.postscale
                          )
          HltANNSvc().Hlt2SelectionID.update( { "Hlt2IncPhiDecision" : self.getProp('HltANNSvcID')['IncPhi'] } )

 
        ############################################################################
        #    Inclusive Phi robust only line
        ############################################################################
        
        if self.getProp('IncludeLines')['IncPhiRobust']:
          line = Hlt2Line('IncPhiRobust'
                          , prescale = self.prescale
                          , algos = [ GoodKaons, 
                                      Hlt2InclusivePhi]
                          , postscale = self.postscale
                          )
          HltANNSvc().Hlt2SelectionID.update( { "Hlt2IncPhiRobustDecision" : self.getProp('HltANNSvcID')['IncPhiRobust'] } )


        ############################################################################
        #    Inclusive Phi robust and TF line
        ############################################################################

        if self.getProp('IncludeLines')['IncPhiTF']:
          line = Hlt2Line('IncPhiTrackFit'
                          , prescale = self.prescale
                          , algos = [ GoodKaons, 
                                      Hlt2InclusivePhi, 
                                      GaudiSequencer("Hlt2IncPhiTFParticlesSeq"),
                                      Hlt2InclusivePhiTF]
                          , postscale = self.postscale
                          )
          HltANNSvc().Hlt2SelectionID.update( { "Hlt2IncPhiTrackFitDecision" : self.getProp('HltANNSvcID')['IncPhiTrackFit'] } )


        ############################################################################
        #    Inclusive Phi complete line for mass sidebands
        ############################################################################

        if self.getProp('IncludeLines')['IncPhiSidebands']:
          line = Hlt2Line('IncPhiSidebands'
                          , prescale = self.prescale
                          , algos = [ GoodKaons, 
                                      Hlt2InclusivePhiSB, 
                                      GaudiSequencer("Hlt2IncPhiTFParticlesSeq"),
                                      Hlt2InclusivePhiTFSB,
                                      GaudiSequencer("Hlt2IncPhiRichParticlesSeq"),
                                      Hlt2InclusivePhiRichSB]
                          , postscale = self.postscale
                          )
          HltANNSvc().Hlt2SelectionID.update( { "Hlt2IncPhiSidebandsDecision" : self.getProp('HltANNSvcID')['IncPhiSidebands'] } )
          
