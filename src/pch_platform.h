
#pragma comment(lib, "opengl32.lib")

#ifndef m_sdl

#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "Ole32.lib")

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <gl/GL.h>
#include "external/glcorearb.h"
#include "external/wglext.h"
#include <xinput.h>
#include <xaudio2.h>
#endif // m_sdl
#include <stdio.h>
#include <string.h>
#include <math.h>
#include <stdint.h>

#ifdef m_sdl
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "GL\glew.h"
#endif // m_sdl

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#include "emscripten/html5.h"
#endif // __EMSCRIPTEN__

#include "types.h"
#include "rng.h"
#include "math.h"
#include "utils.h"
