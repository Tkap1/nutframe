#define m_game

#pragma comment(lib, "Shlwapi.lib")

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#include <shlwapi.h>

#pragma warning(push, 0)
#define CUTE_C2_IMPLEMENTATION
#include "cute_c2.h"
#pragma warning(pop)

#include "../../src/platform_shared.h"
#include "variables.h"


#define zero {}
#define func static
#define global static
#define null NULL

global constexpr char* c_python_path = "C:/Users/34687/AppData/Local/Programs/Python/Python311/python.exe";

// @Note(tkap, 22/10/2023): If we change this we break the stored maps! Careful!
global constexpr int c_max_tiles = 128;
global constexpr int c_max_balls = 128;
global constexpr float c_seconds_after_first_beat = 120.0f;
global constexpr s_v2 c_base_res = {1920, 1080};
global constexpr s_v2 c_half_res = {c_base_res.x / 2.0f, c_base_res.y / 2.0f};
global constexpr f64 c_updates_per_second = 240.0;
global constexpr float c_delta = (float)(1.0 / c_updates_per_second);
global constexpr int c_starting_map = 0;
global constexpr float c_max_inactivity_time = 120.0f;

enum e_state
{
	e_state_mode_select,
	e_state_play,
	e_state_stats,
	e_state_victory,
	e_state_map_editor,
};

enum e_game_mode
{
	e_game_mode_default,
	e_game_mode_first_to_hole,
	e_game_mode_count,
};

enum e_game_modifier
{
	e_game_modifier_all_shoot_at_once,
	e_game_modifier_random_map_order,
	e_game_modifier_count,
};

enum e_layer
{
	e_layer_background,
	e_layer_tile,
	e_layer_hole,
	e_layer_ball_angle_indicator,
	e_layer_ball,
	e_layer_particle,
	e_layer_tile_ghost,
	e_layer_ui,
};

enum e_map_editor_state
{
	e_map_editor_state_default,
	e_map_editor_state_saving,
	e_map_editor_state_loading,
};

enum e_rotation : u8
{
	e_rotation_right,
	e_rotation_down,
	e_rotation_left,
	e_rotation_top,
	e_rotation_count,
};

global constexpr float c_rotation_to_rad[4] = {
	0, tau * 0.25f, tau * 0.5f, tau * 0.75f
};

enum e_tile
{
	e_tile_wall,
	e_tile_default,
	e_tile_acceleration,
	e_tile_directional,
	e_tile_sand,
	e_tile_water,
	e_tile_one_way,
	e_tile_ice,
	e_tile_bouncy_wall,
	e_tile_count,
};

enum e_map
{
	e_map_001,
	e_map_002,
	e_map_003,
	e_map_004,
	e_map_005,
	e_map_006,
	e_map_007,
	e_map_008,
	e_map_009,
	e_map_010,
	// e_map_011,
	e_map_count,
};

struct s_collision_data
{
	u8 tile_type;
	c2Manifold manifold;
};

struct s_ball
{
	char name[64];
	float inactivity_time;
	c2Circle c;
	s_v2 vel;
	s_v2 pos_before_last_push;
	s_v4 color;
	int push_count;
	s_v2 rotation;
	int maps_beaten;
	b8 has_push_queued;
	s_v2 queued_push; // @Note(tkap, 27/10/2023): For all shoot at once mode
	float last_collision_time; // @Note(tkap, 27/10/2023): This is to prevent the crazy sound when someone gets stuck between a directional booster
	// and a wall
};

struct s_name_and_push_count
{
	b8 beat_level;
	int total_count;
	int level_count;
	int maps_beaten;
	char* name;
};

static bool operator>(s_name_and_push_count a, s_name_and_push_count b)
{
	return a.total_count > b.total_count;
}

global constexpr int c_map_version = 3;
#pragma pack(push, 1)
struct s_map
{
	float zoom;
	s_carray2<b8, c_max_tiles, c_max_tiles> tiles_active;
	s_carray2<u8, c_max_tiles, c_max_tiles> tiles;
	s_v2i hole;
	s_v2i spawn;
	s_carray2<u8, c_max_tiles, c_max_tiles> rotation;
};
#pragma pack(pop)

struct s_str
{
	char* data;
	int len;
};

struct s_map_editor
{
	e_map_editor_state state;
	int save_index;
	int curr_tile;
	s_map map;
	s_v2i shift_index;
	s_v2i select_start;
	s_sarray<s_v2i, c_max_tiles * c_max_tiles> selected;
	int rotation;
};

struct s_game_transient
{
	s_carray<int, c_max_balls> push_count;
	s_carray<b8, c_max_balls> has_beat_level;
	s_v2 spawn_offset;
	float first_beat_time;
	float stats_timer;
};

struct s_particle
{
	float fade;
	float shrink;
	float slowdown;
	s_v2 pos;
	s_v2 dir;
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
	s_v3 color = {0.1f, 0.1f, 0.1f};
	s_v3 color_rand;
};

struct s_game
{
	b8 initialized;
	b8 reset_level;
	e_game_mode mode;
	s_carray<b8, e_game_modifier_count> modifiers;
	int last_chat_file_offset;
	e_state state;
	s_rng rng;
	s_sarray<s_ball, c_max_balls> balls;
	s_font* font;
	int curr_map;
	s_map_editor editor;
	s_carray<s_map, e_map_count> maps;
	s_texture angle_indicator;
	s_carray<s_sound*, 3> push_sounds;
	s_sound* collide_sound;
	s_sound* win_sound;
	s_sound* water_sound;
	s_game_transient transient;
	s_texture directional_tile_texture;
	s_texture sand;
	s_texture water;
	s_texture noise;
	s_texture one_way;
	s_texture ice;
	s_texture bouncy_wall;
	float total_time;
	s_framebuffer* particle_framebuffer;
	s_sarray<s_particle, 16384> particles;
	s_sarray<int, e_map_count> random_maps;
};

global s_input* g_input;
global s_game* game;
global s_game_renderer* g_r;
global s_v2 g_mouse;


func s_sarray<s_collision_data, 16> get_collisions(c2Circle circle, s_v2 vel, s_map* map);
func s_sarray<s_v2i, 16> get_interactive_tile_collisions(c2Circle circle, s_map* map);
func s_v2 v2(c2v v);
func void make_process_close_when_app_closes(HANDLE process);
func s_sarray<s_str, 128> get_chat_messages(s_platform_data* platform_data);
func int parse_int(char* in, char** out);
func int get_ball_by_name(s_str name);
func b8 is_valid_index(s_v2i index, int width, int height);
func void draw_map(s_map* map);
func void load_map(char* file_path, s_platform_data* platform_data, s_map* out_map);
func s_v2 tile_index_to_pos(s_v2i index);
func s_v2 tile_index_to_tile_center(s_v2i index);
func b8 has_anyone_beaten_level();
func b8 has_everyone_beaten_level();
func void move_ball_to_spawn(s_ball* ball, s_map* map);
func b8 are_we_on_last_map();
func b8 are_we_placing_tile(s_map_editor* editor);
func b8 is_something_selected(s_map_editor* editor);
func void draw_tile(s_game_renderer* renderer, s_v2 pos, e_layer layer, s_v2 size, int type, u8 rotation, s_render_data render_data = {}, s_transform t = {});
func b8 are_we_placing_spawn_or_hole(s_map_editor* editor);
func void editor_on_left_mouse_down(s_map_editor* editor);
func void editor_on_left_mouse_up(s_map_editor* editor);
func void editor_on_left_mouse_pressed(s_map_editor* editor);
func b8 is_tile_active(s_map* map, int x, int y);
func b8 is_tile_active(s_map* map, s_v2i index);
func c2v make_c2v(s_v2 v);
func int get_worst_pushes_that_beat_level();
func void do_particles(int count, s_v2 pos, s_particle_data data);
func void beat_level_particles(s_v2 pos);
func void do_collision_particles(s_v2 pos, float mag, s_v4 color);
func void maybe_add_new_ball(s_str user);
func bool compare_maps_beaten(s_name_and_push_count a, s_name_and_push_count b);
func void set_globals(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, s_input* input);

#ifdef m_build_dll
extern "C" {
#endif // m_build_dll
m_dll_export void init_game(s_platform_data* platform_data)
{
	platform_data->set_window_size((int)c_base_res.x, (int)c_base_res.y);
	platform_data->update_delay = 1.0 / c_updates_per_second;
}

m_dll_export void update(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{
	static_assert(sizeof(s_game) <= c_game_memory);
	set_globals(platform_data, game_memory, renderer, &platform_data->logic_input);

	if(!game->initialized) {
		game->initialized = true;
		game->rng.seed = platform_data->get_random_seed();
		game->font = &renderer->fonts[0];
		g_r->set_vsync(true);
		game->reset_level = true;
		platform_data->variables_path = "examples/golf/variables.h";
		game->angle_indicator = g_r->load_texture(g_r, "examples/golf/angle_indicator.png");
		game->directional_tile_texture = g_r->load_texture(g_r, "examples/golf/directional_tile.png");
		game->sand = g_r->load_texture(g_r, "examples/golf/sand.png");
		game->water = g_r->load_texture(g_r, "examples/golf/water.png");
		game->noise = g_r->load_texture(g_r, "examples/golf/noise.png");
		game->one_way = g_r->load_texture(g_r, "examples/golf/one_way.png");
		game->ice = g_r->load_texture(g_r, "examples/golf/ice.png");
		game->bouncy_wall = g_r->load_texture(g_r, "examples/golf/bouncy_wall.png");
		game->push_sounds[0] = platform_data->load_sound(platform_data, "examples/golf/push1.wav", platform_data->frame_arena);
		game->push_sounds[1] = platform_data->load_sound(platform_data, "examples/golf/push2.wav", platform_data->frame_arena);
		game->push_sounds[2] = platform_data->load_sound(platform_data, "examples/golf/push3.wav", platform_data->frame_arena);
		game->collide_sound = platform_data->load_sound(platform_data, "examples/golf/collide.wav", platform_data->frame_arena);
		game->water_sound = platform_data->load_sound(platform_data, "examples/golf/water.wav", platform_data->frame_arena);
		game->win_sound = platform_data->load_sound(platform_data, "examples/golf/win.wav", platform_data->frame_arena);
		game->particle_framebuffer = g_r->make_framebuffer(renderer, false);

		game->editor.curr_tile = -1;

		game->curr_map = c_starting_map;
		for(int map_i = 0; map_i < e_map_count; map_i++) {
			char* file_path = format_text("map%i", map_i);
			load_map(file_path, platform_data, &game->maps[map_i]);
		}

		DeleteFileA("chat_messages.txt");

		{
			STARTUPINFO startup_info = zero;
			PROCESS_INFORMATION process_info = zero;
			BOOL result = CreateProcessA(
				c_python_path,
				"python.exe examples/golf/read_twitch_chat.py",
				null,
				null,
				FALSE,
				0,
				// CREATE_NEW_CONSOLE,
				null,
				null,
				&startup_info,
				&process_info
			);
			assert(result);

			make_process_close_when_app_closes(process_info.hProcess);
		}
	}

	game->total_time += c_delta;

	#ifdef m_debug
	if(platform_data->loaded_a_state) {
		game->last_chat_file_offset = 0;
		char* path = "chat_messages.txt";
		if(PathFileExistsA(path)) {
			while(DeleteFile(path) == 0) {}
		}
	}
	#endif // m_debug

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		handle twitch messages start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	s_sarray<s_str, 128> msgs = get_chat_messages(platform_data);
	foreach_val(msg_i, msg, msgs) {
		s_str user = zero;
		int index = str_find_from_left(msg.data, msg.len, ":", 1);
		user.data = msg.data;
		user.len = index;
		s_str content = zero;
		content.data = msg.data + index + 1;
		content.len = msg.len - index - 1;

		if(game->state == e_state_play) {
			if(content.len >= 4 && strncmp_ignore_case(content.data, "join", 4)) {
				maybe_add_new_ball(user);
			}
		}

		// printf("%.*s: %.*s\n", user.len, user.data, content.len, content.data);
		if(game->state == e_state_play && content.len > 4 && strncmp_ignore_case(content.data, "push", 4)) {
			char* out = null;
			char* out2 = null;
			int angle = parse_int(content.data + 4, &out);
			if(!out) { continue; }
			int strength = parse_int(out, &out2);
			if(!out2) { continue; }
			strength = clamp(strength, 1, 100);
			// printf("%i, %i\n", angle, strength);

			int ball_index = get_ball_by_name(user);
			if(ball_index >= 0) {
				s_ball* ball = &game->balls[ball_index];
				ball->inactivity_time = 0;
				s_v2 push_vector = v2_from_angle(deg_to_rad((float)angle)) * range_lerp((float)strength, 1, 100, 25, 2000);
				ball->has_push_queued = true;
				ball->queued_push = push_vector;

				if(!game->modifiers[e_game_modifier_all_shoot_at_once]) {
					ball->vel += push_vector;
					ball->pos_before_last_push = v2(ball->c.p);
					platform_data->play_sound(game->push_sounds[game->rng.randu() % array_count(game->push_sounds)]);

					if(!game->transient.has_beat_level[ball_index]) {
						game->transient.push_count[ball_index] += 1;
						ball->push_count += 1;
					}
				}

			}
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		handle twitch messages end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	switch(game->state) {
		case e_state_play: {
			if(game->reset_level) {
				memset(&game->transient, 0, sizeof(game->transient));
				game->reset_level = false;
				game->transient.spawn_offset.x = (float)game->rng.randf2() * c_ball_radius * 2;
				game->transient.spawn_offset.y = (float)game->rng.randf2() * c_ball_radius * 2;

				s_map* map = &game->maps[game->curr_map];
				foreach_ptr(ball_i, ball, game->balls) {
					move_ball_to_spawn(ball, map);
					ball->inactivity_time = 0;
					ball->vel = zero;
				}
			}

			#ifdef m_debug
			if(is_key_pressed(g_input, c_key_f5)) {
				// @Hack(tkap, 27/10/2023):
				g_input->keys[c_key_f5].count = 0;

				s_str name = zero;
				char buffer[1024] = zero;
				for(int i = 0; i < 6; i++) {
					buffer[i] = game->rng.randu() % ('z' - 'a') + 'a';
				}
				name.data = buffer;
				name.len = 6;
				maybe_add_new_ball(name);
			}
			if(is_key_pressed(g_input, c_key_f2)) {
				game->state = e_state_map_editor;
			}
			#endif // m_debug

			if(game->modifiers[e_game_modifier_all_shoot_at_once]) {
				b8 all_ready = true;
				foreach_ptr(ball_i, ball, game->balls) {
					if(game->transient.has_beat_level[ball_i]) { continue; }
					if(!ball->has_push_queued) {
						all_ready = false;
					}
					else {
						ball->inactivity_time = 0;
					}
				}
				if(all_ready) {
					platform_data->play_sound(game->push_sounds[game->rng.randu() % array_count(game->push_sounds)]);
					foreach_ptr(ball_i, ball, game->balls) {
						ball->pos_before_last_push = v2(ball->c.p);
						ball->vel += ball->queued_push;
						ball->has_push_queued = false;
						game->transient.push_count[ball_i] += 1;
						ball->push_count += 1;
					}
				}
			}

			{
				s_v2 pos = c_base_res * v2(0.5f, 0.5f);
				if(has_anyone_beaten_level()) {
					b8 advance_state = false;
					if(game->mode == e_game_mode_first_to_hole) {
						advance_state = true;
					}
					else {
						float time_left = c_seconds_after_first_beat - (game->total_time - game->transient.first_beat_time);
						if(time_left <= 0 || has_everyone_beaten_level()) {
							advance_state = true;
						}
					}
					if(advance_state) {
						// @Note(tkap, 24/10/2023): Add artificial pushes to everyone who didn't beat the level
						foreach_ptr(ball_i, ball, game->balls) {
							if(!game->transient.has_beat_level[ball_i]) {
								int didnt_beat_level_pushes = game->transient.push_count[ball_i];
								int worst_pushes_that_beat_level = get_worst_pushes_that_beat_level() + 1;
								int diff = worst_pushes_that_beat_level - didnt_beat_level_pushes;
								ball->push_count += diff;
								game->transient.push_count[ball_i] = worst_pushes_that_beat_level;
							}
						}
						game->state = e_state_stats;
					}
				}
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		ball update start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_ptr(ball_i, ball, game->balls) {
				s_map* map = &game->maps[game->curr_map];
				ball->c.r = c_ball_radius;
				b8 in_sand = false;
				b8 in_ice = false;
				if(game->balls.count > 0 && !game->transient.has_beat_level[ball_i]) {
					ball->inactivity_time += c_delta;
					if(ball->inactivity_time >= c_max_inactivity_time) {
						game->transient.push_count.swap(ball_i, game->balls.count - 1);
						game->transient.has_beat_level.swap(ball_i, game->balls.count - 1);
						game->balls.remove_and_swap(ball_i);
						ball_i -= 1;
						continue;
					}
				}

				{
					auto collisions = get_interactive_tile_collisions(ball->c, map);
					float mul = 1.0f;
					foreach_val(tile_i, tile, collisions) {
						u8 tile_type = map->tiles[tile.y][tile.x];
						if(tile_type == e_tile_acceleration) {
							mul += 0.02f;
						}
						else if(tile_type == e_tile_directional) {
							ball->vel += v2_from_angle(c_rotation_to_rad[map->rotation[tile.y][tile.x]]) * 4;
						}
						else if(tile_type == e_tile_sand) {
							in_sand = true;
						}
						else if(tile_type == e_tile_ice) {
							in_ice = true;
						}
						else if(tile_type == e_tile_water) {
							float d = v2_distance(v2(ball->c.p), tile_index_to_tile_center(tile));
							if(d <= c_tile_size * 0.5f) {
								ball->c.p = make_c2v(ball->pos_before_last_push);
								ball->vel = zero;
								mul = 0;
								platform_data->play_sound(game->water_sound);
								break;
							}
						}
					}
					ball->vel *= mul;
				}
				ball->vel.x = clamp(ball->vel.x, -3000.0f, 3000.0f);
				ball->vel.y = clamp(ball->vel.y, -3000.0f, 3000.0f);

				for(int i = 0; i < 10; i++) {
					s_v2 movement = ball->vel * 0.1f;
					ball->rotation += movement;

					ball->c.p.x += movement.x * c_delta;
					ball->c.p.y += movement.y * c_delta;
					auto collisions = get_collisions(ball->c, ball->vel, map);
					if(collisions.count > 0) {
						c2Manifold c = collisions[0].manifold;

						ball->c.p.x -= (c.depths[0]) * c.n.x;
						ball->c.p.y -= (c.depths[0]) * c.n.y;
						if(game->total_time - ball->last_collision_time > 0.1f) {
							platform_data->play_sound(game->collide_sound);
						}
						ball->last_collision_time = game->total_time;

						float mag = v2_length(ball->vel);
						do_collision_particles(v2(c.contact_points[0]), mag, ball->color);

						float speed_mul = 0.9f;
						s_v2 speed_add = zero;
						if(collisions[0].tile_type == e_tile_bouncy_wall) {
							speed_mul = 1.0f;
							speed_add = v2(400);
						}
						s_v2 reflect = v2_reflect(ball->vel, v2(c.n));
						ball->vel = reflect * speed_mul + (speed_add * v2_normalized(reflect));

						break;
					}
				}

				if(is_key_pressed(g_input, c_left_mouse)) {
					game->balls[0].c.p.x = g_mouse.x;
					game->balls[0].c.p.y = g_mouse.y;
				}

				s_v2 hole_pos = tile_index_to_pos(map->hole) + v2(c_tile_size * 0.5f);
				b8 in_hole = c2CircletoCircle(ball->c, {.p = {hole_pos.x, hole_pos.y}, .r = c_ball_radius * 2.0f}) != 0;

				if(in_ice) {
					ball->vel *= 0.999f;
				}
				else if(in_sand) {
					ball->vel *= 0.96f;
				}
				else {
					ball->vel *= 0.995f;
				}
				s_v4 color = make_color(0.5f);
				if(game->transient.has_beat_level[ball_i]) {
					color = brighter(rgb(0xA4BB96), 1.2f);
				}

				// @Note(tkap, 22/10/2023): Check for win
				float length = v2_length(ball->vel);
				if(in_hole && length < 0.3f && !game->transient.has_beat_level[ball_i]) {
					platform_data->play_sound(game->win_sound);
					printf("%s has beaten the level!\n", ball->name);
					beat_level_particles(v2(ball->c.p));
					ball->maps_beaten += 1;

					if(!has_anyone_beaten_level()) {
						game->transient.first_beat_time = game->total_time;
					}
					game->transient.has_beat_level[ball_i] = true;
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		ball update end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		} break;

		case e_state_stats: {
			game->transient.stats_timer += c_delta;
			if(game->transient.stats_timer >= 10 || is_key_pressed(g_input, c_key_enter)) {
				game->reset_level = true;
				if(are_we_on_last_map()) {
					game->state = e_state_victory;
				}
				else {
					game->state = e_state_play;
					if(game->modifiers[e_game_modifier_random_map_order]) {
						game->curr_map = game->random_maps.pop();
					}
					else {
						game->curr_map += 1;
					}
				}
			}

		} break;
	}
}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{
	set_globals(platform_data, game_memory, renderer, &platform_data->render_input);

	live_variable(&platform_data->vars, c_tile_size, 4, 128, true);
	live_variable(&platform_data->vars, c_ball_impulse, 100.0f, 2000.0f, true);
	live_variable(&platform_data->vars, c_ball_radius, 16.0f, 128.0f, true);
	live_variable(&platform_data->vars, c_angle_indicator_x, 0.0f, 1.0f, true);
	live_variable(&platform_data->vars, c_angle_indicator_y, 0.0f, 1.0f, true);

	switch(game->state) {

		case e_state_mode_select: {
			{
				constexpr char* c_mode_names[] = {
					"Normal", "First to Hole",
				};
				static_assert(e_game_mode_count == array_count(c_mode_names));

				if(is_key_pressed(g_input, c_key_up)) {
					game->mode = (e_game_mode)circular_index(game->mode - 1, e_game_mode_count);
				}
				if(is_key_pressed(g_input, c_key_down)) {
					game->mode = (e_game_mode)circular_index(game->mode + 1, e_game_mode_count);
				}

				constexpr float font_size = 64;
				draw_text(g_r, "Select mode", c_base_res * v2(0.5f, 0.2f), e_layer_ui, font_size, make_color(1), true, game->font);
				s_v2 pos = c_base_res * v2(0.5f, 0.35f);
				for(int mode_i = 0; mode_i < e_game_mode_count; mode_i++) {
					s_v4 color = make_color(0.5f);
					if(game->mode == mode_i) {
						color = make_color(1);
					}
					draw_text(g_r, c_mode_names[mode_i], pos, e_layer_ui, font_size, color, true, game->font);
					pos.y += font_size;
				}

				if(is_key_pressed(g_input, c_key_enter)) {
					game->state = e_state_play;
					game->reset_level = true;
					for(int i = 0; i < e_map_count; i++) {
						game->random_maps.add(i);
					}
					game->random_maps.shuffle(&game->rng);
					if(game->modifiers[e_game_modifier_random_map_order]) {
						game->curr_map = game->random_maps.pop();
					}
				}
			}

			{
				s_v2 pos = c_base_res * v2(0.01f, 0.6f);
				constexpr char* c_names[] = {
					"All shoot at once", "Random map order",
				};
				static_assert(array_count(c_names) == e_game_modifier_count);
				for(int modifier_i = 0; modifier_i < e_game_modifier_count; modifier_i++) {
					char* text = c_names[modifier_i];
					draw_text(g_r, text, pos, e_layer_ui, 48.0f, make_color(1), false, game->font);
					platform_data->ui_checkbox(g_r, text, pos + v2(500.0f, -12.0f), v2(64), &game->modifiers[modifier_i], g_input, g_mouse);
					pos.y += 68.0f;
				}
			}
		} break;

		case e_state_play: {

			s_map* map = &game->maps[game->curr_map];
			draw_map(map);
			draw_texture(g_r, c_base_res * v2(c_angle_indicator_x, c_angle_indicator_y), e_layer_ui, v2(128), make_color(1), game->angle_indicator);

			{
				s_v2 pos = c_base_res * v2(0.575f, 0.2f);
				s_v4 color = hsv_to_rgb(v3(sinf2((float)renderer->total_time * 0.5f), 1.0f, 1.0f));
				draw_text(g_r, "Type join to play", pos, e_layer_ui, 24.0f, color, false, game->font);
				pos.y += 24.0f;
				draw_text(g_r, "Type push angle strength", pos, e_layer_ui, 24.0f, color, false, game->font);
			}

			s_v2 pos = c_base_res * v2(0.5f, 0.5f);
			if(game->mode != e_game_mode_first_to_hole && has_anyone_beaten_level()) {
				float time_left = c_seconds_after_first_beat - (game->total_time - game->transient.first_beat_time);
				draw_text(g_r, format_text("%.0f", time_left), pos, e_layer_ui, 64.0f, make_color(1), true, game->font);
			}

			foreach_val(ball_i, ball, game->balls) {
				s_v2 hole_pos = tile_index_to_pos(map->hole) + v2(c_tile_size * 0.5f);
				b8 in_hole = c2CircletoCircle(ball.c, {.p = {hole_pos.x, hole_pos.y}, .r = c_ball_radius * 2.0f}) != 0;

				s_v4 color = ball.color;
				if(game->transient.has_beat_level[ball_i]) {
					color = brighter(rgb(0xA4BB96), 1.2f);
				}
				draw_texture(g_r, v2(ball.c.p), e_layer_ball, v2(ball.c.r * 2.0f), color, game->noise, {}, {.effect_id = 2, .texture_size = ball.rotation});
				if(!in_hole) {
					draw_text(g_r, ball.name, v2(ball.c.p), 50, 24, make_color(1), true, game->font);

					// @Note(tkap, 27/10/2023): Angle indicator from ball
					for(int j = 0; j < 2; j++) {
						float a = j * 0.125f;
						for(int i = 0; i < 2; i++)
						{
							float angle = (i / 4.0f + a) * tau;
							s_v2 d = v2_from_angle(angle) * 64.0f;
							draw_line(g_r, v2(ball.c.p) - d, v2(ball.c.p) + d, e_layer_ball_angle_indicator, 8.0f, make_color(0.1f), {.blend_mode = e_blend_mode_additive, .framebuffer = game->particle_framebuffer});
						}
					}
				}

			}

		} break;

		case e_state_stats: {
			s_sarray<s_name_and_push_count, c_max_balls> arr;
			foreach_ptr(ball_i, ball, game->balls) {
				s_name_and_push_count x = zero;
				x.name = ball->name;
				x.beat_level = game->transient.has_beat_level[ball_i];
				x.total_count = ball->push_count;
				x.level_count = game->transient.push_count[ball_i];
				x.maps_beaten = ball->maps_beaten;
				arr.add(x);
			}
			if(game->mode == e_game_mode_first_to_hole) {
				arr.small_sort(compare_maps_beaten);
			}
			else {
				arr.small_sort();
			}

			float name_x = c_base_res.x * 0.15f;
			float level_x = c_base_res.x * 0.4f;
			float total_x = c_base_res.x * 0.6f;
			float beaten_x = c_base_res.x * 0.8f;
			float y = c_base_res.y * 0.05f;
			s_v2 pos = c_base_res * v2(0.5f, 0.1f);
			constexpr float font_size = 64.0f;
			draw_text(g_r, "Name", v2(name_x, y), e_layer_ui, font_size, make_color(1), false, game->font);
			draw_text(g_r, "Level", v2(level_x, y), e_layer_ui, font_size, make_color(1), false, game->font);
			draw_text(g_r, "Total", v2(total_x, y), e_layer_ui, font_size, make_color(1), false, game->font);
			draw_text(g_r, "Beaten", v2(beaten_x, y), e_layer_ui, font_size, make_color(1), false, game->font);
			y += font_size * 2;
			foreach_val(x_i, x, arr) {
				s_v4 color = make_color(1);
				if(!x.beat_level) {
					color = rgb(0x932437);
				}
				draw_text(g_r, x.name, v2(name_x, y), e_layer_ui, font_size, color, false, game->font);
				draw_text(g_r, format_text("%i", x.level_count), v2(level_x, y), e_layer_ui, font_size, color, false, game->font);
				draw_text(g_r, format_text("%i", x.total_count), v2(total_x, y), e_layer_ui, font_size, color, false, game->font);
				draw_text(g_r, format_text("%i", x.maps_beaten), v2(beaten_x, y), e_layer_ui, font_size, color, false, game->font);
				y += font_size;
			}

			draw_rect(g_r, c_half_res, e_layer_background, c_base_res, make_color(1), zero, {.effect_id = 3});
		} break;

		case e_state_victory: {
			s_v2 pos = c_base_res * v2(0.5f, 0.5f);
			draw_text(g_r, "That's it lol", pos, e_layer_ui, 64.0f, make_color(1), true, game->font);
			draw_rect(g_r, c_half_res, e_layer_background, c_base_res, make_color(1), zero, {.effect_id = 3});
		} break;

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		map editor start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_state_map_editor: {
			s_map_editor* editor = &game->editor;

			if(editor->state == e_map_editor_state_saving || editor->state == e_map_editor_state_loading) {
				if(is_key_pressed(g_input, c_key_up)) {
					editor->save_index = circular_index(editor->save_index - 1, e_map_count);
				}
				if(is_key_pressed(g_input, c_key_down)) {
					editor->save_index = circular_index(editor->save_index + 1, e_map_count);
				}

				if(is_key_pressed(g_input, c_key_escape)) {
					editor->state = e_map_editor_state_default;
				}
			}

			switch(editor->state) {

				case e_map_editor_state_default: {
					b8 click_handled = false;

					s_v2i index = v2i(floorfi(g_mouse.x / c_tile_size), floorfi(g_mouse.y / c_tile_size));

					if(is_key_pressed(g_input, c_key_escape)) {
						if(is_something_selected(editor)) {
							editor->selected.count = 0;
						}
						else if(are_we_placing_tile(editor) || are_we_placing_spawn_or_hole(editor)) {
							editor->curr_tile = -1;
						}
					}

					if(!is_something_selected(editor) && !are_we_placing_tile(editor) && !are_we_placing_spawn_or_hole(editor)) {
						s_v2 pos = c_base_res * v2(0.7f, 0.1f);
						int pos_x = 0;
						int pos_y = 0;
						for(int i = 0; i < e_tile_count; i++) {
							s_v2 temp = pos;
							temp.x += pos_x % 3 * 68.0f;
							temp.y += pos_y / 3 * 68.0f;
							s_ui_interaction interaction = platform_data->ui_button(g_r, format_text("%i", i), temp, v2(64), game->font, 32.0f, g_input, g_mouse);
							if(interaction.state == e_ui_pressed) { click_handled = true; }
							else if(interaction.state == e_ui_active) {
								editor->curr_tile = (u8)i;
							}
							pos_x += 1;
							pos_y += 1;
						}

						{
							s_v2 temp = pos;
							temp.x += pos_x % 3 * 68.0f;
							temp.y += pos_y / 3 * 68.0f;
							s_ui_interaction interaction = platform_data->ui_button(g_r, "Hole", temp, v2(64), game->font, 32.0f, g_input, g_mouse);
							if(interaction.state == e_ui_pressed) { click_handled = true; }
							else if(interaction.state == e_ui_active) {
								editor->curr_tile = 69;
							}
							pos_x += 1;
							pos_y += 1;
						}
						{
							s_v2 temp = pos;
							temp.x += pos_x % 3 * 68.0f;
							temp.y += pos_y / 3 * 68.0f;
							s_ui_interaction interaction = platform_data->ui_button(g_r, "Spawn", temp, v2(64), game->font, 32.0f, g_input, g_mouse);
							if(interaction.state == e_ui_pressed) { click_handled = true; }
							else if(interaction.state == e_ui_active) {
								editor->curr_tile = 70;
							}
							pos_x += 1;
							pos_y += 1;
						}
					}

					if(are_we_placing_tile(editor) && is_key_pressed(g_input, c_key_r)) {
						int val = 1;
						if(is_key_down(g_input, c_key_left_shift)) {
							val = -1;
						}
						editor->rotation = circular_index(editor->rotation + val, e_rotation_count);
					}

					if(is_key_pressed(g_input, c_key_left_shift)) {
						editor->shift_index = index;
					}
					else if(is_key_up(g_input, c_key_left_shift)) {
						editor->shift_index = index;
					}
					if(!click_handled && is_key_pressed(g_input, c_left_mouse)) {
						click_handled = true;
						editor_on_left_mouse_pressed(editor);
					}
					if(!click_handled && is_key_down(g_input, c_left_mouse)) {
						editor_on_left_mouse_down(editor);
					}
					if(!click_handled && is_key_released(g_input, c_left_mouse)) {
						editor_on_left_mouse_up(editor);
					}

					if(are_we_placing_tile(editor) && is_valid_index(index, c_max_tiles, c_max_tiles)) {
						int x_min = min(index.x, editor->shift_index.x);
						int y_min = min(index.y, editor->shift_index.y);
						int x_max = max(index.x, editor->shift_index.x) + 1;
						int y_max = max(index.y, editor->shift_index.y) + 1;
						for(int y = y_min; y < y_max; y++) {
							for(int x = x_min; x < x_max; x++) {
								s_v2i temp_index = v2i(x, y);
								draw_tile(
									g_r, tile_index_to_pos(temp_index), e_layer_tile_ghost, v2(c_tile_size - 1), editor->curr_tile, (u8)editor->rotation, {},
									{.mix_weight = 0.5f, .origin_offset = c_origin_topleft, .mix_color = make_color(1)}
								);
							}
						}
					}

					{
						b8 delete_tile = is_key_pressed(g_input, c_key_delete);
						b8 rotate = is_key_pressed(g_input, c_key_r);
						foreach_val(tile_i, tile, editor->selected) {
							if(!is_tile_active(&editor->map, tile)) {
								editor->selected.remove_and_swap(tile_i--);
							}
							if(rotate) {
								editor->map.rotation[tile.y][tile.x] += 1;
								editor->map.rotation[tile.y][tile.x] %= e_rotation_count;
							}
							draw_rect(
								g_r, tile_index_to_tile_center(tile), e_layer_tile_ghost,
								v2(5 + (c_tile_size - 10) * sinf2((float)renderer->total_time * 8)), make_color(0.8f, 1.0f, 0.8f)
							);
							if(delete_tile) {
								editor->map.tiles_active[tile.y][tile.x] = false;
							}
						}
					}

					if(is_key_down(g_input, c_right_mouse)) {
						if(is_valid_index(index, c_max_tiles, c_max_tiles)) {
							editor->map.tiles_active[index.y][index.x] = false;
						}
					}

					if(is_key_down(g_input, c_key_left_ctrl) && is_key_pressed(g_input, c_key_s)) {
						editor->state = e_map_editor_state_saving;
					}
					if(is_key_down(g_input, c_key_left_ctrl) && is_key_pressed(g_input, c_key_l)) {
						editor->state = e_map_editor_state_loading;
					}
				} break;

				case e_map_editor_state_saving: {

					s_v2 pos = v2(40);
					constexpr float font_size = 64;
					for(int map_i = 0; map_i < e_map_count; map_i++) {
						s_v4 color = make_color(0.5f);
						if(editor->save_index == map_i) {
							color = make_color(1);
							if(is_key_pressed(g_input, c_key_enter)) {
								char* file_name = format_text("map%i", map_i);
								u8* data = (u8*)la_get(platform_data->frame_arena, sizeof(s_map) + sizeof(int));
								u8* cursor = data;
								*(int*)cursor = c_map_version;
								cursor += sizeof(int);
								memcpy(cursor, &editor->map, sizeof(s_map));
								cursor += sizeof(s_map);
								platform_data->write_file(file_name, data, cursor - data);
								editor->state = e_map_editor_state_default;
							}
						}
						draw_text(g_r, format_text("Map %i", map_i + 1), pos, 5, font_size, color, false, game->font);
						pos.y += font_size;
					}
				} break;

				case e_map_editor_state_loading: {

					s_v2 pos = v2(40);
					constexpr float font_size = 64;
					for(int map_i = 0; map_i < e_map_count; map_i++) {
						s_v4 color = make_color(0.5f);
						if(editor->save_index == map_i) {
							color = make_color(1);
							if(is_key_pressed(g_input, c_key_enter)) {
								char* file_path = format_text("map%i", map_i);
								load_map(file_path, platform_data, &editor->map);
								editor->state = e_map_editor_state_default;
							}
						}
						draw_text(g_r, format_text("Map %i", map_i + 1), pos, 5, font_size, color, false, game->font);
						pos.y += font_size;
					}
				} break;

			}

			draw_map(&editor->map);
		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		map editor end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	}

	// if(is_key_down(g_input, c_left_mouse)) {
	// if(is_key_pressed(g_input, c_right_mouse)) {
	// 	do_particles(100, g_mouse, {
	// 		.shrink = 0.5f,
	// 		.slowdown = 1.0f,
	// 		.duration = 2.5f,
	// 		.duration_rand = 1,
	// 		.speed = 200 * (i+1) * 3.0f,
	// 		.speed_rand = 1,
	// 		.angle_rand = 1,
	// 		.radius = 8 * (i+1) * 2.0f,
	// 		.radius_rand = 1,
	// 		.color = v3(0.25f),
	// 		.color_rand = v3(1, 1, 0),
	// 	});
	// }

	foreach_ptr(particle_i, p, game->particles) {
		s_v4 color;
		color.x = p->color.x;
		color.y = p->color.y;
		color.z = p->color.z;
		color.w = 1.0f;
		float percent_done = at_most(1.0f, p->timer / p->duration);
		float speed = p->speed * (1.0f - percent_done * p->slowdown);
		speed = at_least(0.0f, speed);
		p->pos += p->dir * speed * (float)platform_data->frame_time;
		color.w *= 1.0f - (percent_done * p->fade);
		color.w = at_least(0.0f, color.w);
		float radius = p->radius * (1.0f - percent_done * p->shrink);
		radius = at_least(0.0f, radius);
		draw_rect(
			g_r, p->pos, e_layer_particle, v2(radius * 2.0f), color,
			{.blend_mode = e_blend_mode_additive, .framebuffer = game->particle_framebuffer}, {.flags = e_render_flag_circle}
		);
		p->timer += (float)platform_data->frame_time;
		if(percent_done >= 1) {
			game->particles.remove_and_swap(particle_i--);
		}
	}
	draw_texture(g_r, c_half_res, e_layer_particle, c_base_res, make_color(1), game->particle_framebuffer->texture, {.blend_mode = e_blend_mode_additive});
}


#ifdef m_build_dll
}
#endif // m_build_dll

func void set_globals(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, s_input* input)
{
	g_mouse = platform_data->mouse;
	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = input;
}

func b8 is_valid_index(int x, int y, int width, int height)
{
	return x >= 0 && x < width && y >= 0 && y < height;
}

func b8 is_valid_index(s_v2i index, int width, int height)
{
	return is_valid_index(index.x, index.y, width, height);
}

func s_sarray<s_collision_data, 16> get_collisions(c2Circle circle, s_v2 vel, s_map* map)
{
	s_sarray<s_collision_data, 16> result;
	s_v2i index = v2i(floorfi(circle.p.x / c_tile_size), floorfi(circle.p.y / c_tile_size));

	for(int y = -1; y <= 1; y++) {
		for(int x = -1; x <= 1; x++) {
			s_v2i new_index = index + v2i(x, y);
			if(!is_valid_index(new_index, c_max_tiles, c_max_tiles)) { continue; }
			if(!map->tiles_active[new_index.y][new_index.x]) { continue; }
			u8 tile_type = map->tiles[new_index.y][new_index.x];
			if(
				tile_type != e_tile_wall && tile_type != e_tile_one_way && tile_type != e_tile_bouncy_wall
			) { continue; }

			if(tile_type == e_tile_one_way) {
				if(floats_equal(vel.x, 0.0f) && floats_equal(vel.y, 0.0f)) { continue; }
				u8 rot = map->rotation[new_index.y][new_index.x];
				s_v2 tile_dir = v2_from_angle(c_rotation_to_rad[rot]);
				if(!floats_equal(tile_dir.x, 0.0f)) {
					if(sign(vel.x) == sign(tile_dir.x)) { continue; }
				}
				if(!floats_equal(tile_dir.y, 0.0f)) {
					if(sign(vel.y) == sign(tile_dir.y)) { continue; }
				}
			}

			s_v2 tile_pos = v2(new_index.x * c_tile_size, new_index.y * c_tile_size);
			c2AABB aabb = zero;
			aabb.min.x = tile_pos.x;
			aabb.min.y = tile_pos.y;
			aabb.max.x = tile_pos.x + c_tile_size;
			aabb.max.y = tile_pos.y + c_tile_size;
			c2Manifold m = zero;
			c2CircletoAABBManifold(circle, aabb, &m);
			if(m.count > 0) {
				s_collision_data col = zero;
				col.manifold = m;
				col.tile_type = tile_type;
				result.add(col);
			}
		}
	}

	return result;
}

func s_sarray<s_v2i, 16> get_interactive_tile_collisions(c2Circle circle, s_map* map)
{
	s_sarray<s_v2i, 16> result;
	s_v2i index = v2i(floorfi(circle.p.x / c_tile_size), floorfi(circle.p.y / c_tile_size));

	for(int y = -1; y <= 1; y++) {
		for(int x = -1; x <= 1; x++) {
			s_v2i new_index = index + v2i(x, y);
			if(!is_valid_index(new_index, c_max_tiles, c_max_tiles)) { continue; }
			if(!map->tiles_active[new_index.y][new_index.x]) { continue; }

			u8 tile_type = map->tiles[new_index.y][new_index.x];
			if(
				tile_type != e_tile_acceleration && tile_type != e_tile_directional && tile_type != e_tile_sand &&
				tile_type != e_tile_water && tile_type != e_tile_ice
			) { continue; }

			s_v2 tile_pos = v2(new_index.x * c_tile_size, new_index.y * c_tile_size);
			c2AABB aabb = zero;
			aabb.min.x = tile_pos.x;
			aabb.min.y = tile_pos.y;
			aabb.max.x = tile_pos.x + c_tile_size;
			aabb.max.y = tile_pos.y + c_tile_size;
			c2Manifold m = zero;
			b8 collides = c2CircletoAABB(circle, aabb) != 0;
			if(collides) {
				result.add(new_index);
			}
		}
	}

	return result;
}

func s_v2 v2(c2v v)
{
	return v2(v.x, v.y);
}

func c2v make_c2v(s_v2 v)
{
	return {.x = v.x, .y = v.y};
}

func void make_process_close_when_app_closes(HANDLE process)
{
	HANDLE job = CreateJobObjectA(null, null);
	assert(job);

	JOBOBJECT_EXTENDED_LIMIT_INFORMATION job_info = zero;
	job_info.BasicLimitInformation.LimitFlags = JOB_OBJECT_LIMIT_KILL_ON_JOB_CLOSE;
	BOOL set_info_result = SetInformationJobObject(job, JobObjectExtendedLimitInformation, &job_info, sizeof(job_info));
	UNREFERENCED_PARAMETER(set_info_result);
	assert(set_info_result);

	BOOL assign_result = AssignProcessToJobObject(job, process);
	UNREFERENCED_PARAMETER(assign_result);
	assert(assign_result);
}

func s_sarray<s_str, 128> get_chat_messages(s_platform_data* platform_data)
{
	s_sarray<s_str, 128> result;
	char* data = platform_data->read_file("chat_messages.txt", platform_data->frame_arena, null);
	if(!data) { return result; }
	data += game->last_chat_file_offset;
	int start = 0;
	int index = 0;
	while(true) {
		b8 do_thing = false;
		b8 end = false;
		char c = data[index];
		if(!c) { do_thing = true; end = true; }
		else if(c == '\r') { do_thing = true; }

		if(do_thing) {
			s_str s = zero;
			s.data = &data[start];
			s.len = index - start;
			if(s.len > 0) {
				result.add(s);
				start = index + 2;
				index += 1; // @Note(tkap, 21/10/2023): Because there is a \n coming
			}
		}
		if(end) { break; }
		index += 1;
	}
	game->last_chat_file_offset += index;
	return result;
}

func b8 is_number(char c)
{
	return c >= '0' && c <= '9';
}

func int parse_int(char *in, char **out)
{
	int result = 0;
	while(*in && *in <= ' ') { in++; }
	int sign = 1;
	while(*in == '-') {
		++in;
		sign *= -1;
	}
	b8 found_number = false;
	while(is_number(*in)) {
		result = result * 10 + (*in++ - '0');
		found_number = true;
	}
	if(found_number) { *out = in; }
	return result * sign;
}

func int get_ball_by_name(s_str name)
{
	foreach_ptr(ball_i, ball, game->balls) {
		if(strncmp(ball->name, name.data, name.len) == 0) {
			return ball_i;
		}
	}
	return -1;
}

func s_v2 tile_index_to_pos(s_v2i index)
{
	return v2(
		index.x * c_tile_size,
		index.y * c_tile_size
	);
}

func s_v2 tile_index_to_tile_center(s_v2i index)
{
	return v2(
		index.x * c_tile_size + c_tile_size * 0.5f,
		index.y * c_tile_size + c_tile_size * 0.5f
	);
}

func void draw_map(s_map* map)
{
	int grid = game->state == e_state_map_editor ? 1 : 0;
	for(int y = 0; y < c_max_tiles; y++) {
		for(int x = 0; x < c_max_tiles; x++) {
			if(!map->tiles_active[y][x]) { continue; }
			draw_tile(
				g_r, v2(x * c_tile_size, y * c_tile_size), e_layer_tile, v2(c_tile_size - grid), map->tiles[y][x], map->rotation[y][x], {}, {.origin_offset = c_origin_topleft}
			);
		}
	}
	{
		s_v2 pos = tile_index_to_pos(map->hole) + v2(c_tile_size * 0.5f);
		draw_rect(g_r, pos, e_layer_hole, v2(c_ball_radius * 4.0f), rgb(0xA98841), {.blend_mode = e_blend_mode_additive}, {.flags = e_render_flag_circle});
	}

	if(game->state == e_state_map_editor) {
		s_v2 pos = tile_index_to_pos(map->spawn) + v2(c_tile_size * 0.5f);
		draw_rect(g_r, pos, e_layer_hole, v2(c_ball_radius * 4.0f), rgb(0xD6DFAC), {.blend_mode = e_blend_mode_additive}, {.flags = e_render_flag_circle});
	}
}

func void draw_tile(s_game_renderer* renderer, s_v2 pos, e_layer layer, s_v2 size, int type, u8 rotation, s_render_data render_data, s_transform t) {
	s_v4 color = zero;
	t.rotation += c_rotation_to_rad[rotation];

	// @Hack(tkap, 23/10/2023):
	if(type == e_tile_directional) {
		draw_texture(renderer, pos, layer, size, make_color(1), game->directional_tile_texture, render_data, t);
		return;
	}
	if(type == e_tile_sand) {
		draw_texture(renderer, pos, layer, size, make_color(1), game->sand, render_data, t);
		return;
	}
	if(type == e_tile_water) {
		draw_texture(renderer, pos, layer, size, make_color(1), game->water, render_data, t);
		return;
	}
	if(type == e_tile_one_way) {
		draw_texture(renderer, pos, layer, size, make_color(1), game->one_way, render_data, t);
		return;
	}
	if(type == e_tile_ice) {
		draw_texture(renderer, pos, layer, size, make_color(1), game->ice, render_data, t);
		return;
	}
	if(type == e_tile_bouncy_wall) {
		draw_texture(renderer, pos, layer, size, make_color(1), game->bouncy_wall, render_data, t);
		return;
	}

	switch(type) {
		case e_tile_wall:	{
			color = rgb(0xC9522E);
		} break;
		case e_tile_default:	{
			color = rgb(0xC2BD95);
		} break;
		case e_tile_acceleration:	{
			color = rgb(0xA2BDA5);
		} break;
		invalid_default_case;
	}
	draw_rect(renderer, pos, layer, size, color, render_data, t);
}

func void load_map(char* file_path, s_platform_data* platform_data, s_map* out_map)
{
	u8* data = (u8*)platform_data->read_file(file_path, platform_data->frame_arena, null);
	if(!data) { return; }
	u8* cursor = data;
	int version = *(int*)cursor;
	cursor += sizeof(int);
	if(version == 1) {
		memcpy(out_map, cursor, sizeof(s_map) - sizeof(s_v2i) - (sizeof(u8) * c_max_tiles * c_max_tiles));
	}
	if(version == 2) {
		memcpy(out_map, cursor, sizeof(s_map) - (sizeof(u8) * c_max_tiles * c_max_tiles));
	}
	else {
		memcpy(out_map, cursor, sizeof(s_map));
	}
}

func b8 has_anyone_beaten_level()
{
	for(int i = 0; i < game->balls.count; i++) {
		if(game->transient.has_beat_level[i]) {
			return true;
		}
	}
	return false;
}

func b8 has_everyone_beaten_level()
{
	for(int i = 0; i < game->balls.count; i++) {
		if(!game->transient.has_beat_level[i]) {
			return false;
		}
	}
	return true;
}

func void move_ball_to_spawn(s_ball* ball, s_map* map)
{
	ball->c.p.x = tile_index_to_tile_center(map->spawn).x + game->transient.spawn_offset.x;
	ball->c.p.y = tile_index_to_tile_center(map->spawn).y + game->transient.spawn_offset.y;
	ball->pos_before_last_push = v2(ball->c.p);
	ball->has_push_queued = false;
}

func b8 are_we_on_last_map()
{
	if(game->modifiers[e_game_modifier_random_map_order]) {
		return game->random_maps.count == 0;
	}
	return game->curr_map >= e_map_count - 1;
}

func b8 are_we_placing_spawn_or_hole(s_map_editor* editor)
{
	return editor->curr_tile == 69 ||editor->curr_tile == 70;
}

func b8 are_we_placing_tile(s_map_editor* editor)
{
	return editor->curr_tile >= 0 && !are_we_placing_spawn_or_hole(editor);
}

func b8 is_something_selected(s_map_editor* editor)
{
	return editor->selected.count > 0;
}

func void editor_on_left_mouse_pressed(s_map_editor* editor)
{
	s_v2i index = v2i(floorfi(g_mouse.x / c_tile_size), floorfi(g_mouse.y / c_tile_size));
	if(are_we_placing_tile(editor) || are_we_placing_spawn_or_hole(editor)) {
	}
	else {
		editor->select_start = index;
	}
}

func void editor_on_left_mouse_down(s_map_editor* editor)
{
	s_v2i index = v2i(floorfi(g_mouse.x / c_tile_size), floorfi(g_mouse.y / c_tile_size));
	if(are_we_placing_tile(editor) || are_we_placing_spawn_or_hole(editor)) {
		if(is_valid_index(index, c_max_tiles, c_max_tiles)) {
			if(editor->curr_tile == 69) {
				editor->map.hole = index;
			}
			else if(editor->curr_tile == 70) {
				editor->map.spawn = index;
			}
			else {
				int x_min = min(index.x, editor->shift_index.x);
				int y_min = min(index.y, editor->shift_index.y);
				int x_max = max(index.x, editor->shift_index.x) + 1;
				int y_max = max(index.y, editor->shift_index.y) + 1;
				for(int y = y_min; y < y_max; y++) {
					for(int x = x_min; x < x_max; x++) {
						if(!is_valid_index(v2i(x, y), c_max_tiles, c_max_tiles)) { continue; }
						editor->map.tiles_active[y][x] = true;
						editor->map.tiles[y][x] = (u8)editor->curr_tile;
						editor->map.rotation[y][x] = (u8)editor->rotation;
					}
				}
			}
		}
	}
	else {
	}
}

func void editor_on_left_mouse_up(s_map_editor* editor)
{
	s_v2i index = v2i(floorfi(g_mouse.x / c_tile_size), floorfi(g_mouse.y / c_tile_size));
	if(are_we_placing_tile(editor) || are_we_placing_spawn_or_hole(editor)) {
	}
	else {
		int x_min = min(index.x, editor->select_start.x);
		int y_min = min(index.y, editor->select_start.y);
		int x_max = max(index.x, editor->select_start.x) + 1;
		int y_max = max(index.y, editor->select_start.y) + 1;

		editor->selected.count = 0;
		for(int y = y_min; y < y_max; y++) {
			for(int x = x_min; x < x_max; x++) {
				s_v2i temp_index = v2i(x, y);
				if(is_valid_index(temp_index, c_max_tiles, c_max_tiles) && editor->map.tiles_active[temp_index.y][temp_index.x]) {
					editor->selected.add(temp_index);
				}
			}
		}
	}
}

func b8 is_tile_active(s_map* map, int x, int y)
{
	assert(is_valid_index(x, y, c_max_tiles, c_max_tiles));
	return map->tiles_active[y][x];
}

func b8 is_tile_active(s_map* map, s_v2i index)
{
	return is_tile_active(map, index.x, index.y);
}

func int get_worst_pushes_that_beat_level()
{
	int result = 0;
	foreach_val(ball_i, ball, game->balls) {
		if(game->transient.has_beat_level[ball_i]) {
			result = max(result, game->transient.push_count[ball_i]);
		}
	}
	return result;
}

func void do_particles(int count, s_v2 pos, s_particle_data data)
{
	s_rng* rng = &game->rng;
	for(int particle_i = 0; particle_i < count; particle_i++) {
		s_particle p = zero;
		p.pos = pos;
		p.fade = data.fade;
		p.shrink = data.shrink;
		p.duration = data.duration * (1.0f - rng->randf32() * data.duration_rand);
		p.dir = v2_from_angle(data.angle + tau * rng->randf32() * data.angle_rand);
		p.speed = data.speed * (1.0f - rng->randf32() * data.speed_rand);
		p.radius = data.radius * (1.0f - rng->randf32() * data.radius_rand);
		p.slowdown = data.slowdown;
		p.color = data.color;
		p.color.x *= (1.0f - rng->randf32() * data.color_rand.x);
		p.color.y *= (1.0f - rng->randf32() * data.color_rand.y);
		p.color.z *= (1.0f - rng->randf32() * data.color_rand.z);
		game->particles.add_checked(p);
	}
}

func void beat_level_particles(s_v2 pos)
{
	for(int i = 0; i < 3; i++) {
		do_particles(100, pos, {
			.shrink = 0.5f,
			.slowdown = 1.0f,
			.duration = 2.5f,
			.duration_rand = 1,
			.speed = 200 * (i+1) * 3.0f,
			.speed_rand = 1,
			.angle_rand = 1,
			.radius = 8 * (i+1) * 2.0f,
			.radius_rand = 1,
			.color = v3(0.25f),
			.color_rand = v3(1, 1, 0),
		});
	}
}

func void do_collision_particles(s_v2 pos, float mag, s_v4 color)
{
	#define mod(a, b) range_lerp(mag, 0.0f, 1000.0f, a, b)
	do_particles((int)mod(10, 100), pos, {
		.slowdown = 2.0f,
		.duration = 0.25f,
		.duration_rand = 0.8f,
		.speed = mod(100, 400),
		.speed_rand = 0.8f,
		.angle_rand = 1,
		.radius = mod(4, 10),
		.radius_rand = 1,
		.color = v3(color) * mod(0.1f, 0.2f),
		.color_rand = v3(0.25f),
	});
	#undef mod
}

func void maybe_add_new_ball(s_str user)
{
	int already_present_index = get_ball_by_name(user);
	if(already_present_index < 0) {

		int new_ball_index = game->balls.count;
		game->transient.has_beat_level[new_ball_index] = false;
		game->transient.push_count[new_ball_index] = 0;

		s_ball ball = zero;
		s_map* map = &game->maps[game->curr_map];
		move_ball_to_spawn(&ball, map);
		ball.c.r = c_ball_radius;
		memcpy(ball.name, user.data, user.len);
		if(game->curr_map > 0) {
			int worst_total = 0;
			int worst_level = 0;
			foreach_val(worst_i, worst, game->balls) {
				if(worst.push_count > worst_total) {
					worst_total = worst.push_count;
					worst_level = game->transient.push_count[worst_i];
				}
			}
			ball.push_count = worst_total - worst_level;
		}
		ball.color = make_color(game->rng.randf32(), game->rng.randf32(), game->rng.randf32());
		game->balls.add(ball);
	}
}

func bool compare_maps_beaten(s_name_and_push_count a, s_name_and_push_count b)
{
	return a.maps_beaten < b.maps_beaten;
}