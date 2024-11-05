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
		game->hotkey_texture = g_r->load_texture(renderer, "examples/test/hotkey.png", e_filter_linear, e_wrap_clamp);

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
		game->upgrade_button_texture_arr[e_upgrade_deposit_spawn_rate] = g_r->load_texture(renderer, "examples/test/deposit_spawn.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_deposit_health] = g_r->load_texture(renderer, "examples/test/deposit_yield.png", e_filter_nearest, e_wrap_clamp);
		game->upgrade_button_texture_arr[e_upgrade_dash_cooldown] = g_r->load_texture(renderer, "examples/test/dash_cooldown.png", e_filter_nearest, e_wrap_clamp);

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
		register_action(g_platform_data, e_action_dash_to_keyboard, c_key_space, 0);
		register_action(g_platform_data, e_action_dash_to_mouse, c_right_mouse, 0);

		#if defined(m_debug)
		game->hide_tutorial = true;
		game->pick_free_upgrade_automatically = true;
		#endif

		#ifdef m_emscripten
		platform_data->create_websocket("wss://discrete-miserably-gopher.ngrok-free.app");
		platform_data->websocket_set_on_open_callback(on_websocket_open, NULL);
		platform_data->websocket_set_on_close_callback(on_websocket_close, NULL);
		platform_data->websocket_set_on_error_callback(on_websocket_error, NULL);
		platform_data->websocket_set_on_message_callback(on_websocket_message, NULL);
		#endif // m_emscripten
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

	result.size = optional.theme.button_size;

	result.hovered = !optional.disabled && mouse_collides_rect_topleft(g_mouse, pos, result.size);

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
	float font_size = optional.theme.font_size;
	s_button_interaction interaction = ui_button_interaction(id_str, pos, optional);

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
		do_button_tooltip(g_mouse, optional.description, optional.theme.tooltip_font_size);
	}

	return interaction.clicked;
}

func b8 ui_texture_button(s_len_str id_str, s_v2 pos, s_texture texture, s_ui_optional optional)
{
	pos -= v2(optional.grow_from_center * 0.5f);
	optional.theme.button_size += v2(optional.grow_from_center);
	s_button_interaction interaction = ui_button_interaction(id_str, pos, optional);

	s_v4 color = make_color(0.7f);

	if(interaction.hovered) {
		color = make_color(1);
	}
	float color_multi = optional.darken;
	draw_texture(g_r, pos, 0, interaction.size, brighter(color, color_multi), texture, game->ui_render_pass0, {}, {.mix_weight = optional.flash, .origin_offset = c_origin_topleft});

	if(interaction.hovered && optional.description.len > 0) {
		do_button_tooltip(g_mouse, optional.description, optional.theme.tooltip_font_size);
	}

	return interaction.clicked;
}

func s_tooltip make_tooltip(s_v2 pos, s_len_str description, float font_size, b8 offset)
{
	s_tooltip t = zero;
	s_v2 text_size = get_text_size(description, game->font, font_size);
	float padding = 16;
	t.size = text_size + v2(padding * 2);
	t.pos = pos;
	if(offset) {
		t.pos -= v2(0.0f, t.size.y);
	}
	t.text_pos = t.pos + v2(padding);
	t.text_pos.y += 4;
	return t;
}

func void do_button_tooltip(s_v2 pos, s_len_str description, float font_size)
{
	s_tooltip t = make_tooltip(pos, description, font_size, true);
	draw_tooltip(t, description, font_size);
}

func void draw_tooltip(s_tooltip tooltip, s_len_str description, float font_size)
{
	s_rectf panel = fit_rect(tooltip.pos, tooltip.size, c_base_res_bounds);
	tooltip.pos = panel.pos;
	tooltip.size = panel.size;
	draw_rect(g_r, tooltip.pos, 0, tooltip.size, hex_rgb_plus_alpha(0x9E8642, 0.85f), game->ui_render_pass2, {}, {.origin_offset = c_origin_topleft});
	draw_text(g_r, description, tooltip.text_pos, 0, font_size, make_color(1), false, game->font, game->ui_render_pass3);
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
			// @Fixme(tkap, 21/10/2024): garbage? we should think of the positions as points? because what if we intend to draw with the origin at the topleft?
			// now we have to undo this??
			area.pos.x = center_pos.x - element_size.x * 0.5f;
		}
	}
	else {
		area.pos.x = pos.x;
	}

	if(center_y) {
		if(horizontal) {
			// @Fixme(tkap, 21/10/2024): garbage? we should think of the positions as points? because what if we intend to draw with the origin at the topleft?
			// now we have to undo this??
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

func s_v2 pos_area_get_advance(s_pos_area* area, float advance_x, float advance_y)
{
	s_v2 result = area->pos;
	area->pos.x += area->advance.x * advance_x;
	area->pos.y += area->advance.y * advance_y;
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

func b8 set_state_next_frame_with_transition(e_state new_state)
{
	if(set_state_next_frame(new_state)) {
		do_state_transition();
		return true;
	}
	return false;
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
	return s == e_sub_state_pause || s == e_sub_state_defeat || s == e_sub_state_level_up || s == e_sub_state_controls;
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
	s_ui_optional optional = zero;
	optional.theme = c_theme_big;
	s_play_state* play_state = &game->play_state;
	int button_count = in_play_mode ? 10 : 8;

	s_pos_area area = make_pos_area(wxy(0.0f, 0.4f), wxy(1.0f, 0.2f), c_theme_big.button_size, 8, button_count, e_pos_area_flag_center_x | e_pos_area_flag_center_y | e_pos_area_flag_vertical);
	if(in_play_mode && ui_button(strlit("Resume"), pos_area_get_advance(&area), optional)) {
		play_state->sub_state = e_sub_state_default;
	}
	if(ui_button(strlit("Leaderboard"), pos_area_get_advance(&area), optional)) {
		set_state_next_frame_with_transition(e_state_leaderboard);
		if constexpr(c_are_we_on_web) {
			on_leaderboard_score_submitted();
		}
	}
	if(ui_button(strlit("Controls"), pos_area_get_advance(&area), optional)) {
		if(in_play_mode) {
			game->play_state.sub_state = e_sub_state_controls;
		}
		else {
			game->main_menu.sub_state = e_sub_state_controls;
		}
	}
	if(ui_button(format_text("Sounds: %s", game->sound_disabled ? "Off" : "On"), pos_area_get_advance(&area), optional)) {
		game->sound_disabled = !game->sound_disabled;
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
		go_back_to_prev_state_with_transition();
	}
}

func void do_controls_menu(b8 in_play_mode)
{
	s_ui_optional optional = zero;
	optional.theme = c_theme_big;
	optional.disabled = game->waiting_for_key;
	s_pos_area area = make_vertical_layout(wxy(0.05f, 0.3f), c_theme_big.button_size, 8, 0);

	if(game->waiting_for_key) {
		draw_text(g_r, m_strlit("Press a key"), wxy(0.5f, 0.1f), 0, 64 * sin_range(1, 1.25f, game->render_time * 8.0f), make_color(1), true, game->font, game->ui_render_pass1);
		draw_text(g_r, m_strlit("Press escape to cancel..."), wxy(0.5f, 0.18f), 0, 40, make_color(0.66f), true, game->font, game->ui_render_pass1);
		foreach_val(event_i, event, g_input->key_events) {
			if(event.went_down && is_valid_keybind(event.key)) {
				g_platform_data->action_key_arr[game->target_action][game->target_key] = event.key;
				game->waiting_for_key = false;
				break;
			}
		}
	}

	for_enum(action_i, e_action) {
		s_pos_area temp_area = make_horizontal_layout(pos_area_get_advance(&area), c_theme_big.button_size, 8, 0);
		s_v2 text_pos = pos_area_get_advance(&temp_area);
		text_pos.y += c_theme_big.button_size.y * 0.5f;
		text_pos.y -= c_theme_big.font_size * 0.5f;
		draw_text(g_r, c_action_name_arr[action_i], text_pos, 0, c_theme_big.font_size, make_color(1), false, game->font, game->ui_render_pass1);
		for(int key_i = 0; key_i < 2; key_i += 1) {
			s_v2 button_pos = pos_area_get_advance(&temp_area);
			s_v2 x_pos = pos_area_get_advance(&temp_area, 0.2f, 1.0f);
			int* key = &g_platform_data->action_key_arr[action_i][key_i];
			s_len_str str = virtual_key_to_str(*key);
			if(str.len <= 0) {
				str = format_text(" ##key%i%i", action_i, key_i);
			}
			if(ui_button(format_text("%.*s", expand_str(str)), button_pos, optional)) {
				game->waiting_for_key = true;
				game->target_action = action_i;
				game->target_key = key_i;
			}
			s_ui_optional temp_optional = optional;
			temp_optional.theme.button_size.x = 40;
			if(ui_button(m_strlit("$$ed1c23x"), x_pos, temp_optional)) {
				*key = 0;
			}
		}
	}

	b8 want_to_exit = false;
	if(ui_button(strlit("Back"), wxy(0.7f, 0.9f), optional)) {
		want_to_exit = true;
	}

	if(is_key_pressed(g_input, c_key_escape)) {
		if(game->waiting_for_key) {
			game->waiting_for_key = false;
		}
		else {
			want_to_exit = true;
		}
	}

	if(want_to_exit) {
		if(in_play_mode) {
			game->play_state.sub_state = e_sub_state_pause;
		}
		else {
			game->main_menu.sub_state = e_sub_state_pause;
		}
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

func void go_back_to_prev_state_with_transition()
{
	go_back_to_prev_state();
	do_state_transition();
}

func void do_state_transition()
{
	game->in_state_transition = true;
	game->state_transition_timer = 0;
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

func void draw_hotkey(s_v2 pos, s_len_str str, float font_size, float color_multi)
{
	s_v2 text_size = get_text_size(str, game->font, font_size);
	draw_texture(g_r, pos, 0, v2(font_size), make_color(0.9f * color_multi), game->hotkey_texture, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});
	pos.x += font_size * 0.5f;
	pos.x -= text_size.x * 0.5f;
	pos.y += font_size * 0.5f;
	pos.y -= text_size.y * 0.5f;
	draw_text(g_r, str, pos, 0, font_size, make_color(0.4f * color_multi), false, game->font, game->ui_render_pass1);
}

func void draw_cost_and_hotkey(s_v2 pos, s_len_str cost_str, s_len_str hotkey_str, float font_size, float color_multi)
{
	s_v2 text_size = get_text_size(cost_str, game->font, font_size);
	text_size.x += get_text_size(hotkey_str, game->font, font_size).x;
	text_size.x += font_size;

	s_v2 temp_pos = pos - v2(0.0f, font_size * 0.5f);
	temp_pos.x -= text_size.x * 0.5f;
	draw_texture(g_r, temp_pos - v2(0.0f, 4.0f), 0, v2(font_size), make_color(color_multi), game->base_texture, game->ui_render_pass0, {}, {.origin_offset = c_origin_topleft});
	temp_pos.x += font_size;
	temp_pos = draw_text(g_r, format_text("%.*s", expand_str(cost_str)), temp_pos, 0, font_size, make_color(color_multi), false, game->font, game->ui_render_pass1);
	temp_pos.x += font_size * 0.5f;
	draw_hotkey(temp_pos, hotkey_str, font_size, color_multi);
}

func int update_animator(s_animator* animator, float* time_ptr, float speed, b8 loop)
{
	assert(animator->step_count > 0);
	assert(speed > 0);
	assert(!animator->needs_wait_call);

	int result = 0;
	if(*time_ptr * speed >= animator->total_duration) {
		result = -1;
	}

	float t;
	if(loop) {
		t = fmodf(*time_ptr * speed, animator->total_duration);
	}
	else {
		t = min(*time_ptr * speed, animator->total_duration);
	}

	int step_index = -1;
	for(int step_i = 0; step_i < animator->step_count; step_i += 1) {
		float start = animator->step_start_time_arr[step_i];
		if(t >= start && t <= start + animator->step_duration_arr[step_i]) {
			step_index = step_i;
			break;
		}
	}
	assert(step_index >= 0);
	if(result == 0 && step_index > 0) {
		result = animator->result_on_end[step_index - 1];
	}

	for(int step_i = 0; step_i < step_index + 1; step_i += 1) {
		float step_start = animator->step_start_time_arr[step_i];
		foreach_val(property_i, property, animator->property_arr[step_i]) {
			float interp_dt = ilerp_clamp(step_start + property.delay, step_start + property.delay + property.duration, t);

			switch(property.ease_mode) {
				#define X(fname, ename) case ename: { interp_dt = fname(interp_dt); } break;
				m_advanced_easings
				#undef X
				invalid_default_case;
			}

			switch(property.type) {
				case e_animator_curve: {
					s_v2 p = bezier(property.curve.a, property.curve.b, property.curve.pivot, interp_dt);
					*(s_v2*)property.ptr = p;
				} break;

				case e_animator_color: {
					s_v4 p = lerp(property.color.a, property.color.b, interp_dt);
					*(s_v4*)property.ptr = p;
				} break;

				case e_animator_float: {
					float p = lerp(property.nfloat.a, property.nfloat.b, interp_dt);
					*(float*)property.ptr = p;
				} break;

				case e_animator_point: {
					*(s_v2*)property.ptr = property.point.a;
				} break;
				invalid_default_case;
			}
		}
	}
	if(loop) {
		*time_ptr = fmodf(*time_ptr, animator->total_duration / speed);
	}
	else {
		*time_ptr = min(*time_ptr, animator->total_duration / speed);
	}
	return result;
}

func void animator_wait_completed(s_animator* animator, float delay)
{
	animator_wait_completed_ex(animator, delay, 0);
}

func void animator_wait_completed_ex(s_animator* animator, float delay, int result_on_end)
{
	assert(delay >= 0);
	assert(animator->property_arr[animator->curr_step].count > 0);
	animator->curr_step += 1;
	animator->step_count += 1;
	animator->step_start_time_arr[animator->curr_step] = animator->step_start_time_arr[animator->curr_step - 1] + animator->step_duration_arr[animator->curr_step - 1] + delay;
	animator->total_duration += animator->step_duration_arr[animator->curr_step - 1] + delay;
	animator->step_duration_arr[animator->curr_step - 1] += delay;
	animator->result_on_end[animator->curr_step - 1] = result_on_end;

	#ifdef m_debug
	animator->needs_wait_call = false;
	#endif // m_debug
}

func void add_curve(s_animator* animator, s_v2 a, s_v2 b, s_v2 pivot, float duration, float delay, s_v2* ptr, e_ease ease_mode)
{
	assert(ptr);

	#ifdef m_debug
	animator->needs_wait_call = true;
	#endif // m_debug

	max_by_ptr(&animator->step_duration_arr[animator->curr_step], delay + duration);
	s_animator_property p = zero;
	p.type = e_animator_curve;
	p.ease_mode = ease_mode;
	p.duration = duration;
	p.delay = delay;
	p.ptr = ptr;
	p.curve.a = a;
	p.curve.b = b;
	p.curve.pivot = pivot;
	animator->property_arr[animator->curr_step].add(p);
}

func void add_color(s_animator* animator, s_v4 a, s_v4 b, float duration, float delay, s_v4* ptr, e_ease ease_mode)
{
	assert(ptr);

	#ifdef m_debug
	animator->needs_wait_call = true;
	#endif // m_debug

	max_by_ptr(&animator->step_duration_arr[animator->curr_step], delay + duration);
	s_animator_property p = zero;
	p.ease_mode = ease_mode;
	p.type = e_animator_color;
	p.duration = duration;
	p.delay = delay;
	p.ptr = ptr;
	p.color.a = a;
	p.color.b = b;
	animator->property_arr[animator->curr_step].add(p);
}

func void add_point(s_animator* animator, s_v2 a, float duration, float delay, s_v2* ptr, e_ease ease_mode)
{
	assert(ptr);

	#ifdef m_debug
	animator->needs_wait_call = true;
	#endif // m_debug

	max_by_ptr(&animator->step_duration_arr[animator->curr_step], delay + duration);
	s_animator_property p = zero;
	p.ease_mode = ease_mode;
	p.type = e_animator_point;
	p.duration = duration;
	p.delay = delay;
	p.ptr = ptr;
	p.point.a = a;
	animator->property_arr[animator->curr_step].add(p);
}

func void add_float(s_animator* animator, float a, float b, float duration, float delay, float* ptr, e_ease ease_mode)
{
	assert(ptr);

	#ifdef m_debug
	animator->needs_wait_call = true;
	#endif // m_debug

	max_by_ptr(&animator->step_duration_arr[animator->curr_step], delay + duration);
	s_animator_property p = zero;
	p.ease_mode = ease_mode;
	p.type = e_animator_float;
	p.duration = duration;
	p.delay = delay;
	p.ptr = ptr;
	p.nfloat.a = a;
	p.nfloat.b = b;
	animator->property_arr[animator->curr_step].add(p);
}

#ifdef m_emscripten
func void on_websocket_open(void* user_data)
{
	printf("websocket open\n");
	g_platform_data->websocket_send("fuck azenris baseg!", 19);
}

func void on_websocket_close(void* user_data)
{
	printf("websocket close\n");
}

func void on_websocket_error(void* user_data)
{
	printf("websocket error\n");
}

func void on_websocket_message(void* data, int data_len, void* user_data)
{
	printf("websocket message\n");
	printf("%.*s\n", data_len, data);
}
#endif // m_emscripten