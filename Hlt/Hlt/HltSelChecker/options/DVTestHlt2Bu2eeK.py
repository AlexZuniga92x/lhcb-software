### @Id: $
 #
 #  Test file for HLT Bu->LLK* selection
 #
 #  @author P. Koppenburg
 #  @date 2007-07-23
###
from Gaudi.Configuration import *
from Configurables import CheckSelResult, L0Filter, AlgorithmCorrelationsAlg
#--------------------------------------------------------------
signal = "Bu2eeK"
#
# True filter criterion
#
from Configurables import FilterTrueTracks, MCDecayFinder
importOptions( "$HLTSELCHECKERROOT/options/FilterTrueTracks.py")
FilterTrueTracks().addTool(MCDecayFinder)
FilterTrueTracks().MCDecayFinder.Decay =  "{[B+ -> ^K+ ^mu+ ^mu- {,gamma}{,gamma}{,gamma}{,gamma}{,gamma}]cc, [B+ -> ^K+ ^e+ ^e- {,gamma}{,gamma}{,gamma}{,gamma}{,gamma}]cc}"
#
from Configurables import GaudiSequencer
#
# Set the following to false if you want only events with a signal
# fully reconstructed in the HLT
#
GaudiSequencer("SeqTrueSignalTracks").IgnoreFilterPassed = True
#
# Overwrite input - uncomment to run HLT on True signal only
#
# importOptions( "$HLTSELCHECKERROOT/options/OverwriteWithTruth.py")
#
# Monitoring
#
moni = GaudiSequencer("Hlt2MonitorSeq")
moni.IgnoreFilterPassed = True
moni.Context = "HLT"
importOptions( "$HLTSELCHECKERROOT/options/Hlt2Correlations.py")
importOptions( "$HLTSELCHECKERROOT/options/Hlt2EventTuple.py" )
###
 # Tuple
###
from Configurables import DecayTreeTuple
importOptions( "$HLTSELCHECKERROOT/options/Hlt2DecayTreeTuple.py")
DecayTreeTuple("Hlt2DecayTreeTuple").InputLocations = ["Hlt2Bu2eeKSignalBu"]
DecayTreeTuple("Hlt2DecayTreeTuple").Decay = "[B+ -> (^J/psi(1S) => ^e+ ^e-) ^K+]cc"
#
# Configuration
#
from Configurables import DaVinci
DaVinci().EvtMax = -1
DaVinci().PrintFreq = 1
DaVinci().SkipEvents = 0
DaVinci().Hlt = True                # Both Hlt levels
DaVinci().Hlt2Requires = 'L0'                  # Ignore Hlt1 in 2
DaVinci().ReplaceL0BanksWithEmulated = False   # Redo L0
DaVinci().DataType = "DC06" 
DaVinci().Simulation = True 
DaVinci().TupleFile =  "HLT-"+signal+".root"
DaVinci().HistogramFile = "DVHlt2-"+signal+".root"
DaVinci().MoniSequence += [ moni, DecayTreeTuple("Hlt2DecayTreeTuple") ]
DaVinci().Input = [ "DATAFILE='PFN:castor:/castor/cern.ch/user/d/dijkstra/Selections-DC06/Bu2eeK-lum2-fixedL0.dst' TYP='POOL_ROOTTREE' OPT='READ'" ]
DaVinci().RedoMCLinks = True 
MessageSvc().Format = "% F%60W%S%7W%R%T %0W%M"

