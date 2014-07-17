"""
 script to configure HLT2 trigger lines

 @author P. Koppenburg
 @date 2009-07-08
"""
# =============================================================================
__author__  = "P. Koppenburg Patrick.Koppenburg@cern.ch"
__version__ = "CVS Tag $Name: not supported by cvs2svn $, $Revision: 1.72 $"
# =============================================================================
import types
from Gaudi.Configuration import *
from LHCbKernel.Configuration import *
from Configurables import GaudiSequencer as Sequence

def import_line_configurables(pkg) :
    # import all modules in Hlt2Lines, and require each file xyz to contain a class xyzConf
    # i.e. do the equivalent of 
    #    from Hlt2Lines.Hlt2SomeLines import Hlt2SomeLinesConf 
    #
    import os.path, pkgutil, importlib
    return  [ getattr( importlib.import_module(pkg.__name__+'.'+name), name+'Conf' ) 
              for _,name,_ in pkgutil.iter_modules([os.path.dirname(pkg.__file__)]) ]


#import all Hlt2 lines configurables in local scope so that genConfUser can find it... (i.e. make sure it is in 'dir()')
import Hlt2Lines
def expose( tps, nm ) : 
    return [ '%s = %s[%d]' % ( i.__name__, nm, j ) for (j,i) in enumerate( tps ) ]
__hlt2linesconfs = import_line_configurables(Hlt2Lines)
for _ in expose(__hlt2linesconfs,'__hlt2linesconfs') : exec(_)

#
# The tracking configurations
#
from HltTracking.Hlt2Tracking import Hlt2Tracking
#
from HltTracking.Hlt2TrackingConfigurations import Hlt2BiKalmanFittedDownstreamTracking
from HltTracking.Hlt2TrackingConfigurations import Hlt2BiKalmanFittedForwardTracking
from HltTracking.Hlt2TrackingConfigurations import Hlt2BiKalmanFittedRichForProtonsForwardTracking
from HltTracking.Hlt2TrackingConfigurations import Hlt2BiKalmanFittedRichForLowPTParticlesForwardTracking
# Define what categories stand for
# There are the strings used in HltThresholdSettings


class Hlt2Conf(LHCbConfigurableUser):
    __used_configurables__ = [ (Hlt2Tracking, "Hlt2BiKalmanFittedRichForProtonsForwardTracking")
                             , (Hlt2Tracking, "Hlt2BiKalmanFittedRichForLowPTParticlesForwardTracking")
                             , (Hlt2Tracking, "Hlt2BiKalmanFittedDownstreamTracking") 
                             , (Hlt2Tracking, "Hlt2BiKalmanFittedForwardTracking")
                             ] + import_line_configurables(Hlt2Lines)

    __slots__ = { "DataType"                   : '2010'    # datatype is one of 2009, MC09, DC06...
                , "ThresholdSettings"          : {} # ThresholdSettings predefined by Configuration
                , "DefaultVoidFilter"          : ''
                , "Hlt2ForwardMaxVelo"         : 1000
                , "Hlt1TrackOption" : "Decode" #what to do with tracks from Hlt1?
                }
    #__known_hlt1trackoptions__ = ["Decode","Encode-Decode","Rerun","Copy"]
    __known_hlt1trackoptions__ = ["Decode","Rerun"]
    #############################  ^ take from HLT1Track raw banks
    #############################            ^ Create banks and then decode them
    ############################# Recreate tracks from scratch ^
    ############################# Direct Copy from Hlt1 location(s)  ^
###################################################################################
#
# Threshold settings
#
# This is copied and edited from Hlt1.confType().
# One could outsource that to some function, but who cares?
#
    def hlt2Lines(self,Hlt2):
        """
        The actual lines
        """
        from HltLine.HltLine     import Hlt2Line
        Hlt2Line( "Global", HLT= "HLT_PASS_SUBSTR('Hlt2') ", priority = 255, VoidFilter = '' )
        ThresholdSettings = self.getProp("ThresholdSettings")

        #
        # check if this threshold setting has some global event cut...
        #  if so, set it, and add a line which does (prescaled) the opposite
        #
        if self.getProp('DefaultVoidFilter') :
            from HltLine.HltLine import Hlt2Line
            Hlt2Line.setDefaultVoidFilter( self.getProp('DefaultVoidFilter') )
            Hlt2Line( 'PileUp', VoidFilter = '  ~ ( %s ) ' % self.getProp('DefaultVoidFilter') , postscale = 0.01 )
        #
        # Loop over thresholds
        #
        from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser
        for i in self.__used_configurables__ :
            # Time for some python magic
            if type(i) is tuple : # if we are dealing with a named instance in the used configurables
                i, i_name = i # copy what is done in GaudiKernel/Configurable.py 
            if not issubclass(i,HltLinesConfigurableUser) : continue
            from ThresholdUtils import setThresholds
            setThresholds(ThresholdSettings,i)
       
###################################################################################
#
# Reconstruction
#
    def configureReconstruction(self):

        definedTrackings = [ Hlt2BiKalmanFittedDownstreamTracking()
                           , Hlt2BiKalmanFittedForwardTracking()
                           , Hlt2BiKalmanFittedRichForProtonsForwardTracking()
                           , Hlt2BiKalmanFittedRichForLowPTParticlesForwardTracking() ]

        # And now we have to, for each of the configurables we just created, 
        # tell it the data type and tell it to use all the Hlt2 lines...
        from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser
        from Gaudi.Configuration import ConfigurableUser
        from HltTracking.Hlt2TrackingConfigurations import setDataTypeForTracking
        for thistracking in definedTrackings :
            setDataTypeForTracking(thistracking,self.getProp("DataType"))
            if self.getProp('Hlt2ForwardMaxVelo') : thistracking.Hlt2ForwardMaxVelo = self.getProp("Hlt2ForwardMaxVelo")
        
###################################################################################
#
# Main configuration
#
    def __apply_configuration__(self):
        """
        Hlt2 configuration
        """
        if self.getProp("Hlt1TrackOption") not in self.__known_hlt1trackoptions__:
            raise ValueError("You must choose a valid Hlt1TrackOption for the input of Hlt2, you chose "+self.getProp("Hlt1TrackOption")+" which is not one of: "+self.__known_hlt1trackoptions__.__str__())
        Hlt2 = Sequence("Hlt2", Context = 'HLT',ModeOR=True,ShortCircuit=False) 
        # set Hlt2 PID
        self.configureReconstruction()
        # lines
        if self.getProp("DefaultVoidFilter") :
            from HltLine.HltLine import Hlt2Line  
            Hlt2Line.setDefaultVoidFilter( self.getProp("DefaultVoidFilter") )
        self.hlt2Lines(Hlt2)
