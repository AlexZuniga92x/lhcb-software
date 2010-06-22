''' Helpers for DST writing.
'''
__author__ = "Juan Palacios <juan.palacios@nikhef.nl>"

__all__ = ('ConfigurableList', 'setCloneFilteredParticlesToTrue', 'flattenList')

def isIterable(obj) :
    '''
    Test if an object is iterable but not a string type.
    '''
    return hasattr(obj, "__iter__") and not isinstance(obj, basestring) 

def flattenList(sequence) :
    """flattenList(sequence) -> flat list
    Return a flat list containing all the non-iterable elements retrieved sequence and sequence's sub-sequences. 
    """
    flatList = []
    for member in sequence :
        if isIterable(member) :
            flatList.extend(flattenList(member))
        else :
            flatList.append(member)
    return flatList

def setCloneFilteredParticlesToTrue(algs) :
    for alg in algs :
        try :
            alg.CloneFilteredParticles = True
            print 'Set CloneFilteredParticles of', alg.name(), ' to True'
        except :
            try :
                setCloneFilteredParticlesToTrue(alg.Members)
            except :
                pass

class ConfigurableList(object) :
    '''
    Return a list of plain configurables from a SelectionSequence style object.
    Intended to deal with difference in interfaces of SelectionSequence, StrippingStream, etc.
    '''
    _algos = { 'DEFAULT' :lambda x : x.__getattribute__('algos'),
               'StrippingStream' : lambda x : x.filterMembers(),
               'StrippingLine' : lambda x : x.filterMembers(),
               'StrippingConf' : lambda x : x.filterMembers()
               }
    def __init__(self, selection) :
        self._sel = selection
    def name(self) :
        return type(self._sel).__name__
    def flatList(self) :
        if self.name() in ConfigurableList._algos.keys() :
            return ConfigurableList._algos[self.name()](self._sel)
        else :
            return ConfigurableList._algos['DEFAULT'](self._sel)

