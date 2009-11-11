# options to run mdf reader for lumi reader
__version__ = "$Id: LumiReaderConf.py,v 1.2 2009-11-11 16:52:57 panmanj Exp $"
__author__  = "Jaap Panman"

from os import environ, path

from Gaudi.Configuration import *
from GaudiConf.Configuration import *
from LHCbKernel.Configuration import *

from Configurables import GaudiSequencer as Sequence
from Configurables import ( LHCbConfigurableUser, LHCbApp )

from Configurables import createODIN
from Configurables import LoKi__ODINFilter  as ODINFilter
from Configurables import RawEventDump
from Configurables import HltLumiSummaryDecoder
from Configurables import LumiFileReader

import GaudiKernel.ProcessJobOptions

def _ext(name) : return path.splitext(name)[-1].lstrip('.')

def _sequenceAppender( seq ) :
    return lambda x : seq.Members.append( x )

class LumiReaderConf(LHCbConfigurableUser):
  ## Possible used Configurables
  __used_configurables__ = [ LHCbApp,
                               ]

  __slots__ = {
    "EvtMax":             -1      # Maximum number of events to process
    , "SkipEvents":        0
    , "DataType":          '2009' # Data type
    , "outputFile" :       ''     # output filename
    , "inputFiles" :       [ ]    # input
    , "userAlgorithms":    [ ]    # put here user algorithms to add
    , "OutputLevel" :      ERROR  #
    , "Debug" :            False  #
    }   


  def _createReader(self, sequence):
    '''
    create reader sequence
    '''
    debugOPL = self.getProp("OutputLevel")
    debugging = self.getProp("Debug")
    readLumiSequence = _sequenceAppender( Sequence('readLumiSeq',
                                                    ModeOR = False,
                                                    ShortCircuit = True,
                                                    IgnoreFilterPassed = False,
                                                    MeasureTime = True,
                                                    OutputLevel = debugOPL  ) )
        
    # create ODIN by hand
    readLumiSequence( createODIN ('createODIN') )
    # select only the right Trigger Type
    readLumiSequence( ODINFilter ( 'OdinTriggerTypes',
                                   Code = ' ( ODIN_TRGTYP == LHCb.ODIN.LumiTrigger ) ' ))
    # decode lumi
    readLumiSequence( HltLumiSummaryDecoder( 'LumiDecoder' ) ) 
    # read and decode input file ---
    readLumiSequence( LumiFileReader( OutputLevel = debugOPL ) ) 
    # verbose output
    if debugging:
        readLumiSequence( RawEventDump( 'InputDump', DumpData = True, OutputLevel = debugOPL ) )

    # and activate it
    sequence.Members+=[ Sequence('readLumiSeq') ]
 

  def _configureInput(self):
    files = self.getProp('inputFiles')
    extensions = { 'RAW' : "' SVC='LHCb::MDFSelector'",
                   'MDF' : "' SVC='LHCb::MDFSelector'",
                   'DST' : "' TYP='POOL_ROOTTREE' OPT='READ'" }
    EventSelector().Input = [ "DATAFILE='PFN:"+ f + extensions[ _ext(f).upper() ] for f in files ]

  def _configureOutput(self):
    # first empty the outstream, because it would write all the time
    ApplicationMgr().OutStream = [ ]
    # only configure the writer - use it where applicable
    fname = self.getProp('outputFile')
    if not fname : return

  def __apply_configuration__(self):

    GaudiKernel.ProcessJobOptions.PrintOff()

    EventPersistencySvc().CnvServices.append( 'LHCb::RawDataCnvSvc' )
    importOptions('$STDOPTS/DecodeRawEvent.py')

    # forward some settings...
    self.setOtherProps( LHCbApp(), ['EvtMax','SkipEvents','DataType'] )

    # instantiate the sequencer
    mainSeq = GaudiSequencer("LumiSeq")

    # activate the sequence
    appMgr=ApplicationMgr(TopAlg=[],
                          HistogramPersistency='NONE',
                          )
    ApplicationMgr().TopAlg += [ mainSeq ]
    
    # add user algorithms at the end
    if self.getProp("userAlgorithms"):
      for userAlg in self.getProp("userAlgorithms"):
        ApplicationMgr().TopAlg += [ userAlg ]

    # input
    self._configureInput()

    # configure the main sequence
    self._createReader(mainSeq)

    # output
    self._configureOutput()
