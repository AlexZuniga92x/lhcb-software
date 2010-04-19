## @package Escher
#  High level configuration tools for Escher
#  @author Johan Blouw <Johan.Blouw@physi.uni-heidelberg.de>
#  @date   15/08/2008

__version__ = "$Id: Configuration.py,v 1.19 2010-04-19 10:30:19 jblouw Exp $"
__author__  = "Johan Blouw <Johan.Blouw@physi.uni-heidelberg.de>"

from Gaudi.Configuration  import *
import GaudiKernel.ProcessJobOptions
from Configurables import ( LHCbConfigurableUser, LHCbApp, RecSysConf, TrackSys,
                            ProcessPhase, GaudiSequencer, DstConf, TAlignment, VeloAlignment,
                            CountingPrescaler )
from RecSys.Configuration import RecMoniConf

## @class Escher
#  Configurable for Escher application
#  @author Johan Blouw <Johan.Blouw@physi.uni-heidelberg.de>
#  @date   15/08/2008
class Escher(LHCbConfigurableUser):

    ## Possible used Configurables
    #__used_configurables__ = [ TAlignment, VeloAlignment, TrackSys, RecSysConf, LHCbApp, DstConf ]

    __used_configurables__ = [ TrackSys, RecSysConf, RecMoniConf, LHCbApp, DstConf, TAlignment ]
    ## Default main sequences for real and simulated data
    DefaultSequence = [ CountingPrescaler("EscherPrescaler")
                        , "ProcessPhase/Init"
			, "ProcessPhase/Reco"
			, "ProcessPhase/Moni"
                        , GaudiSequencer("AlignSequence") 
			]

    
    # Steering options
    __slots__ = {
         "EvtMax"		: -1       # Maximum number of events to process
       , "SkipEvents"		: 0        # events to skip
       , "PrintFreq"		: 300      # The frequency at which to print event numbers
       , "DataType"   		: "2008"   # Data type, can be ['DC06','2008']
       , "WithMC"		: False    # set to True to use MC truth
       , "Simulation"		: False    # set to True to use SimCond
       , "InputType"		: "DST"    # or "DIGI" or "ETC" or "RDST" or "DST"
       , "OutputType"		: "NONE"   # or "RDST" or "NONE". Also forwarded to RecSys
       , "PackedOutput"		: True     # Flag whether or not to use packed containers
       , "NoWarnings"		: False    # suppress all messages with MSG::WARNING or below 
       , "TrackContainer" 	: "Long"   # Tracktype to be used for alignment (Long, Seed, VeloTT...)
       , "Detectors" 		: ["VELO", "TT", "IT", "OT", "MUON", "Tr", "Vertex"] # detectors to be aligned
       , "AlignmentLevel" 	: "layers" # level of alignment, stations, layers, quarters, modules, sensors...
       , "Constraints"          : []       # list of constraints
       , "DoF"                  : [0, 0, 0, 0, 0, 0]       # list of degrees of freedom to align
       , "DatasetName"		: ""       # string used to build file names
       , "DDDBtag"		: ""       # Tag for DDDB. Default as set in DDDBConf for DataType
       , "CondDBtag"		: ""       # Tag for CondDB. Default as set in DDDBConf for DataType
       , "UseOracle"		: False    # if False, use SQLDDDB instead
       , "MainSequence"		: []       # The default main sequence, see self.DefaultSequence
       , "InitSequence"		: ["Reproc", "Escher", "Calo"] # default init sequence
       , "AlignSequence"	: []
       , "Kalman" 		: False    # run the kalman filter type alignment
       , "Millepede"		: False    # run the Millepede type alignment
       , "skipBigCluster"       : False    # used for cosmic alignment, removes OT cluster > 2 hits
       , "OutputLevel" 		: 3        # 
       , "Incident"     	:  ""      # for Millepede style alignment, there are two incident handles: GlobalMPedeFit and Converged
                                           # for Kalman style alignment, there is a handle: UpdateConstants.
        # Following are options forwarded to RecSys
       , "RecoSequence"   	: [] # The Sub-detector reconstruction sequencing. See RecSys for default
       , "MoniSequence"         : ["VELO","Tr", "OT","ST"]
       , "SpecialData"    	: [] # Various special data processing options. See KnownSpecialData for all options
       , "Context"		: "Offline" # The context within which to run
        }



    def defineGeometry(self):
        # DIGI is always simulation, as is usage of MC truth!
        if self.getProp( "WithMC" ) or self.getProp( "InputType" ).upper() == 'DIGI':
            self.setProp( "Simulation", True )

        # Delegate handling to LHCbApp configurable
        self.setOtherProps(LHCbApp(),["DataType","CondDBtag","DDDBtag","Simulation"])
        # specify the use of the oracle database
        if self.getProp("UseOracle"):
            from Configurables import CondDB
            CondDB(UseOracle = True)
        
    def defineEvents(self):
        # Delegate handling to LHCbApp configurable
        self.setOtherProps(LHCbApp(),["EvtMax","SkipEvents"])

    def defineOptions(self):
        log.info("Defining options!")

        inputType = self.getProp( "InputType" ).upper()
        if inputType not in [ "MDF", "DST", "DIGI", "ETC", "RDST" ]:
            raise TypeError( "Invalid inputType '%s'"%inputType )

        outputType = self.getProp( "OutputType" ).upper()
        if outputType not in [ "NONE", "DST", "RDST" ]:
            raise TypeError( "Invalid outputType '%s'"%outputType )

        self.configureSequences( )

        self.configureInput( inputType )

        self.configureOutput( outputType )

        # ROOT persistency for histograms
        importOptions('$STDOPTS/RootHist.opts')
        from Configurables import RootHistCnv__PersSvc
        RootHistCnv__PersSvc('RootHistCnv').ForceAlphaIds = True

        # Use a default histogram file name if not already set
        if not hasattr( HistogramPersistencySvc(), "OutputFile" ):
            histosName   = self.getProp("DatasetName")
            if (self.evtMax() > 0): histosName += '-' + str(self.evtMax()) + 'ev'
            histosName += '-histos.root'
            HistogramPersistencySvc().OutputFile = histosName

    def configureSequences(self):
        escherSeq = GaudiSequencer("EscherSequencer")
        #escherSeq.Context = self.getProp("Context")
        ApplicationMgr().TopAlg = [ escherSeq ]
        mainSeq = self.getProp("MainSequence")
        if len( mainSeq ) == 0:
            self.MainSequence = self.DefaultSequence
        mainSeq = self.MainSequence
        escherSeq.Members += mainSeq
        
        ProcessPhase("Init").DetectorList += self.getProp("InitSequence")
        ProcessPhase("Init").Context = self.getProp("Context")
        from Configurables import RecInit, TrackSys
        log.info("Setting up alignment sequence")
        recInit = RecInit( name = "EscherInit",
                           PrintFreq = self.getProp("PrintFreq"))
        GaudiSequencer("InitEscherSeq").Members += [ recInit ]
        alignSeq = GaudiSequencer("AlignSequence")
        # if the patter reco is not run, we need the DataOnDemand svc
        # so that e.g. the track container(s) is unpacked:
        if not GaudiSequencer("RecoTrSeq").getProp("Enable"):
	    ApplicationMgr().ExtSvc += [ "DataOnDemandSvc" ]
	    stConf( EnableUnpack = True )

        if  self.getProp("Millepede") :
            self.setProp("Kalman", False )
            log.info("Using Millepede type alignment!")
            self.setProp("Incident", "GlobalMPedeFit")
	    ta = TAlignment()
            ta.Method = "Millepede"
            ta.Sequencer = GaudiSequencer("MpedeAlignSeq")
            alignSeq.Members.append( ta.Sequencer )

        if self.getProp("Kalman") :
	    log.info("Using Kalman style alignment!")
            self.setProp("Incident", "UpdateConstants")
            ta = TAlignment()
            ta.Method = "Kalman"
            ta.Sequencer = GaudiSequencer("KalmanAlignSeq")
            alignSeq.Members.append( ta.Sequencer )

		  
    def configureInput(self, inputType):
        """
        Tune initialisation according to input type
        """

        # POOL Persistency
        importOptions("$GAUDIPOOLDBROOT/options/GaudiPoolDbRoot.opts")

        # By default, Escher only needs to open one input file at a time
        # Only set to zero if not previously set to something else.
        if not IODataManager().isPropertySet("AgeLimit") : IODataManager().AgeLimit = 0

        if inputType in [ "XDST", "DST", "RDST", "ETC" ]:
            # Kill knowledge of any previous Brunel processing
            from Configurables import ( TESCheck, EventNodeKiller )
            InitReprocSeq = GaudiSequencer( "InitReprocSeq" )
            if ( self.getProp("WithMC") and inputType in ["XDST","DST"] ):
                # Load linkers, to kill them (avoid appending to them later)
                InitReprocSeq.Members.append( "TESCheck" )
                TESCheck().Inputs = ["Link/Rec/Track/Best"]
            InitReprocSeq.Members.append( "EventNodeKiller" )
            EventNodeKiller().Nodes = [ "pRec", "Rec", "Raw", "Link/Rec" ]

        if inputType == "ETC":
            from Configurables import  TagCollectionSvc
            ApplicationMgr().ExtSvc  += [ TagCollectionSvc("EvtTupleSvc") ]
            # Read ETC selection results into TES for writing to DST
            IODataManager().AgeLimit += 1

        if inputType in [ "MDF", "RDST", "ETC" ]:
            # In case raw data resides in MDF file
            EventPersistencySvc().CnvServices.append("LHCb::RawDataCnvSvc")

        # Get the event time (for CondDb) from ODIN
        from Configurables import EventClockSvc
        EventClockSvc().EventTimeDecoder = "OdinTimeDecoder";



    def configureOutput(self, dstType):
        """
        Set up output stream
        """
        if dstType in [ "DST", "RDST" ]:
            writerName = "DstWriter"
            packedDST  = self.getProp( "PackedOutput" )
            # Do not pack DC06 DSTs, for consistency with existing productions
            if self.getProp("DataType") == "DC06": packedDST = False

            dstWriter = OutputStream( writerName )
            dstWriter.RequireAlgs += ["Reco"] # Write only if Rec phase completed

            # Set a default output file name if not already defined in the user job options
            if not hasattr( dstWriter, "Output" ):
                dstWriter.Output  = "DATAFILE='PFN:" + self.outputName() + "' TYP='POOL_ROOTTREE' OPT='REC'"

            # Define the file content
            DstConf().Writer     = writerName
            DstConf().DstType    = dstType
            DstConf().EnablePack = packedDST

            from Configurables import TrackToDST
            if dstType == "DST":
                # Sequence for altering DST content
                ProcessPhase("Output").DetectorList += [ "DST" ]
                # Filter Track States to be written
                trackFilter = TrackToDST()
            else:
                # Sequence for altering content of rDST compared to DST
                ProcessPhase("Output").DetectorList += [ "L0", "DST" ]
                # Filter Track States to be written
                trackFilter = TrackToDST("TrackToRDST")
                trackFilter.veloStates = ["ClosestToBeam"]
                trackFilter.longStates = ["ClosestToBeam"]
                trackFilter.TTrackStates = ["FirstMeasurement"]
                trackFilter.downstreamStates = ["FirstMeasurement"]
                trackFilter.upstreamStates = ["ClosestToBeam"]
                
            GaudiSequencer("OutputDSTSeq").Members += [ trackFilter ]

            if packedDST:
                # Add the sequence to pack the DST containers
                packSeq = GaudiSequencer("PackDST")
                DstConf().PackSequencer = packSeq
                GaudiSequencer("OutputDSTSeq").Members += [ packSeq ]

        # Always write an ETC if ETC input
        if self.getProp( "InputType" ).upper() == "ETC":
            etcWriter = TagCollectionSvc("EvtTupleSvc")
            ApplicationMgr().ExtSvc.append(etcWriter)
            ApplicationMgr().OutStream.append("GaudiSequencer/SeqTagWriter")
            importOptions( "$ESCHEROPTS/DefineETC.opts" )
            if not hasattr( etcWriter, "Output" ):
               etcWriter.Output = [ "EVTTAGS2 DATAFILE='" + self.getProp("DatasetName") + "-etc.root' TYP='POOL_ROOTTREE' OPT='RECREATE' " ]

        # Do not print event number at every event (done already by Brunel)
        EventSelector().PrintFreq = -1
        CountingPrescaler("EscherPrescaler").PrintFreq = self.getProp( "PrintFreq" )
        # Modify printout defaults
        if self.getProp( "NoWarnings" ):
            importOptions( "$ESCHEROPTS/SuppressWarnings.opts" )

    def outputName(self):
        """
        Build a name for the output file, based in input options
        """
        outputName = self.getProp("DatasetName")
        if ( self.evtMax() > 0 ): outputName += '-' + str(self.evtMax()) + 'ev'
        outputType = self.getProp("OutputType").lower()
        return outputName + '.' + outputType

    def evtMax(self):
        return LHCbApp().evtMax()

    ## Apply the configuration
    def __apply_configuration__(self):
        
        GaudiKernel.ProcessJobOptions.PrintOff()
        GaudiKernel.ProcessJobOptions.PrintOn()
        log.info("Initializing sequences!")
        self.setOtherProps(RecSysConf(),["SpecialData","Context",
                                         "OutputType","DataType"])

        if self.isPropertySet("RecoSequence") :
            self.setOtherProp(RecSysConf(),"RecoSequence")

        # there is a bug in setOtherProps, so we cannot use it to set the MoniSequence.
        self.setOtherProps(RecMoniConf(),["Context","DataType"])
        RecMoniConf().MoniSequence = self.getProp("MoniSequence")
        self.setOtherProps(TAlignment(),["DatasetName"])
        
        self.defineGeometry()
        self.defineEvents()
        self.defineOptions()

        # Use TimingAuditor for timing, suppress printout from SequencerTimerTool
        from Configurables import (ApplicationMgr,AuditorSvc,SequencerTimerTool)
        ApplicationMgr().ExtSvc += [ 'ToolSvc', 'AuditorSvc' ]
        ApplicationMgr().AuditAlgorithms = True
        AuditorSvc().Auditors += [ 'TimingAuditor' ] 
        SequencerTimerTool().OutputLevel = 4

        log.info( self )
        log.info( LHCbApp() )
        log.info( RecSysConf() )
        log.info( TrackSys() )
        log.info( RecMoniConf() )
        log.info( TAlignment() )
        GaudiKernel.ProcessJobOptions.PrintOff()
