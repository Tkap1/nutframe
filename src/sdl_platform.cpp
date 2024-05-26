
#pragma comment(lib, "opengl32.lib")

// #if !defined(m_debug) && defined(_WIN32)
// #define WIN32_LEAN_AND_MEAN
// #define NOMINMAX
// #include <windows.h>
// #endif

#pragma warning(push, 0)
#define SDL_MAIN_HANDLED
#include "SDL2/SDL.h"
#include "SDL_mixer.h"

#ifdef __EMSCRIPTEN__
#include "emscripten.h"
#include "emscripten/html5.h"
#endif // __EMSCRIPTEN__

#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wunused-value"
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT
#include "external/stb_image.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_assert
#include "external/stb_truetype.h"

#pragma clang diagnostic pop
#pragma warning(pop)

#include "resource.h"

static void set_cursor_pos(int x, int y);
#include "platform_shared.h"

static b8 play_sound(s_sound* sound);
static void set_vsync(b8 val);
void printProgramLog( GLuint program );
static void do_one_frame();
static u32 get_random_seed();
static int sdl_key_to_windows_key(int key);
static f64 get_seconds();
static s_sound* load_sound(s_platform_data* platform_data, const char* path, s_lin_arena* arena);
static Mix_Chunk* load_sound_from_file(const char* path);
static Mix_Chunk* load_sound_from_data(u8* data, int data_size);
static void set_window_size(int width, int height);
static void center_window();

static u64 g_cycle_frequency;
static u64 g_start_cycles;
static SDL_GLContext g_gl_context;
static SDL_Window* g_window = NULL;
static f64 g_start_of_frame_seconds = 0;

static void* g_game_memory;
static s_game_renderer* g_game_renderer;
static s_sarray<Mix_Chunk*, 16> g_sdl_audio;
s_lin_arena g_game_frame_arena = {};

#ifdef __EMSCRIPTEN__
static int g_previous_window_width = 0;
static int g_previous_window_height = 0;
#endif // __EMSCRIPTEN__

#if defined(m_debug) || !defined(_WIN32)
int main(int argc, char** argv)
#else
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
#endif
{
	SDL_SetMainReady();
	if(SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO) < 0) {
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	if(Mix_OpenAudioDevice(44100, MIX_DEFAULT_FORMAT, 2, 4096, NULL, 0) == -1) {
		printf("Failed to initialize SDL audio\n");
		return 1;
	}
	Mix_Volume(-1, floorfi(MIX_MAX_VOLUME * 0.1f));

	#if defined(m_debug) || !defined(_WIN32)
	if(argc > 1 && strcmp(argv[1], "embed") == 0) {
		g_do_embed = true;
	}
	#else
	if(__argc > 1 && strcmp(__argv[1], "embed") == 0) {
		g_do_embed = true;
	}
	#endif

	g_platform_data.get_random_seed = get_random_seed;
	g_platform_data.load_sound = load_sound;
	g_platform_data.play_sound = play_sound;
	g_platform_data.read_file = read_file;
	g_platform_data.write_file = write_file;
	g_platform_data.reset_ui = reset_ui;
	g_platform_data.ui_button = ui_button;
	g_platform_data.ui_checkbox = ui_checkbox;
	g_platform_data.set_window_size = set_window_size;
	g_platform_data.set_base_resolution = set_base_resolution;


	g_window = SDL_CreateWindow(
		"DigHard", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		256, 256, SDL_WINDOW_HIDDEN | SDL_WINDOW_OPENGL
	);

	init_game(&g_platform_data);
	if(g_base_res.x <= 0 && g_base_res.y <= 0) {
		log_error("Invalid base resolution");
		exit(1);
	}
	g_platform_data.window_width = (int)g_base_res.x;
	g_platform_data.window_height = (int)g_base_res.y;

	if(!g_platform_data.game_called_set_window_size) {
		set_window_size(g_platform_data.window_width, g_platform_data.window_height);
	}
	center_window();
	SDL_ShowWindow(g_window);

	#ifdef __EMSCRIPTEN__
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_ES);
	#else
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
	SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
	#endif

	g_start_cycles = SDL_GetPerformanceCounter();
	g_cycle_frequency = SDL_GetPerformanceFrequency();

	if(g_window == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	g_gl_context = SDL_GL_CreateContext(g_window);
	if(g_gl_context == NULL)
	{
		printf("OpenGL context could not be created! SDL Error: %s\n", SDL_GetError());
		return 1;
	}

	#define X(type, name) name = (type)SDL_GL_GetProcAddress(#name);
		m_gl_funcs
	#undef X

	s_lin_arena platform_frame_arena = {};

	#ifdef m_debug
	unreferenced(argc);
	unreferenced(argv);
	#endif

	// g_platform_funcs.play_sound = play_sound;
	// g_platform_funcs.show_cursor = ShowCursor;
	// g_platform_funcs.cycle_between_available_resolutions = cycle_between_available_resolutions;

	{
		s_lin_arena all = {};
		all.capacity = 100 * c_mb;

		// @Note(tkap, 26/06/2023): We expect this memory to be zero'd
		all.memory = malloc(all.capacity);
		memset(all.memory, 0, all.capacity);

		g_game_renderer = (s_game_renderer*)la_get(&all, sizeof(s_game_renderer));

		g_game_memory = la_get(&all, c_game_memory);
		platform_frame_arena = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		g_game_frame_arena = make_lin_arena_from_memory(10 * c_mb, la_get(&all, 10 * c_mb));
		g_platform_data.frame_arena = &g_game_frame_arena;

		g_game_renderer->arenas[0] = make_lin_arena_from_memory(10 * c_mb, la_get(&all, 10 * c_mb));
		g_game_renderer->arenas[1] = make_lin_arena_from_memory(10 * c_mb, la_get(&all, 10 * c_mb));
		g_game_renderer->transform_arenas[0] = make_lin_arena_from_memory(10 * c_mb, la_get(&all, 10 * c_mb));
		g_game_renderer->transform_arenas[1] = make_lin_arena_from_memory(10 * c_mb, la_get(&all, 10 * c_mb));
		g_game_renderer->textures.add({});
		after_loading_texture(g_game_renderer);
	}

	g_game_renderer->set_vsync = set_vsync;
	g_game_renderer->load_texture = load_texture;
	g_game_renderer->load_font = load_font;
	g_game_renderer->make_framebuffer = make_framebuffer;
	g_game_renderer->set_shader_float = set_shader_float;
	g_game_renderer->set_shader_v2 = set_shader_v2;
	init_gl(&g_platform_renderer, g_game_renderer, &platform_frame_arena);

	b8 running = true;
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

static void set_vsync(b8 val)
{
	SDL_GL_SetSwapInterval(val ? 1 : 0);
}

#ifdef __EMSCRIPTEN__
EM_JS(int, browser_get_width, (), {
    const { width, height } = canvas.getBoundingClientRect();
    return width;
});

EM_JS(int, browser_get_height, (), {
    const { width, height } = canvas.getBoundingClientRect();
    return height;
});
#endif // __EMSCRIPTEN__

static void do_one_frame()
{
	f64 seconds = get_seconds();
	f64 time_passed = seconds - g_start_of_frame_seconds;
	g_platform_data.frame_time = time_passed;
	g_game_renderer->total_time += time_passed;
	g_start_of_frame_seconds = seconds;

	SDL_Event e;
	while(SDL_PollEvent(&e) != 0)
	{
		switch(e.type) {
			case SDL_QUIT: {
			exit(0);
			} break;

			case SDL_WINDOWEVENT: {
				if(e.window.event == SDL_WINDOWEVENT_SIZE_CHANGED)
				{
					int width = e.window.data1;
					int height = e.window.data2;
					g_platform_data.window_width = width;
					g_platform_data.window_height = height;
					g_platform_data.window_resized = true;
				}
			} break;

			case SDL_KEYDOWN:
			case SDL_KEYUP: {
				int key = sdl_key_to_windows_key(e.key.keysym.sym);
				b8 is_repeat = e.key.repeat;
				if(key == -1) { break; }
				b8 is_down = e.type == SDL_KEYDOWN;
				s_stored_input si = {};
				si.key = key;
				si.is_down = is_down;
				if(!is_repeat) {
					apply_event_to_input(&g_platform_data.logic_input, si);
					apply_event_to_input(&g_platform_data.render_input, si);
				}

				// @Note(tkap, 11/11/2023): SDL does not give us a text input event for backspace, so let's hack it
				if(key == c_key_backspace && is_down) {
					g_platform_data.logic_input.char_events.add('\b');
					g_platform_data.render_input.char_events.add('\b');
				}

				#ifdef m_debug
				if(g_platform_data.recording_input && key != c_key_f10 && !is_repeat) {
					s_foo ri = {};
					ri.input = si;
					ri.update_count = g_platform_data.update_count;
					g_platform_data.recorded_input.keys.add(ri);
				}
				#endif // m_debug

			} break;

			case SDL_MOUSEBUTTONDOWN:
			case SDL_MOUSEBUTTONUP:
			{
				int key = sdl_key_to_windows_key(e.button.button);
				b8 is_down = e.type == SDL_MOUSEBUTTONDOWN;
				s_stored_input si = {};
				si.key = key;
				si.is_down = is_down;
				apply_event_to_input(&g_platform_data.logic_input, si);
				apply_event_to_input(&g_platform_data.render_input, si);

				#ifdef m_debug
				if(g_platform_data.recording_input && key != c_key_f10) {
					s_foo ri = {};
					ri.input = si;
					ri.update_count = g_platform_data.update_count;
					g_platform_data.recorded_input.keys.add(ri);
				}
				#endif // m_debug

			} break;

			case SDL_TEXTINPUT: {
				char c = e.text.text[0];
				g_platform_data.logic_input.char_events.add((char)c);
				g_platform_data.render_input.char_events.add((char)c);
			} break;
		}
	}

	// g_platform_data.quit_after_this_frame = !result;

	{
		int x, y;
		SDL_GetMouseState(&x, &y);
		g_platform_data.mouse.x = (float)x;
		g_platform_data.mouse.y = (float)y;
	}
	// g_platform_data.is_window_active = GetActiveWindow() == g_window.handle;

	#ifdef __EMSCRIPTEN__
	int width = browser_get_width();
	int height = browser_get_height();
	g_platform_data.window_width = width;
	g_platform_data.window_height = height;
	if(g_previous_window_width != width || g_previous_window_height != height) {
		set_window_size(width, height);
		g_platform_data.window_resized = true;
	}
	g_previous_window_width = width;
	g_previous_window_height = width;
	#endif // __EMSCRIPTEN__


	do_game_layer(g_game_renderer, g_game_memory);

	gl_render(&g_platform_renderer, g_game_renderer);

	SDL_GL_SwapWindow(g_window);
	// return result;
}

static u32 get_random_seed() {
	return (u32)SDL_GetPerformanceCounter();
}

static int sdl_key_to_windows_key(int key) {

	breakable_block {
		if(key >= SDLK_a && key <= SDLK_z) {
			key -= 'a' - 'A';
			break;
		}

		if(key >= SDLK_F1 && key <= SDLK_F12) {
			key = key - SDLK_F1 + c_key_f1;
			break;
		}

		struct s_key_map
		{
			int sdl;
			int win;
		};

		constexpr s_key_map map[] = {
			{.sdl = SDLK_UP, .win = c_key_up},
			{.sdl = SDLK_DOWN, .win = c_key_down},
			{.sdl = SDLK_LEFT, .win = c_key_left},
			{.sdl = SDLK_RIGHT, .win = c_key_right},
			{.sdl = SDLK_LALT, .win = c_key_left_alt},
			{.sdl = SDLK_RALT, .win = c_key_right_alt},
			{.sdl = SDLK_LCTRL, .win = c_key_left_ctrl},
			{.sdl = SDLK_RCTRL, .win = c_key_right_ctrl},
			{.sdl = SDLK_LSHIFT, .win = c_key_left_shift},
			{.sdl = SDLK_RSHIFT, .win = c_key_right_shift},
			{.sdl = SDLK_ESCAPE, .win = c_key_escape},
			{.sdl = SDLK_RETURN, .win = c_key_enter},
			{.sdl = SDLK_SPACE, .win = c_key_space},
			{.sdl = SDLK_KP_PLUS, .win = c_key_add},
			{.sdl = SDLK_KP_MINUS, .win = c_key_subtract},
			{.sdl = SDLK_BACKSPACE, .win = c_key_backspace},
			{.sdl = SDL_BUTTON_LEFT, .win = c_left_mouse},
			{.sdl = SDL_BUTTON_RIGHT, .win = c_right_mouse},
			{.sdl = SDL_BUTTON_MIDDLE, .win = c_middle_mouse},
		};

		b8 handled = false;
		for(int key_i = 0; key_i < (int)array_count(map); key_i++)
		{
			if(map[key_i].sdl == key) {
				key = map[key_i].win;
				handled = true;
				break;
			}
		}
		if(handled) { break; }

		printf("Untranslated key %i\n", key);
		key = -1;
	}
	return key;
}

static f64 get_seconds()
{
	u64 now =	SDL_GetPerformanceCounter();
	return (now - g_start_cycles) / (f64)g_cycle_frequency;
}

static s_sound* load_sound(s_platform_data* platform_data, const char* path, s_lin_arena* arena)
{
	if(g_do_embed) {
		g_to_embed.add(path);
	}

	s_sound sound = {};

	#ifdef m_debug

	Mix_Chunk* chunk = load_sound_from_file(path);
	assert(chunk);

	#else // m_debug

	Mix_Chunk* chunk = load_sound_from_data(embed_data[g_asset_index], embed_sizes[g_asset_index]);
	g_asset_index += 1;

	#endif // m_debug

	g_sdl_audio.add(chunk);
	sound.index = platform_data->sounds.count;
	int index = platform_data->sounds.add(sound);
	return &platform_data->sounds[index];
}

static Mix_Chunk* load_sound_from_file(const char* path)
{
	Mix_Chunk* chunk = Mix_LoadWAV(path);
	assert(chunk);
	return chunk;
}

static Mix_Chunk* load_sound_from_data(u8* data, int data_size)
{
	SDL_RWops* idk = SDL_RWFromMem(data, data_size);
	Mix_Chunk* chunk = Mix_LoadWAV_RW(idk, 1);
	assert(chunk);
	return chunk;
}

static b8 play_sound(s_sound* sound)
{
	Mix_Chunk* chunk = g_sdl_audio[sound->index];
	if(Mix_PlayChannel(-1, chunk, 0) == -1) {
		return false;
	}
	return true;
}

static void set_cursor_pos(int x, int y)
{
	SDL_WarpMouseInWindow(g_window, x, y);
}

static void set_window_size(int width, int height)
{
	g_platform_data.game_called_set_window_size = true;
	SDL_SetWindowSize(g_window, width, height);
}

static void center_window()
{
	SDL_SetWindowPosition(g_window, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED);
}