#!/usr/bin/env gaudirun.py
# =============================================================================
## Configurable for Calorimeter PID 
#  @author Vanya BELYAEV Ivan.Belyaev@nikhe.nl
#  @date 2008-07-17
# =============================================================================
"""
Configurable for Calorimeter PID
"""
# =============================================================================
__author__  = "Vanya BELYAEV Ivan.Belyaev@nikhef.nl"
__version__ = "CVS tag $Name: not supported by cvs2svn $, version $Revision: 1.4 $"
# =============================================================================
__all__   = (
    'HltCaloPIDsConf'     ,
    'OffLineCaloPIDsConf' ,
    'CaloPIDsConf'
    )
# =============================================================================

from LHCbKernel.Configuration import *
from Configurables            import HistogramSvc

from CaloKernel.ConfUtils     import ( addAlgs        ,
                                       printOnDemand  ,
                                       prntCmp        ,
                                       hltContext     ,
                                       setTheProperty ) 
from CaloPIDs.PIDs            import caloPIDs

# =============================================================================
## @class CaloPIDsConf
#  Configurable for Calorimeter PID
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2008-07-17
class CaloPIDsConf(LHCbConfigurableUser):
    """
    Class/Configurable to define the calorimeter PID
    """
    ## define the slots
    __slots__ = {
        ##
        "Context"              : "Offline"   # The context within which to run
        , "MeasureTime"        : True        # Measure the time for sequencers
        , "OutputLevel"        : INFO        # The global output level
        ##
        , 'Sequence'           : ''          # The sequencer to add the CALO reconstruction algorithms to
        , 'EnablePIDsOnDemand' : False       # enable Reco-On-Demand
        ##
        , 'DataType'           : 'MC09'      # Data type  
        , 'TrackLocations'     : []          # track locations to be used (default use CaloAlgUtils default)
        , 'SkipNeutrals'       : False       # skip neutralID (already run in CaloRecoConf by default)
        , 'SkipCharged'        : False       # skip chargedID 
        }
    
    ## Configure recontruction of Calo Charged  PIDs  
    def caloPIDs ( self ) :
        """
        Configure recontruction of Calo Charged  PIDs 
        """

        ## confuse configurable on purpose 
        _locs = self.getProp ( 'TrackLocations'    ) 
        _elocs = [] 
        for l in _locs :
            if l.find( '/Event/' )  != 0 :
                l = '/Event/' + l
            _elocs.append( l )



        cmp = caloPIDs ( self.getProp( 'Context'            )  ,
                         self.getProp( 'EnablePIDsOnDemand' )  ,
                         _elocs  ,
                         self.getProp('SkipNeutrals'),
                         self.getProp('SkipCharged')
                         ) 
        log.info ('Configured Calo PIDs           : %s ' % cmp.name()  )
        ##
        return cmp 
    
    ## Check the configuration
    def checkConfiguration ( self ) :
        """
        Check the configuration
        """
        log.debug('CaloPIDsConf: Configuration is not checked!')
    
   ## Calorimeter PID Configuration
    def applyConf ( self ) :
        """
        Calorimeter PID Configuration
        """
        log.info ( 'Apply Calo PIDs Configuration ')

        self.checkConfiguration()

        
        pids = self.caloPIDs()
        
        setTheProperty ( pids , 'Context'     , self.getProp('Context'    ) )
        setTheProperty ( pids , 'OutputLevel' , self.getProp('OutputLevel') )
        setTheProperty ( pids , 'MeasureTime' , self.getProp('MeasureTime') )

        if self.getProp ( 'Sequence' ) :
            addAlgs  ( self.Sequence , pids ) 
            log.info ('Configure main Calo PIDs Sequence  : %s '% self.Sequence.name() )
            log.info ( prntCmp ( self.Sequence ) ) 
        else :
            log.info ('Configure Calorimeter PIDs blocks ' )            
            log.info ( prntCmp ( pids ) )
            
        # configure the histogram input: 
        hsvc = HistogramSvc ( 'HistogramDataSvc' )
        inputs = hsvc.Input
        found  = False  
        for line in inputs :
            if 0 == line.find ( 'CaloPIDs') : found = True
            
        if found :
            log.info ("CaloPIDsConf: LUN 'CaloPIDs' has been defined already") 
        elif 'DC06' == self.getProp('DataType') :
            hsvc.Input += [ "CaloPIDs DATAFILE='$PARAMFILESROOT/data/CaloPIDs_DC06_v2.root' TYP='ROOT'" ]
        else:
            hsvc.Input += [ "CaloPIDs DATAFILE='$PARAMFILESROOT/data/CaloPIDs_DC09_v1.root' TYP='ROOT'" ]

                    
        if self.getProp( 'EnablePIDsOnDemand')  :
            log.info ( printOnDemand () ) 

        ##print ' ON-DEMAND ' , printOnDemand () 

# =============================================================================
## @class HltCaloPIDsConf
#  Configurable for Calorimeter PID
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2008-07-17
class HltCaloPIDsConf(CaloPIDsConf):
    """
    Class/Configurable to define the calorimeter PID for Hlt 
    """
    __slots__ = {}
    
    ## Check the configuration
    def checkConfiguration ( self ) :
        """
        Check the configuration
        """
        if not hltContext ( self.getProp('Context') ) :
            raise AttributeError, 'Invalid context for HltCaloPIDsConf'
        
# =============================================================================
## @class OffLineCaloPIDsConf
#  Configurable for Calorimeter PID
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#  @date 2008-07-17
class OffLineCaloPIDsConf(CaloPIDsConf):
    """
    Class/Configurable to define the calorimeter PID for Off-Line 
    """
    __slots__ = {}
    
    ## Check the configuration
    def checkConfiguration ( self ) :
        """
        Check the configuration
        """
        if hltContext ( self.getProp('Context') ) :
            raise AttributeError, 'Invalid context for OffLineCaloPIDsConf'
        


        
# =============================================================================
# The END
# =============================================================================
