#!/usr/bin/env python
# -*- coding: utf-8 -*-
# =============================================================================
# $Id$ 
# =============================================================================
## @file TestCorr2d.py
#
#  tests for decorrelation utilities 
#
#  @author Vanya BELYAEV Ivan.Belyaeve@itep.ru
#  @date 2014-06-06
# 
#                    $Revision$
#  Last modification $Date$
#                 by $Author$
# =============================================================================
"""
Tests for various 2D-fit modelsa
"""
# =============================================================================
__version__ = "$Revision:"
__author__  = "Vanya BELYAEV Ivan.Belyaev@itep.ru"
__date__    = "2014-06-08"
__all__     = ()  ## nothing to be imported 
# =============================================================================
import ROOT, random 
from   PyPAW.PyRoUts import *
from   PyPAW.Utils   import rooSilent 
# =============================================================================
# logging 
# =============================================================================
from AnalysisPython.Logger import getLogger
if '__main__' == __name__ or '__builtin__' == __name__ : 
    logger = getLogger( 'PyPAW.TestCorr2D' )
else : 
    logger = getLogger( __name__ )
# =============================================================================
logger.info ( '')
# =============================================================================
## make simple test vars 
m_x     = ROOT.RooRealVar ( 'x' , 'Some test-(X)' , -100 , 100 )
m_y     = ROOT.RooRealVar ( 'y' , 'Some test-(Y)' , -100 , 100 )
# 
m1      = VE ( 0 , 5 )
m2      = VE ( 0 , 5 )


RAD = ROOT.RooAbsData
if RAD.Tree != RAD.getDefaultStorageType() :
    logger.info ('DEFINE default storage type to be TTree! ')
    RAD.setDefaultStorageType ( RAD.Tree )

## book very simple data set
varset  = ROOT.RooArgSet  ( m_x , m_y )
dataset = ROOT.RooDataSet ( dsID() , 'Test Data set-1' , varset )  

## fill it : 5000 events  Gauss * Gauss 
for i in xrange(0,50000 ) :

    v1 =        m1.gauss()
    v2 = 3*v1 + m2.gauss()
    
    m_x.setVal  ( v1 )
    m_y.setVal  ( v2 )
    dataset.add ( varset  )

print dataset 

from PyPAW.Corr2D import Corr2D

logger.info( 'Elimininate the leading correlations')
o2 = Corr2D ( dataset , 'x' , 'y' )


logger.info( 'Check the remaining correlations')
o3 = Corr2D ( dataset , o2.nvar1 , o2.nvar2 ) 

logger.info( 'Check the remaining correlations once more ')
o4 = Corr2D ( dataset , o3.nvar1 , o3.nvar2 ) 

## dataset.draw( o2.qvar1 )
## dataset.draw( o2.qvar1 + ' : ' + o2.qvar2 , '' , 'box' )

# =============================================================================
# The END 
# =============================================================================
