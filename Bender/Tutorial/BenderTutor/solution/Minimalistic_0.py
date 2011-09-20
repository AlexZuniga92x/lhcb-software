# $Id: Minimalistic_0.py,v 1.13 2010-09-13 13:24:04 ibelyaev Exp $
# =============================================================================
## @file solution/Minimalictic_0.py
#
#  The most trivial ``Bender-based'' script:
#                                 Essentially it is DaVinci + GaudiPython
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
#  @author Vanya BELYAEV ibelyaev@physics.syr.edu
#  @date 2006-10-12
#
#  Last modification $Date: 2010-09-13 13:24:04 $
#                 by $Author: ibelyaev $
# =============================================================================
"""

The most trivial ``Bender-based'' script:
                               Essentially it is DaVinci + GaudiPython

This file is a part of BENDER project:
``Python-based Interactive Environment for Smart and Friendly Physics Analysis''

The project has been designed with the kind help from
Pere MATO and Andrey TSAREGORODTSEV. 

And it is based on the 
LoKi project: ``C++ ToolKit for Smart and Friendly Physics Analysis''

By usage of this code one clearly states the disagreement 
with the smear campaign of Dr.O.Callot et al.: 
``No Vanya's lines are allowed in LHCb/Gaudi software.''

Last modification $Date: 2010-09-13 13:24:04 $
               by $Author: ibelyaev $
"""
# =============================================================================
__author__  = " Vanya BELYAEV Ivan.Belyaev@nikhef.nl "
__date__    = " 2006-10-12 " 
__version__ = " Version $Revision: 1.13 $ "
# =============================================================================

# =============================================================================
from Bender.Main import *

from Configurables import DaVinci

DaVinci (
    DataType   = '2011' , 
    Simulation = False
    )

importOptions('$STRIPPINGSELECTIONSROOT/tests/data/Reco11aNewRoot_Run97120_SDSTs.py')

## get or create application manager 
gaudi = appMgr() 

## run 10 events:
run(10)

## browse transient store: 
ls ( )
ls ('/Event/Strip')

## tun 3-events 
for i in range(0,3) :
    
    run ( 1 )
    
    ls  ( '/Event/Strip/Phys' )

# =============================================================================
# The END 
# =============================================================================
