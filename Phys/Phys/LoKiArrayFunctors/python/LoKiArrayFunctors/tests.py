#!/usr/bin/env python
# =============================================================================
# $Id$ 
# =============================================================================
## @file  LoKiArrayFunctors/decorators.py
#  The set of basic decorator for objects from LoKiHlt library
#  The file is a part of LoKi and Bender projects
#
#  This file is a part of LoKi project - 
#   'C++ ToolKit  for Smart and Friendly Physics Analysis'
#
#  The package has been designed with the kind help from
#  Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas, 
#  contributions and advices from G.Raven, J.van Tilburg, 
#  A.Golutvin, P.Koppenburg have been used in the design.
#
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @daet 2007-06-09
# =============================================================================
"""
The set of basic decorators for objects from LoKiHlt library

This file is a part of LoKi project - 
'C++ ToolKit  for Smart and Friendly Physics Analysis'

The package has been designed with the kind help from
Galina PAKHLOVA and Sergey BARSUK.  Many bright ideas, 
contributions and advices from G.Raven, J.van Tilburg, 
A.Golutvin, P.Koppenburg have been used in the design.
"""
# =============================================================================
__author__ = "Vanya BELYAEV ibelyaev@physics.syr.edu" 
# =============================================================================
import LoKiArrayFunctors.Array 
from LoKiArrayFunctors.decorators import *
from LoKiCore.functions           import *
from LoKiCore.math                import *
from LoKiPhys.decorators          import P , PT, Q 
# =============================================================================
def testAll() :


    print ( ALV01 < 0 )  & ( AETA > 0 ) | (  AM < 50 ) & AALL

    print aval_max ( P , PT ) , aval_min  ( P, PT , 0 != Q )
    print aval_min  ( P, PT , 0 != Q , 10000 )
    print ASUM ( P , PT > 1 ) 
    pass 
    
# =============================================================================
if '__main__' == __name__ :
    testAll() 

        
# =============================================================================
# The END 
# =============================================================================
