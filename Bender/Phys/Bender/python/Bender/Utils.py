#!/usr/bin/env python 
# =============================================================================
# $Id$ 
# =============================================================================
## @file Bender/Utils.py
#
#  Set of helper functions  for Bender 
#
#  This file is a part of 
#  <a href="http://cern.ch/lhcb-comp/Analysis/Bender/index.html">Bender project</a>
#  <b>``Python-based Interactive Environment for Smart and Friendly 
#   Physics Analysis''</b>
#
#  The package has been designed with the kind help from
#  Pere MATO and Andrey TSAREGORODTSEV. 
#  And it is based on the 
#  <a href="http://cern.ch/lhcb-comp/Analysis/LoKi/index.html">LoKi project:</a>
#  ``C++ ToolKit for Smart and Friendly Physics Analysis''
#
#  By usage of this code one clearly states the disagreement 
#  with the smear campaign of Dr.O.Callot et al.: 
#  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
#
#  @date   2010-09-12
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#
#                    $Revision$
#  Last modification $Date$
#                 by $Author$ 
# =============================================================================
"""Set of helper functions  for Bender

oooooooooo.                              .o8                     
`888'   `Y8b                            \"888                     
 888     888  .ooooo.  ooo. .oo.    .oooo888   .ooooo.  oooo d8b 
 888oooo888' d88' `88b `888P\"Y88b  d88' `888  d88' `88b `888\"\"8P 
 888    `88b 888ooo888  888   888  888   888  888ooo888  888     
 888    .88P 888    .o  888   888  888   888  888    .o  888     
o888bood8P'  `Y8bod8P' o888o o888o `Y8bod88P\" `Y8bod8P' d888b    

This file is a part of BENDER project:
   ``Python-based Interactive Environment for Smart and Friendly Physics Analysis''

The project has been designed with the kind help from Pere MATO and Andrey TSAREGORODTSEV. 

And it is based on the LoKi project:
   ``C++ ToolKit for Smart and Friendly Physics Analysis''
   
By usage of this code one clearly states the disagreement 
with the smear campaign of Dr.O.Callot et al.: 
   ``No Vanya's lines are allowed in LHCb/Gaudi software.''

                    $Revision$
  Last modification $Date$
                 by $Author$  
"""
# =============================================================================
__author__  = 'Vanya BELYAEV Ivan.Belyaev@itep.ru'
__date__    = "2010-00-12"
__version__ = '$Revision$'
__all__     = (
    ##
    'run'               , ## run  N events
    'skip'              , ## skip N events 
    'next'              , ## go to run event
    'rewind'            , ## rewind 
    'ls'                ,
    'get'               ,
    'appMgr'            ,
    'dumpHistos'        , 
    ##
    'post_Action'       , 
    'pre_Action'        , 
    'setPostAction'     , 
    'setPreAction'      ,
    'setData'           ,
    ##
    )
# =============================================================================
## logging
# =============================================================================
from Bender.Logger import getLogger 
if '__main__' == __name__ : logger = getLogger ( 'Bender.Utils' )
else                      : logger = getLogger ( __name__ )
## ============================================================================
## post action 
__Bender_Post_Action = None 
## pre action 
__Bender_Pre_Action  = None
## ============================================================================
## post-action 
def post_Action () :
    """
    Get the ``post-action''
    """
    return __Bender_Post_Action
## ============================================================================
## post-action 
def pre_Action () :
    """
    Get the ``pre-action''
    """
    return __Bender_Pre_Action
## ============================================================================
## set new Post-action 
def setPostAction ( action ) :
    """
    set new ``post-action'' for Bender
    """
    global __Bender_Post_Action
    __Bender_Post_Action = action
    
## ============================================================================
## set new Pre-action 
def setPreAction ( action ) :
    """
    set new ``pre-action'' for Bender
    """
    global __Bender_Pre_Action
    __Bender_Pre_Action = action


# =============================================================================
## @class Action
#  simple ``action''/contect-manager for ``run''-fuction
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2016-02-15
class Action(object) :
    """ Simple ``action''/context-manager for ``run''-fuction
    """
    def __init__ ( self              ,
                   preAction  = None ,
                   postAction = None ) :
        
        self.preAction  = preAction  if preAction  else pre_Action  ()
        self.postAction = postAction if postAction else post_Action ()

    ## context manager: ENTER 
    def __enter__ ( self ) :

        if self.preAction :
            logger.debug ('Execute pre-action')
            self.preAction()
        return self
    
    ## context manager: EXIT
    def __exit__ ( self , *_ ) :
        
        if self.postAction :
            logger.debug ('Execute post-action')
            self.postAction()
                       
# =============================================================================
## run N events
#  @code
#  run(10)
#  @endcode 
def run ( nEvents     =   -1 ,
          postAction  = None ,
          preAction   = None ) :
    """Run gauidi  ( invoke 
    >>> run(50)
    """
    with Action ( preAction , postAction ) :
        
        ## get the application manager
        _g = appMgr() 
        st = _g.run ( nEvents )
        if st.isSuccess() and not get('/Event') :
            logger.warning('No more events in event selection...')
            st.setCode ( 2 )
            
    return st 

# =============================================================================
## go to next  event
#  @code 
#  next()
#  @endcode 
def next ( num = 1 )  :
    """Go to next event
    >>> next()
    """
    return run ( num )

# ==============================================================================
## define the input data for Bender job
def setData ( files            ,
              catalogs = []    ,
              castor   = False ,
              grid     = None  ) :
    """
    Define the input data for Bender job:
    
    >>> files    = [ 'file1.dst' , 'file2.dst' ]
    
    >>> catalogs = ....
    
    >>> import USERSCRIPT
    
    >>> USERSCRIPT.setData ( files , catalogs )
    
    """

        
    if   type ( files    ) is str   : files    =      [ files    ]
    elif type ( files    ) is tuple : files    = list ( files    ) 
    if   type ( catalogs ) is str   : catalogs =      [ catalogs ]    
    elif type ( catalogs ) is tuple : catalogs = list ( catalogs )
    
    if not issubclass ( type ( files    ) , list ) :
        files    = [ f for f in files    ] 
    if not issubclass ( type ( catalogs ) , list ) :
        catalogs = [ c for c in catalogs ] 
    
    from GaudiPython.Bindings import _gaudi

    if not _gaudi :               ## here we deal with configurables!

        from Configurables import Gaudi__RootCnvSvc
        rcnv = Gaudi__RootCnvSvc ( 'RootCnvSvc' )
        rcnv.CacheBranches = [   ]
        rcnv.VetoBranches  = ['*']    

        if files :
            
            from Bender.DataUtils import extendfile2
            files = [ extendfile2 ( f , castor , grid ) for f in files ]

            from Gaudi.Configuration import EventSelector
            inpts  = EventSelector().Input
            inpts += files  
            EventSelector ( Input = inpts )
            
        if catalogs :

            from Gaudi.Configuration import Gaudi__MultiFileCatalog as FileCatalog
            ctlgs  = FileCatalog().Catalogs

            cc     = [] 
            for c in catalogs :
                if c in cc : continue
                if 1<=c.find('.xml') : 
                    import os
                    if   os.path.exists ( c ) or 0 != c.find('xmlcatalog_file:') :
                        logger.debug ( 'Prepend catalog with protocol: %s' % c )
                        c = 'xmlcatalog_file:' + c
                if c in cc : continue
                cc.append ( c )
                
            logger.debug ( 'The catalogs: %s' % cc )
            ctlgs += cc 

            FileCatalog ( Catalogs = ctlgs )
            logger.debug ( 'FileCatalog:\n %s' % FileCatalog() )
        
            ## from Gaudi.Configuration import FileCatalog
            
    else :                        ## here we deal with the actual components


        rcnv = _gaudi.service('Gaudi::RootCnvSvc/RootCnvSvc')
        rcnv.CacheBranches = [   ]
        rcnv.VetoBranches  = ['*']    
        
        if files :
            
            from Bender.DataUtils import extendfile1
            files = [ extendfile1 ( f , castor ) for f in files ]
        
            _e = _gaudi.evtSel()
            _e.open ( files )
            
        if catalogs :
            _f = _gaudi.service ( 'FileCatalog' )
            _f.Catalogs = catalogs

## ditto...
setInputData = setData

## ============================================================================
## Get the application manager 
def appMgr( *varg , **kwarg ) :
    """
    Get the application manager 
    """
    from GaudiPython.Bindings import AppMgr 
    _g = AppMgr()
    if not 'LoKiSvc' in _g.ExtSvc : _g.ExtSvc += [ 'LoKiSvc']
    return _g

# =============================================================================
## define "easy"-function to browse TES
#  @code
#  ls('/Event')
#  ls('/Event/MC', forceload = True )
#  @endcode 
def ls  ( *args , **kwargs ) :
    """Browse the Transient Store

    >>> run(1)
    >>> ls()

    >>> ls('/Event/MC')
    >>> ls('/Event/MC', forceload = True)
    
    """
    _g  = appMgr()
    _es = _g.evtSvc()
    
    return _es.ls ( *args , **kwargs ) 

# =============================================================================
## define "easy-get"-function to get data for TES 
#  @code
#  run(1)    
#  o = get('/Event/MC')
#  o = get('/Event/MC/Particles' , MCPT > 1 )
#  @endcode 
def get  ( path , selector = lambda s : s ) :
    """Get object from  the Transient Store
    """
    _g  = appMgr()
    _es = _g.evtSvc()
    data = _es[path]
    #
    try : ## try to use the selector 
        return selector ( data )
    except:
        pass
    
    try : ## try to use selector 
        return filter ( selector , data )
    except :
        pass 
    
    return data 

# =============================================================================
## dump all histogram from the given component 
#  @code
#  dump( 'MakeKs' )
#  alg = ...
#  dumpHistos ( alg , 30 , 20 )
#  @endcode 
def dumpHistos ( o , *args ) :
    """Dump all histogram from the given component:
    >>> dump( 'MakeKs' )
    >>> alg = ...
    >>> dumpHistos ( alg , 30 , 20 )
    """
    if isinstance ( o , str ) :
        _g = appMgr()
        o  = _g.algorithm( o )
    return o.dumpHistos ( *args ) 

# =============================================================================
## Temporarily disable all algorithms
#  @code
#  with DisableAlgos() :
#       ... do something 
#  @endocde 
class DisabledAlgos(object) :
    """Temporarily disable all algorithms
    >>> with DisableAlgos() :
    >>> ... do something 
    """
    
    def __init__  ( self )  :
        self._disabled = ()

    ## enter context: disable everything 
    def __enter__ ( self ) :
        _g  = appMgr() 
        self._disabled = tuple( _g.disableAllAlgs() ) 
        return self._disabled

    ## exit the context: reenable algorithms 
    def __exit__ ( self , *_ ) :
        for _a in self._disabled : _a.setEnabled ( True ) 
        del self._disabled    


# =============================================================================
## skip the events
#  @code
#  skip ( 10 ) 
#  @endcode 
def skip ( nEvents ) : 
    """Skip N-events
    >>> skip ( 50 )    
    """
    from GaudiPython.Bindings import gbl as cpp 
    st = cpp.StatusCode(cpp.StatusCode.FAILURE,True)
    with DisabledAlgos() :
        st = run ( nEvents )
        
    return st 

# =============================================================================
## rewind
#  @code
#  rewind()
#  run(1)   ##   needed  to actually get the 1st event after rewind 
#  @endcode
#  @thanks Thomas RUF 
def rewind ( ) :
    """Rewind:
    >>> rewind() 
    >>> run(1)    ## needed to actually get the 1st event after rewind 
    """
    _g =    appMgr()
    with DisabledAlgos () :
        _g.evtSel().rewind() 
    
# =============================================================================
if __name__ == '__main__' :
    
    logger.info ( 80*'*'  ) 
    logger.info ( __doc__ ) 
    logger.info ( ' Author  : %s ' %  __author__  ) 
    logger.info ( ' Version : %s ' %  __version__ ) 
    logger.info ( ' Date    : %s ' %  __date__    ) 
    logger.info ( ' Symbols : %s ' %  list ( __all__ ) ) 
    logger.info ( 80*'*'  ) 
 
# =============================================================================
# The END 
# =============================================================================
