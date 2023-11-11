

#define m_game

#include "../../src/platform_shared.h"
#include "variables.h"
#include "common_words.h"


#define zero {}
#define func static
#define global static
#define null NULL


static constexpr s_v2 c_base_res = {1920, 1080};
static constexpr s_v2 c_half_res = {c_base_res.x / 2.0f, c_base_res.y / 2.0f};
static constexpr f64 c_updates_per_second = 100.0;
static constexpr float c_delta = (float)(1.0 / c_updates_per_second);

static constexpr int c_max_words = 1024;

template <int n>
struct s_str
{
	int len;
	char data[n];

	char& operator[](int i)
	{
		assert(i >= 0);
		assert(i <= len);
		return data[i];
	}
};

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

struct s_level_data
{
	int score;
	float spawn_speed_multiplier;
	s_carray<b8, c_max_words> words_active;
	s_carray<float, c_max_words> words_width;
	s_carray<s_v2, c_max_words> prev_words_pos;
	s_carray<s_v2, c_max_words> words_pos;
	s_carray<s_str<16>, c_max_words> words_text;
	s_str<1024> text_input;
	s_loop_timer spawn_word_timer;
	s_sarray<int, array_count(c_common_words)> available_word_indices;
};

struct s_game
{
	b8 initialized;
	b8 reset_level;
	float total_time;
	s_level_data level_data;
	s_font* font;
	s_rng rng;
};

static s_input* g_input;
static s_game* game;
static s_game_renderer* g_r;
static s_v2 g_mouse;

func void set_globals(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, s_input* input);
func int make_word(char* text, s_v2 pos);
func void prepare_random_words();
func float get_best_new_word_y_pos(s_rng* rng);

template <int n>
func s_str<n> make_str(char* text);

#ifdef m_build_dll
extern "C" {
#endif // m_build_dll
m_dll_export void init_game(s_platform_data* platform_data)
{
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
	}

	s_level_data* ld = &game->level_data;

	if(game->reset_level) {
		game->reset_level = false;
		memset(ld, 0, sizeof(s_level_data));

		prepare_random_words();
	}

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
			}
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		handle input end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

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
			}
		}
		if(score_to_add > 0) {
			ld->text_input.len = 0;
			ld->score += score_to_add;
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		remove completed words end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		spawn words start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		ld->spawn_word_timer.duration = 1.0f;
		int count = ld->spawn_word_timer.update(c_delta * (1.0f + ld->spawn_speed_multiplier));
		for(int i = 0; i < count; i++) {
			if(ld->available_word_indices.count <= 0) {
				prepare_random_words();
			}
			make_word(
				c_common_words[ld->available_word_indices.pop()],
				v2(c_base_res.x, get_best_new_word_y_pos(&game->rng))
			);
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		spawn words end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		update words start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		for(int word_i = 0; word_i < c_max_words; word_i++) {
			if(!ld->words_active[word_i]) { continue; }
			ld->words_pos[word_i].x -= c_delta * 100;

			if(ld->words_pos[word_i].x + ld->words_width[word_i] < 0) {
				ld->words_active[word_i] = false;
				// @TODO(tkap, 11/11/2023): lose
			}
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		update words end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	ld->spawn_speed_multiplier += 1 / 60.0f * c_delta;

	game->total_time += c_delta;

}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt)
{
	set_globals(platform_data, game_memory, renderer, &platform_data->render_input);

	live_variable(&platform_data->vars, c_font_size, 4.0f, 128.0f, true);

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
			pos = draw_text(g_r, buffer, pos, 1, c_font_size, sub_str.color, false, game->font);
		}
	}
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
		ld->words_text[word_i] = make_str<16>(text);
		ld->prev_words_pos[word_i] = pos;
		ld->words_pos[word_i] = pos;
		ld->words_width[word_i] = get_text_size(text, game->font, c_font_size).x;
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

func float get_best_new_word_y_pos(s_rng* rng)
{
	s_level_data* ld = &game->level_data;
	float best_y = 0;
	for(int i = 0; i < 100; i++) {
		best_y = rng->randf_range(0.0f, c_base_res.y - c_font_size);
		b8 too_close_to_other_word = false;
		for(int word_i = 0; word_i < c_max_words; word_i++) {
			if(!ld->words_active[word_i]) { continue; }

			float text_end_x = ld->words_pos[word_i].x + get_text_size(ld->words_text[word_i].data, game->font, c_font_size).x;
			if(text_end_x < c_base_res.x - c_font_size * 2) { continue; }

			float y_dist = fabsf(ld->words_pos[word_i].y + c_font_size / 2 - (best_y + c_font_size / 2));
			if(y_dist > c_font_size * 2) { continue; }

			too_close_to_other_word = true;
			break;
		}
		if(!too_close_to_other_word) { break; }
	}
	return best_y;
}