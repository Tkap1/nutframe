
#define zero {}
#define func static
#define global static
#define tag(...)

global constexpr int c_max_leaderboard_entries = 16;
global constexpr s_v2 c_player_size = v2(128);
global constexpr s_v2 c_creature_size = v2(64);
global constexpr s_v2 c_bot_size = v2(64);
global constexpr s_v2 c_base_size = v2(512);
global constexpr s_v2i c_sprite_size = v2i(64, 64);
global constexpr int c_max_creatures = 2048;
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
global constexpr float c_dash_cooldown = 0.833f;
global constexpr float c_dash_speed = 24;
global constexpr int c_win_animation_duration_in_ticks = c_updates_per_second * 3;
global constexpr int c_invalid_entity = -1000000000;
global constexpr int c_nectar_gain_num_updates = c_updates_per_second * 5;
global constexpr int c_max_broken_drones = 1024;
global constexpr int c_max_statistics_index = 3600; // @Note(tkap, 16/10/2024): 1 save every second, 1 hour worth of data
global constexpr float c_deposit_spawn_rate_buff_per_upgrade = 25;
global constexpr int c_deposit_health_multi_per_upgrade = 60;
global constexpr int c_dash_cooldown_speed_per_upgrade = 60;

enum e_action
{
	e_action_left,
	e_action_right,
	e_action_up,
	e_action_down,
	e_action_dash_to_keyboard,
	e_action_dash_to_mouse,
	e_action_count,
};

enum e_sub_state
{
	e_sub_state_default,
	e_sub_state_pause,
	e_sub_state_defeat,
	e_sub_state_level_up,
	e_sub_state_winning,
	e_sub_state_controls,
};

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
	e_upgrade_broken_bot_spawn,
	e_upgrade_deposit_spawn_rate,
	e_upgrade_deposit_health,
	e_upgrade_dash_cooldown,
	e_upgrade_count,
};

struct s_upgrade_data
{
	int base_cost;
	int max_upgrades = 1000000;
	int key;
};

global constexpr s_upgrade_data c_upgrade_data[] = {
	{.base_cost = 5, .max_upgrades = 3500, .key = c_key_f},
	{.base_cost = 5, .key = c_key_q},
	{.base_cost = 20, .key = c_key_g},
	{.base_cost = 10, .max_upgrades = 20, .key = c_key_e},
	{.base_cost = 40, .max_upgrades = 30, .key = c_key_h},
	{.base_cost = 20, .key = c_key_z},
	{.base_cost = 100, .key = c_key_x},
	{.base_cost = 50, .max_upgrades = 40, .key = c_key_r},
	{.base_cost = 80, .max_upgrades = 50, .key = c_key_k},
	{.base_cost = 5000, .max_upgrades = 1, .key = c_key_c},
	{.base_cost = 100, .max_upgrades = 19, .key = c_key_j},
	{.base_cost = 500, .max_upgrades = 4, .key = c_key_t},
	{.base_cost = 50, .max_upgrades = 15, .key = c_key_l},
	{.base_cost = 50, .max_upgrades = 20, .key = c_key_v},
	{.base_cost = 75, .key = c_key_b},
	{.base_cost = 20, .max_upgrades = 20, .key = c_key_y},
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


struct s_timer
{
	b8 ready;
	float curr;
	float duration;
	float speed = 1.0f;

	void tick();
	void reset();
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
	s_v2 target_pos;
	s_v2 offset;
	float zoom;
	float target_zoom;

	s_v2 world_to_screen(s_v2 v)
	{
		s_v2 result = v;
		result.x -= pos.x;
		result.y -= pos.y;
		result.x += offset.x;
		result.y += offset.y;
		result *= zoom;
		return result;
	}

	s_v2 screen_to_world(s_v2 v)
	{
		s_v2 result = v;
		result.x -= offset.x;
		result.y -= offset.y;
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


struct s_ui_data
{
	b8 present;
	b8 asking_for_confirmation;
};

struct s_button_interaction
{
	b8 hovered;
	b8 clicked;
	s_v2 size;
	s_ui_data* data;
	s_parse_ui_id id;
};

struct s_ui_theme
{
	float font_size;
	float tooltip_font_size;
	s_v2 button_size;
};

global constexpr s_ui_theme c_theme_big = {
	.font_size = 48,
	.button_size = v2(500, 60)
};

global constexpr s_ui_theme c_theme_small = {
	.font_size = 20,
	.button_size = v2(256, 40)
};

global constexpr s_ui_theme c_theme_upgrades0 = {
	.font_size = 24,
	.tooltip_font_size = 38,
	.button_size = v2(54)
};

global constexpr s_ui_theme c_theme_upgrades1 = {
	.font_size = 40,
	.tooltip_font_size = 20,
	.button_size = v2(128)
};

struct s_ui_optional
{
	b8 disabled;
	s_ui_theme theme;
	s_len_str description;
	float darken = 1.0f;
	float grow_from_center;
	float flash;
};

struct s_leaderboard_state
{
	b8 received;
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

struct s_auto_timer
{
	float curr;
	float duration;
	float speed = 1.0f;

	int tick();
	float get_duration_in_seconds();
	float get_rate_in_seconds();
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
	s_sarray<s_particle, c_max_particles> particle_arr;
	s_camera2d cam;
	int update_count;
	int update_count_at_win_time;
	s_sarray<s_visual_effect, 1024> visual_effect_arr;
};


struct s_main_menu
{
	e_sub_state sub_state;
};

struct s_ui_iterator
{
	int index;
	s_ui_data* element;
};

struct s_damage_creature
{
	b8 creature_died;
	int resource_gain_from_deposit;
};

struct s_hold_input
{
	b8 left;
	b8 right;
	b8 up;
	b8 down;
};

struct s_press_input
{
	b8 dash_to_keyboard;
	b8 dash_to_mouse;
};

struct s_tooltip
{
	s_v2 pos;
	s_v2 size;
	s_v2 text_pos;
};

enum e_animator
{
	e_animator_curve,
	e_animator_float,
	e_animator_point,
	e_animator_color,
};

#define X(fname, ename) ename,
enum e_ease
{
	m_advanced_easings
};
#undef X

struct s_animator_property
{
	e_animator type;
	e_ease ease_mode;
	float duration;
	float delay;
	void* ptr;
	union
	{
		struct
		{
			s_v2 a;
			s_v2 b;
			s_v2 pivot;
		} curve;

		struct
		{
			s_v4 a;
			s_v4 b;
		} color;

		struct
		{
			float a;
			float b;
		} nfloat;

		struct
		{
			s_v2 a;
		} point;
	};
};

struct s_animator
{
	// #ifdef m_debug
	b8 needs_wait_call = true;
	// #endif // m_debug

	s_carray<float, 8> step_start_time_arr;
	s_carray<int, 8> result_on_end;
	int step_count;
	int curr_step;
	float total_duration;
	s_carray<float, 8> step_duration_arr;
	s_carray<s_sarray<s_animator_property, 8>, 8> property_arr;
};

#include "generated/generated_test.cpp"