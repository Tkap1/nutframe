@echo off

cls

pushd build
	call emcc ..\src\sdl_platform.cpp ..\src\client.cpp -Dm_sdl -I"C:\Users\34687\Desktop\Dev\C\sdl" -Dm_app -std=c++20 -Wno-writable-strings -sUSE_SDL=2 -sFULL_ES3 -o hello.html
popd
