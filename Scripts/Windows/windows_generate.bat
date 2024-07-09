@echo off
pushd %~dp0\..\..\
call Thirdparty\premake\bin\windows\premake5.exe vs2022
popd
PAUSE