
Do this once if you have never setup emscripten
git clone https://github.com/emscripten-core/emsdk.git
cd emsdk
git pull

After that, every time you open a new console, you can setup the compiler like this:

call "YOUR_PATH\emsdk\emsdk.bat"
call "YOUR_PATH\emsdk\emsdk_env.bat"

To compile, run build_em.bat
