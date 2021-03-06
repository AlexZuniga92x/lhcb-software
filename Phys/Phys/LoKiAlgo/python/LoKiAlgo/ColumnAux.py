#!/usr/bin/env python
# =============================================================================
# $Id:$
# =============================================================================
## @file
#  helper module for decoration of standard N-tuples
#
#  This file is a part of LoKi project - 
#    "C++ ToolKit  for Smart and Friendly Physics Analysis"
#
#  The package has been designed with the kind help from
#  Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas, 
#  contributions and advices from G.Raven, J.van Tilburg, 
#  A.Golutvin, P.Koppenburg have been used in the design.
#
#  By usage of this code one clearly states the disagreement 
#  with the smear campaign of Dr.O.Callot et al.: 
#  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
#
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#  @date 2012-02-16
#
#                   $Revision$
# Last modification $Date$
#                by $Author$
# =============================================================================
"""
Helper module for decoration of standard N-tuples

This file is a part of LoKi project - 
``C++ ToolKit  for Smart and Friendly Physics Analysis''

The package has been designed with the kind help from
Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas, 
contributions and advices from G.Raven, J.van Tilburg, 
A.Golutvin, P.Koppenburg have been used in the design.

By usage of this code one clearly states the disagreement 
with the smear campaign of Dr.O.Callot et al.: 
``No Vanya's lines are allowed in LHCb/Gaudi software.''

"""
# =============================================================================
__author__  = "Vanya BELYAEV Ivan.Belyaev@itep.ru "
__date__    = "2012-02-16"
__version__ = "$Revision$"
# =============================================================================
from   LoKiCore.basic        import cpp, LoKi
import GaudiPython.GaudiAlgs 

_Tuple    = cpp.Tuples.Tuple
_ColAux   = LoKi.Dicts.TupleAux 
_ColBasic = LoKi.Dicts.TupleBasic 

## simple function for decoration of "Tuples::Tuple::farray" method 
def _colAux_ ( self , *args ) :
    """
    Simple function for decoration of 'Tuples::Tuple::column_aux' method
    
    time = ...                    ## Gaudi::Time object
    tup.column_aux ( 't' , time ) ## name/prefix is optional
    tup.column_aux ( time )

    odin = ...                    ## LHCb::Odin object
    tup.column_aux ( 'o' , odin ) ## name/prefix is optional
    tup.column_aux ( odin )
    
    rhdr = ...                     ## LHCb::RecHeader object  
    tup.column_aux ( 'r' , rhdr )  ## name/prefix is optional
    tup.column_aux ( rhdr )

    rsum = ...                     ## LHCb::RecSummary object  
    tup.column_aux ( 's' , rsum )  ## name/prefix is optional
    tup.column_aux ( rsum )

    rnum = ...                     ## Gaudi::Numbers object  
    tup.column_aux ( 'n' , rnum )  ## name/prefix is optional
    tup.column_aux ( rnum )
    
    """
    return _ColAux.column ( self ,*args )

_colAux_ . __doc__ += '\n' + _ColAux . column . __doc__

# =============================================================================
## finally decorate tuple 
_Tuple.column_aux    = _colAux_

# =============================================================================
## few basic types 
_ColBasic = LoKi.Dicts.TupleBasic 

## specialization for bool values 
def _column_bool_ ( self , name , value ) :
    """
    Add boolean value to n-tuple
    """
    return _ColBasic.column_bool ( self , name , value )
# =============================================================================
## specialization for float  values 
def _column_float_ ( self , name , value ) :
    """
    Add float value to n-tuple    
    """
    return _ColBasic.column_float ( self , name , value )
# =============================================================================
## specialization for double  values 
def _column_double_ ( self , name , value ) :
    """
    Add float value to n-tuple
    
    """
    return _ColBasic.column_double ( self , name , value )
# =============================================================================
## specialization for char values 
def _column_char_ ( self , name , value ) :
    """
    Add char value to n-tuple
    """
    return _ColBasic.column_char ( self , name , value )
# =============================================================================
## specialization for unsigned char  values 
def _column_uchar_ ( self , name , value ) :
    """
    Add unsigned char value to n-tuple
    """
    return _ColBasic.column_uchar ( self , name , value )
# =============================================================================
## specialization for short values 
def _column_short_ ( self , name , value ) :
    """
    Add short value to n-tuple
    """
    return _ColBasic.column_short ( self , name , value )
# =============================================================================
## specialization for unsigned short  values 
def _column_ushort_ ( self , name , value ) :
    """
    Add unsigned short value to n-tuple
    """
    return _ColBasic.column_ushort ( self , name , value )
# =============================================================================
## specialization for int  values 
def _column_int_ ( self , name , value ) :
    """
    Add int value to n-tuple
    """
    return _ColBasic.column_int ( self , name , value )
# =============================================================================
## specialization for unsigned int  values 
def _column_uint_ ( self , name , value ) :
    """
    Add unsigned int value to n-tuple
    """
    return _ColBasic.column_uint ( self , name , value )
# =============================================================================
## specialization for long  values 
def _column_long_ ( self , name , value ) :
    """
    Add long value to n-tuple
    """
    return _ColBasic.column_long ( self , name , value )
# =============================================================================
## specialization for unsigned long  values 
def _column_ulong_ ( self , name , value ) :
    """
    Add unsigned long value to n-tuple
    """
    return _ColBasic.column_ulong ( self , name , value )

_column_bool_   . __doc__ += '\n' + _ColBasic . column_bool   . __doc__
_column_float_  . __doc__ += '\n' + _ColBasic . column_float  . __doc__
_column_double_ . __doc__ += '\n' + _ColBasic . column_double . __doc__
_column_char_   . __doc__ += '\n' + _ColBasic . column_char   . __doc__
_column_uchar_  . __doc__ += '\n' + _ColBasic . column_uchar  . __doc__
_column_short_  . __doc__ += '\n' + _ColBasic . column_short  . __doc__
_column_ushort_ . __doc__ += '\n' + _ColBasic . column_ushort . __doc__
_column_int_    . __doc__ += '\n' + _ColBasic . column_int    . __doc__
_column_uint_   . __doc__ += '\n' + _ColBasic . column_uint   . __doc__
_column_long_   . __doc__ += '\n' + _ColBasic . column_long   . __doc__
_column_ulong_  . __doc__ += '\n' + _ColBasic . column_ulong  . __doc__

# =============================================================================
## finally decorate tuple 
_Tuple.column_bool       =       _column_bool_ 
_Tuple.column_float      =       _column_float_ 
_Tuple.column_double     =       _column_double_ 
_Tuple.column_char       =       _column_char_ 
_Tuple.column_uchar      =       _column_uchar_ 
_Tuple.column_short      =       _column_short_ 
_Tuple.column_ushort     =       _column_ushort_ 
_Tuple.column_int        =       _column_int_ 
_Tuple.column_uint       =       _column_uint_ 
_Tuple.column_long       =       _column_long_ 
_Tuple.column_ulong      =       _column_ulong_ 
_Tuple.column_longlong   = _Tuple.column_ll 
_Tuple.column_ulonglong  = _Tuple.column_ull 

# =============================================================================
if '__main__' == __name__ :
    
    print 80*'*'
    print __doc__
    print ' Author  : ' , __author__
    print ' Version : ' , __version__
    print ' Date    : ' , __date__    
    print 80*'*'
    lst = dir(_Tuple)
    for i in lst :
        if   0<= i.lower().find('colu') : pass
        elif 0<= i.lower().find('arra') : pass
        elif 0<= i.lower().find('matr') : pass
        else : continue
        m = getattr ( _Tuple , i )
        print m.__doc__
        
    print 80*'*'
 
# =============================================================================
# The END 
# =============================================================================
