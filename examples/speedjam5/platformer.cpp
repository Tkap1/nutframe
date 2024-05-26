#define m_game

#include "../../src/platform_shared.h"
#include "variables.h"

#include "platformer.h"

static constexpr s_v2 c_base_res = {800, 800};
// static constexpr s_v2 c_half_res = {c_base_res.x * 0.5f, c_base_res.y * 0.5f};

enum e_state
{
	e_state_play,
	e_state_editor,
};

template <typename t, int n>
struct s_array_with_mask
{
	s_carray<b8, n> active;
	s_carray<t, n> elements;

	t& operator[](int index)
	{
		assert(index >= 0);
		assert(index < n);
		return elements[index];
	}
};

struct s_camera2d
{
	s_v2 pos;

	s_recti get_tile_bounds();
	s_v2 world_to_screen(s_v2 v)
	{
		s_v2 result = v;
		result.x -= pos.x;
		result.y -= pos.y;
		return result;
	}

	s_v2 screen_to_world(s_v2 v)
	{
		s_v2 result = v;
		result.x += pos.x;
		result.y += pos.y;
		return result;
	}
};

struct s_projectile
{
	int timer;
	s_v2 prev_pos;
	s_v2 pos;
	s_v2 dir;
};

struct s_player
{
	int jumps_left;
	int shoot_timer;
	s_v2 prev_pos;
	s_v2 pos;
	s_v2 vel;
};

struct s_editor
{
	s8 curr_tile;
};

struct s_game
{
	b8 initialized;
	b8 follow_player;
	b8 reset_game;
	e_state state;
	s_framebuffer* particle_framebuffer;
	s_framebuffer* text_framebuffer;
	s_camera3d cam;
	s_camera2d editor_cam;
	s_texture sheet;
	s_texture noise;
	s_sarray<s_projectile, c_max_projectiles> projectile_arr;
	s_rng rng;
	s_font* font;
	s_player player;
	s_map map;
	int curr_save_point;
	s_editor editor;
	s_ray ray;
	s_sarray<s_visual_effect, 128> visual_effect_arr;
};

static s_input* g_input;
static s_game* game;
static s_game_renderer* g_r;
static s_v2 g_mouse;

#ifdef m_build_dll
extern "C" {
#endif // m_build_dll

m_dll_export void init_game(s_platform_data* platform_data)
{
	platform_data->set_base_resolution((int)c_base_res.x, (int)c_base_res.y);
	platform_data->set_window_size((int)c_base_res.x, (int)c_base_res.y);
	platform_data->update_delay = 1.0 / c_updates_per_second;
}

m_dll_export void update(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{

	game = (s_game*)game_memory;
	g_input = &platform_data->logic_input;
	g_mouse = platform_data->mouse;

	switch(game->state) {
		case e_state_play: {

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		player update start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* player = &game->player;
				player->prev_pos = player->pos;

				printf("%f\n", game->player.vel.y);

				float input_vel = 0;
				if(game->follow_player) {
					if(is_key_down(g_input, c_key_a) && in_perspective) {
						input_vel -= c_player_speed;
					}
					if(is_key_down(g_input, c_key_d) && in_perspective) {
						input_vel += c_player_speed;
					}
				}
				input_vel = clamp(input_vel, -c_max_input_vel, c_max_input_vel);

				player->shoot_timer = at_most(c_shoot_delay, player->shoot_timer + 1);
				if(player->shoot_timer >= c_shoot_delay && is_key_down(g_input, c_left_mouse)) {
					player->shoot_timer -= c_shoot_delay;
					s_projectile p = {};
					p.pos = player->pos;
					s_v3 pos = ray_at_z(game->ray, c_player_z);
					p.dir = v2_normalized(pos.xy - player->pos);
					game->projectile_arr.add_checked(p);
				}

				if(is_key_pressed(g_input, c_key_right)) {
					player->vel.x += 0.25f;
				}

				// right, right
				if(player->vel.x > 0 && input_vel > 0) {
					if(player->vel.x < c_max_input_vel) {
						input_vel = min(input_vel, c_max_input_vel - player->vel.x);
					}
				}

				// right, left or
				// left, right or
				else if(!floats_equal(player->vel.x, 0) && !floats_equal(input_vel, 0) && sign(player->vel.x) != sign(input_vel)) {
					input_vel *= 0.02f;
					player->vel.x = player->vel.x + input_vel;
					input_vel = 0;
				}

				// left, left
				else if(player->vel.x < 0 && input_vel < 0) {
					if(player->vel.x > -c_max_input_vel) {
						input_vel = -min(fabsf(input_vel), c_max_input_vel - fabsf(player->vel.x));
					}
				}

				if(c_apply_gravity >= 0.5f) {
					player->vel.y = min(0.17f, player->vel.y + c_gravity);
				}

				if(is_key_pressed(g_input, c_key_space)) {
					if(player->jumps_left > 0) {
						player->vel.y = -c_jump_strength;
						player->jumps_left -= 1;
					}
				}


				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		player x collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				float x_vel = player->vel.x + input_vel;
				player->pos.x += x_vel;
				auto collision_arr = get_tile_collisions(player->pos, c_player_size, c_play_tile_size);
				foreach_val(collision_i, collision, collision_arr) {
					if(x_vel > 0) {
						player->pos.x = collision.tile_center.x - (c_play_tile_size * 0.5f + c_player_size.x * 0.5f) - c_small;
					}
					else {
						player->pos.x = collision.tile_center.x + (c_play_tile_size * 0.5f + c_player_size.x * 0.5f) + c_small;
					}
					player->vel.x = 0;

					// @Hack(tkap, 26/05/2024): shutting up warning
					if(collision_i >= 0) { break; }
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		player x collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		player y collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				player->pos.y += player->vel.y;
				b8 on_ground = false;
				collision_arr = get_tile_collisions(player->pos, c_player_size, c_play_tile_size);
				foreach_val(collision_i, collision, collision_arr) {
					if(player->vel.y > 0) {
						player->pos.y = collision.tile_center.y - (c_play_tile_size * 0.5f + c_player_size.y * 0.5f) - c_small;
						player->jumps_left = c_max_jumps;
						on_ground = true;
					}
					else {
						player->pos.y = collision.tile_center.y + (c_play_tile_size * 0.5f + c_player_size.y * 0.5f) + c_small;
					}
					player->vel.y = 0;

					// @Hack(tkap, 26/05/2024): shutting up warning
					if(collision_i >= 0) { break; }
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		player y collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				if(on_ground) {
					player->vel.x *= c_ground_friction;
					if(fabsf(player->vel.x) < 0.001f) {
						player->vel.x = 0;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		player update end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update projectiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_ptr(projectile_i, projectile, game->projectile_arr) {
				projectile->prev_pos = projectile->pos;
				projectile->pos += projectile->dir * c_projectile_speed;
				b8 remove = false;

				auto collision_arr = get_tile_collisions(projectile->pos, c_projectile_collision_size, c_play_tile_size);
				if(collision_arr.count > 0) {
					remove = true;
					s_visual_effect ve = {};
					ve.type = e_visual_effect_projectile_explosion;
					ve.pos = projectile->pos;
					game->visual_effect_arr.add(ve);
					s_v2 dir = v2_normalized(game->player.pos - projectile->pos);
					float distance = v2_distance(game->player.pos, projectile->pos);
					float strength = smoothstep(c_far_push_range, c_near_push_range, distance) * c_push_strength;
					game->player.vel += dir * strength;
				}

				projectile->timer += 1;
				if(projectile->timer >= c_projectile_duration) {
					remove = true;
				}

				if(remove) {
					game->projectile_arr.remove_and_shift(projectile_i--);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update projectiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		} break;
	}
}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt)
{
	static_assert(sizeof(s_game) <= c_game_memory);

	g_mouse = platform_data->mouse;

	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = &platform_data->render_input;
	if(!game->initialized) {
		game->initialized = true;
		game->rng.seed = platform_data->get_random_seed();
		g_r->set_vsync(true);
		game->sheet = g_r->load_texture(renderer, "examples/speedjam5/sheet.png");
		// game->particle_framebuffer = g_r->make_framebuffer(renderer, false);
		// game->text_framebuffer = g_r->make_framebuffer(renderer, false);
		// game->eat_apple_sound = platform_data->load_sound(platform_data, "examples/snake/eat_apple.wav", platform_data->frame_arena);
		game->font = &renderer->fonts[0];
		platform_data->variables_path = "examples/speedjam5/variables.h";
		game->cam.pos = v3(0, 0, -5);
		game->cam.target = v3(0, 0, 1);
		game->cam.pos.x = c_play_tile_size * (c_tiles_right / 2);
		game->cam.pos.y = c_play_tile_size * c_tiles_down;
		game->editor_cam.pos.x = c_editor_tile_size * (c_tiles_right / 2);
		game->editor_cam.pos.y = c_editor_tile_size * (c_tiles_down - 10);
		game->reset_game = true;

		load_map(&game->map, platform_data);

	}

	live_variable(&platform_data->vars, c_player_speed, 0.0f, 1.0f, true);
	live_variable(&platform_data->vars, c_ground_friction, 0.0f, 1.0f, true);
	live_variable(&platform_data->vars, c_apply_gravity, 0.0f, 1.0f, true);
	live_variable(&platform_data->vars, c_jump_strength, 0.0f, 0.5f, true);
	live_variable(&platform_data->vars, c_max_input_vel, 0.0f, 1.0f, true);
	live_variable(&platform_data->vars, c_shoot_delay, 0, 500, true);
	live_variable(&platform_data->vars, c_near_push_range, 0.0f, 2.0f, true);
	live_variable(&platform_data->vars, c_far_push_range, 0.0f, 20.0f, true);
	live_variable(&platform_data->vars, c_push_strength, 0.0f, 0.33f, true);
	live_variable(&platform_data->vars, c_projectile_speed, 0.0f, 5.0f, true);

	float delta = (float)platform_data->frame_time;
	float time = (float)g_r->total_time;
	unreferenced(time);

	static float timer = 0;


	if(is_key_pressed(g_input, c_key_f)) {
		in_perspective = !in_perspective;
	}
	if(in_perspective) {
		timer = at_most(1.0f, timer + delta * 2);
	}
	else {
		timer = at_least(0.0f, timer - delta * 2);
	}

	if(is_key_pressed(g_input, c_key_f1)) {
		if(game->state == e_state_editor) {
			game->state = e_state_play;
		}
		else {
			game->state = e_state_editor;
		}
	}

	if(is_key_pressed(g_input, c_key_f2)) {
		game->follow_player = !game->follow_player;
	}

	switch(game->state) {
		case e_state_play: {

			if(is_key_down(g_input, c_key_r)) {
				game->reset_game = true;
			}

			if(game->reset_game) {
				game->reset_game = false;

				s_player* player = &game->player;
				*player = {};
				if(game->map.save_point_arr.count > 0) {
					player->pos = v2(game->map.save_point_arr[0].pos) * c_play_tile_size;
				}
			}

			if(!game->follow_player) {
				constexpr float c_cam_speed = 50;
				if(is_key_down(g_input, c_key_w) && in_perspective) {
					game->cam.pos.z += 1 * delta * c_cam_speed;
				}
				if(is_key_down(g_input, c_key_a)) {
					game->cam.pos.x -= 1 * delta * c_cam_speed;
				}
				if(is_key_down(g_input, c_key_s) && in_perspective) {
					game->cam.pos.z -= 1 * delta * c_cam_speed;
				}
				if(is_key_down(g_input, c_key_d)) {
					game->cam.pos.x += 1 * delta * c_cam_speed;
				}
				if(is_key_down(g_input, c_key_up)) {
					game->cam.pos.y -= 1 * delta * c_cam_speed;
				}
				if(is_key_down(g_input, c_key_down)) {
					game->cam.pos.y += 1 * delta * c_cam_speed;
				}
			}

			if(game->follow_player) {
				s_v2 pos = lerp(game->player.prev_pos, game->player.pos, interp_dt);
				game->cam.pos.xy = pos;
				game->cam.pos.y -= 2;
				game->cam.pos.z = -10;
			}
			s_m4 view = look_at(game->cam.pos, game->cam.pos + game->cam.target, v3(0, -1, 0));
			s_m4 perspective = m4_perspective(90, c_base_res.x / c_base_res.y, 0.01f, 10000.0f);
			s_m4 orthographic = m4_orthographic(-10, 10, -10, 10, -10, 10);
			s_m4 projection = lerp_m4(orthographic, perspective, timer);
			g_r->view_projection = m4_multiply(projection, view);

			game->ray = get_ray(g_mouse, c_base_res, game->cam, view, projection);


			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw play tiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			s_recti bounds = get_3d_tile_bounds(game->cam);
			for(int y = bounds.y0; y <= bounds.y1; y++) {
			// for(int y = 0; y < c_tiles_down; y++) {
				for(int x = bounds.x0; x <= bounds.x1; x++) {
				// for(int x = 0; x < c_tiles_right; x++) {
					s8 tile = game->map.tile_arr[y][x];
					if(tile > 0) {
						draw_cube(g_r, v3((float)x * c_play_tile_size, (float)y * c_play_tile_size, 0.0f), v3(1.0f, 1.0f, 1.0f), make_color(1));
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw play tiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player player = game->player;
				s_v2 pos = lerp(player.prev_pos, player.pos, interp_dt);
				// draw_cube(g_r, v3(pos.x, pos.y, c_player_z), v3(1.0f, 1.0f, 1.0f), make_color(0.1f, 1.0f, 0.1f));
				draw_atlas_3d(g_r, v3(pos, c_player_z), c_player_size, make_color(1), game->sheet, v2i(0, 0), v2i(64, 64));
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw projectiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(projectile_i, projectile, game->projectile_arr) {
				s_v2 pos = lerp(projectile.prev_pos, projectile.pos, interp_dt);
				// draw_cube(g_r, v3(pos.x, pos.y, c_player_z), c_projectile_size, make_color(1.0f, 1.0f, 0.1f));
				draw_atlas_3d(g_r, v3(pos, c_player_z), c_projectile_visual_size, make_color(1), game->sheet, v2i(256, 0), c_sprite_size, {}, {.rotation = v2_angle(projectile.dir)});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw projectiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		visual effects start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_ptr(ve_i, ve, game->visual_effect_arr) {
				ve->timer += delta;
				float duration = 0;
				switch(ve->type) {
					case e_visual_effect_projectile_explosion: {
						duration = 0.25f;
						int index = clamp(roundfi(ve->timer / 0.25f * 2), 0, 2);
						s_v2i index_arr[] = {v2i(64, 0), v2i(128, 0), v2i(192, 0)};
						draw_atlas_3d(g_r, v3(ve->pos, c_player_z), v2(2), make_color(1), game->sheet, index_arr[index], c_sprite_size);
					} break;
					invalid_default_case;
				}

				if(ve->timer > duration) {
					game->visual_effect_arr.remove_and_swap(ve_i--);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		visual effects end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		} break;
		case e_state_editor: {
			s_v2i mouse_index = pos_to_index(game->editor_cam.screen_to_world(g_mouse), c_editor_tile_size);

			if(is_key_pressed(g_input, c_key_s) && is_key_down(g_input, c_key_left_ctrl)) {
				u8* data = (u8*)la_get(platform_data->frame_arena, sizeof(game->map) + sizeof(int));
				u8* cursor = data;
				cursor = buffer_write(cursor, c_map_version);
				cursor = buffer_write(cursor, c_tile_version);
				cursor = buffer_write2(cursor, game->map.tile_arr.elements, sizeof(game->map.tile_arr.elements));
				cursor = buffer_write(cursor, c_save_point_version);
				cursor = buffer_write2(cursor, &game->map.save_point_arr, sizeof(game->map.save_point_arr));
				platform_data->write_file("platform_map.map", data, (u64)(cursor - data));
			}

			if(is_key_pressed(g_input, c_key_l) && is_key_down(g_input, c_key_left_ctrl)) {
				load_map(&game->map, platform_data);
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		move editor camera start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			s_v2 dir = {};
			float cam_speed = 1024;
			if(is_key_down(g_input, c_key_space)) {
				cam_speed *= 3;
			}
			if(is_key_down(g_input, c_key_w)) {
				dir.y -= 1;
			}
			if(is_key_down(g_input, c_key_a)) {
				dir.x -= 1;
			}
			if(is_key_down(g_input, c_key_s)) {
				dir.y += 1;
			}
			if(is_key_down(g_input, c_key_d)) {
				dir.x += 1;
			}
			dir = v2_normalized(dir);
			game->editor_cam.pos += dir * delta * cam_speed;
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		move editor camera end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			for(int i = 0; i < 10; i++) {
				if(is_key_pressed(g_input, c_key_0 + i)) {
					if(i == 0) {
						game->editor.curr_tile = 9;
					}
					else {
						game->editor.curr_tile = (s8)(i - 1);
					}
				}
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		add tile start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(game->editor.curr_tile == 0) {
				if(is_key_down(g_input, c_left_mouse)) {
					if(is_index_valid(mouse_index)) {
						game->map.tile_arr[mouse_index.y][mouse_index.x] = 1;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		add tile end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		add save point start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(game->editor.curr_tile == 1) {
				if(is_key_pressed(g_input, c_left_mouse)) {
					if(is_index_valid(mouse_index)) {
						b8 duplicate = false;
						foreach_val(save_point_i, save_point, game->map.save_point_arr) {
							if(save_point.pos == mouse_index) {
								duplicate = true;
								break;
							}
						}
						if(!duplicate) {
							s_save_point sp = {};
							sp.pos = mouse_index;
							game->map.save_point_arr.add_checked(sp);
						}
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		add save point end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			if(is_key_down(g_input, c_right_mouse)) {
				if(is_index_valid(mouse_index)) {
					game->map.tile_arr[mouse_index.y][mouse_index.x] = 0;
				}
			}

			s_recti bounds = game->editor_cam.get_tile_bounds();

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw tiles 2d start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			for(int y = bounds.y0; y <= bounds.y1; y++) {
				for(int x = bounds.x0; x <= bounds.x1; x++) {
					s8 tile = game->map.tile_arr[y][x];
					if(tile > 0) {
						s_v2 pos = v2(x, y) * c_editor_tile_size;
						pos = game->editor_cam.world_to_screen(pos);
						draw_rect(g_r, pos, 1, v2(c_editor_tile_size), make_color(1), {}, {.origin_offset = c_origin_topleft});
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw tiles 2d end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw save points start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(save_point_i, save_point, game->map.save_point_arr) {
				s_v2 pos = v2(save_point.pos) * v2(c_editor_tile_size);
				pos = game->editor_cam.world_to_screen(pos);
				draw_rect(g_r, pos, 2, v2(c_editor_tile_size), make_color(1, 0, 0), {}, {.origin_offset = c_origin_topleft});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw save points end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		} break;

		invalid_default_case;
	}

	// draw_cube(g_r, v3(0.0f, 0.0f, 0.0f), v3(1.0f, 1.0f, 1.0f), make_color(1));
	// draw_cube(g_r, v3(1.0f, 1.0f, 0.0f), v3(1.0f, 1.0f, 1.0f), make_color(1));
	// draw_texture_3d(g_r, v3(2, 0, 0), v3(1), make_color(1), game->apple_texture, {.shader = 2});
	// // draw_texture(g_r,	v2(200, 200), 1, v2(64), make_color(1), game->apple_texture, {.shader = 1});
	// // draw_texture(g_r,	v2(400, 200), 1, v2(64), make_color(1), game->apple_texture, {.shader = 0});

}

#ifdef m_build_dll
}
#endif // m_build_dll



static s_v2i pos_to_index(s_v2 pos, int tile_size)
{
	s_v2i result;
	result.x = floorfi(pos.x / tile_size);
	result.y = floorfi(pos.y / tile_size);
	return result;
}

static b8 is_index_valid(s_v2i index)
{
	return index.x >= 0 && index.x < c_tiles_right && index.y >= 0 && index.y < c_tiles_down;
}

static b8 index_has_tile(s_v2i index)
{
	if(!is_index_valid(index)) { return false; }
	return game->map.tile_arr[index.y][index.x] > 0;
}

s_recti s_camera2d::get_tile_bounds()
{
	s_recti result;
	result.x0 = floorfi(pos.x / c_editor_tile_size);
	result.x1 = result.x0 + ceilfi(c_base_res.x / c_editor_tile_size);
	result.y0 = floorfi(pos.y / c_editor_tile_size);
	result.y1 = result.y0 + ceilfi(c_base_res.y / c_editor_tile_size);

	result.x0 = clamp(result.x0, 0, c_tiles_right - 1);
	result.x1 = clamp(result.x1, 0, c_tiles_right - 1);
	result.y0 = clamp(result.y0, 0, c_tiles_down - 1);
	result.y1 = clamp(result.y1, 0, c_tiles_down - 1);

	return result;
}

static s_recti get_3d_tile_bounds(s_camera3d cam)
{
	s_recti result;
	result.x0 = floorfi(cam.pos.x / c_play_tile_size);
	result.x1 = result.x0 + ceilfi(c_base_res.x / c_play_tile_size);
	result.y0 = floorfi(cam.pos.y / c_play_tile_size);
	result.y1 = result.y0 + ceilfi(c_base_res.y / c_play_tile_size);

	result.x0 /= 2;
	result.x1 *= 2;
	result.y0 /= 2;
	result.y1 *= 2;

	result.x0 = clamp(result.x0, 0, c_tiles_right - 1);
	result.x1 = clamp(result.x1, 0, c_tiles_right - 1);
	result.y0 = clamp(result.y0, 0, c_tiles_down - 1);
	result.y1 = clamp(result.y1, 0, c_tiles_down - 1);

	return result;
}

static void load_map(s_map* map, s_platform_data* platform_data)
{
	u8* data = (u8*)platform_data->read_file("platform_map.map", platform_data->frame_arena, NULL);
	u8* cursor = data;
	if(data) {
		int map_version = buffer_read<int>(&cursor);
		assert(map_version == 1);

		int tile_version = buffer_read<int>(&cursor);
		assert(tile_version == 1);
		cursor = buffer_read2(cursor, game->map.tile_arr.elements, sizeof(game->map.tile_arr.elements));

		int save_point_version = buffer_read<int>(&cursor);
		assert(save_point_version == 1);
		cursor = buffer_read2(cursor, &game->map.save_point_arr, sizeof(game->map.save_point_arr));
	}
}

static s_sarray<s_tile_collision, 16> get_tile_collisions(s_v2 pos, s_v2 size, int tile_size)
{
	s_sarray<s_tile_collision, 16> result;
	s_v2i base_index = pos_to_index(pos, tile_size);
	for(int y = -1; y <= 1; y++) {
		for(int x = -1; x <= 1; x++) {
			s_v2i index = base_index + v2i(x, y);
			if(index_has_tile(index)) {
				s_v2 tile_center = v2(index) * v2(tile_size);
				b8 collides = rect_collides_rect_center(pos, size, tile_center, v2(tile_size));
				if(collides) {
					s_tile_collision collision = {};
					collision.tile_center = tile_center;
					collision.tile = game->map.tile_arr[index.y][index.x];
					result.add(collision);
				}
			}
		}
	}
	return result;
}