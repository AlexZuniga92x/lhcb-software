__author__ = 'Ramon Niet'
__date__ = '2014/10/23'

'''
Bd->JpsieeKS stripping selection

Exports the following stripping lines
- Bd2JpsieeKSBd2JpsieeKSFromTracksPrescaledLine
- Bd2JpsieeKSBd2JpsieeKSFromTracksDetachedLine
- Bd2JpsieeKSBd2JpsieeKSDetachedLine
- Bd2JpsieeKSBd2JpsieeKstarFromTracksPrescaledLine
- Bd2JpsieeKSBd2JpsieeKstarFromTracksDetachedLine
- Bd2JpsieeKSBu2JpsieeKFromTracksDetachedLine
'''

__all__ = (
    'Bd2JpsieeKSConf',
    'default_config'
    )

default_config = {
    'NAME'              : 'Bd2JpsieeKS',
    'BUILDERTYPE'       : 'Bd2JpsieeKSConf',
    'CONFIG'    : {
          'BPVLTIME'              :     0.2   # ps
        , 'ElectronPT'            :   500.    # MeV
        , 'ElectronPID'           :     0.    # adimensional
        , 'ElectronTrackCHI2pDOF' :     5.    # adimensional
        , 'TRCHI2DOF'             :     5.    # adimensional
        , 'JpsiVertexCHI2pDOF'    :    15.    # adimensional
        , 'JpsiMassMin'           :  2300.    # MeV
        , 'JpsiMassMax'           :  3300.    # MeV
        , 'BdVertexCHI2pDOF'      :    7.     # adimensional
        , 'BdMassMin'             :  4400.    # MeV
        , 'BdMassMax'             :  6000.    # MeV
        , 'Prescale'              :     0.1   # adamenssional
        , 'Bd2JpsieeKstarPrescale':     0.1   # adimensional
        , 'Bu2JpsieeKPrescale'    :     0.1   # adimensional
        },
    'STREAMS' : [ 'Radiative' ],
    'WGs'    : [ 'B2CC' ]
    }

from Gaudi.Configuration import *
from GaudiConfUtils.ConfigurableGenerators import FilterDesktop, CombineParticles
from CommonParticles.Utils import updateDoD
from StandardParticles import StdLoosePions
from StandardParticles import StdLooseKaons
from StandardParticles import StdAllLooseKaons
from PhysSelPython.Wrappers import Selection, DataOnDemand, MergedSelection
from StrippingConf.StrippingLine import StrippingLine
from StrippingUtils.Utils import LineBuilder
from GaudiKernel.SystemOfUnits import MeV


class Bd2JpsieeKSConf(LineBuilder):

    __configuration_keys__ = (
                  'BPVLTIME'                  # ps
                , 'ElectronPT'                # MeV
                , 'ElectronPID'               # adimensional
                , 'ElectronTrackCHI2pDOF'     # adimensional
                , 'TRCHI2DOF'                 # adimensional
                , 'JpsiVertexCHI2pDOF'        # adimensional
                , 'JpsiMassMin'               # MeV
                , 'JpsiMassMax'               # MeV
                , 'BdVertexCHI2pDOF'          # adimensional
                , 'BdMassMin'                 # MeV
                , 'BdMassMax'                 # MeV
                , 'Prescale'                  # adimensional
                , 'Bd2JpsieeKstarPrescale'    # adimensional
                , 'Bu2JpsieeKPrescale'        # adimensional
                )

    def __init__(self, name, config) :
        LineBuilder.__init__(self, name, config)

        # if name not set outside, set it to empty 
        if name == None:
            name = ""        
        
        self.name   = name
        self.config = config

        DiElectrons           = DataOnDemand( Location = "Phys/StdLooseDiElectron/Particles" )
        DiElectronsFromTracks = DataOnDemand( Location = "Phys/StdDiElectronFromTracks/Particles" )

        self.NoIPKaonList = self.createSubSel( OutputList = "NoIPKaonsForBetaS" + self.name,
                                               InputList = DataOnDemand(Location = "Phys/StdAllLooseKaons/Particles"),
                                               Cuts = "(TRCHI2DOF < %(TRCHI2DOF)s ) & (PIDK > 0)" % self.config )

        self.KsListLoose = MergedSelection( "StdLooseKsMergedForBetaS" + self.name,
                                            RequiredSelections = [DataOnDemand(Location = "Phys/StdLooseKsDD/Particles"),
                                                                  DataOnDemand(Location = "Phys/StdLooseKsLL/Particles")] )
        self.KSList = self.createSubSel( OutputList = "KsForBetaS" + self.name,
                                InputList = self.KsListLoose,
                                Cuts = "(VFASPF(VCHI2)<20) & (BPVDLS>5)" )

        self.KstarList = self.createSubSel( OutputList = "Kstar2KpiForBetaS" + self.name,
                                            InputList = DataOnDemand(Location = "Phys/StdLooseKstar2Kpi/Particles"),
                                            Cuts = "(in_range(826,M,966))" \
                                            "& (PT > 1300.*MeV) " \
                                            "& (VFASPF(VCHI2) < 25)" \
                                            "& (MAXTREE('K+'==ABSID,  TRCHI2DOF) < %(TRCHI2DOF)s )" \
                                            "& (MAXTREE('pi-'==ABSID, TRCHI2DOF) < %(TRCHI2DOF)s )" \
                                            "& (MINTREE('K+'==ABSID, PIDK) > 0)" % self.config)

        self._jpsi = FilterDesktop( Code = "   (MM > %(JpsiMassMin)s *MeV)" \
                                           " & (MM < %(JpsiMassMax)s *MeV)" \
                                           " & (MINTREE('e+'==ABSID,PIDe-PIDpi) > %(ElectronPID)s )" \
                                           " & (MINTREE('e+'==ABSID,PT) > %(ElectronPT)s *MeV)" \
                                           " & (MAXTREE('e+'==ABSID,TRCHI2DOF) < %(ElectronTrackCHI2pDOF)s)" \
                                           " & (VFASPF(VCHI2/VDOF) < %(JpsiVertexCHI2pDOF)s)" % self.config )

        self.Jpsi = Selection( "SelJpsi2eeFor" + self.name,
                               Algorithm = self._jpsi,
                               RequiredSelections = [DiElectrons] )
    

        self.JpsiFromTracks = Selection( "SelJpsi2eeFromTracksFor" + self.name,
                                         Algorithm = self._jpsi,
                                         RequiredSelections = [DiElectronsFromTracks] )

        self.makeBd2JpsieeKS()
        self.makeBd2JpsieeKstar()
        self.makeBu2JpsieeK()

        # Tests CPU time required for construction of StdLooseDiElectron
        # self.DielectronTestLine      = self._DielectronTestLine( DiElectrons, "DielectronTest", config )
        # self.registerLine( self.DielectronTestLine )


    def createSubSel( self, OutputList, InputList, Cuts ) :
        '''create a selection using a FilterDesktop'''
        filter = FilterDesktop(Code = Cuts)
        return Selection( OutputList,
                      Algorithm = filter,
                      RequiredSelections = [ InputList ] )

    def createCombinationSel( self, OutputList,
                          DecayDescriptor,
                          DaughterLists,
                          DaughterCuts = {} ,
                          PreVertexCuts = "ALL",
                          PostVertexCuts = "ALL",
                          ReFitPVs = True ) :
        '''create a selection using a ParticleCombiner with a single decay descriptor'''
        combiner = CombineParticles( DecayDescriptor = DecayDescriptor,
                                     DaughtersCuts = DaughterCuts,
                                     MotherCut = PostVertexCuts,
                                     CombinationCut = PreVertexCuts,
                                     ReFitPVs = ReFitPVs)
        return Selection ( OutputList,
                           Algorithm = combiner,
                           RequiredSelections = DaughterLists)


    def makeBd2JpsieeKS( self ) :
        Bd2JpsieeKS = self.createCombinationSel( OutputList = "Bd2JpsiKS",
                                                 DecayDescriptor = "B0 -> J/psi(1S) KS0",
                                                 DaughterLists  = [ self.Jpsi, self.KSList ],
                                                 PreVertexCuts = "in_range(%(BdMassMin)s,AM,%(BdMassMax)s)" % self.config,
                                                 PostVertexCuts = "(VFASPF(VCHI2/VDOF) < %(BdVertexCHI2pDOF)s)" % self.config )
        
        Bd2JpsieeKSFromTracks = self.createCombinationSel( OutputList = "Bd2JpsiKSFromTracks",
                                                           DecayDescriptor = "B0 -> J/psi(1S) KS0",
                                                           DaughterLists  = [ self.JpsiFromTracks, self.KSList ],
                                                           PreVertexCuts = "in_range(%(BdMassMin)s,AM,%(BdMassMax)s)" % self.config,
                                                           PostVertexCuts = "(VFASPF(VCHI2/VDOF) < %(BdVertexCHI2pDOF)s)" % self.config )

        Bd2JpsieeKSFromTracksPrescaledLine = StrippingLine( self.name + "Bd2JpsieeKSFromTracksPrescaledLine",
                                                            prescale = self.config['Prescale'],
                                                            algos = [ Bd2JpsieeKSFromTracks ],
                                                            MDSTFlag = False )

        Bd2JpsieeKSFromTracksDetached = self.createSubSel( InputList = Bd2JpsieeKSFromTracks,
                                                           OutputList = Bd2JpsieeKSFromTracks.name() + "Detached",
                                                           Cuts = "(BPVLTIME() > %(BPVLTIME)s*ps)" % self.config )

        Bd2JpsieeKSFromTracksDetachedLine = StrippingLine( self.name + "Bd2JpsieeKSFromTracksDetachedLine",
                                                           prescale = 1.0,
                                                           algos = [ Bd2JpsieeKSFromTracksDetached ],
                                                           MDSTFlag = False )

        Bd2JpsieeKSDetached = self.createSubSel( InputList = Bd2JpsieeKS,
                                                 OutputList = Bd2JpsieeKS.name() + "Detached",
                                                 Cuts = "(BPVLTIME() > %(BPVLTIME)s*ps)" % self.config )

        Bd2JpsieeKSDetachedLine = StrippingLine( self.name + "Bd2JpsieeKSDetachedLine",
                                                 prescale = 1.0,
                                                 algos = [ Bd2JpsieeKSDetached ],
                                                 MDSTFlag = False )
        
        self.registerLine(Bd2JpsieeKSFromTracksPrescaledLine)
        self.registerLine(Bd2JpsieeKSFromTracksDetachedLine)
        self.registerLine(Bd2JpsieeKSDetachedLine)

    def makeBd2JpsieeKstar( self ):
        Bd2JpsieeKstarFromTracks = self.createCombinationSel( OutputList = "Bd2JpsieeKstarFromTracks" + self.name,
                                                              DecayDescriptor = "[B0 -> J/psi(1S) K*(892)0]cc",
                                                              DaughterLists   = [ self.JpsiFromTracks, self.KstarList ],
                                                              PreVertexCuts   = "in_range(%(BdMassMin)s,AM,%(BdMassMax)s)" % self.config,
                                                              PostVertexCuts  = "(VFASPF(VCHI2/VDOF) < 20)" ) # for the other particles is 10.

        Bd2JpsieeKstarFromTracksPrescaledLine = StrippingLine( self.name + "Bd2JpsieeKstarFromTracksPrescaledLine",
                                                               algos = [ Bd2JpsieeKstarFromTracks ],
                                                               prescale = self.config['Bd2JpsieeKstarPrescale'],
                                                               MDSTFlag = False )

        Bd2JpsieeKstarFromTracksDetached = self.createSubSel( InputList = Bd2JpsieeKstarFromTracks,
                                                              OutputList = Bd2JpsieeKstarFromTracks.name() + "Detached" + self.name,
                                                              Cuts = "(BPVLTIME() > %(BPVLTIME)s*ps)" % self.config )

        Bd2JpsieeKstarFromTracksDetachedLine  = StrippingLine( self.name + "Bd2JpsieeKstarFromTracksDetachedLine",
                                                               algos = [ Bd2JpsieeKstarFromTracksDetached ],
                                                               MDSTFlag = False )

        self.registerLine(Bd2JpsieeKstarFromTracksPrescaledLine)
        self.registerLine(Bd2JpsieeKstarFromTracksDetachedLine)


    def makeBu2JpsieeK( self ):
        Bu2JpsieeKFromTracks = self.createCombinationSel( OutputList = "Bu2JpsieeKFromTracks" + self.name,
                                                          DecayDescriptor = "[B+ -> J/psi(1S) K+]cc",
                                                          DaughterLists = [ self.JpsiFromTracks, self.NoIPKaonList ],
                                                          DaughterCuts  = {"K+": "(PT > 500.*MeV)" },
                                                          PreVertexCuts   = "in_range(%(BdMassMin)s,AM,%(BdMassMax)s)" % self.config,
                                                          PostVertexCuts = "(VFASPF(VCHI2PDOF) < %(BdVertexCHI2pDOF)s)" % self.config )
     
        Bu2JpsieeKFromTracksDetached = self.createSubSel( InputList = Bu2JpsieeKFromTracks,
                                                          OutputList = Bu2JpsieeKFromTracks.name() + "Detached" + self.name,
                                                          Cuts = "(BPVLTIME() > %(BPVLTIME)s*ps) & "\
                                                                 "(MINTREE('K+'==ABSID, PT) > 500.*MeV)" % self.config )

        Bu2JpsieeKFromTracksDetachedLine  = StrippingLine( self.name + "Bu2JpsieeKFromTracksDetachedLine",
                                                          algos = [ Bu2JpsieeKFromTracksDetached ],
                                                          MDSTFlag = False )
    
        self.registerLine(Bu2JpsieeKFromTracksDetachedLine)
