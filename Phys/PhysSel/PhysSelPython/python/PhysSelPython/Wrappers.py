#$Id: Wrappers.py,v 1.50 2010-08-11 08:42:14 ibelyaev Exp $
"""
Wrapper classes for a DaVinci offline physics selection. The following classes
are available:

   - Selection          Wraps a selection configurable and the selections it
                        requires
   - DataOnDemand       Wraps a string TES location to make it look like a Seleciton
   - AutomaticData      Wraps a string TES location to make it look like a Seleciton
   - SelectionSequence  Creates a sequence from a selection such that all the
                        sub-selections required are executed in the right order
   - MergedSelection    Merges a set of selections in OR mode, writing all data to a single location.
   - EventSelection     Wraps an algorithm that selects an event and produces no output data.
   - VoidEventSelection Selects event based on TES container properties, applying a LoKi__VoidFilter compatible cut to the data from a required selection.
   
"""
__author__ = "Juan PALACIOS juan.palacios@nikhef.nl"

__all__ = ('DataOnDemand',
           'AutomaticData',
           'Selection',
           'MergedSelection',
           'EventSelection',
           'SelectionSequence',
           'MultiSelectionSequence',
           'SelSequence',
           'NameError',
           'NonEmptyInputLocations',
           'IncompatibleInputLocations'
           )

from copy import copy


from Configurables import GaudiSequencer

from SelPy.selection import ( flatAlgorithmList,
                              AutomaticData,
                              NameError,
                              NonEmptyInputLocations,
                              IncompatibleInputLocations,
                              update_overlap,
                              SelSequence                )

from SelPy.selection import Selection as Sel
from SelPy.selection import AutomaticData as autodata

from Configurables import LoKi__VoidFilter as VoidFilter
from Configurables import FilterDesktop
from PhysSelPython.Utils import configurableExists

def Selection(name, *args, **kwargs) :
    """
    Wrapper around SelPy.Selection. Since SelPy.Selection clones it's input
    algorithm giving it it's own name, check first whether a configurable
    with that name already exists and raise a NameError if that is the case.
    If not, construct and return a SelPy.Selection.
    """
    if configurableExists(name) :
        raise NameError('Target Configurable '+ name + ' already exists. Pick a new one')
    return Sel(name, *args, **kwargs)

class AutomaticData(autodata) :
    """
    Simple wrapper for a data location. To be used for locations
    that are guaranteed to be populated. This could be a location
    on a DST or a location registered to the DataOnDemandSvc.
    Returns output location via outputLocation() method.
    Can be used as a Selection in RequiredSelections field of other
    Selections.

    Example: wrap StdLoosePions

    >>> SelStdLoosePions = AutomaticData(Location = 'Phys/StdLoosePions')
    >>> SelStdLoosePions.outputLocation()
    'Phys/StdLoosePions'
    >>> SelStdLoosePions.name()
    'Phys_StdLoosePions'
    >>> pions = AutomaticData(Location='Phys/StdLoosePions/Particles')
    >>> pions.outputLocation()
    'Phys/StdLoosePions/Particles'
    """

    def __init__(self,
                 Location,
                 Extension='Particles') :

        _extension = ''
        if Extension != '' :
            _extension = ('/'+Extension).replace('//', '/')
        self._location = (Location + _extension).replace(_extension+_extension, _extension)
        autodata.__init__(self,
                          Location=self._location)
        
        self._algoName = 'SelFilter'+self._name

    def algorithm(self) :
        return VoidFilter('SelFilter'+self._name,
                          Code = "CONTAINS('"+self._location+"')>0")

#    def algName(self) :
#        return self.algorithm().name()

DataOnDemand = AutomaticData

class EventSelection(object) :
    """
    Selection wrapper class for configured event selection algorithm.
    Algorithm produces no output data and is assumed to be correctly
    configured. Only action expected from it is that is selects or
    rejects an event. Can be used just like a Selection object.
    Example:
    from PhysSelPython.Wrappers import EventSelection, SelectionSequence
    from Configurables import SomeEventSelectionAlg
    evtSel = EventSelection(SomeEventSelectionAlg('MyEvtSel', **params)
    help(SelectionSequence)
    selSeq = SelectionSequence('MyEvtSelSeq', TopSelection = evtSel)
    """
    def __init__(self, algorithm ,
                 RequiredSelection = None  ) :
        self._alg = algorithm
        self.requiredSelection  = RequiredSelection 
        if self.requiredSelection :
            self.requiredSelections = [ RequiredSelection ]
        else   : self.requiredSelections = []
    
    def name(self) :
        return self._alg.name()
    def algorithm(self) :
        return self._alg
    def outputLocation(self) :
        if self.requiredSelection :
            return self.requiredSelection.outputLocation()
        return ''

class VoidEventSelection(object) :
    """
    Selection class for event selection based on contents of TES location.
    Can be used just like a Selection object.
    Constructor argument Code is LoKi__VoidFilter compatible single-location
    cut, where the location is expressed as '<Location>' and determined
    from the RequiredSelection's outputLocation(). The outputLocation() of
    this algorithm is the same as that of the RequiredSelection.
    Example:
    from PhysSelPython.Wrappers import VoidEventSelection, SelectionSequence
    evtSel = VoidEventSelection( name='MyEvtSel',
                                 Code="CONTAINS('<Location>')>0",
                                 RequiredSelection = AutomaticData(....) )
    help(SelectionSequence)
    selSeq = SelectionSequence('MyEvtSelSeq', TopSelection = evtSel)
    """

    __author__ = "Juan Palacios palacios@physik.uzh.ch"

    __used_names = []
    
    def __init__(self,
                 name,
                 Code,
                 RequiredSelection ) :

        if name in VoidEventSelection.__used_names :
            raise NameError('VoidEventSelection name ' + name + ' has already been used. Pick a new one.')

        
        if configurableExists(name) :
            raise NameError('Target Configurable '+ name + ' already exists. Pick a new one.')
        
        self._name = name
        self.requiredSelection  = RequiredSelection 
        self.requiredSelections = [ RequiredSelection ]
        _code = Code.replace('<Location>',
                             "'"+self.requiredSelection.outputLocation()+"'")
        _code = _code.replace('\"\'', '\'').replace('\'\"', '\'').replace('\'\'','\'')
        self._alg = VoidFilter(self._name,
                               Code = _code)

        VoidEventSelection.__used_names.append(name)
        
    def name(self) :
        return self._name
    def algorithm(self) :
        return self._alg
    def outputLocation(self) :
        return self.requiredSelection.outputLocation()
    
class MergedSelection(object) :
    """
    Selection wrapper class for merging output of various Selections.
    Can be used just like any Selection object.
    Example:
    
    from PhysSelPython.Wrappers import MergedSelection, AutomaticData, SelectionSequence
    sel00 = AutomaticData(Location = 'Phys/Sel00')
    sel01 = AutomaticData(Location = 'Phys/Sel01')
    sel02 = AutomaticData(Location = 'Phys/Sel02')
    sel03 = AutomaticData(Location = 'Phys/Sel03')
    merge = MergedSelection('SelMerger', RequiredSelections = [sel00, sel01, sel02, sel03])
    seqMerge = SelectionSequence('SeqMergeSelections, TopSelection = merge)

    This will OR all the selections and place the output in merge.outputLocation().
    """
    def __init__(self,
                 name,
                 RequiredSelections = [],
                 OutputBranch = "Phys",
                 sequencerType=GaudiSequencer) :

        self.__ctor_dict__ = dict(locals())
        del self.__ctor_dict__['self']
        del self.__ctor_dict__['name']

        _algName = '_'+ name
        if configurableExists(_algName) :
            raise NameError('Target Configurable '+ name + ' already exists. Pick a new one')

        self._sel = Selection(name,
                              Algorithm = FilterDesktop(_algName,
                                                        Code='ALL'),
                              RequiredSelections = RequiredSelections,
                              OutputBranch = OutputBranch)
        
        self.algos = flatAlgorithmList(self._sel)

        self.requiredSelections = []
        self._name = name
        self._alg = sequencerType('Seq'+self.name(),
                                  Members = self.algos,
                                  ModeOR = True,
                                  ShortCircuit = False)

    def algorithm(self) :
        return self._alg
    def name(self) :
        return self._name
    def outputLocation(self) :
        return self._sel.outputLocation()

    def clone(self, name, **args) :
        new_dict = update_overlap(self.__ctor_dict__, args)
        return MergedSelection(name, **new_dict)

class SelectionSequence(SelSequence) :
    """
    Wrapper class for offline selection sequence creation.
    Takes a Selection object
    corresponding to the top selection algorithm, and recursively uses
    Selection.requiredSelections to form a GaudiSequencer with all the required
    selections needed to run the top selection. Can add list of event selection
    algorithms to be added at the beginning of the sequence, and a list of
    algorithms to be run straight after the selection algoritms.
    Wraps SelSequence, simply adding a method sequence() that creates the
    GaudiSequencer on-demand.
    Example: selection sequence for A -> B(bb), C(cc). Add pre-selectors alg0
             and alg1, and counter counter0.

    # Assume module A2B2bbC2cc defining a Selection object for the decay
    # A -> B(bb), C(cc)
    from A2B2bbC2cc import SelA2B2bbC2cc
    from PhysSelPython.Wrappers import SelectionSequence
    SeqA2B2bbC2cc = SelectionSequence( 'SeqA2B2bbC2cc',
                                       TopSelection = SelA2B2bbC2cc,
                                       EventPreSelector = [alg0, alg1],
                                       PostSelectionAlgs = [counter0]   )
    # use it
    mySelSeq = SeqA2B2bbC2cc.sequence()
    dv = DaVinci()
    dv.UserAlgorithms = [mySelSeq]

    Uses selection.SelSequence and selection.flatAlgorithmList
    help(SelSequence)
    help(flatAlgorithmList)
    """
    __author__ = "Juan Palacios juan.palacios@nikhef.nl"

    def __init__(self,
                 name,
                 TopSelection,
                 EventPreSelector = [],
                 PostSelectionAlgs = [],
                 sequencerType = GaudiSequencer) :
        SelSequence.__init__(self,
                             name,
                             TopSelection,
                             EventPreSelector,
                             PostSelectionAlgs)

        if configurableExists(self.name()) :
            raise NameError('Target Configurable '+ self.name() + ' already exists. Pick a new one.')

        self.gaudiseq = sequencerType(self.name(), Members = self.algos)

    def sequence(self) :
        return self.gaudiseq

    def clone(self, name, **args) :
        new_dict = update_overlap(self.__ctor_dict__, args)
        return SelectionSequence(name, **new_dict)


    
class MultiSelectionSequence(object) :
    """
    Wrapper class for offline multiple selection sequence creation.
    Takes a list of SelectionSequence objects and produces on demand a
    sequence with an OR of the list of sequences. 
    Uses SelectionSequence:
    help(SelSequence)
    """
    __author__ = "Juan Palacios juan.palacios@nikhef.nl"

    def __init__(self,
                 name,
                 Sequences = [],
                 sequencerType = GaudiSequencer) :

        self._name = name
        self.sequences = list(Sequences)
        self.gaudiseq = None
        self.algos = []
        for seq in self.sequences :
            self.algos += seq.algos

        if configurableExists(self.name()) :
            raise NameError('Target Configurable '+ self.name() + ' already exists. Pick a new one.')

        self.gaudiseq = sequencerType(self.name(),
                                      ModeOR = True,
                                      ShortCircuit = False,
                                      Members = [seq.sequence() for seq in self.sequences])
        
    def name(self) :
        return self._name
        
    def outputLocations(self) :
        return [seq.outputLocation() for seq in self.sequences]

    def sequence(self) :
        return self.gaudiseq
