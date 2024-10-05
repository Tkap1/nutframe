
#define zero {}
#define func static
#define global static

global constexpr int c_updates_per_second = 60;
global constexpr int c_max_leaderboard_entries = 16;
global constexpr int c_max_particles = 8192;
global constexpr s_v2 c_base_button_size = v2(128, 48);
global constexpr s_v2 c_player_size = v2(32);
global constexpr s_v2 c_creature_size = v2(24);
global constexpr s_v2 c_bot_size = v2(16);
global constexpr s_v2 c_base_size = v2(128);
global constexpr s_v2i c_sprite_size = v2i(64, 64);
global constexpr int c_max_creatures = 4096;
global constexpr int c_max_bots = 4096;
// @TODO(tkap, 05/10/2024): set me
global constexpr int c_leaderboard_id = -1;
global constexpr s_v2 c_base_pos = v2(400, 400);
global f64 c_spawns_per_second = 0.33;

enum e_layer
{
	e_layer_background,
	e_layer_base,
	e_layer_player,
	e_layer_creature,
	e_layer_bot,
	e_layer_laser,
};

enum e_upgrade
{
	e_upgrade_buy_bot,
	e_upgrade_player_damage,
	e_upgrade_bot_damage,
	e_upgrade_player_movement_speed,
	e_upgrade_bot_movement_speed,
	e_upgrade_spawn_rate,
	e_upgrade_creature_tier,
	e_upgrade_player_harvest_range,
	e_upgrade_bot_harvest_range,
	e_upgrade_double_harvest,
	e_upgrade_count,
};

struct s_upgrade_data
{
	int base_cost;
	int max_upgrades = 1000000;
	char* name;
};

global constexpr s_upgrade_data c_upgrade_data[] = {
	{.base_cost = 100, .name = "Buy drone (%i)"},
	{.base_cost = 50, .name = "+ player damage (%i)"},
	{.base_cost = 50, .name = "+ drone damage (%i)"},
	{.base_cost = 50, .name = "+ player speed (%i)"},
	{.base_cost = 300, .name = "+ drone speed (%i)"},
	{.base_cost = 100, .name = "+ spawn rate (%i)"},
	{.base_cost = 1000, .name = "+ creature tier (%i)"},
	{.base_cost = 100, .name = "+ player range (%i)"},
	{.base_cost = 500, .name = "+ drone range (%i)"},
	{.base_cost = 100000, .max_upgrades = 1, .name = "x2 harvest (%i)"},
};

enum e_state
{
	e_state_play,
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

struct s_trail
{
	b8 flip_x;
	float time;
	s_v2 pos;
};

enum e_visual_effect
{
	e_visual_effect_projectile_explosion,
};

enum e_pos_area_flag
{
	e_pos_area_flag_vertical = 1 << 0,
	e_pos_area_flag_center_x = 1 << 1,
	e_pos_area_flag_center_y = 1 << 2,
};

typedef struct s_pos_area
{
	float spacing;
	s_v2 advance;
	s_v2 pos;
} s_pos_area;


#define for_creature_partial(mname) for(int mname = game->creature_arr.index_data.lowest_index; mname < game->creature_arr.index_data.max_index_plus_one; mname += 1)
#define for_bot_partial(mname) for(int mname = game->bot_arr.index_data.lowest_index; mname < game->bot_arr.index_data.max_index_plus_one; mname += 1)

struct s_entity_index_data
{
	int lowest_index;
	int max_index_plus_one;
};

struct s_entity_index
{
	int index;
	int id;
};


struct s_player
{
	b8 flip_x;
	int harvest_timer;
	s_entity_index target;
	s_v2 prev_pos;
	s_v2 pos;
};


struct s_creature_arr
{
	s_entity_index_data index_data;
	b8 active[c_max_creatures];
	b8 targeted[c_max_creatures];
	int id[c_max_creatures];
	int roam_timer[c_max_creatures];
	int curr_health[c_max_creatures];
	int tier[c_max_creatures];
	int tick_when_last_damaged[c_max_creatures];
	s_v2 prev_pos[c_max_creatures];
	s_v2 pos[c_max_creatures];
	s_v2 target_pos[c_max_creatures];
};

enum e_bot_state
{
	e_bot_state_going_to_creature,
	e_bot_state_harvesting_creature,
	e_bot_state_going_back_to_base,
};

struct s_bot_arr
{
	s_entity_index_data index_data;
	b8 active[c_max_bots];
	int id[c_max_bots];
	int harvest_timer[c_max_bots];
	int cargo[c_max_bots];
	s_entity_index target[c_max_bots];
	s_entity_index laser_target[c_max_bots];
	e_bot_state state[c_max_bots];
	s_v2 prev_pos[c_max_bots];
	s_v2 pos[c_max_bots];
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

struct s_camera2d
{
	s_v2 pos;
	float zoom;

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

struct s_get_closest_creature
{
	float smallest_non_targeted_dist = 999999;
	s_entity_index closest_non_targeted_creature = zero;
	float smallest_dist = 999999;
	s_entity_index closest_creature = zero;
};


struct s_game
{
	b8 initialized;
	b8 reset_game;
	e_state state;

	s_carray<int, e_upgrade_count> upgrade_level_arr;

	int update_count;

	int next_entity_id;

	f64 spawn_creature_timer;
	int resource_count;

	s_render_pass* background_render_pass;
	s_render_pass* world_render_pass0;
	s_render_pass* world_render_pass1;
	s_render_pass* ui_render_pass0;
	s_render_pass* ui_render_pass1;

	s_texture placeholder_texture;

	s_sarray<s_particle, c_max_particles> particle_arr;

	s_leaderboard_state leaderboard_state;
	s_input_name_state input_name_state;

	s_ui ui;

	s_dev_menu dev_menu;

	s_creature_arr creature_arr;
	s_bot_arr bot_arr;

	f64 timer;
	float render_time;
	s_framebuffer* particle_framebuffer;
	s_framebuffer* text_framebuffer;
	s_camera2d cam;
	s_texture sheet;
	s_texture noise;
	s_rng rng;
	s_font* font;
	s_framebuffer* main_fbo;
	s_framebuffer* bloom_fbo;
	s_carray<s_framebuffer*, 2> fbo_arr;
	s_player player;
	s_sarray<s_visual_effect, 128> visual_effect_arr;
	s_sarray<s_leaderboard_entry, c_max_leaderboard_entries> leaderboard_arr;
};


func s_v2i pos_to_index(s_v2 pos, int tile_size);
func b8 is_index_valid(s_v2i index);
func b8 index_has_tile(s_v2i index);
func s_v2 index_to_pos(s_v2i index, int tile_size);
func void do_particles(int count, s_v3 pos, s_particle_data data);
func void on_leaderboard_received(s_json* json);
func void on_our_leaderboard_received(s_json* json);
func void after_submitted_leaderboard();
func void on_leaderboard_score_submitted();
func void ui_start(int selected);
func void ui_bool_button(s_len_str id_str, s_v2 pos, b8* ptr);
func int ui_end();
func void do_ui(s_m4 ortho);
func b8 ui_button(s_len_str id_str, s_v2 pos, s_ui_optional optional = {});
func void on_set_leaderboard_name(b8 success);
func int make_creature(s_v2 pos, int tier);
func void pick_target_for_bot(int bot);
func int get_creature(s_entity_index index);
func int make_bot(s_v2 pos);
func void remove_entity(int entity, b8* active, s_entity_index_data* index_data);
func b8 damage_creature(int creature, int damage);
func s_get_closest_creature get_closest_creature(s_v2 pos);
func s_pos_area make_pos_area(s_v2 pos, s_v2 size, s_v2 element_size, float spacing, int count, int flags);
func s_v2 pos_area_get_advance(s_pos_area* area);
static b8 ui_button2(s_len_str id_str, s_v2 pos, s_ui_optional optional = zero);
func void console_add_resource(s_console* cn, char* text);
func int get_player_damage();
func int get_bot_damage();
func float get_player_movement_speed();
func float get_bot_movement_speed();
func f64 get_creature_spawn_delay();
func int get_creature_spawn_tier();
func float get_player_harvest_range();
func float get_bot_harvest_range();
func int get_creature_resource_reward(int tier);