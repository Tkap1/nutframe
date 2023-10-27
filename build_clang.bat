
@echo off

cls

if not exist build\NUL mkdir build

SETLOCAL ENABLEDELAYEDEXPANSION

@REM 0 for win32, 1 for SDL
set platform=0
set client_file=..\examples\golf\golf.cpp
set exe_name=DigHard

set build_dll=1
set comp=-std=c++20 -FC -D_CRT_SECURE_NO_WARNINGS
set comp=!comp! -Wall -Wextra -Wpedantic
set comp=!comp! -Wno-unused-function
@REM set comp=!comp! -Wno-unused-parameter
set comp=!comp! -Wno-language-extension-token
set comp=!comp! -Wno-char-subscripts
@REM set comp=!comp! -fsanitize=address
set linker=-INCREMENTAL:NO

if !platform!==0 (
	set platform_file=..\src\win32_platform.cpp
)

if !platform!==1 (
	set platform_file=..\src\sdl_platform.cpp
	set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\sdl"
	set comp=!comp! -Dm_sdl
	set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\sdl"
	set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\sdl\SDL2"
	set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\SDL_mixer\include"
	set comp=!comp! -l..\SDL2
	set comp=!comp! -l..\SDL2_mixer
	set comp=!comp! -lWinmm -lUser32 -lGdi32 -lShell32 -lSetupapi -lVersion -lOle32 -lImm32 -lAdvapi32 -lOleAut32
	set build_dll=0
)

set debug=2
if !debug!==0 (
	set comp=!comp! -O3
	set linker=!linker! -SUBSYSTEM:windows
	set build_dll=0
	rc.exe /nologo icon.rc
)
if !debug!==1 (
	set comp=!comp! -O3 -Dm_debug
)
if !debug!==2 (
	set comp=!comp! -g -O0 -Dm_debug
)

if !build_dll!==1 (
	set comp=!comp! -Dm_build_dll
)

pushd build

	if !build_dll!==0 (
		clang !platform_file! !client_file! !comp! !linker! > temp_compiler_output.txt
		if NOT !ErrorLevel! == 0 (
			type temp_compiler_output.txt
			popd
			goto fail
		)
		type temp_compiler_output.txt
	) else (

		@REM @Note(tkap, 13/10/2023): we use temp_dll_name instead of !exe_name! because otherwise we will get
		@REM a PDB file named !exe_name!.pdb and then when we build the platform layer, we will override said PDB
		@REM because the names are the same. If clang allowed us to specify the PDB name, this wouldn't be necessary,
		@REM but it doesn't.
		clang !client_file! -shared -o temp_dll_name.dll !comp! !linker! > temp_compiler_output.txt
		if NOT !ErrorLevel! == 0 (
			type temp_compiler_output.txt
			popd
			goto fail
		)
		move /y temp_dll_name.dll !exe_name!.dll > NUL
		type temp_compiler_output.txt

		tasklist /fi "ImageName eq !exe_name!.exe" /fo csv 2>NUL | find /I "!exe_name!.exe">NUL
		if NOT !ERRORLEVEL!==0 (
			clang !platform_file! -o !exe_name!.exe !comp! !linker! > temp_compiler_output.txt
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