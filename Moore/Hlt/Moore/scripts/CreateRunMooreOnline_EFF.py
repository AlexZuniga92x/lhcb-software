#!/usr/bin/env python

import os, os.path
import sys
import subprocess
import string

#
cmt = subprocess.Popen(['cmt','show','version'],stdout=subprocess.PIPE)
version = string.strip(cmt.communicate()[0],'\n')

env   = 'SetupMoore_'+version+'.sh'
setup = '$LHCBSCRIPTS/' + env
moore = '$MOOREROOT'
if len(sys.argv)>2:
    moore = os.environ['MOOREROOT']
    setup = os.path.join(moore,'job',env)
    print 'generating ' + setup
    #ret = subprocess.call(['python',os.path.join(os.environ['LHCBPYTHON'],'SetupProject.py'),'--dev-dir=/home/online/ONLINE:/group/hlt/GAUDI','--shell=sh','--output='+setup,'Moore',version])
    from LbConfiguration.SetupProject import SetupProject
    SetupProject().main(['--dev-dir=/home/online/ONLINE:/group/hlt/GAUDI','--shell=sh','--output='+setup,'Moore',version])

    print 'removing use of StripPath.sh'
    # remove call to StripPath.sh from generated SetupProject.sh
    f = open(setup, 'r')
    txt = f.read()
    f.close()
    f = open(setup, 'w')
    import re
    f.write(re.sub('test -f [^ ]*StripPath.sh','false', txt))
    f.close()



print 'using ' + setup

print 'generating '+ sys.argv[1]
file = open(sys.argv[1],'w+')
file.write( """#!/bin/sh
# pick up the online setup...
source ./setupOnline.sh $*
# pick up 'our' setup... 
source %(setup)s
# pick up partition specific OnlineEnv module
export PYTHONPATH=/group/online/dataflow/options/${PARTNAME}/HLT:${PYTHONPATH}
# and go for it!
exec -a ${UTGID} ${CLASS1_TASK} -opt=command="import Moore.runOnline; Moore.runOnline.start()"

"""%({'setup': setup,'moore':moore}) )


from stat import *
orig = os.stat(sys.argv[1])[0]
rwxrwxrx = S_IRUSR | S_IRGRP | S_IROTH | S_IWUSR | S_IWGRP | S_IXUSR | S_IXGRP | S_IXOTH 
if orig|rwxrwxrx != orig :
    print '%s has permissions %d -- want %d instead' % (sys.argv[1],orig,orig|rwxrwxrx)
    try :
        os.chmod(sys.argv[1],rwxrwxrx)
        print 'updated permission of %s'%(sys.argv[1])
    except :
        print 'WARNING: could not update permissions of %s -- please make sure it is executable!' % sys.argv[1]
