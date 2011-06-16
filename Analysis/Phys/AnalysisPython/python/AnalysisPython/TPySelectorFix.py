#!/usr/bin/env python
# =============================================================================
# $Id:$
# =============================================================================
# @file TPySelectorFix.py
# 
# Helper module to fix a problems in communication of
# <c>TTree/TChain::Process</c> and <c>TPySelector</c>.
# In PyROOT some of original C++ methods are disable.
# The module provides the "recovery" for missing methods
#
# @see TTree::Process
# @see TChain::Process
# @see TPySelector
#
# @code
#
# from AnalysisPython.TpySelectorFix import Selector
#
# class MySelector ( Selector ) :
#
#      def __init__ ( self ) :
#
#           return Selector ( self )
#
#      def Process  ( self , entry ) :
#           # == getting the next entry from the tree
#           if self.GetEntry ( entry ) <= 0 : return 0             ## RETURN 
#        
#           # == for more convenience
#           tree=self.fChain
#
#           # apply trivial "acceptance" cuts 
#           if not 2 <= tree.y   <=  4.5   : return 0               ## RETURN
#           if not 1 <= tree.pt  <=  10.0  : return 0               ## RETURN
#
#           return 1
#
# 
# selector = MySelector()
#
# chain = ...
# chain.process ( selector )  ## NB: note lowercase "process" here !!!
#
# @endcode
#
# The module has been developed and used with great success in
# ``Kali, framework for fine calibration of LHCb Electormagnetic Calorimeter''
#
# @author Vanya BELYAEV Ivan.Belyaev@cern.ch
# @date   2010-04-30
# 
#                    $Revision$
#  Last Modification $Date$
#                 by $Author$
# =============================================================================
"""
 Helper module to fix a problems in communication of
 TTree/TChain::Process< and TPySelector.
 In PyROOT some of original C++ methods are disable.
 The module provides the 'recovery' for missing methods


    from AnalysisPython.TpySelectorFix import Analysis

    class MySelector ( Analysis.Selector ) :

       def __init__ ( self ) :

           return Analysis.Selector ( self )
       def Process  ( self , entry ) :
           # == getting the next entry from the tree
           if self.GetEntry ( entry ) <= 0 : return 0             ## RETURN 
        
           # == for more convenience
           tree=self.fChain

           # apply trivial 'acceptance' cuts 
           if not 2 <= tree.y   <=  4.5   : return 0               ## RETURN
           if not 1 <= tree.pt  <=  10.0  : return 0               ## RETURN

           return 1

 
    selector = MySelector()

    chain = ...
    
    chain.process ( selector )  ## NB: note lowercase "process" here !!!

 The module has been developed and used with great success in
 ``Kali, framework for fine calibration of LHCb Electormagnetic Calorimeter''
 
"""
# =============================================================================
__author__  = "Vanya BELYAEV Ivan.Belyaev@cern.ch"
__date__    = "2010-04-30"
__version__ = "$Revision$" 
# =============================================================================
__all__ = (
'Analysis' ,   ## The C++ namespace  Analysis
'Selector'     ## The ``fixed'' TPySelector
)
# =============================================================================
import ROOT, PyCintex

# construct the global namespace 
_global   = PyCintex.makeNamespace('')
Analysis  = _global.Analysis

## @see Analysis::Process 
Process   = Analysis.Process

## @see Analysis::Selector
Selector  = Analysis.Selector

# =============================================================================
## define the helper function for proper decoration of ROOT.TTree/TChain
#
# @code
#
# from AnalysisPython.TpySelectorFix import Selector
#
# class MySelector ( Selector ) :
#
#      def __init__ ( self ) :
#
#           return Selector ( self )
#
#      def Process  ( self , entry ) :
#          # == getting the next entry from the tree
#           if self.GetEntry ( entry ) <= 0 : return 0             ## RETURN 
#        
#           # == for more convenience
#           tree=self.fChain
#
#           # apply trivial "acceptance" cuts 
#           if not 2 <= tree.y   <=  4.5   : return 0               ## RETURN
#           if not 1 <= tree.pt  <=  10.0  : return 0               ## RETURN
#
#           return 1
#
# 
# selector = MySelector()
#
# chain = ...
# chain.process ( selector )  ## NB: note lowercase "process" here !!!
#
# @endcode
#
# The module has been developed and used with great success in
# ``Kali, framework for fine cailbration of LHCb Electormagnetic Calorimeter''
#
# @see Analysis::Selector 
# @see Analysis::Process 
# @author Vanya BELYAEV Ivan.Belyaev@cern.ch
# @date   2010-04-30
#
def _process_ ( self , selector ) :
    """
    ``Process'' the tree/chain with proper TPySelector :
    
    from AnalysisPython.TpySelectorFix import Selector
    
    class MySelector ( Selector ) : ... 
    
    selector = MySelector()
    
    chain = ...
    chain.process ( selector )  ## NB: note lowercase ``process'' here !!!
    
    """
    return Analysis.Process.process ( self , selector )
_process_. __doc__ += '\n' + Analysis.Process.process.__doc__


# =============================================================================
## finally: decorate TTree/TChain
for t in ( ROOT.TTree , ROOT.TChain ) : t.process  = _process_ 

# =============================================================================
if '__main__' == __name__ :

    print 100*'*'
    print __doc__
    print 100*'*'
    print ' Author  : ' , __author__
    print ' Date    : ' , __date__
    print ' Version : ' , __version__
    print 100*'*'
    
# =============================================================================
# The END 
# =============================================================================

