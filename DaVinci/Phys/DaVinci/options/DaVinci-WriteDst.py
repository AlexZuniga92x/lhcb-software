########################################################################
#
# $Id: DaVinci-WriteDst.py,v 1.8 2009-02-24 14:07:16 pkoppenb Exp $
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
from Configurables import DaVinci, DaVinciWriteDst
##############################################################################
from Configurables import GaudiSequencer, FilterDesktop, PhysDesktop, DeterministicPrescaler, PrintDecayTree
#
# Get a J/psi
#
jpsi = FilterDesktop('MyJpsi')
jpsi.addTool(PhysDesktop)
jpsi.PhysDesktop.InputLocations = [ 'StdLooseJpsi2MuMu' ]
# @todo This does not work properly
# jpsi.CloneTree = True # Write the muons as well
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
    seq = GaudiSequencer("Seq_"+name)
    d = DeterministicPrescaler("Prescale_"+name)
    d.AcceptFraction = 100./(hm-lm)  # same background level in each
    j = jpsi.clone("Jpsi_"+name)
    j.Code = "(MM>"+ln+") & (MM<"+hn+")"
    p = PrintDecayTree("Print_"+name)
    p.addTool(PhysDesktop)
    p.PhysDesktop.InputLocations = [ "Jpsi_"+name ]
    seq.Members += [ d, j, p ] 
    DaVinciWriteDst().DstFiles[ "Jpsi_"+name+".dst" ] = seq
    DaVinciWriteDst().Items += [ "/Event/Phys/Jpsi_"+name+"#2" ] # will not save StdLooseMuos
##############################################################################
# get write out sequence. Pass it to DaVinci()
#
seq =  DaVinciWriteDst().dstSequence()
DaVinci().UserAlgorithms = [ seq ]
#
##############################################################################
DaVinci().EvtMax = 500
DaVinci().PrintFreq = 1 
# DaVinci().SkipEvents = 0
DaVinci().DataType = "2008" # Default is "DC06"
# DaVinci().Simulation   = False
# DaVinci().Input = []
# DaVinci().MainOptions  = "$DAVINCIROOT/options/DaVinci.py" # Default is ""
##############################################################################
