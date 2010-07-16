__author__ = 'Patrick Koppenburg, Rob Lambert, Mitesh Patel'
__date__ = '21/01/2009'
__version__ = '$Revision: 1.27 $'

"""
Bd->K*MuMu selections 
"""
from Gaudi.Configuration import *
from LHCbKernel.Configuration import *

class StrippingBd2KstarMuMuConf(LHCbConfigurableUser):
    """
    Definition of B->MuMuK* stripping

    @todo Now only new (low lumi) cuts in slots. The nominal lumi stripping lines
    by Rob Lambert are hard-coded.
    """
    
    __slots__ = { 
# standard selection
                   'BMassLow'           : 5050       # MeV, low mass cut
                ,  'BMassHighWin'       :  500       # MeV, high mass window
                ,  'BMassMedWin'        :  300       # MeV, high mass window
                ,  'BMassLowWin'        :  150       # MeV, high mass window
                ,  'BDIRA'              : 0.9999     # adimentional
                ,  'BFlightCHI2'        : 16         # adimentional 
                ,  'BFlightCHI2Tight'   : 100        # adimentional 
                ,  'BIPCHI2'            : 64         # adimentional  
                ,  'BIPCHI2Tight'       : 36         # adimentional  
                ,  'BVertexCHI2'        : 36         # adimentional
                ,  'BVertexCHI2Tight'   : 25         # adimentional
                ,  'KstarHighMass'      : 2500       # MeV
                ,  'IntDIRA'            : -0.95      # adimentional
                ,  'IntVertexCHI2Tight' : 36         # adimentional
                ,  'IntFlightCHI2'      : 9          # adimentional
                ,  'TrackChi2'          : 10         # adimentional
# simple selection
                ,  'SimpleDiMuonPT'      : 0         # MeV
                ,  'SimpleBdLT'          : 0.01      # unit ?
# D -> K pi pi pi selection for calibration

                ,  'DKpipipiTrackCHI2'   : 6         # dimensionless
                ,  'DKpipipiTrackIPCHI2' : 9         # dimensionless
                ,  'DKpipipiTightTrackIPCHI2' : 16 #dimensionless
                ,  'DKpipipiTrackP'      : 1000.0    # MeV
                ,  'DKpipipiTightTrackP' : 3000.0    # MeV
                ,  'DKpipipiTrackPT'     : 200.0     # MeV
                ,  'DKpipipiFDCHI2'      : 400        # dimensionless
                ,  'DKpipipiDIRA'        : 0.99995   # dimensionless
                ,  'DKpipipiVertexCHI2'  : 16        # per degree of freedom, dimensionless
                ,  'DKpipipiIPCHI2'      : 64        # dimensionless
                ,  'DKpipipiMassHighWin' : 150       # MeV
                ,  'DKpipipiMassLowWin'  : -75       # MeV
                ,  'DKpipipiRequiresMB'  : False
                   
                   }
                   
    _line_for_nominal_high = None



####################################################################################################
# Mitesh's selections. Stripping workshop December 2009
#
#####
# The signal line
#
    def _Early_DiMuonFilter(self):
        """
        Filtered DiMuon
        """
        from Configurables import FilterDesktop
        Early_loose_DiMuon = FilterDesktop("DiMuon_For_Early_Bd2KstarMuMu", InputLocations = [ "StdVeryLooseDiMuon" ] )
        Early_loose_DiMuon.Code = "(BPVDIRA> %(IntDIRA)s ) & (2 == NINTREE((ABSID=='mu-') & (TRCHI2DOF< %(TrackChi2)s )))" % self.getProps()
        return Early_loose_DiMuon
        
    def _Early_DiMuon(self):
        """
        The bindMembers for the DiMuon
        """
        from StrippingConf.StrippingLine import bindMembers
        from CommonParticles.StdVeryLooseDiMuon import StdVeryLooseDiMuon
        return bindMembers( None , [ StdVeryLooseDiMuon, self._Early_DiMuonFilter() ] )
        
    def _Early_Bd(self):
        """
        Common Bd for Eraly data selections
        
        @todo These should be coded in a smarter way.
        """
        from Configurables import CombineParticles
        Early_loose_Bd = CombineParticles("Early_Signal_Bd2KstarMuMu")
        Early_loose_Bd.InputLocations = ["DiMuon_For_Early_Bd2KstarMuMu", "StdVeryLooseDetachedKst2Kpi"]
        Early_loose_Bd.DecayDescriptor = "[B0 -> K*(892)0 J/psi(1S)]cc"
        Early_loose_Bd.DaughtersCuts = {
            'K*(892)0':  "(BPVDIRA> %(IntDIRA)s ) & (INTREE((ABSID=='pi+') & (TRCHI2DOF< %(TrackChi2)s ))) & (INTREE((ABSID=='K+') & (TRCHI2DOF< %(TrackChi2)s )))" % self.getProps() 
            }
        Early_loose_Bd.CombinationCut = "(AM > %(BMassLow)s *MeV) & (ADAMASS('B0') < %(BMassHighWin)s *MeV)"  % self.getProps()
        Early_loose_Bd.MotherCut = "(VFASPF(VCHI2/VDOF) < %(BVertexCHI2)s ) & (BPVDIRA> %(BDIRA)s ) & (BPVVDCHI2 > %(BFlightCHI2)s ) & (BPVIPCHI2() < %(BIPCHI2)s )"  % self.getProps()
        return Early_loose_Bd


    def Early_SignalLine(self):
        """
        The Signal line
        """
        from StrippingConf.StrippingLine import StrippingLine      
        from PhysSelPython.Wrappers import DataOnDemand
        from CommonParticles.StdVeryLooseDetachedKstar import StdVeryLooseDetachedKst2Kpi
        return StrippingLine('Bd2KstarMuMu_Early_Signal'
                             , prescale = 1
                             , algos = [ self._Early_DiMuon(),
                                         StdVeryLooseDetachedKst2Kpi,
                                         self._Early_Bd() ]
                             )

#####
# The same sign line
#
    def _Early_SameSignMuMu(self):
        """
        The MuMu algorithm for same sign dimuons: clone of standard DiMuon
        """
        from CommonParticles.StdVeryLooseDiMuon import StdVeryLooseDiMuon
        from StrippingConf.StrippingLine import bindMembers
        # this violates charge
        mm = StdVeryLooseDiMuon.clone("ClonedVeryLooseSameSignDiMuon",
                                      DecayDescriptor = "[J/psi(1S) -> mu+ mu+]cc")
        fi = self._Early_DiMuonFilter().clone("SameSignDiMuon_For_Early_Bd2KstarMuMu",
                                              InputLocations = [ "ClonedVeryLooseSameSignDiMuon" ])
        return bindMembers( None, [ mm, fi ] )
        
        
    def _Early_SameSignBd(self):
        """
        The Bd algorithm for same sign dimuons : clone of Bd algorithm
        """
        return self._Early_Bd().clone("Early_SameSign_Bd2KstarMuMu",
                                      InputLocations = ["SameSignDiMuon_For_Early_Bd2KstarMuMu",
                                                        "StdVeryLooseDetachedKst2Kpi"])

    def Early_SameSignLine(self):
        """
        The same sign line
        """
        from StrippingConf.StrippingLine import StrippingLine      
        from CommonParticles.StdVeryLooseDetachedKstar import StdVeryLooseDetachedKst2Kpi
        return StrippingLine('Bd2KstarMuMu_Early_SameSign'
                             , prescale = 1
                             , algos = [ self._Early_SameSignMuMu(),
                                         StdVeryLooseDetachedKst2Kpi,
                                         self._Early_SameSignBd()]
                             )
    
#####
# That will be useful later
#
    def _tightMotherCuts(self):
        return "(VFASPF(VCHI2/VDOF) < %(BVertexCHI2Tight)s ) & (BPVDIRA> %(BDIRA)s ) & (BPVVDCHI2 > %(BFlightCHI2Tight)s ) & (BPVIPCHI2() < %(BIPCHI2Tight)s )"  % self.getProps()
#####
# The wide K* mass line
#
    def _Early_WideKstar(self):
        """
        The Wide K*
        """
        from CommonParticles.StdVeryLooseDetachedKstar import StdVeryLooseDetachedKst2Kpi
        return StdVeryLooseDetachedKst2Kpi.clone("WideKstarForBd2KstarMuMu",
                                                 CombinationCut = "AM < %(KstarHighMass)s" % self.getProps(),
                                                 MotherCut = "(VFASPF(VCHI2/VDOF) < %(IntVertexCHI2Tight)s ) & ( BPVVDCHI2 > %(IntFlightCHI2)s )" % self.getProps() ) 
     
    def _Early_WideKstarBd(self):
        """
        The Bd with a wide K*
        """
        algo = self._Early_Bd().clone("Early_WideKstar_Bd2KstarMuMu",
                                      InputLocations = ["DiMuon_For_Early_Bd2KstarMuMu",
                                                        "WideKstarForBd2KstarMuMu"])
        jcuts = "(VFASPF(VCHI2/VDOF) < %(IntVertexCHI2Tight)s )" % self.getProps()
        kcuts = algo.DaughtersCuts['K*(892)0']
        kcuts = kcuts+" & (VFASPF(VCHI2/VDOF) < %(IntVertexCHI2Tight)s )" % self.getProps()
        algo.DaughtersCuts = { 'J/psi(1S)' : jcuts,
                               'K*(892)0' : kcuts }
        algo.CombinationCut = "(ADAMASS('B0') < %(BMassMedWin)s *MeV)"  % self.getProps()
        algo.MotherCut = self._tightMotherCuts()
        return algo
    
        
    def Early_WideKstarLine(self):
        """
        The Wide K* line
        """
        from StrippingConf.StrippingLine import StrippingLine      
        return StrippingLine('Bd2KstarMuMu_Early_WideKstar'
                             , prescale = 1
                             , algos = [ self._Early_DiMuon() ,
                                         self._Early_WideKstar(),
                                         self._Early_WideKstarBd() ]
                             )

#####
# The no muon-ID line

    def _Early_NoMuIDMuMu(self):
        """
        The MuMu algorithm for no MuID dimuons: clone of standard DiMuon
        """
        from CommonParticles.StdVeryLooseDiMuon import StdVeryLooseDiMuon
        
        # this violates charge
        return StdVeryLooseDiMuon.clone("NoMuIDDiMuonForBd2KstarMuMu",  
                                    InputLocations = ["StdVeryLooseMuons",
                                                      "StdNoPIDsPions"],
                                    DecayDescriptor = "[J/psi(1S) -> pi+ mu-]cc" ,
                                    MotherCut = "(VFASPF(VCHI2/VDOF)<36)" )
        
    def _Early_NoMuIDBd(self):
        """
        The Bd algorithm for no MuID dimuons : clone of Bd algorithm
        """
        algo = self._Early_WideKstarBd().clone("Early_NoMuID_Bd2KstarMuMu",
                                               InputLocations = ["NoMuIDDiMuonForBd2KstarMuMu",
                                                                 "StdVeryLooseDetachedKst2Kpi"])
        algo.DaughtersCuts['J/psi(1S)'] =  "(BPVDIRA> %(IntDIRA)s ) & (INTREE((ABSID=='mu-') & (TRCHI2DOF< %(TrackChi2)s ))) & (INTREE((ABSID=='pi+') & (TRCHI2DOF< %(TrackChi2)s ))) & ( BPVVDCHI2 > %(IntFlightCHI2)s )" % self.getProps()
        algo.CombinationCut = "(ADAMASS('B0') < %(BMassLowWin)s *MeV)"  % self.getProps()
        algo.MotherCut = self._tightMotherCuts()
        return algo

    def Early_NoMuIDLine(self):
        """
        The no MuID line
        """
        from StrippingConf.StrippingLine import StrippingLine      
        from CommonParticles.StdVeryLooseDetachedKstar import StdVeryLooseDetachedKst2Kpi
        return StrippingLine('Bd2KstarMuMu_Early_NoMuID'
                             , prescale = 0.2   # lower prescale
                             , algos = [ self._Early_NoMuIDMuMu(),
                                         StdVeryLooseDetachedKst2Kpi,
                                         self._Early_NoMuIDBd()]
                             )
    
#####
# The e-mu line

    def _Early_eMu(self):
        """
        The MuMu algorithm for eMu: clone of NoMuIDMuMu
        """
        
        # this violates charge
        return self._Early_NoMuIDMuMu().clone("MuonEleForBd2KstarMuMu",
                                    InputLocations = ["StdVeryLooseMuons",
                                                      "StdLooseElectrons"],
                                    DecayDescriptor = "[J/psi(1S) -> e+ mu-]cc")
        
    def _Early_eMuBd(self):
        """
        The Bd algorithm for eMu : clone of Bd algorithm
        """
        algo = self._Early_Bd().clone("Early_eMu_Bd2KstarMuMu",
                                      InputLocations = ["MuonEleForBd2KstarMuMu",
                                                        "StdVeryLooseDetachedKst2Kpi"])
        algo.DaughtersCuts['J/psi(1S)'] = "(BPVDIRA> %(IntDIRA)s ) & (INTREE((ABSID=='mu-') & (TRCHI2DOF< %(TrackChi2)s ))) & (INTREE((ABSID=='e+') & (TRCHI2DOF< %(TrackChi2)s ))) & ( BPVVDCHI2 > %(IntFlightCHI2)s )"  % self.getProps()
        algo.MotherCut = self._tightMotherCuts()
        
        return algo

    def Early_eMuLine(self):
        """
        The eMu line
        """
        from StrippingConf.StrippingLine import StrippingLine      
        from CommonParticles.StdVeryLooseDetachedKstar import StdVeryLooseDetachedKst2Kpi
        return StrippingLine('Bd2KstarMuMu_Early_eMu'
                             , prescale = 1
                             , algos = [ self._Early_eMu(),
                                         StdVeryLooseDetachedKst2Kpi,
                                         self._Early_eMuBd()]
                             )

####################################################################################################
# Only the dimuon. Will be prescaled
#
    def Early_DiMuonLine(self):
        """
        Just the dimuon

        This line will need serious prescaling as soon as we get some non-0 luminosity
        """
        from StrippingConf.StrippingLine import StrippingLine      
        from PhysSelPython.Wrappers import DataOnDemand
        from CommonParticles.StdVeryLooseDiMuon import StdVeryLooseDiMuon
        return StrippingLine('Bd2KstarMuMu_Early_DiMuonOnly'
                             , prescale = 1
                             , algos = [ self._Early_DiMuon() ]
                             )


####################################################################################################
#
# Simplest line based on LT
#
    def simplestDiMuon( self, trail="" ):
        """
        Very simple line based on ONE cut!
        
        @author P. Koppenburg
        @date 25/2/2010
        """
        from PhysSelPython.Wrappers import Selection, DataOnDemand
        from StrippingConf.StrippingLine import StrippingLine
        from Configurables import FilterDesktop, CombineParticles
        
	_muons =  DataOnDemand(Location = 'Phys/StdVeryLooseDiMuon')

        _diMu = FilterDesktop("FilterForSimpleBd2KstarMuMu")
        _diMu.Code = "(PT > %(SimpleDiMuonPT)s *MeV)" % self.getProps()
        _sd = Selection("SelFilterForSimpleBd2KstarMuMu"+trail,
                       Algorithm = _diMu,
                       RequiredSelections = [ _muons ] )

        return _sd

        
    def simplestCombineLT( self ):
        """
        Very simple line based on ONE cut!
        
        @author P. Koppenburg
        @date 25/2/2010
        """
        from Configurables import CombineParticles
        _comb = CombineParticles("SimpleBd2KstarMuMuLT",
                                 DecayDescriptor = "[B0 -> J/psi(1S) K*(892)0 ]cc" ,
                                 CombinationCut = "(ADAMASS('B0') < %(BMassMedWin)s *MeV)"  % self.getProps(),
                                 MotherCut = "(VFASPF(VCHI2/VDOF) < %(IntVertexCHI2Tight)s ) & ( BPVLTIME() > %(SimpleBdLT)s ) & (ADMASS('B0') < %(BMassMedWin)s *MeV)" % self.getProps() )
        return _comb 
        
       
    def simplestLTLine( self ):
        """
        Very simple line based on ONE cut! : the lifetime
        
        @author P. Koppenburg
        @date 25/2/2010
        """
        from StrippingConf.StrippingLine import StrippingLine
        from PhysSelPython.Wrappers import Selection, DataOnDemand

 	_kstar =  DataOnDemand(Location = 'Phys/StdVeryLooseDetachedKst2Kpi')
        _comb = self.simplestCombineLT()
        _sb = Selection("SelSimpleBd2KstarMuMuLT",
                       Algorithm = _comb,
                       RequiredSelections = [ self.simplestDiMuon("LT"), _kstar ] )
	return StrippingLine('SimpleBd2KstarMuMuLT', prescale = 1, algos = [ _sb ])   
#
#
####################################################################################################
# Rob's selections. See RDWG talk, 08.07.2009, Rob Lambert
#
    def _Strip_loose_Bd2KstarMuMu(self):
        """
        CombineParticles for Rob's loose B->MuMuK*. This is the common loosest set of cuts.
        """
        from Configurables import CombineParticles
        
        Strip_loose_Bd2KstarMuMu = CombineParticles("Strip_loose_Bd2KstarMuMu")
        
        Strip_loose_Bd2KstarMuMu.InputLocations = ["StdVeryLooseDiMuon", "StdLooseDetachedKst2Kpi"]
        Strip_loose_Bd2KstarMuMu.DecayDescriptor = "[B0 -> K*(892)0 J/psi(1S)]cc"
        Strip_loose_Bd2KstarMuMu.DaughtersCuts = {
            'K*(892)0':  ("(BPVVDZ>-50*mm) & "+
                          "(INTREE((ABSID=='pi+') & ((PIDpi-PIDmu)>-10) & ((PIDpi-PIDK)>-10))) &"+
                          "(INTREE((ABSID=='K+') & ((PIDK-PIDmu)>-10) & ((PIDK-PIDpi)>-5)))" ),
            'J/psi(1S)' : "(2 == NINTREE((ABSID=='mu+') & ((PIDmu-PIDpi)>-10) & ((PIDmu-PIDK)>-10) & (MIPCHI2DV(PRIMARY)>0)) )"
            }
        Strip_loose_Bd2KstarMuMu.CombinationCut = "(ADAMASS('B0')<300*MeV)"
        Strip_loose_Bd2KstarMuMu.MotherCut=( "(MIPDV(PRIMARY)<0.08*mm) & (MIPCHI2DV(PRIMARY) < 150) & (PT>300*MeV) & (BPVDIRA>0.9998) & "+
                                             "(VFASPF(VCHI2/VDOF)<64) & (DMASS('B0')<250*MeV) & (DMASS('B0')>-150*MeV)"
                                             )

        return Strip_loose_Bd2KstarMuMu


    def _ForNominal_Bd2KstarMuMu_High(self):
        """
        Loosest filter
        """
        from Configurables import FilterDesktop
        ForNominal_Bd2KstarMuMu_High = FilterDesktop("ForNominal_Bd2KstarMuMu_High")
        ForNominal_Bd2KstarMuMu_High.InputLocations = ["Strip_loose_Bd2KstarMuMu"]
        ForNominal_Bd2KstarMuMu_High.Code=( "(INTREE((ABSID=='K+') & (TRCHI2DOF < 36))) & "+
                                        "(INTREE((ABSID=='pi+') & (TRCHI2DOF < 36))) & "+
                                        "(2 == NINTREE((ABSID=='mu+') & (MIPCHI2DV(PRIMARY)>4) & (TRCHI2DOF < 16))) & "+
                                        "(INTREE((ABSID=='K*(892)0') & (BPVVDCHI2>25) & (VFASPF(VCHI2/VDOF)<36))) & "+
                                        "(PT>300*MeV) & (BPVDIRA>0.9998) & (BPVVDCHI2>36) & (VFASPF(VCHI2/VDOF)<36) "
                                        )
        return ForNominal_Bd2KstarMuMu_High

    def _ForNominal_Bd2KstarMuMu_Med(self):
        """
        Medium filter
        """
        from Configurables import FilterDesktop
        ForNominal_Bd2KstarMuMu_Med = FilterDesktop("ForNominal_Bd2KstarMuMu_Med")
        ForNominal_Bd2KstarMuMu_Med.InputLocations = ["ForNominal_Bd2KstarMuMu_High"]
        ForNominal_Bd2KstarMuMu_Med.Code = ( "(INTREE((ABSID=='K+') & ((PIDK-PIDpi)>-5) & ((PIDK-PIDmu)>-5) & (TRCHI2DOF < 9))) & "+
                                         "(2 == NINTREE((ABSID=='mu+') & ((PIDmu-PIDpi)>-5) & (TRCHI2DOF < 9))) & "+
                                         "(INTREE((ABSID=='pi+') & (TRCHI2DOF < 16))) & "+
                                         "(INTREE((ABSID=='K*(892)0') & (BPVVDZ>-30*mm) & (VFASPF(VCHI2/VDOF)<16) & (PT>300*MeV))) & "+
                                         "(INTREE((ABSID=='J/psi(1S)') & (BPVVDCHI2>25) )) & "+
                                         "(BPVDIRA>0.9999) & (BPVVDCHI2>64) & (MIPCHI2DV(PRIMARY) < 36) & (VFASPF(VCHI2/VDOF)<16) "
                                         )
        return ForNominal_Bd2KstarMuMu_Med
    
    def _ForNominal_Bd2KstarMuMu_Low(self):
        """
        Hard filter
        """
        from Configurables import FilterDesktop
        ForNominal_Bd2KstarMuMu_Low = FilterDesktop("ForNominal_Bd2KstarMuMu_Low")
        ForNominal_Bd2KstarMuMu_Low.InputLocations = ["Strip_loose_Bd2KstarMuMu"]
        ForNominal_Bd2KstarMuMu_Low.Code = ( "(INTREE((ABSID=='K+') & ((PIDK-PIDpi)>-5) & ((PIDK-PIDmu)>-5))) & "+
                                         "(2 == NINTREE((ABSID=='mu+') & ((PIDmu-PIDpi)>-5) & (MIPDV(PRIMARY) > 0.08*mm))) & "+
                                         "(INTREE((ABSID=='K*(892)0') & (BPVVDZ>-5*mm) & (PT>1000*MeV) & (MIPDV(PRIMARY) > 0.06*mm))) & "+
                                         "(BPVDIRA>0.99995) & (VFASPF(VMINVDDV(PRIMARY))>1*mm) & (MIPDV(PRIMARY) < 0.06*mm)"
                                         )
        return ForNominal_Bd2KstarMuMu_Low
        

    def line_for_nominal_high(self):
        """
        The '10 Hz' line
        """
        
        if StrippingBd2KstarMuMuConf._line_for_nominal_high == None :
        
            from StrippingConf.StrippingLine import StrippingLine
	    StrippingBd2KstarMuMuConf._line_for_nominal_high = StrippingLine('Bd2KstarMuMu_ForNominal_High'
                             , prescale = 1
                             , algos = [ self._Strip_loose_Bd2KstarMuMu(),
                                         self._ForNominal_Bd2KstarMuMu_High() ]
                             )

        return StrippingBd2KstarMuMuConf._line_for_nominal_high

    def line_for_nominal_med(self):
        """
        The '2 Hz' line
        """
        from Configurables import FilterDesktop
        from StrippingConf.StrippingLine import StrippingLine

         
        return self.line_for_nominal_high().clone("Bd2KstarMuMu_ForNominal_Med"
                                              , algos = [ self._Strip_loose_Bd2KstarMuMu(),
                                                          self._ForNominal_Bd2KstarMuMu_High(),
                                                          self._ForNominal_Bd2KstarMuMu_Med() ]
                                              )
    
    def line_for_nominal_low(self):
        """
        The '1 Hz' line
        """
        from Configurables import FilterDesktop
        from StrippingConf.StrippingLine import StrippingLine

        return self.line_for_nominal_high().clone("Bd2KstarMuMu_ForNominal_Low"
                                             , algos = [ self._Strip_loose_Bd2KstarMuMu(),
                                                         self._ForNominal_Bd2KstarMuMu_Low() ]
                                             )


####################################################################################################
#
# Lines for D -> Kpipipi calibration, added by T.Blake, 23-05-10 
#

    def _DKpipipi_Kstar(self):
        """
        FilterDesktop on StdVeryLooseDetachedKstar for  D0 -> K- pi+ pi- pi+
        """
        from PhysSelPython.Wrappers import Selection, DataOnDemand
        from Configurables import FilterDesktop

        _kstar = DataOnDemand(Location='Phys/StdVeryLooseDetachedKst2Kpi')

        _filter = FilterDesktop('Kstar_Bd2KstarMuMu_DKpipipi')
        _filter.Code = "(VFASPF(VCHI2/VDOF) < %(DKpipipiVertexCHI2)s ) &" \
                       "(2 == NINTREE( (ISBASIC) & (P > %(DKpipipiTrackP)s ) & " \
                       "(PT > %(DKpipipiTrackPT)s ) & " \
                       "(TRCHI2DOF < %(DKpipipiTrackCHI2)s ) & " \
                       "(MIPCHI2DV(PRIMARY) > %(DKpipipiTrackIPCHI2)s )))" % self.getProps() 

        return Selection( 'Sel_Kstar_Bd2KstarMuMu_DKpipipi',
                          Algorithm=_filter,
                          RequiredSelections = [_kstar] )
        
        
    def _DKpipipi_Rho(self):
        """
        CombineParticles for pi- pi+ pair in  D0 -> K- pi+ pi- pi+
        """
        from PhysSelPython.Wrappers import Selection, DataOnDemand
        from Configurables import CombineParticles
        
        _pions = DataOnDemand(Location='Phys/StdLoosePions')

        _pionCuts = "(P > %(DKpipipiTightTrackP)s ) & " \
                    "(PT > %(DKpipipiTrackPT)s ) & " \
                    "(TRCHI2DOF < %(DKpipipiTrackCHI2)s ) & " \
                    "(MIPCHI2DV(PRIMARY) > %(DKpipipiTightTrackIPCHI2)s )" % self.getProps()

        _rho = CombineParticles('Rho_Bd2KstarMuMu_DKpipipi')
        _rho.DecayDescriptor =  "[rho(770)0 -> pi+ pi-]cc"              
        _rho.DaughtersCuts = { "pi+" : _pionCuts }
        _rho.MotherCut = "(VFASPF(VCHI2/VDOF) < %(DKpipipiVertexCHI2)s )" % self.getProps()

        return Selection( 'Sel_Rho_Bd2KstarMuMu_DKpipipi',
                          Algorithm=_rho,
                          RequiredSelections=[_pions] )
        

    def _DKpipipi_D(self):
        """
        CombineParticles for D0 -> K- pi+ pi- pi+
        """
        from Configurables import CombineParticles
        from PhysSelPython.Wrappers import Selection
    
        _DKpipipi = CombineParticles('D_Bd2KstarMuMu_DKpipipi')
        _DKpipipi.DecayDescriptor = '[D0 -> K*(892)~0 rho(770)0]cc'

        _DKpipipi.CombinationCut = "ADAMASS('D0')<200*MeV"
        
        _DKpipipi.MotherCut = "(VFASPF(VCHI2/VDOF) < %(DKpipipiVertexCHI2)s ) & " \
                              "(DMASS('D0') > %(DKpipipiMassLowWin)s *MeV) & " \
                              "(DMASS('D0') < %(DKpipipiMassHighWin)s *MeV) & " \
                              "(BPVVDCHI2 > %(DKpipipiFDCHI2)s ) & " \
                              "(BPVDIRA > %(DKpipipiDIRA)s ) & " \
                              "(MIPCHI2DV(PRIMARY) < %(DKpipipiIPCHI2)s )" % self.getProps()

        return Selection( 'Sel_D_Bd2KstarMuMu_DKpipipi',
                          Algorithm=_DKpipipi,
                          RequiredSelections=[ self._DKpipipi_Kstar(), self._DKpipipi_Rho() ] )
                         
        

    def line_for_DKpipipi(self):
        """
        Stripping line for D0 -> K- pi+ pi- pi+
        """
        from StrippingConf.StrippingLine import StrippingLine
        from PhysSelPython.Wrappers import Selection

        _line = StrippingLine( 'Line_Bd2KstarMuMu_DKpipipi',
                              prescale = 1, algos = [ self._DKpipipi_D() ] )

        if self.getProp('DKpipipiRequiresMB'):
            _line.HLT = "HLT_PASS_RE('Hlt1MB.*Decision')"
        return _line

## ####################################################################################################


#
####################################################################################################
#
#
    def getProps(self) :
        """
        From HltLinesConfigurableUser
        @todo Should be shared between Hlt and stripping
        """
        d = dict()
        for (k,v) in self.getDefaultProperties().iteritems() :
            d[k] = getattr(self,k) if hasattr(self,k) else v
        return d

