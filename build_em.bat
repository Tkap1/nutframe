@echo off

cls

SETLOCAL ENABLEDELAYEDEXPANSION

set comp=
set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\sdl"
set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\SDL_mixer\include"
set comp=!comp! -lSDL2_mixer
set comp=!comp! -sSTACK_SIZE=1048576

set debug=1
if !debug!==0 (
	@REM a
) else (
	set comp=!comp! -Dm_debug
	set comp=!comp! -sSAFE_HEAP=1
	set comp=!comp! -sASSERTIONS=1
	set comp=!comp! --preload-file ../shaders/vertex.vertex@shaders/vertex.vertex
	set comp=!comp! --preload-file ../shaders/fragment.fragment@shaders/fragment.fragment
	set comp=!comp! --preload-file ../examples/snake/noise.png@examples/snake/noise.png
	set comp=!comp! --preload-file ../examples/snake/snake_head.png@examples/snake/snake_head.png
	set comp=!comp! --preload-file ../examples/snake/snake_body.png@examples/snake/snake_body.png
	set comp=!comp! --preload-file ../examples/snake/snake_tail.png@examples/snake/snake_tail.png
	set comp=!comp! --preload-file ../examples/snake/apple.png@examples/snake/apple.png
	set comp=!comp! --preload-file ../examples/snake/noise.png@examples/snake/noise.png
	set comp=!comp! --preload-file ../examples/snake/eat_apple.wav@examples/snake/eat_apple.wav
	set comp=!comp! --preload-file ../assets/consola.ttf@assets/consola.ttf
	set comp=!comp! --preload-file ../assets/checkmark.png@assets/checkmark.png
	@REM set comp=!comp! -fsanitize=address
)


@REM -sFULL_ES3
pushd build
	call emcc ..\src\sdl_platform.cpp ..\examples\snake\snake.cpp -gsource-map -Dm_sdl -sFULL_ES3 !comp! -std=c++20 -Wno-writable-strings -sUSE_SDL=2 -sUSE_WEBGL2=1 -sALLOW_MEMORY_GROWTH -o hello.html -I"C:\Users\34687\Desktop\Dev\C\emsdk\upstream\emscripten\cache\sysroot\include"
popd

copy build\hello.html hello.html > NUL
copy build\hello.js hello.js > NUL
copy build\hello.wasm hello.wasm > NUL
copy build\hello.data hello.data > NUL
copy build\hello.wasm.map hello.wasm.map > NUL