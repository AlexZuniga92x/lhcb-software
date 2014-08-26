"""
Module for construction of Xb->phh and Xb->phhh Stripping Selections and StrippingLines.
Provides class Xb2p2hOrXb2p3hConf, which constructs the Selections and StrippingLines
given a configuration dictionary.
Exported symbols (use python help!):
   - Xb2p2hOrXb2p3hConf
"""

__author__  = ['Jan Mickelle V. Maratas', 'Stephane Monteil']
__date__    = '24/08/2014'
__version__ = 'v1r0'
__all__     = 'Xb2p2hOrXb2p3hConf'

from Gaudi.Configuration                   import *
from PhysSelPython.Wrappers                import Selection
from StrippingConf.StrippingLine           import StrippingLine
from StrippingUtils.Utils                  import LineBuilder
from GaudiConfUtils.ConfigurableGenerators import FilterDesktop, DaVinci__N3BodyDecays, DaVinci__N4BodyDecays

from StandardParticles                     import StdNoPIDsPions     as Pions
from StandardParticles                     import StdLooseANNProtons as Protons

default_config = { 'Trk_MaxChi2Ndof'   : 3.0,
                   'Trk_MaxGhostProb'  : 0.4,
                   'Trk_MinIPChi2'     : 16.0,
                   'Trk_MinP'          : 1500.0,
                   'Trk_MinProbNNp'    : 0.05,
                   'Xb_MinSumPTppi'    : 1500.0,
                   'Xb_MinSumPTppipi'  : 2500.0,
                   'Xb_MinM_3body'     : 5195.0,
                   'Xb_MaxM_3body'     : 6405.0,
                   'Xb_MinSumPT_3body' : 3500.0,
                   'Xb_MinPT_3body'    : 1500.0,
                   'Xb_MinM_4body'     : 5195.0,
                   'Xb_MaxM_4body'     : 6405.0,
                   'Xb_MinSumPT_4body' : 3500.0,
                   'Xb_MinPT_4body'    : 1500.0,
                   'Xb_MaxDOCAChi2'    : 20.0,
                   'Xb_MaxVtxChi2'     : 20.0,
                   'Xb_MinFDChi2'      : 50.0,
                   'Xb_MaxIPChi2'      : 16.0,
                   'Xb_MinDira'        : 0.9999,
                   'Prescale'          : 1.0,
                   'Postscale'         : 1.0 }

class Xb2p2hOrXb2p3hConf(LineBuilder) :
    """
    Builder of Xb -> phh or Xb -> phhh Stripping Selection and StrippingLine.
    Constructs Xb -> phh or Xb -> phhh Selections and StrippingLines from a configuration dictionary.
    Usage:
    >>> config              = { .... }
    >>> Xb2p2hOrXb2p3hConf  = Xb2p2hOrXb2p3hConf('Xb2p2hOrXb2p3hTest', config)
    >>> Xb2p2hOrXb2p3hLines = Xb2p2hOrXb2p3hConf.lines
    >>> for line in Xb2p2hOrXb2p3hLines :
    >>>   print line.name(), line.outputLocation()
    The lines can be used directly to build a StrippingStream object.

    Exports as instance data members:
    selXb2phhh   : Xb -> phhh
    selXb2phh    : Xb -> phh
    lineXb2phhh  : StrippingLine made out of selXb2phhh
    lineXb2phh   : StrippingLine made out of selXb2phh
    lines        : List of lines, [lineXb2phhh,
                                   lineXb2phh]

    Exports as class data member:
    Xb2p2hOrXb2p3hConf.__configuration_keys__ : List of required configuration parameters.
    """
    __configuration_keys__ = ('Trk_MaxChi2Ndof',
                              'Trk_MaxGhostProb',
                              'Trk_MinIPChi2',
                              'Trk_MinP',
                              'Trk_MinProbNNp',
                              'Xb_MinSumPTppi',
                              'Xb_MinSumPTppipi',
                              'Xb_MinM_3body',
                              'Xb_MaxM_3body',
                              'Xb_MinSumPT_3body',
                              'Xb_MinPT_3body',
                              'Xb_MinM_4body',
                              'Xb_MaxM_4body',
                              'Xb_MinSumPT_4body',
                              'Xb_MinPT_4body',
                              'Xb_MaxDOCAChi2',
                              'Xb_MaxVtxChi2',
                              'Xb_MinFDChi2',
                              'Xb_MaxIPChi2',
                              'Xb_MinDira',
                              'Prescale',
                              'Postscale')

    def __init__(self, name, config) :

        LineBuilder.__init__(self, name, config)

        self.pion      = Pions
        self.proton    = Protons

        trkFilterP     = FilterDesktop(Code = '(MIPCHI2DV(PRIMARY) > %(Trk_MinIPChi2)s)    & \
                                               (TRGHOSTPROB        < %(Trk_MaxGhostProb)s) & \
                                               (TRCHI2DOF          < %(Trk_MaxChi2Ndof)s)  & \
                                               (P                  > %(Trk_MinP)s)         & \
                                               (PROBNNp            > %(Trk_MinProbNNp)s)' % config )

        trkFilterPi    = FilterDesktop(Code = '(MIPCHI2DV(PRIMARY) > %(Trk_MinIPChi2)s)    & \
                                               (TRGHOSTPROB        < %(Trk_MaxGhostProb)s) & \
                                               (TRCHI2DOF          < %(Trk_MaxChi2Ndof)s)  & \
                                               (P                  > %(Trk_MinP)s)' % config )

        self.myPions   = Selection( 'PionsFor'+name,
                                    Algorithm = trkFilterPi,
                                    RequiredSelections = [self.pion] )

        self.myProtons = Selection( 'ProtonsFor'+name,
                                    Algorithm = trkFilterP,
                                    RequiredSelections = [self.proton] )

        self.makeXb2phh (name+'_3body', config)
        self.makeXb2phhh(name+'_4body', config)

        self.lineXb2phh  = StrippingLine('Xb2phhLine',
                                         prescale         = config['Prescale'],
                                         postscale        = config['Postscale'],
                                         selection        = self.selXb2phh,
                                         RelatedInfoTools = [ { "Type"         : "RelInfoConeVariables",
                                                                "ConeAngle"    : 0.8,
                                                                "Variables"    : ['CONEANGLE', 'CONEMULT', 'CONEPTASYM'],
                                                                "TopSelection" : self.selXb2phh,
                                                                "Location"     : 'P2ConeVar1' },
                                                              { "Type"         : "RelInfoConeVariables",
                                                                "ConeAngle"    : 1.0,
                                                                "Variables"    : ['CONEANGLE', 'CONEMULT', 'CONEPTASYM'],
                                                                "TopSelection" : self.selXb2phh,
                                                                "Location"     : 'P2ConeVar2' },
                                                              { "Type"         : "RelInfoConeVariables",
                                                                "ConeAngle"    : 1.3,
                                                                "Variables"    : ['CONEANGLE', 'CONEMULT', 'CONEPTASYM'],
                                                                "TopSelection" : self.selXb2phh,
                                                                "Location"     : 'P2ConeVar3' },
                                                              { "Type"         : "RelInfoConeVariables",
                                                                "ConeAngle"    : 1.7,
                                                                "Variables"    : ['CONEANGLE', 'CONEMULT', 'CONEPTASYM'],
                                                                "TopSelection" : self.selXb2phh,
                                                                "Location"     : 'P2ConeVar4' },
                                                              { "Type"         : "RelInfoVertexIsolation",
                                                                "Location"     : "VertexIsoInfo "} ] )

        self.lineXb2phhh = StrippingLine('Xb2phhhLine',
                                         prescale         = config['Prescale'],
                                         postscale        = config['Postscale'],
                                         selection        = self.selXb2phhh,
                                         RelatedInfoTools = [ { "Type"         : "RelInfoConeVariables",
                                                                "ConeAngle"    : 0.8,
                                                                "Variables"    : ['CONEANGLE', 'CONEMULT', 'CONEPTASYM'],
                                                                "TopSelection" : self.selXb2phhh,
                                                                "Location"     : 'P2ConeVar1' },
                                                              { "Type"         : "RelInfoConeVariables",
                                                                "ConeAngle"    : 1.0,
                                                                "Variables"    : ['CONEANGLE', 'CONEMULT', 'CONEPTASYM'],
                                                                "TopSelection" : self.selXb2phhh,
                                                                "Location"     : 'P2ConeVar2' },
                                                              { "Type"         : "RelInfoConeVariables",
                                                                "ConeAngle"    : 1.3,
                                                                "Variables"    : ['CONEANGLE', 'CONEMULT', 'CONEPTASYM'],
                                                                "TopSelection" : self.selXb2phhh,
                                                                "Location"     : 'P2ConeVar3' },
                                                              { "Type"         : "RelInfoConeVariables",
                                                                "ConeAngle"    : 1.7,
                                                                "Variables"    : ['CONEANGLE', 'CONEMULT', 'CONEPTASYM'],
                                                                "TopSelection" : self.selXb2phhh,
                                                                "Location"     : 'P2ConeVar4' },
                                                              { "Type"         : "RelInfoVertexIsolation",
                                                                "Location"     : "VertexIsoInfo" } ] )

        self.registerLine(self.lineXb2phh )
        self.registerLine(self.lineXb2phhh)


    def makeXb2phh (self, name, config) :
        # Define all the cuts
        _mass12CutPreVtx          = '(AM < (%s - 140)*MeV)'                                   % config['Xb_MaxM_3body']
        _sumpt12CutPreVtx         = '( (APT1 + APT2) > %s*MeV)'                               % config['Xb_MinSumPTppi']
        _doca12chi2CutPreVtx      = '(ACHI2DOCA(1,2) < %s)'                                   % config['Xb_MaxDOCAChi2']
        _combCuts12               = _mass12CutPreVtx+' & '+_sumpt12CutPreVtx+' & '+_doca12chi2CutPreVtx

        _wmassp2KCutPreVtx        = "(AWM('p+','K-','K-') > %s*MeV)"                          % config['Xb_MinM_3body']
        _massCutPreVtx            = '(AM             < %s*MeV)'                               % config['Xb_MaxM_3body']
        _sumptCutPreVtx           = '( (APT1 + APT2 + APT3) > %s*MeV)'                        % config['Xb_MinSumPT_3body']
        _ptCutPreVtx              = '(APT            > %s*MeV)'                               % config['Xb_MinPT_3body']
        _docachi2CutPreVtx13      = '(ACHI2DOCA(1,3) < %s)'                                   % config['Xb_MaxDOCAChi2']
        _docachi2CutPreVtx23      = '(ACHI2DOCA(2,3) < %s)'                                   % config['Xb_MaxDOCAChi2']
        _massCutsPreVtx           = _massCutPreVtx+' & '+_wmassp2KCutPreVtx
        _combCuts                 = _massCutsPreVtx+' & '+_sumptCutPreVtx+' & '+_ptCutPreVtx+' & '+\
                                    _docachi2CutPreVtx13+' & '+_docachi2CutPreVtx23

        _vtxChi2CutPostVtx        = '(VFASPF(VCHI2)  < %s)'                                   % config['Xb_MaxVtxChi2']
        _fdChi2CutPostVtx         = '(BPVVDCHI2      > %s)'                                   % config['Xb_MinFDChi2']
        _diraCutPostVtx           = '(BPVDIRA        > %s)'                                   % config['Xb_MinDira']
        _ipChi2CutPostVtx         = '(BPVIPCHI2()    < %s)'                                   % config['Xb_MaxIPChi2']
        _motherCuts               = _vtxChi2CutPostVtx+' & '+_fdChi2CutPostVtx+' & '+_diraCutPostVtx+' & '+_ipChi2CutPostVtx

        _Xb                       = DaVinci__N3BodyDecays()
        _Xb.Combination12Cut      = _combCuts12
        _Xb.CombinationCut        = _combCuts
        _Xb.MotherCut             = _motherCuts
        _Xb.DecayDescriptors      = [ '[Xi_b- -> p+ pi- pi-]cc', '[Xi_b~+ -> p+ pi+ pi-]cc', '[Xi_b~+ -> p+ pi+ pi+]cc' ]
        _Xb.ReFitPVs              = True

        self.selXb2phh            = Selection(name, Algorithm = _Xb, RequiredSelections = [self.myProtons, self.myPions])


    def makeXb2phhh(self, name, config) :
        # Define all the cuts
        _mass12CutPreVtx          = '(AM < (%s - 280)*MeV)'                                   % config['Xb_MaxM_4body']
        _sumpt12CutPreVtx         = '( (APT1 + APT2) > %s*MeV)'                               % config['Xb_MinSumPTppi']
        _doca12chi2CutPreVtx      = '(ACHI2DOCA(1,2) < %s)'                                   % config['Xb_MaxDOCAChi2']
        _combCuts12               = _mass12CutPreVtx+' & '+_sumpt12CutPreVtx+' & '+_doca12chi2CutPreVtx

        _mass123CutPreVtx         = '(AM < (%s - 140)*MeV)'                                   % config['Xb_MaxM_4body']
        _sumpt123CutPreVtx        = '( (APT1 + APT2 + APT3) > %s*MeV)'                        % config['Xb_MinSumPTppipi']
        _doca123chi2CutPreVtx13   = '(ACHI2DOCA(1,3) < %s)'                                   % config['Xb_MaxDOCAChi2']
        _doca123chi2CutPreVtx23   = '(ACHI2DOCA(2,3) < %s)'                                   % config['Xb_MaxDOCAChi2']
        _combCuts123              = _mass123CutPreVtx+' & '+_sumpt123CutPreVtx+' & '+_doca123chi2CutPreVtx13+' & '+_doca123chi2CutPreVtx23

        _wmassp3KCutPreVtx        = "(AWM('p+','K-','K+','K-') > %s*MeV)"                     % config['Xb_MinM_4body']
        _massCutPreVtx            = '(AM             < %s*MeV)'                               % config['Xb_MaxM_4body']
        _sumptCutPreVtx           = '( (APT1 + APT2 + APT3 + APT4) > %s*MeV)'                 % config['Xb_MinSumPT_4body']
        _ptCutPreVtx              = '(APT            > %s*MeV)'                               % config['Xb_MinPT_4body']
        _docachi2CutPreVtx14      = '(ACHI2DOCA(1,4) < %s)'                                   % config['Xb_MaxDOCAChi2']
        _docachi2CutPreVtx24      = '(ACHI2DOCA(2,4) < %s)'                                   % config['Xb_MaxDOCAChi2']
        _docachi2CutPreVtx34      = '(ACHI2DOCA(3,4) < %s)'                                   % config['Xb_MaxDOCAChi2']
        _massCutsPreVtx           = _massCutPreVtx+' & '+_wmassp3KCutPreVtx
        _combCuts                 = _massCutsPreVtx+' & '+_sumptCutPreVtx+' & '+_ptCutPreVtx+' & '+\
                                    _docachi2CutPreVtx14+' & '+_docachi2CutPreVtx24+' & '+_docachi2CutPreVtx34

        _vtxChi2CutPostVtx        = '(VFASPF(VCHI2)  < %s)'                                   % config['Xb_MaxVtxChi2']
        _fdChi2CutPostVtx         = '(BPVVDCHI2      > %s)'                                   % config['Xb_MinFDChi2']
        _diraCutPostVtx           = '(BPVDIRA        > %s)'                                   % config['Xb_MinDira']
        _ipChi2CutPostVtx         = '(BPVIPCHI2()    < %s)'                                   % config['Xb_MaxIPChi2']
        _motherCuts               = _vtxChi2CutPostVtx+' & '+_fdChi2CutPostVtx+' & '+_diraCutPostVtx+' & '+_ipChi2CutPostVtx

        _Xb                       = DaVinci__N4BodyDecays()
        _Xb.Combination12Cut      = _combCuts12
        _Xb.Combination123Cut     = _combCuts123
        _Xb.CombinationCut        = _combCuts
        _Xb.MotherCut             = _motherCuts
        _Xb.DecayDescriptor       = "[Lambda_b0 -> p+ pi- pi+ pi-]cc"
        _Xb.ReFitPVs              = True

        self.selXb2phhh           = Selection(name, Algorithm = _Xb, RequiredSelections = [self.myProtons, self.myPions])

