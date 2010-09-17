#!/usr/bin/env python 
# =============================================================================
# $Id: Utils.py,v 1.2 2010-09-17 17:56:12 ibelyaev Exp $ 
# =============================================================================
# $URL$
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
#  with the campain of Dr.O.Callot et al.: 
#  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
#
#  @date   2010-09-12
#  @author Vanya BELYAEV Ivan.Belyaev@nikhef.nl
#
#  Last modification $Date: 2010-09-17 17:56:12 $
#                 by $Author: ibelyaev $ 
# =============================================================================
"""
Set of helper functions  for Bender 

This file is a part of BENDER project:
   ``Python-based Interactive Environment for Smart and Friendly Physics Analysis''

The project has been designed with the kind help from Pere MATO and Andrey TSAREGORODTSEV. 

And it is based on the LoKi project:
   ``C++ ToolKit for Smart and Friendly Physics Analysis''

By usage of this code one clearly states the disagreement 
with the campain of Dr.O.Callot et al.: 
   ``No Vanya's lines are allowed in LHCb/Gaudi software.''

  Last modification $Date: 2010-09-17 17:56:12 $
                 by $Author: ibelyaev $  
"""
# =============================================================================
__author__  = 'Vanya BELYAEV Ivan.Belyaev@nikhef.nl'
__date__    = "2010-00-12"
__version__ = 'Version $Revision: 1.2 $'
__all__     = (
    ##
    'run'               , 
    'skip'              , 
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
    'seekVoidDecision'  ,
    'seekAlgDecision'   ,
    'seekHltDecision'   ,
    'seekStripDecision' 
    )
# =============================================================================

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
## run events 
def run ( nEvents     =   -1 ,
          postAction  = None ,
          preAction   = None ) :
    """
    Run gaudi

    >>> run(50)
    
    """
    ## apply "pre-action", if defined 
    if   preAction    : preAction ()
    elif pre_Action() :
        action = pre_Action()
        action() 
        
    ## get the application manager
    _g = appMgr() 
    st = _g.run ( nEvents )
    
    ## apply "post-action", if defined 
    if   postAction    : postAction ()
    elif post_Action() :
        action = post_Action()
        action() 
        
    return st 


# ==============================================================================
## define the input data for Bender job
def setData ( files , catalogs = [] ) :
    """
    Define the input data for Bender job:
    
    >>> files    = [ 'file1.dst' , 'file2.dst' ]
    
    >>> catalogs = ....
    
    >>> import USERSCRIPT
    
    >>> USERSCRIPT.setData ( files , catalogs )
    
    """
    from GaudiPython.Bindings import _gaudi
    
    if   type ( files    ) is str   : files    =      [ files    ]
    elif type ( files    ) is tuple : files    = list ( files    ) 
    if   type ( catalogs ) is str   : catalogs =      [ catalogs ]    
    elif type ( catalogs ) is tuple : catalogs = list ( catalogs )
    
    if not issubclass ( type ( files    ) , list ) :
        files    = [ f for f in files    ] 
    if not issubclass ( type ( catalogs ) , list ) :
        catalogs = [ c for c in catalogs ] 
    
    if not _gaudi :               ## here we deal with configurables!
        
        if files :
            
            from Bender.DataUtils import extendfile2
            files = [ extendfile2 ( f ) for f in files ]

            print 'I AM FILES' , files
            
            from Gaudi.Configuration import EventSelector
            EventSelector ( Input = files )
            
        if catalogs :
            
            from Gaudi.Configuration import Gaudi__MultiFileCatalog as FileCatalog
            FileCatalog   ( Catalogs = catalogs )
            from Gaudi.Configuration import FileCatalog
            FileCatalog   ( Catalogs = catalogs )
            
    else :                        ## here we deal with the actual components
        
        if files :
            
            from Bender.DataUtils import extendfile1
            files = [ extendfile1 ( f ) for f in files ]
        
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
def ls  ( *args ) :
    """
    Browse the Transient Store

    >>> run(1)
    >>> ls()

    >>> ls('/Event/Strip')
    
    """
    _g  = appMgr()
    _es = _g.evtSvc()
    return _es.ls ( *args ) 

# =============================================================================
## define "easy-get"-function to get data for TES 
def get  ( path ) :
    """
    Get object from  the Transient Store

    >>> run(1)

    >>> o = get('/Event/Strip')
    
    """
    _g  = appMgr()
    _es = _g.evtSvc()
    return _es[ path ]

# =============================================================================
## dump all histogram from the given component 
def dumpHistos ( o , *args ) :
    """
    Dump all histogram from the given component

    >>> dump( 'MakeKs' )

    >>> alg = ...
    >>> dumpHistos ( alg , 30 , 20 )
    """
    if isinstance ( o , str ) :
        _g = appMgr()
        o  = _g.algorithm( o )
    return o.dumpHistos ( *args ) 

# =============================================================================
## seek the decision for the certain void-functor 
def seekVoidDecision ( fun , EvtMax = 1000 ) :
    """
    Seek the Decision for the certains algorithm    
    """
    ok  = fun()
    if ok : return True 

    evt = 0 
    while  not ok and evt < EvtMax :
        run(1)
        ok   = fun()
        evt += 1
        
    return ok, evt  

# =============================================================================
## seek the decision for the certain algorithm
def seekAlgDecision ( alg , EvtMax = 1000 ) :
    """
    Seek the Decision for the certains algorithm    
    """

    fun = alg
    
    if isinstance ( alg , str ) :
        from LoKiHlt.algorithms import ALG_EXECUTED, ALG_PASSED
        fun = ALG_EXECUTED  ( alg ) & ALG_PASSED ( alg )
        
    return seekVoidDecision ( fun ) 

# =============================================================================
## seek the decision for the certain Trigger Line 
def seekHltDecision ( expr                                 ,
                      EvtMax     = 10000                   ,
                      disableAll = False                   ,
                      location   = '/Event/Hlt/DecReports' ) :
    
    from LoKiHlt.decorators import HLT_PASS_RE 

    fun = HLT_PASS_RE ( expr ) if isinstance ( expr , str ) else expr 
    
    hlt = get ( location )
    if not hlt : return False , 0     ## RETURN 

    ok  = fun ( hlt )
    if ok      : return True  , 0     ## RETURN 
    
    evt = 0

    _g        = appMgr()

    ## disable the algorithms 
    _disabled = _g.disableAllAlgs() if disableAll else _g.disableTopAlgs()
    
    try : 
        while  not ok and evt < EvtMax :
            run(1)
            hlt  = get ( location )
            if not hlt : break            ## BREAK 
            ok   = fun ( hlt )
            evt += 1
    finally :
        for _a in _disabled :
            _a.setEnable ( True )
        
        
    return ok, evt

# =============================================================================
## seek the decision for the certain stripping line 
def seekStripDecision ( expr                                 ,
                        EvtMax     = 10000                   ,
                        disableAll = False                   ,
                        location   = '/Event/Hlt/DecReports' ) :
    """
    Seek the decision for the certain Stripping Line
    """
    return seekHltDecision ( expr       ,
                             EvtMax     ,
                             disableAll , 
                             '/Event/Strip/Phys/DecReports' ) 

    
# =============================================================================
## skip the events 
def skip ( nEvents ) : 
    """
    Skip events
    
    >>> skip ( 50 )
    
    """
    _g = appMgr()

    ## disable all algorithms
    _disabled = _g.disableAllAlgs()

    try:
        st = run ( nEvents )
    finally:
        for _a in _disabled :
            _a.setEnabled ( True )

    return st 



# =============================================================================
if __name__ == '__main__' :
    print '*'*120
    print                      __doc__
    print ' Author  : %s ' %   __author__    
    print ' Version : %s ' %   __version__
    print ' Date    : %s ' %   __date__
    print '*'*120
    

# =============================================================================
# The END 
# =============================================================================
