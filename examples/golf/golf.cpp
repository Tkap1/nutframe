#define m_game

#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>

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

enum e_state
{
	e_state_join,
	e_state_play,
	e_state_map_editor,
};

enum e_layer
{
	e_layer_tile,
	e_layer_hole,
	e_layer_ball,
	e_layer_ui,
};

enum e_map_editor_state
{
	e_map_editor_state_default,
	e_map_editor_state_saving,
	e_map_editor_state_loading,
};

enum e_map
{
	e_map_001,
	e_map_002,
	e_map_003,
	e_map_count,
};

struct s_ball
{
	char name[64];
	c2Circle c;
	s_v2 vel;
	s_v4 color;
};

#pragma pack(push, 1)
struct s_map
{
	float zoom;
	b8 tiles_active[c_max_tiles][c_max_tiles];
	u8 tiles[c_max_tiles][c_max_tiles];
	s_v2i hole;
	s_v2i spawn;
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
	u8 curr_tile;
	s_map map;
	s_v2i shift_index;
};

struct s_game
{
	b8 initialized;
	int last_chat_file_offset;
	e_state state;
	s_rng rng;
	s_sarray<s_ball, c_max_balls> balls;
	s_font* font;
	int curr_map;
	s_map_editor editor;
	s_map maps[e_map_count];
	b8 has_beat_level[c_max_balls];
	s_texture angle_indicator;
};

global s_input* g_input;
global s_game* game;
global s_game_renderer* g_r;
global s_v2 g_mouse;

func s_sarray<c2Manifold, 16> get_collisions(c2Circle circle, s_map* map);
func s_sarray<u8, 16> get_interactive_tile_collisions(c2Circle circle, s_map* map);
func s_v2 v2(c2v v);
func void make_process_close_when_app_closes(HANDLE process);
func s_sarray<s_str, 128> get_chat_messages(s_platform_data* platform_data);
func int parse_int(char* in, char** out);
func s_ball* get_ball_by_name(s_str name);
func b8 is_valid_index(s_v2i index, int width, int height);
func void draw_map(s_map* map);
func void load_map(char* file_path, s_platform_data* platform_data, s_map* out_map);
func s_v2 tile_index_to_pos(s_v2i index);

#ifdef m_build_dll
extern "C" {
m_dll_export
#endif // m_build_dll
m_update_game(update_game)
{
	static_assert(sizeof(s_game) <= c_game_memory);

	g_mouse = platform_data->mouse;
	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = platform_data->input;
	if(!game->initialized) {
		game->initialized = true;
		game->rng.seed = platform_data->get_random_seed();
		game->font = &renderer->fonts[0];
		g_r->set_vsync(true);
		platform_data->variables_path = "examples/golf/variables.h";
		game->angle_indicator = g_r->load_texture(g_r, "examples/golf/angle_indicator.png");

		game->curr_map = 2;
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

	live_variable(&platform_data->vars, c_tile_size, 4, 128, true);
	live_variable(&platform_data->vars, c_ball_impulse, 100.0f, 2000.0f, true);
	live_variable(&platform_data->vars, c_ball_radius, 16.0f, 128.0f, true);
	live_variable(&platform_data->vars, c_angle_indicator_x, 0.0f, 1.0f, true);
	live_variable(&platform_data->vars, c_angle_indicator_y, 0.0f, 1.0f, true);

	float delta = (float)platform_data->frame_time;

	s_sarray<s_str, 128> msgs = get_chat_messages(platform_data);
	foreach_val(msg_i, msg, msgs) {
		s_str user = zero;
		int index = str_find_from_left(msg.data, msg.len, ":", 1);
		user.data = msg.data;
		user.len = index;
		s_str content = zero;
		content.data = msg.data + index + 1;
		content.len = msg.len - index - 1;

		if(game->state == e_state_join) {
			if(content.len >= 4 && strncmp(content.data, "join", 4) == 0) {
				s_ball* already_present = get_ball_by_name(user);
				if(!already_present) {
					s_ball ball = zero;
					s_map* map = &game->maps[game->curr_map];
					ball.c.p.x = tile_index_to_pos(map->spawn).x + c_tile_size / 2.0f;
					ball.c.p.y = tile_index_to_pos(map->spawn).y + c_tile_size / 2.0f;
					ball.c.r = c_ball_radius;
					memcpy(ball.name, user.data, user.len);
					game->balls.add(ball);
				}
			}
		}

		// printf("%.*s: %.*s\n", user.len, user.data, content.len, content.data);
		if(content.len > 4 && strncmp(content.data, "push", 4) == 0) {
			char* out;
			char* out2;
			int angle = parse_int(content.data + 5, &out);
			if(!out) { continue; }
			int strength = parse_int(out, &out2);
			if(!out2) { continue; }
			strength = clamp(strength, 1, 100);
			// printf("%i, %i\n", angle, strength);

			s_ball* ball = get_ball_by_name(user);
			if(ball) {
				ball->vel += v2_from_angle(deg_to_rad((float)angle)) * range_lerp((float)strength, 1, 100, 25, 2000);
			}
		}
	}

	switch(game->state) {
		case e_state_join: {
			if(is_key_pressed(g_input, c_key_f2)) {
				game->state = e_state_map_editor;
			}
			draw_map(&game->maps[game->curr_map]);
			draw_texture(g_r, c_base_res * v2(c_angle_indicator_x, c_angle_indicator_y), e_layer_ui, v2(128), make_color(1), game->angle_indicator);
		} break;

		case e_state_play: {
			draw_map(&game->maps[game->curr_map]);
			draw_texture(g_r, c_base_res * v2(c_angle_indicator_x, c_angle_indicator_y), e_layer_ui, v2(128), make_color(1), game->angle_indicator);
		} break;

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
					s_v2 pos = c_base_res * v2(0.7f, 0.1f);
					b8 click_handled = false;
					for(int i = 0; i < 3; i++) {
						s_ui_interaction interaction = platform_data->ui_button(g_r, format_text("%i", i), pos, v2(64), game->font, 32.0f, g_input, g_mouse);
						if(interaction.state == e_ui_pressed) { click_handled = true; }
						else if(interaction.state == e_ui_active) {
							editor->curr_tile = (u8)i;
						}
						pos.x += 68.0f;
					}
					{
						pos = c_base_res * v2(0.7f, 0.1f);
						pos.y += 68.0f;
						s_ui_interaction interaction = platform_data->ui_button(g_r, "Hole", pos, v2(64), game->font, 32.0f, g_input, g_mouse);
						if(interaction.state == e_ui_pressed) { click_handled = true; }
						else if(interaction.state == e_ui_active) {
							editor->curr_tile = 69;
						}
						pos.x += 68.0f;
					}
					{
						s_ui_interaction interaction = platform_data->ui_button(g_r, "Spawn", pos, v2(64), game->font, 32.0f, g_input, g_mouse);
						if(interaction.state == e_ui_pressed) { click_handled = true; }
						else if(interaction.state == e_ui_active) {
							editor->curr_tile = 70;
						}
					}

					s_v2i index = v2i(floorfi(g_mouse.x / c_tile_size), floorfi(g_mouse.y / c_tile_size));
					if(is_key_pressed(g_input, c_key_left_shift)) {
						editor->shift_index = index;
					}
					else if(is_key_up(g_input, c_key_left_shift)) {
						editor->shift_index = index;
					}

					if(!click_handled && is_key_down(g_input, c_left_mouse)) {
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
										editor->map.tiles[y][x] = editor->curr_tile;
									}
								}
							}
						}
					}

					if(is_key_down(g_input, c_right_mouse)) {
						if(is_valid_index(index, c_max_tiles, c_max_tiles)) {
							editor->map.tiles_active[index.y][index.x] = false;
							editor->map.tiles[index.y][index.x] = editor->curr_tile;
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
								*(int*)cursor = 2;
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
	}

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		ball update start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	foreach_ptr(ball_i, ball, game->balls) {
		s_map* map = &game->maps[game->curr_map];
		ball->c.r = c_ball_radius;

		{
			auto collisions = get_interactive_tile_collisions(ball->c, map);
			float mul = 1.0f;
			foreach_val(tile_i, tile, collisions) {
				mul += 0.03f;
			}
			ball->vel *= mul;
		}
		ball->vel.x = clamp(ball->vel.x, -3000.0f, 3000.0f);
		ball->vel.y = clamp(ball->vel.y, -3000.0f, 3000.0f);

		for(int i = 0; i < 10; i++) {
			s_v2 movement = ball->vel * 0.1f;

			ball->c.p.x += movement.x * delta;
			ball->c.p.y += movement.y * delta;
			auto collisions = get_collisions(ball->c, map);
			if(collisions.count > 0) {
				c2Manifold c = collisions[0];

				ball->c.p.x -= (c.depths[0]) * c.n.x;
				ball->c.p.y -= (c.depths[0]) * c.n.y;
				ball->vel = v2_reflect(ball->vel, v2(c.n));
				break;
			}
		}

		if(is_key_pressed(g_input, c_left_mouse)) {
			game->balls[0].c.p.x = g_mouse.x;
			game->balls[0].c.p.y = g_mouse.y;
		}

		s_v2 hole_pos = tile_index_to_pos(map->hole) + v2(c_tile_size * 0.5f);
		b8 in_hole = c2CircletoCircle(ball->c, {.p = {hole_pos.x, hole_pos.y}, .r = c_ball_radius * 2.0f}) != 0;

		ball->vel *= 0.99f;
		s_v4 color = rgb(0x0D232D);
		if(game->has_beat_level[ball_i]) {
			color = brighter(rgb(0xA4BB96), 1.2f);
		}
		draw_rect(g_r, v2(ball->c.p), e_layer_ball, v2(ball->c.r * 2.0f), color, {}, {.flags = e_render_flag_circle});
		if(!in_hole) {
			draw_text(g_r, ball->name, v2(ball->c.p), 50, 24, make_color(1), true, game->font);
		}

		// @Note(tkap, 22/10/2023): Check for win
		float length = v2_length(ball->vel);
		if(in_hole && length < 0.3f && !game->has_beat_level[ball_i]) {
			game->has_beat_level[ball_i] = true;
			printf("%s has beaten the level!\n", ball->name);
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		ball update end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


	for(int i = 0; i < c_max_keys; i++) {
		g_input->keys[i].count = 0;
	}

	platform_data->reset_ui();

}

#ifdef m_build_dll
}
#endif // m_build_dll

func b8 is_valid_index(s_v2i index, int width, int height)
{
	return index.x >= 0 && index.x < width && index.y >= 0 && index.y < height;
}

func s_sarray<c2Manifold, 16> get_collisions(c2Circle circle, s_map* map)
{
	s_sarray<c2Manifold, 16> result;
	s_v2i index = v2i(floorfi(circle.p.x / c_tile_size), floorfi(circle.p.y / c_tile_size));

	for(int y = -1; y <= 1; y++) {
		for(int x = -1; x <= 1; x++) {
			s_v2i new_index = index + v2i(x, y);
			if(!is_valid_index(new_index, c_max_tiles, c_max_tiles)) { continue; }
			if(!map->tiles_active[new_index.y][new_index.x]) { continue; }
			if(map->tiles[new_index.y][new_index.x] != 0) { continue; }

			s_v2 tile_pos = v2(new_index.x * c_tile_size, new_index.y * c_tile_size);
			c2AABB aabb = zero;
			aabb.min.x = tile_pos.x;
			aabb.min.y = tile_pos.y;
			aabb.max.x = tile_pos.x + c_tile_size;
			aabb.max.y = tile_pos.y + c_tile_size;
			c2Manifold m = zero;
			c2CircletoAABBManifold(circle, aabb, &m);
			if(m.count > 0) {
				result.add(m);
			}
		}
	}

	return result;
}

func s_sarray<u8, 16> get_interactive_tile_collisions(c2Circle circle, s_map* map)
{
	s_sarray<u8, 16> result;
	s_v2i index = v2i(floorfi(circle.p.x / c_tile_size), floorfi(circle.p.y / c_tile_size));

	for(int y = -1; y <= 1; y++) {
		for(int x = -1; x <= 1; x++) {
			s_v2i new_index = index + v2i(x, y);
			if(!is_valid_index(new_index, c_max_tiles, c_max_tiles)) { continue; }
			if(!map->tiles_active[new_index.y][new_index.x]) { continue; }
			if(map->tiles[new_index.y][new_index.x] != 2) { continue; }

			s_v2 tile_pos = v2(new_index.x * c_tile_size, new_index.y * c_tile_size);
			c2AABB aabb = zero;
			aabb.min.x = tile_pos.x;
			aabb.min.y = tile_pos.y;
			aabb.max.x = tile_pos.x + c_tile_size;
			aabb.max.y = tile_pos.y + c_tile_size;
			c2Manifold m = zero;
			b8 collides = c2CircletoAABB(circle, aabb) != 0;
			if(collides) {
				result.add(map->tiles[new_index.y][new_index.x]);
			}
		}
	}

	return result;
}

func s_v2 v2(c2v v)
{
	return v2(v.x, v.y);
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

func s_ball* get_ball_by_name(s_str name)
{
	foreach_ptr(ball_i, ball, game->balls) {
		if(strncmp(ball->name, name.data, name.len) == 0) {
			return ball;
		}
	}
	return null;
}

func s_v2 tile_index_to_pos(s_v2i index)
{
	return v2(
		index.x * c_tile_size,
		index.y * c_tile_size
	);
}

func void draw_map(s_map* map)
{
	int grid = game->state == e_state_map_editor ? 1 : 0;
	for(int y = 0; y < c_max_tiles; y++) {
		for(int x = 0; x < c_max_tiles; x++) {
			if(!map->tiles_active[y][x]) { continue; }
			if(map->tiles[y][x] == 0) {
				draw_rect(g_r, v2(x * c_tile_size, y * c_tile_size), e_layer_tile, v2(c_tile_size - grid), rgb(0xC9522E), {}, {.origin_offset = c_origin_topleft});
			}
			else if(map->tiles[y][x] == 1) {
				draw_rect(g_r, v2(x * c_tile_size, y * c_tile_size), e_layer_tile, v2(c_tile_size - grid), rgb(0xC2BD95), {}, {.origin_offset = c_origin_topleft});
			}
			else if(map->tiles[y][x] == 2) {
				draw_rect(g_r, v2(x * c_tile_size, y * c_tile_size), e_layer_tile, v2(c_tile_size - grid), rgb(0xA2BDA5), {}, {.origin_offset = c_origin_topleft});
			}
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

func void load_map(char* file_path, s_platform_data* platform_data, s_map* out_map)
{
	u8* data = (u8*)platform_data->read_file(file_path, platform_data->frame_arena, null);
	if(!data) { return; }
	u8* cursor = data;
	int version = *(int*)cursor;
	cursor += sizeof(int);
	if(version == 1) {
		memcpy(out_map, cursor, sizeof(s_map) - sizeof(s_v2i));
	}
	else {
		memcpy(out_map, cursor, sizeof(s_map));
	}
}