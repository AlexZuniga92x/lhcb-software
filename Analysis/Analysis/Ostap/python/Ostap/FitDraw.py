#!/usr/bin/env python
# -*- coding: utf-8 -*-
# =============================================================================
## @file FitBasic.py
#
#  Set of useful basic utilities to build various fit models 
#
#  @author Vanya BELYAEV Ivan.Belyaeve@itep.ru
#  @date 2011-07-25
# 
#                    $Revision$
#  Last modification $Date$
#                 by $Author$
# =============================================================================
"""Set of useful basic utilities to build various fit models"""
# =============================================================================
__version__ = "$Revision:"
__author__  = "Vanya BELYAEV Ivan.Belyaev@itep.ru"
__date__    = "2011-07-25"
__all__     = (
    ##
    'lineWidth'             , ## line width
    'lineColor'             , ## line color 
    'lineStyle'             , ## line style 
    ##
    'data_options'          , ## option for data
    'data_options_nobars'   , ## option for data without bars 
    'signal_options'        , ## signal options    
    'background_options'    , ## background options 
    'component_options'     , ## other components
    'fit_options'           , ## the fit curve
    ##
    'base_signal_color'     , ## base signal color 
    'base_background_color' , ## base signal color 
    'base_component_color'  , ## base signal color 
    )
# =============================================================================
import ROOT
from   Ostap.Logger     import getLogger
# =============================================================================
if '__main__' ==  __name__ : logger = getLogger ( 'Ostap.FitDraw' )
else                       : logger = getLogger ( __name__         )
# =============================================================================
def lineWidth ( w ) : return ROOT.RooFit.LineWidth ( w )
def lineStyle ( s ) : return ROOT.RooFit.LineStyle ( s )
def lineColor ( c ) : return ROOT.RooFit.LineColor ( c )
#

## plain, default
data_options           = ()

## suppress small bars at the end of error bars 
data_options_nobars     = ( ROOT.RooFit.MarkerStyle ( 20   ) ,
                            ROOT.RooFit.DrawOption  ( "zp" ) )

## signal:          thin dotted line
signal_options          = lineWidth ( 1 ) , lineStyle ( 1  )

## 1D background:   thin long-dashed line
background_options      = lineWidth ( 1 ) , lineStyle ( 7  )

## "component":     thin dash-dotted line
component_options       = lineWidth ( 1 ) , lineStyle ( ROOT.kDashDotted )

## total fit curve: thick red solid line 
total_fit_options       = lineWidth ( 3 ) , lineColor ( ROOT.kOrange + 1 ) , lineStyle ( 1 ) 

base_signal_color       = ROOT.kRed
base_background_color   = ROOT.kBlue
base_component_color    = ROOT.kMagenta

## background:  thin short-dashed line
background2D_options    = lineWidth ( 1 ) , lineStyle ( ROOT.kDashed     ) 
base_background2D_color = ROOT.kBlue 

crossterm1_options      = lineWidth ( 1 ) , lineStyle ( 7 )  
crossterm2_options      = lineWidth ( 1 ) , lineStyle ( 9 )  

base_crossterm1_color   = ROOT.kMagenta
base_crossterm2_color   = ROOT.kGreen+1 

# =============================================================================
if '__main__' == __name__ :
    
    from Ostap.Line import line 
    logger.info ( __file__ + '\n' + line  )
    logger.info ( 80*'*' )
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
