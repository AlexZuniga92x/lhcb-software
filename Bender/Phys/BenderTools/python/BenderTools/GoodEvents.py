#!/usr/bin/env python 
# =============================================================================
# $Id:$ 
# =============================================================================
## @file BenderTools/GoodEvents.py
#
#  Set  of utiltiles to copy ``good''-events 
#
#  This file is a part of 
#  <a href="http://cern.ch/lhcb-comp/Analysis/Bender/index.html">Bender project</a>
#  <b>``Python-based Interactive Environment for Smart and Friendly Physics Analysis''</b>
#
#  The package has been designed with the kind help from
#  Pere MATO and Andrey TSAREGORODTSEV. 
#  And it is based on the 
#  <a href="http://cern.ch/lhcb-comp/Analysis/LoKi/index.html">LoKi project:</a>
#  <b>``C++ ToolKit for Smart and Friendly Physics Analysis''</b>
#
#  By usage of this code one clearly states the disagreement 
#  with the smear campaign of Dr.O.Callot et al.: 
#  ``No Vanya's lines are allowed in LHCb/Gaudi software''
#

#  @date   2012-06-12
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#
#                    $Revision: 138072 $
#  Last modification $Date: 2012-04-04 16:41:10 +0200 (Wed, 04 Apr 2012) $
#                 by $Author: ibelyaev $
# =============================================================================
"""

Set  of utiltiles to copy ``good''-events 

This file is a part of BENDER project:

  ``Python-based Interactive Environment for Smart and Friendly Physics Analysis''

The project has been designed with the kind help from Pere MATO and Andrey TSAREGORODTSEV. 

And it is based on the LoKi project:
 
   ``C++ ToolKit for Smart and Friendly Physics Analysis''

By usage of this code one clearly states the disagreement with the smear campaign of Dr.O.Callot et al.:

   ``No Vanya's lines are allowed in LHCb/Gaudi software''

"""
# =============================================================================
__author__  = 'Vanya BELYAEV Ivan.Belyaev@itep.ru'
__date__    = "2012-06-12"
__version__ = '$Revision: 138072 $'
__all__     = (
    #
    'fireIncident'      ,
    'tagEvent'          ,    
    'markEvent'         ,
    'copyGoodEvents'    ,
    'writeEvent'        ,
    #
    )  
# =============================================================================
## logging
# =============================================================================
from Bender.Logger import getLogger 
logger = getLogger( __name__ )
# =============================================================================

# ===========================================================================
## fire the incident with given name
def fireIncident ( name   = "GoodEvent" ,
                   source = "Bender"    ) :
    """
    Fire the incident
    
    >>> fireIncident()
    
    """
    from Bender.Utils import appMgr
    _g  = appMgr    ()
    _is = _g.incSvc ()
    _is.fireIncident ( name , source )

# =============================================================================
## fire the incident with the given name    
tagEvent  = fireIncident
markEvent = fireIncident

# =============================================================================
## helper function to write interesint event into the separate stream
#
#  It is assumed that the proper configuration is done with
#   "copyGoodEvent"  function  ( static CONFIGURABLE)
#
#  @code
#
#  #
#  ##Configurables:
#  #
#  from BenderTools.GoodEvents import copyGoodEvents
#  copyGoodEvents ( filename = 'SelectedEvents' )
#
#  ...
#
#  #
#  ## run-time command line: 
#  #
#  >>> writeEvent () 
#
#  #
#  ## run-time (algorithm)
#  #
#          ...
#          writeEvent()
#          ...
#  @endcode 
#
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date   2011-12-17
#
#  @see copyGoodEvents
#
def writeEvent ( name = "GoodEvent" ) :
    """
    Helper function to write interesint event into the separate stream

    It is assumed that the proper configuration is done with
    copyGoodEvent  function  ( static CONFIGURABLE)
    
    #
    ##Configurables:
    #
    from BenderTools.GoodEvents import copyGoodEvents
    copyGoodEvents ( filename = 'SelectedEvents' )
    
    ...
    
    #
    ## run-time command line: 
    #
    >>> writeEvent () 
    
    #
    ## run-time (algorithm)
    #
                  ...

                  writeEvent()
                  ...
    
    """
    from Bender.Utils import appMgr
    _g     = appMgr()      
    _icntx = _g.cntxSvc ()
    #
    ## get the "current" algorithm from ContextService 
    _curr  = _icntx.current()
    if not _curr :
        ## we are out of event-scope ?
        #  try to execute manually the writer 
        writer = _g.algorithm ( name )
        if not writer._ialg : writer.retrieveInterface()
        if not writer._ialg :
            raise RuntimeError , 'Unable to find writer "%s"' % name
        # run it! 
        return writer.sysExecute()

    #
    ## otherwise fire the incident
    #
    fireIncident ( name )
    #
    from GaudiPython.Bindings import SUCCESS
    #
    return SUCCESS 
    
# ============================================================================
## Utility to copy selected events to some file
#  It is based on configurables,
#  thus it needs to be invoked BEFORE GaudiPython
#
#  @code
#
#  #
#  ##Configurables:
#  #
#  from BenderTools.GoodEvents import copyGoodEvents
#  copyGoodEvents ( filename = 'SelectedEvents' )
#
#  @endcode 
#
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date 2011-12-16
#  @see InputCopyStream
#  @see IOHelper
#  @see writeEvents
#  @see Gaudi::IncidentFilter
#  @see fireIncident
def copyGoodEvents ( filename                 ,
                     extraitems = []          ,
                     incident   = 'GoodEvent' ) :
    """
    Utitity to copy ``good/tagged'' events

    It is based on configurables,
    thus it needs to be invoked BEFORE GaudiPython
    
    #
    ##Configurables:
    #
    from Bender.Utils import copyGoodEvents
    copyGoodEvents ( filename = 'SelectedEvents' )
    
    """
    from GaudiConf import IOHelper
    ioh   = IOHelper       ( 'ROOT' , 'ROOT' ) 
    algs  = ioh.outputAlgs ( filename , 'InputCopyStream/%s' % incident )
    
    ioh.setupServices()
    from Configurables import Gaudi__IncidentFilter as Tagger    
    tag   = Tagger ( "%sInc"   % incident , Incidents = [ incident] ) 
    
    from Configurables import GaudiSequencer
    seq   = GaudiSequencer ( '%sSeq' % incident        ,
                             Members = [ tag ] +  algs ) 

    from Configurables import ApplicationMgr
    AM    = ApplicationMgr ()
    
    if not AM.OutStream : AM.OutStream =[]
    
    AM.OutStream.append ( seq )
    
    logger.info ( "Prepare the file %s to keep ``%s'' incidents" % ( filename ,
                                                                     incident ) )
    

        
# =============================================================================
if __name__ == '__main__' :
    print '*'*120
    print                      __doc__
    print ' Author  : %s ' %   __author__    
    print ' Version : %s ' %   __version__
    print ' Date    : %s ' %   __date__
    print '*'*120
    
# =============================================================================

# =============================================================================
# The END 
# =============================================================================

