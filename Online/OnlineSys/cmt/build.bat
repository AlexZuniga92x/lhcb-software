if exist %CMTINSTALLAREA% rd /q/s %CMTINSTALLAREA%
cmt broadcast if exist ..\Visual rd /q/s ..\Visual
cmt broadcast if exist ..\%CMTCONFIG% rd /q/s ..\%CMTCONFIG%
cmt broadcast cmt config
cmt broadcast cmt build vsnet
cmt broadcast nmake /f nmake
