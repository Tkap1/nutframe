
global constexpr int c_key_backspace = 0x08;
global constexpr int c_key_tab = 0x09;
global constexpr int c_key_enter = 0x0D;
global constexpr int c_key_alt = 0x12;
global constexpr int c_key_escape = 0x1B;
global constexpr int c_key_space = 0x20;
global constexpr int c_key_end = 0x23;
global constexpr int c_key_home = 0x24;
global constexpr int c_key_left = 0x25;
global constexpr int c_key_up = 0x26;
global constexpr int c_key_right = 0x27;
global constexpr int c_key_down = 0x28;
global constexpr int c_key_delete = 0x2E;
global constexpr int c_key_0 = 0x30;
global constexpr int c_key_1 = 0x31;
global constexpr int c_key_2 = 0x32;
global constexpr int c_key_3 = 0x33;
global constexpr int c_key_4 = 0x34;
global constexpr int c_key_5 = 0x35;
global constexpr int c_key_6 = 0x36;
global constexpr int c_key_7 = 0x37;
global constexpr int c_key_8 = 0x38;
global constexpr int c_key_9 = 0x39;
global constexpr int c_key_a = 0x41;
global constexpr int c_key_b = 0x42;
global constexpr int c_key_c = 0x43;
global constexpr int c_key_d = 0x44;
global constexpr int c_key_e = 0x45;
global constexpr int c_key_f = 0x46;
global constexpr int c_key_g = 0x47;
global constexpr int c_key_h = 0x48;
global constexpr int c_key_i = 0x49;
global constexpr int c_key_j = 0x4A;
global constexpr int c_key_k = 0x4B;
global constexpr int c_key_l = 0x4C;
global constexpr int c_key_m = 0x4D;
global constexpr int c_key_n = 0x4E;
global constexpr int c_key_o = 0x4F;
global constexpr int c_key_p = 0x50;
global constexpr int c_key_q = 0x51;
global constexpr int c_key_r = 0x52;
global constexpr int c_key_s = 0x53;
global constexpr int c_key_t = 0x54;
global constexpr int c_key_u = 0x55;
global constexpr int c_key_v = 0x56;
global constexpr int c_key_w = 0x57;
global constexpr int c_key_x = 0x58;
global constexpr int c_key_y = 0x59;
global constexpr int c_key_z = 0x5A;
global constexpr int c_key_add = 0x6B;
global constexpr int c_key_subtract = 0x6D;
global constexpr int c_key_f1 = 0x70;
global constexpr int c_key_f2 = 0x71;
global constexpr int c_key_f3 = 0x72;
global constexpr int c_key_f4 = 0x73;
global constexpr int c_key_f5 = 0x74;
global constexpr int c_key_f6 = 0x75;
global constexpr int c_key_f7 = 0x76;
global constexpr int c_key_f8 = 0x77;
global constexpr int c_key_f9 = 0x78;
global constexpr int c_key_f10 = 0x79;
global constexpr int c_key_f11 = 0x7A;
global constexpr int c_key_f12 = 0x7B;
global constexpr int c_key_left_shift = 0xA0;
global constexpr int c_key_right_shift = 0xA1;
global constexpr int c_key_left_control = 0xA2;
global constexpr int c_key_right_control = 0xA3;
global constexpr int c_left_mouse = 1020;
global constexpr int c_right_mouse = 1021;
global constexpr int c_max_keys = 1024;

global constexpr int c_game_memory = 1 * c_mb;

struct s_sound
{
	int sample_count;
	s16* samples;
};

typedef b8 (*t_play_sound)(s_sound);
typedef void (*t_set_vsync)(b8);
typedef int (*t_show_cursor)(b8);
typedef int (*t_cycle_between_available_resolutions)(int);

struct s_texture
{
	u32 gpu_id;
	int game_id;
	s_v2 size;
	s_v2 sub_size;
	char* path;
};


struct s_game_window
{
	int width;
	int height;

	// @Note(tkap, 24/06/2023): Set by the game
	s_v2 size;
	s_v2 center;
};

struct s_stored_input
{
	b8 is_down;
	int key;
};

struct s_key
{
	b8 is_down;
	int count;
};

struct s_input
{
	s_key keys[c_max_keys];
};

struct s_platform_data
{
	b8 recompiled;
	b8 quit_after_this_frame;
	b8 any_key_pressed;
	b8 is_window_active;
	b8 window_resized;
	int window_width;
	int window_height;
	f64 time_passed;
	s_input* input;
	s_input* logic_input;
	s_lin_arena* frame_arena;
	s_v2 mouse;
};

struct s_platform_funcs
{
	t_play_sound play_sound;
	t_show_cursor show_cursor;
	t_cycle_between_available_resolutions cycle_between_available_resolutions;
};

struct s_game_renderer;
typedef s_texture (*t_load_texture)(s_game_renderer*, char*);
struct s_game_renderer
{
	b8 did_we_alloc;
	t_set_vsync set_vsync;
	t_load_texture load_texture;
	f64 total_time;

	// @Note(tkap, 08/10/2023): We esentially want s_bucket_array<s_transform> transforms[e_texture_count];
	// but we don't know how many textures there will be at compile time, because the game code may load any amount
	s_bucket_array<s_transform>* transforms;

	int transform_arena_index;
	s_lin_arena transform_arenas[2];
	int arena_index;
	s_lin_arena arenas[2];
	s_sarray<s_texture, 16> textures;
};



#define m_update_game(name) void name(s_platform_data* platform_data, s_platform_funcs platform_funcs, void* game_memory, s_game_renderer* rendering)
#ifdef m_build_dll
typedef m_update_game(t_update_game);
#else // m_build_dll
m_update_game(update_game);
#endif
