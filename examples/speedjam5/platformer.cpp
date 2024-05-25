#define m_game

#include "../../src/platform_shared.h"
// #include "variables.h"

static constexpr int c_tile_count = 12;
static constexpr int c_max_snake_len = c_tile_count * c_tile_count / 2;
static constexpr int c_score_to_win = 20;
static constexpr int c_tile_size = 64;
static constexpr s_v2 c_base_res = {c_tile_size * c_tile_count, c_tile_size * c_tile_count};
static constexpr s_v2 c_half_res = {c_base_res.x * 0.5f, c_base_res.y * 0.5f};

enum e_state
{
	e_state_play,
	e_state_victory,
};

struct s_snake
{
	s_v2i pos;
	float rotation;
};

struct s_game
{
	b8 initialized;
	e_state state;
	s_framebuffer* particle_framebuffer;
	s_framebuffer* text_framebuffer;
	s_texture snake_head;
	s_texture snake_body;
	s_texture snake_tail;
	s_texture apple_texture;
	s_texture noise;
	s_sarray<s_v2i, 4> inputs;
	s_v2i last_dir;
	s_snake snake[c_max_snake_len];
	int snake_len;
	float move_timer;
	s_v2i apple;
	s_rng rng;
	b8 reset_level;
	s_sound* eat_apple_sound;
	s_font* font;
	s_v2 snake_light_pos;
	float snake_apple_time;
};

static s_input* g_input;
static s_game* game;
static s_game_renderer* g_r;
static s_v2 g_mouse;

#ifdef m_build_dll
extern "C" {
#endif // m_build_dll

m_dll_export void init_game(s_platform_data* platform_data)
{
	platform_data->set_base_resolution((int)c_base_res.x, (int)c_base_res.y);
	platform_data->set_window_size((int)c_base_res.x, (int)c_base_res.y);
}

m_dll_export void update(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{
}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt)
{
	static_assert(sizeof(s_game) <= c_game_memory);

	g_mouse = platform_data->mouse;

	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = &platform_data->render_input;
	if(!game->initialized) {
		game->initialized = true;
		game->rng.seed = platform_data->get_random_seed();
		g_r->set_vsync(true);
		game->snake_head = g_r->load_texture(renderer, "examples/snake/snake_head.png");
		game->snake_body = g_r->load_texture(renderer, "examples/snake/snake_body.png");
		game->snake_tail = g_r->load_texture(renderer, "examples/snake/snake_tail.png");
		game->noise = g_r->load_texture(renderer, "examples/snake/noise.png");
		game->apple_texture = g_r->load_texture(renderer, "examples/snake/apple.png");
		game->particle_framebuffer = g_r->make_framebuffer(renderer, false);
		game->text_framebuffer = g_r->make_framebuffer(renderer, false);
		game->eat_apple_sound = platform_data->load_sound(platform_data, "examples/snake/eat_apple.wav", platform_data->frame_arena);
		game->reset_level = true;
		game->font = &renderer->fonts[0];
		platform_data->variables_path = "examples/snake/variables.h";
	}

	draw_texture(g_r,	v2(200, 200), 1, v2(64), make_color(1), game->apple_texture);

}

#ifdef m_build_dll
}
#endif // m_build_dll



