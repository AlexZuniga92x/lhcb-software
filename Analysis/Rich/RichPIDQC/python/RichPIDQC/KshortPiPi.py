
## @package RichRecQC
#  RICH PID Calibration and Monitoring
#  @author Chris Jones  (Christopher.Rob.Jones@cern.ch)
#  @date   10/02/2009

__version__ = "$Id: KshortPiPi.py,v 1.6 2009-07-06 16:02:19 jonrob Exp $"
__author__  = "Chris Jones <Christopher.Rob.Jones@cern.ch>"

from LHCbKernel.Configuration import *
        
## @class KshortPiPiConf
#  Configurable for RICH Ds -> phi(KK) pi PID monitoring
#  @author Chris Jones  (Christopher.Rob.Jones@cern.ch)
#  @author Andrew Powell
#  @date   03/05/2009
class KshortPiPiConf(LHCbConfigurableUser) :

    ## Possible used Configurables
    __used_configurables__ = [ ]
    
    ## Steering options
    __slots__ = {
        "Sequencer"   : None    # The sequencer to add the calibration algorithms too
        ,"MCChecks"   : False
        ,"MakeNTuple" : False
        }

    ## Configure Ds -> Phi Pi selection
    def __apply_configuration__(self) :

        from Configurables import ( GaudiSequencer,
                                    CombineParticles, FilterDesktop )

        if not self.isPropertySet("Sequencer") :
            raise RuntimeError("ERROR : Sequence not set")
        seq = self.getProp("Sequencer")

        # Filter Pi Tracks
        pionFilterName = "RichKsSelPiFilter"
        pionfilter = FilterDesktop(pionFilterName)
        pionfilter.InputLocations = [ "Phys/StdNoPIDsPions" ]
        pionfilter.Code = "(P > 2*GeV) & (MIPCHI2DV(PRIMARY) > 30) & (TRCHI2DOF < 5) & (ISLONG)"

        # Make the KS0
        ks02pipiName = "RichKsToPiPiSel"
        ks02pipi = CombineParticles(ks02pipiName)
        ks02pipi.InputLocations = [ "Phys/" + pionFilterName ]
        ks02pipi.DecayDescriptor = "KS0 -> pi+ pi-"
        ks02pipi.CombinationCut = "(ADAMASS('KS0') < 200*MeV) & (AMAXDOCA('') < 0.6*mm)"
        ks02pipi.MotherCut = "(ADMASS('KS0') < 100*MeV) & (VFASPF(VCHI2/VDOF) < 10) & (MIPDV(PRIMARY) < 0.75) & (BPVVDCHI2 > 150)  & (MIPCHI2DV(PRIMARY) < 100) & ( ADWM( 'Lambda0' , WM( 'p+' , 'pi-') ) > 8*MeV ) & ( ADWM( 'Lambda0' , WM( 'pi+' , 'p~-') ) > 8*MeV )"

        # Add selection algs to the sequence
        seq.Members += [pionfilter,ks02pipi]

        # Particle Monitoring plots
        from Configurables import ParticleMonitor
        plotter = ParticleMonitor(ks02pipiName+"Plots")
        plotter.InputLocations = [ "Phys/"+ks02pipiName ]
        plotter.PeakCut     = "(ADMASS('KS0')<7*MeV)"
        plotter.SideBandCut = "(ADMASS('KS0')>7*MeV)"
        plotter.PlotTools = [ "MassPlotTool","MomentumPlotTool",
                              "CombinedPidPlotTool",
                              "RichPlotTool","CaloPlotTool","MuonPlotTool" ]
        seq.Members += [ plotter ]

        # MC Performance checking ?
        if self.getProp("MCChecks") :

            from Configurables import ParticleEffPurMoni

            #mcPerfPi = ParticleEffPurMoni("StdNoPIDsPionsMCPerf")
            #mcPerfPi.InputLocations = ["Phys/StdNoPIDsPions"]
            #mcPerfPi.OutputLevel = DEBUG
            #seq.Members += [mcPerfPi]
 
            #mcPerfPiFilt = ParticleEffPurMoni(pionFilterName+"MCPerf")
            #mcPerfPiFilt.InputLocations = ["Phys/"+pionFilterName]
            #seq.Members += [mcPerfPiFilt]
             
            mcPerfD = ParticleEffPurMoni(ks02pipiName+"MCPerf")
            mcPerfD.InputLocations = ["Phys/"+ks02pipiName]
            seq.Members += [mcPerfD]
                    
        # Ntuple ?
        if self.getProp("MakeNTuple") : pass
        
