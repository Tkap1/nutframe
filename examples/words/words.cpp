

#define m_game

#include "../../src/platform_shared.h"
#include "variables.h"
#include "common_words.h"


#define zero {}
#define func static
#define global static
#define null NULL


// static constexpr s_v2 c_base_res = {1920, 1080};
static constexpr s_v2 c_base_res = {1366, 768};
// static constexpr s_v2 c_base_res = {800, 600};
static constexpr s_v2 c_half_res = {c_base_res.x / 2.0f, c_base_res.y / 2.0f};
static constexpr f64 c_updates_per_second = 100.0;
static float c_delta = (float)(1.0 / c_updates_per_second);
static constexpr int c_max_words = 1024;
static constexpr float c_game_speeds[] = {0.0f, 0.1f, 0.25f, 0.5f, 1.0f, 2.0f, 4.0f, 8.0f};
static constexpr int c_max_player_name = 17;

constexpr char* c_game_modes_str[] = {
	"Normal", "snake_case", "camelCase",
};

constexpr float c_game_modes_spawn_delay[] = {
	1.0f, 3.0f, 3.0f,
};

constexpr float c_game_modes_font_size[] = {
	64.0f, 32.0f, 32.0f,
};

enum e_state
{
	e_state_game_mode,
	e_state_play,
	e_state_leaderboard,
};

enum e_layer
{
	e_layer_background,
	e_layer_particle,
	e_layer_word,
	e_layer_ui,
};

enum e_particle_spawn
{
	e_particle_spawn_point,
	e_particle_spawn_rect,
	e_particle_spawn_circle,
	e_particle_spawn_star,
};

enum e_game_mode
{
	e_game_mode_normal,
	e_game_mode_snake_case,
	e_game_mode_camel_case,
	e_game_mode_count,
};
static_assert(array_count(c_game_modes_str) == e_game_mode_count);

struct s_loop_timer
{
	float time;
	float duration;

	int update(float dt)
	{
		int result = 0;
		time += dt;
		while(time >= duration) {
			time -= duration;
			result += 1;
		}
		return result;
	}
};

struct s_leaderboard_entry
{
	int score;
	s_str<c_max_player_name> name;
};

func bool operator>(s_leaderboard_entry& a, s_leaderboard_entry& b)
{
	return b.score > a.score;
}

struct s_level_data
{
	int score;
	float spawn_speed_multiplier;
	s_carray<b8, c_max_words> words_active;
	s_carray<s_v2, c_max_words> words_size;
	s_carray<s_v2, c_max_words> prev_words_pos;
	s_carray<s_v2, c_max_words> words_pos;
	s_carray<s_str<64>, c_max_words> words_text;
	s_str<1024> text_input;
	s_loop_timer spawn_word_timer;
	s_sarray<int, array_count(c_common_words)> available_word_indices;
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
	e_game_mode mode;
	int speed_index;
	e_state state;
	int next_state;
	b8 reset_level;
	float total_time;
	s_level_data level_data;
	s_font* font;
	s_rng rng;
	b8 has_submitted_player_name;
	s_str<128> player_name;
	s_sarray<s_leaderboard_entry, 16> leaderboard_entries;
	s_texture atlas;
	s_carray<s_sound*, 3> pop_sounds;
	s_sarray<s_particle, 16384> particles;
	s_framebuffer* particle_framebuffer;
};

global s_input* g_input;
global s_game* game;
global s_game_renderer* g_r;
global s_v2 g_mouse;

func void set_globals(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, s_input* input);
func int make_word(char* text, s_v2 pos);
func void prepare_random_words();
func float get_best_new_word_y_pos(s_rng* rng, float font_size);
func float get_game_speed_multiplier();
func char* get_word_str_for_game_mode(e_game_mode mode, s_rng* rng);
func void do_particles(int count, s_v2 pos, s_v2 pos2, e_particle_spawn spawn_type, s_particle_data data);
func float get_font_size_for_mode(e_game_mode mode);
func int pick_from_weights(s64* weights, int count, s_rng* rng);
static void send_message(char* text);

#ifdef __EMSCRIPTEN__
global b8 g_want_to_send_data = false;
global b8 g_received_message = false;
global char g_socket_buffer[16384];
#endif // __EMSCRIPTEN__

template <int n>
func s_str<n> make_str(char* text);

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
	static_assert(sizeof(s_game) <= c_game_memory);
	set_globals(platform_data, game_memory, renderer, &platform_data->logic_input);

	if(!game->initialized) {
		game->initialized = true;
		platform_data->variables_path = "examples/words/variables.h";
		game->font = &renderer->fonts[0];
		game->rng.seed = platform_data->get_random_seed();
		game->reset_level = true;
		game->speed_index = 4;
		game->next_state = -1;
		game->pop_sounds[0] = platform_data->load_sound(platform_data, "examples/words/pop.wav", platform_data->frame_arena);
		game->pop_sounds[1] = platform_data->load_sound(platform_data, "examples/words/pop2.wav", platform_data->frame_arena);
		game->pop_sounds[2] = platform_data->load_sound(platform_data, "examples/words/pop3.wav", platform_data->frame_arena);
		game->particle_framebuffer = g_r->make_framebuffer(renderer, false);
		renderer->set_vsync(true);
	}

	c_delta = (float)(1.0 / c_updates_per_second) * get_game_speed_multiplier();

	s_level_data* ld = &game->level_data;

	if(game->next_state != -1) {
		game->state = (e_state)game->next_state;
		game->next_state = -1;
		if(game->state == e_state_play) {
			game->reset_level = true;
		}
	}

	if(game->reset_level) {
		game->reset_level = false;
		memset(ld, 0, sizeof(s_level_data));

		prepare_random_words();
	}


	#ifdef __EMSCRIPTEN__
	if(g_want_to_send_data) {
		char* text = format_text("%i:%s:%i", game->mode, game->player_name.data, ld->score);
		send_message(text);
	}

	if(g_received_message) {
		g_received_message = false;
		game->leaderboard_entries.count = 0;
		char* cursor = g_socket_buffer;
		int start = 0;
		int index = 0;
		b8 done_parsing = false;
		while(true) {
			while(cursor[index] != ':') {
				if(!cursor[index]) {
					done_parsing = true;
					break;
				}
				index += 1;
			}
			if(done_parsing) { break; }
			s_str<c_max_player_name> name = zero;
			memcpy(name.data, &cursor[start], index - start);
			start = index + 1;
			while(cursor[index] && cursor[index] != '\n') {
				index += 1;
			}
			int score = atoi(&cursor[start]);
			start = index + 1;
			game->leaderboard_entries.add({.score = score, .name = name});
		}
		game->leaderboard_entries.small_sort();
	}

	#endif // __EMSCRIPTEN__

	#if 0
	{
		int add = 0;
		if(is_key_pressed(g_input, c_key_add)) {
			add += 1;
		}
		if(is_key_pressed(g_input, c_key_subtract)) {
			add -= 1;
		}
		game->speed_index = circular_index(game->speed_index + add, array_count(c_game_speeds));
	}
	#endif

	switch(game->state) {

		case e_state_game_mode: {
		} break;

		case e_state_play: {
			memcpy(&ld->prev_words_pos[0], &ld->words_pos[0], c_max_words * sizeof(ld->words_pos[0]));

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		handle input start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				foreach_val(c_i, c, g_input->char_events) {
					if(c == '\b' && ld->text_input.len > 0) {
						ld->text_input.len -= 1;
						ld->text_input.data[ld->text_input.len] = 0;
					}
					else if(is_alpha_numeric(c) || c == '_' || c == '\'') {
						ld->text_input.data[ld->text_input.len] = c;
						ld->text_input.len += 1;
						ld->text_input.data[ld->text_input.len] = 0;

						// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		remove completed words start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
						{
							int score_to_add = 0;
							for(int word_i = 0; word_i < c_max_words; word_i++) {
								if(!ld->words_active[word_i]) { continue; }
								if(ld->text_input.len < ld->words_text[word_i].len) { continue; }

								b8 word_matches = true;
								for(int text_i = 0; text_i < ld->words_text[word_i].len; text_i++) {
									int input_index = ld->text_input.len - ld->words_text[word_i].len + text_i;
									char input_c = ld->text_input[input_index];
									char text_c = ld->words_text[word_i][text_i];
									if(text_c != input_c) {
										word_matches = false;
										break;
									}
								}
								if(word_matches) {
									ld->words_active[word_i] = false;
									score_to_add += ld->words_text[word_i].len;

									s_v2 text_size = ld->words_size[word_i];
									do_particles(100, ld->words_pos[word_i] + text_size * 0.5f, text_size, e_particle_spawn_rect, {
										.shrink = 0.5f,
										.slowdown = 1.0f,
										.duration = 0.5f,
										.duration_rand = 1,
										.speed = 100 * (1 + ld->words_text[word_i].len / 10.0f),
										.speed_rand = 1,
										.angle_rand = 1,
										.radius = 4 * 2.0f,
										.radius_rand = 0,
										.color = v3(0.25f),
										.color_rand = v3(1, 1, 1),
									});
								}
							}
							if(score_to_add > 0) {
								ld->text_input.len = 0;
								ld->score += score_to_add;
								platform_data->play_sound(game->pop_sounds[game->rng.randu() % game->pop_sounds.max_elements()]);
							}
						}
						// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		remove completed words end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^
					}
				}

				if(is_key_pressed(g_input, c_key_backspace) && is_key_down(g_input, c_key_left_ctrl)) {
					ld->text_input.len = 0;
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		handle input end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		spawn words start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				ld->spawn_word_timer.duration = c_game_modes_spawn_delay[game->mode];
				int count = ld->spawn_word_timer.update(c_delta * (1.0f + ld->spawn_speed_multiplier));
				for(int i = 0; i < count; i++) {
					make_word(
						get_word_str_for_game_mode(game->mode, &game->rng),
						v2(c_base_res.x, get_best_new_word_y_pos(&game->rng, get_font_size_for_mode(game->mode)))
					);
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		spawn words end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update words start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
			{
				for(int word_i = 0; word_i < c_max_words; word_i++) {
					if(!ld->words_active[word_i]) { continue; }
					ld->words_pos[word_i].x -= c_delta * 100;

					if(ld->words_pos[word_i].x + ld->words_size[word_i].x < 0) {
						ld->words_active[word_i] = false;
						if(ld->score > 0) {
							if(game->has_submitted_player_name) {
									#ifdef __EMSCRIPTEN__
									send_message(format_text("%i:%s:%i", game->mode, game->player_name.data, ld->score));
									#endif // __EMSCRIPTEN__
							}
							game->next_state = e_state_leaderboard;
						}
						else {
							game->next_state = e_state_play;
						}
					}
				}
			}
			// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update words end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

			ld->spawn_speed_multiplier += 1 / 60.0f * c_delta;


		} break;

		case e_state_leaderboard: {
			if(!game->has_submitted_player_name) {
				foreach_val(c_i, c, g_input->char_events) {
					if(c == '\b' && game->player_name.len > 0) {
						game->player_name.len -= 1;
						game->player_name.data[game->player_name.len] = 0;
					}
					else if(is_alpha_numeric(c) || c == '_' || c == '\'' && game->player_name.len < c_max_player_name - 1) {
						game->player_name.data[game->player_name.len] = c;
						game->player_name.len += 1;
						game->player_name.data[game->player_name.len] = 0;
					}
				}
				if(is_key_pressed(g_input, c_key_enter) && game->player_name.len >= 3) {
					game->has_submitted_player_name = true;
					#ifdef __EMSCRIPTEN__
					send_message(format_text("%i:%s:%i", game->mode, game->player_name.data, ld->score));
					#endif // __EMSCRIPTEN__
				}
			}
			else {
				if(is_key_pressed(g_input, c_key_enter)) {
					game->next_state = e_state_play;
				}
			}
		} break;
	}

	game->total_time += c_delta;

}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt)
{
	set_globals(platform_data, game_memory, renderer, &platform_data->render_input);

	live_variable(&platform_data->vars, c_font_size, 4.0f, 128.0f, true);

	draw_rect(g_r, c_half_res, 0, c_base_res, make_color(0.15f));

	if(is_key_down(g_input, c_left_mouse)) {
	// if(is_key_pressed(g_input, c_right_mouse)) {
		do_particles(100, v2(g_mouse.x, g_mouse.y), v2(512), e_particle_spawn_star, {
			.shrink = 0.5f,
			.slowdown = 1.0f,
			.duration = 2.5f,
			.duration_rand = 1,
			.speed = 10,
			.speed_rand = 1,
			.angle_rand = 1,
			.radius = 4 * 2.0f,
			.radius_rand = 0,
			.color = v3(0.25f),
			.color_rand = v3(1, 0, 1),
		});
	}

	switch(game->state) {

		case e_state_game_mode: {
			float button_width = 400.0f;
			s_v2 pos = v2(c_half_res.x - button_width / 2, c_half_res.y - c_font_size * 2);
			for(int mode_i = 0; mode_i < e_game_mode_count; mode_i++) {
				s_ui_interaction interaction = platform_data->ui_button(
					g_r, c_game_modes_str[mode_i], pos, v2(button_width, c_font_size), game->font, c_font_size, g_input, g_mouse
				);
				if(interaction.state == e_ui_active) {
					game->mode = (e_game_mode)mode_i;
					game->next_state = e_state_play;
				}
				pos.y += c_font_size + 8;
			}
		} break;

		case e_state_play: {
			s_level_data* ld = &game->level_data;

			if(ld->text_input.len > 0) {
				draw_text(g_r, ld->text_input.data, c_half_res, 1, c_font_size, rgb(0xFCF5D5), true, game->font);
			}
			draw_text(g_r, format_text("%i", ld->score), c_base_res * v2(0.5f, 0.1f), 1, c_font_size, rgb(0xFCF5D5), true, game->font);

			struct s_sub_str
			{
				int start;
				int end;
				s_v4 color;
			};
			for(int word_i = 0; word_i < c_max_words; word_i++) {
				if(!ld->words_active[word_i]) { continue; }
				s_sarray<s_sub_str, c_max_words * 3> sub_strs;
				s_v2 base_pos = lerp(ld->prev_words_pos[word_i], ld->words_pos[word_i], interp_dt);

				b8 match_something = false;
				int start = 0;
				if(ld->text_input.len > 0) {
					for(int text_i = 0; text_i < ld->words_text[word_i].len; text_i++) {
						int input_index = at_least(0, ld->text_input.len - ld->words_text[word_i].len) + text_i;
						char input_c = ld->text_input[input_index];
						char text_c = ld->words_text[word_i][text_i];
						if(text_c != input_c) {
							if(match_something) {
								sub_strs.add({.start = 0, .end = text_i - 1, .color = make_color(0, 1, 0)});
								if(text_i > 0 && input_c != 0) {
									sub_strs.add({.start = text_i, .end = text_i, .color = make_color(1, 0, 0)});
									start = text_i + 1;
								}
								else {
									start = text_i;
								}
							}
							break;
						}
						match_something = true;
					}
				}
				sub_strs.add({.start = start, .end = ld->words_text[word_i].len - 1, .color = make_color(1)});

				s_v2 pos = base_pos;
				foreach_val(sub_str_i, sub_str, sub_strs) {
					if(sub_str.start > sub_str.end) { continue; }
					char buffer[128] = zero;
					memcpy(buffer, &ld->words_text[word_i][sub_str.start], (sub_str.end - sub_str.start) + 1);
					pos = draw_text(g_r, buffer, pos, 1, get_font_size_for_mode(game->mode), sub_str.color, false, game->font);
				}
			}

		} break;

		case e_state_leaderboard: {
			if(!game->has_submitted_player_name) {
				draw_text(g_r, "Enter name...", c_half_res * v2(1.0f, 0.85f), 1, c_font_size, make_color(1), true, game->font);
				if(game->player_name.len > 0) {
					draw_text(g_r, game->player_name.data, c_half_res, 1, c_font_size, make_color(1), true, game->font);
				}
			}
			else {
				draw_text(g_r, "Press ENTER to play again...", c_base_res * v2(0.5f, 0.05f), 1, c_font_size, make_color(1), true, game->font);
				float name_x = c_base_res.x * 0.1f;
				float score_x = c_base_res.x * 0.6f;
				float y = 150;
				for(int entry_i = 0; entry_i < game->leaderboard_entries.count; entry_i++) {
					draw_text(g_r, game->leaderboard_entries[entry_i].name.data, v2(name_x, y), 1, c_font_size * 0.75f, make_color(1), false, game->font);
					draw_text(g_r, format_text("%i", game->leaderboard_entries[entry_i].score), v2(score_x, y), 1, c_font_size * 0.75f, make_color(1), false, game->font);
					y += c_font_size * 0.75f + 8;
				}
			}
		} break;

	}

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


func int make_word(char* text, s_v2 pos)
{
	s_level_data* ld = &game->level_data;
	for(int word_i = 0; word_i < c_max_words; word_i++) {
		if(ld->words_active[word_i]) { continue; }
		ld->words_active[word_i] = true;
		ld->words_text[word_i] = make_str<64>(text);
		ld->prev_words_pos[word_i] = pos;
		ld->words_pos[word_i] = pos;
		ld->words_size[word_i] = get_text_size(text, game->font, get_font_size_for_mode(game->mode));
		return word_i;
	}
	assert(false);
	return -1;
}

template <int n>
func s_str<n> make_str(char* text)
{
	s_str<n> str;
	int len = (int)strlen(text);
	assert(len < n);
	str.len = len;
	memcpy(str.data, text, len);
	str.data[len] = 0;
	return str;
}

func void prepare_random_words()
{
	s_level_data* ld = &game->level_data;
	for(int i = 0; i < array_count(c_common_words); i++) {
		ld->available_word_indices.add(i);
	}
	ld->available_word_indices.shuffle(&game->rng);
}

func float get_best_new_word_y_pos(s_rng* rng, float font_size)
{
	s_level_data* ld = &game->level_data;
	float best_y = 0;
	for(int i = 0; i < 1000; i++) {
		best_y = rng->randf_range(0.0f, c_base_res.y - font_size);
		b8 too_close_to_other_word = false;
		for(int word_i = 0; word_i < c_max_words; word_i++) {
			if(!ld->words_active[word_i]) { continue; }

			float text_end_x = ld->words_pos[word_i].x + ld->words_size[word_i].x;
			if(text_end_x < c_base_res.x - font_size * 2) { continue; }

			float y_dist = fabsf(ld->words_pos[word_i].y + font_size / 2 - (best_y + font_size / 2));
			if(y_dist > font_size * 1.1f) { continue; }

			too_close_to_other_word = true;
			break;
		}
		if(!too_close_to_other_word) { break; }
	}
	return best_y;
}

func float get_game_speed_multiplier()
{
	return c_game_speeds[game->speed_index];
}

func char* get_word_str_for_game_mode(e_game_mode mode, s_rng* rng)
{
	s_level_data* ld = &game->level_data;
	switch(mode) {
		case e_game_mode_normal: {
			if(ld->available_word_indices.count <= 0) {
				prepare_random_words();
			}
			return c_common_words[ld->available_word_indices.pop()];
		} break;

		case e_game_mode_snake_case: {
			s_str_builder<1024> result;
			s64 weights[] = {1000, 700, 200};
			int options[] = {2, 3, 4};
			int num_words = options[pick_from_weights(weights, array_count(weights), rng)];
			for(int i = 0; i < num_words; i++) {
				if(ld->available_word_indices.count <= 0) {
					prepare_random_words();
				}
				result.add("%s", c_common_words[ld->available_word_indices.pop()]);
				if(i < num_words - 1) {
					result.add("_");
				}
			}
			return format_text("%s", result.data);

		} break;

		case e_game_mode_camel_case: {
			s_str_builder<1024> result;

			s64 weights[] = {1000, 700, 200};
			int options[] = {2, 3, 4};
			int num_words = options[pick_from_weights(weights, array_count(weights), rng)];

			for(int i = 0; i < num_words; i++) {
				if(ld->available_word_indices.count <= 0) {
					prepare_random_words();
				}
				int len_before = result.len;
				result.add("%s", c_common_words[ld->available_word_indices.pop()]);
				if(i > 0) {
					result.data[len_before] = to_upper_case(result.data[len_before]);
				}
			}
			return format_text("%s", result.data);
		} break;
	}

	return null;
}

func float star(s_v2 uv)
{
	float foo = fabsf(uv.x * uv.y);
	s_v2 uv2 = v2_rotated(uv, pi * 0.25f);
	float bar = fabsf(uv2.x * uv2.y);
	foo = smoothstep(0.05f, 0.0f, foo);
	bar = smoothstep(0.05f, 0.0f, bar);
	float c = foo * 0.5f + bar * 0.5f;
	c *= smoothstep(0.8f, 0.4f, fabsf(uv.y));
	c *= smoothstep(0.8f, 0.4f, fabsf(uv.x));
	return c;
}

func void do_particles(int count, s_v2 pos, s_v2 pos2, e_particle_spawn spawn_type, s_particle_data data)
{
	s_rng* rng = &game->rng;
	for(int particle_i = 0; particle_i < count; particle_i++) {
		s_particle p = zero;
		switch(spawn_type) {
			case e_particle_spawn_point: {
				p.pos = pos;
			} break;

			case e_particle_spawn_star: {
				s_v2 centered_pos = pos - pos2 * 0.5f;
				s_v2 r;
				while(true) {
					s_v2 r2 = random_point_rect_topleft(centered_pos, pos2, &game->rng);
					r.x = ((r2.x - centered_pos.x) / pos2.x) * 2.0f - 1.0f;
					r.y = ((r2.y - centered_pos.y) / pos2.y) * 2.0f - 1.0f;
					float s = star(r);
					if(s > 0.1) {
						p.pos = r2;
						break;
					}
				}
			} break;

			case e_particle_spawn_rect: {
				p.pos = random_point_rect_topleft(pos - pos2 * 0.5f, pos2, &game->rng);
			} break;

			case e_particle_spawn_circle: {
				while(true) {
					s_v2 centered_pos = pos - pos2 * 0.5f;
					s_v2 r = random_point_rect_topleft(centered_pos, pos2, &game->rng);
					if(rect_collides_circle_topleft(r, v2(1), pos, pos2.x * 0.5f)) {
						p.pos = r;
						break;
					}
				}

			} break;

			invalid_default_case;
		}
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

func float get_font_size_for_mode(e_game_mode mode)
{
	return c_game_modes_font_size[mode];
}

func int pick_from_weights(s64* weights, int count, s_rng* rng)
{
	s64 total = 0;
	for(int i = 0; i < count; i++)
	{
		s64 weight = weights[i];
		total += weight;
	}
	for(int i = 0; i < count; i++)
	{
		s64 weight = weights[i];
		s64 rand = rng->randu() % total;
		if(rand < weight) { return i; }
		total -= weight;
	}
	return -1;
}


#ifdef __EMSCRIPTEN__
#undef func
#include <emscripten/emscripten.h>
#include <emscripten/websocket.h>


global int g_socket = -1;
global EMSCRIPTEN_WEBSOCKET_T g_ws;
EM_BOOL onopen(int eventType, const EmscriptenWebSocketOpenEvent *websocketEvent, void *userData) {
	puts("onopen");
	g_socket = websocketEvent->socket;
	return EM_TRUE;
}
EM_BOOL onerror(int eventType, const EmscriptenWebSocketErrorEvent *websocketEvent, void *userData) {
	puts("onerror");
	return EM_TRUE;
}
EM_BOOL onclose(int eventType, const EmscriptenWebSocketCloseEvent *websocketEvent, void *userData) {
	puts("onclose");
	return EM_TRUE;
}
EM_BOOL onmessage(int eventType, const EmscriptenWebSocketMessageEvent *websocketEvent, void *userData) {
	puts("onmessage");
	if (websocketEvent->isText) {
		// For only ascii chars.
		g_received_message = true;
		strcpy(g_socket_buffer, (char*)websocketEvent->data);
		printf("message: %s\n", websocketEvent->data);
	}
	g_socket = websocketEvent->socket;

	return EM_TRUE;
}

static void send_message(char* text)
{
	if(g_socket == -1 && !g_want_to_send_data) {
		printf("INIT WEBSOCKET GARBAGE\n");
		EmscriptenWebSocketCreateAttributes ws_attrs = {
			"ws://try-explains.gl.at.ply.gg:20119",
			NULL,
			EM_TRUE
		};
		g_ws = emscripten_websocket_new(&ws_attrs);
		emscripten_websocket_set_onopen_callback(g_ws, NULL, onopen);
		emscripten_websocket_set_onerror_callback(g_ws, NULL, onerror);
		emscripten_websocket_set_onclose_callback(g_ws, NULL, onclose);
		emscripten_websocket_set_onmessage_callback(g_ws, NULL, onmessage);
	}

	g_want_to_send_data = true;
	if(g_socket != -1) {
		printf("SENDING: %s\n", text);
		emscripten_websocket_send_utf8_text(g_socket, text);
		g_want_to_send_data = false;
	}


}
#endif // __EMSCRIPTEN__