start %ONLINEKERNELROOT%\win32_vc71_dbg\test.exe mbm_install -s=10000 -e=10 -u=20 -i=MEP
sleep 1
start %ONLINEKERNELROOT%\win32_vc71_dbg\test.exe mbm_install -s=4096 -e=50 -u=20 -i=EVENT
sleep 1
start %ONLINEKERNELROOT%\win32_vc71_dbg\test.exe mbm_install -s=4096 -e=100 -u=20 -i=RESULT
sleep 1
start %ONLINEKERNELROOT%\win32_vc71_dbg\test.exe mbm_mon
sleep 1
start %ONLINEKERNELROOT%\win32_vc71_dbg\test.exe mep_holder_a -i=MEP    -n=MepHold_0 -p=333
rem start %GAUDIONLINEROOT%\win32_vc71_dbg\Gaudi.exe GaudiOnline mep2event_prod -n=evtprod_0 -p=333
sleep 1
rem start %GAUDIONLINEROOT%\win32_vc71_dbg\Gaudi.exe GaudiOnline mep_producer -n=prod_0 -p=333 -s=500

rem start %GAUDIONLINEROOT%\win32_vc71_dbg\Gaudi.exe MDF GaudiTask ..\options\ReadMBM.opts
