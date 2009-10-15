### @file DVTestHlt2Dstar.opts
 #
 #  Test file for HLT D*->D0(hh)pi selection
 #
 #  @author P. Koppenburg
 #  @date 2007-07-20
###
#--------------------------------------------------------------
signal = "SecondaryDstar"
#
#
#
from Gaudi.Configuration import *
from Configurables import HltCorrelations, FilterTrueTracks, MCDecayFinder, GaudiSequencer, DecayTreeTuple, CheckSelResult
#
# Preselection
#
importOptions( "$STDOPTS/PreloadUnits.opts")
# BROKEN # importOptions( "$B2DSTARXROOT/options/DVPreselDstarNoPID.opts")
#
# True filter criterion 
#
importOptions( "$HLTSELCHECKERROOT/options/FilterTrueTracks.py")
FilterTrueTracks().addTool(MCDecayFinder)
FilterTrueTracks().MCDecayFinder.Decay =  "{[D*(2010)+ -> (D0 -> ^K- ^pi+ {,gamma}{,gamma}{,gamma}) ^pi+  {,gamma}{,gamma}]cc, [D*(2010)+ -> (D0 -> ^K+ ^pi- {,gamma}{,gamma}{,gamma}) ^pi+  {,gamma}{,gamma}]cc, [D*(2010)+ -> (D0 -> ^pi+ ^pi- {,gamma}{,gamma}{,gamma}) ^pi+  {,gamma}{,gamma}]cc, [D*(2010)+ -> (D0 -> ^K+ ^K- {,gamma}{,gamma}{,gamma}) ^pi+  {,gamma}{,gamma}]cc}"
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
#
# Tuple tool
#
#importOptions( "$HLTSELCHECKERROOT/options/Hlt2DecayTreeTuple.py")
#DecayTreeTuple("Hlt2DecayTreeTuple").InputLocations = ["Hlt2DstarFilter"]
#DecayTreeTuple("Hlt2DecayTreeTuple").Decay = "[D*(2010)+ -> (^D0 -> ^K- ^pi+) ^pi+]cc" # only Cabibbo-favoured ? 
#
# Configuration
#
from Configurables import DaVinci
DaVinci().EvtMax = -1
DaVinci().Hlt = True                # Both Hlt levels
DaVinci().Hlt2Requires = 'L0'                  # Ignore Hlt1 in 2
DaVinci().ReplaceL0BanksWithEmulated = False   # Redo L0
DaVinci().DataType = "DC06" 
DaVinci().Simulation = True 
DaVinci().TupleFile =  "HLT-"+signal+".root"
DaVinci().HistogramFile = "DVHlt2-"+signal+".root"
DaVinci().MoniSequence += [ moni ]#, DecayTreeTuple("Hlt2DecayTreeTuple") ]
DaVinci().Input = [
  "DATAFILE='PFN:castor:/castor/cern.ch/user/s/spradlin/DC06_DST/DC06-phys-v4-lumi2/11463000-Bd_DstX_2body/SelDstar2D0Pi_D02HH.00.dst' TYP='POOL_ROOTTREE' OPT='READ'" ]
