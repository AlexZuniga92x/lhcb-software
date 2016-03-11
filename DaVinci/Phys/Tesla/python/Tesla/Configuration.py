from Gaudi.Configuration import *
from LHCbKernel.Configuration import *
from Configurables import LHCbApp
import GaudiKernel.ProcessJobOptions
from Configurables import ( LHCbConfigurableUser, LHCbApp, RecSysConf, TrackSys,
GaudiSequencer, DstConf, L0Conf, CondDB, GlobalRecoConf, RawEventJuggler, DecodeRawEvent,
RawEventFormatConf, LumiAlgsConf, InputCopyStream, RecombineRawEvent)
from Configurables import TrackAssociator, ChargedPP2MC, P2MCPFromProtoP
from Configurables import PackParticlesAndVertices

class Tesla(LHCbConfigurableUser):
    __used_configurables__ = [ LHCbApp, LumiAlgsConf, RawEventJuggler, DecodeRawEvent, RawEventFormatConf, DstConf, RecombineRawEvent, PackParticlesAndVertices, TrackAssociator, ChargedPP2MC ]

    __slots__ = {
            "EvtMax"		: -1    	# Maximum number of events to process
          , "SkipEvents"	: 0		# Skip to event
          , "Simulation"	: True 		# True implies use SimCond
          , "DataType"		: '2012' 	# Data type, can be [ 'DC06','2008' etc...]
          , "InputType"		: 'DST' 	# Input type, can be [ 'DST','LDST' etc...]
          , "DDDBtag" 		: 'default' 	# default as set in DDDBConf for DataType
          , "CondDBtag" 	: 'default' 	# default as set in DDDBConf for DataType
          , 'Persistency' 	: '' 		# None, Root or Pool?
          , 'OutputLevel' 	: 4 		# Er, output level
          , 'DuplicateCheck' 	: False		# Do we want to add the algorithm to check for duplicates
          , 'KillInputTurbo' 	: False		# If rerunning Tesla, kill input Turbo locations
          , "outputFile" 	: 'Tesla.dst' 	# output filename
          , 'WriteFSR'    	: False 	# copy FSRs as required
          , 'PV'	        : "Offline"     # Associate to the PV chosen by the HLT or the offline one
          , 'VertRepLoc'	: "Hlt1"        # Do we use the maker from Hlt1 or Hlt2
          , 'TriggerLines'	: ["Hlt2IncPhi"]# Which trigger line
          , 'Mode'	        : "Offline"     # "Online" (strip unnecessary banks and run lumi algorithms) or "Offline"?
          , 'Pack'	        : True          # Do we want to pack the objects?
          , 'RawFormatVersion'	: 0.2           # Which banks form the Turbo stream
          , 'Monitors'	        : []            # Histogram monitors to run in the job
          , 'Histogram'  	: ""            # Name of histogram file
          }
    _propertyDocDct ={
            "EvtMax"		: "Maximum number of events to process, default all"
            , "SkipEvents"	: "Events to skip, default     0"
            , "Simulation"	: "True implies use SimCond"
            , "DataType"	: "Data type, can be [ 'DC06','2008' ...]"
            , "InputType"	: "Input type, can be [ 'DST','LDST' ...]"
            , "DDDBtag" 	: "Databse tag, default as set in DDDBConf for DataType"
            , "CondDBtag" 	: "Databse tag, default as set in DDDBConf for DataType"
            , 'Persistency' 	: "Root or Pool?"
            , 'OutputLevel' 	: "Output level"
            , 'DuplicateCheck' 	: "Add the test for duplicates?"
            , 'KillInputTurbo' 	: "Are we rerunning Tesla"
            , "outputFile" 	: 'output filename, automatically selects MDF or InputCopyStream'
            , 'WriteFSR'    	: 'copy FSRs as required'
            , 'PV'     	        : 'Associate to the PV chosen by the HLT or the offline one'
            , 'VertRepLoc'	: 'Do we use the maker from Hlt1 or Hlt2'
            , 'TriggerLines'    : 'Which trigger line to process'
            , 'Mode'     	: '"Online" (strip unnecessary banks and run lumi algorithms) or "Offline"?'
            , 'Pack'     	: 'Do we want to pack the object?'
            , 'RawFormatVersion': 'Which banks form the Turbo stream'
            , 'Monitors'        : 'Histogram monitors to run in the job'
            , 'Histogram'       : 'File name for histogram file'
            }

    writerName = "DstWriter"
    teslaSeq = GaudiSequencer("TeslaSequence")
    base = "Turbo/"
    
    def _safeSet(self,conf,param):
        for p in param:
            if (not self.isPropertySet(p)) or conf.isPropertySet(p):
                continue
            conf.setProp(p,self.getProp(p))
        
    def _configureDataOnDemand(self) :
        if 'DataOnDemandSvc' in ApplicationMgr().ExtSvc : 
            ApplicationMgr().ExtSvc.pop('DataOnDemandSvc')
        else: 
            from Configurables import DataOnDemandSvc
            dod = DataOnDemandSvc()
        if dod not in ApplicationMgr().ExtSvc :
            ApplicationMgr().ExtSvc.append( dod ) 
    
    def _configureHistos(self):
        monitorSeq = GaudiSequencer('TelaMonitors')
        monitorSeq.IgnoreFilterPassed = True 
        monitorSeq.Members += self.getProp('Monitors')
        self.teslaSeq.Members += [ monitorSeq ]
        #
        ApplicationMgr().HistogramPersistency = "ROOT"
        
        if ( self.getProp("Histogram") != "" ):
            HistogramPersistencySvc().OutputFile = self.getProp("Histogram")
    
    def _configureTruth(self,assocpp) :
        tesROOT="/Event/"+self.base
        protocont = self.base + "Protos"
        trackcont = self.base + "Tracks"
        assoctr = TrackAssociator("TurboAssocTr")
        assoctr.OutputLevel = self.getProp('OutputLevel')
        assoctr.TracksInContainer = trackcont
        assocpp.TrackLocations += [ trackcont ]
        assocpp.InputData += [ protocont ]
        # Add it to a sequence
        seq = GaudiSequencer('TurboSeqT2MC')
        seq.Members += [assoctr]
        return seq
    
    def _configureDigitsTruth(self) :
        from Configurables import CaloDigit2MCLinks2Table
        assocdigits = CaloDigit2MCLinks2Table("TurboDigitAssoc")
        return 

    def _configureClustersAndProtosTruth(self,digits) :
        retSeq = GaudiSequencer("NeutralTruth")
        tesROOT="/Event/"+self.base
        clusterTabLoc = tesROOT + self.base + "Relations/CaloClusters"
        clustLoc = tesROOT + "CaloClusters"
        inputLoc = tesROOT + "Protos"

        from Configurables import CaloClusterMCTruth
        assoccluster = CaloClusterMCTruth("TurboClusterAssoc")
        assoccluster.OutputLevel = self.getProp('OutputLevel')
        assoccluster.Input = digits
        assoccluster.Output = clusterTabLoc
        assoccluster.Clusters+=[clustLoc]
        
        from Configurables import NeutralPP2MC
        protoTabLoc=tesROOT + "Relations/Turbo/NeutralPP2MC"
        assocneutral = NeutralPP2MC("TurboNeutralPP2MC")
        assocneutral.InputData += [ inputLoc ]
        assocneutral.OutputLevel = self.getProp('OutputLevel')
        assocneutral.OutputTable = protoTabLoc
        assocneutral.MCCaloTable = clusterTabLoc
        
        retSeq.Members += [assoccluster,assocneutral]
        return protoTabLoc, retSeq
    
    def _unpackMC(self):
        DataOnDemandSvc().NodeMap['/Event/MC']   = 'DataObject'
        DataOnDemandSvc().AlgMap["MC/Particles"] = "UnpackMCParticle"
        DataOnDemandSvc().AlgMap["MC/Vertices"]  = "UnpackMCVertex"

    def _configureForOnline(self):
        #
        DecodeRawEvent().DataOnDemand=False
        writer=InputCopyStream( self.writerName )
        DstConf().setProp("SplitRawEventOutput", self.getProp("RawFormatVersion"))
        
        # Use RawEventJuggler to create the Turbo stream raw event format
        tck = "0x409f0045" # DUMMY
        TurboBanksSeq=GaudiSequencer("TurboBanksSeq")
        RawEventJuggler().TCK=tck
        RawEventJuggler().Input="Moore"
        RawEventJuggler().Output=self.getProp("RawFormatVersion")
        RawEventJuggler().Sequencer=TurboBanksSeq
        RawEventJuggler().WriterOptItemList=writer
        RawEventJuggler().KillExtraNodes=True
        RawEventJuggler().KillExtraBanks=True
        RawEventJuggler().KillExtraDirectories = True
        self.teslaSeq.Members += [TurboBanksSeq]
        
        # Begin Lumi configuration
        lumiSeq = GaudiSequencer("LumiSeq")
        #
        # Add ODIN decoder to LumiSeq ***
        from DAQSys.Decoders import DecoderDB
        CreateODIN=DecoderDB["createODIN"].setup()
        #********************************
        #
        # Main algorithm config
        lumiCounters = GaudiSequencer("LumiCounters")
        lumiCounters.Members+=[CreateODIN]
        lumiSeq.Members += [ lumiCounters ]
        LumiAlgsConf().LumiSequencer = lumiCounters
        LumiAlgsConf().OutputLevel = self.getProp('OutputLevel')
        LumiAlgsConf().InputType = "MDF"
        #
        # Filter out Lumi only triggers from further processing, but still write to output
        from Configurables import HltRoutingBitsFilter
        physFilter = HltRoutingBitsFilter( "PhysFilter", RequireMask = [ 0x0, 0x4, 0x0 ] )
        lumiFilter = HltRoutingBitsFilter( "LumiFilter", RequireMask = [ 0x0, 0x2, 0x0 ] )
        lumiSeq.Members += [ lumiFilter, physFilter ]
        lumiSeq.ModeOR = True
        #
        from Configurables import RecordStream
        FSRWriter = RecordStream( "FSROutputStreamDstWriter")
        FSRWriter.OutputLevel = INFO
        #
        # Sequence to be executed if physics sequence not called (nano events)
        notPhysSeq = GaudiSequencer("NotPhysicsSeq")
        notPhysSeq.ModeOR = True
        notPhysSeq.Members = [ physFilter ]
        writer.AcceptAlgs += ["LumiSeq","NotPhysicsSeq"]
        self.teslaSeq.Members += [lumiSeq, notPhysSeq]
    
    def _configureOutput(self):
        fname = self.getProp('outputFile')
        if not fname : return
        
        #retrieve the persistency
        persistency=None
        if hasattr(self, "Persistency"):
            if self.getProp("Persistency") is not None:
                persistency=self.getProp("Persistency")
        
        from GaudiConf import IOExtension, IOHelper
        iox=IOExtension(persistency)
        
        writer = InputCopyStream(self.writerName)
        writer.OutputLevel = self.getProp('OutputLevel')

        seq=GaudiSequencer('TeslaReportAlgoSeq')
        writer.RequireAlgs += ['TeslaReportAlgoSeq']
        
        # make use of PrintDuplicates algorithm
        from Configurables import PrintDuplicates
        dplic = PrintDuplicates("TurboDuplicates")
        dplic.OutputLevel = self.getProp('OutputLevel')
        
        #
        if self.getProp('Mode') is "Online":
            from DAQSys.Decoders import DecoderDB
            Hlt1VertexReportsDecoder=DecoderDB["HltVertexReportsDecoder/Hlt1VertexReportsDecoder"].setup()
            Hlt2VertexReportsDecoder=DecoderDB["HltVertexReportsDecoder/Hlt2VertexReportsDecoder"].setup()
            Hlt2SelReportsDecoder=DecoderDB["HltSelReportsDecoder/Hlt2SelReportsDecoder"].setup()
            seq.Members += [ Hlt1VertexReportsDecoder, Hlt2VertexReportsDecoder, Hlt2SelReportsDecoder ]
        #
        lines = self.getProp('TriggerLines')
        trig1 = self._configureReportAlg(lines)
        seq.Members+=[trig1]
        for l in lines:
            # IF NOT PACKING NEED TO SET THE LOCATIONS IN THE REPORT ALGORITHM
            if not self.getProp('Pack'):
                writer.OptItemList+=[ self.base + l + "/Particles#99" 
                        , self.base + l + "/Vertices#99"
                        , self.base + l + "/_ReFitPVs#99"
                        , self.base + l + "/Particle2VertexRelations#99"
                        ]
            dplic.Inputs+=[self.base + l + "/Particles"]
        if not self.getProp('Pack'):
            writer.OptItemList+=[
                self.base + "Protos#99"
                ,self.base + "Tracks#99"
                ,self.base + "RichPIDs#99"
                ,self.base + "MuonPIDs#99"
                ,self.base + "CaloHypos#99"
                ,self.base + "CaloClusters#99"
                ]
        
        inputType = self.getProp('InputType')
        if (inputType=='XDST') or (inputType=='LDST'):
            assocpp=ChargedPP2MC("TurboProtoAssocPP")
            assocpp.OutputLevel = self.getProp('OutputLevel')
            assocpp.TrackLocations = []
            assocpp.InputData = []
            assocpp.VetoEmpty=True
            tesROOT="/Event/"+self.base
            outputPPLoc = tesROOT+'Relations/Turbo/'
            assocpp.RootInTES=tesROOT
            
            truthSeq = self._configureTruth(assocpp)
            seq.Members += [ truthSeq ]
            if not self.getProp('Pack'):
                writer.OptItemList+=[
                        outputPPLoc + 'Protos' + '#99'
                        ]
            seq.Members+=[assocpp]
            
            NeutralProtoSeq = GaudiSequencer("NeutralTruthSequencer")
            
            ## Add the digits associator
            outputDigiLoc = tesROOT + "Digi2MCP"
            from Configurables import CaloDigit2MCLinks2Table
            assocdigits = CaloDigit2MCLinks2Table("TurboDigitAssoc")
            assocdigits.OutputLevel = self.getProp('OutputLevel')
            outputiDigiLoc = tesROOT + "Relations/CaloDigits"
            assocdigits.Output = outputDigiLoc
            
            ## Add the cluster associator
            ## Finally configure the neutral PP2MC associator
            NeutralProtoSeq.Members+=[assocdigits]
            clustLoc = self.base + "CaloClusters"
            protoLoc = self.base + "Protos"
            self._configureDigitsTruth()
            protoneutral, retSeq = self._configureClustersAndProtosTruth(outputDigiLoc)
            NeutralProtoSeq.Members+=[retSeq]
            if not self.getProp('Pack'):
                writer.OptItemList+=[
                        protoneutral + '#99'
                        ]
            seq.Members+=[NeutralProtoSeq]
        
        # Add shared places to writer
        if not self.getProp('Pack'):
            writer.OptItemList+=[
                    self.base + "Primary#99"
                    , self.base + "Rec/Summary#99"
                    ]

        if self.getProp('Pack'):
            packer = PackParticlesAndVertices( name = "TurboPacker",
                    InputStream        = "/Event"+"/"+self.base.rstrip('/'),
                    DeleteInput        = False,
                    EnableCheck        = False,
                    AlwaysCreateOutput = True)
            seq.Members +=[packer]
            
            from Configurables import PackCaloHypo as PackCaloHypos
            hypopacker = PackCaloHypos( name = "PackCaloHypos",
                    AlwaysCreateOutput = True,
                    DeleteInput        = False,
                    OutputLevel        = self.getProp('OutputLevel'),
                    InputName          = self.base+"CaloHypos",
                    OutputName         = self.base+"pRec/neutral/Hypos" )
            from Configurables import DataPacking__Pack_LHCb__CaloClusterPacker_ as PackCaloClusters
            clusterpacker = PackCaloClusters( name = "PackCaloClusters",
                    AlwaysCreateOutput = True,
                    DeleteInput        = False,
                    OutputLevel        = self.getProp('OutputLevel'),
                    InputName          = self.base+"CaloClusters",
                    OutputName         = self.base+"pRec/neutral/Clusters" )
            seq.Members +=[hypopacker,clusterpacker]
            
            writer.OptItemList+=[
                    self.base+"pPhys/Particles#99"
                    ,self.base+"pPhys/Vertices#99"
                    ,self.base+"pPhys/RecVertices#99"
                    ,self.base+"pPhys/Relations#99"
                    ,self.base+"pPhys/PP2MCPRelations#99"
                    ,self.base+"pPhys/PartToRelatedInfoRelations#99"
                    ,self.base+"pRec/Track/Custom#99"
                    ,self.base+"pRec/Muon/CustomPIDs#99"
                    ,self.base+"pRec/Rich/CustomPIDs#99"
                    ,self.base+"pRec/ProtoP/Custom#99"
                    ,self.base+"pRec/neutral/Hypos#99"
                    ,self.base+"pRec/neutral/Clusters#99"
                    ,self.base+"Rec/Summary#99"
                    ]
            packer.OutputLevel = self.getProp('OutputLevel')
        
        
        self.teslaSeq.Members += [ seq ]
        if self.getProp('DuplicateCheck'):
            self.teslaSeq.Members += [dplic]
        IOHelper(persistency,persistency).outStream(fname,writer,writeFSR=self.getProp('WriteFSR'))
    
    def _configureReportAlg(self,lines):
        from Configurables import TeslaReportAlgo
        trig1 = TeslaReportAlgo("TeslaReportAlgo")
        trig1.OutputPrefix=self.base
        trig1.PV=self.getProp('PV')
        if self.getProp('VertRepLoc')=="Hlt1":
            trig1.VertRepLoc="Hlt1/VertexReports"
        else:
            trig1.VertRepLoc="Hlt2/VertexReports"
        trig1.PVLoc=self.base+"Primary"
        trig1.TriggerLines=lines
        trig1.OutputLevel=self.getProp('OutputLevel')
        return trig1
    
    def __apply_configuration__(self):
        #GaudiKernel.ProcessJobOptions.PrintOff()
        
        from GaudiKernel.Configurable import ConfigurableGeneric as RFileCnv
        RFileCnv('RFileCnv').GlobalCompression = "LZMA:6"

        ############## Set other properties ###########
        self._safeSet( LHCbApp(), ['EvtMax','SkipEvents','Simulation', 'DataType' , 'CondDBtag','DDDBtag'] )
        ApplicationMgr().AppName="Tesla, utilising DaVinci"
        #
        if self.getProp('Mode') is "Online":
            self.setProp('WriteFSR',True)
            self._configureForOnline()
        else:
            DecodeRawEvent().DataOnDemand=True
            RecombineRawEvent()
            if self.getProp('Simulation')==True:
                self._unpackMC()
        #
        self._configureOutput()
        #
        from Configurables import EventSelector
        EventSelector().PrintFreq = 1000

        # Add monitors if they are there
        if len(self.getProp('Monitors'))>0:
            self._configureHistos()

        if self.getProp('KillInputTurbo'):
            enk = EventNodeKiller('KillTurbo')
            enk.Nodes = [ "Turbo" ]
            ApplicationMgr().TopAlg.insert( 0,  enk.getFullName() ) 

        ApplicationMgr().TopAlg+=[self.teslaSeq]
