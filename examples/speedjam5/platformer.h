
static constexpr int c_tiles_right = 512;
static constexpr int c_tiles_down = 2048;
static constexpr int c_editor_tile_size = 32;
static constexpr int c_play_tile_size = 1;
static constexpr int c_updates_per_second = 240;
static constexpr int c_max_save_points = 64;
static constexpr int c_map_version = 1;
static constexpr int c_tile_version = 1;
static constexpr int c_save_point_version = 1;
static constexpr int c_end_point_version = 1;
static constexpr int c_max_jumps = 1;
static constexpr int c_max_projectiles = 128;
static constexpr int c_projectile_duration = 2000;
static constexpr int c_max_leaderboard_entries = 128;
static constexpr int c_max_particles = 8192;
static constexpr int c_max_death_sounds = 3;

static constexpr float c_player_z = 0.0f;
static constexpr float c_gravity = 0.001f;
static constexpr float c_small = epsilon;
static constexpr float c_particle_z = c_player_z - 0.01f;

static constexpr s_v2 c_projectile_visual_size = v2(1.0f);
static constexpr s_v2 c_projectile_collision_size = v2(0.25f);
static constexpr s_v2 c_player_visual_size = v2(c_play_tile_size);
static constexpr s_v2 c_player_collision_size = v2(c_play_tile_size * 0.9f, (float)c_play_tile_size);
static constexpr s_v2 c_save_point_visual_size = v2(c_play_tile_size);
static constexpr s_v2 c_save_point_collision_size = v2(c_play_tile_size * 1.5f);
static constexpr s_v2 c_end_point_size = v2(c_play_tile_size);

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


enum e_state
{
	e_state_play,
	e_state_editor,
	e_state_victory,
};


struct s_tile_collision
{
	e_tile tile;
	s_v2 tile_center;
};

enum e_visual_effect
{
	e_visual_effect_projectile_explosion,
};

struct s_end_point
{
	s_v2i pos;
};

struct s_editor
{
	e_tile curr_tile;
};

struct s_player
{
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

struct s_map
{
	s_end_point end_point;
	s_carray2<e_tile, c_tiles_down, c_tiles_right> tile_arr;
	s_sarray<s_save_point, c_max_save_points> save_point_arr;
};


static s_v2i pos_to_index(s_v2 pos, int tile_size);
static b8 is_index_valid(s_v2i index);
static s_recti get_3d_tile_bounds(s_camera3d cam);
static void load_map(s_map* map, s_platform_data* platform_data);
static b8 index_has_tile(s_v2i index);
static s_sarray<s_tile_collision, 16> get_tile_collisions(s_v2 pos, s_v2 size, int tile_size, int tile_blacklist_mask = 0);
static s_v2 index_to_pos(s_v2i index, int tile_size);
static void do_particles(int count, s_v3 pos, s_particle_data data);