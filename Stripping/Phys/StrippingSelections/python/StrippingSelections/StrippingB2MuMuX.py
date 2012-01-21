#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#
'''
B->mumugamma,mumumumu,v0mu ...

Author: M. Williams
'''
__author__ = ['Mike Williams','Kostas Petridis']

moduleName = 'B2MuMuX'

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

from Gaudi.Configuration import *
from GaudiConfUtils.ConfigurableGenerators import FilterDesktop
from PhysSelPython.Wrappers import Selection, DataOnDemand, MergedSelection
from StrippingConf.StrippingLine import StrippingLine
from StrippingUtils.Utils import LineBuilder
from StandardParticles import StdAllNoPIDsPions, StdAllNoPIDsKaons
from StandardParticles import StdAllVeryLooseMuons, StdAllLooseMuons,\
     StdAllNoPIDsMuons
from StandardParticles import StdLooseAllPhotons
from Beauty2Charm_LoKiCuts import LoKiCuts
from GaudiConfUtils.ConfigurableGenerators import CombineParticles
from Configurables import OfflineVertexFitter

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

# Default configuration dictionary
config = {
    'MuonsLoose': {
    'ISMUON' : True,
    'TRCHI2DOF_MAX' : 5,
    'MIPCHI2DV_MIN' : 4,
    'InAccMuon'  : True
    },
    'MuonsNoPid': {
    'ISMUON' : False,
    'TRCHI2DOF_MAX' : 5,
    'MIPCHI2DV_MIN' : 4,
    'InAccMuon'  : True
    },
    'DiMuons': {
    'AMAXDOCA_MAX'  : '0.5*mm',
    'ASUMPT_MIN'    : '1000*MeV',
    'VCHI2DOF_MAX'  : 16,
    'BPVVDCHI2_MIN' : 16,
    'BPVDIRA_MIN'   : 0.0
    },
    'DiMuonsCorrM': {
    'AMAXDOCA_MAX'  : '0.5*mm',
    'ASUMPT_MIN'    : '1000*MeV',
    'VCHI2DOF_MAX'  : 8,
    'BPVVDCHI2_MIN' : 125,
    'BPVDIRA_MIN'   : 0.0
    },
    'Photons': {
    'PT_MIN' : '1500*MeV'
    },    
    "V0s": { # Cuts for rho, K*, phi
    'MASS_MIN'      : {'KST':'700*MeV','RHO':'600*MeV','PHI':'900*MeV'},
    'MASS_MAX'      : {'KST':'1100*MeV','RHO':'1000*MeV','PHI':'1100*MeV'},
    'DAUGHTERS'     : {'PT_MIN':'100*MeV','P_MIN':'2000*MeV',
                       'MIPCHI2DV_MIN' : 4, 'TRCHI2DOF_MAX' : 4},
    'AMAXDOCA_MAX'  : '0.5*mm',
    'VCHI2DOF_MAX'  : 16,
    'BPVVDCHI2_MIN' : 16, 
    'BPVDIRA_MIN'   : 0,
    'ASUMPT_MIN'    : '1000*MeV'
    },    
    "B2X3BODY" : { 
    'SUMPT_MIN'     : '4500*MeV',
    'VCHI2DOF_MAX'  : 10,
    'BPVIPCHI2_MAX' : 25,
    'BPVVDCHI2_MIN' : 25,
    'BPVDIRA_MIN'   : 0.4, #0.0
    'MASS_MIN'      : {'B':'4200*MeV','Y':'8500*MeV'},
    'MASS_MAX'      : {'B':'6500*MeV','Y':'10500*MeV'},
    
    #only used in CORRM lines
    'FORCORRM'      : {'B':{'CORRM_MIN': '4800*MeV',
                            'CORRM_MAX': '5800*MeV',
                            'PTMU' : '500*MeV'},
                       'Y':{'CORRM_MIN': '8500*MeV',
                            'CORRM_MAX': '10500*MeV',
                            'PTMU' : '500*MeV'}
                       }
    },
    "B2X4BODY" : { 
    'SUMPT_MIN'     : '5000*MeV',
    'VCHI2DOF_MAX'  : 10,
    'BPVIPCHI2_MAX' : 25,
    'BPVVDCHI2_MIN' : 25,
    'BPVDIRA_MIN'   : 0.0,
    'MASS_MIN'      : {'B':'4300*MeV','Y':'9999999*MeV'},
    'MASS_MAX'      : {'B':'6300*MeV','Y':'-999999*MeV'}
    },
    "Prescales" : {
    'OS'            : 1.0,
    'SS'            : 3.0
    },
    'GECNTrkMax'   : 500
    }

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

def filterMuons(muons,config):
    name='B2MuMuXNoPidMuonFilter'
    code = ''
    if config['ISMUON']:
        code += '(ISMUON) & '
        name=name.replace("NoPid","Loose")
    if config['InAccMuon']:
        code += '(PPINFO(LHCb.ProtoParticle.InAccMuon,-1) == 1) &'
    code += LoKiCuts(['TRCHI2DOF','MIPCHI2DV'],config).code()
    return Selection(name,Algorithm=FilterDesktop(Code=code),
                     RequiredSelections=[muons])

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

def diMuonBuilder(muons,config,decay,tag,oneLooseMuInDimu):
    comboCuts = LoKiCuts(['ASUMPT','AMAXDOCA'],config).code()
    if oneLooseMuInDimu:
        #comboCuts+='& ( ACHILDCUT(ISMUON, 1) | ACHILDCUT(ISMUON, 2) )'
        comboCuts+='& ( (ANUM(ISMUON))>0 )'
    momCuts = LoKiCuts(['VCHI2DOF','BPVVDCHI2','BPVDIRA'],config).code()
    cp = CombineParticles(CombinationCut=comboCuts,MotherCut=momCuts,
                          DecayDescriptors=[decay])
    return  Selection('B2MuMuXDiMuonBuilder'+tag,Algorithm=cp,
                      RequiredSelections=[muons])

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

def filterPhotons(photons,config):
    code = LoKiCuts(['PT'],config).code()
    return Selection('B2MuMuXPhotonFilter',
                     Algorithm=FilterDesktop(Code=code),
                     RequiredSelections=[photons])

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

#def makeV0s(pions,config):
#    ''' Makes all V0 -> X+X- selections.'''
#    dauCuts = {'pi+' : LoKiCuts(['P','PT','TRCHI2DOF','MIPCHI2DV'],config['DAUGHTERS']).code()}
#    wm = ['in_range(%s,AM,%s)' % (config['MASS_MIN']['RHO'],
#                                  config['MASS_MAX']['RHO']),
#          "in_range(%s,AWM('K+','pi-'),%s)" % (config['MASS_MIN']['KST'],
#                                               config['MASS_MAX']['KST']),
#          "in_range(%s,AWM('pi+','K-'),%s)" % (config['MASS_MIN']['KST'],
#                                               config['MASS_MAX']['KST']),
#          "in_range(%s,AWM('K+','K-'),%s)" % (config['MASS_MIN']['PHI'],
#                                              config['MASS_MAX']['PHI'])]
#    wm = '('+('|'.join(wm))+')'
#    comboCuts = [LoKiCuts(['ASUMPT'],config).code(),wm]
#    comboCuts.append(LoKiCuts(['AMAXDOCA'],config).code())
#    comboCuts = LoKiCuts.combine(comboCuts)
#    momCuts = LoKiCuts(['VCHI2DOF','BPVVDCHI2','BPVDIRA'],config).code()
#    cp = CombineParticles(CombinationCut=comboCuts,MotherCut=momCuts,
#                          DecayDescriptors=['rho(770)0 -> pi+ pi-'])
#    return  Selection('Rho2PiPiB2MuMuX',Algorithm=cp,
#                      RequiredSelections=[pions])                                                       
#

def makeV0s(name,particles,decay,config):  
    ''' Makes all V0 -> X+X- selections.'''

    dauPiCuts = {'pi+' : LoKiCuts(['P','PT','TRCHI2DOF','MIPCHI2DV'],config['DAUGHTERS']).code()}
    dauKCuts  = {'K+'  : LoKiCuts(['P','PT','TRCHI2DOF','MIPCHI2DV'],config['DAUGHTERS']).code()}

    if 'Rho' in name:
        wm = ['in_range(%s,AM,%s)' % (config['MASS_MIN']['RHO'],config['MASS_MAX']['RHO'])]
    elif 'Kst' in name:
        wm = ['in_range(%s,AM,%s)' % (config['MASS_MIN']['KST'],config['MASS_MAX']['KST'])]
    elif 'Phi' in name:
        wm = ['in_range(%s,AM,%s)' % (config['MASS_MIN']['PHI'],config['MASS_MAX']['PHI'])]

    wm = '('+('|'.join(wm))+')'
    comboCuts = [LoKiCuts(['ASUMPT'],config).code(),wm]
    comboCuts.append(LoKiCuts(['AMAXDOCA'],config).code())
    comboCuts = LoKiCuts.combine(comboCuts)

    momCuts = LoKiCuts(['VCHI2DOF','BPVVDCHI2','BPVDIRA'],config).code()

    cp = CombineParticles(CombinationCut=comboCuts,MotherCut=momCuts,
                          DecayDescriptors=[decay])

    return  Selection(name+'B2MuMuX',Algorithm=cp,
                      RequiredSelections=particles)  
    
#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

def makeB2X(name,decay,inputs,config,vertex=True):
    wm = ['in_range(%s,AM,%s)' % (config['MASS_MIN']['B'],
                                  config['MASS_MAX']['B'])]

    wm = '('+('|'.join(wm))+')'
    comboCuts = [LoKiCuts(['SUMPT'],config).code(),wm]
    comboCuts = LoKiCuts.combine(comboCuts)
    momCuts = LoKiCuts(['VCHI2DOF','BPVVDCHI2','BPVIPCHI2','BPVDIRA'],
                       config).code()
    b2x = CombineParticles(DecayDescriptors=decay,CombinationCut=comboCuts,
                           MotherCut=momCuts)
    if not vertex:
        b2x = b2x.configurable(name+'B2MuMuXVertexFitter')
        b2x.addTool(OfflineVertexFitter)
        b2x.VertexFitters.update( { "" : "OfflineVertexFitter"} )
        b2x.OfflineVertexFitter.useResonanceVertex = False
    return Selection(name,Algorithm=b2x,RequiredSelections=inputs)

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#


def makeY2X(name,decay,inputs,config,vertex=True):
    wm = ['in_range(%s,AM,%s)' % (config['MASS_MIN']['Y'],
                                  config['MASS_MAX']['Y'])]

    wm = '('+('|'.join(wm))+')'
    comboCuts = [LoKiCuts(['SUMPT'],config).code(),wm]
    comboCuts = LoKiCuts.combine(comboCuts)
    momCuts = LoKiCuts(['VCHI2DOF','BPVVDCHI2','BPVIPCHI2','BPVDIRA'],
                       config).code()
    y2x = CombineParticles(DecayDescriptors=decay,CombinationCut=comboCuts,
                           MotherCut=momCuts)
    if not vertex:
        y2x = y2x.configurable(name+'B2MuMuXVertexFitter')
        y2x.addTool(OfflineVertexFitter)
        y2x.VertexFitters.update( { "" : "OfflineVertexFitter"} )
        y2x.OfflineVertexFitter.useResonanceVertex = False
    return Selection(name,Algorithm=y2x,RequiredSelections=inputs)

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

def makeB2MuMuGammaCORRM(name,input,config):
    code  = '(BPVCORRM > %s) & (BPVCORRM < %s)' % (config['CORRM_MIN'],config['CORRM_MAX'])
    code += '& (NINGENERATION(PT<%s,1)==0)' % (config['PTMU'])


    return Selection(name+'CORRMFilter',Algorithm=FilterDesktop(Code=code),
                     RequiredSelections=[input])

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

class B2MuMuXConf(LineBuilder):
    __configuration_keys__ = ('MuonsLoose','MuonsNoPid','DiMuons','DiMuonsCorrM','Photons',
                              'B2X3BODY','B2X4BODY','Prescales','V0s','GECNTrkMax')

    def __init__(self, moduleName, config) :
        
        LineBuilder.__init__(self, moduleName, config)
        muons_loose  = filterMuons(StdAllLooseMuons,config['MuonsLoose'])
        muons_noPid  = filterMuons(StdAllVeryLooseMuons,config['MuonsNoPid'])

        
        # os fly and no fly dimuons
        oneLooseMuInDiMu=False
        dimuons_nofly_loose = diMuonBuilder(muons_loose,config['DiMuons'],
                                            'J/psi(1S) -> mu+ mu-','NoFly_loose',
                                            oneLooseMuInDiMu)
        dimuons_nofly_loose_corrm = diMuonBuilder(muons_loose,config['DiMuonsCorrM'],
                                                  'J/psi(1S) -> mu+ mu-','NoFly_loose_CoorM',
                                                  oneLooseMuInDiMu)
        dimuons_fly_loose = diMuonBuilder(muons_loose,config['DiMuons'],
                                          'KS0 -> mu+ mu-','Fly_loose',
                                          oneLooseMuInDiMu)
        oneLooseMuInDiMu=True
        dimuons_fly_onePid = diMuonBuilder(muons_noPid,config['DiMuons'],
                                           'KS0 -> mu+ mu-','Fly_onePid',
                                           oneLooseMuInDiMu)

        # ss fly and no fly dimuons
        oneLooseMuInDiMu=False
        dimuons_nofly_loose_ss = diMuonBuilder(muons_loose,config['DiMuons'],
                                               '[J/psi(1S) -> mu+ mu+]cc',
                                               'NoFly_loose_ss',
                                               oneLooseMuInDiMu)
        dimuons_fly_loose_ss = diMuonBuilder(muons_loose,config['DiMuons'],
                                             '[KS0 -> mu+ mu+]cc',
                                             'Fly_loose_ss',
                                             oneLooseMuInDiMu)

        # photons
        photons = filterPhotons(StdLooseAllPhotons,config['Photons'])

        # v0s for rho0,Kst,phi
        rhos = makeV0s('Rho2PiPi',[StdAllNoPIDsPions],'rho(770)0 -> pi+ pi-',config['V0s'])
        ksts = makeV0s('Kst2KPi' ,[StdAllNoPIDsPions,StdAllNoPIDsKaons],'[K*(892)0 -> K+ pi-]cc',config['V0s'])
        phis = makeV0s('Phi2KK'  ,[StdAllNoPIDsKaons],'phi(1020) -> K+ K-',config['V0s'])
        
        # form B hadron and stripping lines
        # B to Mu Mu Gamma
        b2mumug = makeB2X('B2MuMuGamma',['B0 -> J/psi(1S) gamma'],
                          [dimuons_nofly_loose,photons],
                          config['B2X3BODY'],False)
        self._makeLine(b2mumug,config)

        # Y to Mu Mu Gamma
        y2mumug = makeY2X('B2MuMuGammaHighMass',['B0 -> J/psi(1S) gamma'],
                          [dimuons_nofly_loose,photons],
                          config['B2X3BODY'],False)
        self._makeLine(y2mumug,config)


        # B to Mu Mu Mu Mu
        b24mu = makeB2X('B2MuMuMuMu',['B0 -> KS0 KS0'],[dimuons_fly_onePid],
                        config['B2X4BODY'])
        self._makeLine(b24mu,config)
        
        # B to V0 MuMu
        #rho
        b2rhomumu = makeB2X('B2RhoMuMu',['B0 -> KS0 rho(770)0' ],
                            [dimuons_fly_loose,rhos],
                            config['B2X4BODY'])
        self._makeLine(b2rhomumu,config)
        #kst
        b2kstmumu = makeB2X('B2KstMuMu',['[B0 -> KS0 K*(892)0]cc' ],
                            [dimuons_fly_loose,ksts],
                            config['B2X4BODY'])
        self._makeLine(b2kstmumu,config)
        #phi
        b2phimumu = makeB2X('B2PhiMuMu',['B0 -> KS0 phi(1020)' ],
                            [dimuons_fly_loose,phis],
                            config['B2X4BODY'])
        self._makeLine(b2phimumu,config)


        # SS stripping lines

        b2mumug_ss = makeB2X('B2MuMuGamma_ss',['B0 -> J/psi(1S) gamma'],
                             [dimuons_nofly_loose_ss,photons],
                             config['B2X3BODY'],False)
        self._makeLine(b2mumug_ss,config)
        #rho ss
        b2rhomumu_ss = makeB2X('B2RhoMuMu_ss',['B0 -> KS0 rho(770)0' ],
                               [dimuons_fly_loose_ss,rhos],
                               config['B2X4BODY'])
        self._makeLine(b2rhomumu_ss,config)
        # kst ss
        b2kstmumu_ss = makeB2X('B2KstMuMu_ss',['[B0 -> KS0 K*(892)0]cc' ],
                            [dimuons_fly_loose_ss,ksts],
                           config['B2X4BODY'])
        self._makeLine(b2kstmumu_ss,config)
        # phi ss
        b2phimumu_ss = makeB2X('B2PhiMuMu_ss',['B0 -> KS0 phi(1020)' ],
                            [dimuons_fly_loose_ss,phis],
                           config['B2X4BODY'])
        self._makeLine(b2phimumu_ss,config)
        
        
        # Corrected mass stripping lines
        # B to Mu Mu Gamma
        b2mumug_mcor = makeB2MuMuGammaCORRM('B2MuMuGamma',dimuons_nofly_loose,
                                           config['B2X3BODY']['FORCORRM']['B'])
        self._makeLine(b2mumug_mcor,config)
        # Y to Mu Mu Gamma
        y2mumug_mcor = makeB2MuMuGammaCORRM('B2MuMuGammaHighMass',dimuons_nofly_loose,
                                           config['B2X3BODY']['FORCORRM']['Y'])
        self._makeLine(y2mumug_mcor,config)
    

        
    def _makeLine(self,sel,config):
        tmpSel = Selection(sel.name()+'FilterALL',
                           Algorithm=FilterDesktop(Code='ALL'),
                           RequiredSelections=[sel])
        name = sel.name()+'Line'
        filter = {'Code' :
                  "(recSummaryTrack(LHCb.RecSummary.nLongTracks, TrLONG)"\
                  " < %s )" \
                  % config['GECNTrkMax'],
                  'Preambulo' : [ "from LoKiTracks.decorators import *",
                                  'from LoKiCore.functions    import *' ]
                  }

        hlt = "HLT_PASS_RE('Hlt2.*Topo.*Decision')"\
              "| HLT_PASS_RE('Hlt2IncPhi.*Decision')"\
              "| HLT_PASS_RE('Hlt2.*Muon.*Decision')"\
              "| HLT_PASS_RE('Hlt2.*Gamma.*Decision')"\
              "| HLT_PASS_RE('Hlt2.*Photon.*Decision')"

        if ("B2RhoMuMu" in name) | ("B2KstMuMu" in name) | ("B2PhiMuMu" in name) :
            hlt = "HLT_PASS_RE('Hlt2.*Topo.*Decision')"\
                  "| HLT_PASS_RE('Hlt2IncPhi.*Decision')"

        print ":::::::::::::::::::::::::::::::::::::::::::::::::::::::",hlt
              
        presc = config['Prescales']['OS']
        if "ss" in name:
            presc = config['Prescales']['SS']


        sline = StrippingLine(name,prescale=presc,selection=tmpSel,checkPV=True,
                              FILTER=filter,HLT=hlt)
        
        self.registerLine(sline)
        
