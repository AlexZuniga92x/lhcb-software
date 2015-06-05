#!/usr/bin/env python
# =============================================================================
# $Id$ 
# =============================================================================
## @file 	
#  Simple script to extract DB-tags from the file 
#
#  This file is a part of 
#  <a href="http://cern.ch/lhcb-comp/Analysis/Bender/index.html">Bender project</a>
#  <b>``Python-based Interactive Environment for Smart and Friendly Physics Analysis''</b>
#
#  The package has been designed with the kind help from
#  Pere MATO and Andrey TSAREGORODTSEV. 
#  And it is based on the 
#  <a href="http://cern.ch/lhcb-comp/Analysis/LoKi/index.html">LoKi project:</a>
#  <b>``C++ ToolKit for Smart and Friendly Physics Analysis''</b>
#
#  By usage of this code one clearly states the disagreement 
#  with the smear campaign of Dr.O.Callot et al.: 
#  ``No Vanya's lines are allowed in LHCb/Gaudi software''
#
#  @code
# 
#  Usage:
#
#     get_dbtags [options] file1 [ file2 [ file3 [ file4 ....'
#
#  @endcode
#
#  @author Thomas RUF 
#  @author Vanya BELYAEV 
#  @date   2012-10-24
#
#                    $Revision$
#  Last modification $Date$
#                 by $Author$
#
# =============================================================================
"""
Simple script to extract DB-tags from the file

This file is a part of BENDER project:

  ``Python-based Interactive Environment for Smart and Friendly Physics Analysis''

The project has been designed with the kind help from Pere MATO and Andrey TSAREGORODTSEV. 

And it is based on the LoKi project:
 
   ``C++ ToolKit for Smart and Friendly Physics Analysis''

By usage of this code one clearly states the disagreement with the smear campaign of Dr.O.Callot et al.:

   ``No Vanya's lines are allowed in LHCb/Gaudi software''

Usage:

    get_dbtags [options] file1 [ file2 [ file3 [ file4 ....'

"""
# =============================================================================
__author__  = "Thomas RUF, Vanya BELYAEV"
__date__    = "2012-10-24"
__version__ = "$Revision$"

# =============================================================================
if '__main__' == __name__ :
    
    import ROOT
    ROOT.PyConfig.IgnoreCommandLineOptions = True
    
    print 120*'*'
    print                   __doc__ 
    print ' Author(s)  : ', __author__ 
    print ' Version    : ', __version__
    print ' Date       : ', __date__ 
    print 120*'*'
    
    from BenderTools.GetDBtags  import extractTags 

    import sys
    extractTags ( sys.argv[1:] )
    
# =============================================================================
# The END 
# =============================================================================
