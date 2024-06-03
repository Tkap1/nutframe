
@echo off

cls

set comp=-wd4505
set comp=%comp% -D_CRT_SECURE_NO_WARNINGS
pushd build
	cl -nologo -Od -Zi ..\src\serialize.cpp -Fe:serialize.exe -W4 -std:c++20 -I..\..\my_libs -Zc:strictStrings- %comp%
popd