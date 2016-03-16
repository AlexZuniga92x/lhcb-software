## @file
#  HLT2 lines for decays involving one or more phi mesons
#
#  @author Jon Harrison jonathan.harrison@manchester.ac.uk, Matthew Kenzie matthew.william.kenzie@cern.ch
#=============================================================================
""" HLT2 lines for decays involving one or more phi mesons
	
"""
#=============================================================================
__author__  = "Jon Harrison jonathan.harrison@manchester.ac.uk, Matthew Kenzie matthew.william.kenzie@cern.ch"

from GaudiKernel.SystemOfUnits import GeV, MeV, mm

from Hlt2Lines.Utilities.Hlt2LinesConfigurableUser import Hlt2LinesConfigurableUser
class PhiLines(Hlt2LinesConfigurableUser) :
  __slots__ = {'_stages' : {},
               'Prescale' : {},
               'Postscale' : {},
               'PhiIncPhi' : {'TisTosSpec' : "Hlt1IncPhi.*Decision%TOS" ,
                           'KaonPT'     : 800 * MeV ,
                           'KaonIPS'    : 6 ,
                           'KaonPID'    : 0 ,
                           'TrChi2DOF'  : 5,
                           'PhiPT'      : 1800 * MeV,
                           'PhiVCHI2'   : 20 ,
                           'PhiMassWin' : 20 * MeV},
               'PhiBs2PhiPhi': {'TisTosSpec' : "Hlt1B2PhiPhi_LTUNB.*Decision%TOS" ,
                            },
               'GoodKs' : {'KS_MASS_WINDOW'   : 20 * MeV,
                           'KS_PT_MIN'        : 400 * MeV,
                           'KS_VCHI2NDOF_MAX' : 4,
                           'GHOSTPROB_MAX'    :       0.35,
                           'KS_FD_MIN'        :      10.0 * mm},
               'PhiPhi2KsKs' : {'Phi_MASS_WINDOW'   : 70 * MeV,
                             'Phi_PT_MIN'        : 800 * MeV,
                             'Phi_DOCACHI2_MAX'  : 20,
                             'Phi_VCHI2NDOF_MAX' : 6},
               'PhiPhi2KsKsD0Ctrl' : {'D0_MASS_WINDOW'   : 70 * MeV,
                                   'D0_PT_MIN'        : 800 * MeV,
                                   'D0_DOCACHI2_MAX'  : 20,
                                   'D0_VCHI2NDOF_MAX' : 6},
              }

  def stages(self, nickname = ""):
    if hasattr(self, '_stages') and self._stages:
      if nickname:
        return self._stages[nickname]
      else:
        return self._stages

    from Stages import IncPhiFilter, Bs2PhiPhiCombiner, Phi2KsKsCombiner, D02KsKsCombiner
    self._stages = {'PhiIncPhi'         : [IncPhiFilter('PhiIncPhi')],
                    'PhiBs2PhiPhi'      : [Bs2PhiPhiCombiner('PhiBs2PhiPhi')],
                    'PhiPhi2KsKs'       : [Phi2KsKsCombiner('PhiPhi2KsKs')],
                    'PhiPhi2KsKsD0Ctrl' : [D02KsKsCombiner('PhiPhi2KsKsD0Ctrl')]}

    if nickname:
      return self._stages[nickname]
    else:
      return self._stages

  def __apply_configuration__(self) :
    from HltLine.HltLine import Hlt2Line
    from Configurables import HltANNSvc
    stages = self.stages()
    for (nickname, algos) in self.algorithms(stages):
      linename = nickname
      Hlt2Line(linename, prescale = self.prescale, algos = algos, postscale = self.postscale)
