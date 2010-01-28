########################################################################
# $Id: DVTutorial_4.py,v 1.7 2010-01-28 09:47:45 pkoppenb Exp $
#
# Options for exercise 4
#
# @author Patrick Koppenburg
# @author Juan Palacios
# @date 2008-06-03
#
########################################################################
from Gaudi.Configuration import *
#######################################################################
#
# import the SelectionSequence
from DaVinci4.solutions.Bs2JpsiPhi import SeqBs2JpsiPhi
# get the GaudiSequencer with everything we need
seq = SeqBs2JpsiPhi.sequence()
#######################################################################
#
# Configure the application
#
from Configurables import DaVinci
DaVinci().HistogramFile = "DVHistos_4.root"    # Histogram file
DaVinci().EvtMax = 1000                          # Number of events
DaVinci().DataType = "MC09"                    # Default is "MC09"
DaVinci().Simulation   = True                  # It's MC
#
# Add our own stuff
#
DaVinci().UserAlgorithms = [ seq ]
DaVinci().MainOptions  = ""                    # None
########################################################################
#
# To run in shell :
# gaudirun.py solutions/DaVinci4/DVTutorial_4.py options/MC09-Bs2JpsiPhi.py
#
########################################################################
