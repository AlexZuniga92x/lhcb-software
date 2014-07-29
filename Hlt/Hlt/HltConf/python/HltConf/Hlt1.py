#!/usr/bin/env gaudirun.py
# =============================================================================
# $Id: Hlt1.py,v 1.44 2010-08-22 22:50:46 gligorov Exp $
# =============================================================================
## @file
#  Configuration of HLT1
#  @author Gerhard Raven Gerhard.Raven@nikhef.nl
#  @date 2008-08-25
# =============================================================================
"""
 script to configure HLT1 trigger lines
 Note: the actual line definitions are NOT in this file, 
 this script relies on them being available through hlt1Lines()
"""
# =============================================================================
__author__  = "Gerhard Raven Gerhard.Raven@nikhef.nl"
__version__ = "CVS Tag $Name: not supported by cvs2svn $, $Revision: 1.44 $"
# =============================================================================

from Gaudi.Configuration import * 
from LHCbKernel.Configuration import *
def import_line_configurables(pkg) :
    # import all modules in Hlt1Lines, and require each module xyz to contain a class xyzConf
    # i.e. do the equivalent of 
    #    from pkg.xyz import xyzConf 
    #
    import os.path, pkgutil, importlib
    return  [ getattr( importlib.import_module(pkg.__name__+'.'+name), name+'Conf' ) 
              for _,name,_ in pkgutil.iter_modules([os.path.dirname(pkg.__file__)]) 
              if name.endswith('Lines') ]

#import all Hlt1 lines configurables  -- and make sure the are kept alive long enough!
import Hlt1Lines
__hlt1linesconfs = import_line_configurables(Hlt1Lines)
# add them in our scope so that genConfUser can find it... 
globals().update( ( cfg.__name__, cfg ) for cfg in __hlt1linesconfs )

class Hlt1Conf(LHCbConfigurableUser):
   __used_configurables__ = import_line_configurables(Hlt1Lines)

   __slots__ = { "ThresholdSettings"            : {} # dictionary decoded in HltThresholdSettings
               }

##################################################################################
#
#      
   def __apply_configuration__(self):
      ## New event model
      import HltTracking.Hlt1TrackUpgradeConf
      from Configurables import LoKi__Hybrid__CoreFactory as CoreFactory
      factory = CoreFactory( "Hlt1Factory" )
      for m in [ "LoKiCore.decorators"    ,
                 "LoKiTracks.decorators"  ,
                 "LoKiTrigger.decorators" ,
                 "LoKiNumbers.decorators" ,
                 "LoKiCore.functions"     ,
                 "LoKiCore.math"          ,
                 "LoKiHlt.algorithms"     ,
                 "LoKiPhys.decorators"    ] :
         if not m in factory.Modules : factory.Modules.append ( m )
      factory.Lines += [
         "from GaudiKernel.SystemOfUnits import GeV, MeV, mm" ,
         "import HltTracking.Hlt1StreamerConf"
         ]

      ## Existing stuff
      from Configurables import GaudiSequencer as Sequence
      from HltLine.HltLinesConfigurableUser import HltLinesConfigurableUser
      ThresholdSettings = self.getProp("ThresholdSettings")
      
      for i in  self.__used_configurables__  :
         if not issubclass(i,HltLinesConfigurableUser) : continue
         from ThresholdUtils import setThresholds
         setThresholds(ThresholdSettings,i)
         
      # add a few thing to our printout
      from HltLine.HltLine     import addHlt1Prop
      addHlt1Prop([ 'RoutingBits', 'Accept', 'FilterDescriptor'
                  , 'Code', 'Preambulo', 'InputLocations', 'Input','Inputs', 'Output','OutputProtoParticleLocation','InputTrackLocation'
                  , 'DaughtersCuts', 'CombinationCut', 'MotherCut', 'DecayDescriptor'
                  , 'OutputSelection','Context' ])
      
      
      ## finally, define the Hlt1 sequence!!
      Sequence('Hlt1',  ModeOR = True, ShortCircuit = False )

      from HltLine.HltLine     import Hlt1Line
      Hlt1Line( 'Global', HLT= "HLT_PASS_SUBSTR('Hlt1') ", priority = 255 ) 
