#define m_game

#include "../src/platform_shared.h"
#include "../src/variables.h"

global constexpr int c_tile_count = 12;
global constexpr int c_max_snake_len = c_tile_count * c_tile_count / 2;
global constexpr int c_score_to_win = 20;

enum e_state
{
	e_state_play,
	e_state_victory,
};

enum e_ui
{
	e_ui_nothing,
	e_ui_hovered,
	e_ui_pressed,
	e_ui_active,
};

struct s_ui_interaction
{
	b8 pressed_this_frame;
	e_ui state;
};

struct s_snake
{
	s_v2i pos;
	float rotation;
};

enum e_var_type
{
	e_var_type_int,
	e_var_type_float,
	e_var_type_bool,
};

union s_var_value
{
	float val_float;
	int val_int;
};

struct s_ui_id
{
	u32 id;
};

struct s_ui
{
	b8 pressed_present;
	s_ui_id hovered;
	s_ui_id pressed;
};

struct s_var
{
	b8 display;
	e_var_type type;
	void* ptr;
	const char* name;
	s_var_value min_val;
	s_var_value max_val;
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
	s_texture checkmark_texture;
	s_sarray<s_v2i, 4> inputs;
	s_v2i last_dir;
	s_snake snake[c_max_snake_len];
	int snake_len;
	float move_timer;
	s_v2i apple;
	s_rng rng;
	b8 reset_level;
	s_font* font;
	s_sound* eat_apple_sound;
	s_v2 snake_light_pos;
	float snake_apple_time;

	#ifdef m_debug
	s_v2 vars_pos;
	s_v2 vars_pos_offset;
	s_sarray<s_var, 16> variables;
	b8 show_live_vars;
	#endif // m_debug
};

global s_input* g_input;
global s_game* game;
global s_game_renderer* g_r;
global s_v2 mouse;
global s_ui g_ui;

#ifdef m_debug
#define register_live_variable(var) register_live_variable_(&var, #var)
#define live_variable(var, min_val, max_val) live_variable_(&var, #var, min_val, max_val)
#else // m_debug
#define register_live_variable(var)
#define live_variable(var, min_val, max_val)
#endif // m_debug

func s_v2i spawn_apple();
template <typename t>
func void live_variable_(t* ptr, const char* name, t min_val, t max_val);
func s_ui_interaction ui_button(const char* text, s_v2 pos, s_v2 size, s_font* font, float font_size);
template <typename t>
func t ui_slider(const char* text, s_v2 pos, s_v2 size, s_font* font, float font_size, t min_val, t max_val, t curr_val);
func void ui_request_pressed(u32 id);
func void ui_checkbox(const char* text, s_v2 pos, s_v2 size, b8* val);
template <typename t>
func void register_live_variable_(t* ptr, const char* name);
func s_var* get_var_by_ptr(void* ptr);


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
		game->noise = g_r->load_texture(renderer, "examples/noise.png");
		game->apple_texture = g_r->load_texture(renderer, "examples/apple.png");
		game->checkmark_texture = g_r->load_texture(renderer, "examples/checkmark.png");
		game->font = g_r->load_font(renderer, "examples/consola.ttf", 96, platform_data->frame_arena);
		game->particle_framebuffer = g_r->make_framebuffer(renderer, false);
		game->text_framebuffer = g_r->make_framebuffer(renderer, false);
		game->eat_apple_sound = platform_data->load_sound(platform_data, "examples/eat_apple.wav", platform_data->frame_arena);
		game->reset_level = true;
	}

	draw_texture(g_r, c_half_res, 0, c_base_res, make_color(1), game->noise, zero, {.effect_id = 1});
	renderer->set_shader_float("snake_apple_time", game->snake_apple_time);
	game->snake_apple_time = at_least(0.0f, game->snake_apple_time - (float)platform_data->frame_time);

	register_live_variable(c_apple_light_duration);
	register_live_variable(c_move_delay);
	register_live_variable(c_tile_size);
	register_live_variable(c_foo);

	live_variable(c_apple_light_duration, 0.0f, 5.0f);
	live_variable(c_move_delay, 0.01f, 0.5f);
	live_variable(c_tile_size, 64, 65);
	live_variable(c_foo, (b8)0, (b8)0);

	if(is_key_pressed(g_input, c_key_f1)) {
		game->show_live_vars = !game->show_live_vars;
	}

	#ifdef m_debug

	if(game->show_live_vars) {
		s_v2 pos = game->vars_pos;
		s_ui_interaction interaction = ui_button("Move", pos, v2(32), null, 32);
		if(interaction.state == e_ui_pressed) {
			if(interaction.pressed_this_frame) {
				game->vars_pos_offset = mouse - pos;
			}
			s_v2 m = mouse;
			m.x = clamp(mouse.x, 0.0f, c_base_res.x - 32);
			m.y = clamp(mouse.y, 0.0f, c_base_res.y - 32);
			m -= game->vars_pos_offset;
			game->vars_pos = m;
			pos = m;
		}
		pos += v2(100);

		constexpr float font_size = 24;

		foreach_val(var_i, var, game->variables) {
			if(!var.display) { continue; }
			s_v2 text_pos = pos;
			text_pos.x -= 300;
			text_pos.y += 24 - font_size * 0.5f;
			s_v2 slider_pos = pos;
			draw_text(g_r, var.name, text_pos, 15, font_size, rgb(0xffffff), false, game->font);
			if(var.type == e_var_type_int) {
				*(int*)var.ptr = ui_slider(var.name, slider_pos, v2(200, 48), game->font, font_size, var.min_val.val_int, var.max_val.val_int, *(int*)var.ptr);
			}
			else if(var.type == e_var_type_float) {
				*(float*)var.ptr = ui_slider(
					var.name, slider_pos, v2(200, 48), game->font, font_size, var.min_val.val_float, var.max_val.val_float, *(float*)var.ptr
				);
			}
			else if(var.type == e_var_type_bool) {
				s_v2 temp = slider_pos;
				temp.x += 100 - 24;
				ui_checkbox(var.name, temp, v2(48), (b8*)var.ptr);
			}
			pos.y += 50;
		}

		if(ui_button("Save", pos, v2(200, 48), game->font, font_size).state == e_ui_active) {
			s_str_builder<10 * c_kb> builder;
			foreach_val(var_i, var, game->variables) {
				if(var.type == e_var_type_int) {
					builder.add_line("global int %s = %i;", var.name, *(int*)var.ptr);
				}
				else if(var.type == e_var_type_float) {
					builder.add_line("global float %s = %ff;", var.name, *(float*)var.ptr);
				}
				else if(var.type == e_var_type_bool) {
					builder.add_line("global b8 %s = %s;", var.name, *(b8*)var.ptr ? "true" : "false");
				}
				invalid_else;
			}
			platform_data->write_file("src/variables.h", builder.data, builder.len);
		}
	}
	#endif // m_debug

	switch(game->state) {
		case e_state_play: {
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
					game->snake_len += 1;
					game->snake_apple_time = c_apple_light_duration;
					platform_data->play_sound(game->eat_apple_sound);
					if(game->snake_len - 1 >= c_score_to_win) {
						game->state = e_state_victory;
					}
					game->apple = spawn_apple();
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

				s_v2 snake_center = v2(head.pos * c_tile_size) + v2(c_tile_size / 2);
				game->snake_light_pos = lerp(game->snake_light_pos, snake_center, 0.1f);
				renderer->set_shader_v2("snake_pos", game->snake_light_pos);
			}

			for(int snake_i = 0; snake_i < game->snake_len; snake_i++)
			{
				s_texture texture;
				if(snake_i == 0) { texture = game->snake_head; }
				else if(snake_i == game->snake_len - 1) { texture = game->snake_tail; }
				else { texture = game->snake_body; }
				s_snake s = game->snake[snake_i];
				draw_texture(g_r,
					v2(s.pos * c_tile_size), 1, v2(c_tile_size), make_color(1), texture, zero, {.rotation = s.rotation, .origin_offset = c_origin_topleft}
				);
			}

			draw_texture(g_r,
				v2(game->apple * c_tile_size), 2, v2(c_tile_size), make_color(1), game->apple_texture, zero, {.origin_offset = c_origin_topleft}
			);
		} break;

		case e_state_victory: {

			if(is_key_pressed(g_input, c_key_enter)) {
				game->state = e_state_play;
				game->reset_level = true;
			}
			else if(is_key_pressed(g_input, c_key_escape)) {
				exit(0);
			}

			s_v2 pos = c_half_res;
			draw_text(g_r, "You win!", pos, 50, game->font->size, make_color(1), true, game->font);
			pos.y += game->font->size;
			draw_text(g_r, "Press ENTER to play again", pos, 50, game->font->size * 0.5f, make_color(0.5f), true, game->font);
			pos.y += game->font->size * 0.5f;
			draw_text(g_r, "Press Escape to exit", pos, 50, game->font->size * 0.5f, make_color(0.5f), true, game->font);
			draw_texture(g_r,
				c_half_res, 10, c_base_res, make_color(1), game->text_framebuffer->texture,
				{.blend_mode = e_blend_mode_additive}
			);
		} break;
	}

	for(int i = 0; i < c_max_keys; i++) {
		g_input->keys[i].count = 0;
	}

	game->variables.count = 0;
	g_ui.hovered.id = 0;
	if(!g_ui.pressed_present && g_ui.pressed.id != 0) {
		ui_request_pressed(0);
	}
	g_ui.pressed_present = false;

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
		if(game->apple == pos) { collision = true; }
		if(!collision) { break; }
	}
	return pos;
}

template <typename t>
func void live_variable_(t* ptr, const char* name, t min_val, t max_val)
{
	constexpr b8 is_int = is_same<t, int>;
	constexpr b8 is_float = is_same<t, float>;
	constexpr b8 is_bool = is_same<t, b8>;
	static_assert(is_int || is_float || is_bool);
	s_var var = zero;
	var.display = true;
	var.ptr = ptr;
	var.name = name;
	if constexpr(is_int) {
		var.type = e_var_type_int;
		var.min_val.val_int = min_val;
		var.max_val.val_int = max_val;
	}
	else if constexpr(is_float) {
		var.type = e_var_type_float;
		var.min_val.val_float = min_val;
		var.max_val.val_float = max_val;
	}
	else if constexpr(is_bool) {
		var.type = e_var_type_bool;
	}
	s_var* temp = get_var_by_ptr(ptr);
	if(temp) {
		*temp = var;
	}
	else {
		game->variables.add(var);
	}
}

template <typename t>
func void register_live_variable_(t* ptr, const char* name)
{
	constexpr b8 is_int = is_same<t, int>;
	constexpr b8 is_float = is_same<t, float>;
	constexpr b8 is_bool = is_same<t, b8>;
	static_assert(is_int || is_float || is_bool);
	s_var var = zero;
	var.ptr = ptr;
	var.name = name;
	if constexpr(is_int) {
		var.type = e_var_type_int;
	}
	else if constexpr(is_float) {
		var.type = e_var_type_float;
	}
	else if constexpr(is_bool) {
		var.type = e_var_type_bool;
	}
	assert(get_var_by_ptr(ptr) == null);
	game->variables.add(var);
}

func s_var* get_var_by_ptr(void* ptr)
{
	foreach_ptr(var_i, var, game->variables) {
		if(var->ptr == ptr) { return var; }
	}
	return null;
}

func void ui_request_hovered(u32 id)
{
	if(g_ui.pressed.id != 0) { return; }
	g_ui.hovered.id = id;
}

func void ui_request_pressed(u32 id)
{
	g_ui.hovered.id = 0;
	g_ui.pressed.id = id;
}

func void ui_request_active(u32 id)
{
	g_ui.hovered.id = 0;
	g_ui.pressed.id = 0;
}

func s_ui_interaction ui_button(const char* text, s_v2 pos, s_v2 size, s_font* font, float font_size)
{
	s_ui_interaction result = zero;
	u32 id = hash(text);
	s_v4 button_color = rgb(0x217278);
	b8 hovered = mouse_collides_rect_topleft(mouse, pos, size);
	if(hovered) {
		ui_request_hovered(id);
	}
	if(g_ui.hovered.id == id) {
		result.state = e_ui_hovered;
		button_color = brighter(button_color, 1.4f);
		if(hovered && is_key_pressed(g_input, c_left_mouse)) {
			result.pressed_this_frame = true;
			ui_request_pressed(id);
		}
		else if(!hovered) {
			ui_request_hovered(0);
		}
	}
	if(g_ui.pressed.id == id) {
		result.state = e_ui_pressed;
		g_ui.pressed_present = true;
		button_color = brighter(button_color, 0.6f);
		if(is_key_released(g_input, c_left_mouse)) {
			if(hovered) {
				result.state = e_ui_active;
				ui_request_active(id);
			}
			else {
				ui_request_pressed(0);
			}
		}
	}

	draw_rect(g_r, pos, 10, size, button_color, zero, {.origin_offset = c_origin_topleft});
	if(font) {
		s_v2 text_pos = pos;
		text_pos += size / 2;
		draw_text(g_r, text, text_pos, 11, font_size, rgb(0xFB9766), true, font);
	}
	return result;
}

template <typename t>
func t ui_slider(const char* text, s_v2 pos, s_v2 size, s_font* font, float font_size, t min_val, t max_val, t curr_val)
{
	constexpr b8 is_int = is_same<t, int>;
	constexpr b8 is_float = is_same<t, float>;
	static_assert(is_int || is_float);

	t result = curr_val;
	u32 id = hash(text);
	s_v4 button_color = rgb(0x217278);
	s_v4 handle_color = rgb(0xEA5D58);
	s_v2 handle_size = v2(size.y);
	b8 hovered = mouse_collides_rect_topleft(mouse, pos, size);
	if(hovered) {
		ui_request_hovered(id);
	}
	if(g_ui.hovered.id == id) {
		button_color = brighter(button_color, 1.4f);
		if(hovered && is_key_pressed(g_input, c_left_mouse)) {
			ui_request_pressed(id);
		}
		else if(!hovered) {
			ui_request_hovered(0);
		}
	}
	if(g_ui.pressed.id == id) {
		g_ui.pressed_present = true;
		button_color = brighter(button_color, 0.6f);
		if(is_key_released(g_input, c_left_mouse)) {
			if(hovered) {
				ui_request_active(id);
			}
			else {
				ui_request_pressed(0);
			}
		}
		float percent;
		if(is_key_down(g_input, c_key_left_ctrl)) {
			percent = ilerp(handle_size.x * 0.5f, c_base_res.x - handle_size.x * 0.5f, mouse.x);
		}
		else {
			percent = ilerp(pos.x + handle_size.x * 0.5f, pos.x + size.x - handle_size.x * 0.5f, mouse.x);
		}
		result = (t)lerp((float)min_val, (float)max_val, percent);
	}
	result = clamp(result, min_val, max_val);

	s_v2 handle_pos = v2(
		pos.x + ilerp((float)min_val, (float)max_val, (float)result) * (size.x - handle_size.y),
		pos.y - handle_size.y / 2 + size.y / 2
	);

	draw_rect(g_r, pos, 10, size, button_color, zero, {.origin_offset = c_origin_topleft});
	draw_rect(g_r, handle_pos, 11, handle_size, handle_color, zero, {.flags = e_render_flag_circle, .origin_offset = c_origin_topleft});
	s_v2 text_pos = pos;
	text_pos += size / 2;
	if(is_int) {
		draw_text(g_r, format_text("%i", result), text_pos, 15, font_size, rgb(0xFB9766), true, font);
	}
	else if(is_float) {
		draw_text(g_r, format_text("%.2f", result), text_pos, 15, font_size, rgb(0xFB9766), true, font);
	}
	return result;
}

func void ui_checkbox(const char* text, s_v2 pos, s_v2 size, b8* val)
{
	assert(val);
	u32 id = hash(text);
	s_v4 button_color = rgb(0x217278);
	b8 hovered = mouse_collides_rect_topleft(mouse, pos, size);
	if(hovered) {
		ui_request_hovered(id);
	}
	if(g_ui.hovered.id == id) {
		button_color = brighter(button_color, 1.4f);
		if(hovered && is_key_pressed(g_input, c_left_mouse)) {
			ui_request_pressed(id);
		}
		else if(!hovered) {
			ui_request_hovered(0);
		}
	}
	if(g_ui.pressed.id == id) {
		g_ui.pressed_present = true;
		button_color = brighter(button_color, 0.6f);
		if(is_key_released(g_input, c_left_mouse)) {
			if(hovered) {
				*val = !(*val);
				ui_request_active(id);
			}
			else {
				ui_request_pressed(0);
			}
		}
	}

	draw_rect(g_r, pos, 10, size, button_color, zero, {.origin_offset = c_origin_topleft});
	if(*val) {
		draw_texture(g_r, pos, 11, size, make_color(0,1,0), game->checkmark_texture, zero, {.origin_offset = c_origin_topleft});
	}
}

