'''Classes and functions to control the configuration of output streams used for DST writing. '''

__author__ = "Juan PALACIOS juan.palacios@nikhef.nl"

__all__ = ('OutputStreamConf')

from copy import copy

class OutputStreamConf(object) :
    def __init__(self,
                 streamType = None,
                 filePrefix = 'Sel',
                 fileExtension = '.dst',
                 extraItems = [],
                 vetoItems  = [],
                 selectiveRawEvent = False,
                 killTESAddressHistory = True ) :
        self.streamType = streamType
        self.filePrefix = filePrefix
        self.extension  = fileExtension
        self.extraItems = copy(extraItems)
        self.vetoItems  = vetoItems
        self.selectiveRawEvent = selectiveRawEvent
        self.killTESAddressHistory = killTESAddressHistory
        self.name = ''

    def __str__(self) :
        output = '\nOutputStreamConfXX\n'
        output += 'name       : '+ self.name + '\n'
        output += 'streamType : '+ str(self.streamType) + '\n'
        output += 'filePrefix : '+ str(self.filePrefix) + '\n'
        output += 'extension  : '+ str(self.extension) + '\n'
        output += 'extraItems : '+ str(self.extraItems) + '\n'
        output += 'selectiveRawEvent : '+ str(self.selectiveRawEvent) + '\n'
        output += 'killTESAddressHistory : '+ str(self.killTESAddressHistory) + '\n'
        return output
