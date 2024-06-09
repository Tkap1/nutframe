
@echo off

cls

if NOT defined VSCMD_ARG_TGT_ARCH (
	@REM Replace with your path
	call "C:\Program Files\Microsoft Visual Studio\2022\Community\VC\Auxiliary\Build\vcvarsall.bat" amd64
)

if not exist build\NUL mkdir build

SETLOCAL ENABLEDELAYEDEXPANSION

@REM 0 for win32, 1 for SDL
set platform=0
set client_file=..\examples\speedjam5\platformer.cpp
set exe_name=DigHard

set build_dll=1
set comp=-nologo -std:c++20 -Zc:strictStrings- -W4 -Wall -FC -Gm- -GR- -EHa- -wd 4324 -wd 4127 -wd 4505 -D_CRT_SECURE_NO_WARNINGS
set linker=-INCREMENTAL:NO
set comp=!comp! -wd4201 -wd4100 -wd4464 -wd4820 -wd5219 -wd4365 -wd4514 -wd5045 -wd5220 -wd5204 -wd4191 -wd4577 -wd4062 -wd4686 -wd4711 -wd4710
@REM set comp=!comp! -fsanitize=address

if !platform!==0 (
	set platform_file=..\src\win32_platform.cpp
	set comp=!comp! -Dm_windows
)

if !platform!==1 (
	set platform_file=..\src\sdl_platform.cpp
	set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\sdl"
	set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\sdl\SDL2"
	set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\SDL_mixer\include"
	set comp=!comp! -Dm_sdl
	set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\engine_thing\src\external"
	set linker=!linker! ..\SDL2.lib
	set linker=!linker! ..\SDL2_mixer.lib
	set linker=!linker! Winmm.lib User32.lib Gdi32.lib Shell32.lib Setupapi.lib Version.lib Ole32.lib Imm32.lib Advapi32.lib OleAut32.lib
	set linker=!linker! -IGNORE:4099
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
	set comp=!comp! -O2 -Dm_debug
	if !platform!==0 (
		set comp=!comp! -MTd
	) else (
		set comp=!comp! -MT
	)
	set linker=!linker! -DYNAMICBASE:NO
)
if !debug!==2 (
	set comp=!comp! -Od -Dm_debug -Zi
	if !platform!==0 (
		set comp=!comp! -MTd
	) else (
		set comp=!comp! -MT
	)
	set linker=!linker! -DYNAMICBASE:NO
)

if !build_dll!==1 (
	set comp=!comp! -Dm_build_dll
)

pushd build

	if !build_dll!==0 (
		cl !platform_file! !client_file! -Fe!exe_name!.exe !comp! -link !linker! -PDB:platform_client.pdb ..\icon.res > temp_compiler_output.txt
		if NOT !ErrorLevel! == 0 (
			type temp_compiler_output.txt
			popd
			goto fail
		)
		type temp_compiler_output.txt
	) else (
		cl !client_file! -LD -Fe!exe_name!.dll !comp! -link !linker! -PDB:client.pdb > temp_compiler_output.txt
		if NOT !ErrorLevel! == 0 (
			type temp_compiler_output.txt
			popd
			goto fail
		)
		type temp_compiler_output.txt

		tasklist /fi "ImageName eq !exe_name!.exe" /fo csv 2>NUL | find /I "!exe_name!.exe">NUL
		if NOT !ERRORLEVEL!==0 (
			cl !platform_file! -Fe!exe_name!.exe !comp! -link !linker! -PDB:platform_client.pdb > temp_compiler_output.txt
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
copy build\!exe_name!.exe !exe_name!.exe > NUL
goto end

:fail

:end
copy build\temp_compiler_output.txt compiler_output.txt > NUL

:foo