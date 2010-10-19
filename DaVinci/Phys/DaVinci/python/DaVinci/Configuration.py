"""
High level configuration tools for DaVinci
"""
__version__ = "$Id: Configuration.py,v 1.125 2010-09-10 11:31:19 jpalac Exp $"
__author__ = "Juan Palacios <juan.palacios@nikhef.nl>"

from LHCbKernel.Configuration import *
from GaudiConf.Configuration import *
from Configurables import GaudiSequencer
from Configurables import ( LHCbConfigurableUser, LHCbApp, PhysConf, AnalysisConf,
                            HltConf, DstConf, L0Conf, LumiAlgsConf )
from LumiAlgs.LumiIntegratorConf import LumiIntegratorConf
import GaudiKernel.ProcessJobOptions


def isNewCondDBTag(tag, reference_date = '20100414') :
    tag = tag.upper()
    if tag == 'DEFAULT' or tag == '' : return False
    date_start = tag.find('20')
    if date_start == -1 : return False
    date_end = date_start + 8
    date = tag[date_start : date_end]
    return date > reference_date

class DaVinci(LHCbConfigurableUser) :
    
    __slots__ = {
        # Application Configuration : sent to LHCbApp and Gaudi
        "EvtMax"             :  -1             # Number of events to analyse
        , "SkipEvents"         :   0             # Number of events to skip at beginning for file
        , "PrintFreq"          : 100             # The frequency at which to print event numbers
        , "DataType"           : ''              # Data type, can be ['DC06','2008','2009', 'MC09', '2010'] Forwarded to PhysConf. MUST be set.
        , "Simulation"         : False           # set to True to use SimCond. Forwarded to PhysConf
        , "DDDBtag"            : ""              # Tag for DDDB. Default as set in DDDBConf for DataType
        , "CondDBtag"          : ""              # Tag for CondDB. Default as set in DDDBConf for DataType
        # Input
        , "Input"              : []              # Input data. Can also be passed as a second option file.
        # Output
        , "HistogramFile"      : ""              # Name of output Histogram file (set to "" to get no output) 
        , "TupleFile"          : ""              # Name of output Tuple file
        , "ETCFile"            : ""              # Name of output ETC file
        , "WriteFSR"           : True            # Flags whether to write out an FSR
        , "MergeFSR"           : False           # Flags whether to merge the FSRs into one container
        # Monitoring
        , "MoniSequence"       : []              # Add your monitors here
        # DaVinci Options
        , "MainOptions"        : ""              # Main option file to execute
        , "UserAlgorithms"     : []              # User algorithms to run.
        , "RedoMCLinks"        : False           # On some stripped DST one needs to redo the Track<->MC link table. Set to true if problems with association.
        , "InputType"          : "DST"           # or "DIGI" or "ETC" or "RDST" or "DST or "MDST" of "SDST". Nothing means the input type is compatible with being a DST. 
        , 'EnableUnpack' : None                  # Explicitly enable/disable unpacking for input data (if specified) 
        , "Lumi"               : False           # Run Lumi accounting (should normally be True for user jobs)
        # Trigger running
        , "L0"                 : False           # Run L0.
        , "Hlt"                : False           # Run Hlt
        , "ReplaceL0BanksWithEmulated" : False   # Re-run L0 
        , "HltUserAlgorithms"  : [ ]             # put here user algorithms to add
        , "Hlt2Requires"       : 'L0+Hlt1'       # Say what Hlt2 requires
        , "HltThresholdSettings" : ''            # Use some special threshold settings, eg. 'Miriam_20090430' or 'FEST'
        , "OverruleL0TCK"        : ''              # Overrule the TCK defined in the threshold setting to allow the use of the one in the data
        , "EventPreFilters"       : []
       }

    _propertyDocDct = {  
        "EvtMax"             : """ Number of events to analyse """
        , "SkipEvents"         : """ Number of events to skip at beginning for file """
        , "PrintFreq"          : """ The frequency at which to print event numbers """
        , "DataType"           : """ Data type, can be ['DC06','2008', '2009', 'MC09', '2010'] Forwarded to PhysConf, AnalysisConf and LHCbApp """
        , "Simulation"         : """ set to True to use SimCond. Forwarded to PhysConf """
        , "DDDBtag"            : """ Tag for DDDB. Default as set in DDDBConf for DataType """
        , "CondDBtag"          : """ Tag for CondDB. Default as set in DDDBConf for DataType """
        , "Input"              : """ Input data. Can also be passed as a second option file. """
        , "HistogramFile"      : """ Write name of output Histogram file """
        , "TupleFile"          : """ Write name of output Tuple file """
        , "ETCFile"            : """ Write name of output ETC file."""
        , 'WriteFSR'           : """ Flags whether to write out an FSR """
        , "MergeFSR"           : """ Flags whether to merge the FSRs into one container """
        , "MainOptions"        : """ Main option file to execute """
        , "UserAlgorithms"     : """ User algorithms to run. """
        , "RedoMCLinks"        : """ On some stripped DST one needs to redo the Track<->MC link table. Set to true if problems with association. """
        , "InputType"          : """ 'DST' or 'DIGI' or 'ETC' or 'RDST' or 'DST' or 'MDST' or 'SDST'. Nothing means the input type is compatible with being a DST.  """
        , 'EnableUnpack' : """Explicitly enable/disable unpacking for input data (if specified) """
        , "Lumi"               : """ Run event count and Lumi accounting (should normally be True) """
        , "L0"                 : """ Re-Run L0 """
        , "ReplaceL0BanksWithEmulated" : """ Re-run L0 and replace all data with emulation  """
        , "HltUserAlgorithms"  : """ Put here user algorithms to add to Hlt """
        , "Hlt2Requires"       : """ Definition of what Hlt2 requires to run. Default is 'L0+Hlt1' (equivalent to 'Hlt1').
        'L0' will run Hlt1 in pass-all mode.
        """
        , "HltThresholdSettings" : """ Use some special threshold settings, for instance 'Miriam_20090430' or 'FEST' """
        , "OverruleL0TCK"      : """Overrule the TCK defined in the threshold setting to allow the use of the one in the data"""
        , "EventPreFilters"    : """Set of event filtering algorithms to be run before DaVinci initializaton sequence. Only events passing these filters will be processed."""
        }

    __used_configurables__ = [
        PhysConf          ,
        AnalysisConf      ,
        HltConf           ,
        DstConf           ,
        L0Conf            ,
        LumiAlgsConf      ,
        LumiIntegratorConf,
        LHCbApp           ]

    __known_datatypes__ = [ "DC06", "MC09", "2008", "2009", "2010" ]

    ## Known monitoring sequences run by default
    KnownMonitors        = []

    mainSeq = GaudiSequencer("DaVinciUserSequence")
    moniSeq = GaudiSequencer("MonitoringSequence")

################################################################################
# Check Options are OK
#
    def _checkOptions(self):
        """
        Checks options. Changes a few if needed.
        """
        dataType = self.getProp("DataType")
        if (not dataType):
            raise TypeError( "You must set DataType" )
        if dataType not in self.__known_datatypes__ :
            raise TypeError( "Invalid DataType '%s'" %dataType )
        inputType = self.getProp( "InputType" ).upper()
        if inputType not in [ "MDF", "DST", "DIGI", "ETC", "RDST", "MDST", "SDST" ]:
            raise TypeError( "Invalid inputType '%s'"%inputType )
        if ( dataType in [ "DC06", "MC09" ] ):
            if not self.getProp("Simulation"):
                log.warning("Setting Simulation = True for "+dataType)
                self.setProp("Simulation",True)
#        if inputType == 'MDST' and self.getProp('Lumi') :
#            log.warning('Lumi = True not valid for InputType MDST. Setting Lumi = False')
#            self.setProp('Lumi', False )
        if ( self.getProp("Simulation") & ( inputType != "MDF" ) & (inputType != "DIGI") & (inputType != "MDST") ):
            redo = self.getProp("RedoMCLinks")
            if ( self.getProp("DataType")=="DC06" ) and ( not redo ):
                log.warning("Redoing MC links enforced with DC06")
                redo = True
                self.setProp("RedoMCLinks",True) 
            if (inputType == "RDST")  and (redo) :
                log.warning("Re-doing MC links not possible for RDST")
                self.setProp("RedoMCLinks", False )

        if self.getProp("Simulation") and self.getProp('Lumi') :
            log.warning('Lumi not valid for Simulation. Setting Lumi = False')
            self.setProp('Lumi', False )

        if self.getProp("MergeFSR"):
            if self.getProp('TupleFile'):
                self.setProp('TupleFile', False)
                log.warning('MergeFSR option selected. No Lumi ntuple will be produced.')

################################################################################
# Configure slaves
#
    def _configureSubPackages(self):
        """
        Define DB and so on
        """
        # Delegate handling to LHCbApp configurable
        self.setOtherProps(LHCbApp(),["DataType","CondDBtag","DDDBtag","Simulation"])
        
        type = self.getProp("DataType")
        cb = self.getProp("CondDBtag")
        db = self.getProp("DDDBtag")
        self.setOtherProps(PhysConf(),["DataType","Simulation","InputType"])
        self.setOtherProps(AnalysisConf(),["DataType","Simulation"])

    def _analysisSeq(self) :
        return GaudiSequencer('DaVinciAnalysisSeq', IgnoreFilterPassed = True)

    def _filteredEventSeq(self) :
        prefilters = self.getProp('EventPreFilters')
        return GaudiSequencer('FilteredEventSeq',
                              Members = prefilters +  [self._init(), self._analysisSeq()])

################################################################################
# Event Initialisation sequence
#
    def _dvInit(self):
        """
        Initialisation sequence
        """
        from Configurables import (DaVinciInit,
                                   MemoryTool)
        
        di = DaVinciInit('DaVinciInitAlg')
        di.addTool(MemoryTool)
        di.MemoryTool.HistoSize = 5000
        return di

    def _init(self):
        from Configurables import (PhysConf,
                                   AnalysisConf)
        
        # Phys
        inputType = self.getProp( "InputType" ).upper()

        initSeqs = []

        if inputType != 'MDST' :
            if (( inputType != "MDF" ) & (inputType != "DIGI")) :
                physinit = PhysConf().initSequence()         # PhysConf initSequence
                # Analysis
                AnalysisConf().RedoMCLinks = self.getProp("RedoMCLinks") 
                analysisinit = AnalysisConf().initSequence()
                initSeqs = [physinit,analysisinit]
        if inputType == 'RDST' :
            log.info('Setting HltDecReportsDecoder().InputRawEventLocation to "pRec/RawEvent"')
            from Configurables import HltDecReportsDecoder, ANNDispatchSvc
            HltDecReportsDecoder().InputRawEventLocation = "pRec/RawEvent"
            ANNDispatchSvc().RawEventLocation = "pRec/RawEvent"

        return GaudiSequencer('DaVinciEventInitSeq',
                              Members = initSeqs,
                              IgnoreFilterPassed = True)

################################################################################
# Lumi setup
#
    def _lumi(self):
        """
        read FSR and accumulate event and luminosity data
        calculate normalization - toolname:
        """
        log.info("Creating Lumi Algorithms")
        seq = []
        self.setOtherProps(LumiAlgsConf(),["DataType","InputType","MergeFSR"])

        # add merge, touch-and-count sequence
        lumiSeq = GaudiSequencer("LumiSeq")
        LumiAlgsConf().LumiSequencer = lumiSeq
        seq += [ lumiSeq ]

        if self.getProp( "Lumi" ):            
            tupleFile = self.getProp('TupleFile')
            if self.getProp("MergeFSR"):
                log.warning('MergeFSR option selected, no Lumi integral will be produced.')
            else:
                # no integration during merge
                if tupleFile == '' :
                    log.warning('TupleFile has not been set. No Lumi ntuple will be produced.')
                # add integrator for normalization
                self.setOtherProps(LumiIntegratorConf(),["InputType","TupleFile"])
                lumiInt = GaudiSequencer("IntegratorSeq")
                LumiIntegratorConf().LumiSequencer = lumiInt
                seq += [ lumiInt ]
        return seq
        
################################################################################
# Decode DecReports
#
    def _decReports(self):
        from Configurables import L0SelReportsMaker, L0DecReportsMaker
        DataOnDemandSvc().AlgMap["HltLikeL0/DecReports"] = L0DecReportsMaker( OutputLevel = 4 )
        DataOnDemandSvc().AlgMap["HltLikeL0/SelReports"] = L0SelReportsMaker( OutputLevel = 4 )        
        """
        Decode DecReports _if_ Hlt is not run
        """
        if (not self.getProp("Hlt")):
            from Configurables import HltDecReportsDecoder,HltSelReportsDecoder,HltVertexReportsDecoder
            DataOnDemandSvc().AlgMap["Hlt/DecReports"] = HltDecReportsDecoder( OutputLevel = 4 )
            DataOnDemandSvc().AlgMap["Hlt/SelReports"] = HltSelReportsDecoder( OutputLevel = 4 )
            DataOnDemandSvc().AlgMap["Hlt/VertexReports"] = HltVertexReportsDecoder( OutputLevel = 4 )
       
        
################################################################################
# HLT setup
#
    def _hlt(self):
        """
        Define HLT. Make sure it runs first.
        """
        if (self.getProp("Hlt")):
            HltConf().DataType = self.getProp("DataType")
            HltConf().WithMC =  self.getProp("Simulation")                                       
            HltConf().L0TCK =  self.getProp("OverruleL0TCK")                                       
            if ( self.getProp("HltThresholdSettings") != '' ):
                HltConf().ThresholdSettings = self.getProp("HltThresholdSettings")
            if ( self.getProp("Hlt2Requires") == 'L0+Hlt1' or self.getProp("Hlt2Requires") == 'Hlt1' ):
                log.info('Hlt2 requires L0 and Hlt1')
            elif ( self.getProp("Hlt2Requires") == 'L0' ):
                log.info('Hlt2 requires L0 only. Will set Hlt1 passthrough prescale to 1. Note that Hlt1Global has no useful meaning.')
                from Hlt1Lines.Hlt1L0Lines import Hlt1L0LinesConf
                Hlt1L0LinesConf().Prescale.update( { 'Hlt1L0Any'  : 1. } )

            else : raise AttributeError, 'Hlt2 can require L0 or Hlt1'

            from Configurables import GaudiSequencer, HltRoutingBitsFilter
            physFilter = HltRoutingBitsFilter( "PhysFilter", RequireMask = [ 0x0, 0x4, 0x0 ] )  # make sure lumi events are ignored
            hltDVSeq = GaudiSequencer("RunHltInDaVinci")  # 
            hltSeq = GaudiSequencer("Hlt")         # catch the Hlt sequence to make sur it's run first
            from Configurables import bankKiller
            log.warning("Running Hlt. If there are already banks written by Hlt in the data, they will be removed.") 
            bk = bankKiller('KillHltBanks', BankTypes = [ "HltRoutingBits", "HltSelReports", "HltVertexReports", "HltDecReports", "HltLumiSummary" ])
            hltDVSeq.Members = [ physFilter, bk, hltSeq ]
            self._analysisSeq().Members += [ hltDVSeq ]
            log.info("Will run Hlt")
            log.info( HltConf() )
            
        
################################################################################
# L0 setup
#
    def _l0(self):
        """
        Define L0. Make sure it runs before HLT.
        """
        if ( (self.getProp("Hlt")) and (self.getProp("DataType") == 'DC06') and (not self.getProp("L0")) ):
            log.warning("It is mandatory to run the L0 emulation on DC06 data to get the HLT to work correctly")
            log.warning("Will set ReplaceL0BanksWithEmulated = True")
            self.setProp("ReplaceL0BanksWithEmulated",True) 
        if ( self.getProp("ReplaceL0BanksWithEmulated") and (not self.getProp("L0")) ):
            log.warning("You asked to replace L0 banks with emulation. Will set L0 = True")
            self.setProp("L0",True)
        if ( self.getProp("OverruleL0TCK")):
            if (not self.getProp("L0")):
                log.warning("You asked to Overrule the L0TCK defined in Hlt settings. Will set L0 = True")
                self.setProp("L0",True)
            if (not self.getProp("ReplaceL0BanksWithEmulated")):
                log.warning("""
                You asked to Overrule the L0TCK defined in Hlt settings but not to emulate L0.
                This might be possible or not depending on the TCKs. I hope you know what you are doing
                """)
        # done with all warnings. Now do the logic.
        if ( self.getProp("L0") ):
            l0seq = GaudiSequencer("seqL0")
            self._analysisSeq().Members +=  [ l0seq ]
            L0Conf().setProp( "L0Sequencer", l0seq )
            L0Conf().setProp( "ReplaceL0BanksWithEmulated", self.getProp("ReplaceL0BanksWithEmulated") )
            L0Conf().setProp( "DataType", self.getProp("DataType"))

            if ( self.getProp('HltThresholdSettings') ):
                from HltConf.ThresholdUtils import Name2Threshold
                if ( self.getProp("OverruleL0TCK") ):
                    L0TCK = self.getProp("OverruleL0TCK")
                else:
                    L0TCK = Name2Threshold(self.getProp('HltThresholdSettings')).L0TCK()
                L0Conf().setProp( "TCK", L0TCK )
                log.info("Will run L0 with TCK "+L0TCK)
            else :
                if ( self.getProp("OverruleL0TCK") ):
                    L0TCK = self.getProp("OverruleL0TCK")
                    L0Conf().setProp( "TCK", L0TCK )
                    log.info("Will run L0 with TCK "+L0TCK)
                else :
                    log.info("L0 TCK has not been set by DaVinci. Assume it's set otherwise.")
        
################################################################################
# @todo Stolen from Brunel. Could be common to all apps?
#
    def _defineMonitors(self):
        """
        Define monitors
        """
        from Configurables import (ApplicationMgr,
                                   AuditorSvc,
                                   SequencerTimerTool)
        
        ApplicationMgr().ExtSvc += [ 'ToolSvc', 'AuditorSvc' ]
        ApplicationMgr().AuditAlgorithms = True
        AuditorSvc().Auditors += [ 'TimingAuditor' ]
        SequencerTimerTool().OutputLevel = 4
        # Do not print event number at every event
        printfreq = self.getProp("PrintFreq")
        if ( printfreq == 0 ):
            log.warning("Print frequence cannot be 0")
            printfreq = 100
        EventSelector().PrintFreq = printfreq

################################################################################
# set EvtMax
#
    def _defineEvents(self):
        """
        Define number of events
        """
        # Delegate handling to LHCbApp configurable
        self.setOtherProps(LHCbApp(),["EvtMax","SkipEvents"])

################################################################################
# returns evtMax
#
    def evtMax(self):
        """
        Get evtMax
        """
        return LHCbApp().evtMax()
    
################################################################################
# Defines input
#
    def _defineInput(self):
        """
        Define Input
        """
        input = self.getProp("Input")

        if ( len(input) > 0) :
            EventSelector().Input = input
        inputType = self.getProp( "InputType" ).upper()
        if inputType == "MDF" :
            log.info('Adding LHCb::RawDataCnvSvc to EventPersistencySvc().CnvServices.')
            EventPersistencySvc().CnvServices.append( 'LHCb::RawDataCnvSvc' )
            importOptions("$STDOPTS/DecodeRawEvent.py")
        if inputType == 'SDST' or inputType == 'ETC' :
            log.info('Adding LHCb::RawDataCnvSvc to EventPersistencySvc().CnvServices.')
            EventPersistencySvc().CnvServices.append('LHCb::RawDataCnvSvc')

################################################################################
# Tune initialisation
#
    def _configureInput(self):
        """
        Tune initialisation 
        """
        # POOL Persistency
        importOptions("$GAUDIPOOLDBROOT/options/GaudiPoolDbRoot.opts")
        # Get the event time (for CondDb) from ODIN
        from Configurables import EventClockSvc
        EventClockSvc().EventTimeDecoder = "OdinTimeDecoder";
        
        inputType = self.getProp( "InputType" ).upper()
        
        # if property set explcicitly - use it! 
        if self.isPropertySet('EnableUnpack') :
            unPack = self.getProp('EnableUnpack')
            DstConf           ( EnableUnpack = unPack )
            PhysConf ( EnableUnpack = unPack)
        elif inputType!="MDST" and ( self.getProp("DataType") != "DC06"
                                     and inputType != "MDF" ):
            # DST unpacking, not for DC06 unless MDF. Not for MDST, ever.
            DstConf           ( EnableUnpack = True )
            PhysConf ( EnableUnpack = True)
            if self.getProp("Simulation") :
                DstConf().setProp("SimType","Full")
        return inputType

    def _hltCondDBHack(self) :
        cdb = self.getProp("CondDBtag")
        if not isNewCondDBTag(cdb) :
            if cdb == '' : cdb = 'default'
            log.info('CondDB '+ cdb + ' tag considered old. Setting HltReferenceRateSvc().UseCondDB = False')
            from Configurables import HltReferenceRateSvc
            HltReferenceRateSvc().UseCondDB = False
        else :
            log.info('CondDB ' + cdb + ' tag considered NEW.')
################################################################################
# Ntuple files
#
    def _rootFiles(self):
        """
        output files
        """
        ApplicationMgr().HistogramPersistency = "ROOT"
        if ( self.isPropertySet('HistogramFile') and self.getProp("HistogramFile") != "" ):
            HistogramPersistencySvc().OutputFile = self.getProp("HistogramFile")
        if ( self.isPropertySet('TupleFile') and self.getProp("TupleFile") != "" ):
            tupleFile = self.getProp("TupleFile")

            ntSvc = NTupleSvc()
            ApplicationMgr().ExtSvc +=  [ ntSvc ]
            for _line in ntSvc.Output :
                if 0 <= _line.find ('FILE1') :
                    log.warning ('Replace NTuple-LUN FILE1: ' + _line ) 
                    ntSvc.Output.remove ( i )
                    
            tupleStr = "FILE1 DATAFILE='%s' TYP='ROOT' OPT='NEW'" % tupleFile
            NTupleSvc().Output      += [ tupleStr ]
            # NTupleSvc().OutputLevel  = 1
            
        if ( self.isPropertySet('ETCFile') and self.getProp("ETCFile") != "" ):
            if ( self.getProp("WriteFSR") ):
                self._etcfsr(self.getProp("ETCFile"))
            self._etc(self.getProp("ETCFile"))

################################################################################
# ETC
#
    def _etc(self,etcFile):
        """
        write out an ETC
        """
        from Configurables import TagCollectionSvc
        tcname = "EvtTupleSvc"
        ets = TagCollectionSvc(tcname)
        ets.Output += [ "EVTCOL DATAFILE='"+etcFile+"' TYP='POOL_ROOTTREE' OPT='RECREATE' " ]
        ets.OutputLevel = 1 
        ApplicationMgr().ExtSvc  += [ ets ]
        #
        from Configurables import EvtCollectionStream
        tagW = EvtCollectionStream("TagWriter")
        # this somehow matches CollectionName
        log.info("Did not defined itemlist for ETC.")
#        tagW.ItemList = iList
        tagW.EvtDataSvc = tcname
        
        from Configurables import Sequencer
        seq = Sequencer("SeqWriteTag")
        ApplicationMgr().OutStream += [ tagW ]

################################################################################
# ETC + FSR - write fsr to etc file
#
    def _etcfsr(self, fsrFile):
        """
        write out the FSR
        it is IMPERATIVE to define the FSR outputstream before the etc
        """
        # Output stream
        FSRWriter = RecordStream( "FileRecords",
                                  ItemList         = [ "/FileRecords#999" ],
                                  EvtDataSvc       = "FileRecordDataSvc",
                                  Output           = "DATAFILE='"+fsrFile+"' TYP='POOL_ROOTTREE'",
                                  )

        # Write the FSRs to the same file as the events
        ApplicationMgr().OutStream += [ FSRWriter ]
        
################################################################################
# Main sequence
#
    def _mainSequence(self):
        """
        Main Sequence
        """
        self.mainSeq.IgnoreFilterPassed = True
        self._analysisSeq().Members += [ self.mainSeq ]
        opts = self.getProp( "MainOptions" )
        if not (opts == '') :
            importOptions( self.getProp( "MainOptions" ) )
        else :
            log.info("No MainOptions specified. DaVinci() will import no options file!")
        log.info("Creating User Algorithms")
        self.appendToMainSequence( self.getProp("UserAlgorithms")  )        

################################################################################
# Append to Main sequence
#
    def appendToMainSequence(self, algs):
        """
        Append to main sequence. Can be called from other configurables
        """
        log.info("Append to Main Sequence has been called")
        for alg in algs:
            configurable = self._configurable(alg)
            self.mainSeq.Members += [ configurable ]

    def _configurable(self, obj) :
        _obj2ConfMap = { 'SelectionSequence'         : lambda x : x.sequence(),
                         'MultiSelectionSequence'    : lambda x : x.sequence() }
        return _obj2ConfMap.get(type(obj).__name__, lambda x : x)(obj)

################################################################################
# Monitoring sequence
#
    def _moniSequence(self):
        """
        Monitoring sequence
        """
        self.moniSeq.IgnoreFilterPassed = True 
        self.moniSeq.Members = self.KnownMonitors
        self._analysisSeq().Members += [ self.moniSeq ]
        log.info("Creating Moni Algorithms")
        self.appendToMoniSequence( self.getProp("MoniSequence") )

################################################################################
# Append to Moni sequence
#
    def appendToMoniSequence(self, algs):
        """
        Append to moni sequence. Can be called from other configurables
        """
        log.info("Append to Moni Sequence has been called")
        for alg in algs:
            self.moniSeq.Members += [ alg ]

    def sequence(self) :
        return GaudiSequencer('DaVinciEventSeq',
                              IgnoreFilterPassed = True)


    def _setAncestorDepth(self) :
        """
        Calculate depth of ancestry for input files and set IODataManager().AgeLimit accordingly.
        Set to 1 for InputType = 'ETC', 'RDST' or 'SDST' and 0 for others.
        """
        if not IODataManager().isPropertySet('AgeLimit') :
            depthMap = { 'ETC'  : 1,
                         'RDST' : 1,
                         'SDST' : 1  }
            inputType = self.getProp('InputType').upper()
            IODataManager().setProp('AgeLimit', depthMap.get(inputType, 0))

################################################################################
# Apply configuration
#
    def __apply_configuration__(self):
        """
        DaVinci configuration
        """
        log.info("Applying DaVinci configuration")
        log.info( self )

        self._checkOptions()

        self._setAncestorDepth()

        ApplicationMgr().TopAlg = [self.sequence()]
        self._configureSubPackages()
        importOptions("$STDOPTS/PreloadUnits.opts") # to get units in .opts files
        inputType = self._configureInput()
        self.sequence().Members = [
            self._dvInit()
            ]
#        self.sequence().Members += self._init()
        self.sequence().Members += [self._filteredEventSeq()]
        
        # _lumi handles event count, lumi integration and merging independently
        self.sequence().Members += self._lumi()
            
        if inputType != 'MDST' :
            self._l0()
            self._hlt()
            self._decReports()
            self._hltCondDBHack()
        self._defineMonitors()
        self._defineEvents()
        self._defineInput()
        self._rootFiles()
        
        # Add main sequence to TopAlg
        self._mainSequence()
        # monitoring
        self._moniSequence()
        
