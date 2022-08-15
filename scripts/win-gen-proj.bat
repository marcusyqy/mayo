@echo off
pushd %~dp0\..\
call deps\premake\premake5.exe vs2022
call deps\premake\premake5.exe export-compile-commands
popd
PAUSE
