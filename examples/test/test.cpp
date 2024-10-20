#define m_game

#include "../../src/platform_shared.h"
#include "variables.h"

#include "test.h"

static constexpr s_v2 c_base_res = {1920, 1080};
// static constexpr s_v2 c_base_res = {1366, 768};
static constexpr s_v2 c_half_res = {c_base_res.x * 0.5f, c_base_res.y * 0.5f};
static constexpr s_bounds c_base_res_bounds = rect_to_bounds(v2(0), c_base_res);

#ifdef m_emscripten
global constexpr b8 c_are_we_on_web = true;
#else // m_emscripten
global constexpr b8 c_are_we_on_web = false;
#endif // m_emscripten

static s_input* g_input;
static s_game* game;
static s_game_renderer* g_r;
static s_v2 g_mouse;
static s_platform_data* g_platform_data;
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

m_dll_export void update(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, b8 is_last_update_this_frame)
{

	game = (s_game*)game_memory;
	g_mouse = platform_data->mouse;
	g_r = renderer;
	g_platform_data = platform_data;

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		initialize start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	if(!game->initialized) {
		game->initialized = true;
		game->rng = make_rng(platform_data->get_random_seed());
		g_r->set_vsync(true);
		game->placeholder_texture = g_r->load_texture(renderer, "examples/test/placeholder.png", e_filter_linear, e_wrap_clamp);
		game->base_texture = g_r->load_texture(renderer, "examples/test/base.png", e_filter_linear, e_wrap_clamp);
		game->button_texture = g_r->load_texture(renderer, "examples/test/button.png", e_filter_linear, e_wrap_clamp);
		game->tile_texture = g_r->load_texture(renderer, "examples/test/tile.png", e_filter_linear, e_wrap_clamp);
		game->crater_texture = g_r->load_texture(renderer, "examples/test/crater.png", e_filter_linear, e_wrap_clamp);
		game->rock_texture_arr[0] = g_r->load_texture(renderer, "examples/test/rock01.png", e_filter_linear, e_wrap_clamp);
		game->rock_texture_arr[1] = g_r->load_texture(renderer, "examples/test/rock02.png", e_filter_linear, e_wrap_clamp);
		game->broken_bot_texture = g_r->load_texture(renderer, "examples/test/broken_bot.png", e_filter_linear, e_wrap_clamp);

		game->upgrade_button_texture_arr[e_upgrade_buy_bot] = g_r->load_texture(renderer, "examples/test/drone_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_player_damage] = g_r->load_texture(renderer, "examples/test/damage_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_bot_damage] = g_r->load_texture(renderer, "examples/test/drone_damage_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_player_movement_speed] = g_r->load_texture(renderer, "examples/test/speed_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_bot_movement_speed] = g_r->load_texture(renderer, "examples/test/drone_speed_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_spawn_rate] = g_r->load_texture(renderer, "examples/test/spawn_rate_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_creature_tier] = g_r->load_texture(renderer, "examples/test/creature_tier.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_player_harvest_range] = g_r->load_texture(renderer, "examples/test/player_range_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_bot_harvest_range] = g_r->load_texture(renderer, "examples/test/drone_range_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_double_harvest] = g_r->load_texture(renderer, "examples/test/2x_harvest_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_bot_cargo_count] = g_r->load_texture(renderer, "examples/test/drone_cargo_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_player_chain] = g_r->load_texture(renderer, "examples/test/player_chain_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_broken_bot_spawn] = g_r->load_texture(renderer, "examples/test/broken_drone_icon.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_deposit_spawn_rate] = game->placeholder_texture;
		game->upgrade_button_texture_arr[e_upgrade_deposit_health] = game->placeholder_texture;
		game->upgrade_button_texture_arr[e_upgrade_dash_cooldown] = game->placeholder_texture;

		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone000.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone006.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone012.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone018.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone024.png", e_filter_linear, e_wrap_clamp));
		game->bot_animation.fps = 12;

		add_texture(&game->ant_animation, g_r->load_texture(renderer, "examples/test/ant000.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->ant_animation, g_r->load_texture(renderer, "examples/test/ant006.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->ant_animation, g_r->load_texture(renderer, "examples/test/ant012.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->ant_animation, g_r->load_texture(renderer, "examples/test/ant018.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->ant_animation, g_r->load_texture(renderer, "examples/test/ant024.png", e_filter_linear, e_wrap_clamp));
		game->ant_animation.fps = 8;

		add_texture(&game->player_animation, g_r->load_texture(renderer, "examples/test/player000.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->player_animation, g_r->load_texture(renderer, "examples/test/player006.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->player_animation, g_r->load_texture(renderer, "examples/test/player012.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->player_animation, g_r->load_texture(renderer, "examples/test/player018.png", e_filter_linear, e_wrap_clamp));
		add_texture(&game->player_animation, g_r->load_texture(renderer, "examples/test/player024.png", e_filter_linear, e_wrap_clamp));
		game->player_animation.fps = 12;

		game->sound_arr[e_sound_creature_death00] = platform_data->load_sound(platform_data, "examples/test/creature_death00.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_creature_death01] = platform_data->load_sound(platform_data, "examples/test/creature_death01.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_creature_death02] = platform_data->load_sound(platform_data, "examples/test/creature_death02.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_buy_bot] = platform_data->load_sound(platform_data, "examples/test/buy_bot.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_upgrade] = platform_data->load_sound(platform_data, "examples/test/upgrade.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_level_up] = platform_data->load_sound(platform_data, "examples/test/level_up.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_dash] = platform_data->load_sound(platform_data, "examples/test/dash.wav", platform_data->frame_arena);

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
		game->ui_render_pass2 = make_render_pass(g_r, &platform_data->permanent_arena);
		game->ui_render_pass3 = make_render_pass(g_r, &platform_data->permanent_arena);
		g_r->default_render_pass = make_render_pass(g_r, &platform_data->permanent_arena);

		g_r->game_speed_index = 5;

		game->next_state = -1;
		set_state_next_frame(e_state_main_menu);

		for(int i = 0; i < game->statistics_show_arr.max_elements(); i += 1) {
			game->statistics_show_arr[i] = true;
		}

		register_action(g_platform_data, e_action_left, c_key_a, c_key_left);
		register_action(g_platform_data, e_action_right, c_key_d, c_key_right);
		register_action(g_platform_data, e_action_up, c_key_w, c_key_up);
		register_action(g_platform_data, e_action_down, c_key_s, c_key_down);
		register_action(g_platform_data, e_action_dash, c_key_space, c_right_mouse);

		#if defined(m_debug)
		game->hide_tutorial = true;
		game->pick_free_upgrade_automatically = true;
		#endif
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		initialize end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		handle state change start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	if(game->next_state >= 0) {
		if(game->should_pop_state) {
			while(true) {
				game->state_stack.pop();
				s_state previous_state = game->state_stack.get_last();
				if(!previous_state.is_temporary) { break; }
			}
			game->should_pop_state = false;
		}
		else {
			game->state_stack.add({.is_temporary = game->next_state_is_temporary, .state = (e_state)game->next_state});
		}
		game->next_state = -1;
		game->next_state_is_temporary = false;
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		handle state change end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		reset game start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	if(game->reset_game) {
		game->reset_game = false;

		memset(&game->play_state, 0, sizeof(game->play_state));
		game->play_state.cam.zoom = 1;
		game->play_state.cam.target_zoom = 1;

		game->play_state.spawn_broken_bot_timer = make_auto_timer(0, 10);
		game->play_state.spawn_deposit_timer = make_auto_timer(20, 20);

		game->play_state.player.dash_cooldown_timer = make_timer(c_dash_cooldown, c_dash_cooldown);
		game->play_state.player.wanted_to_dash_timestamp = -100000;

		game->play_state.next_pickup_to_drop = game->rng.randu() % e_pickup_count;

		{
			s_v2 pos = c_base_pos + v2(0.0f, c_base_size.y * 0.6f);
			game->play_state.player.pos = pos;
			game->play_state.player.prev_pos = pos;
			game->play_state.player.dash_dir = v2(1, 0);
			game->play_state.player.curr_level = 1;
			game->play_state.cam.pos = pos;
			game->play_state.cam.target_pos = pos;
		}

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		spawn broken bots start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		{
			s_carray<s_v2, 8> broken_bot_pos_arr = get_broken_bot_pos_arr(&game->rng);
			for(int i = 0; i < broken_bot_pos_arr.max_elements(); i += 1) {
				game->play_state.broken_bot_arr.add({.rotation = game->rng.randf_range(-0.25f, 0.25f), .pos = broken_bot_pos_arr[i]});
			}
		}
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		spawn broken bots end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		setup craters start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		{
			s_rng rng = make_rng(4);
			s_bounds map_bounds = get_map_bounds();
			for(int crater_i = 0; crater_i < c_max_craters; crater_i += 1) {
				game->play_state.crater_pos_arr[crater_i] = v2(
					rng.randf_range(map_bounds.min_x, map_bounds.max_x) + 64,
					rng.randf_range(map_bounds.min_y, map_bounds.max_y) - 64
				);
				game->play_state.crater_size_arr[crater_i] = rng.randf_range(96, 136);
				game->play_state.crater_rotation_arr[crater_i] = rng.randf_range(-pi * 0.1f, pi * 0.1f);
				game->play_state.crater_flip_arr[crater_i] = rng.rand_bool();
			}
		}
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		setup craters end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		reset game end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	s_v2 mouse_world = game->play_state.cam.screen_to_world(g_mouse);

	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	s_bot_arr* bot_arr = &game->play_state.bot_arr;

	game->play_state.player.prev_pos = game->play_state.player.pos;
	memcpy(creature_arr->prev_pos, creature_arr->pos, sizeof(creature_arr->prev_pos));
	memcpy(bot_arr->prev_pos, bot_arr->pos, sizeof(bot_arr->prev_pos));
	switch(get_state()) {

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		play state update start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_state_play: {

			s_play_state* state = &game->play_state;

			{
				int index = state->update_count % c_nectar_gain_num_updates;
				state->nectar_gain_arr[index] = 0;
			}

			if(can_go_to_level_up_state() && state->level_up_triggers > 0) {
				state->sub_state = e_sub_state_level_up;
				state->level_up_seed = g_platform_data->get_random_seed();
				play_sound_group(e_sound_group_level_up);
			}

			if(can_lose()) {
				int alive_creatures = count_alive_creatures();
				if(alive_creatures >= c_num_creatures_to_lose) {
					state->sub_state = e_sub_state_defeat;
				}
			}

			if(game_is_paused()) {
				break;
			}

			{
				int level = state->upgrade_level_arr[e_upgrade_broken_bot_spawn];
				if(level > 0) {
					s_auto_timer* t = &state->spawn_broken_bot_timer;
					t->speed = 1 + ((level - 1) * 25 / 100.0f);
					int to_spawn = t->tick();
					for(int i = 0; i < to_spawn; i += 1) {
						s_carray<s_v2, 8> broken_bot_pos_arr = get_broken_bot_pos_arr(&game->rng);
						int rand_index = game->rng.randu() % broken_bot_pos_arr.max_elements();
						game->play_state.broken_bot_arr.add_checked({.rotation = game->rng.randf_range(-0.25f, 0.25f), .pos = broken_bot_pos_arr[rand_index]});
					}
				}
			}

			s_cells cells = zero;
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		build cells start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				for_creature_partial(creature) {
					if(!creature_arr->active[creature]) { continue; }
					s_v2 pos = creature_arr->pos[creature];
					s_v2i index = get_cell_index(pos);
					if(is_valid_index(index.x, index.y, c_num_cells, c_num_cells)) {
						if(cells.cell_arr[index.y][index.x].max_elements <= 0) {
							cells.cell_arr[index.y][index.x] = make_dynamic_array<int>(16, platform_data->frame_arena);
						}
						cells.cell_arr[index.y][index.x].add(creature, platform_data->frame_arena);
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		build cells end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		spawn creatures start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				f64 dt = platform_data->update_delay;
				f64 spawn_delay = get_creature_spawn_delay();
				if(state->has_player_performed_any_action) {
					state->spawn_creature_timer += dt;
				}
				while(state->spawn_creature_timer >= spawn_delay) {

					state->spawn_creature_timer -= spawn_delay;
					s_v2 pos = get_random_creature_spawn_pos();
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

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		spawn deposits start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_auto_timer* t = &state->spawn_deposit_timer;
				t->speed = get_multiplier(state->upgrade_level_arr[e_upgrade_deposit_spawn_rate], c_deposit_spawn_rate_buff_per_upgrade);
				int count = t->tick();
				for(int i = 0; i < count; i += 1) {

					int tier = get_creature_spawn_tier();
					if(game->rng.chance100(1)) {
						tier += 2;
					}
					else if(game->rng.chance100(10)) {
						tier += 1;
					}
					make_deposit(get_random_creature_spawn_pos(), tier);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		spawn deposits end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* player = &game->play_state.player;
				s_v2 dir = zero;
				if(game->hold_input.left) {
					dir.x -= 1;
					state->has_player_performed_any_action = true;
				}
				if(game->hold_input.right) {
					dir.x += 1;
					state->has_player_performed_any_action = true;
				}
				if(game->hold_input.up) {
					dir.y -= 1;
					state->has_player_performed_any_action = true;
				}
				if(game->hold_input.down) {
					dir.y += 1;
					state->has_player_performed_any_action = true;
				}
				dir = v2_normalized(dir);

				if(game->press_input.dash) {
					player->wanted_to_dash_timestamp = state->update_count;
					if(game->dash_to_keyboard) {
						if(v2_length(dir) > 0) {
							player->next_dash_dir = dir;
						}
						else {
							player->next_dash_dir = player->dash_dir;
						}
					}
					else {
						player->next_dash_dir = v2_dir_from_to(player->pos, mouse_world);
					}
				}

				if(player->dashing) {
					dir = player->dash_dir * c_dash_speed * (1.0f + get_player_movement_speed() / 20);
					player->active_dash_timer += 1;
					if(player->active_dash_timer >= c_dash_duration) {
						player->dashing = false;
					}
				}
				else {
					player->dash_cooldown_timer.speed = get_multiplier(state->upgrade_level_arr[e_upgrade_dash_cooldown], c_dash_cooldown_speed_per_upgrade);
					player->dash_cooldown_timer.tick();

					if(player->dash_cooldown_timer.ready && (state->update_count - player->wanted_to_dash_timestamp) <= 12) {
						player->dash_cooldown_timer.reset();
						player->dash_dir = player->next_dash_dir;
						state->has_player_performed_any_action = true;
						dir = player->dash_dir;
						player->dashing = true;
						player->dash_start = player->pos;
						player->active_dash_timer = 0;
						play_sound_group(e_sound_group_dash);
					}

					dir *= get_player_movement_speed();
				}
				if(!floats_equal(dir.x, 0.0f)) {
					player->flip_x = dir.x > 0;
				}
				player->pos += dir;
				{
					s_bounds bounds = get_map_bounds();
					player->pos = constrain_pos(player->pos, bounds);
				}

				player->harvest_timer += 1;
				if(player->harvest_timer >= c_player_harvest_delay) {
					player->laser_target_arr.count = 0;

					s_sarray<int, c_max_player_hits> target_arr;
					float harvest_range = get_player_harvest_range();
					int multi_target = get_player_multi_target();
					int hits = get_player_hits();
					int player_damage = get_player_damage();

					for(int multi_target_i = 0; multi_target_i < multi_target; multi_target_i += 1) {
						int multi_target_creature = get_closest_creature2(player->pos, harvest_range, &cells, platform_data->frame_arena, target_arr);
						if(multi_target_creature < 0) { break; }
						target_arr.add(multi_target_creature);
					}

					foreach_val(target_i, target, target_arr) {
						int curr_target = target;
						s_v2 prev_prev_pos = v2(0);
						s_v2 prev_pos = v2(0);
						s_sarray<int, c_max_player_hits> blacklist;
						if(!creature_arr->active[curr_target]) { continue; }

						for(int hit_i = 0; hit_i < hits; hit_i += 1) {
							blacklist.add(curr_target);

							// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		add laser start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
							s_laser_target laser_target = zero;
							if(hit_i > 0) {
								laser_target.has_prev = true;
							}
							laser_target.from.prev_pos = prev_prev_pos;
							laser_target.from.pos = prev_pos;
							laser_target.to = {creature_arr->prev_pos[curr_target], creature_arr->pos[curr_target]};
							player->laser_target_arr.add(laser_target);
							s_damage_creature damage_result = damage_creature(curr_target, player_damage);
							if(damage_result.creature_died) {
								state->num_player_kills += 1;
							}
							if(damage_result.resource_gain_from_deposit > 0) {
								add_resource(damage_result.resource_gain_from_deposit);
							}
							else if(damage_result.creature_died) {
								add_resource(get_creature_resource_reward(creature_arr->tier[curr_target], creature_arr->boss[curr_target]));
							}
							// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		add laser end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
							player->harvest_timer = 0;

							prev_prev_pos = creature_arr->prev_pos[curr_target];
							prev_pos = creature_arr->pos[curr_target];

							curr_target = get_closest_creature2(creature_arr->pos[curr_target], harvest_range, &cells, platform_data->frame_arena, blacklist);
							if(curr_target < 0) { break; }
						}
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update buffs start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* p = &state->player;
				for_enum(buff_i, e_pickup) {
					at_least_add(&p->buff_arr[buff_i].ticks_left, -1, 0);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update buffs end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		collect pickup start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				foreach_val(pickup_i, pickup, state->pickup_arr) {
					if(rect_collides_rect_center(state->player.pos, c_player_size, pickup.pos, c_pickup_size)) {
						add_buff(&state->player, pickup.type);
						state->pickup_arr.remove_and_swap(pickup_i);
						pickup_i -= 1;
						play_sound_group(e_sound_group_upgrade);
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		collect pickup end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		collect broken bot start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				foreach_val(bot_i, bot, state->broken_bot_arr) {
					if(rect_collides_rect_center(state->player.pos, c_player_size, bot.pos, c_bot_size)) {
						int new_bot = make_bot(bot.pos);
						if(new_bot >= 0) {
							bot_arr->state[new_bot] = e_bot_state_going_back_to_base;
						}
						state->broken_bot_arr.remove_and_swap(bot_i);
						bot_i -= 1;
						play_sound_group(e_sound_group_buy_bot);
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		collect broken bot end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update creatures start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			for_creature_partial(creature) {
				if(!creature_arr->active[creature]) { continue; }
				int time_passed = state->update_count - creature_arr->tick_when_last_damaged[creature];
				if(can_creature_move(creature_arr->type[creature])) {
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
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update creatures end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update bots start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			for_bot_partial(bot) {
				if(!bot_arr->active[bot]) { continue; }

				bot_arr->harvest_timer[bot] += 1;
				if(bot_arr->harvest_timer[bot] >= c_bot_harvest_delay) {
					bot_arr->laser_target_arr[bot].count = 0;
				}

				switch(bot_arr->state[bot]) {
					case e_bot_state_going_to_creature: {
						int creature = get_creature(bot_arr->target[bot]);
						if(creature >= 0) {
							bot_arr->pos[bot] = go_towards(bot_arr->pos[bot], creature_arr->pos[creature], get_bot_movement_speed());
							if(rect_collides_circle_center(creature_arr->pos[creature], get_creature_size(creature), bot_arr->pos[bot], get_bot_harvest_range())) {
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

							if(bot_arr->harvest_timer[bot] >= c_bot_harvest_delay) {
								int curr_target = creature;
								int hit_count = get_bot_hits();
								int bot_damage = get_bot_damage();
								float harvest_range = get_bot_harvest_range();
								s_sarray<int, c_max_bot_hits> blacklist;
								b8 pick_new_target = false;
								int max_cargo = get_bot_max_cargo_count();
								s_v2 prev_prev_pos = v2(0);
								s_v2 prev_pos = v2(0);

								for(int hit_i = 0; hit_i < hit_count; hit_i += 1) {
									bot_arr->harvest_timer[bot] = 0;
									blacklist.add(curr_target);
									s_damage_creature damage_result = damage_creature(curr_target, bot_damage);
									int to_add = 0;
									if(damage_result.creature_died) {
										state->num_bot_kills += 1;
									}
									if(damage_result.resource_gain_from_deposit > 0) {
										to_add = damage_result.resource_gain_from_deposit;
									}
									else if(damage_result.creature_died) {
										to_add = get_creature_resource_reward(creature_arr->tier[curr_target], creature_arr->boss[curr_target]);
									}
									if(to_add > 0) {
										bot_arr->cargo[bot] += to_add;
										bot_arr->cargo_count[bot] += 1;
										if(bot_arr->cargo_count[bot] >= max_cargo) {
											bot_arr->state[bot] = e_bot_state_going_back_to_base;
											pick_new_target = false;
										}
										else if(hit_i == 0 && damage_result.creature_died) {
											pick_new_target = true;
										}
									}

									{
										s_laser_target lt = zero;
										lt.has_prev = hit_i > 0;
										lt.from.prev_pos = prev_prev_pos;
										lt.from.pos = prev_pos;
										lt.to.prev_pos = creature_arr->prev_pos[curr_target];
										lt.to.pos = creature_arr->pos[curr_target];
										bot_arr->laser_target_arr[bot].add(lt);
									}

									prev_prev_pos = creature_arr->prev_pos[curr_target];
									prev_pos = creature_arr->pos[curr_target];

									curr_target = get_closest_creature2(creature_arr->pos[curr_target], harvest_range, &cells, platform_data->frame_arena, blacklist);
									if(curr_target < 0) { break; }


								}
								if(pick_new_target) {
									bot_arr->state[bot] = e_bot_state_going_to_creature;
									pick_target_for_bot(bot);
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
							// assert(bot_arr->cargo[bot] > 0);
							add_resource(bot_arr->cargo[bot]);
							bot_arr->cargo[bot] = 0;
							bot_arr->cargo_count[bot] = 0;
						}
					} break;
				}

			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update bots end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		check win condition start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				if(state->resource_count >= c_resource_to_win || state->sub_state == e_sub_state_winning) {
					if(state->sub_state == e_sub_state_winning) {
						state->win_ticks += 1;
						if(state->win_ticks >= c_win_animation_duration_in_ticks) {
							if constexpr(c_are_we_on_web) {
								if(platform_data->leaderboard_nice_name.len > 0) {
									set_state_next_frame(e_state_win_leaderboard);
									platform_data->submit_leaderboard_score(
										game->play_state.update_count, c_leaderboard_id, on_leaderboard_score_submitted
									);
									game->play_state.update_count_at_win_time = game->play_state.update_count;
								}
								else {
									set_state_next_frame_temporary(e_state_input_name);
								}
							}
							else {
								set_state_next_frame(e_state_win_leaderboard);
								game->play_state.update_count_at_win_time = game->play_state.update_count;
							}
						}
					}
					else {
						state->sub_state = e_sub_state_winning;
						state->win_ticks = 0;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		check win condition end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			float nectar_per_second = get_nectar_per_second();
			int statistics_index = circular_index(state->update_count / 60, c_max_statistics_index);
			state->highest_nectar_gain_per_second = max(state->highest_nectar_gain_per_second, nectar_per_second);
			state->num_player_kills_arr[statistics_index] = state->num_player_kills;
			state->num_bot_kills_arr[statistics_index] = state->num_bot_kills;
			state->nectar_per_second_arr[statistics_index] = nectar_per_second;
			state->nectar_arr[statistics_index] = state->resource_count;

			if(state->has_player_performed_any_action) {
				state->update_count += 1;
			}

		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		play state update end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
	}

	if(is_last_update_this_frame) {
		game->hold_input = zero;
	}
	game->press_input = zero;
}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt)
{
	static_assert(sizeof(s_game) <= c_game_memory);

	g_mouse = platform_data->mouse;

	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = &platform_data->input;
	g_platform_data = platform_data;

	live_variable(&platform_data->vars, c_player_movement_speed, 0.0f, 100.0f, true);
	live_variable(&platform_data->vars, c_creature_roam_delay, 1, 1000, true);
	live_variable(&platform_data->vars, c_creature_roam_distance, 16.0f, 1024.0f, true);
	live_variable(&platform_data->vars, c_bot_movement_speed, 1.0f, 32.0f, true);
	live_variable(&platform_data->vars, c_creature_speed, 0.0f, 100.0f, true);
	live_variable(&platform_data->vars, c_bot_harvest_range, 0.0f, 256.0f, true);
	live_variable(&platform_data->vars, c_player_harvest_range, 0.0f, 256.0f, true);
	live_variable(&platform_data->vars, c_bot_harvest_delay, 0, 100, true);
	live_variable(&platform_data->vars, c_player_harvest_delay, 0, 100, true);
	live_variable(&platform_data->vars, c_tile_rand_min, 0.0f, 1.0f, true);
	live_variable(&platform_data->vars, c_tile_rand_max, 0.0f, 1.0f, true);

	s_m4 ortho = m4_orthographic(0, c_base_res.x, c_base_res.y, 0, -100, 100);

	game->click_consumed = false;

	g_delta = (float)platform_data->frame_time;
	game->render_time += g_delta;

	s_play_state* play_state = &game->play_state;
	s_camera2d* cam = &play_state->cam;

	if(g_input->wheel_movement > 0) {
		cam->target_zoom *= 1.1f;
	}
	else if(g_input->wheel_movement < 0) {
		cam->target_zoom *= 0.9f;
	}
	cam->target_zoom = clamp(cam->target_zoom, 0.2f, 5.0f);

	#ifdef m_debug
	if(is_key_pressed(g_input, c_key_add)) {
		circular_index_add(&g_r->game_speed_index, 1, array_count(c_game_speed_arr));
	}
	if(is_key_pressed(g_input, c_key_subtract)) {
		circular_index_add(&g_r->game_speed_index, -1, array_count(c_game_speed_arr));
	}
	if(is_key_pressed(g_input, c_key_f1)) {
		play_state->resource_count = floorfi(c_resource_to_win * 0.9f);
	}
	if(is_key_pressed(g_input, c_key_f2)) {
		play_state->resource_count = c_resource_to_win;
	}
	if(is_key_pressed(g_input, c_key_f3)) {
		play_state->level_up_triggers += add_exp(&play_state->player, 5);
	}
	if(is_key_pressed(g_input, c_key_f4)) {
		game->show_hitboxes = !game->show_hitboxes;
	}
	if(is_key_pressed(g_input, c_key_f5)) {
		for_enum(pickup_i, e_pickup) {
			add_buff(&game->play_state.player, pickup_i);
		}
	}
	if(is_key_pressed(g_input, c_key_f6)) {
		game->play_state.sub_state = e_sub_state_defeat;
	}
	if(get_state() == e_state_play && is_key_pressed(g_input, c_key_r)) {
		game->reset_game = true;
	}
	#endif // m_debug


	if(can_pause() && (is_key_pressed(g_input, c_key_escape) || is_key_pressed(g_input, c_key_p))) {
		if(game->play_state.sub_state == e_sub_state_pause) {
			game->play_state.sub_state = e_sub_state_default;
		}
		else {
			game->play_state.sub_state = e_sub_state_pause;
		}
	}

	{
		s_v2 pos = lerp(game->play_state.player.prev_pos, game->play_state.player.pos, interp_dt);
		game->play_state.cam.target_pos = pos;
		game->play_state.cam.offset = c_half_res;

		{
			float dt = game->do_instant_camera ? 1.0f : 0.04f;
			game->play_state.cam.pos = lerp_snap(game->play_state.cam.pos, game->play_state.cam.target_pos, dt, v2(1.0f));
		}
		{
			float dt = game->do_instant_camera ? 1.0f : 0.1f;
			game->play_state.cam.zoom = lerp_snap(game->play_state.cam.zoom, game->play_state.cam.target_zoom, dt, 0.001f);
		}
	}

	s_v2 mouse_world = play_state->cam.screen_to_world(g_mouse);

	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	s_bot_arr* bot_arr = &game->play_state.bot_arr;

	s_m4 view = cam->get_matrix();

	g_r->clear_framebuffer(game->main_fbo, zero, c_default_fbo_clear_flags);
	g_r->clear_framebuffer(game->light_fbo, v4(0.75f, 0.75f, 0.75f, 1.0f), e_fbo_clear_color);

	switch(get_state()) {

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		play state draw start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		case e_state_play: {

			game->hold_input.left |= is_action_down(g_platform_data, g_input, e_action_left);
			game->hold_input.right |= is_action_down(g_platform_data, g_input, e_action_right);
			game->hold_input.up |= is_action_down(g_platform_data, g_input, e_action_up);
			game->hold_input.down |= is_action_down(g_platform_data, g_input, e_action_down);

			game->press_input.dash |= is_action_pressed(g_platform_data, g_input, e_action_dash);

			float laser_light_radius = sin_range(120, 128, game->render_time * 32.0f);

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		player buff visuals start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player p = game->play_state.player;
				s_v2 pos = lerp(p.prev_pos, p.pos, interp_dt);
				for_enum(pickup_i, e_pickup) {
					if(has_buff(pickup_i)) {
						do_particles(8, pos, e_layer_particle, true, multiply_particle_data(c_buff_particle_data_arr[pickup_i], {.radius = 3, .speed = 3}));
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		player buff visuals end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw pickups start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				foreach_val(pickup_i, pickup, game->play_state.pickup_arr) {
					do_particles(8, pickup.pos, e_layer_particle, false, c_buff_particle_data_arr[pickup.type]);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw pickups end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			s_carray2<float, 256, 256> brightness_arr = zero;
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw background start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_bounds bounds = get_cam_bounds_snap_to_tile_size(*cam);
				int start_x = floorfi(bounds.min_x / c_tile_size);
				int start_y = floorfi(bounds.min_y / c_tile_size);
				int end_x = ceilfi(bounds.max_x / c_tile_size);
				int end_y = ceilfi(bounds.max_y / c_tile_size);
				for(int y = start_y; y <= end_y; y += 1) {
					for(int x = start_x; x <= end_x; x += 1) {
						s_rng rng = make_rng(x + y * 256);
						int x_dist = abs(roundfi(c_base_pos.x / c_tile_size) - x);
						int y_dist = abs(roundfi(c_base_pos.y / c_tile_size) - y);
						int dist = x_dist + y_dist;
						float bright = rng.randf_range(c_tile_rand_min, c_tile_rand_max);
						bright *= 1.0f - (dist / 30.0f);
						s_v4 color = make_color(bright);
						brightness_arr[y - start_y][x - start_x] = bright;
						s_v2 pos = v2(x, y) * c_tile_size;
						draw_texture(g_r, pos, e_layer_background, v2(c_tile_size), color, game->tile_texture, game->world_render_pass_arr[0], {.flip_x = rng.rand_bool()}, {.origin_offset = c_origin_topleft});
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw background end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		doodads start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_bounds cam_bounds = get_cam_bounds_snap_to_tile_size(game->play_state.cam);
				for(int crater_i = 0; crater_i < c_max_craters; crater_i += 1) {
					s_v2 pos = game->play_state.crater_pos_arr[crater_i];
					float size = game->play_state.crater_size_arr[crater_i];
					s_v2 p = v2(
						pos.x - cam_bounds.min_x,
						pos.y - cam_bounds.min_y
					);
					int x = floorfi(p.x / c_tile_size);
					int y = floorfi(p.y / c_tile_size);
					if(is_valid_index(x, y, 256, 256)) {
						float brightness = at_most(1.0f, brightness_arr[y][x] * 1.1f);
						draw_texture_keep_aspect(
							g_r, pos, e_layer_crater, v2(size), make_color(brightness), game->crater_texture, game->world_render_pass_arr[0],
							{.flip_x = game->play_state.crater_flip_arr[crater_i]}, {.rotation = game->play_state.crater_rotation_arr[crater_i]}
						);
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		doodads end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw base start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				draw_texture_keep_aspect(g_r, c_base_pos, e_layer_base, c_base_size, make_color(1), game->base_texture, game->world_render_pass_arr[0]);
				g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.depth_mode = e_depth_mode_read_and_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw base end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw creatures start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			for_creature_partial(creature) {
				if(!creature_arr->active[creature]) { continue; }
				e_creature type = creature_arr->type[creature];
				s_v2 pos = lerp(creature_arr->prev_pos[creature], creature_arr->pos[creature], interp_dt);
				s_v4 color_arr[] = {
					make_color(1), make_color(0.332f, 0.809f, 0.660f), make_color(0.581f, 0.705f, 0.104f), make_color(0.434f, 0.172f, 0.290f),
					make_color(0.473f, 0.549f, 0.744f), make_color(0.276f, 0.523f, 0.052f), make_color(0.076f, 0.185f, 0.868f), make_color(0.525f, 0.820f, 0.016f),
				};

				int color_index = creature_arr->tier[creature] % array_count(color_arr);
				float mix_weight = 1.0f - (play_state->update_count - creature_arr->tick_when_last_damaged[creature]) / 5.0f;
				mix_weight = clamp(mix_weight, 0.0f, 1.0f);

				s_texture texture;
				if(can_creature_move(type)) {
					b8 moving = v2_distance(creature_arr->target_pos[creature], creature_arr->pos[creature]) > 1;
					if(moving) {
						creature_arr->animation_timer[creature] += g_delta;
						texture = get_animation_texture(&game->ant_animation, &creature_arr->animation_timer[creature]);
					}
					else {
						texture = game->ant_animation.texture_arr[0];
					}
				}
				else {
					texture = game->base_texture;
				}

				draw_texture_keep_aspect(
					g_r, pos, e_layer_creature, get_creature_size(creature), color_arr[color_index], texture,
					game->world_render_pass_arr[0], {.flip_x = creature_arr->flip_x[creature]}, {.mix_weight = mix_weight}
				);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw creatures end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw broken bots start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				foreach_val(bot_i, bot, game->play_state.broken_bot_arr) {
					draw_texture_keep_aspect(g_r, bot.pos, e_layer_broken_bot, c_bot_size, make_color(1), game->broken_bot_texture,
						game->world_render_pass_arr[0], {}, {.rotation = bot.rotation}
					);

					do_particles(1, bot.pos, e_layer_particle, false, {
						.shrink = 3,
						.slowdown = 1,
						.duration = 3,
						.duration_rand = 0.5f,
						.speed = 200,
						.speed_rand = 1,
						.angle = -pi * 0.5f,
						.angle_rand = 0.2f,
						.radius = 8,
						.color = v3(0.5f, 0.1f, 0.1f),
						.color_rand = v3(0.1f, 0.1f, 0.1f),
					});
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw broken bots end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.depth_mode = e_depth_mode_read_and_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});

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
				draw_texture_keep_aspect(g_r, pos, e_layer_bot, c_bot_size, color, texture, game->world_render_pass_arr[0], {}, {.rotation = tilt});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw bots end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.depth_mode = e_depth_mode_read_and_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* p = &game->play_state.player;
				s_v2 pos = lerp(p->prev_pos, p->pos, interp_dt);
				s_texture texture = get_animation_texture(&game->player_animation, &p->animation_timer);
				draw_texture(g_r, pos, e_layer_player, c_player_size, make_color(1), texture, game->world_render_pass_arr[0], {.flip_x = p->flip_x});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.depth_mode = e_depth_mode_read_and_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		shadows start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				foreach_val(bot_i, bot, game->play_state.broken_bot_arr) {
					draw_shadow(bot.pos + v2(0, 10), 36, 0.5f, 0.0f);
				}

				for_creature_partial(creature) {
					if(!creature_arr->active[creature]) { continue; }
					s_v2 pos = lerp(creature_arr->prev_pos[creature], creature_arr->pos[creature], interp_dt);
					float size_multi = get_creature_size_multi(creature);
					draw_shadow(pos + v2(0, 10), 36 * size_multi, 0.33f, 0.0f);
				}

				for_bot_partial(bot) {
					if(!bot_arr->active[bot]) { continue; }
					s_v2 pos = lerp(bot_arr->prev_pos[bot], bot_arr->pos[bot], interp_dt);
					draw_shadow(pos + v2(0, 100), 32, 0.25f, 0.0f);
				}

				{
					s_player* p = &game->play_state.player;
					s_v2 pos = lerp(p->prev_pos, p->pos, interp_dt);
					draw_shadow(pos + v2(0, 32), 128, 0.5f, 0.1f);
				}

				g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->light_fbo, {.depth_mode = e_depth_mode_read_no_write, .blend_mode = e_blend_mode_multiply_inv, .view = view, .projection = ortho});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		shadows end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		lights start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				draw_light(c_base_pos, c_base_size.x, make_color(0.5f), 0.0f);

				{
					s_player p = play_state->player;
					s_v2 pos = lerp(p.prev_pos, p.pos, interp_dt);
					draw_light(pos, 256, make_color(0.5f), 0.0f);

					foreach_val(target_i, target, p.laser_target_arr) {
						if(!target.has_prev) {
							target.from.prev_pos = p.prev_pos;
							target.from.pos = p.pos;
						}
						draw_laser(target, laser_light_radius, make_color(0.5f, 0.1f, 0.1f), interp_dt);
					}
				}

				for_bot_partial(bot) {
					if(!bot_arr->active[bot]) { continue; }
					s_v2 pos = lerp(bot_arr->prev_pos[bot], bot_arr->pos[bot], interp_dt);
					draw_light(pos, 48, make_color(0.33f), 0.0f);

					foreach_val(target_i, target, bot_arr->laser_target_arr[bot]) {
						if(!target.has_prev) {
							target.from.prev_pos = bot_arr->prev_pos[bot];
							target.from.pos = bot_arr->pos[bot];
						}
						draw_laser(target, laser_light_radius, make_color(0.1f, 0.5f, 0.1f), interp_dt);
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		lights end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->light_fbo, {.blend_mode = e_blend_mode_additive, .view = view, .projection = ortho});

			draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->light_fbo, game->world_render_pass_arr[0]);
			g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.blend_mode = e_blend_mode_multiply, .projection = ortho});

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		particles start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
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
					s_v2 offset = zero;
					if(p->attached_to_player) {
						offset = lerp(game->play_state.player.prev_pos, game->play_state.player.pos, interp_dt);
					}
					draw_circle(g_r, offset + p->pos, p->z, radius, color, game->world_render_pass_arr[1]);
					p->timer += (float)platform_data->frame_time;
					if(percent_done >= 1) {
						game->play_state.particle_arr.remove_and_swap(particle_i);
						particle_i -= 1;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		particles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		player dash trail start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* p = &game->play_state.player;
				s_v2 pos = lerp(p->prev_pos, p->pos, interp_dt);
				if(!p->dashing) {
					p->animation_timer += g_delta;
				}
				s_texture texture = get_animation_texture(&game->player_animation, &p->animation_timer);
				if(p->dashing) {
					s_v2 a = p->dash_start;
					s_v2 b = pos;
					float dist = v2_distance(a, b);
					int steps = ceilfi(dist / (c_player_size.x * 0.1f));
					for(int i = 0; i < steps; i += 1) {
						float percent = index_count_safe_div(i, steps);
						float alpha = 1.0f - index_count_safe_div(p->active_dash_timer, c_dash_duration);
						s_v2 c = lerp(a, b, percent);
						draw_texture(g_r, c, e_layer_player, c_player_size * 1.25f, v4(0.75f, 0.75f, 0.75f, alpha * percent), texture, game->world_render_pass_arr[1], {.flip_x = p->flip_x});
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		player dash trail end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			g_r->end_render_pass(g_r, game->world_render_pass_arr[1], game->main_fbo, {.blend_mode = e_blend_mode_additive, .view = view, .projection = ortho});

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		hitboxes start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(game->show_hitboxes) {
				for_creature_partial(creature) {
					if(!creature_arr->active[creature]) { continue; }
					s_v2 pos = lerp(creature_arr->prev_pos[creature], creature_arr->pos[creature], interp_dt);
					draw_rect(g_r, pos, e_layer_hitbox, get_creature_size(creature), v4(0.0f, 0.5f, 0.0f, 0.5f), game->world_render_pass_arr[0]);
				}

				foreach_val(bot_i, bot, game->play_state.broken_bot_arr) {
					draw_rect(g_r, bot.pos, e_layer_hitbox, c_bot_size, v4(0.0f, 0.5f, 0.0f, 0.5f), game->world_render_pass_arr[0]);
				}

				for_bot_partial(bot) {
					if(!bot_arr->active[bot]) { continue; }
					s_v2 pos = lerp(bot_arr->prev_pos[bot], bot_arr->pos[bot], interp_dt);
					draw_circle(g_r, pos, e_layer_hitbox, get_bot_harvest_range(), v4(0.0f, 0.0f, 0.5f, 0.5f), game->world_render_pass_arr[0]);
				}

				{
					s_player* p = &game->play_state.player;
					s_v2 pos = lerp(p->prev_pos, p->pos, interp_dt);
					draw_rect(g_r, pos, e_layer_hitbox, c_player_size, v4(0.0f, 0.5f, 0.0f, 0.5f), game->world_render_pass_arr[0]);
					draw_circle(g_r, pos, e_layer_hitbox, get_player_harvest_range(), v4(0.0f, 0.0f, 0.5f, 0.5f), game->world_render_pass_arr[0]);
				}
			}
			g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		hitboxes end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		display controls tutorial start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(!game->hide_tutorial) {
				s_v2 player_pos = lerp(play_state->player.prev_pos, play_state->player.pos, interp_dt);
				player_pos += v2(0, 100);
				float time_passed = play_state->update_count / (float)c_updates_per_second;
				if(time_passed < 15) {
					s_len_str text_arr[] = {
						strlit("Move - WASD or Arrow keys"),
						strlit("Dash - Space or Right click"),
						strlit("Options - P or Escape"),
						strlit("Zoom - Scroll wheel"),
					};
					float alpha = 1;
					if(time_passed >= 14) {
						alpha = range_lerp(time_passed, 14, 15, 1, 0);
					}

					s_v2 text_pos = player_pos;
					for(int text_i = 0; text_i < array_count(text_arr); text_i += 1) {
						float zoom = play_state->cam.zoom;
						float font_size = clamp(32 / zoom, 2.0f, 70.0f);
						draw_text(g_r, text_arr[text_i], text_pos, 0, font_size, make_color(1.0f, alpha), true, game->font, game->world_render_pass_arr[0]);
						text_pos.y += font_size;
					}
				}
				if(time_passed >= 15 && time_passed <= 30) {
					float zoom = play_state->cam.zoom;
					float alpha = ease_linear_advanced(time_passed, 15.0f, 16.0f, 0.0f, 1.0f);
					alpha *= ease_linear_advanced(time_passed, 29, 30, 1, 0);
					float font_size = clamp(32 / zoom, 2.0f, 70.0f);
					s_len_str text = strlit(
						"Your goal is to get 50000 nectar, but before you can do that\n"
						"you will need to use it to get stronger and force more ants to spawn,\n"
						"thus increasing your nectar income."
					);
					draw_text(g_r, text, player_pos, 0, font_size, make_color(1.0f, alpha), true, game->font, game->world_render_pass_arr[0]);
				}
				if(time_passed >= 30 && time_passed <= 45) {
					float zoom = play_state->cam.zoom;
					float alpha = ease_linear_advanced(time_passed, 30.0f, 31.0f, 0.0f, 1.0f);
					alpha *= ease_linear_advanced(time_passed, 44, 45, 1, 0);
					float font_size = clamp(32 / zoom, 2.0f, 70.0f);
					s_len_str text = strlit(
						"But be careful, as letting too many ants accumulate will be the end.\n"
						"1000 ants would be too much to handle."
					);
					draw_text(g_r, text, player_pos, 0, font_size, make_color(1.0f, alpha), true, game->font, game->world_render_pass_arr[0]);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		display controls tutorial end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});

			b8 show_ui = should_show_ui();
			if(play_state->sub_state == e_sub_state_winning) {
				float prev_timer = ticks_to_seconds(at_least(0, play_state->win_ticks - 1));
				float curr_timer = ticks_to_seconds(at_least(0, play_state->win_ticks));
				float timer = lerp(prev_timer, curr_timer, interp_dt);
				float p = timer / ticks_to_seconds(c_win_animation_duration_in_ticks);
				draw_rect(g_r, c_half_res, 0, c_base_res, make_color(0, p), game->ui_render_pass0);
				float alpha = ease_linear_advanced(timer, 0, 1, 0, 1);
				draw_text(g_r, strlit("Victory!"), c_base_res * v2(0.5f, 0.1f), 10, 36, make_color(1, alpha), true, game->font, game->ui_render_pass1);
			}
			if(play_state->sub_state == e_sub_state_defeat) {
				draw_rect(g_r, c_half_res, 0, c_base_res, v4(0.0f, 0.0f, 0.0f, 0.5f), game->ui_render_pass0);
				draw_text(g_r, strlit("You were overwhelmed!"), c_base_res * v2(0.5f, 0.4f), 0, 64, make_color(1), true, game->font, game->ui_render_pass1);
				draw_text(g_r, strlit("Press R to restart..."), c_base_res * v2(0.5f, 0.5f), 0, 64, make_color(0.6f), true, game->font, game->ui_render_pass1);

				if(is_key_pressed(g_input, c_key_r)) {
					game->reset_game = true;
				}
			}
			if(show_ui) {

				if(!game->hide_timer) {
					s_time_data time_data = update_count_to_time_data(game->play_state.update_count, c_update_delay);
					s_len_str text = format_text("%02i:%02i.%03i", time_data.minutes, time_data.seconds, time_data.ms);
					draw_text(g_r, text, v2(4, 4), 0, 32, make_color(1), false, game->font, game->ui_render_pass1);
				}

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		upgrade buttons start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				{
					constexpr float padding = 8;
					constexpr float font_size = 24;
					constexpr int c_rows = 3;
					constexpr float c_hotkey_font_size = 21;
					struct s_row
					{
						int upgrade_count;
						s_carray<e_upgrade, e_upgrade_count> upgrade_arr;
					};
					s_carray<s_row, c_rows> row_arr = zero;
					row_arr[0].upgrade_count = 5;
					row_arr[0].upgrade_arr = {{e_upgrade_player_damage, e_upgrade_player_movement_speed, e_upgrade_player_harvest_range, e_upgrade_player_chain, e_upgrade_dash_cooldown}};
					row_arr[1].upgrade_count = 6;
					row_arr[1].upgrade_arr = {{e_upgrade_buy_bot, e_upgrade_bot_damage, e_upgrade_bot_movement_speed, e_upgrade_bot_cargo_count, e_upgrade_bot_harvest_range, e_upgrade_broken_bot_spawn}};
					row_arr[2].upgrade_count = 5;
					row_arr[2].upgrade_arr = {{e_upgrade_spawn_rate, e_upgrade_creature_tier, e_upgrade_double_harvest, e_upgrade_deposit_spawn_rate, e_upgrade_deposit_health}};
					s_v2 button_size = v2(54);
					s_v2 panel_pos = v2(0.0f, c_base_res.y - (button_size.y + padding) * 4.3f);
					s_v2 panel_size = v2(c_base_res.x, button_size.y + padding);
					s_carray<s_pos_area, c_rows * 2> area_arr;
					s_pos_area temp0 = make_vertical_layout(panel_pos + v2(padding * 2 + button_size.x * 0.5f, -16.0f), button_size * v2(1.0f, 1.7f), padding, 0);
					s_pos_area temp1 = make_vertical_layout(panel_pos + v2(padding * 2, 0.0f), button_size * v2(1.0f, 1.7f), padding, 0);
					for(int i = 0; i < 3; i += 1) {
						area_arr[i * 2] = make_horizontal_layout(pos_area_get_advance(&temp0), button_size * v2(1.7f, 1.0f), padding, 0);
						area_arr[i * 2 + 1] = make_horizontal_layout(pos_area_get_advance(&temp1), button_size * v2(1.7f, 1.0f), padding, 0);
					}
					for(int row_i = 0; row_i < c_rows; row_i += 1) {
						s_row row = row_arr[row_i];
						for(int upgrade_i = 0; upgrade_i < row.upgrade_count; upgrade_i += 1) {
							e_upgrade upgrade_id = row.upgrade_arr[upgrade_i];
							s_upgrade_data data = c_upgrade_data[upgrade_id];
							int curr_level = play_state->upgrade_level_arr[upgrade_id];
							int cost = data.base_cost * (curr_level + 1);
							b8 over_limit = play_state->upgrade_level_arr[upgrade_id] >= data.max_upgrades;
							s_ui_optional optional = zero;
							optional.description = get_upgrade_tooltip(upgrade_id);
							optional.font_size = font_size;
							optional.size_x = button_size.x;
							optional.size_y = button_size.y;
							optional.darken = cost > play_state->resource_count ? 0.33f : 1.0f;

							if(!over_limit) {
								s_len_str cost_str = shorten_number(cost);
								draw_text(
									g_r, format_text("%.*s[%c]", expand_str(cost_str), (char)data.key), pos_area_get_advance(&area_arr[row_i * 2]), 0,
									c_hotkey_font_size, make_color(1), true, game->font, game->ui_render_pass1
								);
							}

							if(
								!over_limit &&
								(ui_texture_button(
									format_text("upgrade%i", upgrade_id), pos_area_get_advance(&area_arr[row_i * 2 + 1]),
									game->upgrade_button_texture_arr[upgrade_id], optional
								) || is_key_pressed(g_input, data.key))
							) {
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
											play_sound_group(e_sound_group_buy_bot);
										}
									}
									else {
										purchased = true;
										play_sound_group(e_sound_group_upgrade);
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

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		buff display start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				{
					s_v2 size = v2(32);
					s_pos_area area = make_pos_area(wxy(0.005f, 0.04f), wxy(1.0f, 0.07f), size, 8, -1, 0);
					s_v4 color_arr[] = {
						make_color(0, 0.5f, 0), make_color(0.5f, 0, 0), make_color(0, 0, 0.5f),
					};
					for_enum(pickup_i, e_pickup) {
						s_v2 pos = pos_area_get_advance(&area);
						if(has_buff(pickup_i)) {
							draw_rect(g_r, pos, 0, size, color_arr[pickup_i], game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});

							s_carray<float, 3> data = ticks_to_seconds2(play_state->player.buff_arr[pickup_i].ticks_left + 1, interp_dt);
							draw_text(
								g_r, format_text("%.0f", at_most(99.0f, ceilf(data[1]))), pos + size * 0.5f + v2(0.0f, 2.0f), 0,
								30, make_color(1), true, game->font, game->ui_render_pass1
							);
						}
					}
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		buff display end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^


				{
					s_v2 bar_size = wxy(0.33f, 0.025f);
					s_pos_area area = make_vertical_layout(v2(0, 16), bar_size, 8, e_pos_area_flag_center_x);


					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		win progress start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					{
						float nectar_per_second = get_nectar_per_second();
						float progress = play_state->resource_count / (float)c_resource_to_win;
						s_v4 color = make_color(0.786f, 0.615f, 0.252f);
						s_v2 pos = pos_area_get_advance(&area);
						s_len_str str = format_text("%i / %i +%.1f/s", play_state->resource_count, c_resource_to_win, nectar_per_second);
						draw_progress_bar(pos, bar_size, brighter(color, 0.5f), color, str, progress);
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		win progress end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		lose progress start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					{
						int alive_creatures = count_alive_creatures();
						float defeat_progress = alive_creatures / (float)c_num_creatures_to_lose;
						float shake_intensity = 0;
						if(defeat_progress > 0.75f) { shake_intensity = range_lerp(defeat_progress, 0.75f, 1.0f, 2.0f, 5.0f); }
						else if(defeat_progress > 0.5f) { shake_intensity = range_lerp(defeat_progress, 0.5f, 0.75f, 1.0f, 2.0f); }
						s_v2 offset = v2(
							game->rng.randf32_11() * shake_intensity,
							game->rng.randf32_11() * shake_intensity
						);
						s_v2 pos = pos_area_get_advance(&area) + offset;
						s_len_str str = format_text("%i / %i", alive_creatures, c_num_creatures_to_lose);
						draw_progress_bar(pos, bar_size, make_color(0.25f, 0.1f, 0.1f), make_color(0.66f, 0.1f, 0.1f), str, defeat_progress);
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		lose progress end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

					// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		level up progress start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
					{
						s64 required_exp = get_required_exp_to_level(play_state->player.curr_level);
						float level_progress = play_state->player.curr_exp / (float)required_exp;
						s_v4 color = v4(0.0f, 0.474f, 0.945f, 1.0f);
						s_v2 pos = pos_area_get_advance(&area);
						draw_progress_bar(pos, bar_size, brighter(color, 0.5f), color, format_text("%i", play_state->player.curr_level), level_progress);
					}
					// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		level up progress end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
				}
			}

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		options pause menu start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(play_state->sub_state == e_sub_state_pause) {
				do_options_menu(true);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		options pause menu end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		level up menu start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			if(play_state->sub_state == e_sub_state_level_up) {

				{
					float s = sin_range(0.0f, 1.0f, game->render_time * 1.0f);
					s_v4 color = hsv_to_rgb(v3(s, 1, 1));
					draw_text(
						g_r, strlit("Free upgrade!"), wxy(0.5f, sin_range(0.1f, 0.15f, game->render_time * 8)), 10,
						sin_range(90.0f, 128.0f, game->render_time * 8.0f), color, true, game->font, game->ui_render_pass1
					);
				}

				s_rng rng = make_rng(play_state->level_up_seed);
				s_sarray<e_upgrade, e_upgrade_count> possible_upgrade_arr;
				s_sarray<f64, e_upgrade_count> weight_arr;
				s_sarray<e_upgrade, 3> choice_arr;
				for_enum(upgrade_i, e_upgrade) {
					s_upgrade_data data = c_upgrade_data[upgrade_i];
					b8 over_limit = play_state->upgrade_level_arr[upgrade_i] >= data.max_upgrades;
					int curr_level = play_state->upgrade_level_arr[upgrade_i];
					int cost = data.base_cost * (curr_level + 1);
					if(upgrade_i != e_upgrade_double_harvest && !over_limit) {
						possible_upgrade_arr.add(upgrade_i);
						f64 weight = 1.0 / cost;
						weight = weight * weight * weight;
						weight_arr.add(weight);
					}
				}
				assert(possible_upgrade_arr.count == weight_arr.count);

				for(int i = 0; i < 3; i += 1) {
					int index = pick_weighted(weight_arr.elements, weight_arr.count, &rng);
					e_upgrade element = possible_upgrade_arr[index];
					possible_upgrade_arr.remove_and_swap(index);
					weight_arr.remove_and_swap(index);
					choice_arr.add(element);
				}

				// @Note(tkap, 16/10/2024): Sort in ascending order
				if(get_upgrade_cost(choice_arr[0]) > get_upgrade_cost(choice_arr[1])) {
					swap(&choice_arr[0], &choice_arr[1]);
				}
				if(get_upgrade_cost(choice_arr[1]) > get_upgrade_cost(choice_arr[2])) {
					swap(&choice_arr[1], &choice_arr[2]);
				}
				if(get_upgrade_cost(choice_arr[0]) > get_upgrade_cost(choice_arr[2])) {
					swap(&choice_arr[0], &choice_arr[2]);
				}

				int picked_choice = -1;
				s_ui_optional optional = zero;
				s_v2 button_size = v2(128);
				optional.size_x = button_size.x;
				optional.size_y = button_size.y;
				optional.font_size = 40;
				optional.tooltip_font_size = 40;

				constexpr float font_size = 48;
				s_pos_area area = make_pos_area(wxy(0.0f, 0.0f), wxy(1.0f, 1.0f), button_size + v2(128.0f, 0.0f), 32, 3, e_pos_area_flag_center_x | e_pos_area_flag_center_y);
				int highest_cost = 0;
				int highest_cost_index = 0;
				for(int choice_i = 0; choice_i < 3; choice_i += 1) {
					e_upgrade upgrade_id = choice_arr[choice_i];
					s_upgrade_data data = c_upgrade_data[upgrade_id];
					int curr_level = play_state->upgrade_level_arr[upgrade_id];
					int cost = data.base_cost * (curr_level + 1);
					if(cost > highest_cost) {
						highest_cost = cost;
						highest_cost_index = choice_i;
					}
					optional.description = get_upgrade_tooltip(upgrade_id);

					s_v2 pos = pos_area_get_advance(&area) + button_size * v2(0.5f, 0.0f);
					s_v2 text_pos = pos + v2(button_size.x * 0.5f, -font_size * 0.6f);
					s_len_str cost_str = shorten_number(cost);
					draw_text(g_r, format_text("%.*s[%c]", expand_str(cost_str), (char)(c_key_1 + choice_i)), text_pos, 0, font_size, make_color(1), true, game->font, game->ui_render_pass1);
					if(
						ui_texture_button(format_text("upgrade%i", upgrade_id), pos, game->upgrade_button_texture_arr[upgrade_id], optional) ||
						is_key_pressed(g_input, c_key_1 + choice_i)
					) {
						picked_choice = choice_i;
					}
				}

				if(game->pick_free_upgrade_automatically) {
					picked_choice = highest_cost_index;
				}

				if(picked_choice >= 0) {
					e_upgrade upgrade_id = choice_arr[picked_choice];
					play_state->upgrade_level_arr[upgrade_id] += 1;
					if(upgrade_id == e_upgrade_buy_bot) {
						make_bot(c_base_pos);
						play_sound_group(e_sound_group_buy_bot);
					}
					else {
						play_sound_group(e_sound_group_upgrade);
					}
					assert(play_state->level_up_triggers > 0);
					play_state->level_up_triggers -= 1;
					if(play_state->level_up_triggers <= 0) {
						play_state->sub_state = e_sub_state_default;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		level up menu end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// @Note(tkap, 06/10/2024): draw cells
			#if 0
			for(int y = 0; y < c_num_cells; y += 1) {
				for(int x = 0; x < c_num_cells; x += 1) {
					draw_rect(g_r, c_cells_topleft + v2(x, y) * c_cell_size, e_layer_base, v2(c_cell_size), make_color(((x+y)&1)?0.4f:0.2f), get_render_pass(e_layer_base), {}, {.origin_offset = c_origin_topleft});
				}
			}
			#endif

		} break;
		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		play state draw end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		case e_state_main_menu: {
			s_main_menu* state = &game->main_menu;

			if(state->sub_state == e_sub_state_default) {
				draw_text(g_r, strlit("Hive Havoc"), wxy(0.5f, 0.1f), 0, 128, make_color(1), true, game->font, game->ui_render_pass1);

				s_v2 button_size = v2(370, 48);
				s_ui_optional optional = zero;
				optional.size_x = button_size.x;
				optional.size_y = button_size.y;
				s_pos_area area = make_pos_area(wxy(0.0f, 0.0f), wxy(1.0f, 1.0f), button_size, 8, 2, e_pos_area_flag_center_x | e_pos_area_flag_center_y | e_pos_area_flag_vertical);
				if(ui_button(strlit("Play"), pos_area_get_advance(&area), optional) || is_key_pressed(g_input, c_key_enter)) {
					set_state_next_frame(e_state_play);
					game->reset_game = true;
				}
				if(ui_button(strlit("Leaderboard"), pos_area_get_advance(&area), optional)) {
					set_state_next_frame(e_state_leaderboard);
					if constexpr(c_are_we_on_web) {
						on_leaderboard_score_submitted();
					}
				}
				if(ui_button(strlit("Options"), pos_area_get_advance(&area), optional)) {
					state->sub_state = e_sub_state_pause;
				}
			}

			if(state->sub_state == e_sub_state_pause) {
				do_options_menu(false);
			}

		} break;

		case e_state_leaderboard: {

			do_leaderboard_stuff();

			if(
				ui_button(strlit("Back"), wxy(0.75f, 0.92f), {.size_x = c_base_button_size2.x, .size_y = c_base_button_size2.y})
				|| is_key_pressed(g_input, c_key_escape)
			) {
				go_back_to_prev_state();
			}

		} break;

		case e_state_win_leaderboard: {

			do_leaderboard_stuff();

			{
				s_time_data data = update_count_to_time_data(game->play_state.update_count_at_win_time, c_update_delay);
				s_len_str text = format_text("%02i:%02i.%03i", data.minutes, data.seconds, data.ms);
				draw_text(g_r, text, c_half_res * v2(1.0f, 0.2f), 10, 64, make_color(1), true, game->font, game->ui_render_pass1);

				draw_text(g_r, strlit("Press R to restart..."), c_half_res * v2(1.0f, 0.4f), 10, sin_range(48, 60, game->render_time * 8.0f), make_color(0.66f), true, game->font, game->ui_render_pass1);
			}

			b8 want_to_reset = is_key_pressed(g_input, c_key_r);
			if(
				ui_button(strlit("Restart"), c_base_res * v2(0.75f, 0.92f), {.size_x = c_base_button_size2.x, .size_y = c_base_button_size2.y})
				|| is_key_pressed(g_input, c_key_escape) || want_to_reset
			) {
				go_back_to_prev_state();
				game->reset_game = true;
			}

			if(ui_button(strlit("Stats (WIP)"), c_base_res * v2(0.75f, 0.86f), {.size_x = c_base_button_size2.x, .size_y = c_base_button_size2.y})) {
				set_state_next_frame(e_state_stats);
			}

			{
				s_pos_area area = make_pos_area(wxy(0.69f, 0.05f), c_base_res, v2(0, 40), 0, -1, e_pos_area_flag_vertical);
				{
					s_len_str text = format_text("Total nectar: %i", play_state->total_resource);
					draw_text(g_r, text, pos_area_get_advance(&area), 10, 40, make_color(1), false, game->font, game->ui_render_pass1);
				}

				{
					s_len_str text = format_text("Highest nectar/s: %.1f/s", play_state->highest_nectar_gain_per_second);
					draw_text(g_r, text, pos_area_get_advance(&area), 10, 40, make_color(1), false, game->font, game->ui_render_pass1);
				}

				{
					s_len_str text = format_text("Player kills: %i", play_state->num_player_kills);
					draw_text(g_r, text, pos_area_get_advance(&area), 10, 40, make_color(1), false, game->font, game->ui_render_pass1);
				}

				{
					s_len_str text = format_text("Drone kills: %i", play_state->num_bot_kills);
					draw_text(g_r, text, pos_area_get_advance(&area), 10, 40, make_color(1), false, game->font, game->ui_render_pass1);
				}
			}

		} break;

		case e_state_stats: {
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		graph start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				struct s_hovered_point
				{
					int index;
					float val;
					s_v2 pos;
				};

				int count = game->play_state.update_count / 60;
				s_v2 base_pos = wxy(0.1f, 0.9f);
				float max_val = 0;
				s_hovered_point hovered_point = zero;
				b8 hovered = false;
				float smallest_dist = 9999999;

				s_carray<s_v4, 4> color_arr = {
					make_color(1.0f, 0.450f, 0.097f), make_color(0.442f, 0.770f, 0.726f), make_color(0.469f, 0.355f, 0.609f),
					make_color(0.806f, 0.146f, 0.108f)
				};
				s_carray<void*, 4> ptr_arr = {
					game->play_state.num_player_kills_arr.elements, game->play_state.num_bot_kills_arr.elements,
					game->play_state.nectar_per_second_arr.elements, game->play_state.nectar_arr.elements
				};
				s_carray<s_len_str, 4> str_arr = {
					strlit("Player kills"), strlit("Drone kills"), strlit("Nectar per second"),
					strlit("Nectar")
				};

				// @Note(tkap, 17/10/2024): Toggle buttons
				{
					s_pos_area area = make_horizontal_layout(wxy(0.0f, 0.95f), c_base_button_size, 8, 0);
					for(int i = 0; i < 4; i += 1) {
						s_ui_optional optional = zero;
						optional.font_size = 20;
						if(!game->statistics_show_arr[i]) {
							optional.darken = 0.5f;
						}
						if(ui_button(str_arr[i], pos_area_get_advance(&area), optional)) {
							game->statistics_show_arr[i] = !game->statistics_show_arr[i];
						}
					}

					pos_area_get_advance(&area);
					if(ui_button(strlit("Back"), pos_area_get_advance(&area), {.font_size = 20})) {
						go_back_to_prev_state();
					}
				}

				// @Note(tkap, 17/10/2024): Figure out max value
				for(int i = 0; i < ptr_arr.max_elements(); i += 1) {
					if(!game->statistics_show_arr[i]) { continue; }
					for(int point_i = 0; point_i < count; point_i += 1) {
						float curr_val = 0;
						if(i == 2) {
							float* ptr = (float*)ptr_arr[i];
							curr_val = ptr[point_i];
						}
						else {
							int* ptr = (int*)ptr_arr[i];
							curr_val = (float)ptr[point_i];
						}
						max_val = max(max_val, curr_val);
					}
				}

				for(int i = 0; i < ptr_arr.max_elements(); i += 1) {
					if(!game->statistics_show_arr[i]) { continue; }
					s_v2 prev_pos = base_pos;
					for(int point_i = 0; point_i < count; point_i += 1) {
						float curr_val = 0;
						if(i == 2) {
							float* ptr = (float*)ptr_arr[i];
							curr_val = ptr[point_i];
						}
						else {
							int* ptr = (int*)ptr_arr[i];
							curr_val = (float)ptr[point_i];
						}
						// @TODO(tkap, 16/10/2024): handle 0 kills
						float p0 = index_count_safe_div(point_i, count);
						float p1 = curr_val / max_val;
						p0 *= 0.8f;
						p1 *= 0.8f;
						s_v2 pos = base_pos;
						pos.x += p0 * c_base_res.x;
						pos.y -= c_base_res.y * p1;

						draw_line(g_r, prev_pos, pos, 0, 2, color_arr[i], game->ui_render_pass0);

						float dist = 0;
						if(point_vs_line(g_mouse, prev_pos, pos, 16, &dist)) {
							if(dist < smallest_dist) {
								smallest_dist = dist;
								hovered_point = {.index = i, .val = curr_val, .pos = pos};
								hovered = true;
							}
						}
						prev_pos = pos;
					}
				}
				if(hovered) {
					s_len_str str = format_text("%.*s: %.0f", expand_str(str_arr[hovered_point.index]), hovered_point.val);
					s_v2 text_size = get_text_size(str, game->font, 32);
					s_v2 pos = hovered_point.pos + v2(0, 8);
					draw_circle(g_r, hovered_point.pos, 0, 4, make_color(0.486f, 0.848f, 0.895f), game->ui_render_pass2);
					s_rectf rect = fit_rect(pos, text_size, c_base_res_bounds);
					draw_text(g_r, str, rect.pos, 10, 32, make_color(1), false, game->font, game->ui_render_pass3);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		graph end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

			draw_text(g_r, strlit("Victory!"), c_base_res * v2(0.5f, 0.1f), 10, font_size, make_color(1), true, game->font, game->ui_render_pass1);
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
			float extra_height = ease_out_elastic2_advanced(t2, 0, 0.75f, 20, 0);
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

	g_r->end_render_pass(g_r, game->ui_render_pass0, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});
	g_r->end_render_pass(g_r, game->ui_render_pass1, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});
	g_r->end_render_pass(g_r, game->ui_render_pass2, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});
	g_r->end_render_pass(g_r, game->ui_render_pass3, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});

	g_r->clear_framebuffer(g_r->default_fbo, zero, c_default_fbo_clear_flags);
	draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->main_fbo, game->world_render_pass_arr[0]);
	g_r->end_render_pass(g_r, game->world_render_pass_arr[0], g_r->default_fbo, {.projection = ortho});

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		reset ui start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		auto t = &game->ui_table;
		for(int i = 0; i < game->ui_table.max_elements(); i += 1) {
			if(t->used[i] && !t->values[i].present) {
				t->used[i] = false;
			}
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		reset ui end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

}

#ifdef m_build_dll
}
#endif // m_build_dll

func s_v2i pos_to_index(s_v2 pos, int tile_size)
{
	s_v2i result;
	result.x = floorfi(pos.x / tile_size);
	result.y = floorfi(pos.y / tile_size);
	return result;
}

func s_v2 index_to_pos(s_v2i index, int tile_size)
{
	return v2(index) * v2(tile_size);
}


s_m4 s_camera2d::get_matrix()
{
	s_m4 m = m4_identity();
	m = m4_multiply(m, m4_translate(v3(offset.x, offset.y, 0)));
	m = m4_multiply(m, m4_scale(v3(zoom, zoom, 1)));
	m = m4_multiply(m, m4_translate(v3(-pos.x, -pos.y, 0)));
	return m;
}

func void do_particles(int count, s_v2 pos, int z, b8 attached_to_player, s_particle_data data)
{
	s_rng* rng = &game->rng;
	for(int particle_i = 0; particle_i < count; particle_i++) {
		s_particle p = {};
		p.attached_to_player = attached_to_player;
		if(!attached_to_player) {
			p.pos = pos;
		}
		p.z = z;
		p.fade = data.fade;
		p.shrink = data.shrink;
		p.duration = data.duration * (1.0f - rng->randf32() * data.duration_rand);
		float random_angle = rng->randf_range(-pi, pi) * data.angle_rand;
		p.dir = v2_from_angle(data.angle + random_angle);
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

func void on_leaderboard_received(s_json* json)
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

func void on_our_leaderboard_received(s_json* json)
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

func void on_leaderboard_score_submitted()
{
	g_platform_data->get_leaderboard(c_leaderboard_id, on_leaderboard_received);
}

func s_m4 get_camera_view(s_camera3d cam)
{
	return look_at(cam.pos, cam.pos + cam.target, v3(0, -1, 0));
}

func s_button_interaction ui_button_interaction(s_len_str id_str, s_v2 pos, s_ui_optional optional)
{
	s_button_interaction result = zero;

	result.id = parse_ui_id(id_str);
	result.data = get_or_create_ui_data(result.id.id);
	result.data->present = true;

	result.size = c_base_button_size;
	if(optional.size_x > 0) {
		result.size.x = optional.size_x;
	}
	if(optional.size_y > 0) {
		result.size.y = optional.size_y;
	}

	result.hovered = mouse_collides_rect_topleft(g_mouse, pos, result.size);

	if(result.hovered && is_mouse_clicked()) {
		game->click_consumed = true;
		result.clicked = true;

		s_ui_iterator it = zero;
		while(for_ui_data(&it)) {
			if(it.element != result.data) {
				it.element->asking_for_confirmation = false;
			}
		}
	}

	return result;
}

func b8 ui_button(s_len_str id_str, s_v2 pos, s_ui_optional optional)
{
	s_button_interaction interaction = ui_button_interaction(id_str, pos, optional);

	float font_size = 48;
	if(optional.font_size > 0) {
		font_size = optional.font_size;
	}

	s_v4 color = make_color(0.6f);

	if(interaction.hovered) {
		color = make_color(1);
	}
	float color_multi = optional.darken;
	draw_texture(g_r, pos, 0, interaction.size, brighter(color, color_multi), game->button_texture, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});

	{
		s_v2 text_pos = center_text_on_rect(interaction.id.text, game->font, pos, interaction.size, font_size, true, true);
		text_pos.y += font_size * 0.1f;
		draw_text(g_r, interaction.id.text, text_pos, 1, font_size, make_color(color_multi), false, game->font, game->ui_render_pass1);
	}

	if(interaction.hovered && optional.description.len > 0) {
		float temp_font_size = font_size * 1.6f;
		if(optional.tooltip_font_size > 0) {
			temp_font_size = optional.tooltip_font_size;
		}
		do_button_tooltip(optional.description, temp_font_size);
	}

	return interaction.clicked;
}

func b8 ui_texture_button(s_len_str id_str, s_v2 pos, s_texture texture, s_ui_optional optional)
{
	s_button_interaction interaction = ui_button_interaction(id_str, pos, optional);

	float font_size = 48;
	if(optional.font_size > 0) {
		font_size = optional.font_size;
	}

	s_v4 color = make_color(0.7f);

	if(interaction.hovered) {
		color = make_color(1);
	}
	float color_multi = optional.darken;
	draw_texture(g_r, pos, 0, interaction.size, brighter(color, color_multi), texture, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});

	if(interaction.hovered && optional.description.len > 0) {
		float temp_font_size = font_size * 1.6f;
		if(optional.tooltip_font_size > 0) {
			temp_font_size = optional.tooltip_font_size;
		}
		do_button_tooltip(optional.description, temp_font_size);
	}

	return interaction.clicked;
}

func void do_button_tooltip(s_len_str description, float font_size)
{
	s_v2 text_size = get_text_size(description, game->font, font_size);
	float padding = 16;
	s_v2 panel_size = text_size + v2(padding * 2);
	s_v2 panel_pos = g_mouse - v2(0.0f, panel_size.y);
	s_rectf panel = fit_rect(panel_pos, panel_size, c_base_res_bounds);
	s_v2 text_pos = panel.pos + v2(padding);
	text_pos.y += 4;
	draw_rect(g_r, panel.pos, 0, panel.size, hex_rgb_plus_alpha(0x9E8642, 0.85f), game->ui_render_pass2, {}, {.origin_offset = c_origin_topleft});
	draw_text(g_r, description, text_pos, 0, font_size, make_color(1), false, game->font, game->ui_render_pass3);
}

func b8 ui_button_with_confirmation(s_len_str id_str, s_len_str confirmation_str, s_v2 pos, s_ui_optional optional)
{
	b8 result = false;
	s_parse_ui_id id = parse_ui_id(id_str);
	s_ui_data* data = get_or_create_ui_data(id.id);
	if(data->asking_for_confirmation) {
		s_len_str str = format_text("%.*s##%.*s", expand_str(confirmation_str), expand_str(id_str));
		result = ui_button(str, pos, optional);
		if(result) {
			data->asking_for_confirmation = false;
		}
	}
	else {
		b8 button_result = ui_button(id_str, pos, optional);
		if(button_result) {
			data->asking_for_confirmation = true;
		}
	}
	return result;
}

func void on_set_leaderboard_name(b8 success)
{
	if(success) {
		set_state_next_frame(e_state_win_leaderboard);
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
	return c_invalid_entity;
}

func int make_creature(s_v2 pos, int tier, b8 boss)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	int entity = make_entity(creature_arr->active, creature_arr->id, &creature_arr->index_data, c_max_creatures);
	if(entity >= 0) {
		e_creature type = e_creature_ant;
		creature_arr->type[entity] = type;
		creature_arr->pos[entity] = pos;
		creature_arr->prev_pos[entity] = pos;
		creature_arr->target_pos[entity] = pos;
		creature_arr->tier[entity] = tier;
		creature_arr->boss[entity] = boss;
		creature_arr->roam_timer[entity] = 0;
		creature_arr->targeted[entity] = false;
		creature_arr->animation_timer[entity] = 0;
		creature_arr->tick_when_last_damaged[entity] = -10000;
		creature_arr->curr_health[entity] = get_creature_max_health(type, tier, boss);
	}
	return entity;
}

func int make_deposit(s_v2 pos, int tier)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	int entity = make_entity(creature_arr->active, creature_arr->id, &creature_arr->index_data, c_max_creatures);
	if(entity >= 0) {
		e_creature type = e_creature_deposit;
		creature_arr->type[entity] = type;
		creature_arr->pos[entity] = pos;
		creature_arr->prev_pos[entity] = pos;
		creature_arr->tier[entity] = tier;
		creature_arr->boss[entity] = false;
		creature_arr->targeted[entity] = false;
		creature_arr->tick_when_last_damaged[entity] = -10000;
		creature_arr->flip_x[entity] = false;
		creature_arr->curr_health[entity] = get_creature_max_health(type, tier, false);
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
	bot_arr->cargo[entity] = 0;
	bot_arr->cargo_count[entity] = 0;
	return entity;
}


func void pick_target_for_bot(int bot)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	s_bot_arr* bot_arr = &game->play_state.bot_arr;
	assert(bot_arr->active[bot]);

	s_get_closest_creature data = get_closest_creature(bot_arr->pos[bot]);

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
	if(index.id <= 0) { return c_invalid_entity; }
	if(!game->play_state.creature_arr.active[index.index]) { return c_invalid_entity; }

	if(game->play_state.creature_arr.id[index.index] == index.id) { return index.index; }
	return c_invalid_entity;
}

func s_damage_creature damage_creature(int creature, int damage)
{
	s_damage_creature result = zero;
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	e_creature type = creature_arr->type[creature];
	int tier = creature_arr->tier[creature];
	b8 is_boss = creature_arr->boss[creature];
	b8 is_deposit = type == e_creature_deposit;

	if(is_deposit) {
		constexpr int step = 10;
		int health_after_damage = at_least(0, creature_arr->curr_health[creature] - damage);
		int prev_threshold = (get_creature_max_health(type, tier, is_boss) - creature_arr->curr_health[creature]) / step;
		int next_threshold = (get_creature_max_health(type, tier, is_boss) - health_after_damage) / step;
		int resource_from_deposit = next_threshold - prev_threshold;
		int double_val = game->play_state.upgrade_level_arr[e_upgrade_double_harvest] > 0 ? 2 : 1;
		result.resource_gain_from_deposit = resource_from_deposit * double_val;
	}

	creature_arr->curr_health[creature] -= damage;
	creature_arr->tick_when_last_damaged[creature] = game->play_state.update_count;
	if(creature_arr->curr_health[creature] <= 0) {
		remove_entity(creature, creature_arr->active, &creature_arr->index_data);
		play_sound_group(e_sound_group_creature_death);

		do_particles(64, creature_arr->pos[creature], e_layer_particle, false, {
			.shrink = 3,
			.slowdown = 4,
			.duration = 1.33f,
			.duration_rand = 1,
			.speed = 256,
			.speed_rand = 1,
			.radius = 16,
			.color = v3(0.2f, 0.1f, 0.1f),
			.color_rand = v3(0.1f, 0.1f, 0.1f),
		});

		if(!is_deposit) {
			float chance = 1;
			if(is_boss) {
				chance = 10;
			}
			if(game->rng.chance100(chance)) {
				make_pickup(creature_arr->pos[creature], (e_pickup)game->play_state.next_pickup_to_drop);
				circular_index_add(&game->play_state.next_pickup_to_drop, 1, e_pickup_count);
			}

			int level_up_count = add_exp(&game->play_state.player, get_creature_exp_reward(tier, is_boss));
			game->play_state.level_up_triggers += level_up_count;
		}
		result.creature_died = true;

	}
	return result;
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

func int get_closest_creature2(s_v2 pos, float radius, s_cells* cells, s_lin_arena* arena, s_sarray<int, c_max_player_hits> blacklist)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;

	float smallest_dist = 99999999.0f;
	int closest_creature = -1;
	s_dynamic_array<int> query_arr = query_creatures_circle(pos, radius, cells, arena);
	foreach_val(query_i, query, query_arr) {
		assert(creature_arr->active[query]);
		if(blacklist.contains(query)) { continue; }
		float dist = v2_distance(pos, creature_arr->pos[query]);
		if(dist < smallest_dist) {
			smallest_dist = dist;
			closest_creature = query;
		}
	}
	return closest_creature;
}

func s_dynamic_array<int> query_creatures_circle(s_v2 pos, float radius, s_cells* cells, s_lin_arena* frame_arena)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	s_dynamic_array<int> result = make_dynamic_array<int>(16, frame_arena);
	s_v2i min_index = get_cell_index(pos - v2(radius));
	s_v2i max_index = get_cell_index(pos + v2(radius));

	for(int y = min_index.y; y <= max_index.y; y += 1) {
		for(int x = min_index.x; x <= max_index.x; x += 1) {
			if(!is_valid_index(x, y, c_num_cells, c_num_cells)) { continue; }
			if(cells->cell_arr[y][x].count > 0) {
				foreach_val(creature_i, creature, cells->cell_arr[y][x]) {
					if(!creature_arr->active[creature]) { continue; }
					if(rect_collides_circle_center(creature_arr->pos[creature], get_creature_size(creature), pos, radius)) {
						result.add(creature, frame_arena);
					}
				}
			}
		}
	}
	return result;
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
		area.advance.y = element_size.y + spacing;
	}

	return area;
}

func s_pos_area make_horizontal_layout(s_v2 pos, s_v2 element_size, float spacing, int flags)
{
	s_pos_area result = make_pos_area(pos, v2(0), element_size, spacing, -1, flags);
	return result;
}

func s_pos_area make_vertical_layout(s_v2 pos, s_v2 element_size, float spacing, int flags)
{
	flags |= e_pos_area_flag_vertical;
	s_pos_area result = make_pos_area(pos, c_base_res, element_size, spacing, -1, flags);
	return result;
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
	float result = c_player_movement_speed + game->play_state.upgrade_level_arr[e_upgrade_player_movement_speed];
	return result;
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
	int result = game->play_state.upgrade_level_arr[e_upgrade_creature_tier];
	if(game->play_state.upgrade_level_arr[e_upgrade_double_harvest] > 0) {
		result -= 5;
	}
	result = at_least(0, result);
	return result;
}

func float get_player_harvest_range()
{
	float result = c_player_harvest_range + game->play_state.upgrade_level_arr[e_upgrade_player_harvest_range] * 35;
	if(has_buff(e_pickup_chain_and_range)) {
		result += 100;
	}
	if(has_buff(e_pickup_multi_target_and_range)) {
		result += 100;
	}
	return result;
}

func float get_bot_harvest_range()
{
	float result = c_bot_harvest_range + game->play_state.upgrade_level_arr[e_upgrade_bot_harvest_range] * 25;
	if(has_buff(e_pickup_bot_chain_and_range)) {
		result += 100;
	}
	return result;
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

func int get_creature_exp_reward(int tier, b8 boss)
{
	int result = tier + 1;
	if(boss) {
		result *= 11;
	}
	return result;
}

func b8 set_state_next_frame(e_state new_state)
{
	if(game->next_state >= 0) { return false; }

	switch(new_state)	{
		case e_state_leaderboard:
		case e_state_win_leaderboard:
		{
			game->leaderboard_state = zero;
			game->leaderboard_arr.count = 0;
		} break;
	}

	game->next_state = new_state;
	return true;
}

func void set_state_next_frame_temporary(e_state new_state)
{
	if(set_state_next_frame(new_state)) {
		game->next_state_is_temporary = true;
	}
}

func int count_alive_creatures()
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	int result = 0;
	for_creature_partial(creature) {
		if(!creature_arr->active[creature]) { continue; }
		if(counts_towards_defeat(creature_arr->type[creature])) {
			result += 1;
		}
	}
	return result;
}

func void draw_light(s_v2 pos, float radius, s_v4 color, float smoothness)
{
	draw_circle(g_r, pos, 0, radius, color, game->world_render_pass_arr[0], {.shader = 5, .circle_smoothness = smoothness});
}

func void draw_shadow(s_v2 pos, float radius, float strength, float smoothness)
{
	draw_circle(g_r, pos, e_layer_shadow, radius, make_color(strength), game->world_render_pass_arr[0], {.shader = 5, .circle_smoothness = smoothness});
}

func int get_bot_max_cargo_count()
{
	return 1 + game->play_state.upgrade_level_arr[e_upgrade_bot_cargo_count];
}

func s_v2 get_creature_size(int creature)
{
	float multi = get_creature_size_multi(creature);
	return c_creature_size * multi;
}

func float get_creature_size_multi(int creature)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	e_creature type = creature_arr->type[creature];

	switch(type) {
		case e_creature_ant: {
			float multi = creature_arr->boss[creature] ? 3.0f : 1.0f;
			return multi;
		} break;

		case e_creature_deposit: {
			return 2;
		} break;

		invalid_default_case;
	}
	return 1;
}

func s_entity_index creature_to_entity_index(int creature)
{
	s_creature_arr* creature_arr = &game->play_state.creature_arr;
	assert(creature_arr->active[creature]);
	return {.index = creature, .id = creature_arr->id[creature]};
}

func s_v2i get_cell_index(s_v2 pos)
{
	int x_index = floorfi((pos.x - c_cells_topleft.x) / c_cell_size);
	int y_index = floorfi((pos.y - c_cells_topleft.y) / c_cell_size);
	return v2i(x_index, y_index);
}

func s_bounds get_map_bounds()
{
	s_bounds bounds = zero;
	bounds.min_x = c_base_pos.x - c_cell_area * 0.5f;
	bounds.min_y = c_base_pos.y - c_cell_area * 0.5f;
	bounds.max_x = c_base_pos.x + c_cell_area * 0.5f;
	bounds.max_y = c_base_pos.y + c_cell_area * 0.5f;
	bounds.max_x -= 1;
	bounds.max_y -= 1;
	return bounds;
}

func s_bounds get_cam_bounds(s_camera2d cam)
{
	s_bounds bounds = zero;
	bounds.min_x = cam.pos.x - cam.offset.x / cam.zoom;
	bounds.min_y = cam.pos.y - cam.offset.y / cam.zoom;
	bounds.max_x = cam.pos.x + (c_base_res.x - cam.offset.x) / cam.zoom;
	bounds.max_y = cam.pos.y + (c_base_res.y - cam.offset.y) / cam.zoom;
	return bounds;
}

func s_bounds get_cam_bounds_snap_to_tile_size(s_camera2d cam)
{
	s_bounds bounds = zero;
	bounds.min_x = cam.pos.x - cam.offset.x / cam.zoom;
	bounds.min_y = cam.pos.y - cam.offset.y / cam.zoom;
	bounds.max_x = cam.pos.x + (c_base_res.x - cam.offset.x) / cam.zoom;
	bounds.max_y = cam.pos.y + (c_base_res.y - cam.offset.y) / cam.zoom;

	float x_diff = fmodf(bounds.min_x, c_tile_size);
	float y_diff = fmodf(bounds.min_y, c_tile_size);
	if(x_diff < 0) {
		x_diff = c_tile_size + x_diff;
	}
	if(y_diff < 0) {
		y_diff = c_tile_size + y_diff;
	}
	bounds.min_x -= x_diff;
	bounds.max_x -= x_diff;
	bounds.min_y -= y_diff;
	bounds.max_y -= y_diff;
	return bounds;
}

func int get_player_hits()
{
	int result = 1 + game->play_state.upgrade_level_arr[e_upgrade_player_chain];
	if(has_buff(e_pickup_chain_and_range)) {
		result += 4;
	}
	return at_most(c_max_player_hits, result);
}

func int get_bot_hits()
{
	int result = 1;
	if(has_buff(e_pickup_bot_chain_and_range)) {
		result += 4;
	}
	return at_most(c_max_bot_hits, result);
}

func void make_pickup(s_v2 pos, e_pickup type)
{
	s_pickup pickup = zero;
	pickup.pos = pos;
	pickup.type = type;
	game->play_state.pickup_arr.add_checked(pickup);
}

func void add_buff(s_player* player, e_pickup pickup)
{
	at_least_ptr(0, &player->buff_arr[pickup].ticks_left);
	player->buff_arr[pickup].ticks_left += 500;
}

func b8 has_buff(e_pickup type)
{
	return game->play_state.player.buff_arr[type].ticks_left > 0;
}

func s_particle_data multiply_particle_data(s_particle_data data, s_particle_multiplier multi)
{
	data.radius *= multi.radius;
	data.speed *= multi.speed;
	return data;
}

func int get_player_multi_target()
{
	int result = 1;
	if(has_buff(e_pickup_multi_target_and_range)) {
		result += 4;
	}
	return result;
}

func void play_sound_group(e_sound_group group_id)
{
	assert(group_id >= 0);
	assert(group_id < e_sound_group_count);

	if(game->sound_disabled) { return; }

	float* t = &g_sound_group_last_play_time_arr[group_id];
	s_sound_group_data data = c_sound_group_data_arr[group_id];
	float passed = game->render_time - *t;
	if(passed > data.cooldown) {
		*t = game->render_time;
		e_sound sound_id = data.sound_arr[game->rng.randu() % data.sound_count];
		assert(sound_id >= 0);
		assert(sound_id < e_sound_count);
		s_sound* to_play = game->sound_arr[sound_id];
		g_platform_data->play_sound(to_play);
	}

}

func int count_alive_bots()
{
	int result = 0;
	for_bot_partial(bot) {
		if(game->play_state.bot_arr.active[bot]) { result += 1; }
	}
	return result;
}

func s_len_str get_upgrade_tooltip(e_upgrade id)
{
	s_len_str result = zero;
	int level = game->play_state.upgrade_level_arr[id];
	switch(id) {
		case e_upgrade_buy_bot: {
			result = format_text("+1 drone\n\nCurrent: %i", count_alive_bots());
		} break;

		case e_upgrade_player_damage: {
			result = format_text("+1 player damage\n\nCurrent: %i", get_player_damage());
		} break;

		case e_upgrade_bot_damage: {
			result = format_text("+1 drone damage\n\nCurrent: %i", get_bot_damage());
		} break;

		case e_upgrade_player_movement_speed: {
			result = format_text("+1 player movement speed\n\nCurrent: %.1f", get_player_movement_speed());
		} break;

		case e_upgrade_bot_movement_speed: {
			result = format_text("+1 drone movement speed\n\nCurrent: %.1f", get_bot_movement_speed());
		} break;

		case e_upgrade_spawn_rate: {
			int val = game->play_state.upgrade_level_arr[e_upgrade_spawn_rate] * 15;
			result = format_text("Creatures spawn 15%% faster\n\nCurrent: %i%%", val);
		} break;

		case e_upgrade_creature_tier: {
			int val = get_creature_spawn_tier();
			result = format_text("Creatures are stronger and more rewarding\n\nCurrent: %i", val);
		} break;

		case e_upgrade_player_harvest_range: {
			result = format_text("+35 player harvest range\n\nCurrent: %.0f", get_player_harvest_range());
		} break;

		case e_upgrade_bot_harvest_range: {
			result = format_text("+25 drone harvest range\n\nCurrent: %.0f", get_bot_harvest_range());
		} break;

		case e_upgrade_double_harvest: {
			result = format_text("Gain double nectar from harvesting\n-5 creature tier");
		} break;

		case e_upgrade_bot_cargo_count: {
			result = format_text("Drones can harvest more creatures\nbefore having to return to the hive\n\nCurrent: %i", get_bot_max_cargo_count());
		} break;

		case e_upgrade_player_chain: {
			result = format_text("Player attack chains to nearby enemies\n\nCurrent: %i", get_player_hits() - 1);
		} break;

		case e_upgrade_broken_bot_spawn: {
			float rate = game->play_state.spawn_broken_bot_timer.get_rate_in_seconds();
			if(level <= 0) {
				rate = 0;
			}
			result = format_text("Periodically spawn broken drones\nCurrent: %0.2f/s", rate);
		} break;

		case e_upgrade_deposit_spawn_rate: {
			float rate = game->play_state.spawn_deposit_timer.get_rate_in_seconds();
			result = format_text("Nectar deposits spawn %.0f%% faster\nCurrent: %0.2f/s", c_deposit_spawn_rate_buff_per_upgrade, rate);
		} break;

		case e_upgrade_deposit_health: {
			result = format_text("Nectar deposits contain %i%% increased nectar\nCurrent: %i%%", c_deposit_health_multi_per_upgrade, level * c_deposit_health_multi_per_upgrade);
		} break;

		case e_upgrade_dash_cooldown: {
			result = format_text("%i%% faster dash cooldown\nCurrent: %i%%", c_dash_cooldown_speed_per_upgrade, c_dash_cooldown_speed_per_upgrade * level);
		} break;

		invalid_default_case;

	}
	return result;
}

func s_v2 wxy(float x, float y)
{
	return c_base_res * v2(x, y);
}

func s64 get_required_exp_to_level(int level)
{
	int level_minus_one = level - 1;
	s64 result = 5 + floorfi(0.4f * level_minus_one * level) + (level_minus_one) * 5;
	return result;
}

func int add_exp(s_player* player, int to_add)
{
	int level_up_count_result = 0;
	s64 exp_to_level = get_required_exp_to_level(player->curr_level);
	player->curr_exp += to_add;
	while(player->curr_exp >= exp_to_level) {
		player->curr_exp -= exp_to_level;
		player->curr_level += 1;
		exp_to_level = get_required_exp_to_level(player->curr_level);
		level_up_count_result += 1;
	}
	return level_up_count_result;
}

func b8 game_is_paused()
{
	e_sub_state s = game->play_state.sub_state;
	return s == e_sub_state_pause || s == e_sub_state_defeat || s == e_sub_state_level_up;
}

func b8 can_pause()
{
	e_sub_state s = game->play_state.sub_state;
	return s == e_sub_state_pause || s == e_sub_state_default;
}

func b8 can_lose()
{
	e_sub_state s = game->play_state.sub_state;
	return s != e_sub_state_winning;
}

func b8 can_go_to_level_up_state()
{
	e_sub_state s = game->play_state.sub_state;
	return s == e_sub_state_default;
}

func b8 should_show_ui()
{
	e_sub_state s = game->play_state.sub_state;
	return s == e_sub_state_default && s != e_sub_state_winning;
}

func int pick_weighted(f64* arr, int count, s_rng* rng)
{
	assert(count > 0);

	f64 total_weight = 0;
	for(int i = 0; i < count; i += 1) {
		total_weight += arr[i];
	}
	for(int i = 0; i < count; i += 1) {
		f64 roll = rng->randf64() * total_weight;
		if(roll <= arr[i]) {
			return i;
		}
		total_weight -= arr[i];
	}
	assert(false);
	return -1;
}

func float ticks_to_seconds(int ticks)
{
	float result = ticks / (float)c_updates_per_second;
	return result;
}

func s_carray<float, 3> ticks_to_seconds2(int ticks, float interp_dt)
{
	s_carray<float, 3> result;
	result[0] = ticks_to_seconds(ticks - 1);
	result[2] = ticks_to_seconds(ticks);
	result[1] = lerp(result[0], result[2], interp_dt);
	return result;
}

func void draw_laser(s_laser_target target, float laser_light_radius, s_v4 laser_color, float interp_dt)
{
	s_v2 from_pos = lerp(target.from.prev_pos, target.from.pos, interp_dt);
	s_v2 to_pos = lerp(target.to.prev_pos, target.to.pos, interp_dt);
	draw_line(g_r, from_pos, to_pos, e_layer_laser, c_laser_width, laser_color, game->world_render_pass_arr[1], {}, {.effect_id = 5});
	draw_light(to_pos, laser_light_radius * 1.5f, laser_color, 0.0f);
}

func void do_options_menu(b8 in_play_mode)
{
	s_v2 button_size = c_base_button_size2;
	button_size.x += 220;
	button_size.y += 12;
	s_ui_optional optional = zero;
	s_play_state* play_state = &game->play_state;
	int button_count = in_play_mode ? 10 : 8;
	optional.size_x = button_size.x;
	optional.size_y = button_size.y;

	s_pos_area area = make_pos_area(wxy(0.0f, 0.4f), wxy(1.0f, 0.2f), button_size, 8, button_count, e_pos_area_flag_center_x | e_pos_area_flag_center_y | e_pos_area_flag_vertical);
	if(in_play_mode && ui_button(strlit("Resume"), pos_area_get_advance(&area), optional)) {
		play_state->sub_state = e_sub_state_default;
	}
	if(ui_button(strlit("Leaderboard"), pos_area_get_advance(&area), optional)) {
		set_state_next_frame(e_state_leaderboard);
		if constexpr(c_are_we_on_web) {
			on_leaderboard_score_submitted();
		}
	}
	if(ui_button(format_text("Sounds: %s", game->sound_disabled ? "Off" : "On"), pos_area_get_advance(&area), optional)) {
		game->sound_disabled = !game->sound_disabled;
	}
	if(ui_button(format_text("Dash to mouse: %s", game->dash_to_keyboard ? "Off" : "On"), pos_area_get_advance(&area), optional)) {
		game->dash_to_keyboard = !game->dash_to_keyboard;
	}
	if(ui_button(format_text("Smooth camera: %s", game->do_instant_camera ? "Off" : "On"), pos_area_get_advance(&area), optional)) {
		game->do_instant_camera = !game->do_instant_camera;
	}
	if(ui_button(format_text("Timer: %s", game->hide_timer ? "Off" : "On"), pos_area_get_advance(&area), optional)) {
		game->hide_timer = !game->hide_timer;
	}
	if(ui_button(format_text("Tutorial: %s", game->hide_tutorial ? "Off" : "On"), pos_area_get_advance(&area), optional)) {
		game->hide_tutorial = !game->hide_tutorial;
	}
	if(ui_button(format_text("Auto level: %s", game->pick_free_upgrade_automatically ? "On" : "Off"), pos_area_get_advance(&area), optional)) {
		game->pick_free_upgrade_automatically = !game->pick_free_upgrade_automatically;
	}
	if(in_play_mode && ui_button_with_confirmation(strlit("Restart"), strlit("Are you sure?"), pos_area_get_advance(&area), optional)) {
		game->reset_game = true;
	}
	if(
		!in_play_mode && (ui_button(strlit("Back"), pos_area_get_advance(&area), optional) || is_key_pressed(g_input, c_key_escape))
	) {
		game->main_menu.sub_state = e_sub_state_default;
	}
	if(in_play_mode && ui_button_with_confirmation(strlit("Exit"), strlit("Are you sure?"), pos_area_get_advance(&area), optional)) {
		go_back_to_prev_state();
	}

}

func e_state get_state()
{
	return game->state_stack.get_last().state;
}

func void go_back_to_prev_state()
{
	s_state state = zero;
	int index = game->state_stack.count - 2;
	while(true) {
		assert(index >= 0);
		state = game->state_stack[index];
		if(!state.is_temporary) { break; }
		index -= 1;
	}
	set_state_next_frame(state.state);
	game->should_pop_state = true;
}

func void add_resource(int amount)
{
	game->play_state.resource_count += amount;
	game->play_state.total_resource += amount;

	int index = game->play_state.update_count % c_nectar_gain_num_updates;
	game->play_state.nectar_gain_arr[index] += amount;
}

func b8 is_mouse_clicked()
{
	return !game->click_consumed && is_key_pressed(g_input, c_left_mouse);
}

func s_ui_data* get_or_create_ui_data(u32 id)
{
	s_ui_data* data = game->ui_table.get(id);
	if(!data) {
		data = game->ui_table.set(id, zero);
		*data = zero;
	}
	return data;
}

func bool for_ui_data(s_ui_iterator* it)
{
	auto t = &game->ui_table;
	for(int i = it->index; i < t->max_elements(); i += 1) {
		it->index = i + 1;
		if(t->used[i]) {
			it->element = &t->values[i];
			return true;
		}
	}
	return false;
}

func s_v2 get_random_creature_spawn_pos()
{
	float angle = game->rng.randf_range(0, tau);
	float dist = game->rng.randf_range(c_base_size.x * 2.0f, c_base_size.x * 4.0f);
	s_v2 offset = v2(
		cosf(angle) * dist,
		sinf(angle) * dist
	);
	s_v2 pos = c_base_pos + offset;
	pos = constrain_pos(pos, get_map_bounds());
	return pos;
}

func b8 counts_towards_defeat(e_creature type)
{
	switch(type) {
		case e_creature_ant: {
			return true;
		} break;

		case e_creature_deposit: {
			return false;
		} break;

		invalid_default_case;
	}
	return false;
}

func b8 can_creature_move(e_creature type)
{
	switch(type) {
		case e_creature_ant: {
			return true;
		} break;

		case e_creature_deposit: {
			return false;
		} break;

		invalid_default_case;
	}
	return false;
}

func int get_creature_max_health(e_creature type, int tier, b8 is_boss)
{
	int result = 0;

	switch(type) {
		case e_creature_ant: {
			result = 20 * (tier + 1);
			if(is_boss) {
				result *= 10;
			}
		} break;

		case e_creature_deposit: {
			result = floorfi(400 * (tier + 1) * get_multiplier(game->play_state.upgrade_level_arr[e_upgrade_deposit_health], (float)c_deposit_health_multi_per_upgrade));
		} break;

		invalid_default_case;
	}
	return result;
}

func float get_nectar_per_second()
{
	float result = 0;
	for(int i = 0; i < c_nectar_gain_num_updates; i += 1) {
		result += game->play_state.nectar_gain_arr[i];
	}
	float ratio = c_nectar_gain_num_updates / (float)c_updates_per_second;
	result /= ratio;
	return result;
}

int s_auto_timer::tick()
{
	assert(curr >= 0);
	assert(duration > 0);

	int result = 0;
	curr += (float)c_update_delay * speed;
	while(curr >= duration) {
		result += 1;
		curr -= duration;
	}
	return result;
}

float s_auto_timer::get_rate_in_seconds()
{
	float result = 1.0f / (duration / speed);
	return result;
}

func s_auto_timer make_auto_timer(float curr, float duration)
{
	assert(curr >= 0);
	assert(duration > 0);

	return {
		.curr = curr,
		.duration = duration,
	};
}

func s_carray<s_v2, 8> get_broken_bot_pos_arr(s_rng* rng)
{
	s_carray<s_v2, 8> result;
	s_bounds bounds = get_map_bounds();

	// @Note(tkap, 06/10/2024): Left
	for(int i = 0; i < 2; i += 1) {
		s_v2 pos = v2(
			rng->randf_range(bounds.min_x, bounds.min_x + 500),
			rng->randf_range(bounds.min_y, bounds.max_y)
		);
		result[i] = pos;
	}
	// @Note(tkap, 06/10/2024): Right
	for(int i = 0; i < 2; i += 1) {
		s_v2 pos = v2(
			rng->randf_range(bounds.max_x - 500, bounds.max_x),
			rng->randf_range(bounds.min_y, bounds.max_y)
		);
		result[i + 2] = pos;
	}
	// @Note(tkap, 06/10/2024): Top
	for(int i = 0; i < 2; i += 1) {
		s_v2 pos = v2(
			rng->randf_range(bounds.min_x, bounds.max_x),
			rng->randf_range(bounds.min_y, bounds.min_y + 500)
		);
		result[i + 4] = pos;
	}
	// @Note(tkap, 06/10/2024): Bottom
	for(int i = 0; i < 2; i += 1) {
		s_v2 pos = v2(
			rng->randf_range(bounds.min_x, bounds.max_x),
			rng->randf_range(bounds.max_y - 500, bounds.max_y)
		);
		result[i + 6] = pos;
	}

	return result;
}

func int get_upgrade_cost(e_upgrade id)
{
	s_upgrade_data data = c_upgrade_data[id];
	int curr_level = game->play_state.upgrade_level_arr[id];
	int cost = data.base_cost * (curr_level + 1);
	return cost;
}

func void do_leaderboard_stuff()
{
	if(!game->leaderboard_state.received) {
		draw_text(g_r, strlit("Getting leaderboard..."), c_half_res, 10, 48, make_color(0.66f), true, game->font, game->ui_render_pass1);
	}
	else if(game->leaderboard_arr.count <= 0) {
		draw_text(g_r, strlit("No scores yet :("), c_half_res, 10, 48, make_color(0.66f), true, game->font, game->ui_render_pass1);
	}

	constexpr int c_max_visible_entries = 10;
	s_v2 pos = c_half_res * v2(1.0f, 0.7f);
	for(int entry_i = 0; entry_i < at_most(c_max_visible_entries + 1, game->leaderboard_arr.count); entry_i++) {
		s_leaderboard_entry entry = game->leaderboard_arr[entry_i];
		s_time_data data = update_count_to_time_data(entry.time, c_update_delay);
		s_v4 color = make_color(0.8f);
		int rank_number = entry_i + 1;
		if(entry_i == c_max_visible_entries || strcmp(g_platform_data->leaderboard_public_uid.data, entry.internal_name.data) == 0) {
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
}

func void draw_progress_bar(s_v2 pos, s_v2 size, s_v4 under_size, s_v4 over_size, s_len_str str, float progress)
{
	draw_rect(g_r, pos, 0, size, under_size, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});
	float width = progress * size.x;
	draw_rect(g_r, pos, 1, v2(width, size.y), over_size, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});

	if(str.len > 0) {
		draw_text(
			g_r, str, pos + size * 0.5f + v2(0.0f, 3.0f), 0,
			24, make_color(1), true, game->font, game->ui_render_pass1
		);
	}
}

func float get_multiplier(int level, float per_level)
{
	assert(level >= 0);
	float result = 1.0f + (level * per_level / 100.0f);
	return result;
}

void s_timer::tick()
{
	assert(duration > 0);
	curr = at_most(curr, duration);
	curr += (float)c_update_delay * speed;
	if(curr >= duration) {
		ready = true;
	}
	else {
		ready = false;
	}
}

void s_timer::reset()
{
	curr -= duration;
	ready = false;
}

func s_timer make_timer(float curr, float duration)
{
	assert(duration > 0);
	return {
		.curr = curr,
		.duration = duration
	};
}