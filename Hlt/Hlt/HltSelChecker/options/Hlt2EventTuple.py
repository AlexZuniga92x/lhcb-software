### @file Hlt2EventTuple.opts
 #
 #  Generic Event Tuple configuration
 #
 #  See DecayTreeTuple documentation
 #
 #  @author P. Koppenburg
 #  @date 2008-03-07
#/
from Gaudi.Configuration import *
from Configurables import EventTuple, TupleToolGeneration, TupleToolTrigger, L0Filter
#

# add event-info tuple
tuple = EventTuple("TriggerTuple")
tuple.ToolList = [ "TupleToolEventInfo",   #  "TupleToolGeneration",
                   "TupleToolTrigger", "MCTupleToolEventType",
                   "TupleToolTriggerRecoStats",
                   "TupleToolPrimaries", "MCTupleToolPrimaries"]
tuple.addTool( TupleToolTrigger )
tuple.TupleToolTrigger.VerboseHlt1 = True
tuple.TupleToolTrigger.VerboseHlt2 = True

moni = GaudiSequencer("Hlt2MonitorSeq")
moni.Members += [ tuple ] 
