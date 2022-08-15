@echo off
pushd %~dp0\..\
call dev.bat release
popd
PAUSE
