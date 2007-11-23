#!/bin/bash
test -n "$1" ; export TOPLEVEL=$1
test -n "$2" ; export PARTNAME=$2

export INFOOPTIONS=/group/online/dataflow/options/${PARTNAME}/${PARTNAME}_Info.opts
export OPTIONS=/group/online/dataflow/options/${PARTNAME}/${PARTNAME}_${TOPLEVEL}_HLT.opts
shift
. ./setupOnline.sh
${gaudi_exe}  -main=${GAUDIONLINEROOT}/options/ErrorSrv.opts -opt=${GAUDIONLINEROOT}/options/Daemon.opts &

