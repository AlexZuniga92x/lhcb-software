##############################################################################
# File for running Gauss with MC11a configuration and beam conditions as in
# production for 2011 data (3.5 TeV beams, nu=2, spill-over at 50 ns)
#
# Syntax is:
#   gaudirun.py Gauss-MC11a.py <someInputJobConfiguration>.py
##############################################################################

from Gaudi.Configuration import *
importOptions("$APPCONFIGOPTS/Gauss/Beam3500GeV-md100-MC11-nu2-50ns.py")

##############################################################################
# Database tags must be set and are defined in a separate file
##############################################################################
from Configurables import LHCbApp
LHCbApp().DDDBtag   = "MC11-20111102"
LHCbApp().CondDBtag = "sim-20111111-vc-md100"
