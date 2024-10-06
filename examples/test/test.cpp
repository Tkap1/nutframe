#define m_game

#include "../../src/platform_shared.h"
#include "variables.h"

#include "test.h"

static constexpr s_v2 c_base_res = {1920, 1080};
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
	platform_data->update_delay = c_update_delay;
}

m_dll_export void update(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{

	game = (s_game*)game_memory;
	g_input = &platform_data->logic_input;
	g_mouse = platform_data->mouse;
	g_r = renderer;
	g_platform_data = platform_data;
	g_ui = &game->ui;

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		initialize start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	if(!game->initialized) {
		game->initialized = true;
		game->rng.seed = platform_data->get_random_seed();
		g_r->set_vsync(true);
		game->sheet = g_r->load_texture(renderer, "examples/speedjam5/sheet.png", e_wrap_clamp);
		game->placeholder_texture = g_r->load_texture(renderer, "examples/test/placeholder.png", e_wrap_repeat);
		game->base_texture = g_r->load_texture(renderer, "examples/test/base.png", e_wrap_clamp);
		game->ant_texture = g_r->load_texture(renderer, "examples/test/ant.png", e_wrap_clamp);

		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone000.png", e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone006.png", e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone012.png", e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone018.png", e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone024.png", e_wrap_clamp));
		game->bot_animation.fps = 12;

		game->creature_death_sound_arr[0] = platform_data->load_sound(platform_data, "examples/test/creature_death00.wav", platform_data->frame_arena);
		game->creature_death_sound_arr[1] = platform_data->load_sound(platform_data, "examples/test/creature_death01.wav", platform_data->frame_arena);
		game->creature_death_sound_arr[2] = platform_data->load_sound(platform_data, "examples/test/creature_death02.wav", platform_data->frame_arena);
		game->buy_bot_sound = platform_data->load_sound(platform_data, "examples/test/buy_bot.wav", platform_data->frame_arena);
		game->upgrade_sound = platform_data->load_sound(platform_data, "examples/test/upgrade.wav", platform_data->frame_arena);

		game->main_fbo = g_r->make_framebuffer(g_r, v2i(c_base_res));
		game->light_fbo = g_r->make_framebuffer_with_existing_depth(g_r, v2i(c_base_res), game->main_fbo->depth);

		game->font = &renderer->fonts[0];
		platform_data->variables_path = "examples/test/variables.h";

		if(g_platform_data->register_leaderboard_client) {
			g_platform_data->register_leaderboard_client();
		}

		for(int i = 0; i < game->world_render_pass_arr.max_elements(); i += 1) {
			game->world_render_pass_arr[i] = make_render_pass(g_r, &platform_data->permanent_arena);
		}
		game->ui_render_pass0 = make_render_pass(g_r, &platform_data->permanent_arena);
		game->ui_render_pass1 = make_render_pass(g_r, &platform_data->permanent_arena);
		game->light_render_pass = make_render_pass(g_r, &platform_data->permanent_arena);
		g_r->default_render_pass = make_render_pass(g_r, &platform_data->permanent_arena);

		g_r->game_speed_index = 5;

		set_state_next_frame(e_state_play);
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		initialize end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		handle state change start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	if(game->next_state >= 0) {
		game->state = (e_state)game->next_state;
		game->next_state = -1;

		switch(game->state) {
			case e_state_play: {
				memset(&game->play_state, 0, sizeof(game->play_state));
				game->play_state.cam.zoom = 1;
				s_v2 pos = c_base_pos + v2(0.0f, c_base_size.y * 0.6f);
				game->play_state.player.pos = pos;
				game->play_state.player.prev_pos = pos;
			} break;
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		handle state change end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	s_bot_arr* bot_arr = &game->play_state.bot_arr;

	game->play_state.player.prev_pos = game->play_state.player.pos;
	memcpy(creature_arr->prev_pos, creature_arr->pos, sizeof(creature_arr->prev_pos));
	memcpy(bot_arr->prev_pos, bot_arr->pos, sizeof(bot_arr->prev_pos));
	switch(game->state) {

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		play state update start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_state_play: {

			s_play_state* state = &game->play_state;
			int alive_creatures = count_alive_creatures();
			if(alive_creatures >= c_num_creatures_to_lose) {
				game->play_state.defeat = true;
			}
			if(game->play_state.defeat) { break; }

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		spawn creatures start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				f64 dt = platform_data->update_delay;
				state->spawn_creature_timer += dt;
				f64 spawn_delay = get_creature_spawn_delay();
				while(state->spawn_creature_timer >= spawn_delay) {

					state->spawn_creature_timer -= spawn_delay;
					s_v2 offset = v2(
						cosf(game->rng.randf_range(0, tau)) * c_base_size.x * 2.0f,
						sinf(game->rng.randf_range(0, tau)) * c_base_size.x * 2.0f
					);
					s_v2 base = game->rng.rand_bool() ? c_base_pos : game->play_state.player.pos;
					s_v2 pos = base + offset;

					int tier = get_creature_spawn_tier();
					if(game->rng.chance100(1)) {
						tier += 2;
					}
					else if(game->rng.chance100(10)) {
						tier += 1;
					}
					make_creature(pos, tier, game->rng.chance100(1));
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		spawn creatures end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* player = &game->play_state.player;
				s_v2 dir = zero;
				if(is_key_down(g_input, c_key_a)) {
					dir.x -= 1;
				}
				if(is_key_down(g_input, c_key_d)) {
					dir.x += 1;
				}
				if(is_key_down(g_input, c_key_w)) {
					dir.y -= 1;
				}
				if(is_key_down(g_input, c_key_s)) {
					dir.y += 1;
				}
				dir = v2_normalized(dir);
				player->pos += dir * get_player_movement_speed();

				player->harvest_timer += 1;
				if(player->harvest_timer >= c_player_harvest_delay) {

					s_get_closest_creature data = get_closest_creature(player->pos);
					int creature = -1;
					if(data.closest_non_targeted_creature.id > 0 && data.smallest_non_targeted_dist <= get_player_harvest_range()) {
						creature = get_creature(data.closest_non_targeted_creature);
						assert(creature >= 0);
						player->target = data.closest_non_targeted_creature;
					}
					else if(data.closest_creature.id > 0 && data.smallest_dist <= get_player_harvest_range()) {
						creature = get_creature(data.closest_creature);
						assert(creature >= 0);
						player->target = data.closest_creature;
					}
					if(creature >= 0) {
						int player_damage = get_player_damage();
						b8 killed = damage_creature(creature, player_damage);
						if(killed) {
							state->resource_count += get_creature_resource_reward(creature_arr->tier[creature], creature_arr->boss[creature]);
						}
						player->harvest_timer = 0;
					}
					else {
						player->target = zero;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update creatures start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			for_creature_partial(creature) {
				if(!creature_arr->active[creature]) { continue; }
				int time_passed = state->update_count - creature_arr->tick_when_last_damaged[creature];
				if(time_passed > 300) {
					creature_arr->roam_timer[creature] += 1;
					if(creature_arr->roam_timer[creature] >= c_creature_roam_delay) {
						creature_arr->roam_timer[creature] -= c_creature_roam_delay;
						float angle_to_base = v2_angle(c_base_pos - creature_arr->pos[creature]);
						angle_to_base += pi * 0.25f;
						s_v2 dir = v2_from_angle(angle_to_base);
						creature_arr->target_pos[creature] = creature_arr->pos[creature] + dir * c_creature_roam_distance;
						creature_arr->flip_x[creature] = dir.x < 0;
					}
				}
				creature_arr->pos[creature] = go_towards(creature_arr->pos[creature], creature_arr->target_pos[creature], c_creature_speed);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update creatures end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update bots start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			for_bot_partial(bot) {
				if(!bot_arr->active[bot]) { continue; }

				bot_arr->laser_target[bot] = zero;
				bot_arr->harvest_timer[bot] += 1;

				switch(bot_arr->state[bot]) {
					case e_bot_state_going_to_creature: {
						int creature = get_creature(bot_arr->target[bot]);
						if(creature >= 0) {
							bot_arr->pos[bot] = go_towards(bot_arr->pos[bot], creature_arr->pos[creature], get_bot_movement_speed());
							float dist = v2_distance(bot_arr->pos[bot], creature_arr->pos[creature]);
							if(dist <= get_bot_harvest_range()) {
								bot_arr->state[bot] = e_bot_state_harvesting_creature;
							}
						}
						else {
							pick_target_for_bot(bot);
						}
					} break;

					case e_bot_state_harvesting_creature: {
						int creature = get_creature(bot_arr->target[bot]);
						if(creature >= 0) {
							bot_arr->laser_target[bot] = bot_arr->target[bot];

							if(bot_arr->harvest_timer[bot] >= c_bot_harvest_delay) {
								bot_arr->harvest_timer[bot] = 0;
								int bot_damage = get_bot_damage();
								b8 killed = damage_creature(creature, bot_damage);
								if(killed) {
									bot_arr->cargo[bot] += get_creature_resource_reward(creature_arr->tier[creature], creature_arr->boss[creature]);
									bot_arr->cargo_count[bot] += 1;
									if(bot_arr->cargo_count[bot] >= get_bot_max_cargo_count()) {
										bot_arr->state[bot] = e_bot_state_going_back_to_base;
									}
									else {
										bot_arr->state[bot] = e_bot_state_going_to_creature;
										pick_target_for_bot(bot);
									}
								}
							}
						}
						else {
							pick_target_for_bot(bot);
							bot_arr->state[bot] = e_bot_state_going_to_creature;
						}
					} break;

					case e_bot_state_going_back_to_base: {
						bot_arr->pos[bot] = go_towards(bot_arr->pos[bot], c_base_pos, get_bot_movement_speed());
						float dist = v2_distance(bot_arr->pos[bot], c_base_pos);
						if(dist < 1) {
							pick_target_for_bot(bot);
							bot_arr->state[bot] = e_bot_state_going_to_creature;
							assert(bot_arr->cargo[bot] > 0);
							state->resource_count += bot_arr->cargo[bot];
							bot_arr->cargo[bot] = 0;
							bot_arr->cargo_count[bot] = 0;
						}
					} break;
				}

			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update bots end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		check win condition start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				#ifdef m_emscripten
				constexpr b8 are_we_on_web = true;
				#else // m_emscripten
				constexpr b8 are_we_on_web = false;
				#endif // m_emscripten

				if(state->resource_count >= c_resource_to_win) {
					if constexpr(are_we_on_web) {
						if(platform_data->leaderboard_nice_name.len > 0) {
							set_state_next_frame(e_state_leaderboard);
							game->leaderboard_state.coming_from_win = true;
							platform_data->submit_leaderboard_score(
								game->play_state.update_count, c_leaderboard_id, on_leaderboard_score_submitted
							);
							game->play_state.update_count_at_win_time = game->play_state.update_count;
						}
						else {
							set_state_next_frame(e_state_input_name);
						}
					}
					else {
						set_state_next_frame(e_state_leaderboard);
						game->leaderboard_state.coming_from_win = true;
						game->play_state.update_count_at_win_time = game->play_state.update_count;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		check win condition end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			state->update_count += 1;

		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		play state update end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	}
}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt)
{
	static_assert(sizeof(s_game) <= c_game_memory);

	g_mouse = platform_data->mouse;

	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = &platform_data->render_input;
	g_platform_data = platform_data;
	g_ui = &game->ui;

	live_variable(&platform_data->vars, c_player_movement_speed, 0.0f, 100.0f, true);
	live_variable(&platform_data->vars, c_creature_roam_delay, 1, 1000, true);
	live_variable(&platform_data->vars, c_creature_roam_distance, 16.0f, 1024.0f, true);
	live_variable(&platform_data->vars, c_bot_movement_speed, 1.0f, 32.0f, true);
	live_variable(&platform_data->vars, c_creature_speed, 0.0f, 100.0f, true);
	live_variable(&platform_data->vars, c_bot_harvest_range, 0.0f, 256.0f, true);
	live_variable(&platform_data->vars, c_player_harvest_range, 0.0f, 256.0f, true);
	live_variable(&platform_data->vars, c_bot_harvest_delay, 0, 100, true);
	live_variable(&platform_data->vars, c_player_harvest_delay, 0, 100, true);

	s_m4 ortho = m4_orthographic(0, c_base_res.x, c_base_res.y, 0, -100, 100);

	g_delta = (float)platform_data->frame_time;
	game->render_time += g_delta;

	s_play_state* play_state = &game->play_state;
	s_camera2d* cam = &play_state->cam;

	if(g_input->wheel_movement > 0) {
		cam->zoom *= 1.1f;
	}
	else if(g_input->wheel_movement < 0) {
		cam->zoom *= 0.9f;
	}
	cam->zoom = clamp(cam->zoom, 0.2f, 5.0f);

	#ifdef m_debug
	if(is_key_pressed(g_input, c_key_add)) {
		circular_index_add(&g_r->game_speed_index, 1, array_count(c_game_speed_arr));
	}
	if(is_key_pressed(g_input, c_key_subtract)) {
		circular_index_add(&g_r->game_speed_index, -1, array_count(c_game_speed_arr));
	}
	if(is_key_pressed(g_input, c_key_f1)) {
		play_state->resource_count = 90000;
	}
	if(is_key_pressed(g_input, c_key_f2)) {
		play_state->resource_count = c_resource_to_win;
	}
	#endif // m_debug

	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	s_bot_arr* bot_arr = &game->play_state.bot_arr;

	{
		s_v2 pos = lerp(game->play_state.player.prev_pos, game->play_state.player.pos, interp_dt);
		game->play_state.cam.pos = pos - c_base_res * (0.5f / game->play_state.cam.zoom);
	}

	switch(game->state) {

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		play state draw start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_state_play: {

			float laser_light_radius = sin_range(120, 128, game->render_time * 32.0f);

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw background start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_v2 min_bounds = cam->pos;
				s_v2 max_bounds = cam->pos + c_base_res / cam->zoom;
				// s_v2 bound_size = max_bounds - min_bounds;
				constexpr float tile_size = 256;
				int start_x = floorfi(min_bounds.x / tile_size);
				int start_y = floorfi(min_bounds.y / tile_size);
				int end_x = ceilfi(max_bounds.x / tile_size);
				int end_y = ceilfi(max_bounds.y / tile_size);
				for(int y = start_y; y <= end_y; y += 1) {
					for(int x = start_x; x <= end_x; x += 1) {
						s_v4 color = brighter(make_color(0.615, 0.481, 0.069), 0.15f);
						if((x + y) & 1) {
							color = brighter(color, 0.9f);
						}
						s_v2 pos = v2(x, y) * tile_size;
						draw_rect(g_r, pos, e_layer_background, v2(tile_size), color, get_render_pass(e_layer_background));
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw background end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw base start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				draw_texture_keep_aspect(g_r, c_base_pos, e_layer_base, c_base_size, make_color(1), game->base_texture, get_render_pass(e_layer_base));
				draw_light(c_base_pos, c_base_size.x, make_color(0.9f), 0.0f);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw base end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player p = game->play_state.player;
				s_v2 pos = lerp(p.prev_pos, p.pos, interp_dt);
				draw_texture(g_r, pos, e_layer_player, c_player_size, make_color(1), game->placeholder_texture, get_render_pass(e_layer_player));

				int creature = get_creature(p.target);
				if(creature >= 0) {
					s_v2 creature_pos = lerp(creature_arr->prev_pos[creature], creature_arr->pos[creature], interp_dt);
					s_v4 laser_color = make_color(1, 0.1f, 0.1f);
					draw_line(g_r, pos, creature_pos, e_layer_laser, c_laser_width, laser_color, get_render_pass(e_layer_laser), {}, {.effect_id = 5});
					draw_light(creature_pos, laser_light_radius * 1.5f, laser_color, 0.0f);
				}
				draw_light(pos, 256, make_color(0.9f), 0.0f);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw creatures start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			for_creature_partial(creature) {
				if(!creature_arr->active[creature]) { continue; }
				s_v2 pos = lerp(creature_arr->prev_pos[creature], creature_arr->pos[creature], interp_dt);
				s_v4 color_arr[] = {
					make_color(1), /*make_color(0.605f, 0.531f, 0.168f),*/ make_color(0.332f, 0.809f, 0.660f), make_color(0.581f, 0.705f, 0.104f), make_color(0.434f, 0.172f, 0.290f),
					make_color(0.473f, 0.549f, 0.744f), make_color(0.276f, 0.523f, 0.052f), make_color(0.076f, 0.185f, 0.868f), make_color(0.525f, 0.820f, 0.016f),
				};
				float size_multi = 1;
				if(creature_arr->boss[creature]) {
					size_multi = 3;
				}

				int color_index = creature_arr->tier[creature] % array_count(color_arr);
				float mix_weight = 1.0f - (play_state->update_count - creature_arr->tick_when_last_damaged[creature]) / 5.0f;
				mix_weight = clamp(mix_weight, 0.0f, 1.0f);
				draw_texture_keep_aspect(g_r, pos, e_layer_creature, c_creature_size * size_multi, color_arr[color_index], game->ant_texture,
				get_render_pass(e_layer_creature), {.flip_x = creature_arr->flip_x[creature]}, {.mix_weight = mix_weight});

				draw_shadow(pos + v2(0, 10), 36 * size_multi, 0.5f, 0.0f);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw creatures end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw bots start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			for_bot_partial(bot) {
				if(!bot_arr->active[bot]) { continue; }
				s_v2 pos = lerp(bot_arr->prev_pos[bot], bot_arr->pos[bot], interp_dt);
				s_v4 color = make_color(1);
				if(bot_arr->state[bot] == e_bot_state_going_back_to_base) {
					color = make_color(0.5f, 0.5f, 0.5f);
				}

				if(bot_arr->state[bot] == e_bot_state_harvesting_creature) {
					bot_arr->tilt_timer[bot] = go_towards(bot_arr->tilt_timer[bot], 0.0f, g_delta);
				}
				else {
					bot_arr->tilt_timer[bot] = sinf(game->render_time * 4);
				}

				bot_arr->animation_timer[bot] += g_delta;
				s_texture texture = get_animation_texture(&game->bot_animation, &bot_arr->animation_timer[bot]);
				float tilt = bot_arr->tilt_timer[bot] * 0.25f;
				draw_texture_keep_aspect(g_r, pos, e_layer_bot, c_bot_size, color, texture, get_render_pass(e_layer_bot), {}, {.rotation = tilt});

				draw_shadow(pos + v2(0, 100), 32, 0.5f, 0.0f);

				int creature = get_creature(bot_arr->laser_target[bot]);
				if(creature >= 0) {
					s_v2 creature_pos = lerp(creature_arr->prev_pos[creature], creature_arr->pos[creature], interp_dt);
					s_v4 laser_color = make_color(0.1f, 1, 0.1f);
					draw_line(g_r, pos, creature_pos, e_layer_laser, c_laser_width, laser_color, get_render_pass(e_layer_laser), {}, {.effect_id = 5});
					draw_light(creature_pos, laser_light_radius, laser_color, 0.0f);
				}
				draw_light(pos, 48, make_color(1.0f), 0.0f);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw bots end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		particles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				// if(is_key_down(g_input, c_left_mouse)) {
				// // if(is_key_pressed(g_input, c_right_mouse)) {
				// 	do_particles(10, g_mouse, e_layer_particle, {
				// 		// .shrink = 0.5f,
				// 		// .slowdown = 1.0f,
				// 		.duration = 0.33f,
				// 		.duration_rand = 1,
				// 		.speed = 128,
				// 		.speed_rand = 1,
				// 		// .angle_rand = 1,
				// 		// .radius = 16.0f,
				// 		// .radius_rand = 0,
				// 		.color = v3(0.2f, 0.1f, 0.1f),
				// 		.color_rand = v3(0.5f, 0.5f, 0.5f),
				// 	});
				// }

				foreach_ptr(particle_i, p, game->play_state.particle_arr) {
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
					draw_circle(g_r, p->pos, p->z, radius, color, get_render_pass(e_layer_particle));
					p->timer += (float)platform_data->frame_time;
					if(percent_done >= 1) {
						game->play_state.particle_arr.remove_and_swap(particle_i--);
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		particles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			if(play_state->defeat) {
				draw_rect(g_r, c_half_res, 0, c_base_res, v4(0.0f, 0.0f, 0.0f, 0.5f), game->ui_render_pass0);
				draw_text(g_r, strlit("You were overwhelmed!"), c_base_res * v2(0.5f, 0.4f), 0, 64, make_color(1), true, game->font, game->ui_render_pass1);
				draw_text(g_r, strlit("Press R to restart..."), c_base_res * v2(0.5f, 0.5f), 0, 64, make_color(0.6f), true, game->font, game->ui_render_pass1);

				if(is_key_pressed(g_input, c_key_r)) {
					set_state_next_frame(e_state_play);
				}
			}
			else {
				draw_text(g_r, format_text("%i", play_state->resource_count), v2(4), 0, 32, make_color(1), false, game->font, game->ui_render_pass1);

				constexpr float font_size = 20;
				constexpr float padding = 8;
				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		upgrade buttons start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				{
					constexpr int c_rows = 3;
					struct s_row
					{
						int upgrade_count;
						s_carray<e_upgrade, e_upgrade_count> upgrade_arr;
					};
					s_carray<s_row, c_rows> row_arr = zero;
					row_arr[0].upgrade_count = 3;
					row_arr[0].upgrade_arr = {{e_upgrade_player_damage, e_upgrade_player_movement_speed, e_upgrade_player_harvest_range}};
					row_arr[1].upgrade_count = 5;
					row_arr[1].upgrade_arr = {{e_upgrade_buy_bot, e_upgrade_bot_damage, e_upgrade_bot_movement_speed, e_upgrade_bot_cargo_count, e_upgrade_bot_harvest_range}};
					row_arr[2].upgrade_count = 3;
					row_arr[2].upgrade_arr = {{e_upgrade_spawn_rate, e_upgrade_creature_tier, e_upgrade_double_harvest}};
					s_v2 panel_pos = v2(0.0f, c_base_res.y - (c_base_button_size.y + padding) * 3);
					s_v2 panel_size = v2(c_base_res.x, c_base_button_size.y + padding);
					s_carray<s_pos_area, c_rows> area_arr;
					area_arr[0] = make_pos_area(panel_pos, panel_size, c_base_button_size, padding, -1, e_pos_area_flag_center_y);
					area_arr[1] = make_pos_area(panel_pos + v2(0.0f, panel_size.y), panel_size, c_base_button_size, padding, -1, e_pos_area_flag_center_y);
					area_arr[2] = make_pos_area(panel_pos + v2(0.0f, panel_size.y * 2), panel_size, c_base_button_size, padding, -1, e_pos_area_flag_center_y);
					b8 played_buy_bot_sound = false;
					b8 played_upgrade_sound = false;
					for(int row_i = 0; row_i < c_rows; row_i += 1) {
						s_row row = row_arr[row_i];
						for(int upgrade_i = 0; upgrade_i < row.upgrade_count; upgrade_i += 1) {
							e_upgrade upgrade_id = row.upgrade_arr[upgrade_i];
							s_upgrade_data data = c_upgrade_data[upgrade_id];
							int curr_level = play_state->upgrade_level_arr[upgrade_id];
							int cost = data.base_cost * (curr_level + 1);
							b8 over_limit = play_state->upgrade_level_arr[upgrade_id] >= data.max_upgrades;
							if(!over_limit && ui_button2(format_text(data.name, cost), pos_area_get_advance(&area_arr[row_i]), {.font_size = font_size})) {
								int buy_count = 1;
								if(is_key_down(g_input, c_key_left_ctrl)) {
									buy_count *= 10;
								}
								if(is_key_down(g_input, c_key_left_shift)) {
									buy_count *= 100;
								}
								for(int buy_i = 0; buy_i < buy_count; buy_i += 1) {
									curr_level = play_state->upgrade_level_arr[upgrade_id];
									b8 purchased = false;
									cost = data.base_cost * (curr_level + 1);
									over_limit = play_state->upgrade_level_arr[upgrade_id] >= data.max_upgrades;
									if(over_limit) { break; }
									if(cost > play_state->resource_count) { break; }

									if(upgrade_id == e_upgrade_buy_bot) {
										int bot = make_bot(c_base_pos);
										if(bot >= 0) {
											purchased = true;
											if(!played_buy_bot_sound) {
												g_platform_data->play_sound(game->buy_bot_sound);
												played_buy_bot_sound = true;
											}
										}
									}
									else {
										purchased = true;
										if(!played_upgrade_sound) {
											g_platform_data->play_sound(game->upgrade_sound);
											played_upgrade_sound = true;
										}
									}
									if(purchased) {
										play_state->upgrade_level_arr[upgrade_id] += 1;
										play_state->resource_count -= cost;
									}
								}
							}
						}
					}
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		upgrade buttons end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				{
					if(ui_button2(strlit("Telport to base"), v2(c_base_res.x - c_base_button_size.x - padding, padding), {.font_size = font_size})) {
						s_v2 pos = c_base_pos + v2(0.0f, c_base_size.y);
						game->play_state.player.pos = pos;
						game->play_state.player.prev_pos = pos;
					}
				}

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		score goal display start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				{
					s_len_str str = format_text("%i / %i", play_state->resource_count, c_resource_to_win);
					s_v2 text_pos = center_text_on_rect(str, game->font, c_base_pos - c_base_size * 0.5f, c_base_size, 40, true, true);
					// text_pos.y += font_size * 0.1f;
					draw_text(g_r, str, text_pos, 0, 40, make_color(1), false, game->font, get_render_pass(e_layer_text));
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		score goal display end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		lose progress start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				{
					draw_rect(g_r, c_base_res * v2(0.33f, 0.025f), 0, v2(c_base_res.x * 0.33f, c_base_res.y * 0.025f), make_color(0.25f, 0.1f, 0.1f), game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});
					float width = count_alive_creatures() / (float)c_num_creatures_to_lose * c_base_res.x * 0.33f;
					draw_rect(g_r, c_base_res * v2(0.33f, 0.025f), 1, v2(width, c_base_res.y * 0.025f), make_color(0.66f, 0.1f, 0.1f), game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		lose progress end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			}

		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		play state draw end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		case e_state_leaderboard: {

			if(is_key_pressed(g_input, c_key_r)) {
				set_state_next_frame(e_state_play);
			}

			if(!game->leaderboard_state.received) {
				draw_text(g_r, strlit("Getting leaderboard..."), c_half_res, 10, 48, make_color(0.66f), true, game->font, game->ui_render_pass1);
			}
			else if(game->leaderboard_arr.count <= 0) {
				draw_text(g_r, strlit("No one scores :("), c_half_res, 10, 48, make_color(0.66f), true, game->font, game->ui_render_pass1);
			}

			if(game->leaderboard_state.coming_from_win) {
				s_time_data data = update_count_to_time_data(game->play_state.update_count_at_win_time, c_update_delay);
				s_len_str text = format_text("%02i:%02i.%03i", data.minutes, data.seconds, data.ms);
				draw_text(g_r, text, c_half_res * v2(1.0f, 0.2f), 10, 64, make_color(1), true, game->font, game->ui_render_pass1);

				draw_text(g_r, strlit("Press R to restart..."), c_half_res * v2(1.0f, 0.4f), 10, sin_range(48, 60, game->render_time * 8.0f), make_color(0.66f), true, game->font, game->ui_render_pass1);
			}

			constexpr int c_max_visible_entries = 10;
			s_v2 pos = c_half_res * v2(1.0f, 0.7f);
			for(int entry_i = 0; entry_i < at_most(c_max_visible_entries + 1, game->leaderboard_arr.count); entry_i++) {
				s_leaderboard_entry entry = game->leaderboard_arr[entry_i];
				s_time_data data = update_count_to_time_data(entry.time, c_update_delay);
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
				draw_text(g_r, format_text("%i %s", rank_number, name), v2(c_base_res.x * 0.1f, pos.y - 24), 10, 32, color, false, game->font, game->ui_render_pass1);
				s_len_str text = format_text("%02i:%02i.%03i", data.minutes, data.seconds, data.ms);
				draw_text(g_r, text, v2(c_base_res.x * 0.5f, pos.y - 24), 10, 32, color, false, game->font, game->ui_render_pass1);
				pos.y += 48;
			}

			ui_start(99);
			if(ui_button(strlit("Restart"), c_base_res * v2(0.85f, 0.92f), {.font_size = 32, .size_x = 120}) || is_key_pressed(g_input, c_key_escape)) {
				set_state_next_frame(e_state_play);
			}
			ui_end();

		} break;

		case e_state_input_name: {

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

			draw_text(g_r, strlit("Enter your name"), c_base_res * v2(0.5f, 0.2f), 10, font_size, make_color(1), true, game->font, game->ui_render_pass1);
			if(state->error_str.len > 0) {
				draw_text(g_r, strlit(state->error_str.data), c_base_res * v2(0.5f, 0.3f), 10, font_size, rgb(0xD77870), true, game->font, game->ui_render_pass1);
			}

			if(state->name.str.len > 0) {
				draw_text(g_r, strlit(state->name.str.data), pos, 10, font_size, make_color(1), true, game->font, game->ui_render_pass1);
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
				draw_rect(g_r, state->name.cursor_visual_pos - v2(0.0f, extra_height / 2), 15, cursor_size, color, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});
			}
		} break;

		invalid_default_case;
	}

	s_m4 view = cam->get_matrix();

	g_r->clear_framebuffer(game->main_fbo, zero, c_default_fbo_clear_flags);
	g_r->clear_framebuffer(game->light_fbo, v4(0.75f, 0.75f, 0.75f, 1.0f), e_fbo_clear_color);

	g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.depth_mode = e_depth_mode_read_and_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[2], game->main_fbo, {.depth_mode = e_depth_mode_read_and_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[3], game->main_fbo, {.depth_mode = e_depth_mode_no_read_yes_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[4], game->main_fbo, {.depth_mode = e_depth_mode_no_read_yes_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[5], game->main_fbo, {.depth_mode = e_depth_mode_no_read_yes_write, .blend_mode = e_blend_mode_additive, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[6], game->main_fbo, {.depth_mode = e_depth_mode_no_read_yes_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		lights start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		// @Note(tkap, 06/10/2024): Shadows
		g_r->end_render_pass(g_r, game->world_render_pass_arr[1], game->light_fbo, {.depth_mode = e_depth_mode_read_no_write, .blend_mode = e_blend_mode_multiply_inv, .view = view, .projection = ortho});

		g_r->end_render_pass(g_r, game->light_render_pass, game->light_fbo, {.blend_mode = e_blend_mode_additive, .view = view, .projection = ortho});
		draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->light_fbo, game->light_render_pass);
		g_r->end_render_pass(g_r, game->light_render_pass, game->main_fbo, {.blend_mode = e_blend_mode_multiply, .projection = ortho});
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		lights end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	g_r->end_render_pass(g_r, game->ui_render_pass0, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});
	g_r->end_render_pass(g_r, game->ui_render_pass1, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});

	g_r->clear_framebuffer(g_r->default_fbo, zero, c_default_fbo_clear_flags);
	draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->main_fbo, game->world_render_pass_arr[0]);
	g_r->end_render_pass(g_r, game->world_render_pass_arr[0], g_r->default_fbo, {.projection = ortho});

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


s_m4 s_camera2d::get_matrix()
{
	s_m4 m = m4_scale(v3(zoom, zoom, 1));
	m = m4_multiply(m, m4_translate(v3(-pos.x, -pos.y, 0)));

	return m;
}

func void do_particles(int count, s_v2 pos, int z, s_particle_data data)
{
	s_rng* rng = &game->rng;
	for(int particle_i = 0; particle_i < count; particle_i++) {
		s_particle p = {};
		p.pos = pos;
		p.z = z;
		p.fade = data.fade;
		p.shrink = data.shrink;
		p.duration = data.duration * (1.0f - rng->randf32() * data.duration_rand);
		// p.dir.xy = v2_from_angle(data.angle + tau * rng->randf32() * data.angle_rand);
		p.dir = v2_normalized(v2(rng->randf32_11(), rng->randf32_11()));
		p.speed = data.speed * (1.0f - rng->randf32() * data.speed_rand);
		p.radius = data.radius * (1.0f - rng->randf32() * data.radius_rand);
		p.slowdown = data.slowdown;
		p.color = data.color;
		p.color.x *= (1.0f - rng->randf32() * data.color_rand.x);
		p.color.y *= (1.0f - rng->randf32() * data.color_rand.y);
		p.color.z *= (1.0f - rng->randf32() * data.color_rand.z);
		game->play_state.particle_arr.add_checked(p);
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
	g_platform_data->get_our_leaderboard(c_leaderboard_id, on_our_leaderboard_received);
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
	g_platform_data->get_leaderboard(c_leaderboard_id, on_leaderboard_received);
}

static s_m4 get_camera_view(s_camera3d cam)
{
	return look_at(cam.pos, cam.pos + cam.target, v3(0, -1, 0));
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
	draw_rect(g_r, pos, 0, element_data->size, color, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});

	float font_size = size.y * 0.9f;
	s_v2 text_pos = center_text_on_rect(id_str, game->font, pos, element_data->size, font_size, false, true);
	text_pos.x += 4;
	draw_text(g_r, id_str, text_pos, 1, font_size, make_color(1), false, game->font, game->ui_render_pass1);
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
	draw_rect(g_r, pos, 0, element_data->size, color, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});

	s_v2 text_pos = center_text_on_rect(id_str, game->font, pos, element_data->size, font_size, false, true);
	text_pos.x += 4;
	draw_text(g_r, parse_result.text, text_pos, 1, font_size, make_color(1), false, game->font, game->ui_render_pass1);
	data->element_count += 1;

	return result;
}

static b8 ui_button2(s_len_str id_str, s_v2 pos, s_ui_optional optional)
{
	b8 result = false;
	s_parse_ui_id parse_result = parse_ui_id(id_str);

	float font_size = 48;
	if(optional.font_size > 0) {
		font_size = optional.font_size;
	}

	s_v2 size = c_base_button_size;
	if(optional.size_x > 0) {
		size.x = optional.size_x;
	}
	if(optional.size_y > 0) {
		size.y = optional.size_y;
	}

	b8 hovered = mouse_collides_rect_topleft(g_mouse, pos, size);
	s_v4 color = make_color(0.5f, 0.1f, 0.1f);

	if(hovered && is_key_pressed(g_input, c_left_mouse)) {
		result = true;
	}

	if(hovered) {
		color = brighter(color, 1.5f);
	}
	draw_rect(g_r, pos, 0, size, color, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});

	s_v2 text_pos = center_text_on_rect(id_str, game->font, pos, size, font_size, true, true);
	text_pos.y += font_size * 0.1f;
	draw_text(g_r, parse_result.text, text_pos, 1, font_size, make_color(1), false, game->font, game->ui_render_pass1);

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

static void on_set_leaderboard_name(b8 success)
{
	if(success) {
		set_state_next_frame(e_state_leaderboard);
		game->leaderboard_state.coming_from_win = true;
		g_platform_data->submit_leaderboard_score(
			game->play_state.update_count, c_leaderboard_id, on_leaderboard_score_submitted
		);
		game->play_state.update_count_at_win_time = game->play_state.update_count;
	}
	else {
		game->input_name_state.error_str.from_cstr("Name is already taken!");
	}
}

func int make_entity(b8* active, int* id, s_entity_index_data* index_data, int max_entities)
{
	for(int i = 0; i < max_entities; i += 1) {
		if(!active[i]) {
			active[i] = true;
			game->play_state.next_entity_id += 1;
			id[i] = game->play_state.next_entity_id;
			index_data->lowest_index = at_most(i, index_data->lowest_index);
			index_data->max_index_plus_one = at_least(i + 1, index_data->max_index_plus_one);
			return i;
		}
	}
	return -1;
}

func int make_creature(s_v2 pos, int tier, b8 boss)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	int entity = make_entity(creature_arr->active, creature_arr->id, &creature_arr->index_data, c_max_creatures);
	creature_arr->pos[entity] = pos;
	creature_arr->prev_pos[entity] = pos;
	creature_arr->target_pos[entity] = pos;
	creature_arr->tier[entity] = tier;
	creature_arr->curr_health[entity] = 20 * (tier + 1);
	creature_arr->roam_timer[entity] = 0;
	creature_arr->targeted[entity] = false;
	creature_arr->boss[entity] = boss;

	if(boss) {
		creature_arr->curr_health[entity] *= 10;
	}
	return entity;
}

func int make_bot(s_v2 pos)
{
	s_bot_arr* bot_arr = &game->play_state.bot_arr;
	int entity = make_entity(bot_arr->active, bot_arr->id, &bot_arr->index_data, c_max_bots);
	bot_arr->target[entity] = zero;
	bot_arr->state[entity] = e_bot_state_going_to_creature;
	bot_arr->pos[entity] = pos;
	bot_arr->prev_pos[entity] = pos;
	bot_arr->laser_target[entity] = zero;
	bot_arr->cargo[entity] = 0;
	bot_arr->cargo_count[entity] = 0;
	return entity;
}


func void pick_target_for_bot(int bot)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	s_bot_arr* bot_arr = &game->play_state.bot_arr;
	assert(bot_arr->active[bot]);

	s_get_closest_creature data = get_closest_creature(c_base_pos);

	if(data.closest_non_targeted_creature.id > 0) {
		bot_arr->target[bot] = data.closest_non_targeted_creature;
		creature_arr->targeted[data.closest_non_targeted_creature.index] = true;
	}
	else {
		bot_arr->target[bot] = data.closest_creature;
		creature_arr->targeted[data.closest_creature.index] = true;
	}
}

func int get_creature(s_entity_index index)
{
	assert(index.index >= 0);
	if(index.id <= 0) { return -1; }
	if(!game->play_state.creature_arr.active[index.index]) { return -1; }

	if(game->play_state.creature_arr.id[index.index] == index.id) { return index.index; }
	return -1;
}

// @Note(tkap, 05/10/2024): Return true if creature died. Return false if creature still alive
func b8 damage_creature(int creature, int damage)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	creature_arr->curr_health[creature] -= damage;
	creature_arr->tick_when_last_damaged[creature] = game->play_state.update_count;
	if(creature_arr->curr_health[creature] <= 0) {
		remove_entity(creature, creature_arr->active, &creature_arr->index_data);
		g_platform_data->play_sound(game->creature_death_sound_arr.get_random(&game->rng));
		do_particles(32, creature_arr->pos[creature], e_layer_particle, {
			.duration = 0.33f,
			.duration_rand = 1,
			.speed = 128,
			.speed_rand = 1,
			.color = v3(0.2f, 0.1f, 0.1f),
			.color_rand = v3(0.5f, 0.5f, 0.5f),
		});
		return true;
	}
	return false;
}

func void remove_entity(int entity, b8* active, s_entity_index_data* index_data)
{
	active[entity] = false;
	assert(entity >= index_data->lowest_index);
	if(entity == index_data->lowest_index) {
		index_data->lowest_index = at_least(0, entity - 1);
	}
	if(entity + 1 == index_data->max_index_plus_one) {
		index_data->max_index_plus_one = at_least(0, entity);
	}
}

func s_get_closest_creature get_closest_creature(s_v2 pos)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;

	s_get_closest_creature data = zero;

	for_creature_partial(creature) {
		if(!creature_arr->active[creature]) { continue; }
		float dist = v2_distance(creature_arr->pos[creature], pos);
		if(creature_arr->targeted[creature]) {
			if(dist < data.smallest_dist) {
				data.smallest_dist = dist;
				data.closest_creature.index = creature;
				data.closest_creature.id = creature_arr->id[creature];
			}
		}
		else {
			if(dist < data.smallest_non_targeted_dist) {
				data.smallest_non_targeted_dist = dist;
				data.closest_non_targeted_creature.index = creature;
				data.closest_non_targeted_creature.id = creature_arr->id[creature];
			}
		}
	}
	return data;
}

func s_v2 get_center(s_v2 pos, s_v2 size)
{
	pos.x += size.x * 0.5f;
	pos.y += size.y * 0.5f;
	return pos;
}

func s_pos_area make_pos_area(s_v2 pos, s_v2 size, s_v2 element_size, float spacing, int count, int flags)
{
	// @Note(tkap, 25/09/2024): -1 means we don't care about the count
	assert(count > 0 || count == -1);
	s_pos_area area = zero;
	s_v2 center_pos = get_center(pos, size);
	s_v2 space_used = v2(0, 0);
	bool center_x = (flags & e_pos_area_flag_center_x) != 0;
	bool center_y = (flags & e_pos_area_flag_center_y) != 0;
	bool horizontal = (flags & e_pos_area_flag_vertical) == 0;

	space_used.x = element_size.x * count;
	space_used.x += spacing * (count - 1);

	space_used.y = element_size.y * count;
	space_used.y += spacing * (count - 1);

	if(center_x) {
		if(horizontal) {
			area.pos.x = center_pos.x - space_used.x * 0.5f;
		}
		else {
			area.pos.x = center_pos.x - element_size.x * 0.5f;
		}
	}
	else {
		area.pos.x = pos.x;
	}

	if(center_y) {
		if(horizontal) {
			area.pos.y = center_pos.y - element_size.y * 0.5f;
		}
		else {
			area.pos.y = center_pos.y - space_used.y * 0.5f;
		}
	}
	else {
		area.pos.y = pos.y;
	}

	if(horizontal) {
		area.advance.x = element_size.x + spacing;
	}
	else {
		area.advance.y = element_size.x + spacing;
	}

	return area;
}

func s_v2 pos_area_get_advance(s_pos_area* area)
{
	s_v2 result = area->pos;
	area->pos.x += area->advance.x;
	area->pos.y += area->advance.y;
	return result;
}

func int get_player_damage()
{
	return 2 + game->play_state.upgrade_level_arr[e_upgrade_player_damage];
}

func int get_bot_damage()
{
	return 1 + game->play_state.upgrade_level_arr[e_upgrade_bot_damage];
}

func float get_player_movement_speed()
{
	return c_player_movement_speed + game->play_state.upgrade_level_arr[e_upgrade_player_movement_speed];
}

func float get_bot_movement_speed()
{
	return c_bot_movement_speed + game->play_state.upgrade_level_arr[e_upgrade_bot_movement_speed];
}

func f64 get_creature_spawn_delay()
{
	int increase0 = game->play_state.upgrade_level_arr[e_upgrade_spawn_rate] * 15;
	f64 increase1 = (2.0 / 10800) * game->play_state.update_count;
	f64 p = c_spawns_per_second * (1.0 + increase0 / 100.0);
	p *= 1.0 + increase1;
	return 1.0 / p;
}

func int get_creature_spawn_tier()
{
	return game->play_state.upgrade_level_arr[e_upgrade_creature_tier];
}

func float get_player_harvest_range()
{
	return c_player_harvest_range + game->play_state.upgrade_level_arr[e_upgrade_player_harvest_range] * 35;
}

func float get_bot_harvest_range()
{
	return c_bot_harvest_range + game->play_state.upgrade_level_arr[e_upgrade_bot_harvest_range] * 20;
}

func int get_creature_resource_reward(int tier, b8 boss)
{
	int result = tier + 1;
	if(game->play_state.upgrade_level_arr[e_upgrade_double_harvest] > 0) {
		result *= 2;
	}
	if(boss) {
		result *= 11;
	}
	return result;
}

func void set_state_next_frame(e_state new_state)
{
	if(game->next_state >= 0) { return; }

	switch(new_state)	{
		case e_state_leaderboard: {
			game->leaderboard_state = zero;
			game->leaderboard_arr.count = 0;
		} break;
	}

	game->next_state = new_state;
}

func int count_alive_creatures()
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	int result = 0;
	for_creature_partial(creature) {
		if(!creature_arr->active[creature]) { continue; }
		result += 1;
	}
	return result;
}

func s_render_pass* get_render_pass(e_layer layer)
{
	return game->world_render_pass_arr[c_layer_to_render_pass_index_arr[layer]];
}

func void draw_light(s_v2 pos, float radius, s_v4 color, float smoothness)
{
	draw_circle(g_r, pos, 0, radius, color, game->light_render_pass, {.shader = 5, .circle_smoothness = smoothness});
}

func void draw_shadow(s_v2 pos, float radius, float strength, float smoothness)
{
	draw_circle(g_r, pos, e_layer_shadow, radius, make_color(strength), game->world_render_pass_arr[1], {.shader = 5, .circle_smoothness = smoothness});
}

func int get_bot_max_cargo_count()
{
	return 1 + game->play_state.upgrade_level_arr[e_upgrade_bot_cargo_count];
}