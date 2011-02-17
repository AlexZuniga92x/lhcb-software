""" General LHCb configuration for external software """

import sys


Python_version     = "2.5"
CMT_version        = "v1r20p20090520"
tbroadcast_version = "v2.0.5"
doxygen_version    = "1.7.2"


external_projects  = ['LCGCMT', 'GENSER', 'OpenScientist']
lcg_projects       = ['LCGCMT', 'SEAL','POOL','GENSER','COOL','GDML','CORAL','RELAX']

def getExternalVersion(ext_name):
    vers_attr = "%s_version" % ext_name
    if hasattr(sys.modules[__name__], vers_attr) :
        return getattr(sys.modules[__name__], vers_attr)
