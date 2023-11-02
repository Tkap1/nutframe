#define m_game

#include "PerlinNoise.hpp"

#include "../../src/platform_shared.h"
#include "variables.h"

#define zero {}
#define func static
#define global static
#define null NULL

global constexpr s_v2 c_base_res = {1920, 1080};
global constexpr s_v2 c_half_res = {c_base_res.x / 2.0f, c_base_res.y / 2.0f};
global constexpr f64 c_updates_per_second = 240.0;
global constexpr float c_delta = (float)(1.0 / c_updates_per_second);
global constexpr int c_max_tiles_right = 1024;
global constexpr int c_max_tiles_down = 1024;

enum e_layer
{
	e_layer_tile,
	e_layer_projectile,
};

struct s_cam
{
	s_v2 pos;
	s_v2 target_pos;

	s_v2 world_to_screen(s_v2 world_pos)
	{
		world_pos.x -= pos.x;
		world_pos.y -= pos.y;
		return world_pos;
	}

	s_v2 screen_to_world(s_v2 screen_pos)
	{
		screen_pos.x += pos.x;
		screen_pos.y += pos.y;
		return screen_pos;
	}
};

struct s_projectile
{
	int projectiles_to_spawn;
	s_v2 pos;
	s_v2 vel;
};

struct s_game
{
	b8 initialized;
	s_rng rng;
	float total_time;
	s_carray2<b8, c_max_tiles_down, c_max_tiles_right> tiles_active;
	s_carray2<u32, c_max_tiles_down, c_max_tiles_right> tile_colors;
	s_sarray<s_projectile, 16384> projectiles;
	s_cam cam;
};

global s_input* g_input;
global s_game* game;
global s_game_renderer* g_r;
global s_v2 g_mouse;

func void set_globals(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, s_input* input);
func b8 is_tile_active(int x, int y);
func b8 is_tile_active(s_v2i index);
func void generate_map();
func s_v2i pos_to_tile_index(s_v2 pos);
func void set_tile_active(int x, int y, b8 val);
func void set_tile_active(s_v2i index, b8 val);
func s_v2 tile_index_to_pos_center(s_v2i index);
func s_v2 tile_index_to_pos(int x, int y);
func s_v2 tile_index_to_pos(s_v2i index);

#ifdef m_build_dll
extern "C" {
#endif // m_build_dll
m_dll_export void init_game(s_platform_data* platform_data)
{
	platform_data->set_window_size((int)c_base_res.x, (int)c_base_res.y);
	platform_data->update_delay = 1.0 / c_updates_per_second;
	platform_data->variables_path = "examples/blocks/variables.h";
}

m_dll_export void update(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{
	static_assert(sizeof(s_game) <= c_game_memory);
	set_globals(platform_data, game_memory, renderer, &platform_data->logic_input);

	if(!game->initialized) {
		game->initialized = true;
		game->rng.seed = platform_data->get_random_seed();
		generate_map();
	}

	if(is_key_down(g_input, c_key_a)) {
		game->cam.target_pos.x += -c_cam_speed * c_delta;
	}
	if(is_key_down(g_input, c_key_d)) {
		game->cam.target_pos.x += c_cam_speed * c_delta;
	}
	if(is_key_down(g_input, c_key_w)) {
		game->cam.target_pos.y += -c_cam_speed * c_delta;
	}
	if(is_key_down(g_input, c_key_s)) {
		game->cam.target_pos.y += c_cam_speed * c_delta;
	}
	if(is_key_pressed(g_input, c_key_r)) {
		generate_map();
	}
	if(is_key_down(g_input, c_left_mouse)) {
		s_projectile p = zero;
		p.pos = game->cam.screen_to_world(g_mouse);
		p.projectiles_to_spawn = 3;
		game->projectiles.add(p);
	}

	game->cam.pos = lerp(game->cam.pos, game->cam.target_pos, 0.05f);
	if(v2_distance(game->cam.pos, game->cam.target_pos) < 0.01f) {
		game->cam.pos = game->cam.target_pos;
	}

	foreach_ptr(p_i, p, game->projectiles) {
		p->vel.y += c_delta * 10;
		p->pos += p->vel;
		int check_radius = roundfi(c_projectile_size / 2.0f / c_tile_size);
		s_v2i index = pos_to_tile_index(p->pos);
		b8 explode = false;
		for(int cy = -check_radius; cy <= check_radius; cy++) {
			for(int cx = -check_radius; cx <= check_radius; cx++) {
				s_v2i temp = index + v2i(cx, cy);
				if(is_tile_active(temp)) {
					explode = true;
					break;
				}
			}
		}
		if(explode) {
			constexpr int radius = 10;
			s_v2 circle_pos = tile_index_to_pos_center(index);
			float circle_radius = (radius * 2 + 1) * c_tile_size * 0.5f;
			for(int y = -radius; y <= radius; y++) {
				for(int x = -radius; x <= radius; x++) {
					s_v2i temp_index = index + v2i(x, y);
					if(is_tile_active(temp_index)) {
						if(rect_collides_circle_topleft(tile_index_to_pos(temp_index), v2(c_tile_size), circle_pos, circle_radius)) {
							set_tile_active(temp_index, false);
						}
					}
				}
			}
			for(int i = 0; i < p->projectiles_to_spawn; i++) {
				s_projectile new_p = zero;
				new_p.pos = p->pos;
				new_p.vel = v2_rand(&game->rng) * v2(1.0f, 5.0f);
				new_p.projectiles_to_spawn = p->projectiles_to_spawn - 1;
				if(new_p.vel.y > 0) { new_p.vel.y *= -1; }
				game->projectiles.add(new_p);
			}
			game->projectiles.remove_and_swap(p_i--);
		}
	}

	game->total_time += c_delta;

}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{
	set_globals(platform_data, game_memory, renderer, &platform_data->render_input);

	live_variable(&platform_data->vars, c_tile_size, 4, 64, true);
	live_variable(&platform_data->vars, c_cam_speed, 100.0f, 4000.0f, true);
	live_variable(&platform_data->vars, c_projectile_size, 4, 128, true);

	int min_x = at_least(0, floorfi(game->cam.pos.x / c_tile_size));
	int max_x = at_most(c_max_tiles_right - 1, min_x + ceilfi(c_base_res.x / c_tile_size));
	int min_y = at_least(0, floorfi(game->cam.pos.y / c_tile_size));
	int max_y = at_most(c_max_tiles_down - 1, min_y + ceilfi(c_base_res.y / c_tile_size));
	for(int y = min_y; y <= max_y; y++)	{
		for(int x = min_x; x <= max_x; x++) {
			if(!is_tile_active(x, y)) { continue; }
			s_v2 pos = v2(x * c_tile_size, y * c_tile_size);
			s_v4 color = rgb(game->tile_colors[y][x]);
			draw_rect(renderer, game->cam.world_to_screen(pos), e_layer_tile, v2(c_tile_size), color, zero, {.origin_offset = c_origin_topleft});
		}
	}

	foreach_val(p_i, p, game->projectiles) {
		draw_rect(renderer, game->cam.world_to_screen(p.pos), e_layer_projectile, v2(c_projectile_size), rgb(0xFB834A));
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

func b8 is_valid_tile_index(int x, int y)
{
	return x >= 0 && x < c_max_tiles_right && y >= 0 && y < c_max_tiles_down;
}

func b8 is_tile_active(int x, int y)
{
	if(!is_valid_tile_index(x, y)) { return false; }
	return game->tiles_active[y][x];
}

func b8 is_tile_active(s_v2i index)
{
	return is_tile_active(index.x, index.y);
}

func void set_tile_active(int x, int y, b8 val)
{
	game->tiles_active[y][x] = val;
}

func void set_tile_active(s_v2i index, b8 val)
{
	set_tile_active(index.x, index.y, val);
}

func void generate_map()
{
	memset(&game->tiles_active, 0, sizeof(game->tiles_active));
	siv::PerlinNoise perlinA{ std::random_device{} };
	constexpr int start_y = c_max_tiles_down / 2;
	for(int x = 0; x < c_max_tiles_right; x++) {
		constexpr float factor = 0.0015f;
		float noise = (float)perlinA.octave2D_01(x * factor, x * factor, 6);
		noise = noise * 2 - 1;
		int current_y = clamp(start_y + roundfi(noise * c_max_tiles_down / 2), 0, c_max_tiles_down - 1);
		for(int y = current_y; y < c_max_tiles_down; y++) {
			set_tile_active(x, y, true);
			u8 r = (u8)((float)x / (c_max_tiles_right - 1) * 255);
			u8 g = (u8)((float)y / (c_max_tiles_down - 1) * 255);
			// u8 g = (u8)((float)(c_max_tiles_down - y) / (c_max_tiles_down - 1) * 255);
			game->tile_colors[y][x] = r << 16 | g << 8;
			// game->tile_colors[y][x] = g << 8;
		}
	}
}

func s_v2i pos_to_tile_index(s_v2 pos)
{
	return v2i(
		floorfi(pos.x / c_tile_size),
		floorfi(pos.y / c_tile_size)
	);
}

func s_v2 tile_index_to_pos(int x, int y)
{
	return v2(x * c_tile_size, y * c_tile_size);
}

func s_v2 tile_index_to_pos(s_v2i index)
{
	return tile_index_to_pos(index.x, index.y);
}

func s_v2 tile_index_to_pos_center(s_v2i index)
{
	return tile_index_to_pos(index) + v2(c_tile_size / 2);
}