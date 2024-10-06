@echo off

cls

SETLOCAL ENABLEDELAYEDEXPANSION

set comp=
set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\sdl"
set comp=!comp! -I"C:\Users\34687\Desktop\Dev\C\SDL_mixer\include"
set comp=!comp! -I"..\..\libs"
set comp=!comp! -lSDL2_mixer
set comp=!comp! -lwebsocket.js
set comp=!comp! -lidbfs.js
set comp=!comp! -sSTACK_SIZE=1048576
set comp=!comp! --shell-file ../shell.html
set comp=!comp! -sFETCH
set comp=!comp! -Dm_emscripten

set debug=1
if !debug!==0 (
	set comp=!comp! -O3
) else (
	@REM set comp=!comp! -Dm_debug
	set comp=!comp! -O0
	set comp=!comp! -sSAFE_HEAP=0
	set comp=!comp! -sASSERTIONS=0
	set comp=!comp! --preload-file ../shaders/vertex.vertex@shaders/vertex.vertex
	set comp=!comp! --preload-file ../shaders/fragment.fragment@shaders/fragment.fragment
	set comp=!comp! --preload-file ../shaders/basic_3d.vertex@shaders/basic_3d.vertex
	set comp=!comp! --preload-file ../shaders/basic_3d.fragment@shaders/basic_3d.fragment
	set comp=!comp! --preload-file ../shaders/3d_flat.vertex@shaders/3d_flat.vertex
	set comp=!comp! --preload-file ../shaders/3d_flat.fragment@shaders/3d_flat.fragment
	set comp=!comp! --preload-file ../shaders/threshold.fragment@shaders/threshold.fragment
	set comp=!comp! --preload-file ../shaders/blur.fragment@shaders/blur.fragment
	set comp=!comp! --preload-file ../examples/test/noise.png@examples/test/noise.png
	set comp=!comp! --preload-file ../examples/test/placeholder.png@examples/test/placeholder.png
	set comp=!comp! --preload-file ../examples/test/drone000.png@examples/test/drone000.png
	set comp=!comp! --preload-file ../examples/test/drone006.png@examples/test/drone006.png
	set comp=!comp! --preload-file ../examples/test/drone012.png@examples/test/drone012.png
	set comp=!comp! --preload-file ../examples/test/drone018.png@examples/test/drone018.png
	set comp=!comp! --preload-file ../examples/test/drone024.png@examples/test/drone024.png
	set comp=!comp! --preload-file ../examples/test/base.png@examples/test/base.png
	set comp=!comp! --preload-file ../examples/test/ant.png@examples/test/ant.png
	set comp=!comp! --preload-file ../assets/consola.ttf@assets/consola.ttf
	set comp=!comp! --preload-file ../assets/checkmark.png@assets/checkmark.png
	set comp=!comp! --preload-file ../platform_map.map@platform_map.map
	set comp=!comp! --preload-file ../map2.map@map2.map
	set comp=!comp! --preload-file ../aqtun.map@aqtun.map
	set comp=!comp! --preload-file ../zanarias.map@zanarias.map
	set comp=!comp! --preload-file ../azenris.map@azenris.map
	set comp=!comp! --preload-file ../platforms.map@platforms.map
	set comp=!comp! -gsource-map
	@REM set comp=!comp! -fsanitize=address
)

@REM -sFULL_ES3
pushd build
	call emcc ..\src\sdl_platform.cpp ..\examples\test\test.cpp -Dm_sdl -sFULL_ES3 !comp! -std=c++20 -Wno-writable-strings -sUSE_SDL=2 -sUSE_WEBGL2=1 -sALLOW_MEMORY_GROWTH -o index.html -I"C:\Users\34687\Desktop\Dev\C\emsdk\upstream\emscripten\cache\sysroot\include"
popd

copy build\index.html index.html > NUL
copy build\index.js index.js > NUL
copy build\index.wasm index.wasm > NUL
copy build\index.data index.data > NUL
copy build\index.wasm.map index.wasm.map > NUL