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
from CaloPIDs.PIDs            import referencePIDs
import logging
_log = logging.getLogger ('CaloPIDs')

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
        "Context"              : ''   # The context within which to run
        , "MeasureTime"        : False       # Measure the time for sequencers
        , "OutputLevel"        : INFO        # The global output level
        ##
        , 'Sequence'           : ''          # The sequencer to add the CALO reconstruction algorithms to
        , 'PIDList'            : ['InAcceptance',
                                  'Match',
                                  'Energy',
                                  'Chi2',
                                  'DLL',
                                  'NeutralPID'
                                  ] # List of PID fragments to be included (alternative full sequence per technique : [ 'EcalPID', 'BremPID', 'HcalPID', 'PrsPID', 'SpdPID', 'NeutralPID' ] )

        , 'EnablePIDsOnDemand' : False       # enable Reco-On-Demand
        ##
        , 'DataType'           : 'MC09'      # Data type  
        , 'TrackLocations'     : []          # track locations to be used (default use CaloAlgUtils default)
        , 'SkipNeutrals'       : False       # skip neutralID (already run in CaloRecoConf by default)
        , 'SkipCharged'        : False       # skip chargedID 
        , 'FastPID'            : False       # speed-up PID (lighter sequence)
        , 'ExternalClusters'   : ''          # use non-default cluster container
        , 'NoSpdPrs'           : False       # Upgrade configuration without Spd/Prs
        , 'Verbose'            : False
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
                         self.getProp('PIDList'),
                         _elocs  ,
                         self.getProp('SkipNeutrals'),
                         self.getProp('SkipCharged'),
                         self.getProp('FastPID'),
                         self.getProp('ExternalClusters'),
                         self.getName(),
                         self.getProp('NoSpdPrs')
                         ) 



        referencePIDs( self.getProp("DataType" ) )

        _log.info ('Configured Calo PIDs           : %s ' % cmp.name()  )
        ##
        return cmp 
    
    ## Check the configuration
    def checkConfiguration ( self ) :
        """
        Check the configuration
        """
        _log.debug('CaloPIDsConf: Configuration is not checked!')
    


    def printConf(self,verbose=False) :
        if self.getProp('NoSpdPrs') :
            _log.info("CaloPIDsConf : upgrade configuration without Spd/Prs")
        if self.getProp('Verbose') or verbose:
            _log.info ( self )


   ## Calorimeter PID Configuration
    def applyConf ( self ) :
        """
        Calorimeter PID Configuration
        """

        self.printConf()

        pids = self.caloPIDs()
        
        setTheProperty ( pids , 'Context'     , self.getProp('Context'    ) )
        setTheProperty ( pids , 'MeasureTime' , self.getProp('MeasureTime') )
        if self.isPropertySet("OutputLevel") :
            setTheProperty ( pids , 'OutputLevel' , self.getProp('OutputLevel') )

        if self.getProp ( 'Sequence' ) :
            addAlgs  ( self.Sequence , pids ) 
            _log.info ('Configure main Calo PIDs Sequence  : %s '% self.Sequence.name() )
            if self.getProp('Verbose') :
                _log.info ( prntCmp ( self.Sequence ) ) 
        else :
            _log.info ('Configure Calorimeter PIDs blocks ' )            
            if self.getProp('Verbose') :
                _log.info ( prntCmp ( pids ) )
            
                    
        if self.getProp( 'EnablePIDsOnDemand')  :
            _log.info ( printOnDemand () )             




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
