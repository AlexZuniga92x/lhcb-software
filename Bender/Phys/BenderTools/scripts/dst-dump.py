#!/usr/bin/env python
# =============================================================================
## @file dst-dump.py
#
#  Trivial Bender-based script to dump the content of (x,mu,s,r,...)DSTs
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
# Usage:
#
# @code
#
#    dst_dump [options] file1 [ file2 [ file3 [ file4 ....'
#
#  @endcode 
#
#  @date   2011-10-24
#  @author Vanya BELYAEV Ivan.Belyaev@itep.ru
#
#                    $Revision: 138072 $
#  Last modification $Date: 2012-04-04 16:41:10 +0200 (Wed, 04 Apr 2012) $
#                 by $Author: ibelyaev $
# =============================================================================

"""
Trivial Bender-based script to dump the content of (x,mu,s,r,...)DSTs

This file is a part of BENDER project:

  ``Python-based Interactive Environment for Smart and Friendly Physics Analysis''

The project has been designed with the kind help from Pere MATO and Andrey TSAREGORODTSEV. 

And it is based on the LoKi project:
 
   ``C++ ToolKit for Smart and Friendly Physics Analysis''

By usage of this code one clearly states the disagreement with the smear campaign of Dr.O.Callot et al.:

   ``No Vanya's lines are allowed in LHCb/Gaudi software''

Usage:

    dst-dump [options] file1 [ file2 [ file3 [ file4 ....'


                  $Revision: 138072 $
Last modification $Date: 2012-04-04 16:41:10 +0200 (Wed, 04 Apr 2012) $
               by $Author: ibelyaev $

"""
# =============================================================================
__author__  = 'Vanya BELYAEV Ivan.Belyaev@itep.ru'
__date__    = "2011-10-24"
__version__ = '$Revision: 138072 $'
__all__     = ()  ## nothing to import 
__usage__   = 'dst_dump [options] file1 [ file2 [ file3 [ file4 ....'
# =============================================================================

print 120*'*'
print ' Author  : ', __author__ 
print ' Version : ', __version__ 
print ' Date    : ', __date__

from BenderTools.DstDump import dumpDst

dumpDst ( __usage__ , __version__ )

print 120*'*'

# =============================================================================
# The END 
# =============================================================================
