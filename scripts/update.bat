@echo off
pushd %~dp0\..\
echo "deleting compile_commands.json"
del /F /Q compile_commands.json
echo "linking compile_commands.json to %1 build"
call xcopy /s .\compile_commands\%1.json compile_commands.json*
popd
