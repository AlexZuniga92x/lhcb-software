# options to run mdf writer for lumi writer
__version__ = "$Id: LumiWriterConf.py,v 1.1 2009-10-09 15:32:12 panmanj Exp $"
__author__  = "Jaap Panman"


from os import environ, path

from Gaudi.Configuration import *
from GaudiConf.Configuration import *
from LHCbKernel.Configuration import *

from Configurables import LHCb__MDFWriter as MDFWriter
from Configurables import GaudiSequencer as Sequence
from Configurables import ( LHCbConfigurableUser, LHCbApp )

from Configurables import createODIN
from Configurables import OdinTypesFilter 
from Configurables import RawEventDump
from Configurables import bankKiller 
from Configurables import FileIdBankWriter


import GaudiKernel.ProcessJobOptions

def _ext(name) : return path.splitext(name)[-1].lstrip('.')

def _sequenceAppender( seq ) :
    return lambda x : seq.Members.append( x )

class LumiWriterConf(LHCbConfigurableUser):
  ## Possible used Configurables
  __used_configurables__ = [ LHCbApp,
                               ]

  __slots__ = {
    "EvtMax":             -1      # Maximum number of events to process
    , "SkipEvents":        0
    , "DataType":          '2009' # Data type
    , "outputFile" :       ''     # output filename
    , "inputFiles" :       [ ]    # input
    , "KillBanks" :        True   # whether to kill non-lumi banks
    , "userAlgorithms":    [ ]    # put here user algorithms to add
    , "OutputLevel" :      ERROR  #
    , "Debug" :            False  #
    }   


  def _createWriter(self, sequence):
    '''
    create writer sequence
    '''
    debugOPL = self.getProp("OutputLevel")
    debugging = self.getProp("Debug")
    writeLumiSequence = _sequenceAppender( Sequence('writeLumiSeq',
                                                    ModeOR = False,
                                                    ShortCircuit = True,
                                                    IgnoreFilterPassed = False,
                                                    MeasureTime = True,
                                                    OutputLevel = debugOPL  ) )
        
    # create ODIN by hand
    writeLumiSequence( createODIN ('createODIN') )
    # verbose output
    if debugging:
      writeLumiSequence( RawEventDump( 'InputDump', DumpData = False, OutputLevel = debugOPL ) )
      pass
      
    # select only the right Trigger Type
    writeLumiSequence( OdinTypesFilter( 'OdinTriggerTypes', TriggerTypes=['RandomTrigger'] ) )
    # kill non-lumi banks
    if self.getProp('KillBanks') :
      writeLumiSequence(
        bankKiller( 'KillAll', BankTypes=[ 'ODIN','HltLumiSummary','DAQ' ],  DefaultIsKill=True )
        )
    # tag input file ---
    writeLumiSequence( FileIdBankWriter( OutputLevel = debugOPL ) ) 
    # verbose output
    if debugging:
        writeLumiSequence( RawEventDump( 'OutputDump', DumpData = True, OutputLevel = debugOPL ) )

    writeLumiSequence( MDFWriter( "MDFWriter" ) )  # configure somewhere else

    # and activate it
    sequence.Members+=[ Sequence('writeLumiSeq') ]

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
    if _ext(fname).upper() not in [ 'MDF','RAW' ] :
      raise NameError('unsupported filetype for file "%s"'%fname)
    writer = MDFWriter( 'MDFWriter'
                        , Connection = 'file://' + fname
                        , OutputLevel = WARNING
                        )

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
    self._createWriter(mainSeq)

    # output
    self._configureOutput()
