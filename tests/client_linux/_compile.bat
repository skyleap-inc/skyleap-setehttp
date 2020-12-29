@echo off
call rmdir build /S /Q
call mkdir build
call g++ ../../lib/SeteHTTP.cpp testclient.cpp -lws2_32 -lwsock32 -o .\build\testclient.exe
echo Compiled to .\build
pause