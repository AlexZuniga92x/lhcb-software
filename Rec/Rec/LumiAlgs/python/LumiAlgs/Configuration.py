'''
High level Configuration tools for Lumi Accounting Algorithms

Will configure a sequencer to run the Lumi Accounting algs.
For MDF files FSRs are created.
For DST files they are only created if ForceFullSequence is set to true.

The FSR tree should always be navigated, and the EventAccounting alg always called regardless.

'''
## @package LumiAlgs
#  High level Configuration tools for Lumi Accounting Algorithms
#  @author Jaap Panman
#  @date   13/02/2009

__version__ = ""
__author__  = "Jaap Panman"

from Gaudi.Configuration  import *
import GaudiKernel.ProcessJobOptions
from Configurables import LHCbConfigurableUser 
    
# -------------------------------------------------------------------------------------------

## @class LumiAlgsConf
#  High level Configuration tools for Lumi Accounting Algorithms
#  @author Jaap Panman
#  @date   13/02/2009

class LumiAlgsConf(LHCbConfigurableUser):

    ## Steering options
    __slots__ = {
        "Context"       : "Offline"  # The context within which to run
       ,"DataType"      : "2009"     # Data type, can be ['DC06','2008','MC09','2009']
       ,"InputType"     : "MDF"      # Data type, can be ['MDF','DST','RDST', 'SDST', 'MDST','ETC','DIGI']. Different sequencer made.
       ,"ForceFullSequence" : False  # re-write the FSR independent of the InputType
       ,"SetFSRStatus"  : ""         # overwrite the event FSR status to something ['UNRELIABLE', 'ERROR','VERIFIED']
       ,"LumiSequencer" : None       # The sequencer to add the Lumi Accounting to - essential input
       ,"BXTypes"       : [ 'NoBeam', 'BeamCrossing','Beam1','Beam2'] # bunch crossing types
       ,"OutputLevel"   : INFO       
        }

    _propertyDocDct = {
        "Context"       : """ The context within which to run """
       ,"DataType"      : "Data type, can be ['DC06','2008','MC09','2009']"
       ,"InputType"     : "Input Data type, can be ['MDF','DST','RDST', 'SDST', 'MDST','ETC','DIGI']"
       ,"ForceFullSequence" : "False, re-write the FSR independent of the InputType"
       ,"SetFSRStatus"  : "overwrite the event FSR status to something ['UNRELIABLE', 'ERROR','VERIFIED']"
       ,"LumiSequencer" : "The sequencer to add the Lumi Accounting to - essential input"
       ,"BXTypes"       : "bunch crossing types [ 'NoBeam', 'BeamCrossing','Beam1','Beam2'] "
       ,"OutputLevel"   : "printed output"
       }
    
    ## Helper functions
    def fillFSR(self):
        '''
        fill the original FSR, return a sequence
        requires LumiTrigger
        and requires Method counter == RandomMethod
        - WARNING -
        - the Method selection is valid for Moore as of v9 -
        - to keep code clean it is better to use for earlier versions of Moore only the other FSR -
        '''
        from Configurables import ( LumiAccounting,
                                    HltLumiSummaryDecoder, FilterOnLumiSummary, GaudiSequencer )
        from Configurables import LoKi__ODINFilter  as ODINFilter
        # Create sub-sequences according to BXTypes
        crossings = self.getProp("BXTypes")
        BXMembers = []
        for i in crossings:
            seqMembers=[]
            seqMembers.append( ODINFilter ( 'Filter'+i,
                                            Code = ' ( ODIN_TRGTYP == LHCb.ODIN.LumiTrigger ) & ( ODIN_BXTYP == LHCb.ODIN.'+i+' ) ' ))
            decoder = HltLumiSummaryDecoder('LumiDecode'+i)
            seqMembers.append( decoder )
            methodfilter = FilterOnLumiSummary('LumiFilter'+i,
                                               CounterName = "Random",
                                               ValueName = "RandomMethod",
                                               OutputLevel = self.getProp("OutputLevel") )
            seqMembers.append( methodfilter )

            accounting = LumiAccounting('LumiCount'+i,
                                        OutputDataContainer = "/FileRecords/LumiFSR"+i,
                                        OutputLevel = self.getProp("OutputLevel") )
            seqMembers.append( accounting )
            
            BXMembers.append( GaudiSequencer('Lumi'+i+'Seq', 
                                             Members = seqMembers,
                                             ModeOR = False,
                                             ShortCircuit = True,
                                             MeasureTime = True,
                                             ))
            if self.getProp('InputType') == 'DST':
                    decoder.OutputContainerName='LumiSummaries'
                    accounting.InputDataContainer='LumiSummaries'
        
        return BXMembers
    
    def fillLowLumiFSR(self):
        '''
        fill the low lumi FSR, return a sequence
        similar to the fillFSR method, but accepts more trigger types and
        applies the method filter Method:L0RateMethod
        this is valid for all versions om Moore 
        '''
        from Configurables import ( LumiAccounting,
                                    HltLumiSummaryDecoder, FilterOnLumiSummary, GaudiSequencer )
        from Configurables import LoKi__ODINFilter  as ODINFilter
        # Create sub-sequences according to BXTypes
        crossings = self.getProp("BXTypes")
        BXMembers = []
        for i in crossings:
            seqMembers=[]
            seqMembers.append( ODINFilter ( 'FilterLow'+i,
                                            Code = ' ( ODIN_TRGTYP <= LHCb.ODIN.LumiTrigger ) & ( ODIN_BXTYP == LHCb.ODIN.'+i+' ) ' ))
            decoder = HltLumiSummaryDecoder('LumiDecode'+i)  # keep the name without "Low" to avoid double execution
            seqMembers.append( decoder )
            methodfilter = FilterOnLumiSummary('LumiLowFilter'+i,
                                               CounterName = "Method",
                                               ValueName = "L0RateMethod",
                                               OutputLevel = self.getProp("OutputLevel") )
            seqMembers.append( methodfilter )
            
            accounting = LumiAccounting('LumiLowCount'+i,
                                        OutputDataContainer = "/FileRecords/LumiLowFSR"+i,
                                        OutputLevel = self.getProp("OutputLevel") )
            seqMembers.append( accounting )
            
            BXMembers.append( GaudiSequencer('LumiLow'+i+'Seq', 
                                             Members = seqMembers,
                                             ModeOR = False,
                                             ShortCircuit = True,
                                             MeasureTime = True,
                                             ))
            if self.getProp('InputType') == 'DST':
                    decoder.OutputContainerName='LumiSummaries'
                    accounting.InputDataContainer='LumiSummaries'
                    methodfilter.InputDataContainer='LumiSummaries'
        
        return BXMembers
    
    def fillTimeSpanFSR(self):
        '''
        fill the time span FSR, return a sequence
        it looks at the time stamp of events with the LumiTrigger
        '''
        from Configurables import ( TimeAccounting,
                                    GaudiSequencer )
        from Configurables import LoKi__ODINFilter  as ODINFilter
        # Create sub-sequence
        seqMembers=[]
        seqMembers.append( ODINFilter ( 'TimeSpanFilter',
                                        Code = ' ( ODIN_TRGTYP == LHCb.ODIN.LumiTrigger ) ' ))
        accounting = TimeAccounting('TimeSpanAccounting', OutputLevel = self.getProp("OutputLevel") )
        seqMembers.append( accounting )
        seq =  GaudiSequencer('TimeSpanSeq', 
                              Members = seqMembers,
                              ModeOR = False,
                              ShortCircuit = True,
                              MeasureTime = True,
                              )
        
        return [seq]
    
    def fillEventFSR(self,status=""):
        '''fill the EventAccounting'''
        from Configurables import (EventAccounting, GaudiSequencer)
        accounting = EventAccounting('EventAccount', OutputLevel = self.getProp("OutputLevel") )
        if status is not None and status != "":
            accounting.DefaultStatus=status
            accounting.OverrideStatus=True
        return [accounting]
    
    def touchFSR(self):
        '''make sure the FSR is touched and readable'''
        from Configurables import createODIN
        odin=createODIN()
        from Configurables import (LumiIntegrateFSR)
        readingFSR=LumiIntegrateFSR("TouchLumiFSR", OutputLevel = self.getProp("OutputLevel") )
        # this also touches the LumiLow FSRs
        readingLumiLowFSR=LumiIntegrateFSR("TouchLumiLowFSR",
                                           FSRName = "LumiLowFSR",
                                           IntegratorToolName = "LumiLowIntegrator", 
                                           OutputLevel = self.getProp("OutputLevel") )
        return [ odin, readingFSR]
    
    ## Apply the configuration to the given sequence
    def __apply_configuration__(self):
        ''' Apply the configuration to the given sequence '''
        
        sequence = self.getProp("LumiSequencer")
        if sequence == None : raise RuntimeError("ERROR : Lumi Sequencer not set")
        
        # Set context
        context = self.getProp("Context")
        
        # Check input data type
        inputType = self.getProp("InputType")
        if inputType not in [ "MDF", "DST", "DIGI", "ETC", "RDST", "MDST", "SDST" ]:
            raise RuntimeError("ERROR : Unknown input type " + str(inputType))
        
        forced=self.getProp("ForceFullSequence")
        status=self.getProp("SetFSRStatus")
        
        if inputType == 'MDF' or forced:
            # create the FSRs
            
            # Create sub-sequences according to BXTypes
            sequence.Members += self.fillFSR()

            # add other method LumiSummaries
            sequence.Members += self.fillLowLumiFSR()
            
            if status is None or status=='':
                #by definition, all raw files are verified
                status='VERIFIED'
        
            # create the TimeSpan FSR
            sequence.Members += self.fillTimeSpanFSR()
        
        # create the Event FSR
        EvMembers=self.fillEventFSR(status)

        #finally configure the sequence
        sequence.Members += EvMembers
        
        #Touch all FSRs so they are always copied
        if inputType != 'MDF':
            TouchMembers=self.touchFSR()
            sequence.Members += TouchMembers
        
        sequence.MeasureTime = True
        sequence.ModeOR = True
        sequence.ShortCircuit = False
        sequence.IgnoreFilterPassed = True
        
