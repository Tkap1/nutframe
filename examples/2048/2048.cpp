#define m_game

#include "../../src/platform_shared.h"
#include "variables.h"

static constexpr s_v2 c_base_res = {800, 800};
static constexpr s_v2 c_half_res = {c_base_res.x / 2.0f, c_base_res.y / 2.0f};

static constexpr int c_tiles_right = 4;
static constexpr int c_tiles_down = 4;

static constexpr s_v4 c_tile_colors[] = {
	rgb(0xCFA96F),
	rgb(0x7C806E),
	rgb(0x4F615C),
	rgb(0x2A3646),
	rgb(0x757464),
	rgb(0x413B30),
	rgb(0xA19B5C),
	rgb(0xEECD9A),
	rgb(0xB88070),
	rgb(0x61141E),
};

enum e_layer
{
	e_layer_tile_background,
	e_layer_tile,
	e_layer_tile_text,
	e_layer_ui,
};

enum e_state
{
	e_state_play,
	e_state_lost,
	e_state_win,
};

typedef s_carray2<int, c_tiles_down, c_tiles_right> t_grid;

struct s_grid
{
	t_grid grid;
	s_carray2<s_v2, c_tiles_down, c_tiles_right> pos;
};

struct s_game
{
	e_state state;
	int next_state;
	b8 initialized;
	float total_time;
	s_grid grid;
	s_font* font;
	s_rng rng;
};

static s_input* g_input;
static s_game* game;
static s_game_renderer* g_r;
static s_v2 g_mouse;

static void set_globals(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, s_input* input);
static int value_from_tile(int tile);
static s_grid do_movement(s_grid grid, int start_x, int start_y, b8 positive_movement, b8 move_x);
static b8 can_make_a_move(s_grid grid);

#ifdef m_build_dll
extern "C" {
#endif // m_build_dll
m_dll_export void init_game(s_platform_data* platform_data)
{
	platform_data->set_base_resolution((int)c_base_res.x, (int)c_base_res.y);
	platform_data->set_window_size((int)c_base_res.x, (int)c_base_res.y);
	platform_data->update_delay = 1.0;
}

m_dll_export void update(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{
	static_assert(sizeof(s_game) <= c_game_memory);
	set_globals(platform_data, game_memory, renderer, &platform_data->logic_input);

	if(!game->initialized) {
		game->initialized = true;
		game->font = &renderer->fonts[0];
		platform_data->variables_path = "examples/2048/variables.h";
		game->rng.seed = platform_data->get_random_seed();
		game->next_state = e_state_play;
	}
}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt)
{
	set_globals(platform_data, game_memory, renderer, &platform_data->render_input);

	live_variable(&platform_data->vars, c_tile_size, 4, 128, true);
	live_variable(&platform_data->vars, c_tile_pad, 4, 128, true);
	live_variable(&platform_data->vars, c_tile_font_size, 4.0f, 128.0f, true);

	float delta = (float)platform_data->frame_time;

	if(game->next_state >= 0) {
		game->state = (e_state)game->next_state;
		game->next_state = -1;

		if(game->state == e_state_play) {
			memset(&game->grid, 0, sizeof(s_grid));
			game->grid.grid[2][0] = 1;
			game->grid.grid[3][2] = 1;
			game->grid.grid[3][0] = 1;
		}
	}

	switch(game->state) {
		case e_state_play: {
			int start_x = 0;
			int start_y = 0;
			b8 move_x = true;
			b8 do_we_want_to_move = false;
			b8 positive_movement = true;

			if(is_key_pressed(g_input, c_key_left)) {
				start_x = 1;
				do_we_want_to_move = true;
				positive_movement = false;
			}

			else if(is_key_pressed(g_input, c_key_right)) {
				start_x = c_tiles_right - 1;
				start_y = c_tiles_down - 1;
				do_we_want_to_move = true;
			}

			else if(is_key_pressed(g_input, c_key_up)) {
				start_y = 1;
				move_x = false;
				do_we_want_to_move = true;
				positive_movement = false;
			}

			else if(is_key_pressed(g_input, c_key_down)) {
				start_x = c_tiles_right - 1;
				start_y = c_tiles_down - 1;
				do_we_want_to_move = true;
				move_x = false;
			}

			b8 did_we_actually_move = false;
			if(do_we_want_to_move) {
				s_grid grid_before_movement = game->grid;
				game->grid = do_movement(game->grid, start_x, start_y, positive_movement, move_x);
				did_we_actually_move = memcmp(&game->grid, &grid_before_movement, sizeof(game->grid)) != 0;
			}

			if(!can_make_a_move(game->grid)) {
				game->next_state = e_state_lost;
			}

			if(did_we_actually_move) {

				for(int y = 0; y < c_tiles_down; y++) {
					for(int x = 0; x < c_tiles_right; x++) {
						if(game->grid.grid[y][x] >= 11) { // 2048
							game->next_state = e_state_win;
						}
					}
				}

				s_sarray<int, c_tiles_right * c_tiles_down> options;
				for(int i = 0; i < c_tiles_right * c_tiles_down; i++) {
					options.add(i);
				}
				options.shuffle(&game->rng);
				while(true) {
					int choice = options.pop();
					s_v2i index = index_1d_to_2d(choice, c_tiles_right, c_tiles_down);
					if(game->grid.grid[index.y][index.x] <= 0) {
						game->grid.pos[index.y][index.x] = {};
						game->grid.grid[index.y][index.x] = 1;
						break;
					}
				}
			}

		} break;

		case e_state_win: {
			if(is_key_pressed(g_input, c_key_enter)) {
				game->next_state = e_state_play;
			}
			draw_text(renderer, "Victory!", c_half_res, e_layer_ui, 128.0f, make_color(1), true, game->font);
		} break;

		case e_state_lost: {
			if(is_key_pressed(g_input, c_key_enter)) {
				game->next_state = e_state_play;
			}
			draw_text(renderer, "Game Over", c_half_res, e_layer_ui, 128.0f, make_color(1), true, game->font);
		} break;
	}

	for(int y = 0; y < c_tiles_down; y++) {
		for(int x = 0; x < c_tiles_right; x++) {
			int tile_value = game->grid.grid[y][x];
			s_v2 target_pos = v2(x * (c_tile_size + c_tile_pad), y * (c_tile_size + c_tile_pad));
			int x_size = (c_tile_size + c_tile_pad) * (c_tiles_right - 1);
			int y_size = (c_tile_size + c_tile_pad) * (c_tiles_down - 1);
			target_pos += c_half_res;
			target_pos -= v2(x_size, y_size) * 0.5f;
			s_v2* pos = &game->grid.pos[y][x];
			*pos = lerp_snap(*pos, target_pos, delta * 20, v2(0.1f));
			draw_rect(renderer, target_pos, e_layer_tile_background, v2(c_tile_size), rgb(0xCFA96F));
			if(tile_value > 0) {
				int val = value_from_tile(tile_value);
				draw_rect(renderer, *pos, e_layer_tile, v2(c_tile_size - 16), c_tile_colors[tile_value]);
				draw_text(renderer, format_text("%i", val), *pos, e_layer_tile_text, c_tile_font_size, make_color(1), true, game->font);
			}
		}
	}
}


#ifdef m_build_dll
}
#endif // m_build_dll

static void set_globals(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, s_input* input)
{
	g_mouse = platform_data->mouse;
	game = (s_game*)game_memory;
	g_r = renderer;
	g_input = input;
}

static int value_from_tile(int tile)
{
	return (int)pow(2, tile);
}

static b8 can_make_a_move(s_grid grid)
{
	int start_x[] = {1, c_tiles_right - 1, 0, c_tiles_right - 1};
	int start_y[] = {0, c_tiles_down - 1, 1, c_tiles_down - 1};
	b8 positive_movement[] = {false, true, false, true};
	b8 move_x[] = {true, true, false, false};

	for(int i = 0; i < array_count(start_x); i++) {
		auto temp = do_movement(grid, start_x[i], start_y[i], positive_movement[i], move_x[i]);
		if(memcmp(&temp, &grid, sizeof(grid)) != 0) {
			return true;
		}
	}

	return false;
}

static s_grid do_movement(s_grid grid, int start_x, int start_y, b8 positive_movement, b8 move_x)
{
	for(int y = start_y; positive_movement ? (y >= 0) : (y < c_tiles_down); positive_movement ? y-- : y++) {
		for(int x = start_x; positive_movement ? (x >= 0) : (x < c_tiles_right); positive_movement ? x-- : x++) {
			if(!grid.grid[y][x]) { continue; }
			int next_x;
			int next_y;
			if(positive_movement) {
				next_x = move_x ? x + 1 : x;
				next_y = !move_x ? y + 1 : y;
			}
			else {
				next_x = move_x ? x - 1 : x;
				next_y = !move_x ? y - 1 : y;
			}
			int curr_x = x;
			int curr_y = y;
			while(positive_movement ? (next_x < c_tiles_right && next_y < c_tiles_down) : (next_x >= 0 && next_y >= 0)) {
				b8 merge = false;
				b8 target_has_tile = grid.grid[next_y][next_x] > 0;
				if(target_has_tile) {
					if(grid.grid[next_y][next_x] == grid.grid[curr_y][curr_x]) {
						merge = true;
					}
					else {
						break;
					}
				}
				grid.grid[next_y][next_x] = grid.grid[curr_y][curr_x];
				grid.pos[next_y][next_x] = grid.pos[curr_y][curr_x];
				grid.grid[curr_y][curr_x] = 0;
				grid.pos[curr_y][curr_x] = {};
				if(merge) {
					grid.grid[next_y][next_x] += 1;
					break;
				}
				if(move_x) {
					next_x += positive_movement ? 1 : -1;
					curr_x += positive_movement ? 1 : -1;
				}
				else {
					next_y += positive_movement ? 1 : -1;
					curr_y += positive_movement ? 1 : -1;
				}
			}
		}
	}
	return grid;
}