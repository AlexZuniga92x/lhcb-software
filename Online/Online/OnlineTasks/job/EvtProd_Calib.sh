#!/bin/bash
#cd job
export DEBUGGING=YES

. ./setupOnline.sh $*

#always run dummy opts in calib farm
exec -a ${UTGID} ${CLASS1_TASK}  -opts=${HLTOPTS}/EvtProd_TAE.opts

