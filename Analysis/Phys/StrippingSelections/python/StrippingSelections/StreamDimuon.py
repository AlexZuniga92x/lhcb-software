#
#   Dimuon stripping selections 
#
#  all B decay channels with at least 2 muons inside
#  (with minimal muon ID required)
#      - exclusive selections for: B->J/psi(mm)X, B->K*mumu, B->mumu
#      - J/psi(mm) inclusive for physics (with pre-scaling
#        with increasing Luminosity)
#            
#
#   @author A. Poluektov
#   @date 2009-05-15
#
from Gaudi.Configuration import *
from StrippingConf.StrippingStream import StrippingStream

from StrippingSelections.StrippingDiMuon import StrippingDiMuonConf
from StrippingSelections.StrippingBs2JpsiPhi import StrippingBs2JpsiPhiConf
from StrippingSelections import StrippingBs2JpsiPhiDetached
from StrippingSelections import StrippingBs2JpsiPhiPrescaled
from StrippingSelections import StrippingBd2JpsiKS
from StrippingSelections.StrippingBd2JpsiKstar import StrippingBd2JpsiKstarConf 
from StrippingSelections import StrippingBd2JpsiKstarDetached
from StrippingSelections import StrippingBd2JpsiKstarDetachedR
from StrippingSelections.StrippingBs2Jpsif0 import StrippingBs2Jpsif0Conf
from StrippingSelections import StrippingBiasedBs2JpsiPhi
from StrippingSelections.StrippingBu2JpsiK import StrippingBu2JpsiKConf
from StrippingSelections import StrippingBu2JpsiKDetached
from StrippingSelections import StrippingBs2JpsiPhiForBs2MuMu
from StrippingSelections import StrippingBu2JpsiKDetachedR
from StrippingSelections.StrippingBu2LLK  import StrippingBu2LLKConf
from StrippingSelections.StrippingBd2KstarMuMu import StrippingBd2KstarMuMuConf
from StrippingSelections.StrippingBc2JpsiMuX import StrippingBc2JpsiMuXConf
from StrippingSelections.StrippingBc2JpsiH import StrippingBc2JpsiHConf
from StrippingSelections.StrippingBc2JpsiHDetached import StrippingBc2JpsiHDetachedConf
from StrippingSelections.StrippingBs2PhiMuMu import StrippingBs2PhiMuMuConf

from StrippingSelections.StrippingB2XMuMuSS import StrippingB2XMuMuSSConf
from StrippingSelections.StrippingD2XMuMuSS import StrippingD2XMuMuSSConf


from StrippingSelections.StrippingDKPiMu import StrippingDKPiMuConf

stream = StrippingStream("Dimuon")
stream.appendLines( [ 
		        StrippingDiMuonConf().nominal_line(), 
		        StrippingDiMuonConf().loose_line(), 
                        StrippingDiMuonConf().likesign_line(), 
		        StrippingBs2JpsiPhiConf().nominal_line(),  
		        StrippingBs2JpsiPhiConf().loose_line(),  
		        StrippingBs2JpsiPhiPrescaled.line,  
			StrippingBs2JpsiPhiDetached.line, 
		        StrippingBd2JpsiKS.line1,  
		        StrippingBd2JpsiKS.line2,  
		        StrippingBd2JpsiKstarConf().nominal_line(),  
		        StrippingBd2JpsiKstarConf().loose_line(),  
		        StrippingBd2JpsiKstarDetached.line,  
		        StrippingBd2JpsiKstarDetachedR.line,  
		        StrippingBd2KstarMuMuConf().line_for_nominal_low(),  
		        StrippingBd2KstarMuMuConf().line_for_nominal_med(),  
		        StrippingBd2KstarMuMuConf().line_for_nominal_high(),  
		        StrippingBd2KstarMuMuConf().Early_SignalLine(),  
		        StrippingBd2KstarMuMuConf().Early_SameSignLine(),  
		        StrippingBd2KstarMuMuConf().Early_WideKstarLine(),  
		        StrippingBd2KstarMuMuConf().Early_NoMuIDLine(),  
		        StrippingBd2KstarMuMuConf().Early_eMuLine(),
                        StrippingBd2KstarMuMuConf().Early_DiMuonLine(),
                        StrippingBd2KstarMuMuConf().simplestLTLine(),
			StrippingBs2PhiMuMuConf().Bs2PhiMuMuLine(),
                    	StrippingBs2PhiMuMuConf().Bs2f0MuMuLine(),
		        StrippingBs2Jpsif0Conf().nominal_line(),  
		        StrippingBs2Jpsif0Conf().loose_line(),  
                    	StrippingBs2Jpsif0Conf().nominalKst_line(), 
		        StrippingBiasedBs2JpsiPhi.line,  
		        StrippingBu2JpsiKConf().nominal_line(),  
		        StrippingBu2JpsiKConf().loose_line(),  
		        StrippingBu2JpsiKDetached.line,  
		        StrippingBu2JpsiKDetachedR.line,
                        StrippingBs2JpsiPhiForBs2MuMu.line,
		        StrippingBu2LLKConf().mmK(),
                        StrippingBc2JpsiMuXConf().MuMu(),
                        StrippingBc2JpsiHConf().MuMu(),
                        StrippingBc2JpsiHDetachedConf().MuMu(),
                        StrippingDiMuonConf().biasedDiMuon_line()

                        ## Same sign lines from  P. Urquijo, 23-05-10
                        , StrippingB2XMuMuSSConf().PiSS_line()
                        , StrippingB2XMuMuSSConf().PiOS_line()
                        , StrippingB2XMuMuSSConf().KSS_line()
                        , StrippingB2XMuMuSSConf().KOS_line()
                        , StrippingD2XMuMuSSConf().PiSS_line()
                        , StrippingD2XMuMuSSConf().PiOS_line()
                        , StrippingD2XMuMuSSConf().KSS_line()
                        , StrippingD2XMuMuSSConf().KOS_line()

                        
                        , StrippingDKPiMuConf().line_DKpimu()
                        ] )
###
# Stripping lines for Bs2MuMu
# attempt to write them in recommended format 
#
from StrippingSelections.StrippingBs2MuMuNoMuID import Bs2MuMuNoMuIDConf
bs2mm = Bs2MuMuNoMuIDConf("Bs2MuMu",Bs2MuMuNoMuIDConf.config_default)
stream.appendLines(bs2mm.lines)
