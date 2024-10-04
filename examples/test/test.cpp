#define m_game

#include "../../src/platform_shared.h"
#include "variables.h"

#include "platformer.h"

static constexpr s_v2 c_base_res = {800, 800};
// static constexpr s_v2 c_base_res = {1366, 768};
static constexpr s_v2 c_half_res = {c_base_res.x * 0.5f, c_base_res.y * 0.5f};

static s_input* g_input;
static s_game* game;
static s_game_renderer* g_r;
static s_v2 g_mouse;
static s_platform_data* g_platform_data;
static s_ui* g_ui;
static float g_delta = 0;

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
	g_platform_data = platform_data;
	g_ui = &game->ui;

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
		game->death_sound_arr[0] = platform_data->load_sound(platform_data, "examples/speedjam5/death1.wav", platform_data->frame_arena);
		game->death_sound_arr[1] = platform_data->load_sound(platform_data, "examples/speedjam5/death2.wav", platform_data->frame_arena);
		game->death_sound_arr[2] = platform_data->load_sound(platform_data, "examples/speedjam5/death3.wav", platform_data->frame_arena);
		game->player_run_texture = g_r->load_texture(renderer, "examples/speedjam5/player_run.png");
		game->player_idle_texture = g_r->load_texture(renderer, "examples/speedjam5/player_idle.png");
		// game->player_charge_texture = g_r->load_texture(renderer, "examples/speedjam5/player_charge.png");
		// game->player_leap_texture = g_r->load_texture(renderer, "examples/speedjam5/player_leap.png");
		game->player_jump_texture = g_r->load_texture(renderer, "examples/speedjam5/player_jump.png");
		game->player_fall_texture = g_r->load_texture(renderer, "examples/speedjam5/player_fall.png");

		game->main_fbo = g_r->make_framebuffer(g_r, v2i(c_base_res));
		game->fbo_arr[0] = g_r->make_framebuffer(g_r, v2i(c_base_res));
		game->fbo_arr[1] = g_r->make_framebuffer(g_r, v2i(c_base_res * 0.25f));
		game->bloom_fbo = g_r->make_framebuffer_with_existing_depth(g_r, v2i(c_base_res), game->main_fbo->depth);

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
		game->reset_game = true;

		if(g_platform_data->register_leaderboard_client) {
			g_platform_data->register_leaderboard_client();
		}

		// set_state(e_state_input_name);
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
					game->player.did_any_action = false;
				}
				if(game->reset_player == 2) {
					game->timer = 0;
					game->curr_save_point = 0;
					game->player.did_any_action = false;
				}
				if(game->map.save_point_arr.count > 0) {
					game->player.pos = index_to_pos(game->map.save_point_arr[game->curr_save_point].pos, c_play_tile_size);
					game->player.prev_pos = game->player.pos;
				}
				game->reset_player = 0;

				foreach_ptr(jump_refresher_i, jump_refresher, game->map.jump_refresher_arr) {
					jump_refresher->timer = 0;
					jump_refresher->in_cooldown = false;
				}
			}

			if(game->player.did_any_action) {
				game->timer += 1.0 / c_updates_per_second;
			}


			if(is_key_pressed(g_input, c_key_escape)) {
				set_state(e_state_map_select);
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		player update start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* player = &game->player;
				player->prev_pos = player->pos;

				if(!is_key_down(g_input, c_left_mouse)) {
					player->released_left_button_since_death = true;
				}

				float input_vel = 0;
				if(is_key_down(g_input, c_key_a) || is_key_down(g_input, c_key_left)) {
					input_vel -= c_player_speed;
					player->did_any_action = true;
				}
				if(is_key_down(g_input, c_key_d) || is_key_down(g_input, c_key_right)) {
					input_vel += c_player_speed;
					player->did_any_action = true;
				}
				input_vel = clamp(input_vel, -c_max_input_vel, c_max_input_vel);

				player->shoot_timer = at_most(c_shoot_delay, player->shoot_timer + 1);
				if(player->released_left_button_since_death && player->shoot_timer >= c_shoot_delay && is_key_down(g_input, c_left_mouse)) {
					player->did_any_action = true;
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

				if(is_key_pressed(g_input, c_key_space) || is_key_pressed(g_input, c_key_up) || is_key_pressed(g_input, c_key_w)) {
					if(player->jumps_left > 0) {
						player->did_any_action = true;
						player->jump_time = c_player_jump_time;
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
				float x_vel = player->vel.x + input_vel;
				player->last_x_vel = x_vel;

				s_v2 pos_before_moving = game->player.pos;
				b8 do_trail = false;
				if(fabsf(x_vel) + fabsf(game->player.vel.y) > 0.2f) {
					do_trail = true;
				}

				b8 hit_spike = false;
				s_v2 killer_spike_pos;
				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		player x collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				player->pos.x += x_vel;
				auto collision_arr = get_tile_collisions(player->pos, c_player_collision_size, c_play_tile_size);
				foreach_val(collision_i, collision, collision_arr) {
					if(collision.tile == e_tile_spike) {
						hit_spike = true;
						killer_spike_pos = collision.tile_center;
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
						hit_spike = true;
						killer_spike_pos = collision.tile_center;
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

				if(do_trail) {
					float distance = v2_distance(pos_before_moving, game->player.pos);
					float max_movement = v2_length(c_player_visual_size) * 0.01f;
					int steps = ceilfi(distance / max_movement);
					for(int i = 0; i < steps; i++) {
						float p = (i + 1) / (float)steps;
						s_v2 pos = lerp(pos_before_moving, game->player.pos, p);
						s_trail trail = {};
						trail.pos = pos;
						trail.draw_data = get_player_draw_data(*player);
						// trail.texture = get_player_sprite_index(*player);
						trail.flip_x = player->flip_x;
						game->trail_arr.add(trail);
					}
				}

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		check jump refresher collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				foreach_ptr(jump_refresher_i, jump_refresher, game->map.jump_refresher_arr) {
					if(jump_refresher->in_cooldown) { continue; }
					s_v2 jump_refresher_pos = index_to_pos(jump_refresher->pos, c_play_tile_size);
					b8 collides = rect_collides_rect_center(player->pos, c_player_collision_size, jump_refresher_pos, c_jump_refresher_collision_size);
					if(collides) {
						player->jumps_left = 1;
						platform_data->play_sound(game->save_sound);
						do_particles(200, v3(jump_refresher_pos, c_particle_z), {
							.shrink = 0.5f,
							.slowdown = 0.0f,
							.duration = 0.5f,
							.duration_rand = 1,
							.speed = 10.0f,
							.speed_rand = 0.0f,
							.angle_rand = 1,
							.radius = 0.1f,
							.radius_rand = 0,
							.color = v3(0.2f, 0.2f, 0.8f),
							.color_rand = v3(0.2f, 0.2f, 0.2f),
						});
						jump_refresher->in_cooldown = true;
						jump_refresher->timer = 0;
					}
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		check jump refresher collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		check save point collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				foreach_val(save_point_i, save_point, game->map.save_point_arr) {
					if(game->curr_save_point == save_point_i) { continue; }
					s_v2 save_point_pos = index_to_pos(save_point.pos, c_play_tile_size);
					b8 collides = rect_collides_rect_center(player->pos, c_player_collision_size, save_point_pos, c_save_point_collision_size);
					if(collides) {
						game->curr_save_point = save_point_i;
						platform_data->play_sound(game->save_sound);

						do_particles(200, v3(save_point_pos, c_particle_z), {
							.shrink = 0.5f,
							.slowdown = 2.0f,
							.duration = 2.0f,
							.duration_rand = 1,
							.speed = 40.0f,
							.speed_rand = 0.0f,
							.angle_rand = 1,
							.radius = 0.1f,
							.radius_rand = 0,
							.color = v3(0.2f, 0.8f, 0.2f),
							.color_rand = v3(1, 0.2f, 1),
						});

					}
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		check save point collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		check end point collision start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				{
					s_v2 end_point_pos = index_to_pos(game->map.end_point.pos, c_play_tile_size);
					b8 collides = rect_collides_rect_center(player->pos, c_player_collision_size, end_point_pos, c_end_point_size);
					if(collides) {
						#ifdef m_emscripten
						constexpr b8 are_we_on_web = true;
						#else // m_emscripten
						constexpr b8 are_we_on_web = false;
						#endif // m_emscripten

						platform_data->play_sound(game->win_sound);
						if constexpr(are_we_on_web) {
							if(platform_data->leaderboard_nice_name.len > 0) {
								set_state(e_state_leaderboard);
								game->leaderboard_state.coming_from_win = true;
								platform_data->submit_leaderboard_score(
									(int)round(game->timer * 1000.0), c_map_data[game->curr_map].leaderboard_id, on_leaderboard_score_submitted
								);
							}
							else {
								set_state(e_state_input_name);
							}
						}
						else {
							set_state(e_state_leaderboard);
							game->leaderboard_state.coming_from_win = true;
						}
					}
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		check end point collision end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				if(hit_spike) {
					game->reset_player = 1;
					if(is_key_down(g_input, c_left_mouse)) {
						player->released_left_button_since_death = false;
					}

					platform_data->play_sound(game->death_sound_arr[game->rng.rand_range_ie(0, c_max_death_sounds)]);

					do_particles(200, v3(killer_spike_pos, c_particle_z), {
						.shrink = 1.0f,
						.slowdown = 0.0f,
						.duration = 1.0f,
						.duration_rand = 1,
						.speed = 10.0f,
						.speed_rand = 0.0f,
						.angle_rand = 1,
						.radius = 0.1f,
						.radius_rand = 1,
						.color = v3(1.0f, 0.1f, 0.1f),
						.color_rand = v3(0.2f, 1.0f, 1.0f),
					});
				}

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
					// s_visual_effect ve = {};
					// ve.type = e_visual_effect_projectile_explosion;
					// ve.pos = projectile->pos;
					// game->visual_effect_arr.add(ve);
					s_v2 dir = v2_normalized(game->player.pos - projectile->pos);
					float distance = v2_distance(game->player.pos, projectile->pos);
					float strength = smoothstep(c_far_push_range, c_near_push_range, distance) * c_push_strength;
					game->player.vel += dir * strength;
					platform_data->play_sound(game->explosion_sound);

					do_particles(500, v3(projectile->pos, c_particle_z), {
						.shrink = 1.0f,
						.slowdown = 2.0f,
						.duration = 1.0f,
						.duration_rand = 1,
						.speed = 4.0f,
						.speed_rand = 0.0f,
						.angle_rand = 1,
						.radius = 0.3f,
						.radius_rand = 0,
						.color = v3(0.8f, 0.2f, 0.2f),
						.color_rand = v3(0.2f, 1.0f, 1.0f),
					});
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

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update jump refreshers start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_ptr(jump_refresher_i, jump_refresher, game->map.jump_refresher_arr) {
				if(!jump_refresher->in_cooldown) { continue; }
				jump_refresher->timer += 1;
				if(jump_refresher->timer >= 300) {
					jump_refresher->in_cooldown = false;
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update jump refreshers end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		} break;
	}
}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt)
{
	static_assert(sizeof(s_game) <= c_game_memory);

	g_r->clear_framebuffer(game->fbo_arr[0], zero, c_default_fbo_clear_flags);
	g_r->clear_framebuffer(game->fbo_arr[1], zero, c_default_fbo_clear_flags);
	g_r->clear_framebuffer(game->main_fbo, zero, c_default_fbo_clear_flags);
	g_r->clear_framebuffer(game->bloom_fbo, zero, c_default_fbo_clear_flags);

	g_mouse = platform_data->mouse;

	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = &platform_data->render_input;
	g_platform_data = platform_data;
	g_ui = &game->ui;

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

	s_m4 ortho = m4_orthographic(0, c_base_res.x, c_base_res.y, 0, -100, 100);

	g_delta = (float)platform_data->frame_time;
	game->render_time += g_delta;

	if(g_input->wheel_movement > 0) {
		game->editor_cam.zoom *= 1.1f;
	}

	else if(g_input->wheel_movement < 0) {
		game->editor_cam.zoom *= 0.9f;
	}

	#ifdef m_debug
	if(is_key_pressed(g_input, c_key_f1)) {
		if(game->state == e_state_editor) {

			set_state(e_state_play);
		}
		else {
			set_state(e_state_editor);
		}
	}
	#endif // m_debug

	switch(game->state) {

		case e_state_map_select: {

			s_map_select_state* state = &game->map_select_state;
			start_render_pass(g_r);

			ui_start(state->map_selected);
			s_v2 pos = v2(c_half_res.x * 0.2f, c_half_res.y * 0.1f);
			constexpr float font_size = 32;
			for(int map_i = 0; map_i < array_count(c_map_data); map_i++) {
				s_map_data md = c_map_data[map_i];
				if(ui_button(md.name, pos, {.font_size = font_size})) {
					game->curr_map = map_i;
					load_map(&game->map, map_i, platform_data);
					set_state(e_state_play);
				}
				s_v2 pos2 = pos;
				pos2.x += c_base_button_size.x + 80;
				#ifdef m_emscripten
				if(ui_button(format_text("Leaderboard##leaderboard%i", map_i), pos2, {.font_size = font_size, .size_x = 220})) {

					// @Hack(tkap, 05/06/2024): Otherwise we will get leaderboard for the last map we entered (or 0 by default)
					game->curr_map = map_i;

					set_state(e_state_leaderboard);
					on_leaderboard_score_submitted();
				}
				#endif // m_emscripten
				pos.y += c_base_button_size.y * 1.1f;
			}

			if(ui_button(format_text("Bloom: %s", game->disable_bloom ? "Off": "On"), c_base_res * v2(0.05f, 0.92f), {.font_size = 32, .size_x = 180})) {
				game->disable_bloom = !game->disable_bloom;
			}

			state->map_selected = ui_end();
			g_r->end_render_pass(g_r, {.blend_mode = e_blend_mode_normal, .view_projection = ortho, .framebuffer = game->main_fbo});
		} break;

		case e_state_play: {

			{
				s_v2 pos = lerp(game->player.prev_pos, game->player.pos, interp_dt);
				game->cam.pos.xy = pos;
				game->cam.pos.z = 0;
				game->cam.pos += c_map_data[game->curr_map].cam_offset;
			}

			s_m4 view = get_camera_view(game->cam);
			s_m4 projection =  m4_perspective(90, c_base_res.x / c_base_res.y, 1.0f, 1000.0f);
			s_m4 view_projection = m4_multiply(projection, view);

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		background start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				start_render_pass(g_r);
				draw_rect(g_r, c_half_res, 0, c_base_res, make_color(1), {}, {.effect_id = 5});
				g_r->end_render_pass(
					g_r, {.dont_write_depth = true, .cam_pos = game->cam.pos, .view_projection = ortho, .framebuffer = game->main_fbo}
				);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		background end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			start_render_pass(g_r);

			#ifdef m_debug
			if(is_key_pressed(g_input, c_right_mouse)) {
				game->player.pos = ray_at_z(game->ray, c_player_z).xy;
				game->player.vel = {};
			}
			if(is_key_pressed(g_input, c_key_f2)) {
				game->curr_save_point = (game->curr_save_point + 1) % game->map.save_point_arr.count;
				game->reset_player = true;
			}
			if(is_key_pressed(g_input, c_key_f3)) {
				game->player.pos = v2(game->map.end_point.pos) * v2(c_play_tile_size);
			}
			// if(is_key_pressed(g_input, c_key_f4)) {
			// 	static int r = 0;
			// 	r = platform_data->cycle_between_available_resolutions(r);
			// }
			#endif // m_debug

			game->ray = get_ray(g_mouse, c_base_res, game->cam, view, projection);


			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw play tiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			s_recti bounds = get_3d_tile_bounds(game->cam);
			for(int y = bounds.y0; y <= bounds.y1; y++) {
				for(int x = bounds.x0; x <= bounds.x1; x++) {
					s8 tile = game->map.tile_arr[y][x];
					if(tile > 0) {
						s_v3 pos = v3((float)x * c_play_tile_size, (float)y * c_play_tile_size, 0.0f);
						if(tile == e_tile_spike) {
							draw_texture_3d(g_r, pos, v2(c_play_tile_size), make_color(1), game->tile_texture_arr[tile]);

							if(game->dev_menu.show_hitboxes) {
								draw_rect_3d(g_r, pos - v3(0.0f, 0.0f, c_small), v2(c_play_tile_size * c_spike_collision_size_multiplier), make_color(1), {}, {.effect_id = 8});
							}
						}
						else {
							draw_textured_cube(g_r, pos, v3(c_play_tile_size), make_color(1), game->tile_texture_arr[tile]);
						}
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw play tiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw jump refreshers start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(jump_refresher_i, jump_refresher, game->map.jump_refresher_arr) {
				if(jump_refresher.in_cooldown) { continue; }
				s_v2 pos = v2(jump_refresher.pos) * v2(c_play_tile_size);
				draw_atlas_3d(g_r, v3(pos, c_player_z), c_jump_refresher_visual_size, make_color(1), game->sheet, v2i(64, 128), c_sprite_size);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw jump refreshers end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw save points start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(save_point_i, save_point, game->map.save_point_arr) {
				s_v2 pos = v2(save_point.pos) * v2(c_play_tile_size);
				s_v4 color = make_color(1.0f, 0.1f, 0.1f);
				if(game->curr_save_point == save_point_i) {
					color = make_color(1);
				}
				// draw_rect(g_r, pos, 2, v2(game->editor_cam.scale(c_editor_tile_size)), make_color(1, 0, 0), {}, {.origin_offset = c_origin_topleft});
				draw_texture_3d(g_r, v3(pos, c_player_z), c_save_point_visual_size, color, game->save_point_texture);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw save points end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw end point start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_v2 pos = v2(game->map.end_point.pos) * v2(c_play_tile_size);
				// draw_atlas_3d(g_r, v3(pos, c_player_z), c_end_point_size, make_color(1), game->sheet, v2i(0, 64), v2i(64, 64));

				do_particles(1, v3(pos, c_particle_z), {
					.shrink = 1.0f,
					.slowdown = 0.0f,
					.duration = 1.0f,
					.duration_rand = 0,
					.speed = 5.0f,
					.speed_rand = 1.0f,
					.angle_rand = 1,
					.radius = 0.4f,
					.radius_rand = 0.0f,
					.color = v3(0.3f, 0.1f, 0.1f),
					.color_rand = v3(0.0f),
				});


			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw end point end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw projectiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(projectile_i, projectile, game->projectile_arr) {
				s_v2 pos = lerp(projectile.prev_pos, projectile.pos, interp_dt);
				// draw_cube(g_r, v3(pos.x, pos.y, c_player_z), c_projectile_size, make_color(1.0f, 1.0f, 0.1f));
				draw_atlas_3d(
					g_r, v3(pos, c_player_z - c_small), c_projectile_visual_size, make_color(1), game->sheet, v2i(256, 0), c_sprite_size, {}, {.rotation = v2_angle(projectile.dir)}
				);

				if(game->dev_menu.show_hitboxes) {
					draw_rect_3d(g_r, v3(pos, c_player_z - c_small), c_projectile_collision_size, make_color(1), {}, {.effect_id = 8});
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw projectiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		visual effects start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_ptr(ve_i, ve, game->visual_effect_arr) {
				ve->timer += g_delta;
				float duration = 0;
				switch(ve->type) {
					case e_visual_effect_projectile_explosion: {
						duration = 0.25f;
						int index = clamp(roundfi(ve->timer / 0.25f * 2), 0, 2);
						s_v2i index_arr[] = {v2i(64, 0), v2i(128, 0), v2i(192, 0)};
						draw_atlas_3d(g_r, v3(ve->pos, c_player_z - 0.001f), v2(4), make_color(1), game->sheet, index_arr[index], c_sprite_size);
					} break;
					invalid_default_case;
				}

				if(ve->timer > duration) {
					game->visual_effect_arr.remove_and_swap(ve_i--);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		visual effects end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			g_r->end_render_pass(g_r, {.do_depth = true, .do_cull = true, .view_projection = view_projection, .framebuffer = game->main_fbo});


			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		hints start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(game->curr_map == 0) {

				start_render_pass(g_r);

				float z = c_player_z;

				draw_text_3d(g_r, strlit("What if you shot a rocket and\njumped at the same time?"), v3(300, 2015, z), 0.5f, make_color(1), false, game->font);

				draw_text_3d(g_r, strlit("Hold\nShoot!"), v3(286, 2003, z), 0.5f, make_color(1), false, game->font);

				draw_text_3d(g_r, strlit("Hold\nShoot!"), v3(246, 1943, z), 0.5f, make_color(1), false, game->font);
				draw_text_3d(g_r, strlit("It's pogo time!"), v3(250, 1943, z), 0.5f, make_color(1), false, game->font);

				draw_text_3d(g_r, strlit("You can jump mid-air!"), v3(289, 1944, z), 0.5f, make_color(1), false, game->font);

				char* text = "If only there was\n"
					"a way to get hit by 2\n"
					"rockets at the same time...\n\n"
					"I bet that would send\n"
					"you very high up";
				draw_text_3d(g_r, strlit(text), v3(223, 1930, z), 0.5f, make_color(1), false, game->font);

				g_r->end_render_pass(g_r, {.blend_mode = e_blend_mode_normal, .view_projection = view_projection, .framebuffer = game->main_fbo});

			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		hints end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			start_render_pass(g_r);
			{
				s_time_data data = process_time(game->timer);
				s_len_str text = format_text("%02i:%02i.%03i", data.minutes, data.seconds, data.ms);
				draw_text(g_r, text, v2(0), 10, 32, make_color(1), false, game->font);
			}

			#ifdef m_debug
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw coords start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				draw_text(g_r, format_text("x: %0.1f y: %0.1f", game->player.pos.x, game->player.pos.y), v2(0, 32), 10, 32, make_color(1), false, game->font);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw coords end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			#endif // m_debug

			g_r->end_render_pass(g_r, {.blend_mode = e_blend_mode_normal, .view_projection = ortho, .framebuffer = game->main_fbo});


			start_render_pass(g_r);
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* player = &game->player;

				if(player->last_x_vel > 0) {
					player->flip_x = false;
					player->state = 1;
				}
				else if(player->last_x_vel < 0) {
					player->flip_x = true;
					player->state = 1;
				}
				else {
					player->state = 0;
				}
				if(player->vel.y > 0.07f) {
					player->state = 5;
				}

				if(player->jump_time > 0.0f) {
					player->state = 3;
				}
				add_clamp_min(&player->jump_time, -g_delta, 0);

				player->animation_timer += g_delta;
				s_v2 pos = lerp(player->prev_pos, player->pos, interp_dt);
				s_draw_data draw_data = get_player_draw_data(*player);
				draw_atlas_3d(g_r, v3(pos, c_player_z), c_player_visual_size, make_color(0.9f), draw_data.texture, draw_data.index, draw_data.sprite_size, {.flip_x = player->flip_x});
				// s_texture texture = get_player_sprite_index(*player);
				// draw_texture_3d(g_r, v3(pos, c_player_z), c_player_visual_size, make_color(0.9f), texture, {.flip_x = player->flip_x});

				if(game->dev_menu.show_hitboxes) {
					draw_rect_3d(g_r, v3(pos, c_player_z - c_small), c_player_collision_size, make_color(1), {}, {.effect_id = 8});
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			g_r->end_render_pass(
				g_r, {.do_depth = true, .blend_mode = e_blend_mode_normal, .cam_pos = game->cam.pos, .view_projection = view_projection, .framebuffer = game->bloom_fbo}
			);

			start_render_pass(g_r);
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		trail start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				foreach_ptr(trail_i, trail, game->trail_arr) {
					constexpr float duration = 0.25f;
					s_percent_data p = get_percent_data(trail->time, duration);
					s_v4 color = make_color(0.9f);
					color.w = p.percent_inv * 0.2f;
					draw_atlas_3d(g_r, v3(trail->pos, c_player_z), c_player_visual_size, color, trail->draw_data.texture, trail->draw_data.index, trail->draw_data.sprite_size, {.flip_x = trail->flip_x});
					// draw_texture_3d(g_r, v3(trail->pos, c_player_z), c_player_visual_size, make_color(0.9f), trail->texture, {.flip_x = trail->flip_x});
					trail->time += g_delta;
					if(trail->time >= duration) {
						game->trail_arr.remove_and_swap(trail_i--);
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		trail end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			g_r->end_render_pass(
				g_r, {.blend_mode = e_blend_mode_normal, .cam_pos = game->cam.pos, .view_projection = view_projection, .framebuffer = game->bloom_fbo}
			);

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		particles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				start_render_pass(g_r);

				// if(is_key_down(g_input, c_left_mouse)) {
				// if(is_key_pressed(g_input, c_right_mouse)) {
					// do_particles(10, v3(game->player.pos, c_particle_z), {
					// 	.shrink = 0.5f,
					// 	.slowdown = 1.0f,
					// 	.duration = 2.5f,
					// 	.duration_rand = 1,
					// 	.speed = 0.5f,
					// 	.speed_rand = 1,
					// 	.angle_rand = 1,
					// 	.radius = 0.1f,
					// 	.radius_rand = 0,
					// 	.color = v3(0.25f),
					// 	.color_rand = v3(1, 1, 0),
					// });
				// }

				foreach_ptr(particle_i, p, game->particle_arr) {
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
					draw_rect_3d(g_r, p->pos, v2(radius * 2.0f), color, {}, {.flags = e_render_flag_circle});
					p->timer += (float)platform_data->frame_time;
					if(percent_done >= 1) {
						game->particle_arr.remove_and_swap(particle_i--);
					}
				}

				g_r->end_render_pass(g_r, {.do_depth = true, .dont_write_depth = true, .blend_mode = e_blend_mode_additive, .cam_pos = game->cam.pos, .view_projection = view_projection, .framebuffer = game->bloom_fbo});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		particles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



		} break;

		case e_state_editor: {
			#ifdef m_debug

			start_render_pass(g_r);
			s_editor* editor = &game->editor;
			s_v2i mouse_index = pos_to_index(game->editor_cam.screen_to_world(g_mouse), c_editor_tile_size);

			if(is_key_pressed(g_input, c_key_f2) && is_key_down(g_input, c_key_left_shift) && is_key_down(g_input, c_key_left_ctrl)) {
				game->map.save_point_arr.count = 0;
				game->map.jump_refresher_arr.count = 0;
				game->map.tile_arr.clear();
			}

			if(is_key_pressed(g_input, c_key_s) && is_key_down(g_input, c_key_left_ctrl)) {
				u8* data = (u8*)la_get(platform_data->frame_arena, sizeof(game->map) + sizeof(int));
				u8* cursor = data;
				s_map_header header = {};
				header.map_version = c_map_version;
				header.end_point_version = c_end_point_version;
				header.tile_version = c_tile_version;
				header.save_point_version = c_save_point_version;
				header.jump_refresher_version = c_jump_refresher_version;
				header.save_point_count = game->map.save_point_arr.count;
				header.jump_refresher_count = game->map.jump_refresher_arr.count;
				cursor = buffer_write(cursor, header);
				cursor = buffer_write(cursor, game->map.end_point);
				cursor = buffer_write2(cursor, game->map.tile_arr.elements, sizeof(game->map.tile_arr.elements));
				cursor = buffer_write2(cursor, game->map.save_point_arr.elements, sizeof(game->map.save_point_arr.elements));
				cursor = buffer_write2(cursor, game->map.jump_refresher_arr.elements, sizeof(game->map.jump_refresher_arr.elements));
				platform_data->write_file(c_map_data[game->curr_map].path, data, (u64)(cursor - data));
			}

			if(is_key_pressed(g_input, c_key_l) && is_key_down(g_input, c_key_left_ctrl)) {
				load_map(&game->map, game->curr_map, platform_data);
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		move editor camera start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			s_v2 dir = {};
			float cam_speed = 1024 / game->editor_cam.zoom;
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
			game->editor_cam.pos += dir * g_delta * cam_speed;
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		move editor camera end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			for(int i = 0; i < c_things_to_place_count; i++) {
				if(is_key_pressed(g_input, c_key_1 + i)) {
					game->editor.curr_tile = (e_tile)(i + 1);
					editor->state = e_editor_state_place;
				}
			}

			if(is_key_pressed(g_input, c_key_v) && is_key_down(g_input, c_key_left_ctrl)) {
				if(editor->copied_tile_arr.count > 0) {
					editor->state = e_editor_state_place;
					editor->curr_tile = c_things_to_place_count + 1;
				}
			}

			switch(editor->state) {
				case e_editor_state_select: {
					s_editor_select_state* state = &editor->select_state;
					if(is_key_pressed(g_input, c_left_mouse) && !state->selecting) {
						state->selection_start = mouse_index;
						state->selecting = true;
					}

					s_v2i selection_start = mouse_index;
					s_v2i selection_end = mouse_index;
					if(state->selecting) {
						selection_start.x = min(state->selection_start.x, selection_start.x);
						selection_start.y = min(state->selection_start.y, selection_start.y);
						selection_end.x = max(state->selection_start.x, mouse_index.x);
						selection_end.y = max(state->selection_start.y, mouse_index.y);
					}
					selection_start.x = clamp(selection_start.x, 0, c_tiles_right - 1);
					selection_start.y = clamp(selection_start.y, 0, c_tiles_down - 1);
					selection_end.x = clamp(selection_end.x, 0, c_tiles_right - 1);
					selection_end.y = clamp(selection_end.y, 0, c_tiles_down - 1);

					if(is_key_down(g_input, c_key_escape)) {
						editor->selected_tile_arr.clear();
					}

					if(state->selecting && !is_key_down(g_input, c_left_mouse)) {
						state->selecting = false;

						editor->selected_tile_arr.clear();
						for(int y = selection_start.y; y <= selection_end.y; y++) {
							for(int x = selection_start.x; x <= selection_end.x; x++) {
								s8 tile = game->map.tile_arr[y][x];
								if(tile <= e_tile_invalid) { continue; }
								editor->selected_tile_arr[y][x] = true;
							}
						}
					}

					if(is_key_pressed(g_input, c_key_c) && is_key_down(g_input, c_key_left_ctrl)) {
						b8 copied_something = false;
						for(int y = 0; y < c_tiles_down; y++) {
							for(int x = 0; x < c_tiles_right; x++) {
								if(!editor->selected_tile_arr[y][x]) { continue; }
								s8 tile = game->map.tile_arr[y][x];
								assert(tile > e_tile_invalid);
								if(!copied_something) {
									editor->copied_tile_arr.count = 0;
									copied_something = true;
								}
								s_copied_tile copied_tile = {};
								copied_tile.tile = tile;
								copied_tile.index = v2i(x, y);
								editor->copied_tile_arr.add(copied_tile);
							}
						}
						if(copied_something) {
							editor->state = e_editor_state_place;
							editor->curr_tile = c_things_to_place_count + 1;
						}
					}

					s_v2 selection_size = v2(selection_end - selection_start);
					selection_size.x += 1;
					selection_size.y += 1;
					selection_size *= c_editor_tile_size;
					draw_empty_rect(g_r, v2(selection_start) * c_editor_tile_size + selection_size * 0.5f, 15, selection_size, 4, make_color(1));

				} break;
				case e_editor_state_place: {

					if(is_key_pressed(g_input, c_key_escape)) {
						editor->state = e_editor_state_select;
					}

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		add tile start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					if(game->editor.curr_tile > e_tile_invalid && game->editor.curr_tile < e_tile_count) {
						if(is_key_down(g_input, c_left_mouse)) {
							if(is_index_valid(mouse_index)) {
								game->map.tile_arr[mouse_index.y][mouse_index.x] = (e_tile)game->editor.curr_tile;
							}
						}
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		add tile end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


					b8 are_we_pasting = editor->curr_tile == c_things_to_place_count + 1;
					s_v2i paste_offset = v2i(99999, 99999);
					foreach_val(copied_tile_i, copied_tile, editor->copied_tile_arr) {
						paste_offset.x = min(copied_tile.index.x, paste_offset.x);
						paste_offset.y = min(copied_tile.index.y, paste_offset.y);
					}

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw copied tiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					if(are_we_pasting) {
						foreach_val(copied_tile_i, copied_tile, editor->copied_tile_arr) {
							assert(copied_tile.tile > e_tile_invalid);
							s_v2i index = mouse_index;
							s_v2i diff = copied_tile.index - paste_offset;
							index += diff;
							s_v2 pos = v2(index) * c_editor_tile_size;
							s_v4 color = make_color(0.1f, 1.0f, 0.1f);
							if(!is_index_valid(index)) {
								color = make_color(1.0f, 0.1f, 0.1f);
							}
							draw_texture(g_r, pos, 2, v2(c_editor_tile_size), color, game->tile_texture_arr[copied_tile.tile], {}, {.origin_offset = c_origin_topleft});
						}
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw copied tiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		add jump refresher start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					if(game->editor.curr_tile == e_tile_count) {
						if(is_key_pressed(g_input, c_left_mouse)) {
							if(is_index_valid(mouse_index)) {
								b8 duplicate = false;
								foreach_val(jump_refresher_i, jump_refresher, game->map.jump_refresher_arr) {
									if(jump_refresher.pos == mouse_index) {
										duplicate = true;
										break;
									}
								}
								if(!duplicate) {
									s_jump_refresher jr = {};
									jr.pos = mouse_index;
									game->map.jump_refresher_arr.add_checked(jr);
								}
							}
						}
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		add jump refresher end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		add save point start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					if(game->editor.curr_tile == e_tile_count + 1) {
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
					else if(game->editor.curr_tile == e_tile_count + 2) {
						if(is_key_pressed(g_input, c_left_mouse)) {
							if(is_index_valid(mouse_index)) {
								game->map.end_point.pos = mouse_index;
							}
						}
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		add end point end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		paste copied tiles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					else if(are_we_pasting) {
						if(is_key_pressed(g_input, c_left_mouse)) {
							if(is_index_valid(mouse_index)) {
								foreach_val(copied_tile_i, copied_tile, editor->copied_tile_arr) {
									assert(copied_tile.tile > e_tile_invalid);
									s_v2i index = mouse_index;
									s_v2i diff = copied_tile.index - paste_offset;
									index += diff;
									s_v2 pos = v2(index) * c_editor_tile_size;
									s_v4 color = make_color(0.1f, 1.0f, 0.1f);
									if(is_index_valid(index)) {
										game->map.tile_arr[index.y][index.x] = (e_tile)copied_tile.tile;
									}
								}
							}
						}
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		paste copied tiles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				} break;
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		delete start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				b8 deleted_non_tile = false;
				if(is_key_down(g_input, c_right_mouse)) {
					foreach_val(save_point_i, save_point, game->map.save_point_arr) {
						if(mouse_index == save_point.pos) {
							deleted_non_tile = true;
							game->map.save_point_arr.remove_and_swap(save_point_i--);
						}
					}
					foreach_val(jump_refresher_i, jump_refresher, game->map.jump_refresher_arr) {
						if(mouse_index == jump_refresher.pos) {
							deleted_non_tile = true;
							game->map.jump_refresher_arr.remove_and_swap(jump_refresher_i--);
						}
					}
					if(!deleted_non_tile && is_index_valid(mouse_index)) {
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
						draw_texture(g_r, pos, 1, v2(c_editor_tile_size), make_color(1), game->tile_texture_arr[tile], {}, {.origin_offset = c_origin_topleft});
						if(game->editor.selected_tile_arr[y][x]) {
							draw_empty_rect(g_r, pos + v2(c_editor_tile_size) * 0.5f, 2, v2(c_editor_tile_size * sin2(-game->render_time * 8.0f)), 4, make_color(1));
						}
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw tiles 2d end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw jump refreshers start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(jump_refresher_i, jump_refresher, game->map.jump_refresher_arr) {
				s_v2 pos = v2(jump_refresher.pos) * v2(c_editor_tile_size);
				draw_atlas(g_r, pos, 2, v2(c_editor_tile_size), make_color(1), game->sheet, v2i(64, 128), c_sprite_size, {}, {.origin_offset = c_origin_topleft});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw jump refreshers end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw save points start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			foreach_val(save_point_i, save_point, game->map.save_point_arr) {
				s_v2 pos = v2(save_point.pos) * v2(c_editor_tile_size);
				draw_texture(g_r, pos, 2, v2(c_editor_tile_size), make_color(1), game->save_point_texture, {}, {.origin_offset = c_origin_topleft});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw save points end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw end point start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_v2 pos = v2(game->map.end_point.pos) * v2(c_editor_tile_size);
				pos = game->editor_cam.world_to_screen(pos);
				draw_atlas(g_r, pos, 2, v2(c_editor_tile_size), make_color(1), game->sheet, v2i(0, 64), c_sprite_size, {}, {.origin_offset = c_origin_topleft});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw end point end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			g_r->end_render_pass(g_r, {.do_depth = true, .view_projection = m4_multiply(ortho, game->editor_cam.get_matrix()), .framebuffer = game->main_fbo});

			{
				start_render_pass(g_r);

				if(is_key_pressed(g_input, c_key_c) && !is_key_down(g_input, c_key_left_ctrl)) {
					game->editor_cam.pos = game->player.pos * c_editor_tile_size - c_base_res * 0.5f;
				}

				constexpr float font_size = 24;
				s_v2 pos = v2(4, 4);
				draw_text(g_r, format_text("x: %i y: %i", mouse_index.x, mouse_index.y), pos, 10, font_size, make_color(1), false, game->font);
				pos.y += font_size;
				draw_text(g_r, strlit("Press C to move camera to player"), pos, 10, font_size, make_color(1), false, game->font);
				g_r->end_render_pass(g_r, {.blend_mode = e_blend_mode_normal, .view_projection = ortho, .framebuffer = game->main_fbo});
			}

			#endif // m_debug

		} break;

		case e_state_leaderboard: {

			start_render_pass(g_r);

			if(is_key_pressed(g_input, c_key_r)) {
				set_state(e_state_play);
			}

			if(!game->leaderboard_state.received) {
				draw_text(g_r, strlit("Getting leaderboard..."), c_half_res, 10, 48, make_color(0.66f), true, game->font);
			}
			else if(game->leaderboard_arr.count <= 0) {
				draw_text(g_r, strlit("No one has beaten this map :("), c_half_res, 10, 48, make_color(0.66f), true, game->font);
			}

			if(game->leaderboard_state.coming_from_win) {
				s_time_data data = process_time((int)round(game->timer * 1000.0) / 1000.0);
				s_len_str text = format_text("%02i:%02i.%03i", data.minutes, data.seconds, data.ms);
				draw_text(g_r, text, c_half_res * v2(1.0f, 0.2f), 10, 64, make_color(1), true, game->font);

				draw_text(g_r, strlit("Press R to restart"), c_half_res * v2(1.0f, 0.4f), 10, sin_range(48, 60, game->render_time * 8.0f), make_color(0.66f), true, game->font);
			}

			constexpr int c_max_visible_entries = 10;
			s_v2 pos = c_half_res * v2(1.0f, 0.7f);
			for(int entry_i = 0; entry_i < at_most(c_max_visible_entries + 1, game->leaderboard_arr.count); entry_i++) {
				s_leaderboard_entry entry = game->leaderboard_arr[entry_i];
				f64 time = entry.time / 1000.0;
				s_time_data data = process_time(time);
				s_v4 color = make_color(0.8f);
				int rank_number = entry_i + 1;
				if(entry_i == c_max_visible_entries || strcmp(platform_data->leaderboard_public_uid.data, entry.internal_name.data) == 0) {
					color = rgb(0xD3A861);
					rank_number = entry.rank;
				}
				char* name = entry.internal_name.data;
				if(entry.nice_name.len > 0) {
					name = entry.nice_name.data;
				}
				draw_text(g_r, format_text("%i %s", rank_number, name), v2(c_base_res.x * 0.1f, pos.y - 24), 10, 32, color, false, game->font);
				s_len_str text = format_text("%02i:%02i.%03i", data.minutes, data.seconds, data.ms);
				draw_text(g_r, text, v2(c_base_res.x * 0.5f, pos.y - 24), 10, 32, color, false, game->font);
				pos.y += 48;
			}

			ui_start(99);
			if(ui_button(strlit("Back"), c_base_res * v2(0.85f, 0.92f), {.font_size = 32, .size_x = 90}) || is_key_pressed(g_input, c_key_escape)) {
				set_state(e_state_map_select);
			}
			ui_end();

			g_r->end_render_pass(g_r, {.blend_mode = e_blend_mode_normal, .view_projection = ortho, .framebuffer = game->main_fbo});

		} break;

		case e_state_input_name: {
			start_render_pass(g_r);

			s_input_name_state* state = &game->input_name_state;

			float font_size = 36;

			s_v2 pos = c_base_res * v2(0.5f, 0.4f);

			b8 submitted = handle_string_input(&state->name, g_input, game->render_time);
			if(submitted) {
				b8 can_submit = true;
				if(state->name.str.len < 2) {
					can_submit = false;
					state->error_str.from_cstr("Name must have at least 2 characters!");
				}
				if(can_submit) {
					state->error_str.len = 0;
					platform_data->set_leaderboard_name(strlit(state->name.str.data), on_set_leaderboard_name);
					platform_data->leaderboard_nice_name = state->name.str;
				}
			}

			draw_text(g_r, strlit("Enter your name"), c_base_res * v2(0.5f, 0.2f), 10, font_size, make_color(1), true, game->font);
			if(state->error_str.len > 0) {
				draw_text(g_r, strlit(state->error_str.data), c_base_res * v2(0.5f, 0.3f), 10, font_size, rgb(0xD77870), true, game->font);
			}

			if(state->name.str.len > 0) {
				draw_text(g_r, strlit(state->name.str.data), pos, 10, font_size, make_color(1), true, game->font);
			}

			s_v2 full_text_size = get_text_size(strlit(state->name.str.data), game->font, font_size);
			s_v2 partial_text_size = get_text_size_with_count(strlit(state->name.str.data), game->font, font_size, state->name.cursor.value);
			s_v2 cursor_pos = v2(
				-full_text_size.x * 0.5f + pos.x + partial_text_size.x,
				pos.y - font_size * 0.5f
			);

			s_v2 cursor_size = v2(15.0f, font_size);
			float t = game->render_time - max(state->name.last_action_time, state->name.last_edit_time);
			b8 blink = false;
			constexpr float c_blink_rate = 0.75f;
			if(t > 0.75f && fmodf(t, c_blink_rate) >= c_blink_rate / 2) {
				blink = true;
			}
			float t2 = clamp(game->render_time - state->name.last_edit_time, 0.0f, 1.0f);
			s_v4 color = lerp(rgb(0xffdddd), brighter(rgb(0xABC28F), 0.8f), 1 - powf(1 - t2, 3));
			float extra_height = ease_out_elastic2_advanced(t2, 0, 1, 20, 0, 0.75f);
			cursor_size.y += extra_height;

			if(!state->name.visual_pos_initialized) {
				state->name.visual_pos_initialized = true;
				state->name.cursor_visual_pos = cursor_pos;
			}
			else {
				state->name.cursor_visual_pos = lerp_snap(state->name.cursor_visual_pos, cursor_pos, g_delta * 20);
			}

			if(!blink) {
				draw_rect(g_r, state->name.cursor_visual_pos - v2(0.0f, extra_height / 2), 15, cursor_size, color, {}, {.origin_offset = c_origin_topleft});
			}

			g_r->end_render_pass(g_r, {.blend_mode = e_blend_mode_normal, .view_projection = ortho, .framebuffer = game->main_fbo});
		} break;

		invalid_default_case;
	}

	#ifdef m_debug
	if(is_key_pressed(g_input, c_key_f5)) {
		game->dev_menu.active = !game->dev_menu.active;
	}
	#endif // m_debug

	do_ui(ortho);

	// @Note(tkap, 08/06/2024): Luminance
	if(!game->disable_bloom) {
		start_render_pass(g_r);
		draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->bloom_fbo, {.shader = 3});
		g_r->end_render_pass(g_r, {.view_projection = ortho, .framebuffer = game->fbo_arr[0]});

		// @Note(tkap, 08/06/2024): Blur
		start_render_pass(g_r);
		draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->fbo_arr[0], {.shader = 4});
		g_r->end_render_pass(g_r, {.view_projection = ortho, .framebuffer = game->fbo_arr[1]});

		// @Note(tkap, 08/06/2024): Combine
		start_render_pass(g_r);
		draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->fbo_arr[1]);
		g_r->end_render_pass(g_r, {.blend_mode = e_blend_mode_additive, .view_projection = ortho, .framebuffer = game->bloom_fbo});
	}

	start_render_pass(g_r);
	draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->bloom_fbo);
	g_r->end_render_pass(g_r, {.blend_mode = e_blend_mode_additive, .view_projection = ortho, .framebuffer = game->main_fbo});

	// start_render_pass(g_r);
	// draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->fbo_arr[0]);
	// g_r->end_render_pass(g_r, {.view_projection = ortho})

	start_render_pass(g_r);
	draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->main_fbo);
	g_r->end_render_pass(g_r, {.do_clear = true, .view_projection = ortho});

}

#ifdef m_build_dll
}
#endif // m_build_dll

static void do_ui(s_m4 ortho)
{
	start_render_pass(g_r);
	if(game->dev_menu.active) {
		ui_start(game->dev_menu.selected_ui);
		ui_bool_button(strlit("Show hitboxes"), v2(4, 4), &game->dev_menu.show_hitboxes);
		ui_bool_button(strlit("test"), v2(4, 64), &game->dev_menu.show_hitboxes);
		game->dev_menu.selected_ui = ui_end();
	}

	g_r->end_render_pass(
		g_r, {.blend_mode = e_blend_mode_normal, .view_projection = ortho, .framebuffer = game->main_fbo}
	);
}


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

static void load_map(s_map* map, int index, s_platform_data* platform_data)
{
	map->end_point.pos = v2i(99, 99);
	u8* data = (u8*)platform_data->read_file(c_map_data[index].path, platform_data->frame_arena, NULL);
	u8* cursor = data;
	if(data) {

		s_map_header header = buffer_read<s_map_header>(&cursor);

		assert(header.map_version == 1);
		assert(header.end_point_version == 1);
		assert(header.tile_version == 1);
		assert(header.save_point_version == 1);
		assert(header.jump_refresher_version == 1);

		game->map.save_point_arr.count = header.save_point_count;
		game->map.jump_refresher_arr.count = header.jump_refresher_count;

		map->end_point = buffer_read<s_end_point>(&cursor);
		cursor = buffer_read2(cursor, game->map.tile_arr.elements, sizeof(game->map.tile_arr.elements));
		cursor = buffer_read2(cursor, game->map.save_point_arr.elements, sizeof(game->map.save_point_arr.elements));
		cursor = buffer_read2(cursor, game->map.jump_refresher_arr.elements, sizeof(game->map.jump_refresher_arr.elements));
	}
}

static s_sarray<s_tile_collision, 16> get_tile_collisions(s_v2 pos, s_v2 size, int tile_size, int tile_blacklist_mask)
{
	s_sarray<s_tile_collision, 16> result;
	s_v2i base_index = pos_to_index(pos, tile_size);
	for(int y = -1; y <= 1; y++) {
		for(int x = -1; x <= 1; x++) {
			s_v2i index = base_index + v2i(x, y);
			if(index_has_tile(index)) {
				e_tile tile = game->map.tile_arr[index.y][index.x];
				if(is_bit_index_set(tile_blacklist_mask, tile)) { continue; }
				s_v2 tile_center = v2(index) * v2(tile_size);
				float temp_tile_size = (float)tile_size;
				if(tile == e_tile_spike) {
					temp_tile_size *= c_spike_collision_size_multiplier;
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

s_m4 s_camera2d::get_matrix()
{
	s_m4 m = m4_scale(v3(zoom, zoom, 1));
	m = m4_multiply(m, m4_translate(v3(-pos.x, -pos.y, 0)));

	return m;
}

static void do_particles(int count, s_v3 pos, s_particle_data data)
{
	s_rng* rng = &game->rng;
	for(int particle_i = 0; particle_i < count; particle_i++) {
		s_particle p = {};
		p.pos = pos;
		p.fade = data.fade;
		p.shrink = data.shrink;
		p.duration = data.duration * (1.0f - rng->randf32() * data.duration_rand);
		// p.dir.xy = v2_from_angle(data.angle + tau * rng->randf32() * data.angle_rand);
		p.dir = v3_normalized(v3(rng->randf32_11(), rng->randf32_11(), rng->randf32_11()));
		p.speed = data.speed * (1.0f - rng->randf32() * data.speed_rand);
		p.radius = data.radius * (1.0f - rng->randf32() * data.radius_rand);
		p.slowdown = data.slowdown;
		p.color = data.color;
		p.color.x *= (1.0f - rng->randf32() * data.color_rand.x);
		p.color.y *= (1.0f - rng->randf32() * data.color_rand.y);
		p.color.z *= (1.0f - rng->randf32() * data.color_rand.z);
		game->particle_arr.add_checked(p);
	}
}

static void on_leaderboard_received(s_json* json)
{
	game->leaderboard_arr.count = 0;
	s_json* temp = json_get(json, "items", e_json_array);
	if(!temp) { goto end; }
	temp = json_get(json, "items", e_json_array);
	for(s_json* j = temp->array; j; j = j->next) {
		if(j->type != e_json_object) { continue; }

		s_leaderboard_entry entry = {};
		s_json* player = json_get(j->object, "player", e_json_object)->object;

		entry.rank = json_get(j->object, "rank", e_json_integer)->integer;

		char* nice_name = json_get(player, "name", e_json_string)->str;
		if(nice_name) {
			entry.nice_name.from_cstr(nice_name);
		}

		char* internal_name = json_get(player, "public_uid", e_json_string)->str;
		entry.internal_name.from_cstr(internal_name);

		entry.time = json_get(j->object, "score", e_json_integer)->integer;
		game->leaderboard_arr.add(entry);
	}
	end:;

	#ifdef m_emscripten
	g_platform_data->get_our_leaderboard(c_map_data[game->curr_map].leaderboard_id, on_our_leaderboard_received);
	#endif // m_emscripten

	game->leaderboard_state.received = true;
}

static void on_our_leaderboard_received(s_json* json)
{
	s_json* j = json->object;
	if(!j) { return; }

	s_leaderboard_entry new_entry = {};
	s_json* player = json_get(j, "player", e_json_object);
	if(!player) { return; }
	player = player->object;

	new_entry.rank = json_get(j, "rank", e_json_integer)->integer;

	char* nice_name = json_get(player, "name", e_json_string)->str;
	if(nice_name) {
		new_entry.nice_name.from_cstr(nice_name);
	}

	char* internal_name = json_get(player, "public_uid", e_json_string)->str;
	new_entry.internal_name.from_cstr(internal_name);

	new_entry.time = json_get(j, "score", e_json_integer)->integer;

	// @Note(tkap, 05/06/2024): We are not in this leaderboard!
	if(new_entry.rank <= 0 || new_entry.time <= 0) {
		return;
	}

	b8 is_already_in_top_ten = false;
	foreach_val(entry_i, entry, game->leaderboard_arr) {
		if(strcmp(internal_name, entry.internal_name.data) == 0) {
			is_already_in_top_ten = true;
			break;
		}
	}

	if(!is_already_in_top_ten) {
		game->leaderboard_arr.add(new_entry);
	}
}

static void on_leaderboard_score_submitted()
{
	g_platform_data->get_leaderboard(c_map_data[game->curr_map].leaderboard_id, on_leaderboard_received);
}

static s_m4 get_camera_view(s_camera3d cam)
{
	return look_at(cam.pos, cam.pos + cam.target, v3(0, -1, 0));
}

static s_draw_data get_player_draw_data(s_player player)
{
	s_draw_data data = {};
	data.sprite_size = v2i(300);

	if(player.state == 0) {
		data.texture = game->player_idle_texture;
		int i = roundfi(fmodf(player.animation_timer / 0.15f, 8));
		data.index = v2i(300 * i, 0);
	}
	else if(player.state == 1) {
		data.texture = game->player_run_texture;
		int i = roundfi(fmodf(player.animation_timer / 0.05f, 5));
		data.index = v2i(300 * i, 0);
	}
	else if(player.state == 3) {
		data.texture = game->player_jump_texture;
		s_percent_data pdata = get_percent_data(player.jump_time, c_player_jump_time);
		int i = roundfi(pdata.percent_inv * 14);
		data.index = v2i(300 * i, 0);
	}
	else if(player.state == 5) {
		data.texture = game->player_fall_texture;
		int i = roundfi(fmodf(player.animation_timer / 0.05f, 5));
		data.index = v2i(300 * i, 0);
	}
	invalid_else;
	return data;
}

static void ui_start(int selected)
{
	s_ui_data data = {};
	data.selected = selected;
	g_ui->data_stack.add(data);
}

static void ui_bool_button(s_len_str id_str, s_v2 pos, b8* ptr)
{
	s_ui_data* data = &g_ui->data_stack.get_last();
	u32 id = hash(id_str.str);

	s_v2 size = v2(320, 48);
	s_ui_element_data* element_data = g_ui->element_data.get(id);
	if(!element_data) {
		s_ui_element_data temp_data = {};
		temp_data.size = size;
		element_data = g_ui->element_data.set(id, temp_data);
	}

	b8 hovered = mouse_collides_rect_topleft(g_mouse, pos, size);

	if(hovered) {
		data->selected = data->element_count;
	}
	b8 selected = data->element_count == data->selected;
	if(selected) {
		element_data->size.x = lerp_snap(element_data->size.x, size.x * 1.2f, g_delta * 10, 0.1f);
	}
	else {
		element_data->size.x = lerp_snap(element_data->size.x, size.x, g_delta * 10, 0.1f);
	}
	s_v4 color;
	if(!*ptr) {
		color = make_color(0.5f, 0.1f, 0.1f);
	}
	else {
		color = make_color(0.1f, 0.5f, 0.1f);
	}

	if(hovered && is_key_pressed(g_input, c_left_mouse)) {
		*ptr = !(*ptr);
	}
	if(selected && is_key_pressed(g_input, c_key_enter)) {
		*ptr = !(*ptr);
	}

	if(selected) {
		color = brighter(color, 1.5f);
	}
	draw_rect(g_r, pos, 0, element_data->size, color, {}, {.origin_offset = c_origin_topleft});

	float font_size = size.y * 0.9f;
	s_v2 text_pos = center_text_on_rect(pos, element_data->size, font_size, false, true);
	text_pos.x += 4;
	draw_text(g_r, id_str, text_pos, 1, font_size, make_color(1), false, game->font);
	data->element_count += 1;
}

static b8 ui_button(s_len_str id_str, s_v2 pos, s_ui_optional optional)
{
	b8 result = false;
	s_ui_data* data = &g_ui->data_stack.get_last();
	s_parse_ui_id parse_result = parse_ui_id(id_str);

	float font_size = 48;
	if(optional.font_size > 0) {
		font_size = optional.font_size;
	}

	s_v2 size = v2(320, 48);
	if(optional.size_x > 0) {
		size.x = optional.size_x;
	}
	if(optional.size_y > 0) {
		size.y = optional.size_y;
	}

	s_ui_element_data* element_data = g_ui->element_data.get(parse_result.id);
	if(!element_data) {
		s_ui_element_data temp_data = {};
		temp_data.size = size;
		element_data = g_ui->element_data.set(parse_result.id, temp_data);
	}

	b8 hovered = mouse_collides_rect_topleft(g_mouse, pos, size);

	if(hovered) {
		data->selected = data->element_count;
	}
	b8 selected = data->element_count == data->selected;
	if(selected) {
		element_data->size.x = lerp_snap(element_data->size.x, size.x * 1.2f, g_delta * 20, 0.1f);
	}
	else {
		element_data->size.x = lerp_snap(element_data->size.x, size.x, g_delta * 20, 0.1f);
	}
	s_v4 color = make_color(0.5f, 0.1f, 0.1f);

	if(hovered && is_key_pressed(g_input, c_left_mouse)) {
		result = true;
	}
	if(selected && is_key_pressed(g_input, c_key_enter)) {
		result = true;
	}

	if(selected) {
		color = brighter(color, 1.5f);
	}
	draw_rect(g_r, pos, 0, element_data->size, color, {}, {.origin_offset = c_origin_topleft});

	s_v2 text_pos = center_text_on_rect(pos, element_data->size, font_size, false, true);
	text_pos.x += 4;
	draw_text(g_r, parse_result.text, text_pos, 1, font_size, make_color(1), false, game->font);
	data->element_count += 1;

	return result;
}

static int ui_end()
{
	s_ui_data data = g_ui->data_stack.pop();

	if(is_key_pressed(g_input, c_key_up)) {
		data.selected = circular_index(data.selected - 1, data.element_count);
	}
	if(is_key_pressed(g_input, c_key_down)) {
		data.selected = circular_index(data.selected + 1, data.element_count);
	}

	return data.selected;
}

static void set_state(e_state state)
{
	game->state = state;
	switch(state) {
		case e_state_map_select: {
			game->map_select_state = {};
		} break;

		case e_state_leaderboard: {
			game->leaderboard_state = {};
			game->leaderboard_arr.count = 0;
		} break;

		case e_state_play: {
			game->reset_game = true;
		} break;

		case e_state_editor: {

		} break;

		case e_state_input_name: {
			game->input_name_state = {};
		} break;

		invalid_default_case;
	}
}

static void on_set_leaderboard_name(b8 success)
{
	if(success) {
		set_state(e_state_leaderboard);
		game->leaderboard_state.coming_from_win = true;
		g_platform_data->submit_leaderboard_score(
			(int)round(game->timer * 1000.0), c_map_data[game->curr_map].leaderboard_id, on_leaderboard_score_submitted
		);
	}
	else {
		game->input_name_state.error_str.from_cstr("Name is already taken!");
	}
}