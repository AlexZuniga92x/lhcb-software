# $Id: StreamDimuon.py,v 1.3 2010-09-08 09:05:18 gcowan Exp $
#
#   Dimuon stripping selection
#
#   all physics channels with dimuon in the final state
#
#   @author Greig Cowan and Tom Blake 
#   @date 2010-09-07
# 
#   DO NOT MODIFY THIS FILE WITHOUT PERMISSION OF THE AUTHORS
#

from Gaudi.Configuration import *
from StrippingConf.StrippingStream import StrippingStream

stream = StrippingStream("Dimuon")

#
# Lines from beta_s WG
# Olivier Leroy, Wouter Hulsebergen and Greig Cowan
#

from StrippingSelections.StrippingB2JpsiXforBeta_s import B2JpsiXLines
stream.appendLines( B2JpsiXLines )

from StrippingSelections.StrippingB2Psi2SX import B2Psi2SXLines
stream.appendLines( B2Psi2SXLines )

from StrippingSelections.StrippingLambdab2JpsiLambda import StrippingLambdab2JpsiLambdaConf
stream.appendLines( [ StrippingLambdab2JpsiLambdaConf().line_LL(),
                      StrippingLambdab2JpsiLambdaConf().line_DD() ])

from StrippingSelections.StrippingBs2Jpsif0 import StrippingBs2Jpsif0Conf
stream.appendLines( [ StrippingBs2Jpsif0Conf().nominal_line(),
                      StrippingBs2Jpsif0Conf().nominalKst_line()  ])

from StrippingSelections.StrippingBiasedBs2JpsiPhi import line
stream.appendLines( [ line ] )

#
# Lines for the RD WG
# U. Egede
#

# Phillip Urquijo
from StrippingSelections.StrippingB2XMuMuSS import StrippingB2XMuMuSSConf
stream.appendLines( [ StrippingB2XMuMuSSConf().PiSS_line() ] )
stream.appendLines( [ StrippingB2XMuMuSSConf().PiOS_line() ] )
stream.appendLines( [ StrippingB2XMuMuSSConf().KSS_line() ] )
stream.appendLines( [ StrippingB2XMuMuSSConf().KOS_line() ] )

from StrippingSelections.StrippingD2XMuMuSS import StrippingD2XMuMuSSConf
stream.appendLines( [ StrippingD2XMuMuSSConf().PiSS_line() ] )
stream.appendLines( [ StrippingD2XMuMuSSConf().PiOS_line() ] )
stream.appendLines( [ StrippingD2XMuMuSSConf().KSS_line() ] )
stream.appendLines( [ StrippingD2XMuMuSSConf().KOS_line() ] )

# Patrick Koppenburg
from StrippingSelections.StrippingBu2LLK import StrippingBu2LLKConf
from StrippingSelections.StrippingBu2LLK import defaultConfig as Bu2LLKConfig
Bu2LLKConf = StrippingBu2LLKConf( "Bu2mmK", config = Bu2LLKConfig )
stream.appendLines( [ Bu2LLKConf._mmKLine() ] )

#
# Lines from Bs2mumu WG
# Johannes Albrecht
#

from StrippingSelections.StrippingBs2MuMuLines import Bs2MuMuLinesConf
bs2mm = Bs2MuMuLinesConf("Bs2MuMu", Bs2MuMuLinesConf.config_default )
stream.appendLines( bs2mm.lines )

#
# Lines from BdKstarMuMu RD WG
# Tom Blake, Jeremy Dickens, Patrick Koppenburg
#

from StrippingSelections.StrippingBd2KstarMuMuTriggered import StrippingBd2KstarMuMuConf
from StrippingSelections.StrippingBd2KstarMuMuTriggered import defaultLines as Bd2KstarMuMuLines
from StrippingSettings.Stripping11.LineConfigDictionaries import Bd2KstarMuMuConfig

Bd2KstarMuMuConf = StrippingBd2KstarMuMuConf( config = Bd2KstarMuMuConfig, activeLines = Bd2KstarMuMuLines )
stream.appendLines( Bd2KstarMuMuConf.lines() )

from StrippingSelections.StrippingBd2KstarMuMuLT import StrippingBd2KstarMuMuLTConf
from StrippingSelections.StrippingBd2KstarMuMuLT import defaultConfig as Bd2KstarMuMuLTConfig
Bd2KstarMuMuLTConf = StrippingBd2KstarMuMuLTConf( config = Bd2KstarMuMuLTConfig )
stream.appendLines( Bd2KstarMuMuLTConf.lines() )



# Liming Zhang
from StrippingSelections.StrippingBs2PhiMuMu import StrippingBs2PhiMuMuConf
stream.appendLines( [ StrippingBs2PhiMuMuConf().Bs2PhiMuMuLine() ] )
stream.appendLines( [ StrippingBs2PhiMuMuConf().Bs2f0MuMuLine() ] )


# Vanya Belayev
from StrippingSelections.StrippingTriMuons import StrippingTriMuonsConf
stream.appendLines (  StrippingTriMuonsConf({}).lines() )


#
# Lines from Flavour WG
# Patrick Robbe, Jibo He
#


from StrippingSelections.StrippingDiMuonNew import DiMuonConf
FullDSTDiMuonConf = DiMuonConf( config = DiMuonConf.config_default )
stream.appendLines( FullDSTDiMuonConf.Lines )

# Nicola Mangiafave
from StrippingSelections.StrippingBu2KJpsiPiPi import StrippingBu2KJpsiPiPiConf
stream.appendLines( [
    StrippingBu2KJpsiPiPiConf().loose_line(),
    StrippingBu2KJpsiPiPiConf().X3872_line(),
    StrippingBu2KJpsiPiPiConf().Psi2S_line()
    ] )


# Jibo He 
from StrippingSelections.StrippingBc2JpsiMuX import StrippingBc2JpsiMuXConf
stream.appendLines( [ StrippingBc2JpsiMuXConf().MuMu() ] )

from StrippingSelections.StrippingBc2JpsiH import StrippingBc2JpsiHConf
stream.appendLines( [ StrippingBc2JpsiHConf().MuMu()  ] )

from StrippingSelections.StrippingBc2JpsiHDetached import StrippingBc2JpsiHDetachedConf
stream.appendLines( [ StrippingBc2JpsiHDetachedConf().MuMu() ] )



## Micro DST streams 
