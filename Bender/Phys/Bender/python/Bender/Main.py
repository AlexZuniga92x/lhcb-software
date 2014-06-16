#!/usr/bin/env python 
# =============================================================================
# $Id$ 
# =============================================================================
## @file Bender/Main.py
#  The major Python module for Bender application 
#
#  This file is a part of 
#  <a href="http://cern.ch/lhcb-comp/Analysis/Bender/index.html">Bender project</a>
#  <b>``Python-based Interactive Environment for Smart and Friendly 
#   Physics Analysis''</b>
#
#  The package has been designed with the kind help from
#  Pere MATO and Andrey TSAREGORODTSEV. 
#  And it is based on the 
#  <a href="http://cern.ch/lhcb-comp/Analysis/LoKi/index.html">LoKi project:</a>
#  ``C++ ToolKit for Smart and Friendly Physics Analysis''
#
#  By usage of this code one clearly states the disagreement 
#  with the smear campaign of Dr.O.Callot et al.: 
#  ``No Vanya's lines are allowed in LHCb/Gaudi software.''
#
#  @date   2004-07-11
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#
#                    $Revision$
#  Last modification $Date$
#                 by $Author$ 
# =============================================================================
"""This is a major Python Module for Bender application

oooooooooo.                              .o8                     
`888'   `Y8b                            \"888                     
 888     888  .ooooo.  ooo. .oo.    .oooo888   .ooooo.  oooo d8b 
 888oooo888' d88' `88b `888P\"Y88b  d88' `888  d88' `88b `888\"\"8P 
 888    `88b 888ooo888  888   888  888   888  888ooo888  888     
 888    .88P 888    .o  888   888  888   888  888    .o  888     
o888bood8P'  `Y8bod8P' o888o o888o `Y8bod88P\" `Y8bod8P' d888b    

This file is a part of BENDER project:
    ``Python-based Interactive Environment for Smart and Friendly Physics Analysis''

The project has been designed with the kind help from Pere MATO and Andrey TSAREGORODTSEV. 

And it is based on the LoKi project:
    ``C++ ToolKit for Smart and Friendly Physics Analysis''

By usage of this code one clearly states the disagreement 
with the smear campain of Dr.O.Callot et al.: 
    ``No Vanya's lines are allowed in LHCb/Gaudi software.''

  Last modification $Date$
                 by $Author$ 
"""
# =============================================================================
__author__  = 'Vanya BELYAEV ibelyaev@physics.syr.edu'
__date__    = "2004-07-11"
__version__ = '$Revision$' 
# =============================================================================
## logging
# =============================================================================
from Bender.Logger import getLogger 
if '__main__' == __name__ : logger = getLogger ( 'Bender.Main' )
else                      : logger = getLogger ( __name__ )
# =============================================================================
import os
try:
    ## try to get the startup script from environment 
    startup = os.environ.get('PYTHONSTARTUP',None)
    ## use the default startup script
    if not startup :
        startup = os.sep + 'Bender' + os.sep + 'Startup.py'
        bp = os.environ.get('BENDERPYTHON',None)
        if bp : startup = bp + startup
        else  : startup = 'Startup.py'
    
    if os.path.exists( startup ) :
        execfile( startup )
    else :
        import Bender.Startup
        
    logger.info ( 'Define the startup file to be %s' % startup )

except:
    pass

# =============================================================================
import ROOT
from   LoKiCore.basic import cpp
if not hasattr ( ROOT , 'ostream' ) :
    logger.debug ( 'Fix ostream issue. Needed???')
    ROOT.gROOT.ProcessLine("#include <ostream>")     
    
# =============================================================================

## massive imports of everything-II
logger.debug ( "Massive   import of ``everything''")
from LoKiCore.decorators           import *
from LoKiPhys.decorators           import *
from LoKiArrayFunctors.decorators  import *
from LoKiAlgo.decorators           import *
from LoKiCore.functions            import *


## massive imports of everything-II
logger.debug ( "Even more imports of ``everything''")
## from LoKiTrigger.decorators        import *
from LoKiTracks.decorators         import *
from LoKiProtoParticles.decorators import *
from LoKiHlt.decorators            import *
from LoKiNumbers.decorators        import *

# =============================================================================
from Gaudi.Configuration import importOptions
from Bender.Utils        import * 
# =============================================================================
## apply some last-minutes fixes
import Bender.Fixes

## @var LoKi   : define namespace LoKi 
LoKi   = cpp.LoKi
## @var Bender : define namespace Bender 
Bender = cpp.Bender
## @var LHCb   : define namespace LHCb
LHCb   = cpp.LHCb
## @var Gaudi  : define namespace Gaudi
Gaudi  = cpp.Gaudi

_SC     = cpp.StatusCode
SUCCESS = _SC(_SC.SUCCESS,True)
FAILURE = _SC(_SC.FAILURE,True)

from GaudiKernel.PhysicalConstants import c_light

# =============================================================================
## Welcome message:
Bender.Welcome.instance()

# =============================================================================
if __name__ == '__main__' :

    logger.info ( 80*'*'  ) 
    logger.info ( __doc__ ) 
    logger.info ( ' Author  : %s ' %  __author__  ) 
    logger.info ( ' Version : %s ' %  __version__ ) 
    logger.info ( ' Date    : %s ' %  __date__    ) 
    logger.info ( 80*'*'  ) 

# =============================================================================
# The END 
# =============================================================================
