#
#   Electroweak stripping selections 
#            
#
#   @author J. Keaveney
#   @date 2010-01-24
#
from Gaudi.Configuration import *
from StrippingConf.StrippingStream import StrippingStream

from StrippingSelections import StrippingZ02MuMu
from StrippingSelections import StrippingWMu
from StrippingSelections import StrippingDY2MuMu
from StrippingSelections import StrippingZ02ee
from StrippingSelections import StrippingWe
from StrippingSelections import StrippingDY2ee
from StrippingSelections import StrippingDisplVertices
from StrippingSelections import StrippingHighPtJets

from StrippingSelections import StrippingMuMuSS

stream = StrippingStream("EW")
stream.appendLines( [ 
		        StrippingZ02MuMu.line,
                        StrippingZ02MuMu.lineloose,
                        StrippingZ02MuMu.lineNoPIDs,
		        StrippingWMu.line,
                        StrippingWMu.lineloose,
                        StrippingWMu.line_ps,
                        StrippingWMu.lineloose_ps,
                        StrippingWMu.lineNoPIDs_ps,
		        StrippingDY2MuMu.line1,
      		        StrippingDY2MuMu.line1_ps,
                        StrippingDY2MuMu.line2,
		        StrippingDY2MuMu.line3,
		        StrippingDY2MuMu.line4,
                        StrippingZ02ee.line,
                        StrippingWe.line,
                        StrippingWe.line_ps,
                        StrippingDY2ee.line1,
                        StrippingDY2ee.line2,
		        StrippingDY2ee.line3,
		        StrippingDY2ee.line4, 
                        StrippingDisplVertices.line1,  
                        StrippingDisplVertices.line2,
                        StrippingHighPtJets.line1
                        # Lines for same sign combinations
                        , StrippingMuMuSS.line1
                        , StrippingMuMuSS.line2
                        , StrippingMuMuSS.line3
                        , StrippingMuMuSS.line4
                        ] )
