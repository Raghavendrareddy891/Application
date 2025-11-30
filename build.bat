@echo off
set MSYS2_SHELL=C:\msys64\msys2_shell.cmd
set SCRIPT=/d/Application/build_client.sh

"%MSYS2_SHELL%" -mingw64 -full-path -here -c "bash %SCRIPT%"

pause
