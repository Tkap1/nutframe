#define m_game
#define s_list s_sarray
#define s_linear_arena s_lin_arena

#if defined(m_emscripten)
#include <emscripten.h>
#endif // m_emscripten

#include "../../src/platform_shared.h"
#include "variables.h"

#define global static

#include "../../../http_server/src/common.h"
#include "test.h"

// static constexpr s_bounds c_base_res_bounds = rect_to_bounds(v2(0), c_base_res);

static s_input* g_input;
static s_game* game;
static s_game_renderer* g_r;
static s_v2 g_mouse;
static s_platform_data* g_platform_data;
static float g_delta = 0;

#include "../../../http_server/src/common.cpp"

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

		game->sound_arr[e_sound_creature_death00] = platform_data->load_sound(platform_data, "examples/test/creature_death00.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_creature_death01] = platform_data->load_sound(platform_data, "examples/test/creature_death01.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_creature_death02] = platform_data->load_sound(platform_data, "examples/test/creature_death02.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_buy_bot] = platform_data->load_sound(platform_data, "examples/test/buy_bot.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_upgrade] = platform_data->load_sound(platform_data, "examples/test/upgrade.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_level_up] = platform_data->load_sound(platform_data, "examples/test/level_up.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_dash] = platform_data->load_sound(platform_data, "examples/test/dash.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_click] = platform_data->load_sound(platform_data, "examples/test/keypress.wav", platform_data->frame_arena);
		game->sound_arr[e_sound_lose_life] = platform_data->load_sound(platform_data, "examples/test/oof.wav", platform_data->frame_arena);

		game->main_fbo = g_r->make_framebuffer(g_r, v2i(c_base_res));
		game->light_fbo = g_r->make_framebuffer_with_existing_depth(g_r, v2i(c_base_res), game->main_fbo->depth);

		game->font = &renderer->fonts[0];
		platform_data->variables_path = "examples/test/variables.h";

		game->render_pass = make_render_pass(g_r, &platform_data->permanent_arena);
		g_r->default_render_pass = make_render_pass(g_r, &platform_data->permanent_arena);

		g_r->game_speed_index = 5;

		game->next_state = -1;
		set_state_next_frame(e_state_input_name);

		register_action(g_platform_data, e_action_left, c_key_a, c_key_left);
		register_action(g_platform_data, e_action_right, c_key_d, c_key_right);
		register_action(g_platform_data, e_action_up, c_key_w, c_key_up);
		register_action(g_platform_data, e_action_down, c_key_s, c_key_down);
		register_action(g_platform_data, e_action_dash_to_keyboard, c_key_space, 0);
		register_action(g_platform_data, e_action_dash_to_mouse, c_right_mouse, 0);

		#ifdef m_emscripten
		platform_data->create_websocket("wss://discrete-miserably-gopher.ngrok-free.app");
		// platform_data->create_websocket("ws://localhost:8000");
		platform_data->websocket_set_on_open_callback(on_websocket_open, NULL);
		platform_data->websocket_set_on_close_callback(on_websocket_close, NULL);
		platform_data->websocket_set_on_error_callback(on_websocket_error, NULL);
		platform_data->websocket_set_on_message_callback(on_websocket_message, NULL);
		#endif // m_emscripten

		u64 size = 0;
		char* data = platform_data->read_file("examples/test/words_alpha_new.txt", &platform_data->permanent_arena, &size);
		parse_words(data, (s64)size);
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

	float delta = (float)c_update_delay;

	switch(get_state()) {
		case e_state_play: {
			s_play* play = &game->play;
			if(!is_play_paused(play)) {
				foreach_ptr(word_i, word, play->word_arr) {
					word->prev_fall_timer = word->fall_timer;
					word->prev_pos = word->pos;
					if(word->fall_timer < c_word_fall_time) {
						word->fall_timer += delta;
					}
					else {
						word->pos += word->dir * c_word_speed * delta;
					}
				}
			}
		} break;
	}

}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt)
{
	static_assert(sizeof(s_game) <= c_game_memory);

	g_mouse = platform_data->mouse;

	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = &platform_data->input;
	g_platform_data = platform_data;

	s_m4 ortho = m4_orthographic(0, c_base_res.x, c_base_res.y, 0, -100, 100);
	g_delta = (float)platform_data->frame_time;
	game->render_time += g_delta;

	g_r->clear_framebuffer(game->main_fbo, zero, c_default_fbo_clear_flags);
	g_r->clear_framebuffer(game->light_fbo, v4(0.75f, 0.75f, 0.75f, 1.0f), e_fbo_clear_color);

	switch(get_state()) {
		case e_state_input_name: {

			s_input_name_state* state = &game->input_name_state;

			float font_size = 36;

			s_v2 pos = c_base_res * v2(0.5f, 0.4f);

			b8 submitted = handle_string_input(&state->name, g_input, game->render_time);
			if(state->name.cursor.last_edit_time == game->render_time) {
				play_sound_group(e_sound_group_click);
			}
			if(submitted) {
				b8 can_submit = true;
				if(state->name.str.len < 2) {
					can_submit = false;
					state->error_str.from_cstr("Name must have at least 2 characters!");
				}
				if(can_submit && !state->waiting_for_server_response) {
					s_buffer_writer writer = zero;
					buffer_write(&writer, e_packet_send_name);
					buffer_write(&writer, state->name.str.len);
					buffer_write_array(&writer, state->name.str.data, state->name.str.len);
					game->name.from_data(state->name.str.data, state->name.str.len);

					#if defined(m_emscripten)
					platform_data->websocket_send(writer.buffer, writer.len);
					#endif // m_emscripten
					state->error_str.len = 0;
					state->waiting_for_server_response = true;
				}
			}

			draw_text(g_r, strlit("Enter your name"), c_base_res * v2(0.5f, 0.2f), 0, font_size, make_color(1), true, game->font, game->render_pass);
			if(state->error_str.len > 0) {
				draw_text(g_r, strlit(state->error_str.data), c_base_res * v2(0.5f, 0.3f), 0, font_size, rgb(0xD77870), true, game->font, game->render_pass);
			}

			if(state->name.str.len > 0) {
				draw_text(g_r, strlit(state->name.str.data), pos, 0, font_size, make_color(1), true, game->font, game->render_pass);
			}

			draw_cool_cursor(
				pos, strlit(state->name.str.data), &state->name.cursor, font_size
			);

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw "version" start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_str_builder<64> builder0;
				s_str_builder<64> builder1;
				builder_add(&builder1, "%s", __TIME__);
				builder_remove_all(&builder1, m_strlit(":"));
				builder_add(&builder0, "Version: %.*s", builder1.len, builder1.str);
				draw_text(g_r, builder_to_len_str(&builder0), wxy(0.01f, 0.95f), 0, font_size, make_color(1), false, game->font, game->render_pass);
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw "version" end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			g_r->end_render_pass(g_r, game->render_pass, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});

		} break;

		case e_state_play: {
			s_play* play = &game->play;

			#if defined(m_emscripten)

			int num_chars_to_send = 0;
			s_buffer_writer writer = zero;
			buffer_write(&writer, e_packet_type_char);
			u8* count_dst = buffer_write(&writer, num_chars_to_send); // @Note(tkap, 07/11/2024): placeholder to be written later
			foreach_val(c_i, c, g_input->char_events) {
				b8 send = false;
				if(is_alpha(c) && play->input_text.len < c_max_text_input) {
					send = true;
					if(!play->cursor.index.valid) {
						play->cursor.index = maybe(0);
					}
					play->input_text.str[play->cursor.index.value] = c;
					play->cursor.index.value += 1;
					play->input_text.len += 1;

					play->cursor.last_action_time = game->render_time;
					play->cursor.last_edit_time = game->render_time;
				}
				else if(c == '\b') {
					if(play->cursor.index.value > 0) {
						send = true;

						play->cursor.index.value -= 1;
						play->input_text.len -= 1;
						int to_copy = play->input_text.len - play->cursor.index.value;
						memmove(&play->input_text.str[play->cursor.index.value], &play->input_text.str[play->cursor.index.value + 1], to_copy);

						play->cursor.last_action_time = game->render_time;
						play->cursor.last_edit_time = game->render_time;
					}
				}
				else if(c == c_ctrl_backspace) {
					send = true;
					play->input_text.len = 0;
					play->cursor.index.value = 0;
				}
				if(send) {
					buffer_write(&writer, c);
					num_chars_to_send += 1;
					play_sound_group(e_sound_group_click);
				}
			}

			*(int*)count_dst = num_chars_to_send;
			if(num_chars_to_send > 0) {
				platform_data->websocket_send(writer.buffer, writer.len);
			}

			#endif // m_emscripten

			float font_size0 = 48;
			float font_size1 = 24;

			draw_rect(g_r, c_play_area_center, 0, c_play_area_size, make_color(0.1f), game->render_pass);
			draw_rect(g_r, c_play_area_center, 1, c_our_area_size, make_color(0.167f, 0.731f, 0.882f, 0.5f), game->render_pass);
			g_r->end_render_pass(g_r, game->render_pass, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw words start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_len_str input = builder_to_len_str(&play->input_text);
				foreach_val(word_i, word, play->word_arr) {
					s_v2 pos;
					if(word.fall_timer < c_word_fall_time) {
						float lerped_timer = lerp(word.prev_fall_timer, word.fall_timer, interp_dt);
						float p = lerped_timer / c_word_fall_time;
						pos = lerp(word.pos - v2(0.0f, c_play_area_size.y), word.pos, p);
					}
					else {
						pos = lerp(word.prev_pos, word.pos, interp_dt);
					}
					s_len_str word2 = g_word_list[word.index];
					s_len_str match = find_longest_match(word2, input);
					s_v2 text_size = get_text_size(word2, game->font, font_size1);

					pos -= text_size * 0.5f;

					if(match.len > 0) {
						int start = (int)(match.str - input.str);
						int num_bad_chars = input.len - (start + match.len);
						s_len_str s0 = substr_from_to_exclusive(word2, 0, match.len);
						s_len_str s1 = substr_from_to_exclusive(word2, match.len, match.len + num_bad_chars);
						s_len_str s2 = substr_from_to_exclusive(word2, match.len + num_bad_chars, word2.len);
						if(s0.len > 0) {
							pos = draw_text(g_r, s0, pos, 0, font_size1, make_color(0, 1, 0), false, game->font, game->render_pass);
						}
						if(s1.len > 0) {
							pos = draw_text(g_r, s1, pos, 0, font_size1, make_color(1, 0, 0), false, game->font, game->render_pass);
						}
						if(s2.len > 0) {
							pos = draw_text(g_r, s2, pos, 0, font_size1, make_color(1), false, game->font, game->render_pass);
						}
					}
					else {
						draw_text(g_r, word2, pos, 0, font_size1, make_color(1), false, game->font, game->render_pass);
					}
				}
				g_r->end_render_pass(g_r, game->render_pass, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw words end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			draw_rect(g_r, v2(0.0f), 0, c_ui_size, make_color(0.15f), game->render_pass, {}, {.origin_offset = c_origin_topleft});
			g_r->end_render_pass(g_r, game->render_pass, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		display input start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				s_len_str str = builder_to_len_str(&play->input_text);
				if(str.len > 0) {
					draw_text(g_r, str, pxy(0.5f, 0.1f), 0, font_size0, make_color(1), true, game->font, game->render_pass);
				}

				draw_cool_cursor(
					pxy(0.5f, 0.1f), str, &play->cursor, font_size0
				);
				g_r->end_render_pass(g_r, game->render_pass, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		display input end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		ui names start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			s_pos_area area = make_vertical_layout(v2(4), v2(font_size1), 4, 0);
			foreach_val(client_i, client, game->client_arr) {
				if(client.in_play) {
					s_str_builder<64> builder = str_builder_to_builder<64>(&client.name);
					builder_add(&builder, ": %i", client.score);
					b8 is_this_my_client = game->my_index == client_i;
					s_v4 color = make_color(1);
					if(is_this_my_client) {
						color = make_color(0.438f, 0.239f, 0.652f);
					}
					draw_text(g_r, builder_to_len_str(&builder), pos_area_get_advance(&area), 0, font_size1, color, false, game->font, game->render_pass);
				}
			}
			g_r->end_render_pass(g_r, game->render_pass, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		ui names end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		round popup start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				float time_passed = game->render_time - play->set_round_timestamp;
				if(time_passed < 10) {
					s_animator a = zero;
					float alpha = 1;
					float temp_font_size = 0;
					add_float(&a, 0.0f, 64.0f, 0.2f, 0.0f, &temp_font_size, e_ease_out_back);
					animator_wait_completed(&a, 3.0f);
					add_float(&a, 64.0f, 0.0f, 0.5f, 0.0f, &temp_font_size, e_ease_linear);
					add_float(&a, 1.0f, 0.0f, 0.5f, 0.0f, &alpha, e_ease_linear);
					animator_wait_completed(&a, 0.0f);
					update_animator(&a, &time_passed, 1, false);

					if(temp_font_size > 0) {
						s_len_str str = format_text("Round %i", play->curr_round + 1);
						draw_text(g_r, str, pxy(0.5f, 0.3f), 0, temp_font_size, make_color(1.0f, alpha), true, game->font, game->render_pass);
						g_r->end_render_pass(g_r, game->render_pass, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		round popup end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			if(play->state == e_play_state_defeat) {
				s_len_str str = m_strlit("Defeat");
				draw_rect(g_r, c_play_area_center, 0, c_play_area_size, make_color(0.0f, 0.75f), game->render_pass);
				draw_text(g_r, str, pxy(0.5f, 0.3f), 0, 64, make_color(1.0f), true, game->font, game->render_pass);
				g_r->end_render_pass(g_r, game->render_pass, game->main_fbo, {.blend_mode = e_blend_mode_premultiply_alpha, .projection = ortho});
			}

		} break;
	}

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		draw start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

	g_r->clear_framebuffer(g_r->default_fbo, zero, c_default_fbo_clear_flags);
	draw_framebuffer(g_r, c_half_res, 0, c_base_res, make_color(1), game->main_fbo, game->render_pass);
	g_r->end_render_pass(g_r, game->render_pass, g_r->default_fbo, {.projection = ortho});
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		draw end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
		game->play.particle_arr.add_checked(p);
	}
}

// func void draw_tooltip(s_tooltip tooltip, s_len_str description, float font_size)
// {
// 	s_rectf panel = fit_rect(tooltip.pos, tooltip.size, c_base_res_bounds);
// 	tooltip.pos = panel.pos;
// 	tooltip.size = panel.size;
// 	draw_rect(g_r, tooltip.pos, 0, tooltip.size, hex_rgb_plus_alpha(0x9E8642, 0.85f), game->ui_render_pass2, {}, {.origin_offset = c_origin_topleft});
// 	draw_text(g_r, description, tooltip.text_pos, 0, font_size, make_color(1), false, game->font, game->ui_render_pass3);
// }

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

func b8 set_state_next_frame(e_state new_state)
{
	if(game->next_state >= 0) { return false; }


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

func void draw_light(s_v2 pos, float radius, s_v4 color, float smoothness)
{
	draw_circle(g_r, pos, 0, radius, color, game->render_pass, {.shader = 5, .circle_smoothness = smoothness});
}

func void draw_shadow(s_v2 pos, float radius, float strength, float smoothness)
{
	draw_circle(g_r, pos, e_layer_shadow, radius, make_color(strength), game->render_pass, {.shader = 5, .circle_smoothness = smoothness});
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

func s_particle_data multiply_particle_data(s_particle_data data, s_particle_multiplier multi)
{
	data.radius *= multi.radius;
	data.speed *= multi.speed;
	return data;
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

func s_v2 wxy(float x, float y)
{
	return c_base_res * v2(x, y);
}

func s_v2 pxy(float x, float y)
{
	return c_play_area_start + c_play_area_size * v2(x, y);
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

func void do_options_menu(b8 in_play_mode)
{
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

func void draw_progress_bar(s_v2 pos, s_v2 size, s_v4 under_size, s_v4 over_size, s_len_str str, float progress)
{
	draw_rect(g_r, pos, 0, size, under_size, game->render_pass, {}, {.origin_offset = c_origin_topleft});
	float width = progress * size.x;
	draw_rect(g_r, pos, 1, v2(width, size.y), over_size, game->render_pass, {}, {.origin_offset = c_origin_topleft});

	if(str.len > 0) {
		draw_text(
			g_r, str, pos + size * 0.5f + v2(0.0f, 3.0f), 0,
			24, make_color(1), true, game->font, game->render_pass
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

func int update_animator(s_animator* animator, float* time_ptr, float speed, b8 loop)
{
	assert(animator->step_count > 0);
	assert(speed > 0);

	#if defined(m_debug)
	assert(!animator->needs_wait_call);
	#endif // m_debug

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
	s_buffer_reader reader = make_buffer_reader(data, data_len);
	e_packet packet_type = buffer_read<e_packet>(&reader);

	s_play* play = &game->play;

	switch(packet_type) {
		case e_packet_name_is_good: {
			int index = buffer_read<int>(&reader);
			int name_len = buffer_read<int>(&reader);
			assert(name_len > 0);
			char* name_ptr = (char*)(reader.buffer + reader.cursor);

			game->client_arr[index].in_play = true;
			game->client_arr[index].name.from_data(name_ptr, name_len);
			game->input_name_state.waiting_for_server_response = false;

			printf("got name for %i: %i, %.*s\n", index, name_len, name_len, name_ptr);

			if(index == game->my_index) {
				set_state_next_frame(e_state_play);
				printf("it's my name!\n");
			}

		} break;

		case e_packet_new_word: {
			s_word new_word = zero;
			new_word.id = buffer_read<int>(&reader);
			new_word.index = buffer_read<int>(&reader);
			new_word.pos = buffer_read<s_v2>(&reader);
			new_word.dir = buffer_read<s_v2>(&reader);
			new_word.prev_pos = new_word.pos;
			game->play.word_arr.add(new_word);
		} break;

		case e_packet_delete_word: {
			int word_index = buffer_read<int>(&reader);
			int killer_index = buffer_read<int>(&reader);
			assert(word_index < play->word_arr.count);

			s_len_str word = g_word_list[play->word_arr[word_index].index];
			builder_remove_until_and_including(&play->input_text, word);
			play->cursor.index.value = at_most(play->input_text.len, play->cursor.index.value);
			play->word_arr.remove_and_swap(word_index);
			printf("deleted: %.*s\n", word.len, word.str);

			if(killer_index >= 0) {
				s_client* killer = get_client(killer_index);
				assert(killer); // @Note(tkap, 07/11/2024): not sure about this one. we'll see when we remove clients
				if(killer) {
					killer->score += word.len;
				}
			}
		} break;

		case e_packet_new_client: {
			s_client new_client = zero;
			b8 is_this_my_client = buffer_read<b8>(&reader);
			new_client.in_play = buffer_read<b8>(&reader);
			new_client.score = buffer_read<int>(&reader);
			int name_len = buffer_read<int>(&reader);
			if(name_len > 0) {
				char* name_ptr = (char*)(reader.buffer + reader.cursor);
				new_client.name.from_data(name_ptr, name_len);
			}

			printf("new client: %.*s\n", new_client.name.len, new_client.name.str);

			if(is_this_my_client) {
				game->my_index = game->client_arr.count;
				printf("it's my client! I'm index %i\n", game->my_index);
			}

			game->client_arr.add(new_client);
		} break;

		case e_packet_client_disconnected: {
			int index = buffer_read<int>(&reader);
			printf("%i disconnected, my index is %i\n", index, game->my_index);
			assert(index != game->my_index);

			b8 is_my_client_the_last_in_the_array = game->client_arr.count - 1 == game->my_index;
			game->client_arr.remove_and_swap(index);
			if(is_my_client_the_last_in_the_array) {
				game->my_index = index;
			}
		} break;

		case e_packet_set_round: {
			play->set_round_timestamp = game->render_time;
			play->curr_round = buffer_read<int>(&reader);
			printf("got round %i\n", play->curr_round);
		} break;

		case e_packet_set_lives_lost: {
			play->lives_lost = buffer_read<int>(&reader);
		} break;

		case e_packet_lose_life: {
			// @TODO(tkap, 09/11/2024): effect
			play_sound_group(e_sound_group_lose_life);
			play->lives_lost = buffer_read<int>(&reader);
		} break;

		case e_packet_defeat: {
			play_sound_group(e_sound_group_lose_life);
			play->state = e_play_state_defeat;
		} break;

		case e_packet_restart_game: {
			struct_memset_zero(play);
		} break;

		invalid_default_case;
	}
}
#endif // m_emscripten

func void draw_cool_cursor(
	s_v2 base_pos, s_len_str str, s_cool_cursor* cursor, float font_size
)
{
	s_v2 full_text_size = get_text_size(str, game->font, font_size);
	s_v2 partial_text_size = get_text_size_with_count(str, game->font, font_size, cursor->index.value);
	s_v2 cursor_pos = v2(
		-full_text_size.x * 0.5f + base_pos.x + partial_text_size.x,
		base_pos.y - font_size * 0.5f
	);

	s_v2 cursor_size = v2(15.0f, font_size);
	float t = game->render_time - max(cursor->last_action_time, cursor->last_edit_time);
	b8 blink = false;
	constexpr float c_blink_rate = 0.75f;
	if(t > 0.75f && fmodf(t, c_blink_rate) >= c_blink_rate / 2) {
		blink = true;
	}
	float t2 = clamp(game->render_time - cursor->last_edit_time, 0.0f, 1.0f);
	s_v4 color = lerp(rgb(0xffdddd), brighter(rgb(0xABC28F), 0.8f), 1 - powf(1 - t2, 3));
	float extra_height = ease_out_elastic2_advanced(t2, 0, 0.75f, 20, 0);
	cursor_size.y += extra_height;

	if(!cursor->initialized) {
		cursor->initialized = true;
		cursor->visual_pos = cursor_pos;
	}
	else {
		cursor->visual_pos = lerp_snap(cursor->visual_pos, cursor_pos, g_delta * 20);
	}

	if(!blink) {
		draw_rect(g_r, cursor->visual_pos - v2(0.0f, extra_height / 2), 15, cursor_size, color, game->render_pass, {}, {.origin_offset = c_origin_topleft});
	}
}

func s_client* get_client(int index)
{
	s_client* result = &game->client_arr[index];
	assert(result);
	return result;
}

func s_client* get_my_client()
{
	s_client* result = &game->client_arr[game->my_index];
	assert(result);
	return result;
}