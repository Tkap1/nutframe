@echo off

cls

@REM -sFULL_ES3
pushd build
	call emcc ..\src\sdl_platform.cpp ..\src\client.cpp -gsource-map -Dm_sdl -Dm_debug -sASSERTIONS=1 -sFULL_ES3 --shell-file ..\shell.html -fsanitize=address -I"C:\Users\34687\Desktop\Dev\C\sdl" -Dm_app -std=c++20 -Wno-writable-strings -sUSE_SDL=2 -sUSE_WEBGL2=1 -sALLOW_MEMORY_GROWTH -o hello.html -I"C:\Users\34687\Desktop\Dev\C\emsdk\upstream\emscripten\cache\sysroot\include" --preload-file ../shaders/vertex.vertex@shaders/vertex.vertex --preload-file ../shaders/fragment.fragment@shaders/fragment.fragment --preload-file ../assets/noise.png@assets/noise.png
popd

copy build\hello.html hello.html > NUL
copy build\hello.js hello.js > NUL
copy build\hello.wasm hello.wasm > NUL
copy build\hello.data hello.data > NUL
copy build\hello.wasm.map hello.wasm.map > NUL