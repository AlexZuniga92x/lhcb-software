##############################################################################
# $Id: Jpsi2MuPi.py,v 1.2 2009-05-29 17:30:19 pkoppenb Exp $
#
# Author: Patrick Koppenburg <patrick.koppenburg@cern.ch>
#
##############################################################################
from Configurables import GaudiSequencer
JpsiSeq = GaudiSequencer("JpsiMonitorSeq")
##############################################################################
#
# Make a J/psi with only one side mu-IDed
#
from Configurables import CombineParticles
Jpsi2MuPi = CombineParticles("Jpsi2MuPi")
Jpsi2MuPi.Inputs = [ "Phys/StdLooseMuons/Particles", "Phys/StdNoPIDsPions/Particles" ]
Jpsi2MuPi.DecayDescriptor = "[J/psi(1S) -> mu+ pi-]cc"
# a good muon and any pion
Jpsi2MuPi.DaughtersCuts = { "pi+" : "(P>1*GeV)",
                            "mu+" : "(P>10*GeV) & (PT>1*GeV)" }
Jpsi2MuPi.CombinationCut = "(ADAMASS('J/psi(1S)')<100*MeV)"
Jpsi2MuPi.MotherCut = "(ALL)"
JpsiSeq.Members += [ Jpsi2MuPi ]
##############################################################################
#
# Plot quantities
#
from Configurables import ParticleMonitor
plotter = ParticleMonitor("Jpsi2MuPiMoni")
plotter.Inputs = [ "Phys/Jpsi2MuPi/Particles" ]
plotter.PeakCut = "(ADMASS('J/psi(1S)')<5*MeV)"
plotter.SideBandCut = "(ADMASS('J/psi(1S)')>20*MeV)"
plotter.PlotTools = [ "MassPlotTool","MomentumPlotTool",
                      "CombinedPidPlotTool",
                      "RichPlotTool","CaloPlotTool","MuonPlotTool" ]
JpsiSeq.Members += [ plotter ]
##############################################################################
#
#
#
from Configurables import DaVinci
DaVinci().MoniSequence += [ JpsiSeq ] 
