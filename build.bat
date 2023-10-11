
@echo off

cls

if NOT defined VSCMD_ARG_TGT_ARCH (
	@REM Replace with your path
	call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
)

if not exist build\NUL mkdir build

SETLOCAL ENABLEDELAYEDEXPANSION

@REM 0 for win32, 1 for SDL
set platform=1
set client_file=..\examples\snake.cpp

set build_dll=1
set comp=-nologo -std:c++20 -Zc:strictStrings- -W4 -FC -Gm- -GR- -EHa- -wd 4324 -wd 4127 -wd 4505 -D_CRT_SECURE_NO_WARNINGS -Dm_app
set linker=-INCREMENTAL:NO
set comp=!comp! -wd4201
set comp=!comp! -wd4100

if !platform!==0 (
	set platform_file=..\src\win32_platform.cpp
)

if !platform!==1 (
	set platform_file=..\src\sdl_platform.cpp
	set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\sdl"
	set comp=!comp! -Dm_sdl
	set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\glew\include"
	set linker=!linker! "C:\Users\34687\Desktop\Dev\C\sdl\VisualC\x64\Release\SDL2.lib"
	set linker=!linker! "C:\Users\34687\Desktop\Dev\C\glew\lib\Release\x64\glew32.lib"
	set build_dll=0
)

set debug=2
if !debug!==0 (
	set comp=!comp! -O2 -MT
	set linker=!linker! -SUBSYSTEM:windows
	set build_dll=0
	rc.exe /nologo icon.rc
)
if !debug!==1 (
	set comp=!comp! -O2 -Dm_debug -MTd
)
if !debug!==2 (
	set comp=!comp! -Od -Dm_debug -Zi -MTd
)

if !build_dll!==1 (
	set comp=!comp! -Dm_build_dll
)

pushd build

	if "%1%"=="pch" (
		cl /Ycpch_client.h ..\src\pch_client.cpp !comp! /c
		cl /Ycpch_platform.h ..\src\pch_platform.cpp !comp! /c
	)

	if !build_dll!==0 (
		cl !platform_file! !client_file! -FeDigHard.exe !comp! -link !linker! -PDB:platform_client.pdb ..\icon.res > temp_compiler_output.txt
		if NOT !ErrorLevel! == 0 (
			type temp_compiler_output.txt
			popd
			goto fail
		)
		type temp_compiler_output.txt
	) else (
		cl !client_file! /Yupch_client.h -LD -FeDigHard.dll !comp! -link !linker! pch_client.obj -PDB:client.pdb > temp_compiler_output.txt
		if NOT !ErrorLevel! == 0 (
			type temp_compiler_output.txt
			popd
			goto fail
		)
		type temp_compiler_output.txt

		tasklist /fi "ImageName eq DigHard.exe" /fo csv 2>NUL | find /I "DigHard.exe">NUL
		if NOT !ERRORLEVEL!==0 (
			cl !platform_file! /Yupch_platform.h -FeDigHard.exe !comp! -link !linker! pch_platform.obj -PDB:platform_client.pdb > temp_compiler_output.txt
			if NOT !ErrorLevel! == 0 (
				type temp_compiler_output.txt
				popd
				goto fail
			)
			type temp_compiler_output.txt
		)
	)

popd

if !errorlevel!==0 goto success
goto fail

:success
copy build\DigHard.exe DigHard.exe > NUL
goto end

:fail

:end
copy build\temp_compiler_output.txt compiler_output.txt > NUL

:foo