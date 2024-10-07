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

m_dll_export void update(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{

	game = (s_game*)game_memory;
	g_input = &platform_data->logic_input;
	g_mouse = platform_data->mouse;
	g_r = renderer;
	g_platform_data = platform_data;

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		initialize start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	if(!game->initialized) {
		game->initialized = true;
		game->rng = make_rng(platform_data->get_random_seed());
		g_r->set_vsync(true);
		game->sheet = g_r->load_texture(renderer, "examples/speedjam5/sheet.png", e_wrap_clamp);
		game->placeholder_texture = g_r->load_texture(renderer, "examples/test/placeholder.png", e_wrap_clamp);
		game->base_texture = g_r->load_texture(renderer, "examples/test/base.png", e_wrap_clamp);
		game->button_texture = g_r->load_texture(renderer, "examples/test/button.png", e_wrap_clamp);
		game->tile_texture = g_r->load_texture(renderer, "examples/test/tile.png", e_wrap_clamp);
		game->crater_texture = g_r->load_texture(renderer, "examples/test/crater.png", e_wrap_clamp);
		game->rock_texture_arr[0] = g_r->load_texture(renderer, "examples/test/rock01.png", e_wrap_clamp);
		game->rock_texture_arr[1] = g_r->load_texture(renderer, "examples/test/rock02.png", e_wrap_clamp);
		game->broken_bot_texture = g_r->load_texture(renderer, "examples/test/broken_bot.png", e_wrap_clamp);

		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone000.png", e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone006.png", e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone012.png", e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone018.png", e_wrap_clamp));
		add_texture(&game->bot_animation, g_r->load_texture(renderer, "examples/test/drone024.png", e_wrap_clamp));
		game->bot_animation.fps = 12;

		add_texture(&game->ant_animation, g_r->load_texture(renderer, "examples/test/ant000.png", e_wrap_clamp));
		add_texture(&game->ant_animation, g_r->load_texture(renderer, "examples/test/ant006.png", e_wrap_clamp));
		add_texture(&game->ant_animation, g_r->load_texture(renderer, "examples/test/ant012.png", e_wrap_clamp));
		add_texture(&game->ant_animation, g_r->load_texture(renderer, "examples/test/ant018.png", e_wrap_clamp));
		add_texture(&game->ant_animation, g_r->load_texture(renderer, "examples/test/ant024.png", e_wrap_clamp));
		game->ant_animation.fps = 8;

		add_texture(&game->player_animation, g_r->load_texture(renderer, "examples/test/player000.png", e_wrap_clamp));
		add_texture(&game->player_animation, g_r->load_texture(renderer, "examples/test/player006.png", e_wrap_clamp));
		add_texture(&game->player_animation, g_r->load_texture(renderer, "examples/test/player012.png", e_wrap_clamp));
		add_texture(&game->player_animation, g_r->load_texture(renderer, "examples/test/player018.png", e_wrap_clamp));
		add_texture(&game->player_animation, g_r->load_texture(renderer, "examples/test/player024.png", e_wrap_clamp));
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
		game->light_render_pass = make_render_pass(g_r, &platform_data->permanent_arena);
		g_r->default_render_pass = make_render_pass(g_r, &platform_data->permanent_arena);

		g_r->game_speed_index = 5;

		game->next_state = -1;
		set_state_next_frame(e_state_play, true);
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		initialize end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		handle state change start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	if(game->next_state >= 0) {
		game->state = (e_state)game->next_state;
		game->next_state = -1;

		switch(game->state) {
			case e_state_play: {

				if(!game->reset_game_on_state_change) { break; }

				memset(&game->play_state, 0, sizeof(game->play_state));
				game->play_state.cam.zoom = 1;

				{
					s_v2 pos = c_base_pos + v2(0.0f, c_base_size.y * 0.6f);
					game->play_state.player.pos = pos;
					game->play_state.player.prev_pos = pos;
					game->play_state.player.dash_dir = v2(1, 0);
					game->play_state.player.curr_level = 1;
				}

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		spawn broken bots start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				{
					s_bounds bounds = get_map_bounds();
					// @Note(tkap, 06/10/2024): Left
					for(int i = 0; i < 2; i += 1) {
						s_v2 pos = v2(
							game->rng.randf_range(bounds.min_x, bounds.min_x + 500),
							game->rng.randf_range(bounds.min_y, bounds.max_y)
						);
						game->play_state.broken_bot_arr.add({.rotation = game->rng.randf_range(-0.25f, 0.25f), .pos = pos});
					}
					// @Note(tkap, 06/10/2024): Right
					for(int i = 0; i < 2; i += 1) {
						s_v2 pos = v2(
							game->rng.randf_range(bounds.max_x - 500, bounds.max_x),
							game->rng.randf_range(bounds.min_y, bounds.max_y)
						);
						game->play_state.broken_bot_arr.add({.rotation = game->rng.randf_range(-0.25f, 0.25f), .pos = pos});
					}
					// @Note(tkap, 06/10/2024): Top
					for(int i = 0; i < 2; i += 1) {
						s_v2 pos = v2(
							game->rng.randf_range(bounds.min_x, bounds.max_x),
							game->rng.randf_range(bounds.min_y, bounds.min_y + 500)
						);
						game->play_state.broken_bot_arr.add({.rotation = game->rng.randf_range(-0.25f, 0.25f), .pos = pos});
					}
					// @Note(tkap, 06/10/2024): Bottom
					for(int i = 0; i < 2; i += 1) {
						s_v2 pos = v2(
							game->rng.randf_range(bounds.min_x, bounds.max_x),
							game->rng.randf_range(bounds.max_y - 500, bounds.max_y)
						);
						game->play_state.broken_bot_arr.add({.rotation = game->rng.randf_range(-0.25f, 0.25f), .pos = pos});
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

			if(state->sub_state == e_sub_state_default && state->level_up_triggers > 0) {
				state->level_up_triggers -= 1;
				state->sub_state = e_sub_state_level_up;
				state->level_up_seed = g_platform_data->get_random_seed();
				play_sound_group(e_sound_group_level_up);
			}

			{
				int alive_creatures = count_alive_creatures();
				if(alive_creatures >= c_num_creatures_to_lose) {
					state->sub_state = e_sub_state_defeat;
				}
			}

			if(game_is_paused()) {
				break;
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
				state->spawn_creature_timer += dt;
				f64 spawn_delay = get_creature_spawn_delay();
				while(state->spawn_creature_timer >= spawn_delay) {

					state->spawn_creature_timer -= spawn_delay;
					float angle = game->rng.randf_range(0, tau);
					float dist = game->rng.randf_range(c_base_size.x * 2.0f, c_base_size.x * 4.0f);
					s_v2 offset = v2(
						cosf(angle) * dist,
						sinf(angle) * dist
					);
					s_v2 pos = c_base_pos + offset;
					pos = constrain_pos(pos, get_map_bounds());

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

				if(player->dashing) {
					dir = player->dash_dir * c_dash_speed * (1.0f + get_player_movement_speed() / 20);
					player->active_dash_timer += 1;
					if(player->active_dash_timer >= c_dash_duration) {
						player->dashing = false;
					}
				}
				else {
					at_least_add(&player->cooldown_dash_timer, -1, 0);

					if(is_key_down(g_input, c_key_a) || is_key_down(g_input, c_key_left)) {
						dir.x -= 1;
					}
					if(is_key_down(g_input, c_key_d) || is_key_down(g_input, c_key_right)) {
						dir.x += 1;
					}
					if(is_key_down(g_input, c_key_w) || is_key_down(g_input, c_key_up)) {
						dir.y -= 1;
					}
					if(is_key_down(g_input, c_key_s) || is_key_down(g_input, c_key_down)) {
						dir.y += 1;
					}
					dir = v2_normalized(dir);

					if(v2_length(dir) > 0) {
						player->dash_dir = dir;
					}

					if(player->cooldown_dash_timer <= 0 && (is_key_pressed(g_input, c_key_space) || is_key_pressed(g_input, c_right_mouse))) {
						dir = player->dash_dir;
						player->dashing = true;
						player->dash_start = player->pos;
						player->active_dash_timer = 0;
						player->cooldown_dash_timer = c_dash_cooldown;
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
						s_v2 prev_pos = zero;
						int previous = -1;
						s_sarray<int, c_max_player_hits> blacklist;
						s_v2 query_pos = player->pos;
						if(!creature_arr->active[curr_target]) { continue; }

						for(int hit_i = 0; hit_i < hits; hit_i += 1) {
							s_laser_target laser_target = zero;
							if(previous >= 0) { query_pos = prev_pos; }
							prev_pos = creature_arr->pos[curr_target];
							blacklist.add(curr_target);

							// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		add laser start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
							if(previous >= 0) {
								laser_target.from = maybe(s_lerp{creature_arr->prev_pos[previous], creature_arr->pos[previous]});
							}
							laser_target.to = {creature_arr->prev_pos[curr_target], creature_arr->pos[curr_target]};
							player->laser_target_arr.add(laser_target);
							b8 killed = damage_creature(curr_target, player_damage);
							if(killed) {
								state->resource_count += get_creature_resource_reward(creature_arr->tier[curr_target], creature_arr->boss[curr_target]);
							}
							// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		add laser end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
							player->harvest_timer = 0;
							previous = curr_target;

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
							// assert(bot_arr->cargo[bot] > 0);
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

				if(state->resource_count >= c_resource_to_win) {
					if constexpr(c_are_we_on_web) {
						if(platform_data->leaderboard_nice_name.len > 0) {
							set_state_next_frame(e_state_leaderboard, true);
							game->leaderboard_state.coming_from_win = true;
							platform_data->submit_leaderboard_score(
								game->play_state.update_count, c_leaderboard_id, on_leaderboard_score_submitted
							);
							game->play_state.update_count_at_win_time = game->play_state.update_count;
						}
						else {
							set_state_next_frame(e_state_input_name, true);
						}
					}
					else {
						set_state_next_frame(e_state_leaderboard, true);
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
		play_state->resource_count = floorfi(c_resource_to_win * 0.9f);
	}
	if(is_key_pressed(g_input, c_key_f2)) {
		play_state->resource_count = c_resource_to_win;
	}
	if(is_key_pressed(g_input, c_key_f3)) {
		play_state->level_up_triggers += add_exp(&play_state->player, 5);
	}
	if(is_key_pressed(g_input, c_key_r)) {
		set_state_next_frame(e_state_play, true);
	}
	#endif // m_debug


	if(can_pause() && is_key_pressed(g_input, c_key_escape)) {
		if(game->play_state.sub_state == e_sub_state_pause) {
			game->play_state.sub_state = e_sub_state_default;
		}
		else {
			game->play_state.sub_state = e_sub_state_pause;
		}
		play_state->asking_for_restart_confirmation = false;
	}

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
						draw_texture(g_r, pos, e_layer_background, v2(c_tile_size), color, game->tile_texture, get_render_pass(e_layer_background), {.flip_x = rng.rand_bool()}, {.origin_offset = c_origin_topleft});
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw background end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
							g_r, pos, e_layer_background + 1, v2(size), make_color(brightness), game->crater_texture, get_render_pass(e_layer_background),
							{.flip_x = game->play_state.crater_flip_arr[crater_i]}, {.rotation = game->play_state.crater_rotation_arr[crater_i]}
						);
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		doodads end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			#if 0
			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		doodads start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_rng rng = make_rng(0);
				s_bounds bounds = get_map_bounds();
				for(int i = 0; i < 64; i += 1) {
					int index = rng.randu() % 2;
					s_v2 pos = v2(
						rng.randf_range(bounds.min_x, bounds.max_x),
						rng.randf_range(bounds.min_y, bounds.max_y)
					);
					draw_texture_keep_aspect(g_r, pos, e_layer_background + 1, v2(64), make_color(1), game->rock_texture_arr[index], get_render_pass(e_layer_background), {.flip_x = rng.rand_bool()});
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		doodads end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			#endif

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw base start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				draw_texture_keep_aspect(g_r, c_base_pos, e_layer_base, c_base_size, make_color(1), game->base_texture, get_render_pass(e_layer_base));
				draw_light(c_base_pos, c_base_size.x, make_color(0.5f), 0.0f);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw base end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw player start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_player* p = &game->play_state.player;
				s_v2 pos = lerp(p->prev_pos, p->pos, interp_dt);
				if(!p->dashing) {
					p->animation_timer += g_delta;
				}
				s_texture texture = get_animation_texture(&game->player_animation, &p->animation_timer);
				draw_texture(g_r, pos, e_layer_player, c_player_size, make_color(1), texture, get_render_pass(e_layer_player), {.flip_x = p->flip_x});
				draw_light(pos, 256, make_color(0.5f), 0.0f);
				draw_shadow(pos + v2(0, 32), 128, 0.5f, 0.1f);
				if(p->dashing) {
					s_v2 a = p->dash_start;
					s_v2 b = pos;
					float dist = v2_distance(a, b);
					int steps = ceilfi(dist / (c_player_size.x * 0.1f));
					for(int i = 0; i < steps; i += 1) {
						float percent = index_count_safe_div(i, steps);
						float alpha = 1.0f - index_count_safe_div(p->active_dash_timer, c_dash_duration);
						s_v2 c = lerp(a, b, percent);
						draw_texture(g_r, c, e_layer_player, c_player_size, v4(0.75f, 0.75f, 0.75f, alpha * percent), texture, get_render_pass(e_layer_creature), {.flip_x = p->flip_x});
					}
				}

				foreach_val(target_i, target, p->laser_target_arr) {
					s_v2 from_pos;
					s_v2 to_pos = lerp(target.to.prev_pos, target.to.pos, interp_dt);
					if(target.from.valid) {
						from_pos = lerp(target.from.value.prev_pos, target.from.value.pos, interp_dt);
					}
					else {
						from_pos = pos;
					}
					s_v4 laser_color = make_color(0.5f, 0.1f, 0.1f);
					draw_line(g_r, from_pos, to_pos, e_layer_laser, c_laser_width, laser_color, get_render_pass(e_layer_laser), {}, {.effect_id = 5});
					draw_light(to_pos, laser_light_radius * 1.5f, laser_color, 0.0f);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw player end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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

				b8 moving = v2_distance(creature_arr->target_pos[creature], creature_arr->pos[creature]) > 1;
				if(moving) {
					creature_arr->animation_timer[creature] += g_delta;
				}
				s_texture texture = get_animation_texture(&game->ant_animation, &creature_arr->animation_timer[creature]);

				draw_texture_keep_aspect(
					g_r, pos, e_layer_creature, get_creature_size(creature), color_arr[color_index], texture,
					get_render_pass(e_layer_creature), {.flip_x = creature_arr->flip_x[creature]}, {.mix_weight = mix_weight}
				);

				draw_shadow(pos + v2(0, 10), 36 * size_multi, 0.33f, 0.0f);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw creatures end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw broken bots start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				foreach_val(bot_i, bot, game->play_state.broken_bot_arr) {
					draw_texture_keep_aspect(g_r, bot.pos, e_layer_broken_bot, c_bot_size, make_color(1), game->broken_bot_texture,
						get_render_pass(e_layer_broken_bot), {}, {.rotation = bot.rotation}
					);
					draw_shadow(bot.pos + v2(0, 10), 36, 0.5f, 0.0f);

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

				draw_shadow(pos + v2(0, 100), 32, 0.25f, 0.0f);

				int creature = get_creature(bot_arr->laser_target[bot]);
				if(creature >= 0) {
					s_v2 creature_pos = lerp(creature_arr->prev_pos[creature], creature_arr->pos[creature], interp_dt);
					s_v4 laser_color = make_color(0.1f, 0.5f, 0.1f);
					draw_line(g_r, pos, creature_pos, e_layer_laser, c_laser_width, laser_color, get_render_pass(e_layer_laser), {}, {.effect_id = 5});
					draw_light(creature_pos, laser_light_radius, laser_color, 0.0f);
				}
				draw_light(pos, 48, make_color(0.33f), 0.0f);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw bots end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw pickups start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				foreach_val(pickup_i, pickup, game->play_state.pickup_arr) {
					do_particles(8, pickup.pos, e_layer_particle, false, c_buff_particle_data_arr[pickup.type]);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw pickups end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
					s_v2 offset = zero;
					if(p->attached_to_player) {
						offset = lerp(game->play_state.player.prev_pos, game->play_state.player.pos, interp_dt);
					}
					draw_circle(g_r, offset + p->pos, p->z, radius, color, get_render_pass(e_layer_particle));
					p->timer += (float)platform_data->frame_time;
					if(percent_done >= 1) {
						game->play_state.particle_arr.remove_and_swap(particle_i);
						particle_i -= 1;
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		particles end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			b8 show_ui = should_show_ui();

			if(play_state->sub_state == e_sub_state_defeat) {
				draw_rect(g_r, c_half_res, 0, c_base_res, v4(0.0f, 0.0f, 0.0f, 0.5f), game->ui_render_pass0);
				draw_text(g_r, strlit("You were overwhelmed!"), c_base_res * v2(0.5f, 0.4f), 0, 64, make_color(1), true, game->font, game->ui_render_pass1);
				draw_text(g_r, strlit("Press R to restart..."), c_base_res * v2(0.5f, 0.5f), 0, 64, make_color(0.6f), true, game->font, game->ui_render_pass1);

				if(is_key_pressed(g_input, c_key_r)) {
					set_state_next_frame(e_state_play, true);
				}
			}
			if(show_ui) {
				draw_text(g_r, format_text("%i", play_state->resource_count), v2(4), 0, 32, make_color(1), false, game->font, game->ui_render_pass1);

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		upgrade buttons start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				{
					constexpr float padding = 8;
					constexpr float font_size = 24;
					constexpr int c_rows = 3;
					struct s_row
					{
						int upgrade_count;
						s_carray<e_upgrade, e_upgrade_count> upgrade_arr;
					};
					s_carray<s_row, c_rows> row_arr = zero;
					row_arr[0].upgrade_count = 4;
					row_arr[0].upgrade_arr = {{e_upgrade_player_damage, e_upgrade_player_movement_speed, e_upgrade_player_harvest_range, e_upgrade_player_chain}};
					row_arr[1].upgrade_count = 5;
					row_arr[1].upgrade_arr = {{e_upgrade_buy_bot, e_upgrade_bot_damage, e_upgrade_bot_movement_speed, e_upgrade_bot_cargo_count, e_upgrade_bot_harvest_range}};
					row_arr[2].upgrade_count = 3;
					row_arr[2].upgrade_arr = {{e_upgrade_spawn_rate, e_upgrade_creature_tier, e_upgrade_double_harvest}};
					s_v2 panel_pos = v2(0.0f, c_base_res.y - (c_base_button_size.y + padding) * 3);
					s_v2 panel_size = v2(c_base_res.x, c_base_button_size.y + padding);
					s_carray<s_pos_area, c_rows> area_arr;
					s_v2 button_size = v2(370, 32);
					area_arr[0] = make_pos_area(v2(padding, 0.0f) + panel_pos, panel_size, button_size, padding, -1, e_pos_area_flag_center_y);
					area_arr[1] = make_pos_area(v2(padding, 0.0f) + panel_pos + v2(0.0f, panel_size.y), panel_size, button_size, padding, -1, e_pos_area_flag_center_y);
					area_arr[2] = make_pos_area(v2(padding, 0.0f) + panel_pos + v2(0.0f, panel_size.y * 2), panel_size, button_size, padding, -1, e_pos_area_flag_center_y);
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
							s_ui_optional optional = zero;
							optional.description = get_upgrade_tooltip(upgrade_id);
							optional.font_size = font_size;
							optional.size_x = button_size.x;
							optional.size_y = button_size.y;
							if(
								!over_limit &&
								(ui_button(
									format_text("%s (%i) [%c]", data.name, cost, (char)data.key), pos_area_get_advance(&area_arr[row_i]), optional
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
											if(!played_buy_bot_sound) {
												play_sound_group(e_sound_group_buy_bot);
												played_buy_bot_sound = true;
											}
										}
									}
									else {
										purchased = true;
										if(!played_upgrade_sound) {
											play_sound_group(e_sound_group_upgrade);
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
					int alive_creatures = count_alive_creatures();
					float defeat_progress = alive_creatures / (float)c_num_creatures_to_lose;
					float shake_intensity = 0;
					if(defeat_progress > 0.75f) { shake_intensity = range_lerp(defeat_progress, 0.75f, 1.0f, 2.0f, 5.0f); }
					else if(defeat_progress > 0.5f) { shake_intensity = range_lerp(defeat_progress, 0.5f, 0.75f, 1.0f, 2.0f); }
					s_v2 offset = v2(
						game->rng.randf32_11() * shake_intensity,
						game->rng.randf32_11() * shake_intensity
					);
					s_v2 pos = wxy(0.33f, 0.02f) + offset;
					s_v2 size = wxy(0.33f, 0.025f);
					draw_rect(g_r, pos, 0, size, make_color(0.25f, 0.1f, 0.1f), game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});
					float width = defeat_progress * c_base_res.x * 0.33f;
					draw_rect(g_r, pos, 1, v2(width, size.y), make_color(0.66f, 0.1f, 0.1f), game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});

					draw_text(
						g_r, format_text("%i", alive_creatures), pos + size * 0.5f + v2(0.0f, 3.0f), 0,
						24, make_color(1), true, game->font, game->ui_render_pass1
					);
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		lose progress end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

				// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		level up progress start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
				{
					s64 required_exp = get_required_exp_to_level(play_state->player.curr_level);
					float level_progress = play_state->player.curr_exp / (float)required_exp;
					s_v4 color = v4(0.0f, 0.474f, 0.945f, 1.0f);
					float width = level_progress * c_base_res.x * 0.33f;
					s_v2 pos = wxy(0.33f, 0.07f);
					s_v2 size = wxy(0.33f, 0.025f);
					draw_rect(
						g_r, pos, 0, size, brighter(color, 0.5f), game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft}
					);
					draw_rect(
						g_r, pos, 1, v2(width, size.y), color, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft}
					);
					draw_text(
						g_r, format_text("%i", play_state->player.curr_level), pos + size * 0.5f + v2(0.0f, 3.0f), 0,
						24, make_color(1), true, game->font, game->ui_render_pass1
					);
				}
				// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		level up progress end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
			}

			if(play_state->sub_state == e_sub_state_pause) {
				s_v2 button_size = c_base_button_size2;
				s_ui_optional optional = zero;
				optional.size_x = button_size.x;
				optional.size_y = button_size.y;
				s_pos_area area = make_pos_area(wxy(0.0f, 0.4f), wxy(1.0f, 0.2f), button_size, 8, 4, e_pos_area_flag_center_x | e_pos_area_flag_center_y | e_pos_area_flag_vertical);
				if(ui_button(strlit("Resume"), pos_area_get_advance(&area), optional)) {
					play_state->sub_state = e_sub_state_default;
					play_state->asking_for_restart_confirmation = false;
				}
				if(ui_button(strlit("Leaderboard"), pos_area_get_advance(&area), optional)) {
					set_state_next_frame(e_state_leaderboard, false);
					if constexpr(c_are_we_on_web) {
						on_leaderboard_score_submitted();
					}
					play_state->asking_for_restart_confirmation = false;
				}
				if(ui_button(format_text("Sounds: %s", game->sound_disabled ? "Off" : "On"), pos_area_get_advance(&area), optional)) {
					game->sound_disabled = !game->sound_disabled;
					play_state->asking_for_restart_confirmation = false;
				}
				if(ui_button(play_state->asking_for_restart_confirmation ? strlit("Are you sure?") : strlit("Restart"), pos_area_get_advance(&area), optional)) {
					if(play_state->asking_for_restart_confirmation) {
						set_state_next_frame(e_state_play, true);
					}
					else {
						play_state->asking_for_restart_confirmation = true;
					}
				}
			}

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

				int picked_choice = -1;
				s_ui_optional optional = zero;
				s_v2 button_size = c_base_button_size2;
				button_size.x += 200;
				button_size.y += 16;
				optional.size_x = button_size.x;
				optional.size_y = button_size.y;
				optional.font_size = 40;
				optional.tooltip_font_size = 40;

				s_pos_area area = make_pos_area(wxy(0.0f, 0.4f), wxy(1.0f, 0.2f), button_size, 32, 3, e_pos_area_flag_center_x | e_pos_area_flag_center_y | e_pos_area_flag_vertical);
				for(int choice_i = 0; choice_i < 3; choice_i += 1) {
					e_upgrade upgrade_id = choice_arr[choice_i];
					s_upgrade_data data = c_upgrade_data[upgrade_id];
					optional.description = get_upgrade_tooltip(upgrade_id);
					if(ui_button(format_text("%s [%c]", data.name, c_key_1 + choice_i), pos_area_get_advance(&area), optional) || is_key_pressed(g_input, c_key_1 + choice_i)) {
						picked_choice = choice_i;
					}
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
					assert(play_state->level_up_triggers >= 0);
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

		case e_state_leaderboard: {

			if(is_key_pressed(g_input, c_key_r)) {
				set_state_next_frame(e_state_play, true);
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

			b8 win = game->leaderboard_state.coming_from_win;
			if(ui_button(win ? strlit("Restart") : strlit("Back"), c_base_res * v2(0.75f, 0.92f), {.size_x = c_base_button_size2.x, .size_y = c_base_button_size2.y}) || is_key_pressed(g_input, c_key_escape)) {
				b8 reset_game = win ? true : false;
				set_state_next_frame(e_state_play, reset_game);
			}

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

	g_r->end_render_pass(g_r, game->world_render_pass_arr[0], game->main_fbo, {.depth_mode = e_depth_mode_read_no_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[2], game->main_fbo, {.depth_mode = e_depth_mode_read_and_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[3], game->main_fbo, {.depth_mode = e_depth_mode_no_read_yes_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[4], game->main_fbo, {.depth_mode = e_depth_mode_no_read_yes_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[5], game->main_fbo, {.depth_mode = e_depth_mode_no_read_yes_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[6], game->main_fbo, {.depth_mode = e_depth_mode_no_read_yes_write, .blend_mode = e_blend_mode_additive, .view = view, .projection = ortho});
	g_r->end_render_pass(g_r, game->world_render_pass_arr[7], game->main_fbo, {.depth_mode = e_depth_mode_no_read_yes_write, .blend_mode = e_blend_mode_premultiply_alpha, .view = view, .projection = ortho});

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
	g_r->end_render_pass(g_r, game->ui_render_pass2, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});
	g_r->end_render_pass(g_r, game->ui_render_pass3, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});

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

func b8 ui_button(s_len_str id_str, s_v2 pos, s_ui_optional optional)
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
	s_v4 color = make_color(0.6f);

	if(hovered && is_key_pressed(g_input, c_left_mouse)) {
		result = true;
	}

	if(hovered) {
		color = make_color(1);
	}
	draw_texture(g_r, pos, 0, size, color, game->button_texture, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});

	{
		s_v2 text_pos = center_text_on_rect(id_str, game->font, pos, size, font_size, true, true);
		text_pos.y += font_size * 0.1f;
		draw_text(g_r, parse_result.text, text_pos, 1, font_size, make_color(1), false, game->font, game->ui_render_pass1);
	}

	if(hovered && optional.description.len > 0) {
		float temp_font_size = font_size * 1.6f;
		if(optional.tooltip_font_size > 0) {
			temp_font_size = optional.tooltip_font_size;
		}
		s_v2 text_size = get_text_size(optional.description, game->font, temp_font_size);
		float padding = 16;
		s_v2 panel_size = text_size + v2(padding * 2);
		s_v2 panel_pos = g_mouse - v2(0.0f, panel_size.y);
		s_rectf panel = fit_rect(panel_pos, panel_size, c_base_res_bounds);
		s_v2 text_pos = panel.pos + v2(padding);
		text_pos.y += 4;
		draw_rect(g_r, panel.pos, 0, panel.size, hex_rgb_plus_alpha(0x9E8642, 0.85f), game->ui_render_pass2, {}, {.origin_offset = c_origin_topleft});
		draw_text(g_r, optional.description, text_pos, 0, temp_font_size, make_color(1), false, game->font, game->ui_render_pass3);
	}

	return result;
}

static void on_set_leaderboard_name(b8 success)
{
	if(success) {
		set_state_next_frame(e_state_leaderboard, true);
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
	creature_arr->animation_timer[entity] = 0;

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

		float chance = 1;
		if(creature_arr->boss[creature]) {
			chance = 10;
		}

		if(game->rng.chance100(chance)) {
			make_pickup(creature_arr->pos[creature], (e_pickup)game->rng.rand_range_ie(0, e_pickup_count));
		}

		int level_up_count = add_exp(&game->play_state.player, get_creature_exp_reward(creature_arr->tier[creature], creature_arr->boss[creature]));
		game->play_state.level_up_triggers += level_up_count;

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
	return game->play_state.upgrade_level_arr[e_upgrade_creature_tier];
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

func int get_creature_exp_reward(int tier, b8 boss)
{
	int result = tier + 1;
	if(boss) {
		result *= 11;
	}
	return result;
}

func void set_state_next_frame(e_state new_state, b8 reset_game_on_state_change)
{
	if(game->next_state >= 0) { return; }

	game->reset_game_on_state_change = reset_game_on_state_change;

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

func s_v2 get_creature_size(int creature)
{
	float multi = game->play_state.creature_arr.boss[creature] ? 3.0f : 1.0f;
	return c_creature_size * multi;
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
	bounds.min_x = cam.pos.x;
	bounds.min_y = cam.pos.y;
	bounds.max_x = cam.pos.x + c_base_res.x / cam.zoom;
	bounds.max_y = cam.pos.y + c_base_res.y / cam.zoom;
	return bounds;
}

func s_bounds get_cam_bounds_snap_to_tile_size(s_camera2d cam)
{
	s_bounds bounds = zero;
	bounds.min_x = cam.pos.x;
	bounds.min_y = cam.pos.y;
	bounds.max_x = cam.pos.x + c_base_res.x / cam.zoom;
	bounds.max_y = cam.pos.y + c_base_res.y / cam.zoom;

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

func void make_pickup(s_v2 pos, e_pickup type)
{
	s_pickup pickup = zero;
	pickup.pos = pos;
	pickup.type = type;
	game->play_state.pickup_arr.add_checked(pickup);
}

func void add_buff(s_player* player, e_pickup pickup)
{
	player->buff_arr[pickup].ticks_left = 500;
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
			int val = game->play_state.upgrade_level_arr[e_upgrade_creature_tier];
			result = format_text("Creatures are stronger and more rewarding\n\nCurrent: %i", val);
		} break;

		case e_upgrade_player_harvest_range: {
			result = format_text("+35 player harvest range\n\nCurrent: %.0f", get_player_harvest_range());
		} break;

		case e_upgrade_bot_harvest_range: {
			result = format_text("+20 drone harvest range\n\nCurrent: %.0f", get_bot_harvest_range());
		} break;

		case e_upgrade_double_harvest: {
			result = format_text("Gain double resources from harvesting");
		} break;

		case e_upgrade_bot_cargo_count: {
			result = format_text("Drones can harvest more creatures\nbefore having to return to the hive\n\nCurrent: %i", get_bot_max_cargo_count());
		} break;

		case e_upgrade_player_chain: {
			result = format_text("Player attack chains to nearby enemies\n\nCurrent: %i", get_player_hits() - 1);
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
	return 5 + floorfi(0.3f * level_minus_one * level) + (level_minus_one) * 5;
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

func b8 should_show_ui()
{
	e_sub_state s = game->play_state.sub_state;
	return s == e_sub_state_default;
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