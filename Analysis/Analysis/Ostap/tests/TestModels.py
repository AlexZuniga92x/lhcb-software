#!/usr/bin/env python
# -*- coding: utf-8 -*-
# =============================================================================
# $Id: TestModels.py 174117 2014-06-21 13:24:21Z ibelyaev $ 
# =============================================================================
## @file TestModels.py
#
#  tests for various signal models 
#
#  @author Vanya BELYAEV Ivan.Belyaeve@itep.ru
#  @date 2014-05-11
# 
#                    $Revision: 174117 $
#  Last modification $Date: 2014-06-21 15:24:21 +0200 (Sat, 21 Jun 2014) $
#                 by $Author: ibelyaev $
# =============================================================================
"""
Tests for various background fit models  
"""
# =============================================================================
__version__ = "$Revision:"
__author__  = "Vanya BELYAEV Ivan.Belyaev@itep.ru"
__date__    = "2014-05-10"
__all__     = ()  ## nothing to be imported 
# =============================================================================
import ROOT, random
from   Ostap.PyRoUts import *
from   Ostap.Utils   import rooSilent 
# =============================================================================
# logging 
# =============================================================================
from AnalysisPython.Logger import getLogger
if '__main__' == __name__  or '__builtin__' == __name__ : 
    logger = getLogger ( 'Ostap.TestModels' )
else : 
    logger = getLogger ( __name__ )
# =============================================================================
logger.info ( 'Test for signal it models from Analysis/Ostap')
# =============================================================================
## make simple test mass 
mass     = ROOT.RooRealVar ( 'test_mass' , 'Some test mass' , 3.0 , 3.2 )

## book very simple data set
varset0  = ROOT.RooArgSet  ( mass )
dataset0 = ROOT.RooDataSet ( dsID() , 'Test Data set-0' , varset0 )  

mmin = mass.getMin()
mmax = mass.getMin()

## fill it 
m = VE(3.100,0.015**2)
for i in xrange(0,5000) :
    mass.setVal  ( m.gauss () )
    dataset0.add ( varset0    )

for i in xrange(0,500) :
    mass.setVal  ( random.uniform ( mass.getMin() , mass.getMax() ) ) 
    dataset0.add ( varset0   )


print dataset0

import Ostap.FitModels as     Models 

# =============================================================================
## gauss PDF
# =============================================================================
logger.info ('Test Gauss_pdf' ) 
signal_gauss = Models.Gauss_pdf ( name = 'Gauss' ,
                                  mass = mass    )

signal_gauss . mean  . setVal ( m.value () )
signal_gauss . sigma . setVal ( m.error () )



# =============================================================================
## Gauss PDF
# =============================================================================
model_gauss = Models.Fit1D( signal     = signal_gauss ,
                            background = Models.Bkg_pdf ('BkgGauss', mass = mass , power = 0 ) )
model_gauss.background.tau.fix(0)

with rooSilent() : 
    result, frame = model_gauss . fitTo ( dataset0 )
    result, frame = model_gauss . fitTo ( dataset0 )
    
    
if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual()  ) )
    print result 
else :     
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean   & Sigma      are: ', result ( 'mean_Gauss')[0] , result( 'sigma_Gauss' )[0]



# =============================================================================
## CrystalBall PDF
# =============================================================================
logger.info ('Test CrystalBall_pdf' )
model_cb = Models.Fit1D (
    signal = Models.CrystalBall_pdf ( name  = 'CB'  ,
                                      mass  = mass  ,
                                      alpha = 3     , 
                                      sigma = signal_gauss.sigma ,  
                                      mean  = signal_gauss.mean  ) ,
    background = model_gauss.background  )

with rooSilent() : 
    result, frame = model_cb. fitTo ( dataset0 )
    model_cb.signal.alpha.release()
    result, frame = model_cb. fitTo ( dataset0 )
    result, frame = model_cb. fitTo ( dataset0 )

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else : 
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean   & Sigma      are: ', result ( 'mean_Gauss')[0] , result( 'sigma_Gauss' )[0]
    print 'Alpha  & n          are: ', result ( model_cb.signal.alpha.GetName() ) [ 0 ] , result ( model_cb.signal.    n.GetName() ) [ 0 ] 

# =============================================================================
## right side CrystalBall PDF
# =============================================================================
logger.info ('Test CrystalBallRS_pdf' )
model_cbrs = Models.Fit1D (
    signal = Models.CrystalBallRS_pdf ( name  = 'CBRS' , 
                                        mass  = mass               ,
                                        sigma = signal_gauss.sigma ,  
                                        mean  = signal_gauss.mean  ) ,
    background = model_gauss.background  )

model_cbrs.signal.alpha.fix(2) 
model_cbrs.signal.n    .fix(5) 

with rooSilent() : 
    result, frame = model_cbrs. fitTo ( dataset0 )
    model_cbrs.signal.alpha.release()
    result, frame = model_cbrs. fitTo ( dataset0 )
    result, frame = model_cbrs. fitTo ( dataset0 )

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else : 
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean   & Sigma      are: ', result ( 'mean_Gauss')[0] , result( 'sigma_Gauss' )[0]

# =============================================================================
## double sided CrystalBall PDF
# =============================================================================
logger.info ('Test CrystalBallDS_pdf' )
model_cbds = Models.Fit1D (
    signal = Models.CB2_pdf ( name = 'CB2' , 
                              mass  = mass               ,
                              sigma = signal_gauss.sigma ,  
                              mean  = signal_gauss.mean  ) ,
    background = model_gauss.background  )

model_cbds.signal.aL.fix(2) 
model_cbds.signal.nL.fix(5) 
model_cbds.signal.aR.fix(2.5) 
model_cbds.signal.nR.fix(10) 

with rooSilent() : 
    result, frame = model_cbds. fitTo ( dataset0 )
    model_cbds.signal.aL.release()
    model_cbds.signal.aR.release()
    result, frame = model_cbds. fitTo ( dataset0 )
    model_cbds.signal.aL.fix(5) 
    model_cbds.signal.aL.fix(5)    
    result, frame = model_cbds. fitTo ( dataset0 )

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else : 
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean   & Sigma      are: ', result ( 'mean_Gauss')[0] , result( 'sigma_Gauss' )[0]


# =============================================================================
## Needham PDF
# =============================================================================
logger.info ('Test Needham_pdf' )
model_matt = Models.Fit1D (
    signal = Models.Needham_pdf ( name  = 'Matt' , 
                                  mass  = mass               ,
                                  sigma = signal_gauss.sigma ,  
                                  mean  = signal_gauss.mean  ) ,
    background = model_gauss.background  )

with rooSilent() : 
    result, frame = model_matt. fitTo ( dataset0 )
    model_matt.signal.mean .release()
    model_matt.signal.sigma.release()
    result, frame = model_matt. fitTo ( dataset0 )
    result, frame = model_matt. fitTo ( dataset0 )

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else : 
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean   & Sigma      are: ', result ( 'mean_Gauss')[0] , result( 'sigma_Gauss' )[0]




# ==========================================================================
## Apolonios
# ==========================================================================
logger.info ('Test Apolonios_pdf' ) 
model_apolonios = Models.Fit1D (
    signal = Models.Apolonios_pdf ( name  = 'APO', 
                                    mass  = mass ,
                                    mean  = signal_gauss.mean    ,
                                    sigma = signal_gauss.sigma ,
                                    b     =  1 ,
                                    n     = 10 ,
                                    alpha =  3 ) ,
    background = model_gauss.background  )

with rooSilent() : 
    result, frame = model_apolonios. fitTo ( dataset0 )
    result, frame = model_apolonios. fitTo ( dataset0 )
    
if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else : 
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean   & Sigma      are: ', result ( 'mean_Gauss')[0] , result( 'sigma_Gauss' )[0]


# ==========================================================================
## Apolonios2
# ==========================================================================
logger.info ('Test Apolonios2_pdf' ) 
model_apolonios2 = Models.Fit1D (
    signal = Models.Apolonios2_pdf ( name = 'AP2' , 
                                     mass      = mass ,
                                     mean      = signal_gauss.mean  ,
                                     sigma     = signal_gauss.sigma ,
                                     asymmetry = 0 ) ,
    background = model_gauss.background  )

with rooSilent() : 
    result, frame = model_apolonios2. fitTo ( dataset0 )
    model_apolonios2.signal.asym.release () 
    result, frame = model_apolonios2. fitTo ( dataset0 )
    
if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else : 
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean   & Sigma      are: ', result ( 'mean_Gauss')[0] , result( 'sigma_Gauss' )[0]

# =============================================================================
## Bifurkated gauss PDF
# =============================================================================
logger.info ('Test BifurcatedGauss_pdf' ) 
signal_bifurcated = Models.BifurcatedGauss_pdf ( name = 'BfGau' ,
                                                 mean  = signal_gauss.mean  ,
                                                 sigma = signal_gauss.sigma ,
                                                 mass  = mass    )

signal_bifurcated . asym  . setVal ( 0          )

model_bifurcated = Models.Fit1D( signal     = signal_bifurcated       ,
                                 background = model_gauss.background  )

model_bifurcated.b.setVal (  500 )
model_bifurcated.s.setVal ( 6000 )
with rooSilent() : 
    result, frame = model_bifurcated . fitTo ( dataset0 )
    result, frame = model_bifurcated . fitTo ( dataset0 )

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :     
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean   & Sigma      are: ', result ( 'mean_Gauss')[0] , result( 'sigma_Gauss' )[0]


# =============================================================================
## GenGaussV1
# ============================================================================= 
logger.info ('Test GenGaussV1_pdf' ) 
model_gauss_gv1 = Models.Fit1D (
    signal = Models.GenGaussV1_pdf ( name = 'Gv1' , 
                                     mass = mass  ,
                                     mean = signal_gauss.mean ) ,
    background = model_gauss.background  )

model_gauss_gv1.signal.beta .fix(2)
model_gauss_gv1.signal.mean .fix( m.value() ) 

with rooSilent() : 
    result, frame = model_gauss_gv1. fitTo ( dataset0 )
    model_gauss_gv1.signal.alpha.release()
    result, frame = model_gauss_gv1. fitTo ( dataset0 )
    model_gauss_gv1.signal.mean .release() 
    result, frame = model_gauss_gv1. fitTo ( dataset0 )

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :     
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean                is: ' , result ( 'mean_Gauss')[0] 

# =============================================================================
## GenGaussV2
# =============================================================================
logger.info ('Test GenGaussV2_pdf' ) 
model_gauss_gv2 = Models.Fit1D (
    signal = Models.GenGaussV2_pdf ( name = 'Gv2' , 
                                     mass = mass  ,
                                     mean = signal_gauss.mean ) ,
    background = model_gauss.background  )

model_gauss_gv2.signal.kappa.fix(0)
model_gauss_gv2.b.fix(0)

with rooSilent() : 
    result, frame = model_gauss_gv2. fitTo ( dataset0 )
    model_gauss_gv2.signal.mean.release() 
    result, frame = model_gauss_gv2. fitTo ( dataset0 )
    model_gauss_gv2.signal.kappa.release() 
    result, frame = model_gauss_gv2. fitTo ( dataset0 )
    result, frame = model_gauss_gv2. fitTo ( dataset0 )
    model_gauss_gv2.b.release() 
    result, frame = model_gauss_gv2. fitTo ( dataset0 )

if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean                is: ' , result ( 'mean_Gauss')[0] 


# =============================================================================
## SkewGauss
# =============================================================================
logger.info ('Test SkewGauss_pdf' ) 
model_gauss_skew = Models.Fit1D (
    signal = Models.SkewGauss_pdf ( name = 'GSk' , 
                                    mass = mass  , mean = signal_gauss.mean ) ,
    background = model_gauss.background  )

model_gauss_skew.signal.alpha.fix(0)

with rooSilent() : 
    result, frame = model_gauss_skew. fitTo ( dataset0 )
    result, frame = model_gauss_skew. fitTo ( dataset0 )
    
if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else : 
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean                is: ' , result ( 'mean_Gauss')[0] 


# =============================================================================
## Bukin
# =============================================================================
logger.info ('Test Bukin_pdf' ) 
model_bukin = Models.Fit1D (
    signal = Models.Bukin_pdf ( name  = 'Bukin' ,
                                mass  = mass    ,
                                xi    = 0    ,
                                rhol  = 0    ,
                                rhor  = 0    , 
                                mean  = signal_gauss.mean  , 
                                sigma = signal_gauss.sigma ) ,
    background = model_gauss.background  )


model_bukin.signal.mean .fix  ( m.value() )
model_bukin.signal.sigma.fix  ( m.error() )

with rooSilent() : 
    result, frame = model_bukin. fitTo ( dataset0 )
    model_bukin.signal.xi  .release()     
    result, frame = model_bukin. fitTo ( dataset0 )
    model_bukin.signal.rhol.release()     
    result, frame = model_bukin. fitTo ( dataset0 )
    model_bukin.signal.rhor.release()     
    result, frame = model_bukin. fitTo ( dataset0 )
    model_bukin.signal.mean .release() 
    model_bukin.signal.sigma.release() 
    result, frame = model_bukin. fitTo ( dataset0 )
    
if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else : 
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean                is: ' , result ( 'mean_Gauss')[0] 



# =============================================================================
## StudentT
# =============================================================================
logger.info ('Test StudentT_pdf' ) 
model_student = Models.Fit1D (
    signal = Models.StudentT_pdf ( name = 'ST' , 
                                   mass = mass ,
                                   mean = signal_gauss.mean ) ,
    background = model_gauss.background  )


model_student.signal.n    .setVal(20)
model_student.signal.sigma.setVal(0.013)

with rooSilent() : 
    result, frame = model_student. fitTo ( dataset0 )
    result, frame = model_student. fitTo ( dataset0 )
    
if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else : 
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'           )[0]
    print 'Mean                is: ' , result ( 'mean_Gauss')[0] 


# =============================================================================
## Bifurcated StudentT
# =============================================================================
logger.info ('Test bifurcated StudentT_pdf' ) 
model_bstudent = Models.Fit1D (
    signal = Models.BifurcatedStudentT_pdf ( name = 'BfST' , 
                                             mass = mass ,
                                             mean = signal_gauss.mean ) ,
    background = model_gauss.background  )


model_bstudent.signal.sigma.setVal(0.013)
model_bstudent.signal.nL.fix(25)
model_bstudent.signal.nR.fix(25)
with rooSilent() : 
    result, frame = model_bstudent. fitTo ( dataset0 )
    result, frame = model_bstudent. fitTo ( dataset0 )
    
if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else : 
    print 'Signal & Background are: ', result ( 'S' )[0] , result( 'B' )[0]
    print 'Mean                is: ' , result ( model_bstudent.signal.mean  .GetName() )[0] 
    print 'Sigma               is: ' , result ( model_bstudent.signal.sigma .GetName() )[0] 
    print 'Asymmetry           is: ' , result ( model_bstudent.signal.asym  .GetName() )[0] 
    print 'n(L)                is: ' , result ( model_bstudent.signal.nL    .GetName() )[0] 
    print 'n(R)                is: ' , result ( model_bstudent.signal.nR    .GetName() )[0] 
    
    
# =============================================================================
## Breit-Wigner
# =============================================================================
logger.info ('Test BreitWigner_pdf' )
bw = cpp.Gaudi.Math.BreitWigner( m.value() ,
                                 m.error() ,
                                 0.150     ,
                                 0.150     , 1 )
model_bw = Models.Fit1D (
    signal = Models.BreitWigner_pdf ( name        = 'BW' ,
                                      breitwigner = bw   ,     
                                      mass  = mass       ,
                                      mean  = signal_gauss.mean  ) , 
    background = model_gauss.background  )

model_bw.b.fix(500)
model_bw.signal.mean.fix ( m.value() )
with rooSilent() : 
    result, frame = model_bw. fitTo ( dataset0 )
    model_bw.signal.mean .release()
    model_bw.signal.gamma.release()
    result, frame = model_bw. fitTo ( dataset0 )
    
if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :
    print 'Signal & Background are: ', result ( 'S'         )[0] , result( 'B'        )[0]
    print 'Mean   & Gamma      are: ', result ( 'mean_Gauss')[0] , result( 'gamma_BW' )[0]


# =============================================================================
## Voigt
# =============================================================================
logger.info ('Test Voigt_pdf' )
model_vgt = Models.Fit1D (
    signal = Models.Voigt_pdf ( 'V' , mass.getMin() , mass.getMax()  ,
                                mass  = mass                ,
                                mean  = signal_gauss.mean   , 
                                sigma = signal_gauss.sigma  ) , 
    background = model_gauss.background  )

model_vgt.signal.sigma.fix ( m.error() )
model_vgt.signal.gamma.fix ( 0.010     )
with rooSilent() : 
    result, frame = model_vgt. fitTo ( dataset0 )
    model_vgt.signal.gamma.release() 
    result, frame = model_vgt. fitTo ( dataset0 )
    model_vgt.signal.sigma.release() 
    result, frame = model_vgt. fitTo ( dataset0 )
    
if 0 != result.status() or 3 != result.covQual() :
    logger.warning('Fit is not perfect MIGRAD=%d QUAL=%d ' % ( result.status() , result.covQual () ) )
    print result 
else :
    print 'Signal & Background are: ', result ( 'S'           )[0] , result ( 'B'       )[0]
    print 'Mean   & Gamma      are: ', result ( 'mean_Gauss'  )[0] , result ( 'gamma_V' )[0]
    print 'Sigma               is : ', result ( model_vgt.signal.sigma.GetName() )[0] 
    

# =============================================================================
# The END 
# =============================================================================
