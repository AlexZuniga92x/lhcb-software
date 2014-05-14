#!/usr/bin/env python
# -*- coding: utf-8 -*-
# =============================================================================
# $Id:$ 
# =============================================================================
## @file Test2DModels.py
#
#  tests for various 2D-smooth non-factorizable 
#
#  @author Vanya BELYAEV Ivan.Belyaeve@itep.ru
#  @date 2014-05-11
# 
#                    $Revision: 172094 $
#  Last modification $Date: 2014-04-29 16:02:06 +0200 (Tue, 29 Apr 2014) $
#                 by $Author: albarano $
# =============================================================================
"""
Tests for various 2D-smooth non-factorizable 
"""
# =============================================================================
__version__ = "$Revision:"
__author__  = "Vanya BELYAEV Ivan.Belyaev@itep.ru"
__date__    = "2014-05-10"
__all__     = ()  ## nothing to be imported 
# =============================================================================
import ROOT, random 
from   PyPAW.PyRoUts import *
from   PyPAW.Utils   import rooSilent 
# =============================================================================
# logging 
# =============================================================================
from AnalysisPython.Logger import getLogger
if '__main__' == __name__ : 
    logger = getLogger( 'PyPAW.Test2DModels' )
else : 
    logger = getLogger( __name__ )
# =============================================================================
logger.info ( 'Test for 2D nonfactorizable fit models from Analysis/PyPAW')
# =============================================================================
## make simple test mass 
m_x     = ROOT.RooRealVar ( 'mass_x' , 'Some test mass(X)' , 3 , 3.2 )
m_y     = ROOT.RooRealVar ( 'mass_y' , 'Some test mass(Y)' , 3 , 3.2 )

## book very simple data set
varset  = ROOT.RooArgSet  ( m_x , m_y )
dataset = ROOT.RooDataSet ( dsID() , 'Test Data set-1' , varset )  

## fill it with uniform (x,y) 
m = VE(3.100,0.015**2)
for i in xrange(0,5000) :

    m_x.setVal ( random.uniform ( m_x.getMin() , m_x.getMax() ) ) 
    m_y.setVal ( random.uniform ( m_y.getMin() , m_y.getMax() ) ) 
    dataset.add ( varset  )

print dataset

import PyPAW.Fit2DModels as Models 

# =============================================================================
## Positive polynomial in X and Y 
# =============================================================================
logger.info ('Test PolyPos2D_pdf' )
poly2D = Models.PolyPos2D_pdf ( 'P2D',
                                m_x  ,
                                m_y  ,
                                nx = 2 , ny = 2 )

with rooSilent() : 
    result = poly2D.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    
poly2D.pdf.setPars() 
pos  = poly2D.pdf.function()
nsph = pos.sphere()  
b2d  = pos.bernstein()

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :
    print 'Bernstein Coefficients:' , b2d.pars()
    print poly2D.pars() 

b2d_1 = b2d 
# =============================================================================
## Positive *SYMMETRIC* polynomial in X and Y 
# =============================================================================
logger.info ('Test PolyPos2Dsym_pdf' )
poly2Dsym = Models.PolyPos2Dsym_pdf ( 'P2D ',
                                      m_x   ,
                                      m_y   ,
                                      n = 2 )

with rooSilent() : 
    result = poly2Dsym.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2Dsym.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2Dsym.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    
poly2Dsym.pdf.setPars() 
pos  = poly2Dsym.pdf.function()
nsph = pos.sphere()  
b2d  = pos.bernstein()

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :
    print 'Bernstein Coefficients:' , b2d.pars()
    print poly2Dsym.pars() 

b2d_2 = b2d 

# =============================================================================
## product of phase space factors, modulated by positive polynomial in X and Y 
# =============================================================================
logger.info ('Test PSPol2D_pdf' )
## "fictive phase space"
psx   = cpp.Gaudi.Math.PhaseSpaceNL ( 0 , 10 , 2 , 10 )
psy   = cpp.Gaudi.Math.PhaseSpaceNL ( 0 , 10 , 2 , 10 )
poly2D_ps = Models.PSPol2D_pdf ( 'P2D',
                                 m_x    , m_y  ,
                                 psx    , psy  , 
                                 nx = 2 , ny = 2 )

with rooSilent() : 
    result = poly2D_ps.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D_ps.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D_ps.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    
poly2D_ps.pdf.setPars() 
pos  = poly2D_ps.pdf.function().positive() 
nsph = pos.sphere()  
b2d  = pos.bernstein()

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :
    print 'Bernstein Coefficients:' , b2d.pars() 
    print poly2D_ps.pars() 


# =============================================================================
## *SYMMETRIC* product of phase space factors, modulated by positive polynomial in X and Y 
# =============================================================================
logger.info ('Test PSPol2Dsym_pdf' )
## "fictive phase space"
ps    = cpp.Gaudi.Math.PhaseSpaceNL ( 0 , 10 , 2 , 10 )
poly2D_pss = Models.PSPol2Dsym_pdf ( 'P2Ds',
                                     m_x   , m_y  ,
                                     ps    , n = 2 )

with rooSilent() : 
    result = poly2D_pss.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D_pss.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D_pss.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    
poly2D_pss.pdf.setPars() 
pos  = poly2D_pss.pdf.function().positive() 
nsph = pos.sphere()  
b2d  = pos.bernstein()

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :
    print 'Bernstein Coefficients:' , b2d.pars()
    print poly2D_pss.pars() 



# =============================================================================
## exponential times phase space factor, modulated by positive polynomial in X and Y 
# =============================================================================
logger.info ('Test ExpoPSPol2D_pdf' )
## "fictive phase space"
psy   = cpp.Gaudi.Math.PhaseSpaceNL ( 0 , 10 , 2 , 10 )
poly2D_eps = Models.ExpoPSPol2D_pdf ( 'EPS',
                                      m_x    , m_y  ,
                                      psy    , 
                                      nx = 2 , ny = 2 )

poly2D_eps.tau.fix(0.1 )
with rooSilent() : 
    result = poly2D_eps.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D_eps.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D_eps.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    
poly2D_eps.pdf.setPars() 
pos  = poly2D_eps.pdf.function().positive() 
nsph = pos.sphere()  
b2d  = pos.bernstein()

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :
    print 'Bernstein Coefficients:' , b2d.pars() , result('tau_EPS', False)[0]
    print poly2D_eps.pars() 

# =============================================================================
## exponential times exponential modulated by positive polynomial in X and Y 
# =============================================================================
logger.info ('Test ExpoPol2D_pdf' )
## "fictive phase space"
poly2D_ee = Models.ExpoPol2D_pdf ( 'EP',
                                   m_x    , m_y  ,
                                   nx = 2 , ny = 2 )

poly2D_ee.taux.fix(0.1 )
poly2D_ee.tauy.fix(0.1 )
with rooSilent() : 
    result = poly2D_ee.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D_ee.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D_ee.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    
poly2D_ee.pdf.setPars() 
pos  = poly2D_ee.pdf.function().positive() 
nsph = pos.sphere()  
b2d  = pos.bernstein()

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :
    print 'Bernstein Coefficients:' , b2d.pars() , result('taux_EP', False)[0]
    print poly2D_ee.pars() 


# =============================================================================
## symmetric exponential times exponential modulated by positive polynomial in X and Y 
# =============================================================================
logger.info ('Test ExpoPol2Dsym_pdf' )
## "fictive phase space"
poly2D_ees = Models.ExpoPol2Dsym_pdf ( 'EPs',
                                      m_x  , m_y  ,
                                      n = 2 )

poly2D_ees.tau.fix(0.1 )
with rooSilent() : 
    result = poly2D_ees.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    result = poly2D_ees.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    poly2D_ees.tau.release() 
    result = poly2D_ees.pdf.fitTo ( dataset , ROOT.RooFit.Save() )
    
## poly2D_ees.pdf.setPars() 
## pos  = poly2D_ees.pdf.function().positive() 
## nsph = pos.sphere()  
## b2d  = pos.bernstein()

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :
    print 'Bernstein Coefficients:' , result('tau_EPs', False)[0]
    print poly2D_ees.pars() 


# =============================================================================
# The END 
# =============================================================================
