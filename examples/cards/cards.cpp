#define m_game

#include "../../src/platform_shared.h"
#include "variables.h"

#define zero {}
#define func static
#define global static
#define null NULL

global constexpr s_v2 c_base_res = {1920, 1080};
global constexpr s_v2 c_half_res = {c_base_res.x / 2.0f, c_base_res.y / 2.0f};
global constexpr f64 c_updates_per_second = 240.0;
global constexpr int c_max_areas_right = 10;
global constexpr int c_max_areas_down = 5;

enum e_layer
{
	e_layer_area_connection,
	e_layer_area,
	e_layer_card,
	e_layer_card_text,
	e_layer_held_card,
	e_layer_held_card_text,
};

enum e_state
{
	e_state_default,
	e_state_map_select,
	e_state_battle,
};

enum e_card
{
	e_card_single_target_dmg,
	e_card_aoe_dmg,
	e_card_count,
};

struct s_enemy
{
	int health;
};

struct s_connection_cross
{
	b8 is_crossing;
	int area_y[2];
	int connection_index[2];
};

struct s_area
{
	s_sarray<s_enemy, 16> enemies;
};

struct s_area_state
{
	s_sarray<int, 16> cards;
};

struct s_run_state
{
	int curr_x;
	s_area_state area_state;
};

struct s_game
{
	b8 initialized;
	s_rng rng;
	e_state state;
	float total_time;
	s_run_state run_state;
	s_carray2<s_area, c_max_areas_down, c_max_areas_right> areas;
	s_carray2<b8, c_max_areas_down, c_max_areas_right> areas_active;
	s_carray2<s_sarray<s_v2i, c_max_areas_down>, c_max_areas_down, c_max_areas_right> area_connections;
};

global s_input* g_input;
global s_game* game;
global s_game_renderer* g_r;
global s_v2 g_mouse;
global float delta = 0;

func void set_globals(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, s_input* input);
func void generate_map();
func b8 can_area_be_played(int x, int y);
func void enter_area(int x, int y);
func char* get_card_text(int card);


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
}

m_dll_export void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
{
	static_assert(sizeof(s_game) <= c_game_memory);
	set_globals(platform_data, game_memory, renderer, &platform_data->render_input);
	delta = (float)platform_data->frame_time;

	if(!game->initialized) {
		game->initialized = true;
		game->state = e_state_map_select;
		platform_data->variables_path = "examples/cards/variables.h";
		game->rng.seed = platform_data->get_random_seed();
		renderer->set_vsync(true);
		generate_map();
	}
	game->total_time += delta;

	live_variable(&platform_data->vars, c_area_x_spacing, 0.0f, 200.0f, true);
	live_variable(&platform_data->vars, c_area_y_spacing, 0.0f, 200.0f, true);
	live_variable(&platform_data->vars, c_area_size, 4.0f, 64.0f, true);

	switch(game->state) {
		case e_state_default: {

		} break;

		case e_state_map_select: {
			for(int y = 0; y < c_max_areas_down; y++) {
				for(int x = 0; x < c_max_areas_right; x++) {
					if(!game->areas_active[y][x]) { continue; }
					s_v2 pos = v2(100, 300) + v2(x * c_area_x_spacing, y * c_area_y_spacing);

					s_v2 size = v2(c_area_size);
					b8 hovered = mouse_collides_rect_center(g_mouse, pos, size);
					if(hovered) {
						size *= 1.25f;
					}

					if(hovered && is_key_pressed(g_input, c_left_mouse)) {
						if(can_area_be_played(x, y)) {
							enter_area(x, y);
						}
					}

					{
						s_v4 color = make_color(0.7f);
						if(game->run_state.curr_x == x) {
							color = rgb(0x479F67);
						}
						draw_rect(g_r, pos, e_layer_area, size, color);
					}

					foreach_val(connection_i, connection, game->area_connections[y][x]) {
						s_v2 to = v2(100, 300);
						to.x += connection.x * c_area_x_spacing;
						to.y += connection.y * c_area_y_spacing;
						draw_line(g_r, pos, to, e_layer_area_connection, c_area_size * 0.25f, make_color(0.1f));
					}
				}
			}
		} break;

		case e_state_battle: {
			s_area_state* a = &game->run_state.area_state;
			float padding = 4;
			s_v2 card_size = v2(190, 320);
			float space = (card_size.x + padding) * a->cards.count;
			s_v2 base_pos = c_base_res * v2(0.5f, 0.8f) - v2(space * 0.5f, 0.0f) + v2(card_size.x / 2.0f, 0.0f);
			static s_v2 card_pos[3] = zero;
			static float foo[3] = zero;
			static int held = -1;
			foreach_val(card_i, card, a->cards) {
				float base_font_size = 16.0f;
				float font_size = base_font_size;
				s_v2 size = card_size;
				s_v2 pos = base_pos;
				s_v2 card_target = pos;

				b8 hovered = mouse_collides_rect_center(g_mouse, pos, size);
				if(held != -1 && held != card_i) { hovered = false; }
				if(hovered || held == card_i) {
					foo[card_i] = at_most(1.0f, foo[card_i] + delta * 10);
				}
				else {
					foo[card_i] = at_least(0.0f, foo[card_i] - delta * 10);
				}
				size = lerp(size, card_size * 1.25f, foo[card_i]);
				font_size = lerp(font_size, base_font_size * 1.25f, foo[card_i]);

				if(hovered && is_key_pressed(g_input, c_left_mouse)) {
					held = card_i;
				}
				int card_layer = e_layer_card;
				int text_layer = e_layer_card_text;
				s_v4 card_color = rgb(0x0B494D);
				if(held == card_i) {
					card_layer = e_layer_held_card;
					text_layer = e_layer_held_card_text;
					card_color = rgb(0xC0CE59);
					card_target = g_mouse;
					if(is_key_released(g_input, c_left_mouse)) {
						held = -1;
					}
				}
				card_pos[card_i] = lerp(card_pos[card_i], card_target, delta * 20.0f);

				draw_rect(g_r, card_pos[card_i], card_layer, size, card_color);
				s_v2 text_pos = card_pos[card_i];
				text_pos.x -= 60;
				char* text = get_card_text(card);
				draw_text(g_r, text, text_pos, text_layer, font_size, make_color(1), false, &g_r->fonts[0]);
				base_pos.x += card_size.x + padding;
			}
		} break;
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

func int count_areas_in_column(int col)
{
	int result = 0;
	for(int y = 0; y < c_max_areas_down; y++) {
		if(game->areas_active[y][col]) { result += 1; }
	}
	return result;
}

func b8 has_connection_from_left(int xx, int yy)
{
	if(xx <= 1) { return true; }
	if(xx >= c_max_areas_right) { return true; }

	if(!game->areas_active[yy][xx]) { return true; }

	for(int y = 0; y < c_max_areas_down; y++) {
		foreach_val(connection_i, connection, game->area_connections[y][xx - 1]) {
			if(connection.x == xx && connection.y == yy) {
				return true;
			}
		}
	}
	return false;
}

func b8 is_connected_to(int x1, int y1, int x2, int y2)
{
	assert(game->areas_active[y1][x1]);
	assert(game->areas_active[y2][x2]);
	foreach_val(connection_i, connection, game->area_connections[y1][x1]) {
		if(connection.x == x2 && connection.y == y2) {
			return true;
		}
	}
	return false;
}

func s_connection_cross is_there_a_cross(int x)
{
	s_connection_cross cross = zero;
	for(int y = 0; y < c_max_areas_down; y++) {
		if(!game->areas_active[y][x]) { continue; }
		for(int y2 = 0; y2 < c_max_areas_down; y2++) {
			if(y == y2) { continue; }
			if(!game->areas_active[y2][x]) { continue; }
			foreach_val(connection_i1, connection1, game->area_connections[y][x]) {
				foreach_val(connection_i2, connection2, game->area_connections[y2][x]) {
					if(y < y2) {
						if(connection1.y > connection2.y) {
							cross.is_crossing = true;
							cross.area_y[0] = y;
							cross.area_y[1] = y2;
							cross.connection_index[0] = connection_i1;
							cross.connection_index[1] = connection_i2;
							return cross;
						}
					}
					else {
						if(connection1.y < connection2.y) {
							cross.is_crossing = true;
							cross.area_y[0] = y;
							cross.area_y[1] = y2;
							cross.connection_index[0] = connection_i1;
							cross.connection_index[1] = connection_i2;
							return cross;
						}
					}
				}
			}
		}
	}
	return cross;
}

func void generate_map()
{
	game->areas_active[2][0] = true;
	for(int x = 1; x < c_max_areas_right - 1; x++) {
		int r = game->rng.rand_range_ii(1, c_max_areas_down);
		s_sarray<int, c_max_areas_down> options;
		for(int i = 0; i < c_max_areas_down; i++) { options.add(i); }
		for(int y = 0; y < r; y++) {
			int r2 = game->rng.randu() % options.count;
			game->areas_active[options[r2]][x] = true;
			options.remove_and_swap(r2);
		}
	}
	game->areas_active[2][c_max_areas_right - 1] = true;

	// @Note(tkap, 31/10/2023): Connect start to everything in the next layer
	for(int y = 0; y < c_max_areas_down; y++) {
		if(!game->areas_active[y][1]) { continue; }
		game->area_connections[2][0].add(v2i(1, y));
	}

	// @Note(tkap, 31/10/2023): Connect everything in second to last column to end
	for(int y = 0; y < c_max_areas_down; y++) {
		if(!game->areas_active[y][c_max_areas_right - 2]) { continue; }
		game->area_connections[y][c_max_areas_right - 2].add(v2i(c_max_areas_right - 1, 2));
	}

	for(int x = 1; x < c_max_areas_right - 2; x++) {

		while(true) {

			for(int y = 0; y < c_max_areas_down; y++) {
				if(!game->areas_active[y][x]) { continue; }
				while(true) {
					int ry = game->rng.randu() % c_max_areas_down;
					if(!game->areas_active[ry][x + 1]) { continue; }
					if(is_connected_to(x, y, x + 1, ry)) { break; }
					game->area_connections[y][x].add(v2i(x + 1, ry));
					break;
				}
			}

			while(true) {
				s_connection_cross cross = is_there_a_cross(x);
				if(cross.is_crossing) {
					int r = game->rng.randu() % 2;
					game->area_connections[cross.area_y[r]][x].remove_and_shift(cross.connection_index[r]);
				}
				else { break; }
			}

			// @Note(tkap, 31/10/2023): Check that everything to the right is connected
			b8 all_connected = true;
			for(int y = 0; y < c_max_areas_down; y++) {
				if(!has_connection_from_left(x + 1, y)) {
					all_connected = false;
					break;
				}
			}

			for(int y = 0; y < c_max_areas_down; y++) {
				if(!game->areas_active[y][x]) { continue; }
				if(game->area_connections[y][x].count <= 0) {
					all_connected = false;
					break;
				}
			}

			if(all_connected) { break; }
		}
	}

	// @Note(tkap, 31/10/2023): Try removing connections
	for(int i = 0; i < c_max_areas_right * c_max_areas_down; i++) {
		int rand_x = game->rng.rand_range_ie(1, c_max_areas_right - 1);
		int rand_y = game->rng.randu() % c_max_areas_down;
		if(!game->areas_active[rand_y][rand_x]) { continue; }
		if(game->area_connections[rand_y][rand_x].count <= 1) { continue; }
		int rand_connection_index = game->rng.randu() % game->area_connections[rand_y][rand_x].count;
		s_v2i connection = game->area_connections[rand_y][rand_x][rand_connection_index];
		game->area_connections[rand_y][rand_x].remove_and_swap(rand_connection_index);

		b8 all_connected = true;
		for(int y = 0; y < c_max_areas_down; y++) {
			if(!has_connection_from_left(rand_x + 1, y)) {
				all_connected = false;
				break;
			}
		}
		if(!all_connected) {
			game->area_connections[rand_y][rand_x].add(connection);
		}
	}

}

func b8 can_area_be_played(int x, int y)
{
	if(game->run_state.curr_x != x) { return false; }
	return true;
}

func void enter_area(int x, int y)
{
	game->state = e_state_battle;
	game->run_state.area_state = zero;
	for(int i = 0; i < 3; i++) {
		game->run_state.area_state.cards.add(game->rng.randu() % e_card_count);
	}
}

func char* get_card_text(int card)
{
	switch(card) {
		case e_card_single_target_dmg: {
			return "Deal 5 damage";
		} break;
		case e_card_aoe_dmg: {
			return "Deal 2 damage\nto all enemies";
		} break;
	}
	return null;
}