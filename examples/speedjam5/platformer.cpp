#define m_game

#include "../../src/platform_shared.h"
#include "variables.h"

#include "platformer.h"

static constexpr s_v2 c_base_res = {800, 800};
static constexpr s_v2 c_half_res = {c_base_res.x * 0.5f, c_base_res.y * 0.5f};

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
};

struct s_projectile
{
	int timer;
	s_v2 prev_pos;
	s_v2 pos;
	s_v2 dir;
};

struct s_game
{
	b8 initialized;
	b8 follow_player;
	b8 reset_game;
	e_state state;
	int reset_player;
	f64 timer;
	s_framebuffer* particle_framebuffer;
	s_framebuffer* text_framebuffer;
	s_camera3d cam;
	s_camera2d editor_cam;
	s_texture sheet;
	s_texture noise;
	s_texture save_point_texture;
	s_carray<s_texture, e_tile_count> tile_texture_arr;
	s_sarray<s_projectile, c_max_projectiles> projectile_arr;
	s_rng rng;
	s_font* font;
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
	g_r = renderer;

	if(!game->initialized) {
		game->initialized = true;
		game->rng.seed = platform_data->get_random_seed();
		g_r->set_vsync(true);
		game->sheet = g_r->load_texture(renderer, "examples/speedjam5/sheet.png");
		game->save_point_texture = g_r->load_texture(renderer, "examples/speedjam5/save_point.png");
		game->tile_texture_arr[e_tile_normal] = g_r->load_texture(renderer, "examples/speedjam5/tile_normal.png");
		game->tile_texture_arr[e_tile_nullify_explosion] = g_r->load_texture(renderer, "examples/speedjam5/tile_nullify_explosion.png");
		game->tile_texture_arr[e_tile_spike] = g_r->load_texture(renderer, "examples/speedjam5/tile_spike.png");
		game->tile_texture_arr[e_tile_platform] = g_r->load_texture(renderer, "examples/speedjam5/tile_platform.png");
		game->explosion_sound = platform_data->load_sound(platform_data, "examples/speedjam5/explosion.wav", platform_data->frame_arena);
		game->save_sound = platform_data->load_sound(platform_data, "examples/speedjam5/save.wav", platform_data->frame_arena);
		game->thud_sound = platform_data->load_sound(platform_data, "examples/speedjam5/thud.wav", platform_data->frame_arena);
		game->shoot_sound = platform_data->load_sound(platform_data, "examples/speedjam5/shoot.wav", platform_data->frame_arena);
		game->jump_sound = platform_data->load_sound(platform_data, "examples/speedjam5/jump.wav", platform_data->frame_arena);
		game->win_sound = platform_data->load_sound(platform_data, "examples/speedjam5/win.wav", platform_data->frame_arena);
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
		game->editor_cam.zoom = 1;
		game->editor.curr_tile = e_tile_normal;
		game->follow_player = true;
		game->reset_game = true;

		load_map(&game->map, platform_data);
	}


	switch(game->state) {
		case e_state_play: {

			if(game->reset_game) {
				game->reset_game = false;
				game->curr_save_point = 0;
				game->timer = 0;
				game->reset_player = 2;
			}

			if(game->reset_player > 0) {
				game->player.flip_x = false;
				game->player.vel = {};
				game->player.jumps_left = 1;
				game->player.shoot_timer = c_shoot_delay;
				game->projectile_arr.count = 0;
				if(game->curr_save_point == 0) {
					game->timer = 0;
				}
				if(game->reset_player == 2) {
					game->timer = 0;
					game->curr_save_point = 0;
				}
				if(game->map.save_point_arr.count > 0) {
					game->player.pos = index_to_pos(game->map.save_point_arr[game->curr_save_point].pos, c_play_tile_size);
					game->player.prev_pos = game->player.pos;
				}
				game->reset_player = 0;
			}

			game->timer += 1.0 / c_updates_per_second;

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		player update start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* player = &game->player;
				player->prev_pos = player->pos;

				float input_vel = 0;
				if(game->follow_player) {
					if(is_key_down(g_input, c_key_a) || is_key_down(g_input, c_key_left)) {
						input_vel -= c_player_speed;
					}
					if(is_key_down(g_input, c_key_d) || is_key_down(g_input, c_key_right)) {
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
					if(game->projectile_arr.add_checked(p)) {
						platform_data->play_sound(game->shoot_sound);
					}
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
					input_vel *= 0.03f;
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

				if(is_key_pressed(g_input, c_key_space) || is_key_pressed(g_input, c_key_up)) {
					if(player->jumps_left > 0) {
						platform_data->play_sound(game->jump_sound);
						if(player->vel.y > 0) {
							player->vel.y = -c_jump_strength;
						}
						else {
							player->vel.y += -c_jump_strength;
						}
						player->jumps_left -= 1;
					}
				}

				b8 pushed = false;
				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		player x collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				float x_vel = player->vel.x + input_vel;
				if(x_vel > 0) {
					player->flip_x = false;
					player->state = 1;
				}
				else if(x_vel < 0) {
					player->flip_x = true;
					player->state = 1;
				}
				else {
					player->state = 0;
				}
				player->pos.x += x_vel;
				auto collision_arr = get_tile_collisions(player->pos, c_player_collision_size, c_play_tile_size);
				foreach_val(collision_i, collision, collision_arr) {
					if(collision.tile == e_tile_spike) {
						game->reset_player = 1;
					}
					else if(collision.tile == e_tile_platform) { continue; }
					else if(!pushed) {
						pushed = true;
						if(x_vel > 0) {
							player->pos.x = collision.tile_center.x - (c_play_tile_size * 0.5f + c_player_collision_size.x * 0.5f) - c_small;
						}
						else {
							player->pos.x = collision.tile_center.x + (c_play_tile_size * 0.5f + c_player_collision_size.x * 0.5f) + c_small;
						}
						player->vel.x = 0;
					}
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		player x collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		player y collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				player->pos.y += player->vel.y;
				b8 on_ground = false;
				collision_arr = get_tile_collisions(player->pos, c_player_collision_size, c_play_tile_size);
				pushed = false;
				foreach_val(collision_i, collision, collision_arr) {
					if(collision.tile == e_tile_spike) {
						game->reset_player = 1;
					}
					else if(collision.tile == e_tile_platform) {
						float player_bottom = player->pos.y + c_player_collision_size.y * 0.5f;
						float tile_top = collision.tile_center.y - c_play_tile_size * 0.5f;
						float diff = fabsf(player_bottom - tile_top);
						float diff_check = 0.1f + player->vel.y;
						if(!pushed && player->vel.y > 0 && diff <= diff_check) {
							player->pos.y = collision.tile_center.y - (c_play_tile_size * 0.5f + c_player_collision_size.y * 0.5f) - c_small;
							player->jumps_left = c_max_jumps;
							on_ground = true;
							if(player->vel.y >= 0.1f) {
								platform_data->play_sound(game->thud_sound);
							}
							player->vel.y = 0;
							pushed = true;
						}
					}

					else if(!pushed) {
						if(player->vel.y > 0) {
							if(player->vel.y >= 0.1f) {
								platform_data->play_sound(game->thud_sound);
							}
							player->pos.y = collision.tile_center.y - (c_play_tile_size * 0.5f + c_player_collision_size.y * 0.5f) - c_small;
							player->jumps_left = c_max_jumps;
							on_ground = true;
						}
						else {
							player->pos.y = collision.tile_center.y + (c_play_tile_size * 0.5f + c_player_collision_size.y * 0.5f) + c_small;
						}
						player->vel.y = 0;
						pushed = true;
					}
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		player y collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		check save point collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				foreach_val(save_point_i, save_point, game->map.save_point_arr) {
					if(game->curr_save_point == save_point_i) { continue; }
					s_v2 save_point_pos = index_to_pos(save_point.pos, c_play_tile_size);
					b8 collides = rect_collides_rect_center(player->pos, c_player_collision_size, save_point_pos, c_save_point_collision_size);
					if(collides) {
						game->curr_save_point = save_point_i;
						platform_data->play_sound(game->save_sound);
					}
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		check save point collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		check end point collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				{
					s_v2 end_point_pos = index_to_pos(game->map.end_point.pos, c_play_tile_size);
					b8 collides = rect_collides_rect_center(player->pos, c_player_collision_size, end_point_pos, c_end_point_size);
					if(collides) {
						game->state = e_state_victory;
						platform_data->play_sound(game->win_sound);
					}
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		check end point collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


				if(is_key_pressed(g_input, c_key_r) || is_key_pressed(g_input, c_key_enter)) {
					if(is_key_down(g_input, c_key_left_ctrl)) {
						game->reset_player = 2;
					}
					else {
						game->reset_player = 1;
					}
				}

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
				b8 explode = false;
				foreach_val(collision_i, collision, collision_arr) {
					if(collision.tile == e_tile_spike) { continue; }
					else if(collision.tile == e_tile_platform) { continue; }
					remove = true;
					if(collision.tile != e_tile_nullify_explosion) {
						explode = true;
					}
				}
				if(explode) {
					s_visual_effect ve = {};
					ve.type = e_visual_effect_projectile_explosion;
					ve.pos = projectile->pos;
					game->visual_effect_arr.add(ve);
					s_v2 dir = v2_normalized(game->player.pos - projectile->pos);
					float distance = v2_distance(game->player.pos, projectile->pos);
					float strength = smoothstep(c_far_push_range, c_near_push_range, distance) * c_push_strength;
					game->player.vel += dir * strength;
					platform_data->play_sound(game->explosion_sound);
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

	if(g_input->wheel_movement > 0) {
		game->editor_cam.zoom *= 1.1f;
	}

	else if(g_input->wheel_movement < 0) {
		game->editor_cam.zoom *= 0.9f;
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

			#ifdef m_debug
			if(is_key_pressed(g_input, c_right_mouse)) {
				game->player.pos = ray_at_z(game->ray, c_player_z).xy;
				game->player.vel = {};
			}
			#endif // m_debug

			{
				s_time_data data = process_time(game->timer);
				char* text = format_text("%02i:%02i.%03i", data.minutes, data.seconds, data.ms);
				draw_text(g_r, text, v2(0), 10, 32, make_color(1), false, game->font);
			}

			if(!game->follow_player) {
				constexpr float c_cam_speed = 50;
				if(is_key_down(g_input, c_key_w)) {
					game->cam.pos.z += 1 * delta * c_cam_speed;
				}
				if(is_key_down(g_input, c_key_a)) {
					game->cam.pos.x -= 1 * delta * c_cam_speed;
				}
				if(is_key_down(g_input, c_key_s)) {
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
			s_m4 projection =  m4_perspective(90, c_base_res.x / c_base_res.y, 0.01f, 10000.0f);
			g_r->view_projection = m4_multiply(projection, view);

			game->ray = get_ray(g_mouse, c_base_res, game->cam, view, projection);


			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw play tiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			s_recti bounds = get_3d_tile_bounds(game->cam);
			for(int y = bounds.y0; y <= bounds.y1; y++) {
				for(int x = bounds.x0; x <= bounds.x1; x++) {
					s8 tile = game->map.tile_arr[y][x];
					if(tile > 0) {
						if(tile == e_tile_spike) {
							draw_texture_3d(g_r, v3((float)x * c_play_tile_size, (float)y * c_play_tile_size, 0.0f), v2(c_play_tile_size), make_color(1), game->tile_texture_arr[tile]);
						}
						else {
							draw_textured_cube(g_r, v3((float)x * c_play_tile_size, (float)y * c_play_tile_size, 0.0f), v3(c_play_tile_size), make_color(1), game->tile_texture_arr[tile]);
						}
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw play tiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw save points start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(save_point_i, save_point, game->map.save_point_arr) {
				s_v2 pos = v2(save_point.pos) * v2(c_play_tile_size);
				// draw_rect(g_r, pos, 2, v2(game->editor_cam.scale(c_editor_tile_size)), make_color(1, 0, 0), {}, {.origin_offset = c_origin_topleft});
				draw_texture_3d(g_r, v3(pos, c_player_z), c_save_point_visual_size, make_color(1), game->save_point_texture);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw save points end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw end point start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_v2 pos = v2(game->map.end_point.pos) * v2(c_play_tile_size);
				draw_atlas_3d(g_r, v3(pos, c_player_z), c_end_point_size, make_color(1), game->sheet, v2i(0, 64), v2i(64, 64));
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw end point end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* player = &game->player;
				player->animation_timer += delta;
				s_v2 pos = lerp(player->prev_pos, player->pos, interp_dt);
				s_v2i idle_arr[] = {v2i(64, 64), v2i(128, 64)};
				s_v2i run_arr[] = {v2i(196, 64), v2i(256, 64)};
				s_v2i index = {};
				if(player->state == 0) {
					int i = roundfi(fmodf(player->animation_timer / 0.5f, 1));
					index = idle_arr[i];
				}
				else if(player->state == 1) {
					int i = roundfi(fmodf(player->animation_timer / 0.15f, 1));
					index = run_arr[i];
				}
				draw_atlas_3d(g_r, v3(pos, c_player_z), c_player_visual_size, make_color(0.9f), game->sheet, index, v2i(64, 64), {.flip_x = player->flip_x});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw projectiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(projectile_i, projectile, game->projectile_arr) {
				s_v2 pos = lerp(projectile.prev_pos, projectile.pos, interp_dt);
				// draw_cube(g_r, v3(pos.x, pos.y, c_player_z), c_projectile_size, make_color(1.0f, 1.0f, 0.1f));
				draw_atlas_3d(
					g_r, v3(pos, c_player_z), c_projectile_visual_size, make_color(1), game->sheet, v2i(256, 0), c_sprite_size, {}, {.rotation = v2_angle(projectile.dir)}
				);
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
						draw_atlas_3d(g_r, v3(ve->pos, c_player_z - 0.01f), v2(4), make_color(1), game->sheet, index_arr[index], c_sprite_size);
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
				cursor = buffer_write(cursor, c_end_point_version);
				cursor = buffer_write(cursor, game->map.end_point);
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

			for(int i = 0; i <= 9; i++) {
				if(is_key_pressed(g_input, c_key_1 + i)) {
					game->editor.curr_tile = (e_tile)(i + 1);
				}
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		add tile start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(game->editor.curr_tile > e_tile_invalid && game->editor.curr_tile < e_tile_count) {
				if(is_key_down(g_input, c_left_mouse)) {
					if(is_index_valid(mouse_index)) {
						game->map.tile_arr[mouse_index.y][mouse_index.x] = game->editor.curr_tile;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		add tile end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		add save point start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(game->editor.curr_tile == e_tile_count) {
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

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		add end point start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			else if(game->editor.curr_tile == e_tile_count + 1) {
				if(is_key_pressed(g_input, c_left_mouse)) {
					if(is_index_valid(mouse_index)) {
						game->map.end_point.pos = mouse_index;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		add end point end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		delete start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				b8 deleted_save_point = false;
				if(is_key_down(g_input, c_right_mouse)) {
					foreach_val(save_point_i, save_point, game->map.save_point_arr) {
						if(mouse_index == save_point.pos) {
							deleted_save_point = true;
							game->map.save_point_arr.remove_and_swap(save_point_i--);
						}
					}
					if(!deleted_save_point && is_index_valid(mouse_index)) {
						game->map.tile_arr[mouse_index.y][mouse_index.x] = e_tile_invalid;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		delete end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			s_recti bounds = game->editor_cam.get_tile_bounds();

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw tiles 2d start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			for(int y = bounds.y0; y <= bounds.y1; y++) {
				for(int x = bounds.x0; x <= bounds.x1; x++) {
					s8 tile = game->map.tile_arr[y][x];
					if(tile > 0) {
						s_v2 pos = v2(x, y) * c_editor_tile_size;
						pos = game->editor_cam.world_to_screen(pos);
						draw_texture(g_r, pos, 1, v2(game->editor_cam.scale(c_editor_tile_size)), make_color(1), game->tile_texture_arr[tile], {}, {.origin_offset = c_origin_topleft});
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw tiles 2d end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw save points start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(save_point_i, save_point, game->map.save_point_arr) {
				s_v2 pos = v2(save_point.pos) * v2(c_editor_tile_size);
				pos = game->editor_cam.world_to_screen(pos);
				draw_texture(g_r, pos, 2, v2(game->editor_cam.scale(c_editor_tile_size)), make_color(1), game->save_point_texture, {}, {.origin_offset = c_origin_topleft});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw save points end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw end point start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_v2 pos = v2(game->map.end_point.pos) * v2(c_editor_tile_size);
				pos = game->editor_cam.world_to_screen(pos);
				draw_atlas(g_r, pos, 2, v2(game->editor_cam.scale(c_editor_tile_size)), make_color(1), game->sheet, v2i(0, 64), c_sprite_size, {}, {.origin_offset = c_origin_topleft});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw end point end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		} break;

		case e_state_victory: {

			if(is_key_pressed(g_input, c_key_r)) {
				game->reset_game = true;
				game->state = e_state_play;
			}

			s_time_data data = process_time(game->timer);
			char* text = format_text("%02i:%02i.%03i", data.minutes, data.seconds, data.ms);
			draw_text(g_r, text, c_half_res, 10, 64, make_color(1), true, game->font);
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

static s_v2 index_to_pos(s_v2i index, int tile_size)
{
	return v2(index) * v2(tile_size);
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
	s_v2 topleft = screen_to_world({});
	s_v2 bottomright = screen_to_world(c_base_res);

	result.x0 = floorfi(topleft.x / c_editor_tile_size);
	result.x1 = floorfi(bottomright.x / c_editor_tile_size);

	result.y0 = floorfi(topleft.y / c_editor_tile_size);
	result.y1 = floorfi(bottomright.y / c_editor_tile_size);

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

		int end_point_version = buffer_read<int>(&cursor);
		assert(end_point_version == 1);
		map->end_point = buffer_read<s_end_point>(&cursor);

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
				e_tile tile = game->map.tile_arr[index.y][index.x];
				s_v2 tile_center = v2(index) * v2(tile_size);
				float temp_tile_size = (float)tile_size;
				if(tile == e_tile_spike) {
					temp_tile_size *= 0.8f;
				}
				b8 collides = rect_collides_rect_center(pos, size, tile_center, v2(temp_tile_size));
				if(collides) {
					s_tile_collision collision = {};
					collision.tile_center = tile_center;
					collision.tile = tile;
					result.add(collision);
				}
			}
		}
	}
	return result;
}