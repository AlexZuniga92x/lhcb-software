# =============================================================================
# $Id: Hlt2ExpressLines.py,v 1.26 2010-05-20 09:39:26 graven Exp $
# =============================================================================
## @file
#  Configuration of Hlt2 Lines for the express stream
#  @author Johannes Albrecht albrecht@cern.ch
#  @date 2009-12-02
# =============================================================================
"""
 script to configure Hlt2 Express stream
"""
# =============================================================================
__author__  = "Johannes Albrecht albrecht@cern.ch"
__version__ = "CVS Tag $Name: not supported by cvs2svn $, $Revision: 1.26 $"
# =============================================================================

from HltLine.HltLinesConfigurableUser import *

def __monitor__ ( variable ,  label , center , window, id =None, nbins = 100 ) :
      return "process ( monitor( %s,  Gaudi.Histo1DDef( '%s', %s , %s, %s), '%s' ) ) >> ~EMPTY " % (variable, label, center-window,center+window, nbins, id if id else variable )

class Hlt2ExpressLinesConf(HltLinesConfigurableUser):
   
   __slots__ = { 'Prescale'  : { 'Hlt2ExpressJPsi'        : 1.
                               , 'Hlt2ExpressJPsiTagProbe': 1.
                               , 'Hlt2ExpressLambda'      : 1.
                               , 'Hlt2ExpressKS'          : 1.
                               , 'Hlt2ExpressDs2PhiPi'    : 1.
                               , 'Hlt2ExpressBeamHalo'    : 1.
                               , 'Hlt2ExpressDStar2D0Pi'  : 1.
                                 }
               , 'Postscale' : { 'Hlt2ExpressJPsi'        : 'RATE(5)'
                               , 'Hlt2ExpressJPsiTagProbe': 'RATE(5)'
                               , 'Hlt2ExpressLambda'      : 'RATE(1)'
                               , 'Hlt2ExpressKS'          : 'RATE(1)'
                               , 'Hlt2ExpressDs2PhiPi'    : 'RATE(1)'
                               , 'Hlt2ExpressBeamHalo'    : 'RATE(1)'
                               , 'Hlt2ExpressDStar2D0Pi'  : 'RATE(1)'
                                 }
               , 'ExJPsiMassWindow'        :  120   # MeV
               , 'ExJPsiPt'                : 1000   # MeV
               , 'ExJPsiMuPt'              :  500   # MeV
               , 'ExJPsiTPMassWindow'      :  300   # MeV
               , 'ExJPsiTPVChi2'           :   20
               , 'ExJPsiTPCoP'             : 3000   # MeV
               , 'ExJPsiTPCoPt'            :  800   # MeV
               , 'ExJPsiTPTagP'            : 6000   # MeV
               , 'ExJPsiTPTagPt'           : 1500   # MeV
               , 'ExJPsiTPMaxEcalE'        : 1000   # MeV
               , 'ExJPsiTPMinEcalE'        :  -10   # MeV
               , 'ExJPsiTPMaxHcalE'        : 4000   # MeV
               , 'ExJPsiTPMinHcalE'        : 1000   # MeV
               , 'ExLambdaMassWin'         :   20   # MeV
               , 'ExLambdaMassWinWide'     :   30   # MeV
               , 'ExLambdaMinDz'           :   50   #  mm
               , 'ExLambdaMaxDz'           :  600   #  mm
               , 'ExLambdaDira'            : 0.9999995
               , 'ExLambdaPiP'             : 3000   # MeV
               , 'ExLambdaPiPt'            :  100   # MeV
               , 'ExLambdaPiIPChi2'        :    9
               , 'ExLambdaPP'              : 3000   # MeV
               , 'ExLambdaPPt'             :  100   # MeV
               , 'ExLambdaPIPChi2'         :    9
               , 'ExKSNu1'                 :    2   
               , 'ExKSMassWinWide'         :  150   # MeV
               , 'ExKSMassWin'             :  100   # MeV
               , 'ExPhiMassWinWide'        :   70 # MeV
               , 'ExPhiMassWin'            :   50 # MeV
               , 'ExPhiDOCAMax'		   :  10. # mm
	       , 'ExPhiMIPCHI2DV'          :  2.18 #log(2.5) = 12.18
               , 'ExPhiKPt'                :  300 # MeV
               , 'ExPhiKP'                 : 1000 # MeV
               , 'ExPhiKMIPCHI2DV'         :  1.0
	       , 'ExDsMassWinWide'         :  100 # MeV
               , 'ExDsMassWin'             :   50 # MeV
               , 'ExDsBPVDIRA'             :    0.999
               , 'ExDsVCHI2'               :   12.18 #log(2.5) = 12.18
	       , 'ExDsMIPCHI2DV'	   :   12.18 #log(2.5) = 12.18
               , 'ExDsMIPDV'               :  0.05
	       , 'ExDsPiPt'                :  300 # MeV
               , 'ExDsPiP'                 : 1000 # MeV
               , 'ExDsPiMIPCHI2DV'         :  12.18 #log(2.5) = 12.18
               , 'ExHaloMinTot'            :  20 
               , 'ExHaloMaxTot'            : 5000 
               , 'ExHaloMinRSlice'         :  10 
               , 'ExHaloMinCell'           :  10
               , 'ExHaloMaxCell'           :  100
               , 'ExHaloMaxDiffCell'       :  -1
               , 'ExHaloMaxSensor'         :  25 
               , 'ExHaloOverlaps'          :  False
               , 'ExHaloBigCell'           :  False
               , 'ExHaloMinOverlap'        :  3
               , 'ExD0MassWinWide'         :  100   # MeV
               , 'ExD0MassWin'             :   50   # MeV
               , 'ExD0VCHI2'               :   10 
               , 'ExD0Pt'                  : 1000   # MeV
               , 'ExD0BPVDIRA'             : 0.9999
               , 'ExD0BPVVDCHI2'           :   12
               , 'ExD0KP'                  : 2000   # MeV
               , 'ExD0KPt'                 :  400   # MeV
               , 'ExD0KIPChi2'             :    6
               , 'ExD0PiP'                 : 2000   # MeV
               , 'ExD0PiPt'                :  400   # MeV
               , 'ExD0PiIPChi2'            :    6
               , 'ExDStarMassWinWide'      :  100   # MeV
               , 'ExDStarMassWin'          :   50   # MeV
               , 'ExDStarPt'               : 2200   # MeV
               , 'ExDStarVCHI2'            :   15
               , 'ExDStarMassDiff'         :155.5   # MeV
               , 'ExDStarPiPt'             : 110    # MeV
               , 'ExDStarPiIPChi2'         :   2
               }  
   

   def __apply_configuration__(self):
      from HltLine.HltLine import Hlt2Line, Hlt2Member, bindMembers
      from HltLine.Hlt2Monitoring import Hlt2Monitor,Hlt2MonitorMinMax
      from HltTracking.HltPVs import PV3D
      from Configurables import HltANNSvc
      from Hlt2SharedParticles.DiMuon import DiMuon
      from Configurables import FilterDesktop
      '''
      Comment content of express stream here
      '''
      
      #--------------------------------------------
      '''
      unbiased JPsi - prescaled to ~5 Hz
      '''
      
      filter = Hlt2Member(   FilterDesktop 
                             , "Filter"
                             , Code = " (ADMASS('J/psi(1S)')<%(ExJPsiMassWindow)s*MeV)"\
                             " & (PT>%(ExJPsiPt)s*MeV)"\
                             " & (MINTREE('mu-'==ABSID,PT)>%(ExJPsiMuPt)s*MeV) " %  self.getProps() 
                             , InputLocations  = [ DiMuon ]
                             , InputPrimaryVertices = "None"
                             , UseP2PVRelations = False
                             , PreMonitor  =  Hlt2Monitor( "M","M(#mu#mu)",3097,self.getProp("ExJPsiMassWindow"),'M_in',nbins=101) 
                             , PostMonitor =  Hlt2Monitor( "M","M(#mu#mu)",3097,self.getProp("ExJPsiMassWindow"),'M_out',nbins=101)
                             )
      
      line = Hlt2Line('ExpressJPsi'
                      , prescale = self.prescale 
                      , algos = [ DiMuon, filter ]
                      , postscale = self.postscale
                      )
      #--------------------------------------------
      '''
      JPsi from only one muon (tag and probe)
      from StrippingMuIDCalib.py

      '''
      from Hlt2SharedParticles.TagAndProbeParticles import TagAndProbeMuons, TagAndProbePions
      from Configurables import CombineParticles
      from HltTracking.HltPVs import PV3D
 
     ############################
      # Prompt Jpsi tag-and-probe
     #############################
      
      # common cut
      cocut = "(0.5<PPINFO(LHCb.ProtoParticle.InAccMuon,-1))"\
      " & (P>%(ExJPsiTPCoP)s*MeV) &(PT>%(ExJPsiTPCoPt)s*MeV)"%  self.getProps()
      
      #Tag and probe cuts:
      #   TAG::  P>6Gev and Pt>1.5 GeV
      #   PROBE:: Ecal (-10,1000) and Hcal (1000,4000) energy filtered
      tag1cuts = "  (CHILDCUT((P>%(ExJPsiTPTagP)s*MeV),1))"\
                 " & (CHILDCUT((PT>%(ExJPsiTPTagPt)s*MeV),1)) "%  self.getProps()
      tag2cuts = "  (CHILDCUT((P>%(ExJPsiTPTagP)s*MeV),2))"\
                 " & (CHILDCUT((PT>%(ExJPsiTPTagPt)s*MeV),2)) "%  self.getProps()
      
      probe2cuts = " ( "\
      "(CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)<%(ExJPsiTPMaxEcalE)s*MeV),2)) &"\
      "(CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)<%(ExJPsiTPMaxHcalE)s*MeV),2)) &"\
      "(CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)>%(ExJPsiTPMinEcalE)s*MeV),2)) &"\
      "(CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)>%(ExJPsiTPMinHcalE)s*MeV),2)) ) "%  self.getProps()
      probe1cuts = " ( "\
      "(CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)<%(ExJPsiTPMaxEcalE)s*MeV),1)) &"\
      "(CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)<%(ExJPsiTPMaxHcalE)s*MeV),1)) &"\
      "(CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloEcalE,-10000)>%(ExJPsiTPMinEcalE)s*MeV),1)) &"\
      "(CHILDCUT((PPINFO(LHCb.ProtoParticle.CaloHcalE,-10000)>%(ExJPsiTPMinHcalE)s*MeV),1)) ) "%  self.getProps()
      
      child1cuts = tag1cuts + " & " + probe2cuts
      
      child2cuts = tag2cuts + " & " + probe1cuts
     
      JPsiCombine = Hlt2Member( CombineParticles
                                , 'JPsiCombine'
                                , InputLocations = [ TagAndProbePions, TagAndProbeMuons ]
                                , DecayDescriptor = '[J/psi(1S) -> mu+ pi-]cc'
                                , DaughtersCuts = { 'mu+' : cocut ,
                                                    'pi-' : cocut }
                                , CombinationCut = "(ADAMASS('J/psi(1S)')<%(ExJPsiTPMassWindow)s*MeV)"%  self.getProps()
                                , MotherCut = "( " + child1cuts + " | " + child2cuts + " ) "\
                                " & (VFASPF(VCHI2/VDOF)<%(ExJPsiTPVChi2)s)"%  self.getProps()
                                )

      line = Hlt2Line('ExpressJPsiTagProbe'
                      , prescale = self.prescale
                      , HLT = "HLT_PASS_RE('Hlt1.*SingleMuon.*Decision')"
                      , algos = [ PV3D(), TagAndProbeMuons, TagAndProbePions, JPsiCombine ]
                      , postscale = self.postscale
                      )
      
      #--------------------------------------------
      '''
      Lambda no PID
      from StrippingLambdaNoPID.py
      '''
      from Hlt2SharedParticles.BasicParticles import NoCutsPions,NoCutsProtons
      from Configurables import CombineParticles
      from HltTracking.HltPVs import PV3D
     
      LambdaCombine = Hlt2Member( CombineParticles
                                  , "LambdaCombine"
                                  , DecayDescriptor = "[Lambda0 -> p+ pi-]cc"
                                  , InputLocations = [NoCutsPions,NoCutsProtons ]
                                  , CombinationCut = "(ADAMASS('Lambda0')<%(ExLambdaMassWinWide)s*MeV)"%  self.getProps()
                                  , MotherCut = "(ADMASS('Lambda0')<%(ExLambdaMassWin)s*MeV)"\
                                  " & (%(ExLambdaMinDz)s*mm<BPVVDZ)"\
                                  " & (%(ExLambdaMaxDz)s*mm>BPVVDZ)"\
                                  " & (BPVDIRA>%(ExLambdaDira)s) "%  self.getProps()
                                  , DaughtersCuts = { "p+"  :  "(P>%(ExLambdaPP)s*MeV)"\
                                                      " & (PT>%(ExLambdaPPt)s*MeV)"\
                                                      " & (MIPCHI2DV(PRIMARY)>%(ExLambdaPIPChi2)s) "%  self.getProps(),
                                                      "pi-"  :  "(P>%(ExLambdaPiP)s*MeV)"\
                                                      " & (PT>%(ExLambdaPiPt)s*MeV)"\
                                                      " & (MIPCHI2DV(PRIMARY)>%(ExLambdaPiIPChi2)s) "%  self.getProps(),
                                                      }
                                  , MotherMonitor  =  Hlt2Monitor("M", "M(p#pi)",1116,self.getProp("ExLambdaMassWin"))
                                  )
      
      line = Hlt2Line('ExpressLambda'
                      , prescale = self.prescale 
                      , algos = [ PV3D(), NoCutsPions, NoCutsProtons, LambdaCombine ]
                      , postscale = self.postscale
                      )
      #--------------------------------------------
      '''
      Ks no PID
      '''
      from Hlt2SharedParticles.BasicParticles import NoCutsPions,NoCutsProtons
      from Configurables import CombineParticles
      from HltTracking.HltPVs import PV3D

      KsCombine = Hlt2Member( CombineParticles
                              , "KsCombine"
                              , DecayDescriptor = "KS0 -> pi+ pi-"
                              , InputLocations = [NoCutsPions]
                              , CombinationCut = "(ADAMASS('KS0') < %(ExKSMassWinWide)s*MeV)"%  self.getProps()
                              , MotherCut = "(ADMASS('KS0') < %(ExKSMassWin)s*MeV)"\
                              " & (log((CHILD(MIPDV(PRIMARY), 1)) * (CHILD(MIPDV(PRIMARY), 2) )"\
                              " / (MIPDV(PRIMARY) ) )  > %(ExKSNu1)s*mm )"%  self.getProps()
                              , MotherMonitor  =  Hlt2Monitor("M", "M(#pi#pi)",498,self.getProp("ExKSMassWin"))
                              )
      
      line = Hlt2Line('ExpressKS'
                      , prescale = self.prescale 
                      , algos = [ PV3D(), NoCutsPions, KsCombine]
                      , postscale = self.postscale
                      )
      #--------------------------------------------
      '''
      Phi no PID

      TODO JA 18. March
      Updated selection, please check!

      '''
      from Hlt2SharedParticles.BasicParticles import NoCutsKaons
      from Configurables import CombineParticles
      from HltTracking.HltPVs import PV3D

      PhiCombine = Hlt2Member( CombineParticles
                               , "PhiCombine"
                               , DecayDescriptor = "phi(1020) -> K+ K-"
                               , InputLocations = [NoCutsKaons]
                               , CombinationCut = "(ADAMASS('phi(1020)')<%(ExPhiMassWinWide)s*MeV)"%  self.getProps()
                               , MotherCut = "(ADMASS('phi(1020)') < %(ExPhiMassWin)s*MeV)"\
                               " & (DOCAMAX < %(ExPhiDOCAMax)s*mm)"\
			       " & (MIPCHI2DV(PRIMARY) > %(ExPhiMIPCHI2DV)s)"%  self.getProps()
                              , DaughtersCuts = {"K+":"(PT>%(ExPhiKPt)s*MeV)"\
                                                  " & (P>%(ExPhiKP)s*MeV)"\
						  " & (MIPCHI2DV(PRIMARY) > %(ExPhiKMIPCHI2DV)s)"%  self.getProps()}
                             , MotherMonitor  =  Hlt2Monitor("M", "M(KK)",1020,self.getProp("ExPhiMassWin"))
                             )

      DsCombine = Hlt2Member( CombineParticles
                              , "DsCombine"
                              , DecayDescriptor = "[D_s+ -> pi+ phi(1020)]cc"
                              , InputLocations = [NoCutsPions, PhiCombine]
                              , CombinationCut = "(ADAMASS('D_s+')<%(ExDsMassWinWide)s*MeV)"%  self.getProps()
                              , MotherCut = "(ADMASS('D_s+')<%(ExDsMassWin)s*MeV)"\
                              " & (BPVDIRA > %(ExDsBPVDIRA)s)"\
                              " & (VFASPF(VCHI2) < %(ExDsVCHI2)s)"\
			      " & (MIPCHI2DV(PRIMARY) < %(ExDsMIPCHI2DV)s)"\
			      " & (MIPDV(PRIMARY) < %(ExDsMIPDV)s)"%  self.getProps()
                              , DaughtersCuts = {"pi+":"(PT > %(ExDsPiPt)s*MeV)"\
                                                 " & (P > %(ExDsPiP)s*MeV)"\
						 " & (MIPCHI2DV(PRIMARY) > %(ExDsPiMIPCHI2DV)s)"%  self.getProps()}
                              , MotherMonitor  =  Hlt2Monitor( "M","M(#phi#pi)",1968,self.getProp("ExDsMassWin"))
                              )
      
      line = Hlt2Line('ExpressDs2PhiPi'
                      , prescale = self.prescale 
                      , algos = [ PV3D(), NoCutsPions, NoCutsKaons, PhiCombine, DsCombine]
                      , postscale = self.postscale
                      )

    
      #--------------------------------------------      
      '''
      selection of beam halo tracks
      '''
      from Configurables import Tf__PatVeloAlignTrackFilter as PatVeloAlignTrackFilter
      from HltLine.HltDecodeRaw import DecodeVELO

      Hlt2Line('ExpressBeamHalo'
               ,prescale = self.prescale
               , algos = [ DecodeVELO
                           , PatVeloAlignTrackFilter('Hlt2ExpressBeamHaloDecision'
                                                     , MinTot = self.getProp('ExHaloMinTot')  
                                                     , MaxTot = self.getProp('ExHaloMaxTot')  
                                                     , MinRSlice = self.getProp('ExHaloMinRSlice')  
                                                     , MinCell = self.getProp('ExHaloMinCell') 
                                                     , MaxCell = self.getProp('ExHaloMaxCell') 
                                                     , MaxDiffCell = self.getProp('ExHaloMaxDiffCell') 
                                                     , MaxSensor = self.getProp('ExHaloMaxSensor')  
                                                     , Overlaps = self.getProp('ExHaloOverlaps') 
                                                     , BigCell = self.getProp('ExHaloBigCell') 
                                                     , MinOverlap = self.getProp('ExHaloMinOverlap')
                                                     ) 
                           ]
               , postscale = self.postscale
               )
      
      #--------------------------------------------      
      '''
      D* no PID

      From RichPIDQC/DstarToDzeroPi.py

      '''
      from Hlt2SharedParticles.BasicParticles import NoCutsKaons, NoCutsPions
      from Configurables import CombineParticles
      from HltTracking.HltPVs import PV3D
      
      D02KPiCombine = Hlt2Member( CombineParticles
                                  , "D02KPiCombine"
                                  , DecayDescriptor = "[ D0 -> K- pi+ ]cc"
                                  , InputLocations = [NoCutsKaons, NoCutsPions]
                                  , CombinationCut = "(ADAMASS('D0')<%(ExD0MassWinWide)s*MeV)"%  self.getProps()
                                  , MotherCut = "(ADMASS('D0') < %(ExD0MassWin)s*MeV)"\
                                  " & (VFASPF(VCHI2/VDOF) < %(ExD0VCHI2)s)"\
                                  " & (PT > %(ExD0Pt)s*MeV)"\
                                  " & (BPVDIRA > %(ExD0BPVDIRA)s)"\
                                  " & (BPVVDCHI2 > %(ExD0BPVVDCHI2)s)"%  self.getProps()
                                  , DaughtersCuts = { "K-"  :  "(P>%(ExD0KP)s*MeV)"\
                                                      " & (PT>%(ExD0KPt)s*MeV)"\
                                                      " & (MIPCHI2DV(PRIMARY)>%(ExD0KIPChi2)s) "%  self.getProps(),
                                                      "pi+"  :  "(P>%(ExD0PiP)s*MeV)"\
                                                      " & (PT>%(ExD0PiPt)s*MeV)"\
                                                      " & (MIPCHI2DV(PRIMARY)>%(ExD0PiIPChi2)s) "%  self.getProps()
                                                      }
                                  , MotherMonitor  =  Hlt2Monitor("M", "M(K#pi)",1865,self.getProp("ExD0MassWin"))
                                  )

      DStarCombine = Hlt2Member( CombineParticles
                                 , "DStarCombine"
                                 , DecayDescriptor = "[ D*(2010)+ -> D0 pi+ ]cc"
                                 , InputLocations = [NoCutsPions, D02KPiCombine]
                                 , CombinationCut = "(ADAMASS('D*(2010)+')<%(ExDStarMassWinWide)s*MeV)"%  self.getProps()
                                 , MotherCut = "(ADMASS('D*(2010)+')<%(ExDStarMassWin)s*MeV)"\
                                 " & (PT > %(ExDStarPt)s*MeV)"\
                                 " & (VFASPF(VCHI2/VDOF) < %(ExDStarVCHI2)s)"\
                                 " & (M-MAXTREE('D0'==ABSID,M)< %(ExDStarMassDiff)s)"%  self.getProps()
                                 , DaughtersCuts = {"pi+":"(PT>%(ExDStarPiPt)s*MeV)"\
                                                    " & (MIPCHI2DV(PRIMARY)>%(ExDStarPiIPChi2)s) "%  self.getProps()
                                                    }
                                 , MotherMonitor  =  Hlt2Monitor("M", "M(D0#pi)",2010,self.getProp("ExDStarMassWin"))
                                 #                  "process ( monitor( M-MAXTREE('D0'=ABSID,M), Gaudi.Histo1DDef( '#Delta M',120,200),'DM') ) >> ~EMPTY "
                                 )

      line = Hlt2Line('ExpressDStar2D0Pi'
                      , prescale = self.prescale 
                      , algos = [ PV3D(), NoCutsPions, NoCutsKaons, D02KPiCombine, DStarCombine]
                      , postscale = self.postscale
                      ) 
      
      #--------------------------------------------                      
      
      
      HltANNSvc().Hlt2SelectionID.update( { 'Hlt2ExpressJPsiDecision'         : 50090
                                          , 'Hlt2ExpressJPsiTagProbeDecision' : 50091
                                          , 'Hlt2ExpressLambdaDecision'       : 50092
                                          , 'Hlt2ExpressKSDecision'           : 50093
                                          , 'Hlt2ExpressDs2PhiPiDecision'     : 50094
                                          , 'Hlt2ExpressBeamHaloDecision'     : 50095
                                          , 'Hlt2ExpressDStar2D0PiDecision'   : 50096
                                          } )

