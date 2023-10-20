
#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Ole32.lib")

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <gl/GL.h>
#include "external/glcorearb.h"
#include "external/wglext.h"
#include <xaudio2.h>

#pragma warning(push, 0)
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


#define m_gl_funcs \
X(PFNGLBUFFERDATAPROC, glBufferData) \
X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
X(PFNGLGENBUFFERSPROC, glGenBuffers) \
X(PFNGLBINDBUFFERPROC, glBindBuffer) \
X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer) \
X(PFNGLVERTEXATTRIBIPOINTERPROC, glVertexAttribIPointer) \
X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
X(PFNGLCREATESHADERPROC, glCreateShader) \
X(PFNGLSHADERSOURCEPROC, glShaderSource) \
X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
X(PFNGLATTACHSHADERPROC, glAttachShader) \
X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
X(PFNGLCOMPILESHADERPROC, glCompileShader) \
X(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor) \
X(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced) \
X(PFNGLUNIFORM1FVPROC, glUniform1fv) \
X(PFNGLUNIFORM2FVPROC, glUniform2fv) \
X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
X(PFNGLUSEPROGRAMPROC, glUseProgram) \
X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
X(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) \
X(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) \
X(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) \
X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) \
X(PFNGLACTIVETEXTUREPROC, glActiveTexture) \
X(PFNGLBLENDEQUATIONPROC, glBlendEquation) \
X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
X(PFNGLDELETESHADERPROC, glDeleteShader) \
X(PFNGLUNIFORM1IPROC, glUniform1i) \
X(PFNGLUNIFORM1FPROC, glUniform1f) \
X(PFNGLDETACHSHADERPROC, glDetachShader) \
X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
X(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers)

#define X(type, name) static type name = NULL;
m_gl_funcs
#undef X

struct s_window
{
	HDC dc;
	HWND handle;
	int width;
	int height;
};
static s_window g_window;

#include "platform_shared.h"

struct s_voice : IXAudio2VoiceCallback
{
	IXAudio2SourceVoice* voice;

	volatile int playing;

	void OnStreamEnd() noexcept
	{
		voice->Stop();
		InterlockedExchange((LONG*)&playing, false);
	}

	#pragma warning(push, 0)
	void OnBufferStart(void* pBufferContext) noexcept { unreferenced(pBufferContext); }
	void OnVoiceProcessingPassEnd() noexcept { }
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) noexcept { unreferenced(SamplesRequired); }
	void OnBufferEnd(void* pBufferContext) noexcept { unreferenced(pBufferContext); }
	void OnLoopEnd(void* pBufferContext) noexcept { unreferenced(pBufferContext); }
	void OnVoiceError(void* pBufferContext, HRESULT Error) noexcept { unreferenced(pBufferContext); unreferenced(Error);}
	#pragma warning(pop)
};

static void create_window(int width, int height);
static WPARAM remap_key_if_necessary(WPARAM vk, LPARAM lparam);
static PROC load_gl_func(const char* name);
static b8 init_audio();
static b8 play_sound(s_sound* sound);
static void init_performance();
static f64 get_seconds();
static void set_vsync(b8 val);
static int cycle_between_available_resolutions(int current);
static void center_window();
static s_v2i set_actual_window_size(int width, int height);
static void wide_to_unicode(wchar_t* wide, char* out);
static u32 get_random_seed();
static s_sound* load_sound(s_platform_data* platform_data, const char* path, s_lin_arena* arena);
static s_sound load_sound_from_file(const char* path, s_lin_arena* arena);
static s_sound load_sound_from_data(u8* data);
static b8 thread_safe_set_bool_to_true(volatile int* var);

#ifdef m_debug
static DWORD WINAPI watch_dir(void* arg);
#endif // m_debug

static constexpr int c_num_channels = 2;
static constexpr int c_sample_rate = 44100;
static constexpr int c_max_concurrent_sounds = 32;

static s_input g_input;
static s_input g_logic_input;
static s_voice voice_arr[c_max_concurrent_sounds];
static u64 g_cycle_frequency;
static u64 g_start_cycles;

// @Note(tkap, 11/10/2023): File watch
static constexpr int c_max_files = 16;
static volatile int g_file_write = 0;
static int g_file_read = 0;
static char g_files[c_max_files][MAX_PATH];

static PFNWGLSWAPINTERVALEXTPROC wglSwapIntervalEXT;

#ifdef m_debug
// @TODO(tkap, 18/10/2023): We probably don't want this now that we have the file watcher thing going
static FILETIME last_dll_write_time;
static b8 need_to_reload_dll(const char* path);
static HMODULE load_dll(const char* path);
static void unload_dll(HMODULE dll);
#endif // m_debug

#ifdef m_debug
int main(int argc, char** argv)
#else // m_debug
int APIENTRY WinMain(HINSTANCE hInst, HINSTANCE hInstPrev, PSTR cmdline, int cmdshow)
#endif
{

	#ifdef m_debug
	if(argc > 1 && strcmp(argv[1], "embed") == 0) {
		g_do_embed = true;
	}
	#else // m_debug
	if(__argc > 1 && strcmp(__argv[1], "embed") == 0) {
		g_do_embed = true;
	}
	#endif // m_debug

	s_game_renderer* game_renderer = NULL;
	s_lin_arena platform_frame_arena = {};
	s_lin_arena game_frame_arena = {};

	g_window.width = (int)c_base_res.x;
	g_window.height = (int)c_base_res.y;

	#ifdef m_debug
	unreferenced(argc);
	unreferenced(argv);
	#else // m_debug
	unreferenced(hInst);
	unreferenced(hInstPrev);
	unreferenced(cmdline);
	unreferenced(cmdshow);
	#endif

	create_window(64*12, 64*12);
	if(!init_audio())
	{
		printf("failed to init audio\n");
	}
	init_performance();

	#ifdef m_debug
	CreateThread(NULL, 0, watch_dir, NULL, 0, NULL);
	#endif // m_debug

	wglSwapIntervalEXT = (PFNWGLSWAPINTERVALEXTPROC)load_gl_func("wglSwapIntervalEXT");
	#define X(type, name) name = (type)load_gl_func(#name);
		m_gl_funcs
	#undef X

	#ifdef m_debug
	t_update_game* update_game = NULL;
	HMODULE dll = NULL;
	#endif // m_debug
	void* game_memory = NULL;

	{
		s_lin_arena all = {};
		all.capacity = 100 * c_mb;

		// @Note(tkap, 26/06/2023): We expect this memory to be zero'd
		all.memory = VirtualAlloc((void*)(4 * c_tb), all.capacity, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);

		game_renderer = (s_game_renderer*)la_get(&all, sizeof(s_game_renderer));

		game_memory = la_get(&all, c_game_memory);
		platform_frame_arena = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		game_frame_arena = make_lin_arena_from_memory(10 * c_mb, la_get(&all, 10 * c_mb));
		g_platform_data.frame_arena = &game_frame_arena;

		game_renderer->arenas[0] = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		game_renderer->arenas[1] = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		game_renderer->transform_arenas[0] = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		game_renderer->transform_arenas[1] = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		game_renderer->textures.add({});
		after_loading_texture(game_renderer);
	}

	game_renderer->set_vsync = set_vsync;
	game_renderer->load_texture = load_texture;
	game_renderer->load_font = load_font;
	game_renderer->make_framebuffer = make_framebuffer;
	game_renderer->set_shader_float = set_shader_float;
	game_renderer->set_shader_v2 = set_shader_v2;

	init_gl(&g_platform_renderer, game_renderer, &platform_frame_arena);

	b8 running = true;

	#ifndef m_debug
	g_platform_data.recompiled = true;
	#endif // m_debug

	f64 start_of_frame_seconds = 0;
	while(running)
	{

		f64 seconds = get_seconds();
		f64 time_passed = seconds - start_of_frame_seconds;
		g_platform_data.frame_time = time_passed;
		game_renderer->total_time += time_passed;
		start_of_frame_seconds = seconds;

		MSG msg = {};
		while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE) > 0)
		{
			if(msg.message == WM_QUIT)
			{
				running = false;
			}
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}

		g_platform_data.input = &g_input;
		g_platform_data.logic_input = &g_logic_input;
		g_platform_data.quit_after_this_frame = !running;
		g_platform_data.window_width = g_window.width;
		g_platform_data.window_height = g_window.height;
		g_platform_data.get_random_seed = get_random_seed;
		g_platform_data.load_sound = load_sound;
		g_platform_data.play_sound = play_sound;
		g_platform_data.read_file = read_file;
		g_platform_data.write_file = write_file;
		// g_platform_data.show_cursor = ShowCursor;
		g_platform_data.cycle_between_available_resolutions = cycle_between_available_resolutions;

		#ifdef m_debug
		if(need_to_reload_dll("build/DigHard.dll"))
		{
			if(dll) { unload_dll(dll); }

			for(int i = 0; i < 100; i++)
			{
				if(CopyFile("build/DigHard.dll", "DigHard.dll", false)) { break; }
				Sleep(10);
			}
			dll = load_dll("DigHard.dll");
			update_game = (t_update_game*)GetProcAddress(dll, "update_game");
			assert(update_game);
			log_info("Reloaded DLL!\n");
			g_platform_data.recompiled = true;
		}
		#endif // m_debug

		POINT p;
		GetCursorPos(&p);
		ScreenToClient(g_window.handle, &p);
		g_platform_data.mouse.x = (float)p.x;
		g_platform_data.mouse.y = (float)p.y;
		g_platform_data.is_window_active = GetActiveWindow() == g_window.handle;

		update_game(&g_platform_data, game_memory, game_renderer);
		g_platform_data.recompiled = false;

		if(g_do_embed) {
			write_embed_file();
		}

		gl_render(&g_platform_renderer, game_renderer);

		SwapBuffers(g_window.dc);

		#ifdef m_debug
		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		hot reload shaders and textures start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		{
			while(g_file_read < g_file_write)
			{
				char* file_path = g_files[g_file_read % c_max_files];
				b8 is_vertex = strstr(file_path, ".vertex") != NULL;
				b8 is_fragment = strstr(file_path, ".fragment") != NULL;
				b8 advance_file = true;
				if(is_vertex || is_fragment)
				{
					for(int shader_i = 0; shader_i < e_shader_count; shader_i++)
					{
						s_shader_paths paths = c_shader_paths[shader_i];
						b8 do_load = false;
						if(is_vertex) {
							if(strcmp(file_path, paths.vertex_path) == 0) {
								do_load = true;
							}
						}
						else if(is_fragment) {
							if(strcmp(file_path, paths.fragment_path) == 0) {
								do_load = true;
							}
						}
						invalid_else;

						if(do_load) {
							char* vertex_src = read_file(paths.vertex_path, &platform_frame_arena);
							char* fragment_src = read_file(paths.fragment_path, &platform_frame_arena);
							if(!vertex_src || !vertex_src[0] || !fragment_src || !fragment_src[0]) { advance_file = false; }
							u32 program =  load_shader_from_str(vertex_src, fragment_src);

							// @Note(tkap, 11/10/2023): We successfully loaded the shader files, but they don't compile/link, so we want to advance file
							if(!program) { break; }

							gl(glUseProgram(0));
							gl(glDeleteProgram(g_platform_renderer.programs[shader_i]));
							g_platform_renderer.programs[shader_i] = program;

							log_info("Reloaded %s", file_path);

							break;
						}
					}
				}
				else
				{
					foreach_val(texture_i, texture, game_renderer->textures)
					{
						// @Note(tkap, 11/10/2023): Our first texture is a "fake texture", so let's not try to read it's path (it doesn't have any)
						if(!texture.path) { continue; }

						if(strcmp(texture.path, file_path) == 0)
						{
							int width, height, num_channels;
							void* data = stbi_load(file_path, &width, &height, &num_channels, 4);
							if(!data)
							{
								advance_file = false;
								break;
							}
							gl(glDeleteTextures(1, &texture.gpu_id));
							s_texture new_texture = load_texture_from_data(data, width, height, GL_LINEAR);
							new_texture.game_id = texture_i;
							new_texture.path = file_path;
							stbi_image_free(data);
							game_renderer->textures[texture_i] = new_texture;
							log_info("Reloaded %s", file_path);
							break;
						}
					}
				}
				if(advance_file)
				{
					g_file_read += 1;
				}
			}
		}
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		hot reload shaders and textures end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
		#endif // m_debug
	}

	return 0;

}

LRESULT window_proc(HWND window, UINT msg, WPARAM wparam, LPARAM lparam)
{
	LRESULT result = 0;
	int key = 0;
	b8 is_down = false;

	switch(msg)
	{

		case WM_CLOSE:
		case WM_DESTROY:
		{
			PostQuitMessage(0);
		} break;

		case WM_SIZE:
		{
			int width = LOWORD(lparam);
			int height = HIWORD(lparam);
			if(width <= 0 || height <= 0) { break; }

			g_window.width = width;
			g_window.height = height;
			g_platform_data.window_resized = true;
		} break;

		case WM_KEYDOWN:
		case WM_KEYUP:
		{
			if(msg == WM_KEYDOWN)
			{
				g_platform_data.any_key_pressed = true;
			}

			key = (int)remap_key_if_necessary(wparam, lparam);
			is_down = !(b32)((HIWORD(lparam) >> 15) & 1);
			int is_echo = is_down && ((lparam >> 30) & 1);
			if(key < c_max_keys && !is_echo)
			{
				s_stored_input si = {};
				si.key = key;
				si.is_down = is_down;
				apply_event_to_input(&g_input, si);
				apply_event_to_input(&g_logic_input, si);
			}
		} break;

		case WM_LBUTTONDOWN:
			key = c_left_mouse;
			is_down = true;
			goto deez;
		case WM_RBUTTONDOWN:
			key = c_right_mouse;
			is_down = true;
			goto deez;
		case WM_LBUTTONUP:
			key = c_left_mouse;
			is_down = false;
			goto deez;
		case WM_RBUTTONUP:
			key = c_right_mouse;
			is_down = false;
			goto deez;
		{
			deez:
			s_stored_input si = {};
			si.key = key;
			si.is_down = is_down;
			apply_event_to_input(&g_input, si);
			apply_event_to_input(&g_logic_input, si);
			g_platform_data.any_key_pressed = true;
		} break;

		default:
		{
			result = DefWindowProc(window, msg, wparam, lparam);
		}
	}

	return result;
}

static void create_window(int width, int height)
{
	const char* class_name = "DigHard_CLASS";
	HINSTANCE instance = GetModuleHandle(NULL);

	PFNWGLCREATECONTEXTATTRIBSARBPROC wglCreateContextAttribsARB = NULL;
	PFNWGLCHOOSEPIXELFORMATARBPROC wglChoosePixelFormatARB = NULL;

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		dummy start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		WNDCLASSEX window_class = {};
		window_class.cbSize = sizeof(window_class);
		window_class.style = CS_OWNDC;
		window_class.lpfnWndProc = DefWindowProc;
		window_class.lpszClassName = class_name;
		window_class.hInstance = instance;
		check(RegisterClassEx(&window_class));

		HWND dummy_window = CreateWindowEx(
			0,
			class_name,
			"dummy",
			WS_OVERLAPPEDWINDOW,
			CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT, CW_USEDEFAULT,
			NULL,
			NULL,
			instance,
			NULL
		);
		assert(dummy_window != INVALID_HANDLE_VALUE);

		HDC dc = GetDC(dummy_window);

		PIXELFORMATDESCRIPTOR pfd = {};
		pfd.nSize = sizeof(pfd);
		pfd.nVersion = 1;
		pfd.dwFlags = PFD_SUPPORT_OPENGL | PFD_DOUBLEBUFFER | PFD_DRAW_TO_WINDOW;
		pfd.cColorBits = 24;
		pfd.cDepthBits = 24;
		pfd.iLayerType = PFD_MAIN_PLANE;
		int format = ChoosePixelFormat(dc, &pfd);
		check(DescribePixelFormat(dc, format, sizeof(pfd), &pfd));
		check(SetPixelFormat(dc, format, &pfd));

		HGLRC glrc = wglCreateContext(dc);
		check(wglMakeCurrent(dc, glrc));

		wglCreateContextAttribsARB = (PFNWGLCREATECONTEXTATTRIBSARBPROC)load_gl_func("wglCreateContextAttribsARB");
		wglChoosePixelFormatARB = (PFNWGLCHOOSEPIXELFORMATARBPROC)load_gl_func("wglChoosePixelFormatARB");

		check(wglMakeCurrent(NULL, NULL));
		check(wglDeleteContext(glrc));
		check(ReleaseDC(dummy_window, dc));
		check(DestroyWindow(dummy_window));
		check(UnregisterClass(class_name, instance));

	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		dummy end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		window start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		WNDCLASSEX window_class = {};
		window_class.cbSize = sizeof(window_class);
		window_class.style = CS_OWNDC | CS_HREDRAW | CS_VREDRAW;
		window_class.lpfnWndProc = window_proc;
		window_class.lpszClassName = class_name;
		window_class.hInstance = instance;
		window_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		window_class.hIcon = LoadIcon(instance, MAKEINTRESOURCE(MY_ICON));
		check(RegisterClassEx(&window_class));

		DWORD style = (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX;
		// DWORD style = WS_POPUP | WS_VISIBLE;
		RECT rect = {};
		rect.right = width;
		rect.bottom = height;
		AdjustWindowRect(&rect, style, false);

		g_window.handle = CreateWindowEx(
			0,
			class_name,
			"DigHard",
			style,
			CW_USEDEFAULT, CW_USEDEFAULT, rect.right - rect.left, rect.bottom - rect.top,
			NULL,
			NULL,
			instance,
			NULL
		);
		assert(g_window.handle != INVALID_HANDLE_VALUE);

		center_window();

		g_window.dc = GetDC(g_window.handle);

		int pixel_attribs[] = {
			WGL_DRAW_TO_WINDOW_ARB, GL_TRUE,
			WGL_SUPPORT_OPENGL_ARB, GL_TRUE,
			WGL_DOUBLE_BUFFER_ARB, GL_TRUE,
			WGL_SWAP_METHOD_ARB, WGL_SWAP_COPY_ARB,
			WGL_PIXEL_TYPE_ARB, WGL_TYPE_RGBA_ARB,
			WGL_ACCELERATION_ARB, WGL_FULL_ACCELERATION_ARB,
			WGL_COLOR_BITS_ARB, 32,
			WGL_DEPTH_BITS_ARB, 24,
			WGL_STENCIL_BITS_ARB, 8,
			0
		};

		PIXELFORMATDESCRIPTOR pfd = {};
		pfd.nSize = sizeof(pfd);
		int format;
		u32 num_formats;
		check(wglChoosePixelFormatARB(g_window.dc, pixel_attribs, NULL, 1, &format, &num_formats));
		check(DescribePixelFormat(g_window.dc, format, sizeof(pfd), &pfd));
		SetPixelFormat(g_window.dc, format, &pfd);

		int gl_attribs[] = {
			WGL_CONTEXT_MAJOR_VERSION_ARB, 3,
			WGL_CONTEXT_MINOR_VERSION_ARB, 3,
			WGL_CONTEXT_PROFILE_MASK_ARB, WGL_CONTEXT_CORE_PROFILE_BIT_ARB,
			WGL_CONTEXT_FLAGS_ARB,
			// WGL_CONTEXT_DEBUG_BIT_ARB,
			0
		};
		HGLRC glrc = wglCreateContextAttribsARB(g_window.dc, NULL, gl_attribs);
		check(wglMakeCurrent(g_window.dc, glrc));
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		window end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
}

static PROC load_gl_func(const char* name)
{
	PROC result = wglGetProcAddress(name);
	if(!result)
	{
		printf("Failed to load %s\n", name);
		assert(false);
	}
	return result;
}

// @Note(tkap, 16/05/2023): https://stackoverflow.com/a/15977613
static WPARAM remap_key_if_necessary(WPARAM vk, LPARAM lparam)
{
	WPARAM new_vk = vk;
	UINT scancode = (lparam & 0x00ff0000) >> 16;
	int extended  = (lparam & 0x01000000) != 0;

	switch(vk)
	{
		case VK_SHIFT:
		{
			new_vk = MapVirtualKey(scancode, MAPVK_VSC_TO_VK_EX);
		} break;

		case VK_CONTROL:
		{
			new_vk = extended ? VK_RCONTROL : VK_LCONTROL;
		} break;

		case VK_MENU:
		{
			new_vk = extended ? VK_RMENU : VK_LMENU;
		} break;

		default:
		{
			new_vk = vk;
		} break;
	}

	return new_vk;
}

static b8 init_audio()
{
	HRESULT hr = CoInitializeEx(NULL, COINIT_MULTITHREADED);
	if(FAILED(hr)) { return false; }

	IXAudio2* xaudio2 = NULL;
	hr = XAudio2Create(&xaudio2, 0, XAUDIO2_DEFAULT_PROCESSOR);
	if(FAILED(hr)) { return false; }

	IXAudio2MasteringVoice* master_voice = NULL;
	hr = xaudio2->CreateMasteringVoice(&master_voice);
	if(FAILED(hr)) { return false; }

	WAVEFORMATEX wave = {};
	wave.wFormatTag = WAVE_FORMAT_PCM;
	wave.nChannels = c_num_channels;
	wave.nSamplesPerSec = c_sample_rate;
	wave.wBitsPerSample = 16;
	wave.nBlockAlign = (c_num_channels * wave.wBitsPerSample) / 8;
	wave.nAvgBytesPerSec = c_sample_rate * wave.nBlockAlign;

	for(int voice_i = 0; voice_i < c_max_concurrent_sounds; voice_i++)
	{
		s_voice* voice = &voice_arr[voice_i];
		hr = xaudio2->CreateSourceVoice(&voice->voice, &wave, 0, XAUDIO2_DEFAULT_FREQ_RATIO, voice, NULL, NULL);
		voice->voice->SetVolume(0.25f);
		if(FAILED(hr)) { return false; }
	}

	return true;

}

static b8 play_sound(s_sound* sound)
{
	if(!g_platform_data.is_window_active) { return false; }
	assert(sound->sample_count > 0);
	assert(sound->samples);

	XAUDIO2_BUFFER buffer = {};
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = sound->sample_count * c_num_channels * sizeof(s16);
	buffer.pAudioData = (BYTE*)sound->samples;

	s_voice* curr_voice = NULL;
	for(int voice_i = 0; voice_i < c_max_concurrent_sounds; voice_i++)
	{
		s_voice* voice = &voice_arr[voice_i];
		if(!voice->playing)
		{
			if(thread_safe_set_bool_to_true(&voice->playing))
			{
				curr_voice = voice;
				break;
			}
		}
	}

	if(curr_voice == NULL) { return false; }

	HRESULT hr = curr_voice->voice->SubmitSourceBuffer(&buffer);
	if(FAILED(hr)) { return false; }

	curr_voice->voice->Start();
	// curr_voice->sound = sound;

	return true;
}

static b8 thread_safe_set_bool_to_true(volatile int* var)
{
	return InterlockedCompareExchange((LONG*)var, true, false) == false;
}

static void init_performance()
{
	QueryPerformanceFrequency((LARGE_INTEGER*)&g_cycle_frequency);
	QueryPerformanceCounter((LARGE_INTEGER*)&g_start_cycles);
}

static f64 get_seconds()
{
	u64 now;
	QueryPerformanceCounter((LARGE_INTEGER*)&now);
	return (now - g_start_cycles) / (f64)g_cycle_frequency;
}

static void set_vsync(b8 val)
{
	if(wglSwapIntervalEXT)
	{
		wglSwapIntervalEXT(val ? 1 : 0);
	}
}

static int cycle_between_available_resolutions(int current)
{
	HMONITOR monitor = MonitorFromWindow(g_window.handle, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO info = {};
	info.cbSize = sizeof(info);
	BOOL result = GetMonitorInfoA(monitor, &info);
	if(!result) { return current; }
	int monitor_width = info.rcMonitor.right - info.rcMonitor.left;
	int monitor_height = info.rcMonitor.bottom - info.rcMonitor.top;
	if(monitor_width <= 0 || monitor_height <= 0) { return current; }

	int new_index = current;
	while(true)
	{
		new_index = (new_index + 1) % array_count(c_resolutions);
		s_v2i res = c_resolutions[new_index];
		if(res.x > monitor_width || res.y > monitor_height) { continue; }

		if(abs(res.x - monitor_width) < 50 || abs(res.y - monitor_height) < 50)
		{
			SetWindowLongA(g_window.handle, GWL_STYLE, WS_POPUP | WS_VISIBLE);
		}
		else
		{
			SetWindowLongA(g_window.handle, GWL_STYLE, (WS_OVERLAPPEDWINDOW | WS_VISIBLE) & ~WS_MAXIMIZEBOX & ~WS_SIZEBOX);
		}

		set_actual_window_size(res.x, res.y);
		center_window();
		break;
	}
	return new_index;

}

static s_v2i set_actual_window_size(int width, int height)
{
	LONG style = GetWindowLongA(g_window.handle, GWL_STYLE);
	RECT rect = {};
	rect.right = width;
	rect.bottom = height;
	AdjustWindowRect(&rect, style, false);
	int true_width = rect.right - rect.left;
	int true_height = rect.bottom - rect.top;
	SetWindowPos(g_window.handle, NULL, 0, 0, true_width, true_height, SWP_NOMOVE | SWP_NOZORDER);
	return v2i(true_width, true_height);
}

static void center_window()
{
	HMONITOR monitor = MonitorFromWindow(g_window.handle, MONITOR_DEFAULTTOPRIMARY);
	MONITORINFO info = {};
	info.cbSize = sizeof(info);
	BOOL result = GetMonitorInfoA(monitor, &info);
	if(!result) { return; }

	RECT rect;
	GetWindowRect(g_window.handle, &rect);
	int window_width = rect.right - rect.left;
	int window_height = rect.bottom - rect.top;

	int center_x = (info.rcMonitor.left + info.rcMonitor.right) / 2 - window_width / 2;
	int center_y = (info.rcMonitor.top + info.rcMonitor.bottom) / 2 - window_height / 2;
	SetWindowPos(g_window.handle, NULL, center_x, center_y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
}

#ifdef m_debug
static DWORD WINAPI watch_dir(void* arg)
{
	unreferenced(arg);

	HANDLE handle = CreateFile(".", FILE_LIST_DIRECTORY, FILE_SHARE_READ|FILE_SHARE_WRITE, NULL, OPEN_EXISTING, FILE_FLAG_BACKUP_SEMANTICS, NULL);
	while(true)
	{
		DWORD bytes_read;
		FILE_NOTIFY_INFORMATION buffer[16] = {};
		BOOL result = ReadDirectoryChangesW(
			handle, buffer, sizeof(buffer), true, FILE_NOTIFY_CHANGE_LAST_WRITE,
			&bytes_read, NULL, NULL
		);
		assert(result);
		if(bytes_read <= 0) { continue; }

		FILE_NOTIFY_INFORMATION* current = &buffer[0];
		while(true)
		{
			char file_path[MAX_PATH] = {};
			wide_to_unicode(current->FileName, file_path);
			str_replace(file_path, "\\", "/");

			memcpy(g_files[g_file_write % c_max_files], file_path, MAX_PATH);
			InterlockedIncrement((LONG*)&g_file_write);
			if(current->NextEntryOffset > 0)
			{
				current = (FILE_NOTIFY_INFORMATION*)((u8*)current + current->NextEntryOffset);
			}
			else { break; }
		}
	}
	return 0;
}
#endif // m_debug

static void wide_to_unicode(wchar_t* wide, char* out)
{
	assert(wide);
	int required_buffer_size = WideCharToMultiByte(CP_UTF8, 0, wide, -1, NULL, 0, NULL, NULL);
	assert(required_buffer_size > 0);
	assert(required_buffer_size <= MAX_PATH);

	// char* out_unicode = (char*)la_get(frame_arena, MAX_PATH);
	WideCharToMultiByte(CP_UTF8, 0, wide, -1, out, required_buffer_size, NULL, NULL);
}

static u32 get_random_seed()
{
	return (u32)__rdtsc();
}

static s_sound* load_sound(s_platform_data* platform_data, const char* path, s_lin_arena* arena)
{
	if(g_do_embed) {
		g_to_embed.add(path);
	}

	#ifdef m_debug

	s_sound sound = load_sound_from_file(path, arena);

	#else // m_debug

	s_sound sound = load_sound_from_data(embed_data[g_asset_index]);
	g_asset_index += 1;

	#endif // m_debug

	sound.index = platform_data->sounds.count;
	int index = platform_data->sounds.add(sound);
	return &platform_data->sounds[index];
}

static s_sound load_sound_from_file(const char* path, s_lin_arena* arena)
{
	u8* data = (u8*)read_file(path, arena);
	assert(data);
	return load_sound_from_data(data);
}

static s_sound load_sound_from_data(u8* data)
{
	s_sound sound = {};

	s_riff_chunk riff = *(s_riff_chunk*)data;
	data += sizeof(riff);
	s_fmt_chunk fmt = *(s_fmt_chunk*)data;
	assert(fmt.num_channels == c_num_channels);
	assert(fmt.sample_rate == c_sample_rate);
	data += sizeof(fmt);
	s_data_chunk data_chunk = *(s_data_chunk*)data;
	assert(memcmp(&data_chunk.sub_chunk2_id, "data", 4) == 0);
	data += 8;

	sound.sample_count = data_chunk.sub_chunk2_size / c_num_channels / sizeof(s16);
	// @TODO(tkap, 15/10/2023): Think of a way to not need malloc
	sound.samples = (s16*)malloc(c_num_channels * sizeof(s16) * sound.sample_count);
	memcpy(sound.samples, data, sound.sample_count * c_num_channels * sizeof(s16));

	return sound;
}

#ifdef m_debug
static b8 need_to_reload_dll(const char* path)
{
	WIN32_FIND_DATAA find_data = {};
	HANDLE handle = FindFirstFileA(path, &find_data);
	if(handle == INVALID_HANDLE_VALUE) { assert(false); return false; }

	b8 result = CompareFileTime(&last_dll_write_time, &find_data.ftLastWriteTime) == -1;
	FindClose(handle);
	if(result)
	{
		last_dll_write_time = find_data.ftLastWriteTime;
	}
	return result;
}

static HMODULE load_dll(const char* path)
{
	HMODULE result = LoadLibrary(path);
	assert(result);
	return result;
}

static void unload_dll(HMODULE dll)
{
	check(FreeLibrary(dll));
}
#endif // m_debug