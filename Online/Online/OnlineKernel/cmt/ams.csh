#/bin/csh
rm /dev/shm/sem.TAN_*
rm /dev/shm/TAN_*
set EXEC=$ONLINEKERNELROOT/$CMTCONFIG/test.exe
set MINITERM='xterm -geometry 80x12 -e '
set MIDITERM='xterm -geometry 100x12 -e '
$MINITERM "setenv PROCESSNAME TANALLOC_0;   $EXEC tan_nameserver -a" &
sleep 1
$MINITERM "setenv PROCESSNAME TANINQUIRE_0; $EXEC tan_nameserver -i" &
sleep 1
$MINITERM "setenv PROCESSNAME READER_0; $EXEC amsc_bounce -n=READER_0" &
sleep 2
$MINITERM "setenv PROCESSNAME READER_0; $EXEC amsc_bounce -l=555 -s=READER_0 -n=SENDER_0" &
$MINITERM "setenv PROCESSNAME READER_0; $EXEC amsc_bounce -l=444 -s=READER_0 -n=SENDER_1" &
$MINITERM "setenv PROCESSNAME READER_0; $EXEC amsc_bounce -l=333 -s=READER_0 -n=SENDER_2" &
sleep 2
xterm -e "setenv PROCESSNAME TANMON_0;      $EXEC tanmon -c" &
