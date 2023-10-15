


struct s_window
{
	int width;
	int height;
};

func void create_window(int width, int height);
func b8 init_audio();
func b8 thread_safe_set_bool_to_true(volatile int* var);
func b8 play_sound(s_sound* sound);
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
func s_texture load_texture_from_file(const char* path, u32 filtering);
func void after_loading_texture(s_game_renderer* game_renderer);
func void render();
func void printShaderLog( GLuint shader );
void printProgramLog( GLuint program );
func void do_one_frame();
func u32 get_random_seed();
func int sdl_key_to_windows_key(int key);
func f64 get_seconds();
func s_sound* load_sound(s_platform_data* platform_data, const char* path, s_lin_arena* arena);
