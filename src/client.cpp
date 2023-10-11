
#include "pch_client.h"

#include "config.h"
#include "bucket.h"
#include "platform_shared.h"
#include "client.h"

global s_lin_arena* frame_arena;
global s_game_window g_window;
global s_input* g_logic_input;
global s_input* g_input;
global s_platform_data* g_platform_data;
global s_platform_funcs g_platform_funcs;
global s_game* game;
global s_ui g_ui;
global s_game_renderer* g_r;

#include "draw.cpp"
#include "memory.cpp"
#include "bucket.cpp"

#ifdef m_build_dll
extern "C" {
__declspec(dllexport)
#endif // m_build_dll
m_update_game(update_game)
{
	static_assert(sizeof(s_game) <= c_game_memory);

	game = (s_game*)game_memory;
	g_r = rendering;
	if(!game->initialized)
	{
		game->initialized = true;
		g_r->set_vsync(true);
		game->noise = g_r->load_texture(rendering, "assets/noise.png");
		game->dude = g_r->load_texture(rendering, "assets/dude.png");
	}

	g_input = platform_data->input;
	for(int i = 0; i < c_max_keys; i++)
	{
		g_input->keys[i].count = 0;
	}

	draw_rect(c_half_res, 0, c_base_res, make_color(121/255.0f * 0.2f, 241/255.0f * 0.2f, 255/255.0f * 0.2f));
	draw_texture(platform_data->mouse, 1, v2(1024), make_color(1), game->noise, {.effect_id = 1});
	draw_texture(platform_data->mouse, 2, v2(128), make_color(1), game->dude);
}

#ifdef m_build_dll
}
#endif // m_build_dll

func void update()
{

}

func void render(float dt)
{
}

func b8 is_key_down(s_input* input, int key)
{
	assert(key < c_max_keys);
	return input->keys[key].is_down || input->keys[key].count >= 2;
}

func b8 is_key_up(s_input* input, int key)
{
	assert(key < c_max_keys);
	return !input->keys[key].is_down;
}

func b8 is_key_pressed(s_input* input, int key)
{
	assert(key < c_max_keys);
	return (input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

func b8 is_key_released(s_input* input, int key)
{
	assert(key < c_max_keys);
	return (!input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}
