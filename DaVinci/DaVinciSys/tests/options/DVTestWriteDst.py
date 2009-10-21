########################################################################
#
# $Id: DVTestWriteDst.py,v 1.7 2009-10-21 10:14:15 pkoppenb Exp $
#
# Options for a DaVinci job creating DSTs
#
# It looks for a J/psi and then saves the events in DSTs
# according to the number of PVs.
#
# @author Patrick Koppenburg
# @date 2009-02-03
#
##############################################################################
from Gaudi.Configuration import *
from Configurables import DaVinci, DaVinciWriteDst
##############################################################################
from Configurables import GaudiSequencer, FilterDesktop, DeterministicPrescaler, PrintDecayTree
#
# Get a J/psi
#
jpsi = FilterDesktop('MyJpsi')
jpsi.InputLocations = [ 'StdLooseJpsi2MuMu' ]
jpsi.Code = "ALL"
#
# get DoD muons
#
from PhysSelPython.Wrappers import Selection, DataOnDemand, SelectionSequence
from Configurables import SelDSTWriter

MyLooseJpsi = DataOnDemand('DODStdLooseJpsi2MuMu',
                           Location = 'Phys/StdLooseJpsi2MuMu')

#
# Loop over three overlapping mass zones
#
MassRanges = [ [ 2600, 3200 ], [ 3050, 3150 ],  [ 3000, 3500 ] ]

for i in MassRanges :
    lm = i[0]
    hm = i[1]
    ln = str(lm)
    hn = str(hm)
    name = ln+"_"+hn

    # define the prescale to have a constant (assumed flat) level of background
    prescale = DeterministicPrescaler("Prescale_"+name, AcceptFraction = 100./(hm-lm))
    filter   = FilterDesktop("Jpsi_"+name, Code = "(MM>"+ln+") & (MM<"+hn+")")
    printer  = PrintDecayTree("Print_"+name, InputLocations = [ "Jpsi_"+name ])
    
    SelJpsi = Selection('SelJpsi_'+name,
                        Algorithm = filter,
                        RequiredSelections = [ MyLooseJpsi ])
    
    SeqJpsi = SelectionSequence('SeqJpsi_'+name
                                , TopSelection = SelJpsi
                                , EventPreSelector = [ prescale ]
                                , PostSelectionAlgs = [ printer ])
    
    dstWriter = SelDSTWriter("JpsiDSTWriter_"+name,
                             SelectionSequences = [SeqJpsi],
                             SaveCandidates = True,
                             CopyMCTruth = True)
    seq = dstWriter.sequence()
    # seq.Members.append(printer) # does not work

    DaVinci().UserAlgorithms += [ seq ]

##############################################################################
DaVinci().EvtMax = 100
DaVinci().PrintFreq = 1 
# DaVinci().SkipEvents = 0
DaVinci().DataType = "MC09" 
# DaVinci().Simulation   = False
########################################################################
MessageSvc().Format = "% F%60W%S%7W%R%T %0W%M"
##############################################################################
# Bs->J/psi Phi MC09
EventSelector().Input   = [
    "   DATAFILE='castor://castorlhcb.cern.ch:9002/?svcClass=lhcbdata&castorVersion=2&path=/castor/cern.ch/grid/lhcb/MC/MC09/DST/00004879/0000/00004879_00000001_1.dst' TYP='POOL_ROOTTREE' OPT='READ'" ] 
