
#define zero {}
#define func static

static constexpr int c_tiles_right = 512;
static constexpr int c_tiles_down = 2048;
static constexpr int c_editor_tile_size = 32;
static constexpr int c_play_tile_size = 1;
static constexpr int c_updates_per_second = 240;
static constexpr int c_max_save_points = 64;
static constexpr int c_max_jump_refreshers = 256;
static constexpr int c_map_version = 1;
static constexpr int c_tile_version = 1;
static constexpr int c_save_point_version = 1;
static constexpr int c_end_point_version = 1;
static constexpr int c_jump_refresher_version = 1;
static constexpr int c_max_jumps = 1;
static constexpr int c_max_projectiles = 128;
static constexpr int c_projectile_duration = 2000;
static constexpr int c_max_leaderboard_entries = 16;
static constexpr int c_max_particles = 8192;
static constexpr int c_max_death_sounds = 3;

static constexpr float c_player_z = 0.0f;
static constexpr float c_gravity = 0.001f;
static constexpr float c_small = 0.0001f;
static constexpr float c_particle_z = c_player_z - 0.01f;
static constexpr float c_spike_collision_size_multiplier = 0.8f;
static constexpr float c_player_jump_time = 0.4f;

static constexpr s_v2 c_projectile_visual_size = v2(1.0f);
static constexpr s_v2 c_projectile_collision_size = v2(0.25f);
static constexpr s_v2 c_player_visual_size = v2(c_play_tile_size * 1.3f);
static constexpr s_v2 c_player_collision_size = v2(c_play_tile_size * 0.9f, (float)c_play_tile_size);
static constexpr s_v2 c_save_point_visual_size = v2(c_play_tile_size);
static constexpr s_v2 c_save_point_collision_size = v2(c_play_tile_size * 1.5f);
static constexpr s_v2 c_jump_refresher_visual_size = v2(c_play_tile_size);
static constexpr s_v2 c_jump_refresher_collision_size = v2(c_play_tile_size);
static constexpr s_v2 c_end_point_size = v2(c_play_tile_size);
static constexpr s_v2 c_base_button_size = v2(320, 48);

static constexpr s_v2i c_sprite_size = v2i(64, 64);

enum e_tile : s8
{
	e_tile_invalid,
	e_tile_normal,
	e_tile_nullify_explosion,
	e_tile_spike,
	e_tile_platform,
	e_tile_count,
};
constexpr int c_things_to_place_count = (e_tile_count - 1) + 3;

enum e_state
{
	e_state_map_select,
	e_state_play,
	e_state_editor,
	e_state_leaderboard,
	e_state_input_name,
};

struct s_leaderboard_entry
{
	int rank;
	int time;
	s_str<32> nice_name;
	s_str<32> internal_name;
};

struct s_draw_data
{
	s_v2i index;
	s_v2i sprite_size;
	s_texture texture;
};

struct s_tile_collision
{
	e_tile tile;
	s_v2 tile_center;
};

struct s_jump_refresher
{
	b8 in_cooldown;
	int timer;
	s_v2i pos;
};

struct s_trail
{
	b8 flip_x;
	float time;
	s_draw_data draw_data;
	s_v2 pos;
};

enum e_visual_effect
{
	e_visual_effect_projectile_explosion,
};

struct s_end_point
{
	s_v2i pos;
};

enum e_editor_state
{
	e_editor_state_select,
	e_editor_state_place,
};

struct s_editor_select_state
{
	s_v2i selection_start;
	b8 selecting;
};

struct s_editor_place_state
{
	int temp;
};

#ifdef m_debug
struct s_copied_tile
{
	s_v2i index;
	s8 tile;
};
#endif // m_debug

struct s_editor
{
	#ifdef m_debug
	s_sarray<s_copied_tile, 32768> copied_tile_arr;
	#endif // m_debug
	s_carray2<b8, c_tiles_down, c_tiles_right> selected_tile_arr;
	e_editor_state state;
	s_editor_select_state select_state;
	s_editor_place_state place_state;
	int curr_tile;
};

struct s_player
{
	// @Note(tkap, 08/06/2024): For animation
	float jump_time;
	float last_x_vel;

	b8 did_any_action;
	b8 flip_x;
	b8 released_left_button_since_death;
	float animation_timer;
	int state;
	int jumps_left;
	int shoot_timer;
	s_v2 prev_pos;
	s_v2 pos;
	s_v2 vel;
};

struct s_visual_effect
{
	e_visual_effect type;
	float timer;
	s_v2 pos;
};

struct s_save_point
{
	s_v2i pos;
};

struct s_particle
{
	float fade;
	float shrink;
	float slowdown;
	s_v3 pos;
	s_v3 dir;
	float radius;
	float speed;
	float timer;
	float duration;
	s_v3 color;
};

struct s_particle_data
{
	float shrink = 1;
	float fade = 1;
	float slowdown;
	float duration;
	float duration_rand;
	float speed;
	float speed_rand;
	float angle;
	float angle_rand;
	float radius;
	float radius_rand;
	s_v3 color = {.x = 0.1f, .y = 0.1f, .z = 0.1f};
	s_v3 color_rand;
};

#pragma pack(push, 1)
struct s_map_header
{
	int map_version;
	int end_point_version;
	int tile_version;
	int save_point_version;
	int jump_refresher_version;
	int save_point_count;
	int jump_refresher_count;
};
#pragma pack(pop)

struct s_map
{
	s_end_point end_point;
	s_carray2<e_tile, c_tiles_down, c_tiles_right> tile_arr;
	s_sarray<s_save_point, c_max_save_points> save_point_arr;
	s_sarray<s_jump_refresher, c_max_jump_refreshers> jump_refresher_arr;
};


struct s_camera2d
{
	s_v2 pos;
	float zoom;

	s_recti get_tile_bounds();
	s_v2 world_to_screen(s_v2 v)
	{
		s_v2 result = v;
		result.x -= pos.x;
		result.y -= pos.y;
		result *= zoom;
		return result;
	}

	s_v2 screen_to_world(s_v2 v)
	{
		s_v2 result = v;
		result.x /= zoom;
		result.y /= zoom;
		result.x += pos.x;
		result.y += pos.y;
		return result;
	}

	s_v2 scale(s_v2 v) { return v * zoom; }
	float scale(float x) { return x * zoom; }

	s_m4 get_matrix();
};

struct s_projectile
{
	int timer;
	s_v2 prev_pos;
	s_v2 pos;
	s_v2 dir;
};

struct s_ui_data
{
	int element_count;
	int selected;
};

struct s_ui_element_data
{
	s_v2 size;
};

struct s_ui_optional
{
	float font_size;
	float size_x;
	float size_y;
};

struct s_ui
{
	s_hashmap<u32, s_ui_element_data, 1024> element_data;
	s_sarray<s_ui_data, 16> data_stack;
};

struct s_dev_menu
{
	b8 active;
	int selected_ui;
	b8 show_hitboxes;
};

struct s_map_data
{
	s_len_str name;
	char* path;
	s_v3 cam_offset;
	int leaderboard_id;
};

constexpr s_map_data c_map_data[] = {
	{.name = m_strlit("Easy"), .path = "platform_map.map", .cam_offset = v3(0.0f, -2, -10), .leaderboard_id = 22605},
	{.name = m_strlit("Hard"), .path = "map2.map", .cam_offset = v3(0.0f, -2, -10), .leaderboard_id = 22731},
	{.name = m_strlit("AQtun"), .path = "aqtun.map", .cam_offset = v3(0.0f, -2, -10), .leaderboard_id = 22741},
	{.name = m_strlit("Zanarias"), .path = "zanarias.map", .cam_offset = v3(0.0f, -2, -10), .leaderboard_id = 22762},
	{.name = m_strlit("Azenris"), .path = "azenris.map", .cam_offset = v3(0.0f, -2, -10), .leaderboard_id = 22763},
	{.name = m_strlit("Platforms"), .path = "platforms.map", .cam_offset = v3(5.0f, -2, -15), .leaderboard_id = 22806},

	#ifndef m_emscripten
	{.name = m_strlit("Create map"), .path = "example.map", .leaderboard_id = 0},
	#endif // m_emscripten
};

struct s_leaderboard_state
{
	b8 coming_from_win;
	b8 received;
};

struct s_input_name_state
{
	s_input_str<64> name;
	s_str<64> error_str;
};

struct s_map_select_state
{
	int map_selected;
};

struct s_game
{
	b8 initialized;
	b8 disable_bloom;
	b8 reset_game;
	e_state state;
	int reset_player;
	int curr_map;

	s_leaderboard_state leaderboard_state;
	s_map_select_state map_select_state;
	s_input_name_state input_name_state;

	s_ui ui;

	s_dev_menu dev_menu;

	f64 timer;
	float render_time;
	s_framebuffer* particle_framebuffer;
	s_framebuffer* text_framebuffer;
	s_camera3d cam;
	s_camera2d editor_cam;
	s_texture sheet;
	s_texture noise;
	s_texture save_point_texture;
	s_texture player_idle_texture;
	s_texture player_run_texture;
	s_texture player_jump_texture;
	// s_texture player_leap_texture;
	// s_texture player_charge_texture;
	s_texture player_fall_texture;
	s_carray<s_texture, e_tile_count> tile_texture_arr;
	s_sarray<s_projectile, c_max_projectiles> projectile_arr;
	s_sarray<s_particle, c_max_particles> particle_arr;
	s_rng rng;
	s_font* font;
	s_framebuffer* main_fbo;
	s_framebuffer* bloom_fbo;
	s_carray<s_framebuffer*, 2> fbo_arr;
	s_player player;
	s_map map;
	int curr_save_point;
	s_editor editor;
	s_ray ray;
	s_sarray<s_visual_effect, 128> visual_effect_arr;
	s_sound* thud_sound;
	s_sound* explosion_sound;
	s_sound* save_sound;
	s_sound* shoot_sound;
	s_sound* jump_sound;
	s_sound* win_sound;
	s_carray<s_sound*, c_max_death_sounds> death_sound_arr;
	s_sarray<s_leaderboard_entry, c_max_leaderboard_entries> leaderboard_arr;
	s_sarray<s_trail, 4096> trail_arr;
};


static s_v2i pos_to_index(s_v2 pos, int tile_size);
static b8 is_index_valid(s_v2i index);
static s_recti get_3d_tile_bounds(s_camera3d cam);
static void load_map(s_map* map, int index, s_platform_data* platform_data);
static b8 index_has_tile(s_v2i index);
static s_sarray<s_tile_collision, 16> get_tile_collisions(s_v2 pos, s_v2 size, int tile_size, int tile_blacklist_mask = 0);
static s_v2 index_to_pos(s_v2i index, int tile_size);
static void do_particles(int count, s_v3 pos, s_particle_data data);
static void on_leaderboard_received(s_json* json);
static void on_our_leaderboard_received(s_json* json);
static void after_submitted_leaderboard();
static void on_leaderboard_score_submitted();
static s_m4 get_camera_view(s_camera3d cam);
static s_draw_data get_player_draw_data(s_player player);
static void ui_start(int selected);
static void ui_bool_button(s_len_str id_str, s_v2 pos, b8* ptr);
static int ui_end();
static void do_ui(s_m4 ortho);
static b8 ui_button(s_len_str id_str, s_v2 pos, s_ui_optional optional = {});
static void set_state(e_state state);
static void on_set_leaderboard_name(b8 success);