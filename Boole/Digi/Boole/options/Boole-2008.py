##############################################################################
# File for running Boole with default 2008 geometry, as defined in
#                                                        DDDB/Configuration.py
# Syntax is:
#   gaudirun.py Boole-2008.py <someDataFiles>.py
##############################################################################

from Boole.Configuration import *

Boole().useSpillover = False
Boole().DDDBtag      = "2008-default"
Boole().condDBtag    = "2008-default"

##############################################################################
# I/O datasets are defined in a separate file, see examples in 2008-Files.py
##############################################################################
