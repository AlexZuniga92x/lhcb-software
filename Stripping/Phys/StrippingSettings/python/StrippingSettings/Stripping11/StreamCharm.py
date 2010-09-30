# $Id: StreamCharm.py,v 1.4 2010-09-08 09:05:18 gcowan Exp $
#
#   Charm stripping selection
#
#   all physics channels with hadronic final states from charm decay
#
#   @author Greig Cowan and Tom Blake 
#   @date 2010-09-07
# 
#   DO NOT MODIFY THIS FILE WITHOUT PERMISSION OF THE AUTHORS
#

from Gaudi.Configuration import *
from StrippingConf.StrippingStream import StrippingStream

stream = StrippingStream("Charm")

#
# Lines from CP Charm WG
# Marco Gersabeck and Patrick Spradlin
#
from StrippingSelections.StrippingD2hh import StrippingD2hhConf
stream.appendLines( StrippingD2hhConf().lines() )

from StrippingSelections.StrippingPromptCharm import StrippingPromptCharmConf
_conf = { "D0Prescale"    : 1.00 ,
          "DplusPrescale" : 1.00 }
Lines = StrippingPromptCharmConf( config = _conf )
stream.appendLines ( Lines.lines() )

from StrippingSelections.StrippingDstarPromptWithD02HH import StrippingDstarPromptWithD02HHConf
stream.appendLines( StrippingDstarPromptWithD02HHConf().linesDstarOnly() )

from StrippingSelections.StrippingD2hhh_conf import StrippingD2hhhConf
stream.appendLines ( [ StrippingD2hhhConf().stripD2PPP(),
  StrippingD2hhhConf().stripD2KPP(),
  StrippingD2hhhConf().stripD2KKP(),
  StrippingD2hhhConf().stripD2KPPos(),
  StrippingD2hhhConf().stripD2KKK(),
  StrippingD2hhhConf().stripD2hhh_inc()
])

from StrippingSelections.StrippingDstarD2KShh import StrippingDstarD2KShhConf
stream.appendLines ( StrippingDstarD2KShhConf().MakeLines() )

from StrippingSelections.StrippingD2KS0h_KS02PiPi import StrippingD2KS0h_KS02PiPiConf
stream.appendLines ( [ StrippingD2KS0h_KS02PiPiConf().D2KS0h_KS02PiPi() ] )

#
# Lines from RD Charm 
# Walter Bonevito 
#
from StrippingSelections.StrippingDstarD02xx import  StrippingDstarD02xxConf
stream.appendLines( StrippingDstarD02xxConf().lines() )

#
# Lines from Gamma from trees
# Tim Gershon, Anton Poluektov, Andrea Contu
#
from StrippingSelections.StrippingDstar_D2KPiPi0 import lineWS, lineRS
stream.appendLines( [ lineWS, lineRS ] )

#
# Lines from Flavour WG
# This line is taking ~0.2%. This is too high.
from StrippingSelections.StrippingCcbar2Ppbar import StrippingCcbar2PpbarConf
stream.appendLines( [ StrippingCcbar2PpbarConf().Nominal_Line() ] )

#
# Lines from Semileptonic group
# Rob Lambert
#
# Marta Calvi, Stefania Vecchi
from StrippingSelections.StrippingDstarVeryLooseWithD02HH import StrippingDstarVeryLooseWithD02HHConf
stream.appendLines( [ StrippingDstarVeryLooseWithD02HHConf().line() ] )

# Liming Zhang
from StrippingSelections.StrippingD0ForBXX import StrippingD0forBXXConf
stream.appendLines( [ StrippingD0forBXXConf().D02KPiforBXXLine()
		    , StrippingD0forBXXConf().D02K3PiforBXXLine() ] )
from StrippingSelections.StrippingDForBSemi import StrippingDforBSemiConf
stream.appendLines( [ StrippingDforBSemiConf().DpforBSemiLine()
		    , StrippingDforBSemiConf().DsforBSemiLine()
        	    , StrippingDforBSemiConf().LcforBSemiLine() ] )

