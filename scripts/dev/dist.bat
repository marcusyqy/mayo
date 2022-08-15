@echo off
pushd %~dp0\..\
call dev.bat dist
popd
PAUSE
