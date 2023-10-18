
#ifdef _WIN32
#define m_dll_export __declspec(dllexport)
#endif // _WIN32

#ifdef __GNUC__
// @TODO(tkap, 13/10/2023): stackoverflow copy paste
#define m_dll_export __attribute__((visibility("default")))
#endif // __linux__

#ifdef __linux__
// @TODO(tkap, 13/10/2023): I don't know. Nothing??
#define m_dll_export
#endif // __linux__

global constexpr int c_key_backspace = 0x08;
global constexpr int c_key_tab = 0x09;
global constexpr int c_key_enter = 0x0D;
global constexpr int c_key_alt = 0x12;
global constexpr int c_key_left_alt = 0xA4;
global constexpr int c_key_right_alt = 0xA5;
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
global constexpr int c_key_left_ctrl = 0xA2;
global constexpr int c_key_right_ctrl = 0xA3;
global constexpr int c_left_mouse = 1020;
global constexpr int c_right_mouse = 1021;
global constexpr int c_max_keys = 1024;

global constexpr int c_game_memory = 1 * c_mb;

global constexpr s_v2 c_origin_topleft = {1.0f, -1.0f};
global constexpr s_v2 c_origin_bottomleft = {1.0f, 1.0f};
global constexpr s_v2 c_origin_center = {0, 0};

global constexpr s_v2 c_base_res = {64*12, 64*12};
global constexpr s_v2 c_half_res = {c_base_res.x / 2.0f, c_base_res.y / 2.0f};

global constexpr int c_base_resolution_index = 5;
global constexpr s_v2i c_resolutions[] = {
	v2i(640, 360),
	v2i(854, 480),
	v2i(960, 540),
	v2i(1024, 576),
	v2i(1280, 720),
	v2i(1366, 768),
	v2i(1600, 900),
	v2i(1920, 1080),
	v2i(2560, 1440),
	v2i(3200, 1800),
	v2i(3840, 2160),
	v2i(5120, 2880),
	v2i(7680, 4320),
};

enum e_render_flags
{
	e_render_flag_use_texture = 1 << 0,
	e_render_flag_flip_x = 1 << 1,
};

struct s_sound
{
	int index;
	int sample_count;
	s16* samples;
};

global constexpr int c_max_arena_push = 16;

struct s_lin_arena
{
	int push_count;
	u64 push[c_max_arena_push];
	u64 used;
	u64 capacity;
	void* memory;
};

struct s_framebuffer;
struct s_game_renderer;
struct s_platform_data;

typedef b8 (*t_play_sound)(s_sound*);
typedef void (*t_set_vsync)(b8);
typedef int (*t_show_cursor)(b8);
typedef int (*t_cycle_between_available_resolutions)(int);
typedef u32 (*t_get_random_seed)();
typedef s_framebuffer* (*t_make_framebuffer)(s_game_renderer*, b8);
typedef s_sound* (*t_load_sound)(s_platform_data*, const char*, s_lin_arena*);

struct s_texture
{
	b8 comes_from_framebuffer;
	u32 gpu_id;
	int game_id;
	s_v2 size;
	s_v2 sub_size;
	const char* path;
};

struct s_glyph
{
	int advance_width;
	int width;
	int height;
	int x0;
	int y0;
	int x1;
	int y1;
	s_v2 uv_min;
	s_v2 uv_max;
};

struct s_font
{
	float size;
	float scale;
	int ascent;
	int descent;
	int line_gap;
	s_texture texture;
	s_glyph glyph_arr[1024];
};

// @Note(tkap, 08/10/2023): We have a bug with this. If we ever go from having never drawn anything to drawing 64*16+1 things we will
// exceed the max bucket count (16 currently). To fix this, I guess we have to allow merging in the middle of a frame?? Seems messy...
global constexpr int c_bucket_capacity = 64;

template <typename t>
struct s_bucket_array
{
	int bucket_count;
	int capacity[16];
	int element_count[16];
	t* elements[16];
};

#pragma pack(push, 1)
struct s_transform
{
	int flags;
	int layer;
	int sublayer;
	int effect_id;
	float mix_weight;
	float rotation;
	s_v2 pos;
	s_v2 origin_offset;
	s_v2 draw_size;
	s_v2 texture_size;
	s_v2 uv_min;
	s_v2 uv_max;
	s_v4 color;
	s_v4 mix_color;
};
#pragma pack(pop)

struct s_framebuffer
{
	u32 gpu_id;
	int game_id;
	s_texture texture;
	b8 do_depth;

	// @Note(tkap, 08/10/2023): We esentially want s_bucket_array<s_transform> transforms[e_texture_count][e_blend_mode_count];
	// but we don't know how many textures there will be at compile time, because the game code may load any amount
	s_bucket_array<s_transform>* transforms;
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

enum e_blend_mode
{
	e_blend_mode_normal,
	e_blend_mode_additive,
	e_blend_mode_count,
};

struct s_render_data
{
	e_blend_mode blend_mode;
	s_framebuffer* framebuffer;
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
	s_input* input;
	s_input* logic_input;
	s_lin_arena* frame_arena;
	s_v2 mouse;
	f64 frame_time;
	t_get_random_seed get_random_seed;
	t_load_sound load_sound;
	t_play_sound play_sound;
	s_sarray<s_sound, 16> sounds;
	t_show_cursor show_cursor;
	t_cycle_between_available_resolutions cycle_between_available_resolutions;
};

typedef s_texture (*t_load_texture)(s_game_renderer*, const char*);
typedef s_font* (*t_load_font)(s_game_renderer*, const char*, int, s_lin_arena*);
struct s_game_renderer
{
	b8 did_we_alloc;
	t_set_vsync set_vsync;
	t_load_texture load_texture;
	t_load_font load_font;
	t_make_framebuffer make_framebuffer;
	f64 total_time;

	int transform_arena_index;
	s_lin_arena transform_arenas[2];
	int arena_index;
	s_lin_arena arenas[2];
	s_sarray<s_texture, 16> textures;
	s_sarray<s_framebuffer, 4> framebuffers;
	s_sarray<s_font, 4> fonts;
};

#define m_update_game(name) void name(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
#ifdef m_build_dll
typedef m_update_game(t_update_game);
#else // m_build_dll
m_update_game(update_game);
#endif


func int get_render_offset(int texture, int blend_mode);
func s_v2 get_text_size_with_count(const char* text, s_font* font, float font_size, int count);
func s_v2 get_text_size(const char* text, s_font* font, float font_size);
template <typename t>
func void bucket_add(s_bucket_array<t>* arr, t new_element, s_lin_arena* arena, b8* did_we_alloc);
template <typename t>
func void bucket_merge(s_bucket_array<t>* arr, s_lin_arena* arena);