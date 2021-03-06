#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

from copy import deepcopy
from Gaudi.Configuration import *
from GaudiConfUtils.ConfigurableGenerators import CombineParticles
from PhysSelPython.Wrappers import Selection
from Beauty2Charm_LoKiCuts import LoKiCuts
from Beauty2Charm_Utils import *
from Configurables import SubPIDMMFilter 

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

def awmFunctor(decays,min,max):
    wm = []
    for daughters in decays:
        pids = ','.join(["'%s'"%d for d in daughters])
        wm.append("in_range(%s,AWM(%s),%s)" % (min,pids,max))
    return '('+('|'.join(wm))+')'

def getCC(decay):
    cc = []
    for d in decay:
        if d.find('+') >=0: cc.append(d.replace('+','-'))
        elif d.find('-') >=0: cc.append(d.replace('-','+'))
        else: cc.append(d)
    return cc

def getCCs(decays): return [getCC(d) for d in decays]

def subPIDSels(decays,prefix,suffix,min,max,inputs):
    min = float(min.split('*')[0])
    max = float(max.split('*')[0])
    filter = SubPIDMMFilter(prefix+'SubPIDBeauty2Charm'+suffix,Code='ALL',
                            MinMM=min,MaxMM=max,PIDs=decays)
    presel = Selection(prefix+'SubPIDSelBeauty2Charm'+suffix,
                       Algorithm=filter,
                       RequiredSelections=inputs)
    mm =  "in_range(%s,MM,%s)" % (min,max)
    return filterSelection(prefix+suffix,mm,[presel])
                
#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

class DBuilder(object):
    '''Produces all D mesons for the Beauty2Charm module.'''

    def __init__(self,pions,ks,pi0,config,config_pid):
        self.pions = pions
        self.ks    = ks
        self.pi0   = pi0
        self.config = config
        self.hh = self._makeD2hh()
        self.hhh = self._makeD2hhh()
        self.hhhh = self._makeD2hhhh()
        self.ksh_ll  = self._makeD2KSh("LL")
        self.ksh_dd  = self._makeD2KSh("DD")
        self.kshh_ll  = self._makeD2KShh("LL")
        self.kshh_dd  = self._makeD2KShh("DD")
        self.pi0hh_merged = self._makeD2Pi0hh("Merged")
        self.pi0hh_resolved = self._makeD2Pi0hh("Resolved") 
        self.pi0hhh_merged = self._makeD2Pi0hhh("Merged")
        self.pi0hhh_resolved = self._makeD2Pi0hhh("Resolved")
        # PID filtered selections
        self.hh_pid = [filterPID('D2HHPID',self.hh,config_pid)]
        self.hhh_pid = [filterPID('D2HHHPID',self.hhh,config_pid)]        
        # subset decays
        oneK = "NINTREE(ABSID=='K+') == 1"
        d_cf = "(((ID=='D+') & (NINTREE(ID=='K-')==1) & (%s)) | "\
               "((ID=='D-') & (NINTREE(ID=='K+')==1) & (%s)))" % (oneK,oneK)
        ds_cf = "((NINTREE(ID=='K-')==1) & (NINTREE(ID=='K+')==1))"
        self.kpi_pid = [filterSelection('D2KPIPID',oneK,self.hh_pid)]
        self.k3pi = [filterSelection('D2K3PI',oneK,self.hhhh)]
        self.pi0kpi_merged = [filterSelection('D2Pi0KPi_Merged',oneK,
                                              self.pi0hh_merged)]
        self.pi0kpi_resolved = [filterSelection('D2Pi0KPi_Resolved',oneK,
                                                self.pi0hh_resolved)]
        d_min,d_max,ds_min,ds_max = self._massWindowD2HHHCF()
        d_cf = LoKiCuts.combine([d_cf,"in_range(%s,MM,%s)"%(d_min,d_max)])
        ds_cf = LoKiCuts.combine([ds_cf,"in_range(%s,MM,%s)"%(ds_min,ds_max)])
        cf = '('+d_cf+') | (' + ds_cf + ')'
        self.hhh_cf_pid = [filterSelection('D2HHHCFPID',cf,self.hhh_pid)]
        # WS decays
        self.kshh_ll_ws = self._makeD2KShhWS("LL")
        self.kshh_dd_ws = self._makeD2KShhWS("DD")
        self.hh_ws = self._makeD2hhWS()
        self.pi0hh_merged_ws = self._makeD2Pi0hhWS('Merged')
        self.pi0hh_resolved_ws = self._makeD2Pi0hhWS('Resolved')
        self.hhhh_ws = self._makeD2hhhhWS()

    def _makeD2X(self,name,decays,wm,config,extrainputs=[]):
        ''' Makes all D -> X selections.'''
        comboCuts = [LoKiCuts(['ASUMPT'],config).code(),wm,hasTopoChild()]
        comboCuts.append(LoKiCuts(['AMAXDOCA'],config).code())
        comboCuts = LoKiCuts.combine(comboCuts)
        momCuts = LoKiCuts(['VCHI2DOF','BPVVDCHI2','BPVDIRA'],config).code()
        cp = CombineParticles(CombinationCut=comboCuts,MotherCut=momCuts,
                              DecayDescriptors=decays)
        return  Selection('Proto'+name+'Beauty2Charm',Algorithm=cp,
                          RequiredSelections=[self.pions]+extrainputs)

    def _massWindow(self,which):
        dm,units = LoKiCuts.cutValue(self.config['MASS_WINDOW'])
        if which is 'D0':
            min = 1864.84 - dm # D0 - dm
            max = 1864.84 + dm # D0 + dm
        else:
            min = 1869.62 - dm # D+ - dm
            max = 1968.49 + dm # Ds+ + dm
        return ('%s*%s'%(min,units),'%s*%s'%(max,units))

    def _massWindowD2HHHCF(self):
        dm,units = LoKiCuts.cutValue(self.config['MASS_WINDOW'])
        d_min = 1869.62 - dm
        d_max = 1869.62 + dm
        ds_min = 1968.49 - dm
        ds_max = 1968.49 + dm
        return ('%s*%s'%(d_min,units),'%s*%s'%(d_max,units),
                '%s*%s'%(ds_min,units),'%s*%s'%(ds_max,units))

    def _makeD2hh(self):
        '''Makes D->hh'''
        min,max  = self._massWindow('D0')
        decays = [['pi+','pi-'],['pi+','K-'],['K+','pi-'],['K+','K-']]
        wm = awmFunctor(decays,min,max)
        protoD2hh = self._makeD2X('D2HH',['D0 -> pi+ pi-'],wm,self.config)
        return [subPIDSels(decays,'D2HH','',min,max,[protoD2hh])]

    def _makeD2hhWS(self):
        '''Makes D->hh WS'''
        min,max  = self._massWindow('D0')
        decays = [['pi+','pi+'],['pi+','K+'],['K+','pi+'],['K+','K+']]
        wm = awmFunctor(decays,min,max)
        psel = self._makeD2X('D2HHWSPlus',['D0 -> pi+ pi+'],wm,self.config)
        psel = subPIDSels(decays,'D2HHWSPlus','',min,max,[psel])
        msel = self._makeD2X('D2HHWSMinus',['D0 -> pi- pi-'],wm,self.config)
        msel = subPIDSels(getCCs(decays),'D2HHWSMinus','',min,max,[msel])
        return [MergedSelection('D2HHWSBeauty2Charm',
                                RequiredSelections=[psel,msel])]

    def _makeD2hhh(self):
        '''Makes D->hhh'''
        min,max = self._massWindow('D+')
        decays = [['pi+','pi+','pi-'],['pi+','pi+','K-'],
                  ['K+','pi+','pi-'],['K+','pi+','K-'],
                  ['pi+','K+','pi-'],['pi+','K+','K-'],
                  ['K+','K+','pi-']]
        wm = awmFunctor(decays,min,max)
        protoDp2hhh = self._makeD2X('D+2HHH',['D+ -> pi+ pi+ pi-'],wm,
                                    self.config)
        psels = subPIDSels(decays,'D+2HHH','',min,max,[protoDp2hhh])
        protoDm2hhh = self._makeD2X('D-2HHH',['D- -> pi- pi- pi+'],wm,
                                    self.config)
        msels = subPIDSels(getCCs(decays),'D-2HHH','',min,max,[protoDm2hhh])
        return [MergedSelection('D2HHHBeauty2Charm',
                                RequiredSelections=[psels,msels])]
        
    def _makeD2KSh(self,which):
        '''Makes D->Ksh'''
        min,max  = self._massWindow('D+')
        decays = [['KS0','pi+'],['KS0','K+']]
        wm = awmFunctor(decays,min,max)
        config = deepcopy(self.config)    
        config.pop('AMAXDOCA_MAX')
        protoDp2Ksh = self._makeD2X('D+2KsH_'+which,['D+ -> KS0 pi+'],
                                    wm,config,self.ks[which])
        psels = subPIDSels(decays,'D+2KsH',which,min,max,[protoDp2Ksh])
        protoDm2Ksh = self._makeD2X('D-2KsH_'+which,['D- -> KS0 pi-'],
                                    wm,config,self.ks[which])
        msels = subPIDSels(getCCs(decays),'D-2KsH',which,min,max,[protoDm2Ksh])
        return [MergedSelection('D2KsHBeauty2Charm_%s'%which,
                                RequiredSelections=[psels,msels])]
   
    def _makeD2KShh(self,which):
        '''Makes D->Kshh'''
        min,max = self._massWindow('D0')
        decays = [['KS0','pi+','pi-'],['KS0','pi+','K-'],
                  ['KS0','K+','pi-'],['KS0','K+','K-']]
        wm = awmFunctor(decays,min,max)
        config = deepcopy(self.config)    
        config.pop('AMAXDOCA_MAX')
        config['ADOCA(2,3)_MAX'] = self.config['AMAXDOCA_MAX']
        protoD2Kshh = self._makeD2X('D2KSHH'+which,['D0 -> KS0 pi+ pi-'],
                                    wm,config,self.ks[which])
        return [subPIDSels(decays,'D2KsHH',which,min,max,[protoD2Kshh])]

    def _makeD2KShhWS(self,which):
        '''Makes D->Kshh WS'''
        min,max = self._massWindow('D0')
        decays = [['KS0','pi+','pi+'],['KS0','pi+','K+'],
                  ['KS0','K+','pi+'],['KS0','K+','K+']]
        wm = awmFunctor(decays,min,max)
        config = deepcopy(self.config)    
        config.pop('AMAXDOCA_MAX')
        config['ADOCA(2,3)_MAX'] = self.config['AMAXDOCA_MAX']
        protoPlus = self._makeD2X('D2KSHHWSPlus'+which,['D0 -> KS0 pi+ pi+'],
                                  wm,config,self.ks[which])
        psels = subPIDSels(decays,'D2KSHHWSPlus',which,min,max,[protoPlus])
        protoMinus = self._makeD2X('D2KSHHWSMinus'+which,['D0 -> KS0 pi- pi-'],
                                   wm,config,self.ks[which])
        msels = subPIDSels(getCCs(decays),'D2KSHHWSMinus',which,min,max,
                           [protoMinus])
        return [MergedSelection('D2KsHHWSBeauty2Charm_%s'%which,
                                RequiredSelections=[psels,msels])]
  
    def _makeD2Pi0hhh(self,which):
        '''Makes D->Pi0hhh'''        
        min,max = self._massWindow('D+')
        decays = [['pi0','pi+','pi+','pi-'],['pi0','pi+','pi+','K-'],
                  ['pi0','K+','pi+','pi-'],['pi0','K+','pi+','K-'],
                  ['pi0','pi+','K+','pi-'],['pi0','pi+','K+','K-'],
                  ['pi0','K+','K+','pi-']]
        wm = awmFunctor(decays,min,max)
        config = deepcopy(self.config)    
        config.pop('AMAXDOCA_MAX')
        config['ADOCA(2,3)_MAX'] = self.config['AMAXDOCA_MAX']
        config['ADOCA(2,4)_MAX'] = self.config['AMAXDOCA_MAX']
        config['ADOCA(3,4)_MAX'] = self.config['AMAXDOCA_MAX']
        protoDp2pi0hhh = self._makeD2X('D+2Pi0HHH_'+which,
                                       ['D+ -> pi0 pi+ pi+ pi-'],
                                       wm,config,self.pi0[which])
        psels = subPIDSels(decays,'D+2Pi0HHH',which,min,max,[protoDp2pi0hhh])
        protoDm2pi0hhh = self._makeD2X('D-2Pi0HHH_'+which,
                                       ['D- -> pi0 pi- pi- pi+'],
                                       wm,config,self.pi0[which])
        msels = subPIDSels(getCCs(decays),'D-2Pi0HHH',which,min,max,
                          [protoDm2pi0hhh])
        return [MergedSelection('D2Pi0HHHBeauty2Charm_%s'%which,
                                RequiredSelections=[psels,msels])]
    
    def _makeD2Pi0hh(self,which):
        '''Makes D->Pi0hh'''
        min,max = self._massWindow('D0')
        decays = [['pi0','pi+','pi-'],['pi0','pi+','K-'],
                  ['pi0','K+','pi-'],['pi0','K+','K-']]
        wm = awmFunctor(decays,min,max)
        config = deepcopy(self.config)    
        config.pop('AMAXDOCA_MAX')
        config['ADOCA(2,3)_MAX'] = self.config['AMAXDOCA_MAX']
        protoD2pi0hh = self._makeD2X('D2Pi0HH_'+which,['D0 -> pi0 pi+ pi-'],
                                     wm,config,self.pi0[which])
        return [subPIDSels(decays,'D2Pi0HH',which,min,max,[protoD2pi0hh])]

    def _makeD2Pi0hhWS(self,which):
        '''Makes D->Pi0hh WS'''
        min,max = self._massWindow('D0')
        decays = [['pi0','pi+','pi+'],['pi0','pi+','K+'],
                  ['pi0','K+','pi+'],['pi0','K+','K+']]
        wm = awmFunctor(decays,min,max)
        config = deepcopy(self.config)    
        config.pop('AMAXDOCA_MAX')
        config['ADOCA(2,3)_MAX'] = self.config['AMAXDOCA_MAX']
        psel = self._makeD2X('D2Pi0HHWSPlus'+which,['D0 -> pi0 pi+ pi+'],wm,
                             config,self.pi0[which])
        psel = subPIDSels(decays,'D2Pi0HHWSPlus',which,min,max,[psel])
        msel = self._makeD2X('D2Pi0HHWSMinus'+which,['D0 -> pi0 pi- pi-'],wm,
                             config,self.pi0[which])
        msel = subPIDSels(getCCs(decays),'D2Pi0HHWSMinus',which,min,max,[msel])
        return [MergedSelection('D2Pi0HHWSBeauty2Charm'+which,
                                RequiredSelections=[psel,msel])]
    
    def _makeD2hhhh(self):
        '''Makes D->hhhh'''
        min,max = self._massWindow('D0')
        decays = [['pi+','pi+','pi-','pi-'],
                  ['pi+','pi+','K-','pi-'],['pi+','pi+','pi-','K-'],
                  ['K+','pi+','pi-','pi-'],['pi+','K+','pi-','pi-'],
                  ['K+','pi+','K-','pi-'],['K+','pi+','pi-','K-'],
                  ['pi+','K+','K-','pi-'],['pi+','K+','pi-','K-']]
        wm = awmFunctor(decays,min,max)
        protoD2hhhh = self._makeD2X('D2HHHH',['D0 -> pi+ pi+ pi- pi-'],wm,
                                    self.config)
        return [subPIDSels(decays,'D2HHHH','',min,max,[protoD2hhhh])]

    def _makeD2hhhhWS(self):
        '''Makes D->hhhh WS'''
        min,max = self._massWindow('D0')
        decays = [['pi+','pi+','pi+','pi+'],
                  ['pi+','pi+','K+','pi+'],['pi+','pi+','pi+','K+'],
                  ['K+','pi+','pi+','pi+'],['pi+','K+','pi+','pi+'],
                  ['K+','pi+','K+','pi+'],['K+','pi+','pi+','K+'],
                  ['pi+','K+','K+','pi+'],['pi+','K+','pi+','K+']]
        wm = awmFunctor(decays,min,max)
        psel = self._makeD2X('D2HHHHWSPlus',['D0 -> pi+ pi+ pi+ pi+'],wm,
                             self.config)
        psel = subPIDSels(decays,'D2HHHHWSPlus','',min,max,[psel])
        msel = self._makeD2X('D2HHHHWSMinus',['D0 -> pi- pi- pi- pi-'],wm,
                             self.config)
        msel = subPIDSels(decays,'D2HHHHWSMinus','',min,max,[msel])
        return [MergedSelection('D2HHHHWSBeauty2Charm',
                                RequiredSelections=[psel,msel])]

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#

class DstarBuilder(object):
    '''Makes D* mesons for Beauty2Charm.'''

    def __init__(self,d,pions,pi0,config,config_pid):
        self.d = d
        self.pions = pions
        self.pi0 = pi0
        self.config = config
        self.d0pi = self._makeDstar2D0pi()
        self.d0pi_pid = [filterPID('Dstar2D0PiPID',self.d0pi,config_pid,2)]
        self.d0pi0_merged = self._makeDstar02D0Pi0('Merged')
        self.d0pi0_resolved = self._makeDstar02D0Pi0('Resolved')
        d0pi_2460 = self._makeDstar24602D0pi()
        self.d0pi_2460_pid = [filterPID('Dstar24602D0PiPID',d0pi_2460,
                                        config_pid,2)]
        dpi_2460 = self._makeD0star24602Dpi()
        self.dpi_2460_pid = [filterPID('D0star24602DPiPID',dpi_2460,
                                       config_pid,2)]

    def _makeHc2Dpi(self,name,massCut,decays,inputs):
        comboCuts = [massCut,LoKiCuts(['AMAXDOCA'],self.config).code()]
        comboCuts = LoKiCuts.combine(comboCuts)
        momCuts = LoKiCuts(['VCHI2DOF','BPVVDCHI2','BPVDIRA'],
                           self.config).code()
        cp = CombineParticles(CombinationCut=comboCuts,MotherCut=momCuts,
                              DecayDescriptors=decays)
        return [Selection(name+'2D0PiBeauty2Charm',Algorithm=cp,
                          RequiredSelections=inputs)]
        
    def _makeDstar2D0pi(self):
        '''Makes D*+ -> D0 pi+ + c.c.'''
        massCut = "(ADAMASS('D*(2010)+') < %(MASS_WINDOW)s) " % self.config
        decays=["D*(2010)+ -> pi+ D0","D*(2010)- -> pi- D0"]
        return self._makeHc2Dpi('Dstar',massCut,decays,self.d.hh+[self.pions])

    def _makeDstar24602D0pi(self):
        '''Makes D*2(2460)+- -> D0 pi+-'''
        massCut = "(ADAMASS('D*_2(2460)+') < 100*MeV) "
        decays=["D*_2(2460)+ -> pi+ D0","D*_2(2460)- -> pi- D0"]
        return self._makeHc2Dpi('Dstar2460',massCut,decays,
                                self.d.kpi_pid+[self.pions])
    
    def _makeD0star24602Dpi(self):
        '''Makes D*2(2460)0 -> D+- pi-+'''
        massCut = "(ADAMASS('D*_2(2460)0') < 100*MeV) "
        decays=["D*_2(2460)0 -> pi+ D-","D*_2(2460)0 -> pi- D+"]
        return self._makeHc2Dpi('D0star2460',massCut,decays,
                                self.d.hhh_cf_pid+[self.pions])

    def _makeDstar02D0X0(self,name,decays,inputs):
        ''' Makes all X+ -> HH selections involving neutrals.'''
        comboCuts = "(ADAMASS('D*(2007)0') < %(MASS_WINDOW)s) " % self.config
        momCuts = "ALL" 
        cp = CombineParticles(CombinationCut=comboCuts,MotherCut=momCuts,
                              DecayDescriptors=decays)
        cp = cp.configurable(name+'Beauty2CharmCombiner')
        cp.ParticleCombiners.update({'':'MomentumCombiner'})
        return Selection('Dstar02D0'+name+'Beauty2Charm',Algorithm=cp,
                         RequiredSelections=self.d.hh+inputs)

    def _makeDstar02D0Pi0(self,pi0):
        decays = ["D*(2007)0 -> D0 pi0"]
        return [self._makeDstar02D0X0('Pi0_'+pi0,decays,self.pi0[pi0])]

#\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\/\#
