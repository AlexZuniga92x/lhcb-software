#!/usr/bin/env python
# -*- coding: utf-8 -*-
# =============================================================================
# $Id$
# =============================================================================
## @file
#  Module with decoration of some RooFit objects for efficient use in python
#
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2011-06-07
#
#  
#                    $Revision$
#  Last modification $Date$
#  by                $Author$
# =============================================================================
"""Decoration of some RooFit objects for efficient use in python"""
# =============================================================================
__version__ = "$Revision$"
__author__  = "Vanya BELYAEV Ivan.Belyaev@itep.ru"
__date__    = "2011-06-07"
__all__     = () 
# =============================================================================
import ROOT, cppyy              ## attention here!!
cpp = cppyy.makeNamespace('')
VE  = cpp.Gaudi.Math.ValueWithError 
# 
# =============================================================================
# logging 
# =============================================================================
from AnalysisPython.Logger import getLogger 
if '__main__' ==  __name__ : logger = getLogger( 'Ostap.RooFitDeco' )
else                       : logger = getLogger( __name__ )
# =============================================================================
logger.debug( 'Some useful decorations for RooFit objects')
# =============================================================================
## iterator for RooArgList 
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date   2011-06-07
def _ral_iter_ ( self ) :
    """
    Iterator for RooArgList:

    >>> arg_list = ...
    >>> for p in arg_list : print p
    
    """
    l = len ( self )
    for i in range ( 0 , l )  : yield self[i]

## some decoration over RooArgList 
ROOT.RooArgList . __len__       = lambda s   : s.getSize()
ROOT.RooArgList . __contains__  = lambda s,i :  0<= i < len(s)
ROOT.RooArgList . __iter__      = _ral_iter_
ROOT.RooArgList . __nonzero__   = lambda s   : 0 != len ( s ) 

## helper function 
def _rs_list_ ( self ) :
    """
    """
    _l = []
    for i in self :
        
        if   hasattr  ( i , 'GetName' ) and hasattr ( i , 'getVal' ) :
            _l.append ( i.GetName() + ":%s" % i.getVal() )
        elif hasattr  ( i , 'GetName' ) :
            _l.append ( i.GetName()   )
        elif hasattr  ( i , 'getVal'  ) :
            _l.append ( "%s" % i.getVal ()  )
        else :
            _l.append (  str ( i )    )
            
    return _l ;

ROOT.RooArgList . __str__       = lambda s : str ( _rs_list_ ( s ) )  
ROOT.RooArgList . __repr__      = lambda s : str ( _rs_list_ ( s ) )  

# =============================================================================
## iterator for RooArgSet
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date   2011-06-07
def _ras_iter_ ( self ) :
    """
    Simple iterator for RootArgSet:

    >>> arg_set = ...
    >>> for i in arg_set : print i
    
    """    
    it  = self.createIterator()
    val = it.Next()
    while val :
        yield val 
        val = it.Next()
        
    del it

# =============================================================================
## get the attibute for RooArgSet 
def _ras_getattr_ ( self , aname ) :
    """
    Get the attibute from RooArgSet

    >>> aset = ...
    >>> print aset.pt
    
    """
    _v = self.find ( aname )
    if not _v : raise  AttributeError
    return _v 

# =============================================================================
## get the attibute for RooArgSet 
def _ras_getitem_ ( self , aname ) :
    """
    Get the attibute from RooArgSet

    >>> aset = ...
    >>> print aset['pt']
    
    """
    _v = self.find ( aname )
    if not _v : raise  IndexError
    return _v 

# =============================================================================
## check the presence of variable in set 
def _ras_contains_ ( self , ename ) :
    """
    Check the presence of variable in set 
    """
    _v = self.find ( aname )
    if not _v : return False 
    return             True 

## some decoration over RooArgSet 
ROOT.RooArgSet . __len__       = lambda s   : s.getSize()
ROOT.RooArgSet . __iter__      = _ras_iter_ 
ROOT.RooArgSet . __getattr__   = _ras_getattr_ 
ROOT.RooArgSet . __getitem__   = _ras_getitem_ 
ROOT.RooArgSet . __contains__  = _ras_contains_ 
ROOT.RooArgSet . __nonzero__   = lambda s   : 0 != len ( s ) 
        
ROOT.RooArgSet . __str__   = lambda s : str ( tuple ( _rs_list_ ( s ) ) )  
ROOT.RooArgSet . __repr__  = lambda s : str ( tuple ( _rs_list_ ( s ) ) )  

# =============================================================================
## iterator for RooDataSet
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date   2011-06-07
def _rds_iter_ ( self ) :
    """
    Iterator for RooDataSet 
    """
    _l = len ( self )
    for i in xrange ( 0 , _l ) : yield self.get ( i )

# =============================================================================
## access to the entries in  RooDataSet
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date   2013-03-31
def _rds_getitem_ ( self , i ) :
    """
    Get the entry from RooDataSet 
    """
    if 0<= i < len ( self ) :
        return self.get ( i )
    raise IndexError 

## some decoration over RooDataSet 
ROOT.RooDataSet . __len__       = lambda s   : s.numEntries()
ROOT.RooDataSet . __iter__      = _rds_iter_ 
ROOT.RooDataSet . __getitem__   = _rds_getitem_ 
ROOT.RooDataSet . __nonzero__   = lambda s   : 0 != len ( s ) 
        
# =============================================================================
## ``easy'' print of RooFitResult
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date   2011-06-07
def _rfr_print_ ( self , opts = 'v' ) :
    """
    Easy print of RooFitResult

    >>> result = ...
    >>> print result
    
    """
    self.Print( opts )
    return 'RooFitResult'

# =============================================================================
## get parameters from RooFitResult
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date   2011-06-07
def _rfr_params_ ( self , float_only = True ) :
    """
    GetParameters from RooFitResult:

    >>> result = ...
    >>> params = results
    >>> p0     = params['A'][0]  ## get the value
    >>> p0s    = params['A'][1]  ## get the parameter itself 
    
    """
    pars  = self.floatParsFinal()
    pars_ = {}
    for p in pars :
        pars_ [ p.GetName() ] = p.as_VE(), p

    ## also fixed parameters? 
    if not float_only :
        fixed = self.constPars()
        for p in fixed :
            pars_ [ p.GetName() ] = p.as_VE(), p
            
    return pars_

# =============================================================================
## get parameter by name  from RooFitResult
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date   2011-06-07
def _rfr_param_  ( self , pname , float_only = False ) :
    """
    Get Parameter from RooFitResult by name 

    >>> result = ...
    >>> signal = results.param('Signal')
    >>> print signal
    """
    p = self.parameters ( float_only )[ pname ] 
    return p 

# =============================================================================
## get the correlation coefficient
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date   2011-06-07
def _rfr_corr_  ( self , name1 , name2 ) :
    """
    Get correlation coefficient for two parameter 

    >>> result = ...
    >>> corr = results.corr('Signal', 'Background')
    >>> print corr
    """
    p1 = self.parameters()[ name1 ]
    p2 = self.parameters()[ name2 ]
    #
    return self.correlation ( p1[1] , p2[1] ) 

# =============================================================================
## get the covariance (sub) matrix 
#  @author Vanya BELYAEV Ivan.Belyaev@cern.ch
#  @date   2011-06-07
def _rfr_cov_  ( self , name1 , name2 ) :
    """
    Get covariance (sub) matrix 

    >>> result = ...
    >>> cov = results.cov('Signal', 'Background')
    >>> print corr
    """
    p1   = self.parameters()[ name1 ]
    p2   = self.parameters()[ name2 ]
    args = ROOT.RooArgList ( p1[1] , p2[1] ) 
    return self.reducedCovarianceMatrix (  args ) 


# =============================================================================

## some decoration over RooFitResult
ROOT.RooFitResult . __repr__   = _rfr_print_
ROOT.RooFitResult . __str__    = _rfr_print_
ROOT.RooFitResult . __call__   = _rfr_param_
ROOT.RooFitResult . parameters = _rfr_params_
ROOT.RooFitResult . params     = _rfr_params_
ROOT.RooFitResult . param      = _rfr_param_
ROOT.RooFitResult . parameter  = _rfr_param_
ROOT.RooFitResult . corr       = _rfr_corr_
ROOT.RooFitResult . cor        = _rfr_corr_
ROOT.RooFitResult . cov        = _rfr_cov_
ROOT.RooFitResult . parValue   = lambda s,n : s.parameter(n)[0]

# =============================================================================
## fix parameter at some value
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2012-09-20
def _fix_par_ ( var , value  = None ) :
    """
    Fix parameter at some value :

    >>> par = ...
    >>> par.fix ( 10 ) 
    
    """
    #
    if None is value :
        var.setConstant( True )
        return var.ve()
    
    if hasattr ( value , 'value' ) : value = value.value()
    #
    var.setVal      ( value )
    var.setConstant ( True  )
    #
    return var.ve() 

# =============================================================================
## release the parameter
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2012-09-20
def _rel_par_ ( var )  :
    """
    Release the parameters

    >>> par = ...
    >>> par.release () 
    
    """
    var.setConstant ( False )
    #
    return var.ve()

# ==============================================================================
## Convert RooRealVar into ValueWithError 
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2014-06-23
def _rrv_ve_ ( var ) :
    """
    Convert RooRealVar into ValueWithError
    
    >>> par = ...
    >>> ve  = par.ve()
    
    """
    v  =      var.getVal()
    e2 = 0 if var.isConstant() else var.getError()**2
    #
    return VE ( v , e2 ) 
# =============================================================================
## decorate RooRealVar:
ROOT.RooRealVar   . as_VE     = _rrv_ve_ 
ROOT.RooRealVar   . ve        = _rrv_ve_
ROOT.RooRealVar   . fix       = _fix_par_
ROOT.RooRealVar   . Fix       = _fix_par_
ROOT.RooRealVar   . release   = _rel_par_
ROOT.RooRealVar   . Release   = _rel_par_
## convert to float 
ROOT.RooRealVar   . __float__ = lambda s : s.getVal()
## print it in more suitable form 
ROOT.RooRealVar   . __repr__  = lambda s : "'%s' : %s " % ( s.GetName() , s.ve() )

# ============================================================================
## make a histogram for RooRealVar
#  @see RooRealVar
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2013-07-14
def _rrv_as_H1_ ( v , bins = 100 , double = True ) :
    """
    Make TH1 histogram from RooRealVar

    >>> variable = ...
    >>> histo = variable.histo ( 100 )
    
    """
    from Ostap.PyRoUts import hID
    
    _hT = ROOT.TH1D if double else ROOT.TH1F
    _h  = _hT ( hID() , v.GetTitle() , bins , v.getMin()  , v.getMax() )
    _h.Sumw2()
    
    return _h 

ROOT.RooRealVar   . histo = _rrv_as_H1_
ROOT.RooRealVar   . asH1  = _rrv_as_H1_

_RRV_ = ROOT.RooRealVar

# ============================================================================
## Addition of RooRealVar and ``number''
def _rrv_add_ ( s , o ) :
    """
    Addition of RooRealVar and ``number''

    >>> var = ...
    >>> num = ...
    >>> res = var + num
    
    """
    if   isinstance ( o , _RRV_    ) and not o.isConstant() : o = o.ve     () 
    elif hasattr    ( o , 'getVal' )                        : o = o.getVal ()
    #
    v = s.getVal() if s.isConstant() else s.ve()
    #
    return v + o

## Subtraction  of RooRealVar and ``number''
def _rrv_sub_ ( s , o ) :
    """
    Subtraction of RooRealVar and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> res = var - num
    
    """
    if   isinstance ( o , _RRV_    ) and not o.isConstant() : o = o.ve     () 
    elif hasattr    ( o , 'getVal' )                        : o = o.getVal ()
    #
    v = s.getVal() if s.isConstant() else s.ve()
    #
    return v - o

## Multiplication of RooRealVar and ``number''
def _rrv_mul_ ( s , o ) :
    """
    Multiplication  of RooRealVar and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> res = var * num
    
    """
    if   isinstance ( o , _RRV_    ) and not o.isConstant() : o = o.ve     () 
    elif hasattr    ( o , 'getVal' )                        : o = o.getVal ()
    #
    v = s.getVal() if s.isConstant() else s.ve()
    #
    return v * o

## Division of RooRealVar and ``number''
def _rrv_div_ ( s , o ) :
    """
    Division of RooRealVar and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> res = var / num
    
    """
    if   isinstance ( o , _RRV_    ) and not o.isConstant() : o = o.ve     () 
    elif hasattr    ( o , 'getVal' )                        : o = o.getVal ()
    #
    v = s.getVal() if s.isConstant() else s.ve()
    #
    return v / o

## (right) Addition of RooRealVar and ``number''
def _rrv_radd_ ( s , o ) :
    """
    (Right) Addition of RooRealVar and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> res = num + var 
    
    """
    if   isinstance ( o , _RRV_    ) and not o.isConstant() : o = o.ve     () 
    elif hasattr    ( o , 'getVal' )                        : o = o.getVal ()
    #
    v = s.getVal() if s.isConstant() else s.ve()
    #
    return o + v 

## (right) subtraction  of RooRealVar and ``number''
def _rrv_rsub_ ( s , o ) :
    """
    (right) subtraction of RooRealVar and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> res = num - var 
    
    """
    if   isinstance ( o , _RRV_    ) and not o.isConstant() : o = o.ve     () 
    elif hasattr    ( o , 'getVal' )                        : o = o.getVal ()
    #
    v = s.getVal() if s.isConstant() else s.ve()
    #
    return o - v 

## (right) multiplication of RooRealVar and ``number''
def _rrv_rmul_ ( s , o ) :
    """
    (right) Multiplication  of RooRealVar and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> res = num * var 
    
    """
    if   isinstance ( o , _RRV_    ) and not o.isConstant() : o = o.ve     () 
    elif hasattr    ( o , 'getVal' )                        : o = o.getVal ()
    #
    v = s.getVal() if s.isConstant() else s.ve()
    #
    return o * v 

## (right) Division of RooRealVar and ``number''
def _rrv_rdiv_ ( s , o ) :
    """
    (right) Division of RooRealVar and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> res = num / var 
    
    """
    if   isinstance ( o , _RRV_    ) and not o.isConstant() : o = o.ve     () 
    elif hasattr    ( o , 'getVal' )                        : o = o.getVal ()
    #
    v = s.getVal() if s.isConstant() else s.ve()
    #
    return o / v 

## pow of RooRealVar and ``number''
def _rrv_pow_ ( s , o ) :
    """
    pow of RooRealVar and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> res = var ** num 
    
    """
    if   isinstance ( o , _RRV_    ) and not o.isConstant() : o = o.ve     () 
    elif hasattr    ( o , 'getVal' )                        : o = o.getVal ()
    #
    v = s.getVal() if s.isConstant() else s.ve()
    #
    return v**o  

## (right) pow of RooRealVar and ``number''
def _rrv_rpow_ ( s , o ) :
    """
    pow of RooRealVar and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> res = num ** var 
    
    """
    if   isinstance ( o , _RRV_    ) and not o.isConstant() : o = o.ve     () 
    elif hasattr    ( o , 'getVal' )                        : o = o.getVal ()
    #
    v = s.getVal() if s.isConstant() else s.ve()
    #
    return o**v   


ROOT.RooRealVar . __add__   = _rrv_add_
ROOT.RooRealVar . __sub__   = _rrv_sub_
ROOT.RooRealVar . __div__   = _rrv_div_
ROOT.RooRealVar . __mul__   = _rrv_mul_
ROOT.RooRealVar . __pow__   = _rrv_pow_

ROOT.RooRealVar . __radd__  = _rrv_radd_
ROOT.RooRealVar . __rsub__  = _rrv_rsub_
ROOT.RooRealVar . __rdiv__  = _rrv_rdiv_
ROOT.RooRealVar . __rmul__  = _rrv_rmul_
ROOT.RooRealVar . __rpow__  = _rrv_rpow_

# =============================================================================
## (compare RooRealVar and "number"
def _rrv_le_ ( s , o ) :
    """
    compare RooRealVal and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> iv var <= num : print ' ok! '
    """
    return o >= s.getVal()

## (compare RooRealVar and "number"
def _rrv_lt_ ( s , o ) :
    """
    compare RooRealVal and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> iv var < num : print ' ok! '
    """
    return o > s.getVal()

## (compare RooRealVar and "number"
def _rrv_ge_ ( s , o ) :
    """
    compare RooRealVal and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> iv var >= num : print ' ok! '
    """
    return o <= s.getVal()

## (compare RooRealVar and "number"
def _rrv_gt_ ( s , o ) :
    """
    compare RooRealVal and ``number''
    
    >>> var = ...
    >>> num = ...
    >>> iv var > num : print ' ok! '
    """
    return o < s.getVal()

ROOT.RooRealVar . __lt__   = _rrv_lt_
ROOT.RooRealVar . __gt__   = _rrv_gt_
ROOT.RooRealVar . __le__   = _rrv_le_
ROOT.RooRealVar . __ge__   = _rrv_ge_

# =============================================================================
## get min/max in one go 
#  @see RooRealVar
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2013-07-14
def _rrv_minmax_ ( s ) :
    """
    Get min/max in one go

    >>> var = ...
    >>> mn,mx = var.minmax()
    """
    return s.getMin(),s.getMax()

ROOT.RooRealVar   . minmax  = _rrv_minmax_

## # ==========================================
## _is_equal_ = cpp.LHCb.Math.equal_to_double 
## ## (compare RooRealVar and "number"
## def _rrv_eq_ ( s , o ) :
##     """
##     compare RooRealVal and ``number''
    
##     >>> var = ...
##     >>> num = ...
##     >>> iv var == num : print ' ok! '
##     """
##     return    _is_equal_ ( o , s.getVal() ) 

## ## (compare RooRealVar and "number"
## def _rrv_ne_ ( s , o ) :
##     """
##     compare RooRealVal and ``number''
    
##     >>> var = ...
##     >>> num = ...
##     >>> iv var != num : print ' ok! '
##     """
##     return not _is_equal_ ( o , s.getVal() ) 


## ROOT.RooRealVar . __eq__   = _rrv_eq_
## ROOT.RooRealVar . __ne__   = _rrv_ne_

## product of two PDFs 
def _pdf_mul_ ( pdf1 , pdf2 ) :
    """
    Easy contruct for the product of two PDFs:
    
    >>> pdf1 = ...
    >>> pdf2 = ...
    
    >>> product = pdf1 * pdf2 
    """
    return cpp.Analysis.Models.Product ( '%s*%s'             % ( pdf1.GetName  () ,
                                                                 pdf2.GetName  () ) ,
                                         'Product: %s & %s ' % ( pdf1.GetTitle () ,
                                                                 pdf2.GetTitle () ) ,
                                         pdf1 , pdf2 )
ROOT.RooAbsPdf . __mul__  = _pdf_mul_ 

# =============================================================================
## Helper project method for RooDataSet
#
#  @code 
#    
#    >>> h1   = ROOT.TH1D(... )
#    >>> dataset.project ( h1.GetName() , 'm', 'chi2<10' ) ## project variable into histo
#    
#    >>> h1   = ROOT.TH1D(... )
#    >>> dataset.project ( h1           , 'm', 'chi2<10' ) ## use histo
#
#  @endcode
#
#  @see RooDataSet 
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2013-07-06
def _ds_project_  ( dataset , histo , what , *args ) :
    """
    Helper project method for RooDataSet
    
    >>> h1   = ROOT.TH1D(... )
    >>> dataset.project ( h1.GetName() , 'm', 'chi2<10' ) ## project variable into histo
    
    >>> h1   = ROOT.TH1D(... )
    >>> dataset.project ( h1           , 'm', 'chi2<10' ) ## use histo

    """
    
    if isinstance ( what , ROOT.RooArgList ) and isinstance ( histo , ROOT.TH1 ) :
        return dataset.fillHistogram  ( histo , what , *args ) 
    
    store = dataset.store()
    if store and isinstance ( what , str ) : 
        tree = store.tree()
        if tree : return tree.project ( histo , what , *args ) 
        
    if isinstance ( what , str ) : 
        vars  = [ v.replace(' ','') for v in what.split(':') ]
        return _ds_project_ ( dataset , histo , vars , *args ) 
    
    if isinstance ( what , ( list , tuple ) ) : 
        vars_ = dataset.get()
        vars  = ROOT.RooArgList()
        for w in what : vars.add ( vars_[w] ) 
        return _ds_project_ ( dataset , histo , vars , *args ) 
    
    if isinstance ( what , ROOT.RooRealVar ) :
        
        lst = ROOT.RooArgList()
        lst.add ( what )
        for a in args :
            if isinstance ( a , ROOT.RooRealVar ) : lst.add( a )
            else : break            
        return _ds_project_ ( dataset , histo , lst , *args[ len( lst ) - 1 : ] ) 
        
    if isinstance ( histo , str ) :
    
        obj = ROOT.gROOT     .FindObject    ( histo )
        if instance ( obj  , ROOT.TH1 ) :
            return _ds_project_ ( dataset , obj , what , *args )
        obj = ROOT.gROOT     .FindObjectAny ( histo )
        if instance ( obj  , ROOT.TH1 ) :
            return _ds_project_ ( dataset , obj , what , *args )
        obj = ROOT.gDirectory.FindObject    ( histo )
        if instance ( obj  , ROOT.TH1 ) :
            return _ds_project_ ( dataset , obj , what , *args )
        obj = ROOT.gDirectory.FindObjectAny ( histo )
        if instance ( obj  , ROOT.TH1 ) :
            return _ds_project_ ( dataset , obj , what , *args )
        
    if not histo and isinstance ( what , ROOT.RooArgList ) and 1 <= len ( what ) <= 3 :

        from Ostap.PyRoUts import hID
        
        if 1 == len ( what ) :
            histo = ROOT.TH1F ( hID() ,
                                what[0].GetName() , 100 ,
                                what[0].getMin () ,  what[0].getMax () ) ; histo.Sumw2()
            return _ds_project_ ( dataset , histo , what , *args )
        
        if 2 == len ( what ) :
            histo = ROOT.TH2F ( hID() ,
                                what[1].GetName() + ' : ' + what[0].GetName() ,
                                20 , what[0].getMin () ,     what[0].getMax () ,
                                20 , what[1].getMin () ,     what[1].getMax () ) ; histo.Sumw2()
            return _ds_project_ ( dataset , histo , what , *args )
        
        if 3 == len ( what ) :
            histo = ROOT.TH2F ( hID() ,
                                what[2].GetName() + ' : ' + what[1].GetName() + ' : ' + what[0].GetName() ,
                                20 , what[0].getMin () ,     what[0].getMax () ,
                                20 , what[1].getMin () ,     what[1].getMax () , 
                                20 , what[2].getMin () ,     what[2].getMax () ) ; histo.Sumw2()
            return _ds_project_ ( dataset , histo , what , *args )
        
    raise AttributeError ( 'DataSet::project, invalid case' )

# =============================================================================
## Helper draw method for RooDataSet
#
#  @code 
#    
#    >>> dataset.draw ( 'm', 'chi2<10' ) ## use histo
#
#  @endcode
#
#  @see RooDataSet 
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2013-07-06
def _ds_draw_ ( dataset , what , *args ) :
    """
    Helper draw method for RooDataSet
    
    >>> dataset.draw ( 'm', 'chi2<10' ) ## use histo
    
    """
    store = dataset.store()
    if store and isinstance ( what , str ) : 
        tree = store.tree()
        if tree : return tree.Draw( what , *args )

    h = _ds_project_ ( dataset , None , what , *args )
    if isinstance ( h , ROOT.TH1 ) :
        ##                                    OPTIONS                              CUTS 
        if  len ( args ) > 1 and isinstance ( args[-1] , str ) and isinstance ( args[-2] , str ) :  h.Draw ( args[-1] )
        else                 : h.Draw ( ) 
        return h
    
    raise AttributeError ( 'DataSet::draw, invalid case' )

# =============================================================================
## get the attibute for RooDataSet
def _ds_getattr_ ( dataset , aname ) :
    """
    Get the attibute from RooDataSet 

    >>> dset = ...
    >>> print dset.pt
    
    """
    _vars = dataset.get()
    return getattr ( _vars , aname )  

# =============================================================================
## get the statistic for certain expression in Tree/Dataset
#  @code
#  dataset  = ... 
#  stat1 = dataset.statVar( 'S_sw/effic' )
#  stat2 = dataset.statVar( 'S_sw/effic' ,'pt>1000')
#  @endcode
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2013-09-15
def _ds_stat_var_ ( dataset , what , *cuts ) :
    """
    Get the statistic for certain expression in Tree/Dataset
    
    >>> dataset  = ... 
    >>> stat1 = dataset.statVar( 'S_sw/effic' )
    >>> stat2 = dataset.statVar( 'S_sw/effic' ,'pt>1000')
    
    """
    store = dataset.store()
    if store :
        tree = store.tree()
        if tree : return tree.statVar( what , *cuts )
        
    raise AttributeError( "Can't ``statVar'' data set , probably wrong StorageType" )


ROOT.RooDataSet . statVar = _ds_stat_var_

# =============================================================================
## print method for RooDatSet
#  @code
#
#   >>> print dataset
#
#  @endcode 
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2013-07-06
def _ds_print_ ( dataset , opts = 'v' ) :
    """
    Helper print method:
    
    >>> print dataset 
    """
    #
    dataset.Print( opts )
    #
    return dataset.GetName() 

ROOT.RooDataSet.draw        = _ds_draw_
ROOT.RooDataSet.project     = _ds_project_
ROOT.RooDataSet.__repr__    = _ds_print_
ROOT.RooDataSet.__getattr__ = _ds_getattr_

ROOT.RooDataHist.__repr__   = _ds_print_
ROOT.RooDataHist.__len__    = lambda s : s.numEntries() 


# =============================================================================
## make weighted data set form unweighted dataset
#  @code
#  >>> dataset = ...
#  >>> wdata   = dataset.makeWeighted ( 'S_sw' ) 
#  @endcode
#  @param wvarname name of weighting variable
#  @param varset   variables to be used in new dataset
#  @param cuts     optional cuts to be applied 
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date   2013-07-06
def _rds_makeWeighted_ ( dataset , wvarname , varset = None , cuts = '' ) :
    """
    make weighted data set form unweighted dataset
    
    >>> dataset = ...
    >>> wdata   = dataset.makeWeighted ( 'S_sw' )
    
    """
    if dataset.isWeighted () : 
        logger.warning ("Dataset '%s/%s' is already weighted!" % ( dataset.GetName  () , dataset.GetTitle () ) ) 
        
    #
    from Ostap.PyRoUts import dsID
    ## make weighted dataset 
    return ROOT.RooDataSet ( dsID()                   ,
                             dataset.GetTitle()       ,
                             dataset                  ,
                             varset or dataset.get()  , 
                             cuts                     ,
                             wvarname                 )

ROOT.RooDataSet.makeWeighted = _rds_makeWeighted_

# =============================================================================
if '__main__' == __name__ :
    
    import ostapline
    logger.info ( __file__  + '\n' + ostapline.line  ) 
    logger.info ( 80*'*'   )
    logger.info ( __doc__  )
    logger.info ( 80*'*' )
    logger.info ( ' Author  : %s' %         __author__    ) 
    logger.info ( ' Version : %s' %         __version__   ) 
    logger.info ( ' Date    : %s' %         __date__      )
    logger.info ( ' Symbols : %s' %  list ( __all__     ) )
    logger.info ( 80*'*' ) 
    
# =============================================================================
# The END 
# =============================================================================
