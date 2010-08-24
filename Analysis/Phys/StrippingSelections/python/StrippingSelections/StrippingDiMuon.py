# $Id: StrippingDiMuon.py,v 1.9 2010-08-24 00:35:14 jhe Exp $
## #####################################################################
# A stripping selection for inclusive J/psi(1S) -> mu+ mu- decays
#
# @authors G.Lanfranchi, A.Sarti 
# @date 2009-Sept-1
# 
## #####################################################################

from Gaudi.Configuration import *
from LHCbKernel.Configuration import *
from Configurables import FilterDesktop, CombineParticles, OfflineVertexFitter	
from PhysSelPython.Wrappers import Selection, SelectionSequence, DataOnDemand

class StrippingDiMuonConf(LHCbConfigurableUser):
    """
    Definition of Jpsi -> mu mu stripping.
    """
    __slots__ = {
			"MuonPT":		650.	# MeV
		,	"MuonPTLoose":		500.	# MeV
		,	"MuonTRCHI2":		  5.	# adimensional
		,	"MuonTRCHI2Loose":	 10.	# adimensional
		,	"JpsiAM":	       2900.	# MeV
		,	"JpsiAMLoose":	       2500.	# MeV
		,	"JpsiVCHI2":		  9.	# adimensional
		,	"JpsiVCHI2Loose":	 20.	# adimensional
                ,       "DiMuonFDCHI2" :        100.    # adimensional
                ,       "DiMuonHighPT" :       2000.    # MeV
                ,       "DiMuonHighIP" :          9.    # adimensional
                ,       "DiMuonVCHI2" :          10.    # adimensional
		}

    def nominal_line( self ):
        from StrippingConf.StrippingLine import StrippingLine
	JpsiSel = self.Jpsi()
	JpsiSeq = SelectionSequence("SeqDiMuonInc", TopSelection = JpsiSel)
	return StrippingLine('DiMuonIncLine', prescale = 1, algos = [JpsiSeq])   
     	
    def loose_line( self ):
        from StrippingConf.StrippingLine import StrippingLine
	JpsiSel = self.JpsiLoose()
	JpsiSeq = SelectionSequence("SeqDiMuonIncLoose", TopSelection = JpsiSel)
	return StrippingLine('DiMuonIncLooseLine', prescale = 1, algos = [JpsiSeq])   

    def likesign_line( self ):
        from StrippingConf.StrippingLine import StrippingLine
	JpsiSel = self.JpsiSameSign()
	JpsiSeq = SelectionSequence("SeqDiMuonSameSign", TopSelection = JpsiSel)
	return StrippingLine('DiMuonSameSignLine', prescale = 1, algos = [JpsiSeq])   

    def Jpsi( self ):
	_muons =  DataOnDemand(Location = 'Phys/StdLooseMuons')
	mucut = '(PT > %(MuonPT)s *MeV) & (TRCHI2DOF < %(MuonTRCHI2)s) & (ISLONG)' % self.getProps()
	_Jpsi = CombineParticles("DiMuonIncCombine",
                         DecayDescriptor = 'J/psi(1S) -> mu+ mu-',
                         DaughtersCuts = {'mu+': mucut}, 
                         CombinationCut = " (AM > %(JpsiAM)s *MeV)" % self.getProps(),
                         MotherCut = "(VFASPF(VCHI2/VDOF) < %(JpsiVCHI2)s)" % self.getProps(),
                         WriteP2PVRelations = False
                         )
	Jpsi = Selection("SelDiMuonInc",
                  Algorithm = _Jpsi,
                  RequiredSelections = [ _muons ]
                  )
	return Jpsi
	
    def JpsiLoose( self ):
	_muons =  DataOnDemand(Location = 'Phys/StdVeryLooseMuons')
	mucut = '(PT > %(MuonPTLoose)s *MeV) & (TRCHI2DOF < %(MuonTRCHI2Loose)s) & (ISLONG)' % self.getProps()
	_Jpsi = CombineParticles("DiMuonIncCombineLoose",
                         DecayDescriptor = 'J/psi(1S) -> mu+ mu-',
                         DaughtersCuts = {'mu+': mucut},
                         CombinationCut = " (AM > %(JpsiAMLoose)s *MeV)" % self.getProps(),
                         MotherCut = "(VFASPF(VCHI2/VDOF) < %(JpsiVCHI2Loose)s)" % self.getProps(),
                         WriteP2PVRelations = False
                         )
	Jpsi = Selection("SelDiMuonInciLoose",
                  Algorithm = _Jpsi,
                  RequiredSelections = [_muons]
                  )
	return Jpsi

    def JpsiSameSign( self ):
	_muons =  DataOnDemand(Location = 'Phys/StdVeryLooseMuons')
	mucut = '(PT > %(MuonPTLoose)s *MeV) & (TRCHI2DOF < %(MuonTRCHI2Loose)s) & (ISLONG)' % self.getProps()
	_Jpsi = CombineParticles("DiMuonSameSignLoose",
                         DecayDescriptor = '[J/psi(1S) -> mu+ mu+]cc',
                         DaughtersCuts = {'mu+': mucut},
                         CombinationCut = " (AM > %(JpsiAMLoose)s *MeV)" % self.getProps(),
                         MotherCut = "(VFASPF(VCHI2/VDOF) < %(JpsiVCHI2Loose)s)" % self.getProps(),
                         WriteP2PVRelations = False
                         )
	Jpsi = Selection("SelDiMuonSameSignLoose",
                  Algorithm = _Jpsi,
                  RequiredSelections = [_muons]
                  )
	return Jpsi

#########################################################################################
# Biased DiMuon adapted from Antonio Perez-Calero, implemented by P. Koppenburg
#
    def biasedDiMuon_line( self ):
        """
        Biased DiMuon adapted from Antonio Perez-Calero, implemented by P. Koppenburg

        Tightened on 20/7/2010
        
        @author P. Koppenburg
        @date 17/2/2010
        """
        from StrippingConf.StrippingLine import StrippingLine
	_muons =  DataOnDemand(Location = 'Phys/StdVeryLooseDiMuon')
        _diMu = FilterDesktop("FilterForB2DiMuon")
        _diMu.Code = "(MAXTREE('mu+'==ABSID,PT)>%(DiMuonHighPT)s*MeV) & (MAXTREE('mu+'==ABSID,(MIPCHI2DV(PRIMARY)))>%(DiMuonHighIP)s) & (VFASPF(VCHI2/VDOF)<%(DiMuonVCHI2)s) & (BPVVDCHI2> %(DiMuonFDCHI2)s )" % self.getProps()

        s = Selection("SelB2DiMuon",
                         Algorithm = _diMu,
                         RequiredSelections = [ _muons ] )
	ss = SelectionSequence("SeqSelB2DiMuon", TopSelection = s )
	return StrippingLine('SelB2DiMuonLine', prescale = 1, algos = [ ss ])   

#########################################################################################
    def getProps(self) :
        """
        From HltLinesConfigurableUser
        @todo Should be shared between Hlt and stripping
        """
        d = dict()
        for (k,v) in self.getDefaultProperties().iteritems() :
            d[k] = getattr(self,k) if hasattr(self,k) else v
        return d

