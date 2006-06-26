set TAN_PORT=YES
set TAN_NODE=%COMPUTERNAME%.cern.ch
set test_exe=%ONLINEKERNELROOT%\win32_vc71_dbg\test.exe
set gaudi_exe=%GAUDIONLINEROOT%\win32_vc71_dbg\Gaudi.exe GaudiOnline.dll OnlineTask 
set UTGID=MBMMon
start "%UTGID%" %test_exe% mbm_mon
set UTGID=MBMInit
start "%UTGID%" %gaudi_exe% -main=../options/MBMinit.opts  -opt=../options/Daemon.opts -msgsvc=LHCb::DimMessageSvc -a
rem set UTGID=TANServ
rem start "%UTGID%" %gaudi_exe% -main=../options/TanAlloc.opts -opt=../options/Daemon.opts -msgsvc=LHCb::DimMessageSvc -a
rem set UTGID=TANMon
rem start "%UTGID%"  %test_exe% tanmon -c
set UTGID=ErrServ
start "%UTGID%" %gaudi_exe% -opt=../options/ErrorSrv.opts -msgsvc=LHCb::DimMessageSvc -a
sleep 8
set UTGID=EvtProd
start "%UTGID%" %gaudi_exe% -opt=../options/MEPConverter.opts -msgsvc=LHCb::DimMessageSvc -a
set UTGID=EvtHolde
start "%UTGID%" %gaudi_exe% -opt=../options/MEPHolder.opts    -msgsvc=LHCb::DimMessageSvc -a
set UTGID=Moore_0
start "%UTGID%" %gaudi_exe% -opt=../options/ReadMBM.opts      -msgsvc=LHCb::DimMessageSvc -a
set UTGID=Moore_1
start "%UTGID%" %gaudi_exe% -opt=../options/ReadMBM.opts      -msgsvc=LHCb::DimMessageSvc -a
rem
set UTGID=OutputBuffer 
start "%UTGID%" %test_exe% mbm_install -s=8096 -e=64 -u=64 -i=OUTPUT
set UTGID=Sender
start "%UTGID%" %gaudi_exe% -opt=../options/MDFSender.opts    -msgsvc=LHCb::DimMessageSvc -a
set UTGID=NetRead_0
start "%UTGID%" %gaudi_exe% -opt=../options/MDFReceiver.opts  -msgsvc=LHCb::DimMessageSvc -a
set UTGID=DiskWR
start "%UTGID%" %gaudi_exe% -opt=../options/DiskWR.opts  -msgsvc=LHCb::DimMessageSvc -a

rem start /b xterm -132 -geometry 132x45  -title ErrorLogger  -e /bin/bash -l -c "export UTGID=ErrorLogger; cd J:/Gaudi/Online/UPI/cmt; ./upi.sh ../../GaudiOnline/win32_vc71_dbg/gaudi.exe GaudiOnline OnlineTask -opts=../../GaudiUPI/options/Errlog.opts -msgsvc=LHCb::DimMessageSvc -a"

rem start "prod_0" ..\win32_vc71_dbg\Gaudi.exe GaudiOnline mep_producer -n=prod_0 -p=333 -s=500 -r=2
