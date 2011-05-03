#!/usr/bin/env python
# $Id$
""" test file for the AFS module """

from os import environ, sep
from LbUtils.afs.directory import getDirID, getDirVolumeID, getDirVolumeName
from LbUtils.afs.directory import getParentMountPoint, NotInAFS
from LbUtils.afs.directory import isMountPoint, Directory
from LbUtils.afs.volume import Volume

__version__ = "$Id$"


if __name__ == '__main__':
#    print isMountPoint(environ["HOME"])
#    print isMountPoint(sep.join([environ["HOME"],"tmp"]))
#    print isMountPoint(environ["HOME"])
#    print "========================"
#    print getDirID(sep.join([environ["HOME"],"tmp"]))
#    print "========================"
#    print getDirVolumeID(sep.join([environ["HOME"],"tmp"]))
#    print "========================"
#    print getParentMountPoint(environ["HOME"])
#    print getParentMountPoint(sep.join([environ["HOME"], "tmp"] ))
#    print getParentMountPoint(sep.join([environ["HOME"], "scratch0"] ))
#
#    print "============================================="
##    print isMountPoint("/tmp")
##    print getDirID("/tmp")
##   print getDirVolumeID("/tmp")
##    print getParentMountPoint("/tmp")
#    print "============================================="
#    print getDirVolumeName(environ["HOME"])
#    print getDirVolumeName(sep.join([environ["HOME"], "tmp"] ))
#    print getDirVolumeName(sep.join([environ["HOME"], "scratch0"] ))
    try :
    	print getDirVolumeName("/tmp")
    except NotInAFS :
        pass

    try :
        h = Directory(environ["HOME"])
        h.getACL()
    except NotInAFS:
        pass

    try :
        v = Volume(dirname=environ["HOME"])
    except NotInAFS:
        pass

    v2 = Volume(dirname="/afs/cern.ch/user/h/hmd/scratch0")
    print v2.name()
    print v2.quota("B")
    print v2.usedSpace("B")
    print v2.quota("KB")
    print v2.usedSpace("KB")
    print v2.quota("MB")
    print v2.usedSpace("MB")
    print v2.quota("GB")
    print v2.usedSpace("GB")
    print v2.quota("human")
    print v2.usedSpace("human")

