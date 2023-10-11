
// #include "pch_client.h"
#include "../src/pch_client.h"

#include "../src/config.h"
#include "../src/bucket.h"
#include "../src/platform_shared.h"

global constexpr int c_tile_size = 64;
global constexpr int c_tile_count = 12;
global constexpr int c_max_snake_len = c_tile_count * c_tile_count / 2;
global constexpr float c_move_delay = 0.15f;

struct s_snake
{
	s_v2i pos;
	float rotation;
};

struct s_game
{
	b8 initialized;
	s_texture snake_head;
	s_texture snake_body;
	s_texture snake_tail;
	s_texture apple_texture;
	s_sarray<s_v2i, 4> inputs;
	s_v2i last_dir;
	s_snake snake[c_max_snake_len];
	int snake_len;
	float move_timer;
	s_v2i apple;
	s_rng rng;
	b8 reset_level;
};

global s_lin_arena* frame_arena;
global s_game_window g_window;
global s_input* g_logic_input;
global s_input* g_input;
global s_platform_data* g_platform_data;
global s_platform_funcs g_platform_funcs;
global s_game* game;
global s_game_renderer* g_r;

func b8 is_key_down(s_input* input, int key);
func b8 is_key_up(s_input* input, int key);
func b8 is_key_pressed(s_input* input, int key);
func b8 is_key_released(s_input* input, int key);
func s_v2i spawn_apple();
func void reset_level();

#include "../src/draw.cpp"
#include "../src/memory.cpp"
#include "../src/bucket.cpp"

#ifdef m_build_dll
extern "C" {
__declspec(dllexport)
#endif // m_build_dll
m_update_game(update_game) {
	static_assert(sizeof(s_game) <= c_game_memory);

	game = (s_game*)game_memory;
	g_r = rendering;
	g_input = platform_data->input;
	if(!game->initialized) {
		game->initialized = true;
		game->rng.seed = platform_data->get_random_seed();
		g_r->set_vsync(true);
		game->snake_head = g_r->load_texture(rendering, "examples/snake_head.png");
		game->snake_body = g_r->load_texture(rendering, "examples/snake_body.png");
		game->snake_tail = g_r->load_texture(rendering, "examples/snake_tail.png");
		game->apple_texture = g_r->load_texture(rendering, "examples/apple.png");
		game->reset_level = true;
	}

	if(game->reset_level) {
		game->reset_level = false;
		game->snake[0] = zero;
		game->last_dir = v2i(1, 0);
		game->snake_len = 1;
		game->apple = spawn_apple();
		game->move_timer = 0;
	}

	if(is_key_pressed(g_input, c_key_left)) {
		game->inputs.add_checked(v2i(-1, 0));
	}
	if(is_key_pressed(g_input, c_key_right)) {
		game->inputs.add_checked(v2i(1, 0));
	}
	if(is_key_pressed(g_input, c_key_up)) {
		game->inputs.add_checked(v2i(0, -1));
	}
	if(is_key_pressed(g_input, c_key_down)) {
		game->inputs.add_checked(v2i(0, 1));
	}

	game->move_timer += (float)platform_data->frame_time;

	if(game->move_timer >= c_move_delay) {
		game->move_timer -= c_move_delay;
		s_v2i dir = game->last_dir;
		foreach_raw(input_i, input, game->inputs)
		{
			if(input.x != 0 && game->last_dir.x != 0) {
				game->inputs.remove_and_shift(input_i--);
				continue;
			}
			if(input.y != 0 && game->last_dir.y != 0) {
				game->inputs.remove_and_shift(input_i--);
				continue;
			}
			dir = input;
			game->inputs.remove_and_shift(input_i--);
			break;
		}

		s_snake head = game->snake[0];
		head.pos += dir;
		head.pos.x = circular_index(head.pos.x, c_tile_count);
		head.pos.y = circular_index(head.pos.y, c_tile_count);
		head.rotation = v2_angle(v2(dir));

		game->last_dir = dir;

		if(head.pos == game->apple) {
			if(game->snake_len >= c_max_snake_len) {
				// @TODO(tkap, 11/10/2023): Victory?
			}
			else {
				game->snake_len += 1;
				game->apple = spawn_apple();
			}
		}
		for(int snake_i = game->snake_len - 1; snake_i > 0; snake_i--)
		{
			game->snake[snake_i] = game->snake[snake_i - 1];
		}

		for(int snake_i = 0; snake_i < game->snake_len; snake_i++) {
			if(head.pos == game->snake[snake_i].pos) {
				game->reset_level = true;
			}
		}
		game->snake[0] = head;
	}

	for(int snake_i = 0; snake_i < game->snake_len; snake_i++)
	{
		s_texture texture;
		if(snake_i == 0) { texture = game->snake_head; }
		else if(snake_i == game->snake_len - 1) { texture = game->snake_tail; }
		else { texture = game->snake_body; }
		s_snake s = game->snake[snake_i];
		draw_texture(v2(s.pos * c_tile_size), 1, v2(c_tile_size), make_color(1), texture, {.rotation = s.rotation, .origin_offset = c_origin_topleft});
	}

	draw_texture(v2(game->apple * c_tile_size), 0, v2(c_tile_size), make_color(1), game->apple_texture, {.origin_offset = c_origin_topleft});

	for(int i = 0; i < c_max_keys; i++) {
		g_input->keys[i].count = 0;
	}

}

#ifdef m_build_dll
}
#endif // m_build_dll

func void update() {

}

func void render(float dt) {
}

func b8 is_key_down(s_input* input, int key) {
	assert(key < c_max_keys);
	return input->keys[key].is_down || input->keys[key].count >= 2;
}

func b8 is_key_up(s_input* input, int key) {
	assert(key < c_max_keys);
	return !input->keys[key].is_down;
}

func b8 is_key_pressed(s_input* input, int key) {
	assert(key < c_max_keys);
	return (input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

func b8 is_key_released(s_input* input, int key) {
	assert(key < c_max_keys);
	return (!input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

func s_v2i spawn_apple() {
	s_v2i pos;
	while(true) {
		pos = v2i(game->rng.randu() % c_tile_count, game->rng.randu() % c_tile_count);
		b8 collision = false;
		for(int snake_i = 0; snake_i < game->snake_len; snake_i++) {
			if(game->snake[snake_i].pos == pos) {
				collision = true;
				break;
			}
		}
		if(!collision) { break; }
	}
	return pos;
}