
#include "pch_platform.h"

#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT assert
#include "external/stb_image.h"
#pragma warning(pop)

#include "resource.h"
#include "memory.h"
#include "config.h"
#include "shader_shared.h"
#include "bucket.h"
#include "common.h"
#include "platform_shared.h"
#include "sdl_platform.h"

global s_window g_window;
global s_input g_input;
global s_input g_logic_input;
global u64 g_cycle_frequency;
global u64 g_start_cycles;
global s_platform_data g_platform_data = zero;
global SDL_GLContext gContext;
global SDL_Window* gWindow = null;
global GLuint gProgramID = 0;
global GLint gVertexPos2DLocation = -1;
global GLuint gVBO = 0;
global GLuint gIBO = 0;
global f64 time_passed;

global s_platform_funcs g_platform_funcs;
global void* g_game_memory;
global s_platform_renderer g_platform_renderer;
global s_game_renderer* g_game_renderer;

#include "memory.cpp"
#include "platform_shared.cpp"
#include "file.cpp"
#include "bucket.cpp"
#include "common.cpp"

int main(int argc, char** argv)
{
	SDL_SetMainReady();
	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	#ifdef __EMSCRIPTEN__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	#else
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );
	#endif


	gWindow = SDL_CreateWindow(
		"SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		c_resolutions[c_base_resolution_index].x, c_resolutions[c_base_resolution_index].y, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE
	);
	if(gWindow == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	gContext = SDL_GL_CreateContext( gWindow );
	if( gContext == NULL )
	{
		printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
		return 1;
	}

	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
		return 1;
	}

	s_lin_arena platform_frame_arena = zero;
	s_lin_arena game_frame_arena = zero;

	#ifdef m_debug
	unreferenced(argc);
	unreferenced(argv);
	#endif

	// g_platform_funcs.play_sound = play_sound;
	// g_platform_funcs.show_cursor = ShowCursor;
	// g_platform_funcs.cycle_between_available_resolutions = cycle_between_available_resolutions;

	{
		s_lin_arena all = zero;
		all.capacity = 10 * c_mb;

		// @Note(tkap, 26/06/2023): We expect this memory to be zero'd
		all.memory = malloc(all.capacity);
		memset(all.memory, 0, all.capacity);

		g_game_renderer = (s_game_renderer*)la_get(&all, sizeof(s_game_renderer));

		g_game_memory = la_get(&all, c_game_memory);
		platform_frame_arena = make_lin_arena_from_memory(1 * c_mb, la_get(&all, 1 * c_mb));
		game_frame_arena = make_lin_arena_from_memory(1 * c_mb, la_get(&all, 1 * c_mb));
		g_platform_data.frame_arena = &game_frame_arena;

		g_game_renderer->arenas[0] = make_lin_arena_from_memory(1 * c_mb, la_get(&all, 1 * c_mb));
		g_game_renderer->arenas[1] = make_lin_arena_from_memory(1 * c_mb, la_get(&all, 1 * c_mb));
		g_game_renderer->transform_arenas[0] = make_lin_arena_from_memory(1 * c_mb, la_get(&all, 1 * c_mb));
		g_game_renderer->transform_arenas[1] = make_lin_arena_from_memory(1 * c_mb, la_get(&all, 1 * c_mb));
		g_game_renderer->textures.add(zero);
		after_loading_texture(g_game_renderer);
	}

	g_game_renderer->set_vsync = set_vsync;
	g_game_renderer->load_texture = load_texture;
	init_gl(&g_platform_renderer, &platform_frame_arena);

	b8 running = true;
	f64 time_passed = 0;
	g_platform_data.recompiled = true;

	#ifdef __EMSCRIPTEN__
	// emscripten_request_animation_frame_loop(do_one_frame, &foo);
	emscripten_set_main_loop(do_one_frame, -1, 0);
	#else

	while(running)
	{
		do_one_frame();
	}
	#endif

	return 0;

}

func void set_vsync(b8 val)
{
	SDL_GL_SetSwapInterval(val ? 1 : 0);
}

func void do_one_frame()
{

	f64 start_of_frame_ms = SDL_GetTicks();
	// bool result = true;

	SDL_Event e;
	while(SDL_PollEvent(&e) != 0)
	{
		if(e.type == SDL_QUIT)
		{
			// result = false;
		}
		else if(e.type == SDL_WINDOWEVENT)
		{
			if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
			{
				int width = e.window.data1;
				int height = e.window.data2;
				g_window.width = width;
				g_window.height = height;
				gl(glViewport(0, 0, width, height));
			}
			// result = false;
		}
	}

	g_platform_data.input = &g_input;
	g_platform_data.logic_input = &g_logic_input;
	// g_platform_data.quit_after_this_frame = !result;
	g_platform_data.window_width = g_window.width;
	g_platform_data.window_height = g_window.height;
	g_platform_data.time_passed = time_passed;

	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		g_platform_data.mouse.x = (float)x;
		g_platform_data.mouse.y = (float)y;
	}
	// g_platform_data.is_window_active = GetActiveWindow() == g_window.handle;

	update_game(&g_platform_data, g_platform_funcs, g_game_memory, g_game_renderer);
	g_platform_data.recompiled = false;

	gl_render(&g_platform_renderer, g_game_renderer);

	SDL_GL_SwapWindow(gWindow);

	time_passed = (SDL_GetTicks() - start_of_frame_ms) / 1000.0;
	g_game_renderer->total_time += time_passed;
	// return result;
}