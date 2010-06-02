##############################################################################
# $Id: DVMonitorDst.py,v 1.4 2010/03/17 18:44:33 pkoppenb Exp $
#
# syntax: gaudirun.py $DAVINCIMONITORSROOT/options/DVMonitorDst.py
#
# Author: Patrick Koppenburg <patrick.koppenburg@cern.ch>
#
##############################################################################
from DaVinci.Configuration import *
from Gaudi.Configuration import *
##############################################################################
#
# the stuff
#
importOptions( "$DAVINCIMONITORSROOT/options/DaVinciMonitoring.py") 
##############################################################################
#
# Histograms
#
DaVinci().HistogramFile = "DVMonitors.root"
##############################################################################
#
# Most of this will be configured from Dirac
#
##############################################################################
DaVinci().EvtMax = -1
DaVinci().DataType = "2009" # Default is "DC06"
DaVinci().Simulation = True
DaVinci().InputType = "RDST"
EventSelector().Input   = [
    "DATAFILE='PFN:castor:/castor/cern.ch/user/p/pkoppenb/Stripping/Brunel-100000ev.rdst' TYP='POOL_ROOTTREE' OPT='READ'"]
    
