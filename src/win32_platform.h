


struct s_window
{
	HDC dc;
	HWND handle;
	int width;
	int height;
};

enum e_shader
{
	e_shader_default,
	e_shader_count
};

struct s_shader_paths
{
	#ifdef m_debug
	u64 last_write_time;
	#endif // m_debug
	char* vertex_path;
	char* fragment_path;
};

struct s_platform_renderer
{
	u32 default_vao;
	u32 default_vbo;
	u32 programs[e_shader_count];
};

struct s_voice : IXAudio2VoiceCallback
{
	IXAudio2SourceVoice* voice;

	volatile int playing;

	void OnStreamEnd()
	{
		voice->Stop();
		InterlockedExchange((LONG*)&playing, false);
	}

	#pragma warning(push, 0)
	void OnBufferStart(void * pBufferContext) {}
	void OnVoiceProcessingPassEnd() { }
	void OnVoiceProcessingPassStart(UINT32 SamplesRequired) { unreferenced(SamplesRequired); }
	void OnBufferEnd(void * pBufferContext) { unreferenced(pBufferContext); }
	void OnLoopEnd(void * pBufferContext) { unreferenced(pBufferContext); }
	void OnVoiceError(void * pBufferContext, HRESULT Error) { unreferenced(pBufferContext); unreferenced(Error);}
	#pragma warning(pop)
};


func void create_window(int width, int height);
func WPARAM remap_key_if_necessary(WPARAM vk, LPARAM lparam);
func PROC load_gl_func(char* name);
func b8 init_audio();
func b8 thread_safe_set_bool_to_true(volatile int* var);
func b8 play_sound(s_sound sound);
func void init_performance();
func f64 get_seconds();
func void set_vsync(b8 val);
func int cycle_between_available_resolutions(int current);
func void center_window();
func s_v2i set_actual_window_size(int width, int height);
void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam);
func u32 load_shader(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena);
func b8 check_for_shader_errors(u32 id, char* out_error);
func s_texture load_texture(s_game_renderer* game_renderer, char* path);
func s_texture load_texture_from_data(void* data, int width, int height, u32 filtering);
func s_texture load_texture_from_file(char* path, u32 filtering);
func void after_loading_texture(s_game_renderer* game_renderer);

#ifdef m_debug
func DWORD WINAPI watch_dir(void* arg);
#endif // m_debug

func void wide_to_unicode(wchar_t* wide, char* out);