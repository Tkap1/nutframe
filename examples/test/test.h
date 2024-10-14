
#define zero {}
#define func static
#define global static
#define tag(...)

global constexpr int c_updates_per_second = 60;
global constexpr f64 c_update_delay = 1.0 / c_updates_per_second;
global constexpr int c_max_leaderboard_entries = 16;
global constexpr int c_max_particles = 8192;
global constexpr s_v2 c_base_button_size = v2(256, 32);
global constexpr s_v2 c_player_size = v2(128);
global constexpr s_v2 c_creature_size = v2(64);
global constexpr s_v2 c_bot_size = v2(64);
global constexpr s_v2 c_base_size = v2(512);
global constexpr s_v2i c_sprite_size = v2i(64, 64);
global constexpr int c_max_creatures = 1024;
global constexpr int c_max_bots = 4096;
// global constexpr int c_leaderboard_id = 24824;
global constexpr int c_leaderboard_id = 24910;
global constexpr s_v2 c_base_pos = v2(400, 400);
global f64 c_spawns_per_second = 1.5;
global constexpr int c_resource_to_win = 50000;
global constexpr int c_num_creatures_to_lose = 1000;
global constexpr float c_laser_width = 16;
global constexpr int c_cell_area = 4096;
global constexpr int c_cell_size = 256;
global constexpr int c_num_cells = c_cell_area / c_cell_size;
global constexpr s_v2 c_cells_topleft = v2(c_base_pos.x - c_cell_area * 0.5f, c_base_pos.y - c_cell_area * 0.5f);
global constexpr int c_max_player_hits = 16;
global constexpr int c_max_bot_hits = 16;
global constexpr s_v2 c_pickup_size = v2(64);
global constexpr float c_tile_size = 256;
global constexpr int c_max_craters = 32;
global constexpr int c_dash_duration = 20;
global constexpr int c_dash_cooldown = 50;
global constexpr float c_dash_speed = 24;
global constexpr s_v2 c_base_button_size2 = v2(376, 44);
global constexpr int c_win_animation_duration_in_ticks = c_updates_per_second * 3;
global constexpr int c_invalid_entity = -1000000000;

enum e_sub_state
{
	e_sub_state_default,
	e_sub_state_pause,
	e_sub_state_defeat,
	e_sub_state_level_up,
	e_sub_state_winning,
};

enum e_sound
{
	e_sound_creature_death00,
	e_sound_creature_death01,
	e_sound_creature_death02,
	e_sound_buy_bot,
	e_sound_upgrade,
	e_sound_level_up,
	e_sound_dash,
	e_sound_count,
};

enum e_sound_group
{
	e_sound_group_creature_death,
	e_sound_group_buy_bot,
	e_sound_group_upgrade,
	e_sound_group_level_up,
	e_sound_group_dash,
	e_sound_group_count,
};

struct s_sound_group_data
{
	int sound_count;
	float cooldown;
	s_carray<e_sound, 4> sound_arr;
};

global constexpr s_sound_group_data c_sound_group_data_arr[e_sound_group_count] = {
	{3, 0.1f, {e_sound_creature_death00, e_sound_creature_death01, e_sound_creature_death02}},
	{1, 0.1f, {e_sound_buy_bot}},
	{1, 0.1f, {e_sound_upgrade}},
	{1, 0, {e_sound_level_up}},
	{1, 0, {e_sound_dash}},
};

global float g_sound_group_last_play_time_arr[e_sound_group_count];

struct s_cells
{
	s_dynamic_array<int> cell_arr[c_num_cells][c_num_cells];
};

static_assert(c_max_creatures > c_num_creatures_to_lose);

enum e_layer
{
	e_layer_background,
	e_layer_crater,
	e_layer_shadow,
	e_layer_base,
	e_layer_broken_bot,
	e_layer_creature,
	e_layer_bot,
	e_layer_player,
	e_layer_hitbox,
	e_layer_laser,
	e_layer_particle,
	e_layer_text,
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
	e_upgrade_bot_cargo_count,
	e_upgrade_player_chain,
	e_upgrade_count,
};

struct s_upgrade_data
{
	int base_cost;
	int max_upgrades = 1000000;
	char* name;
	int key;
};

global constexpr s_upgrade_data c_upgrade_data[] = {
	{.base_cost = 5, .max_upgrades = 3500, .name = "+ drone", .key = c_key_f},
	{.base_cost = 5, .name = "+ player damage", .key = c_key_q},
	{.base_cost = 20, .name = "+ drone damage", .key = c_key_g},
	{.base_cost = 10, .max_upgrades = 20, .name = "+ player speed", .key = c_key_e},
	{.base_cost = 40, .max_upgrades = 30, .name = "+ drone speed", .key = c_key_h},
	{.base_cost = 20, .name = "+ spawn rate", .key = c_key_z},
	{.base_cost = 100, .name = "+ creature tier", .key = c_key_x},
	{.base_cost = 50, .max_upgrades = 40, .name = "+ player range", .key = c_key_r},
	{.base_cost = 80, .max_upgrades = 50, .name = "+ drone range", .key = c_key_k},
	{.base_cost = 5000, .max_upgrades = 1, .name = "x2 harvest", .key = c_key_c},
	{.base_cost = 100, .max_upgrades = 19, .name = "+ drone cargo", .key = c_key_j},
	{.base_cost = 500, .max_upgrades = 4, .name = "+ player chain", .key = c_key_t},
};

enum e_pickup
{
	e_pickup_chain_and_range,
	e_pickup_multi_target_and_range,
	e_pickup_bot_chain_and_range,
	e_pickup_count,
};

struct s_pickup
{
	e_pickup type;
	s_v2 pos;
};

enum e_state
{
	e_state_main_menu,
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

enum e_visual_effect
{
	e_visual_effect_laser,
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


#define for_creature_partial(mname) for(int mname = game->play_state.creature_arr.index_data.lowest_index; mname < game->play_state.creature_arr.index_data.max_index_plus_one; mname += 1)
#define for_bot_partial(mname) for(int mname = game->play_state.bot_arr.index_data.lowest_index; mname < game->play_state.bot_arr.index_data.max_index_plus_one; mname += 1)

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

struct s_lerp
{
	s_v2 prev_pos;
	s_v2 pos;
};

struct s_laser_target
{
	b8 has_prev;
	s_lerp to;
	s_lerp from;
};

struct s_buff
{
	int ticks_left;
};

struct s_player
{
	b8 flip_x;
	b8 dashing;
	int curr_level;
	s64 curr_exp;
	int active_dash_timer;
	int cooldown_dash_timer;
	int harvest_timer;
	float animation_timer;
	s_v2 dash_dir;
	s_v2 prev_pos;
	s_v2 pos;
	s_v2 dash_start;
	s_sarray<s_laser_target, c_max_player_hits * c_max_player_hits> laser_target_arr;
	s_carray<s_buff, e_pickup_count> buff_arr;
};


struct s_creature_arr
{
	s_entity_index_data index_data;
	b8 active[c_max_creatures];
	b8 targeted[c_max_creatures];
	b8 flip_x[c_max_creatures];
	b8 boss[c_max_creatures];
	int id[c_max_creatures];
	int roam_timer[c_max_creatures];
	int curr_health[c_max_creatures];
	int tier[c_max_creatures];
	int tick_when_last_damaged[c_max_creatures];
	float animation_timer[c_max_creatures];
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
	int cargo_count[c_max_bots];
	float animation_timer[c_max_bots];
	float tilt_timer[c_max_bots];
	s_entity_index target[c_max_bots];
	e_bot_state state[c_max_bots];
	s_v2 prev_pos[c_max_bots];
	s_v2 pos[c_max_bots];
	s_sarray<s_laser_target, c_max_bot_hits> laser_target_arr[c_max_bots];
};

struct s_visual_effect
{
	e_visual_effect type;
	s_v2 from;
	s_v2 to;
	s_v4 color;
};

struct s_save_point
{
	s_v2i pos;
};

struct s_particle
{
	b8 attached_to_player;
	float fade;
	float shrink;
	float slowdown;
	s_v2 pos;
	s_v2 dir;
	int z;
	float radius;
	float speed;
	float timer;
	float duration;
	s_v3 color;
};

struct s_particle_multiplier
{
	float radius = 1;
	float speed = 1;
};

struct s_particle_data
{
	float shrink = 1;
	float fade = 1;
	float slowdown;
	float duration = 0.5f;
	float duration_rand;
	float speed = 64;
	float speed_rand;
	float angle;
	float angle_rand = 1;
	float radius = 8;
	float radius_rand;
	s_v3 color = {.x = 0.1f, .y = 0.1f, .z = 0.1f};
	s_v3 color_rand;
};

global constexpr s_particle_data c_buff_particle_data_arr[] = {
	{
		.slowdown = 1,
		.duration = 0.5f,
		.duration_rand = 1,
		.speed = 150,
		.speed_rand = 1,
		.radius = 6,
		.color = v3(0.367f, 0.826f, 0.506f),
		.color_rand = v3(0.5f, 0.5f, 0.5f),
	},
	{
		.slowdown = 1,
		.duration = 0.5f,
		.duration_rand = 1,
		.speed = 150,
		.speed_rand = 1,
		.radius = 6,
		.color = v3(0.889f, 0.538f, 0.617f),
		.color_rand = v3(0.5f, 0.5f, 0.5f),
	},
	{
		.slowdown = 1,
		.duration = 0.5f,
		.duration_rand = 1,
		.speed = 150,
		.speed_rand = 1,
		.radius = 6,
		.color = v3(0.5f, 0.538f, 1.0f),
		.color_rand = v3(0.5f, 0.5f, 0.5f),
	},
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

struct s_ui_optional
{
	b8 darken;
	s_len_str description;
	float font_size;
	float tooltip_font_size;
	float size_x;
	float size_y;
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

struct s_broken_bot
{
	float rotation;
	s_v2 pos;
};

struct s_play_state
{
	b8 has_player_performed_any_action;
	int next_pickup_to_drop;
	int win_ticks;
	u64 level_up_seed;
	e_sub_state sub_state;
	int next_entity_id;
	f64 spawn_creature_timer;
	int resource_count;
	int total_resource;
	s_carray<int, e_upgrade_count> upgrade_level_arr;
	s_sarray<s_particle, c_max_particles> particle_arr;
	s_creature_arr creature_arr;
	s_bot_arr bot_arr;
	s_camera2d cam;
	s_player player;
	int update_count;
	int update_count_at_win_time;
	s_sarray<s_visual_effect, 1024> visual_effect_arr;
	s_sarray<s_pickup, 128> pickup_arr;
	s_sarray<s_broken_bot, 128> broken_bot_arr;
	s_carray<s_v2, c_max_craters> crater_pos_arr;
	s_carray<float, c_max_craters> crater_size_arr;
	s_carray<float, c_max_craters> crater_rotation_arr;
	s_carray<b8, c_max_craters> crater_flip_arr;
	int level_up_triggers;
};


struct s_main_menu
{
	e_sub_state sub_state;
};

struct s_state
{
	b8 is_temporary;
	e_state state;
};

struct s_game
{
	b8 initialized;
	b8 show_hitboxes;
	b8 asking_for_restart_confirmation;
	b8 should_pop_state;
	b8 reset_game;
	b8 pick_free_upgrade_automatically;
	b8 next_state_is_temporary;
	b8 dash_to_keyboard;
	b8 show_total_nectar;
	b8 click_consumed;

	s_sarray<s_state, 16> state_stack;

	s_main_menu main_menu;

	int next_state;
	b8 sound_disabled;
	b8 hide_timer;
	b8 hide_tutorial;

	s_play_state play_state;

	s_carray<s_render_pass*, 2> world_render_pass_arr;
	s_render_pass* ui_render_pass0;
	s_render_pass* ui_render_pass1;
	s_render_pass* ui_render_pass2;
	s_render_pass* ui_render_pass3;

	s_carray<s_sound*, e_sound_count> sound_arr;

	s_animation bot_animation;
	s_animation ant_animation;
	s_animation player_animation;

	s_texture placeholder_texture;
	s_texture base_texture;
	s_texture button_texture;
	s_texture tile_texture;
	s_texture broken_bot_texture;
	s_texture crater_texture;
	s_carray<s_texture, 2> rock_texture_arr;

	s_leaderboard_state leaderboard_state;
	s_input_name_state input_name_state;

	float render_time;
	s_framebuffer* particle_framebuffer;
	s_framebuffer* text_framebuffer;
	s_texture noise;
	s_rng rng;
	s_font* font;
	s_framebuffer* main_fbo;
	s_framebuffer* light_fbo;
	s_sarray<s_leaderboard_entry, c_max_leaderboard_entries> leaderboard_arr;
};

#include "generated/generated_test.cpp"