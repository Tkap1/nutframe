#define m_game

#include "../../src/platform_shared.h"

struct s_game
{
	b8 initialized;
	s_rng rng;
};

static s_input* g_input;
static s_game* game;
static s_game_renderer* g_r;
static s_v2 g_mouse;

#ifdef m_build_dll
extern "C" {
#endif // m_build_dll
m_dll_export void update_game(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{
	static_assert(sizeof(s_game) <= c_game_memory);

	g_mouse = platform_data->mouse;
	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = platform_data->input;
	if(!game->initialized) {
		game->initialized = true;
		game->rng.seed = platform_data->get_random_seed();
		g_r->set_vsync(true);
	}

	// Your code here. Look at other examples if you don't know what to do!

	for(int i = 0; i < c_max_keys; i++) {
		g_input->keys[i].count = 0;
	}

	platform_data->reset_ui();

}

#ifdef m_build_dll
}
#endif // m_build_dll

