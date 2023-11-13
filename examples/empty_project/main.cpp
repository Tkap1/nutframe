#define m_game

#include "../../src/platform_shared.h"
#include "variables.h"

static constexpr s_v2 c_base_res = {1920, 1080};
static constexpr s_v2 c_half_res = {c_base_res.x / 2.0f, c_base_res.y / 2.0f};
static constexpr f64 c_updates_per_second = 240.0;
static constexpr float c_delta = (float)(1.0 / c_updates_per_second);

struct s_game
{
	b8 initialized;
	float total_time;
};

static s_input* g_input;
static s_game* game;
static s_game_renderer* g_r;
static s_v2 g_mouse;

static void set_globals(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, s_input* input);

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
	}

	game->total_time += c_delta;

}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt)
{
	set_globals(platform_data, game_memory, renderer, &platform_data->render_input);
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
