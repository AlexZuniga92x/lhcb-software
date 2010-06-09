## @file
#
#  Hlt2 dimuon selections
#
#  @author J. Albrecht, Leandro de Paula, Antonio Perez-Calero
#  @date 2009-02-13
#
#   a) unbiased dimuon selections, i.e. dimuon selections without
#           cuts correlated to the Bs lifetime (J. Albrecht)
#
#   b) biased dimuon (Leandro de Paula)
#
##
from Gaudi.Configuration import *
from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser



class Hlt2InclusiveDiMuonLinesConf(HltLinesConfigurableUser) :
    '''
          Put only the most discriminating variables as slots:
           - prescale factors, mass windows and dimuon pt
           - muon pt _not_ as slot as this strongly modifies the angular acceptances
    '''
    
    
    __slots__ = {  'Prescale'                  : {  'Hlt2UnbiasedDiMuon'         :  0.05
                                                    ,'Hlt2UnbiasedDiMuonLowMass' :  0.001
                                                    ,'Hlt2DiMuonUnbiasedJPsiLow' :  0.25
                                                    ,'Hlt2BiasedDiMuonSimple'    :  0.01  
                                                    ,'Hlt2BiasedDiMuonRefined'   :  0.01
                                                    ,'Hlt2BiasedDiMuonIP'        :  0.5
                                                    ,'Hlt2DiMuonDY1'             :  0.01
                                                    ,'Hlt2DiMuonDY2'             :  0.01
                                                    }

                   ,'UnbiasedDiMuonMinMass'   : 2700      # MeV
                   ,'UnbiasedDiMuonPt'        : 1000      # MeV
                   ,'UnbiasedDiMuonMuPt'      :  500      # MeV
                   ,'UnbiasedDiMuonVertexChi2':   25


                   ,'UnbiasedDiMuonLowMinMass':  500      # MeV
                   ,'UnbiasedDiMuonLowPt'     : 1000      # MeV
                   ,'UnbiasedDiMuonLowMuPt'   :  500      # MeV
                   ,'UnbiasedDiMuonLowChi2'   :   25
                    
                   ,'UnbiasedJPsiMassWindow'  :   70      # MeV
                   ,'UnbiasedJPsiPt'          : 1000      # MeV
                   ,'UnbiasedJPsiMuPt'        :  500      # MeV
                   ,'UnbiasedJPsiVertexChi2'  :   25
                   
                   ,'UnbiasedPsi2SMassWindow' :   70      # MeV
                   ,'UnbiasedPsi2SPt'         : 1000      # MeV
                   ,'UnbiasedPsi2SMuPt'       : 1500      # MeV
                   ,'UnbiasedPsi2SVertexChi2' :   25
                   
                   ,'UnbiasedBmmMinMass'      : 5200      # MeV
                   ,'UnbiasedBmmVertexChi2'   :   25

                   ,'UnbiasedZmmMinMass'      :40000      # MeV
                   ,'UnbiasedZmmPt'           :10000      # MeV

                   ,'DYPt'                    :  500      # MeV
                   ,'DY1MinMass'              : 2500      # MeV
                   ,'DY1MaxMass'              : 5000      # MeV
                   ,'DY2MinMass'              : 5000      # MeV
                   ,'DY2MaxMass'              :10000      # MeV
                   ,'DY3MinMass'              :10000      # MeV
                   ,'DY3MaxMass'              :20000      # MeV
                   ,'DY4MinMass'              :20000      # MeV
                   ,'DY4MaxMass'              :40000      # MeV
                   
                   ,'BiasedSingleMuonPt'      :  700      # MeV
                   ,'BiasedMass'              :  500      # MeV
                   ,'BiasedLMass'             : 1200      # MeV
                   ,'BiasedTMass'             : 2900      # MeV
                   ,'BiasedSingleMuonIP'      :     .02   # mm 
                   ,'BiasedSingleMuonTIP'     :     .05   # mm 
                   ,'BiasedLTime'             :     .1    # ps 
                   ,'BiasedLTimeT'            :     .10   # ps 
                   ,'BiasedSingleIPChi2'      :   10     
                   ,'BiasedSingleIPTChi2'     :   50     
                   ,'BiasedVertexChi2'        :    5
                   ,'BiasedPVDistanceChi2'    :   10
                   ,'BiasedPVDistanceTChi2'   :  150

                   }
    

    def __apply_configuration__(self) :
        from HltLine.HltLine import Hlt2Line, Hlt2Member, bindMembers
        from HltLine.Hlt2Monitoring import Hlt2Monitor, Hlt2MonitorMinMax
        from HltTracking.HltPVs import PV3D
        from Configurables import HltANNSvc
        from Hlt2SharedParticles.DiMuon import DiMuon
        from Configurables import FilterDesktop
        '''
          There are in total four unbiased selections:
          
          1) Heavy dimuon for sidebands etc, prescaled by 0.05
          2) mass window around J/psi
          3) mass window around Psi(2S)
          4) mass above 5 GeV
        '''

        from Configurables import CombineParticles
        from Hlt2SharedParticles.BasicParticles import Muons
        #--------------------------------------------        
        '''
        unbiased same sign dimuon
        '''
        SameSignDiMu = Hlt2Member( CombineParticles
                                   , "SameSignDiMu"
                                   , DecayDescriptor = "[B0 -> mu+ mu+]cc"
                                   , DaughtersCuts = { "mu+" : "(PT>%(UnbiasedDiMuonMuPt)s*MeV)"%  self.getProps() }  
                                   
                                   , CombinationCut = "(AM>%(UnbiasedDiMuonMinMass)s*MeV)"%  self.getProps()  
                                   , MotherCut = "(PT>%(UnbiasedDiMuonPt)s*MeV)"\
                                   "& (VFASPF(VCHI2/VDOF)<%(UnbiasedDiMuonVertexChi2)s )" %  self.getProps()
                                   , InputPrimaryVertices = "None"
                                   , UseP2PVRelations = False
                                   , InputLocations  = [ Muons ]
                                   )
        line = Hlt2Line('DiMuonSameSign'
                        , prescale = self.prescale 
                        , algos = [Muons,SameSignDiMu]
                        , postscale = self.postscale
                        )

        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DiMuonSameSignDecision":   50211 } )
         #--------------------------------------------

        filter = Hlt2Member(   FilterDesktop 
                               , "Filter"
                               , Code = "(MM>%(UnbiasedDiMuonMinMass)s*MeV)"\
                               " & (PT>%(UnbiasedDiMuonPt)s*MeV)"\
                               " & (MINTREE('mu-'==ABSID,PT)>%(UnbiasedDiMuonMuPt)s*MeV) "\
                               "& (VFASPF(VCHI2/VDOF)<%(UnbiasedDiMuonVertexChi2)s )" %  self.getProps() 
                               , InputPrimaryVertices = "None"
                               , UseP2PVRelations = False
                               , InputLocations  = [ DiMuon ]
#                               , PreMonitor  =  Hlt2Monitor( "M","M(#mu#mu)",3097,200,'M_in',nbins=25) 
 #                              , PostMonitor =  Hlt2Monitor( "M","M(#mu#mu)",3097,200,'M_out',nbins=25)
                               )
        
        #--------------------------------------------
        '''
        unbiased heavy dimuon - prescaled
        '''
        line = Hlt2Line('UnbiasedDiMuon'
                        , prescale = self.prescale 
                        , algos = [ DiMuon, filter ]
                        , postscale = self.postscale
                        )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2UnbiasedDiMuonDecision" : 50200 } )
            
        #--------------------------------------------
        '''
        unbiased dimuon low mass - prescaled
        '''
        line.clone( 'UnbiasedDiMuonLowMass'
                    , prescale = self.prescale 
                    , Filter = { 'Code': "(MM>%(UnbiasedDiMuonLowMinMass)s*MeV)"\
                                 "& (PT>%(UnbiasedDiMuonLowPt)s*MeV) "\
                                 "& (MINTREE('mu-'==ABSID,PT)>%(UnbiasedDiMuonLowMuPt)s*MeV) "\
                                 "& (VFASPF(VCHI2/VDOF)<%(UnbiasedDiMuonLowChi2)s )"%  self.getProps()
                                 }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2UnbiasedDiMuonLowMassDecision":  50210 } )
        #--------------------------------------------
        '''
        unbiased J/psi
        '''
        line.clone( 'DiMuonUnbiasedJPsi'
                    , prescale = self.prescale 
                    , Filter = { 'Code': "(ADMASS('J/psi(1S)')<%(UnbiasedJPsiMassWindow)s*MeV) "\
                                 "& (PT>%(UnbiasedJPsiPt)s*MeV) "\
                                 "& (MINTREE('mu-'==ABSID,PT)>%(UnbiasedJPsiMuPt)s*MeV) "\
                                 "& (VFASPF(VCHI2/VDOF)<%(UnbiasedJPsiVertexChi2)s )"%  self.getProps()
                                 , 'PreMonitor' : Hlt2Monitor( "M","M(#mu#mu)",3097,200,'M_in',nbins=25) 
                                 , 'PostMonitor' :  
                                 Hlt2Monitor( "M","M(#mu#mu)",3097,200,'M_out',nbins=25)
                                 +" & "+Hlt2MonitorMinMax( "PT","PT(#mu#mu)",0,10000,'JPsiPT_out',nbins=100)
                                 +" & "+Hlt2MonitorMinMax( "MINTREE('mu-'==ABSID,PT)","MINTREE(mu-==ABSID,PT)",0,10000,'MuPT_out',nbins=100)
                                 +" & "+Hlt2MonitorMinMax( "VFASPF(VCHI2/VDOF)","VFASPF(VCHI2/VDOF)",0,25,'JPsiVeterxChi2_out',nbins=100)                           
            
                                 }
                    , postscale = self.postscale
                  
            
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DiMuonUnbiasedJPsiDecision":  50201 } )
        

        #--------------------------------------------
        '''
        unbiased Psi(2S)
        '''
        line.clone( 'DiMuonUnbiasedPsi2S'
                    , prescale = self.prescale 
                    , Filter = { 'Code': "(ADMASS(3686.09*MeV)<%(UnbiasedPsi2SMassWindow)s*MeV) "\
                                 "& (PT>%(UnbiasedPsi2SPt)s*MeV) "\
                                 "& (MINTREE('mu-'==ABSID,PT)>%(UnbiasedPsi2SMuPt)s*MeV) "\
                                 "& (VFASPF(VCHI2/VDOF)<%(UnbiasedPsi2SVertexChi2)s )" %  self.getProps() }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DiMuonUnbiasedPsi2SDecision": 50202 } )
        
        #--------------------------------------------
        '''
        unbiased Bmm
        '''
        line.clone( 'DiMuonUnbiasedBmm'
                    , prescale = self.prescale 
                    , Filter = { 'Code': "(MM>%(UnbiasedBmmMinMass)s*MeV) "\
                                 "& (VFASPF(VCHI2/VDOF)<%(UnbiasedBmmVertexChi2)s )"  %  self.getProps() }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DiMuonUnbiasedBmmDecision":   50203 } )
        
        #--------------------------------------------
        '''
        prescaled unbiased J/psi for low rate scenario
        '''
        line.clone( 'DiMuonUnbiasedJPsiLow'
                    , prescale = self.prescale
                    , Filter = { 'Code': "(ADMASS('J/psi(1S)')<%(UnbiasedJPsiMassWindow)s*MeV) "\
                                 "& (PT>%(UnbiasedJPsiPt)s*MeV) "\
                                 "& (MINTREE('mu-'==ABSID,PT)>%(UnbiasedJPsiMuPt)s*MeV) "\
                                 "& (VFASPF(VCHI2/VDOF)<%(UnbiasedJPsiVertexChi2)s )"% self.getProps() }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DiMuonUnbiasedJPsiLowDecision":  50204 } )
        
        #--------------------------------------------
        '''
        unbiased Zmm
        '''
        line.clone( 'DiMuonUnbiasedZmm'
                    , prescale = self.prescale 
                    , Filter = { 'Code': "(MM>%(UnbiasedZmmMinMass)s*MeV) "\
                                 "& (PT>%(UnbiasedZmmPt)s*MeV) "% self.getProps() }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DiMuonUnbiasedZmmDecision":   50205 } )
        #--------------------------------------------
        '''
        unbiased Drell-Yan 1
        '''
        line.clone( 'DiMuonDY1'
                    , prescale = self.prescale 
                    , Filter = { 'Code': "  (MM>%(DY1MinMass)s*MeV) "\
                                         "& (MM<%(DY1MaxMass)s*MeV) "\
                                         "& (PT>%(DYPt)s*MeV) " % self.getProps() }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DiMuonDY1Decision":   50206 } )

        #--------------------------------------------
        '''
        unbiased Drell-Yan 2
        '''
        line.clone( 'DiMuonDY2'
                    , prescale = self.prescale 
                    , Filter = { 'Code': "  (MM>%(DY2MinMass)s*MeV) "\
                                         "& (MM<%(DY2MaxMass)s*MeV) "\
                                         "& (PT>%(DYPt)s*MeV) " % self.getProps() }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DiMuonDY2Decision":   50207 } )

        #----------------------------------------------------------------------------------------

        '''
        unbiased Drell-Yan 3
        '''
        line.clone( 'DiMuonDY3'
                    , prescale = self.prescale 
                    , Filter = { 'Code': "  (MM>%(DY3MinMass)s*MeV) "\
                                         "& (MM<%(DY3MaxMass)s*MeV) "\
                                         "& (PT>%(DYPt)s*MeV) " % self.getProps() }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DiMuonDY3Decision":   50208 } )

        #--------------------------------------------
        '''
        unbiased Drell-Yan 4
        '''
        line.clone( 'DiMuonDY4'
                    , prescale = self.prescale 
                    , Filter = { 'Code': "  (MM>%(DY4MinMass)s*MeV) "\
                                         "& (MM<%(DY4MaxMass)s*MeV) "\
                                         "& (PT>%(DYPt)s*MeV) " % self.getProps() }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2DiMuonDY4Decision":   50209 } )


        
        #----------------------------------------------------------------------------------------
        
        '''
        Biased DiMuon Lines
               Leandro de Paula - leandro.de.paula@cern.ch
 
        There are in total 4 biased selections:

        Two selections try to reduce the rate without significan drop in signal efficiency.
        At nominal conditions they should be prescaled:

        5) Simple Biased DiMuon: first step of DiMuon selection
        6) Refined Biased DiMuon: use all variables

        and

        7) High Mass Biased DiMuon selection 
        8) Biased DiMuon selection with high IP cuts
        '''
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2BiasedDiMuonSimpleDecision"  : 50040 } )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2BiasedDiMuonRefinedDecision" : 50041 } )
        
        
        '''
           cut definitions for biased DiMuon Lines
        '''
        MuPtCut = "( MAXTREE(ABSID=='mu+',PT)>%(BiasedSingleMuonPt)s*MeV)" % self.getProps()
        MassCut  = "(MM>%(BiasedMass)s*MeV)" % self.getProps()
        MassLCut = "(MM>%(BiasedLMass)s*MeV)" % self.getProps()
        MassTCut = "(MM>%(BiasedTMass)s*MeV)" % self.getProps()
        MuIPCut  = "(2==NINTREE((ABSID=='mu+') & (MIPDV(PRIMARY)>%(BiasedSingleMuonIP)s*mm)))" % self.getProps()
        MuTIPCut = "(2==NINTREE((ABSID=='mu+') & (MIPDV(PRIMARY)>%(BiasedSingleMuonTIP)s*mm)))" % self.getProps()
        LTimeCut  = "(BPVLTIME('PropertimeFitter/properTime:PUBLIC')>%(BiasedLTime)s*ps)" % self.getProps()
        LTimeTCut = "(BPVLTIME('PropertimeFitter/properTime:PUBLIC')>%(BiasedLTimeT)s*ps)" % self.getProps()
        IPChi2Cut  = "( MAXTREE(ABSID=='mu+',MIPCHI2DV(PRIMARY))>%(BiasedSingleIPChi2)s)" % self.getProps()
        TIPChi2Cut = "( MAXTREE(ABSID=='mu+',MIPCHI2DV(PRIMARY))>%(BiasedSingleIPTChi2)s)" % self.getProps()
        VertexChi2Cut = "(VFASPF(VCHI2/VDOF)<%(BiasedVertexChi2)s)" % self.getProps()
        PVDistChi2Cut  = "(BPVVDCHI2>%(BiasedPVDistanceChi2)s)" % self.getProps()
        PVDistTChi2Cut = "(BPVVDCHI2>%(BiasedPVDistanceTChi2)s)" % self.getProps()
        '''
           sequence definitions for biased DiMuon Lines
        '''
        SimpleDiMuon = Hlt2Member( FilterDesktop
                                   , "SimpleDiMuon"          
                                   , InputLocations = [ DiMuon ]
                                   , Code = MuPtCut +"&"+ MassCut +"&"+ MuIPCut +"&"+ LTimeCut
                                   , PreMonitor = 
                                   Hlt2MonitorMinMax( "M","M(#mu#mu)",0,6000,'M_in',nbins=25) 
                                   +" & "+Hlt2MonitorMinMax( "MINTREE('mu-'==ABSID,PT)","Pt(#mu)Min",0,10000,'MuPT_in',nbins=100)
                                   +" & "+Hlt2MonitorMinMax( "PT","PT(#mu#mu)",0,10000,'JPsiPT_in',nbins=100)
                                   +" & "+Hlt2MonitorMinMax( "MIPDV(PRIMARY)","IP(#mu)",0,.5,'MuIP_in',nbins=100)
                                   +" & "+Hlt2MonitorMinMax( "BPVLTIME('PropertimeFitter/properTime:PUBLIC')","Lifetime",0,.002,'LifeTime_in',nbins=40)
                                   , PostMonitor =  
                                   Hlt2MonitorMinMax( "M","M(#mu#mu)",0,6000,'M_out',nbins=25) 
                                   +" & "+Hlt2MonitorMinMax( "MINTREE('mu-'==ABSID,PT)","Pt(#mu)Min",0,10000,'MuPT_out',nbins=100)
                                   +" & "+Hlt2MonitorMinMax( "PT","PT(#mu#mu)",0,10000,'JPsiPT_out',nbins=100)
                                   +" & "+Hlt2MonitorMinMax( "MIPDV(PRIMARY)","IP(#mu)",0,.5,'MuIP_out',nbins=100)
                                   +" & "+Hlt2MonitorMinMax( "BPVLTIME('PropertimeFitter/properTime:PUBLIC')","Lifetime",0,.002,'LifeTime_out',nbins=40)
                                   )
        RefinedDiMuon = Hlt2Member( FilterDesktop
                                    , "RefinedDiMuon"          
                                    , InputLocations = [ SimpleDiMuon ]
                                    , Code = IPChi2Cut +"&"+ VertexChi2Cut +"&"+ PVDistChi2Cut
                                    , PreMonitor = 
                                    Hlt2MonitorMinMax( "MAXTREE(ABSID=='mu+',MIPCHI2DV(PRIMARY))","#chi^{2}(IP#mu)",0,200,'IP_Chi2_in',nbins=100)
                                    +" & "+Hlt2MonitorMinMax( "VFASPF(VCHI2/VDOF)","#chi^{2}/DoF(Vertex)",0,25,'Veter_Chi2_in',nbins=100)
                                    +" & "+Hlt2MonitorMinMax( "BPVVDCHI2","#chi^{2}(Dist)",0,500,'Flight_DistanceChi2_in',nbins=100)
                                    , PostMonitor =  
                                    Hlt2MonitorMinMax( "M","M(#mu#mu)",0,6000,'M_out',nbins=25) 
                                    +" & "+Hlt2MonitorMinMax( "MAXTREE(ABSID=='mu+',MIPCHI2DV(PRIMARY))","#chi^{2}(#muIP)",0,200,'IP_Chi2_out',nbins=100)
                                    +" & "+Hlt2MonitorMinMax( "VFASPF(VCHI2/VDOF)","#chi^{2}/DoF(Vertex)",0,25,'Veter_Chi2_out',nbins=100)
                                    +" & "+Hlt2MonitorMinMax( "BPVVDCHI2","#chi^{2}(Dist)",0,500,'Flight_DistanceChi2_out',nbins=100)

                                  )
        #--------------------------------------------
        '''
            simple biased DiMuon 
        '''
        line = Hlt2Line('BiasedDiMuonSimple'
                        , prescale = self.prescale 
                        , algos = [ PV3D(), DiMuon, SimpleDiMuon ]
                        , postscale = self.postscale
                        )

        #--------------------------------------------
        '''
            refined biased DiMuon
        '''
        line = Hlt2Line('BiasedDiMuonRefined'
                        , prescale = self.prescale 
                        , algos = [ PV3D(), DiMuon, SimpleDiMuon, RefinedDiMuon ]
                        , postscale = self.postscale
                        )

        #--------------------------------------------
        '''
            high mass biased DiMuon
        '''
        line.clone( 'BiasedDiMuonMass'
                    , prescale = self.prescale 
                    , SimpleDiMuon = {"Code" : MuPtCut +"&"+ MassTCut +"&"+ MuIPCut +"&"+ LTimeTCut
                                      }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2BiasedDiMuonMassDecision"    : 50042 } )
        
        
        #--------------------------------------------
        '''
            biased DiMuon with tigh IP cuts
        '''
        line.clone( 'BiasedDiMuonIP'
                    , prescale = self.prescale
                    , SimpleDiMuon = {"Code" : MuPtCut +"&"+ MassLCut +"&"+ MuTIPCut +"&"+ LTimeCut }
                    , RefinedDiMuon = {"Code" : TIPChi2Cut +"&"+ VertexChi2Cut +"&"+ PVDistTChi2Cut }
                    , postscale = self.postscale
                    )
        HltANNSvc().Hlt2SelectionID.update( { "Hlt2BiasedDiMuonIPDecision"      : 50043 } )
        
        
