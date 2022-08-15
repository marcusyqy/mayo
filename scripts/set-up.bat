@echo off
pushd %~dp0
call .\win-gen-proj.bat
call .\dev.bat release
popd

