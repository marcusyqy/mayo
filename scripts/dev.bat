@echo off
pushd %~dp0\..\
echo "deleting compile_commands.json"
del /F /Q compile_commands.json
echo "deleted compile_commands.json"
echo "linking compile_commands.json to %1 build"
call cmd /c mklink compile_commands.json .\compile_commands\%1.json
popd
