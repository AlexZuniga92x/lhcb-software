#!/bin/bash


#export MINITERM='xterm -iconic -sl 10000 -ls -132 -geometry 132x10 -title '

#start_py_task()
#{
#  $MINITERM ${1}@${HOST}   -e "export UTGID=${NODENAME}/${1};   exec -a \${UTGID} $Class1_task -opt=command=\"${2}\"&"
#}


export TAN_PORT=YES
export TAN_NODE=$HOSTNAME
if test -z "${DIM_DNS_NODE}";
then
    export DIM_DNS_NODE=$HOSTNAME;
fi;
#

export OPTS=$GAUDIONLINEROOT/options
export msg_svc=LHCb::FmcMessageSvc
export msg_svc=MessageSvc

export monitorreaderd_run="/home/dsonnick/namedpipe/monitorreader/monitorreaderd"
export test_exe="$ONLINEKERNELROOT/$CMTCONFIG/test.exe "
export gaudi_run="$GAUDIONLINEROOT/$CMTCONFIG/Gaudi.exe libGaudiOnline.so OnlineStart "
export gaudi_exe="$GAUDIONLINEROOT/$CMTCONFIG/Gaudi.exe libGaudiOnline.so OnlineTask -msgsvc=$msg_svc -auto "
export gaudi_exe2="$GAUDIONLINEROOT/$CMTCONFIG/Gaudi.exe libGaudiOnline.so OnlineTask -msgsvc=MessageSvc -auto "
export gaudi_task="$GAUDIONLINEROOT/$CMTCONFIG/Gaudi.exe libGaudiOnline.so OnlineTask -msgsvc=$msg_svc -auto -main=$OPTS/Main.opts "
export Class0_task="$GAUDIONLINEROOT/$CMTCONFIG/Gaudi.exe libGaudiOnline.so OnlineTask -tasktype=LHCb::Class0Task -msgsvc=$msg_svc -auto -main=$OPTS/Main.opts "
export Class1_task_noauto="$GAUDIONLINEROOT/$CMTCONFIG/Gaudi.exe libGaudiOnline.so OnlineTask -tasktype=LHCb::Class1Task -msgsvc=$msg_svc -main=$OPTS/Main.opts "
export Class1_task="$GAUDIONLINEROOT/$CMTCONFIG/Gaudi.exe libGaudiOnline.so OnlineTask -tasktype=LHCb::Class1Task -msgsvc=$msg_svc -auto -main=$OPTS/Main.opts "
export Class2_task="$GAUDIONLINEROOT/$CMTCONFIG/Gaudi.exe libGaudiOnline.so OnlineTask -tasktype=LHCb::Class2Task -msgsvc=$msg_svc -auto -main=$OPTS/Main.opts "
export MINITERM='xterm -hold -ls -132 -geometry 132x12 -title '
export BIGTERM='xterm -hold -ls -132 -geometry 132x55 -title '
export WIDETERM='xterm -hold  -ls -132 -geometry 160x50 -title '
#export MINITERM=echo
export NODENAME=`python -c "print '$HOST'.split('.')[0]"`
if test -z "${PYOPTS}";
  then
    export PYOPTS=${GAUDIONLINEROOT}/python
    export PYTHONPATH=${PYOPTS}:${PYTHONPATH}
fi;
#
start_py_task()
{
  $MINITERM ${1}@${HOST}   -e "export UTGID=${NODENAME}/${1};   exec -a \${UTGID} $Class1_task -opt=command=\"${2}\""&
}





echo $*
id=0

for i in $* ; do
#start_py_task Mdf2Mbm9 "import GaudiOnlineTests;GaudiOnlineTests.runMDF2MBMFile(['TestWriter'],fname='root:$1',partitionBuffers=True)"
    start_py_task Mdf2Mbm_$id "import GaudiOnlineTests;GaudiOnlineTests.runMDF2MBMFile(['TestWriter'],fname='root:$i')"
    sleep 10
    id=$(($id+1))

done

