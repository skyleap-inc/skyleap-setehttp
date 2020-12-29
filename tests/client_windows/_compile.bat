@echo off
call rmdir build /S /Q
call mkdir build
call g++ ..\..\lib\SeteHTTP.cpp testclient.cpp -o .\build\testclient.exe -lws2_32 -lwsock32
echo Compiled to .\build
pause