
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
	s_framebuffer* particle_framebuffer;
	s_framebuffer* text_framebuffer;
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
	s_font* font;
	s_sound* sound;
};

global s_input* g_input;
global s_game* game;
global s_game_renderer* g_r;
global s_v2 mouse;

func b8 is_key_down(s_input* input, int key);
func b8 is_key_up(s_input* input, int key);
func b8 is_key_pressed(s_input* input, int key);
func b8 is_key_released(s_input* input, int key);
func s_v2i spawn_apple();
func void reset_level();

#include "../src/draw.cpp"
#include "../src/memory.cpp"
#include "../src/bucket.cpp"
#include "../src/platform_shared.cpp"

#ifdef m_build_dll
extern "C" {
m_dll_export
#endif // m_build_dll
m_update_game(update_game)
{
	static_assert(sizeof(s_game) <= c_game_memory);

	mouse = platform_data->mouse;

	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = platform_data->input;
	if(!game->initialized) {
		game->initialized = true;
		game->rng.seed = platform_data->get_random_seed();
		g_r->set_vsync(true);
		game->snake_head = g_r->load_texture(renderer, "examples/snake_head.png");
		game->snake_body = g_r->load_texture(renderer, "examples/snake_body.png");
		game->snake_tail = g_r->load_texture(renderer, "examples/snake_tail.png");
		game->apple_texture = g_r->load_texture(renderer, "examples/apple.png");
		game->font = g_r->load_font(renderer, "examples/consola.ttf", 64, platform_data->frame_arena);
		game->particle_framebuffer = g_r->make_framebuffer(renderer, false);
		game->text_framebuffer = g_r->make_framebuffer(renderer, false);
		game->sound = platform_data->load_sound(platform_data, "examples/sound.wav", platform_data->frame_arena);
		game->reset_level = true;
	}

	if(is_key_pressed(g_input, c_key_f)) {
		platform_data->play_sound(game->sound);
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
		foreach_val(input_i, input, game->inputs)
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
		draw_texture(
			v2(s.pos * c_tile_size), 1, v2(c_tile_size), make_color(1), texture, zero, {.rotation = s.rotation, .origin_offset = c_origin_topleft}
		);
	}

	draw_texture(
		v2(game->apple * c_tile_size), 0, v2(c_tile_size), make_color(1), game->apple_texture, zero, {.origin_offset = c_origin_topleft}
	);

	static int count = 0;
	int seed = 0;
	count += 1;
	s_rng rng = zero;
	rng.seed = seed;
	for(int i = 0; i < count; i++) {
		float angle = rng.randf32() * tau;
		float foo = 1-i/(float)count;
		s_v2 vel = v2_from_angle(angle) * (count / 10.0f) * (rng.randf32() + 0.01f) * foo;
		float r = foo * 1;
		draw_rect(
			c_half_res + vel, 1, v2(4 * r), make_color(powf(rng.randf32(), 4), powf(rng.randf32(), 1), powf(rng.randf32(), 8)),
			{.blend_mode = e_blend_mode_additive, .framebuffer = game->particle_framebuffer}
		);
	}
	draw_texture(
		c_half_res, 5, c_base_res * 8, make_color(1), game->particle_framebuffer->texture,
		{.blend_mode = e_blend_mode_additive}, {.rotation = (float)renderer->total_time}
	);

	draw_text("ATHANO BITCH", mouse, 50, 64, make_color(1), true, game->font);
	draw_texture(
		c_half_res, 10, c_base_res, make_color(1), game->text_framebuffer->texture,
		{.blend_mode = e_blend_mode_additive}
	);

	for(int i = 0; i < c_max_keys; i++) {
		g_input->keys[i].count = 0;
	}

}

#ifdef m_build_dll
}
#endif // m_build_dll

func s_v2i spawn_apple()
{
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