
#include "pch_client.h"

#define STB_TRUETYPE_IMPLEMENTATION
#define STBTT_assert assert
#include "external/stb_truetype.h"

#include "config.h"
#include "shader_shared.h"
#include "bucket.h"
#include "platform_shared.h"
#include "client.h"
#include "audio.h"

global s_sarray<s_transform, c_max_entities> text_arr[e_font_count];
global s_lin_arena* frame_arena;
global s_game_window g_window;
global s_input* g_logic_input;
global s_input* g_input;
global s_platform_data* g_platform_data;
global s_platform_funcs g_platform_funcs;
global s_game* game;
global s_v2 previous_mouse;
global s_ui g_ui;
global s_game_renderer* g_r;

global s_shader_paths shader_paths[e_shader_count] = {
	{
		.vertex_path = "shaders/vertex.vertex",
		.fragment_path = "shaders/fragment.fragment",
	},
};



#include "draw.cpp"
#include "memory.cpp"
#include "file.cpp"
#include "str_builder.cpp"
#include "audio.cpp"
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


func void play_delayed_sound(s_sound sound, float delay)
{
	s_delayed_sound s = zero;
	s.sound = sound;
	s.delay = delay;
	game->delayed_sounds.add_checked(s);
}

func s_bounds get_camera_bounds(s_camera camera)
{
	s_bounds result;
	result.left = camera.center.x - c_base_res.x / 2;
	result.right = camera.center.x + c_base_res.x / 2;
	result.top = camera.center.y - c_base_res.y / 2;
	result.bottom = camera.center.y + c_base_res.y / 2;
	return result;
}

func s_v2 get_world_mouse(s_camera camera)
{
	s_v2 result = zero;
	result.x = g_platform_data->mouse.x + (camera.center.x - c_half_res.x);
	result.y = g_platform_data->mouse.y + (camera.center.y - c_half_res.y);
	return result;
}

func s_v2i get_hovered_tile(s_camera camera)
{
	s_v2 mouse = get_world_mouse(camera);
	return point_to_tile(mouse);
}

func s_v2i point_to_tile(s_v2 pos)
{
	int x_index = floorfi(pos.x / (float)c_tile_size);
	int y_index = floorfi(pos.y / (float)c_tile_size);
	return s_v2i(x_index, y_index);
}

func float get_dig_delay()
{
	float result = c_dig_delay;
	float inc = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_dig_speed]; i++)
	{
		inc += get_upgrade_value(e_upgrade_dig_speed, i) / 100;
	}
	result /= inc;
	if(game->super_dig)
	{
		result *= 0.1f;
	}
	return result;
}

func float get_dig_range()
{
	float result = c_dig_range;

	float inc = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_dig_range]; i++)
	{
		inc += get_upgrade_value(e_upgrade_dig_range, i) / 100;
	}
	result *= inc;

	if(game->super_dig)
	{
		result *= 3;
	}
	return result;
}

func float get_movement_speed()
{
	float result = c_player_speed;

	float inc = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_movement_speed]; i++)
	{
		inc += get_upgrade_value(e_upgrade_movement_speed, i) / 100;
	}
	result *= inc;

	if(game->high_speed)
	{
		result *= 5;
	}
	return result;
}

func int get_max_health()
{
	int result = c_player_health;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_health]; i++)
	{
		result += (int)get_upgrade_value(e_upgrade_health, i);
	}
	return result;
}

func int get_max_jumps()
{
	int result = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_extra_jump]; i++)
	{
		result += (int)get_upgrade_value(e_upgrade_extra_jump, i);
	}
	return result;
}

func int get_how_many_blocks_can_dash_break()
{
	int result = 0;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_dash]; i++)
	{
		result += (int)get_upgrade_value(e_upgrade_dash, i);
	}
	return result;
}

func void add_upgrade_to_queue()
{
	game->transient.upgrades_queued += 1;

	if(game->transient.winning) { return; }

	if(game->transient.upgrades_queued == 1)
	{
		trigger_upgrade_menu();
	}
}

func void trigger_upgrade_menu()
{
	assert(!game->transient.in_upgrade_menu);
	s_rng* rng = &game->rng;
	game->transient.in_upgrade_menu = true;
	game->transient.upgrade_choices.count = 0;
	game->transient.upgrade_index = 0;

	s_sarray<int, e_upgrade_count> available_upgrades = zero;
	for(int upgrade_i = 0; upgrade_i < e_upgrade_count; upgrade_i++)
	{
		if(upgrade_i == e_upgrade_dash_cd && game->transient.upgrades_chosen[e_upgrade_dash] <= 0) { continue; }
		available_upgrades.add(upgrade_i);
	}

	for(int i = 0; i < 3; i++)
	{
		int rand_index = rng->randu() % available_upgrades.count;
		game->transient.upgrade_choices.add(available_upgrades[rand_index]);
		available_upgrades.remove_and_swap(rand_index);
	}
}

func b8 mouse_collides_rect_topleft(s_v2 mouse, s_v2 pos, s_v2 size)
{
	return rect_collides_rect_topleft(mouse, v2(1, 1), pos, size);
}

func void apply_upgrade(int index)
{
	game->transient.upgrades_chosen[index] += 1;
	game->transient.in_upgrade_menu = false;

	game->transient.upgrades_queued -= 1;
	if(game->transient.upgrades_queued > 0)
	{
		trigger_upgrade_menu();
	}
}

func char* get_upgrade_description(int id, int level)
{
	float value = get_upgrade_value(id, level);
	switch(id)
	{
		case e_upgrade_dig_speed:
		{
			return format_text("+%0.f%% digging speed", value);
		} break;

		case e_upgrade_dig_range:
		{
			return format_text("+%0.f%% dig range", value);
		} break;

		case e_upgrade_movement_speed:
		{
			return format_text("+%0.f%% movement speed", value);
		} break;

		case e_upgrade_health:
		{
			return format_text("+%0.f health", value);
		} break;

		case e_upgrade_extra_jump:
		{
			return format_text("+%0.f jump", value);
		} break;

		case e_upgrade_slower_kill_area:
		{
			return format_text("The void moves %0.f%% slower", value);
		} break;

		case e_upgrade_dash:
		{
			if(level == 0)
			{
				return format_text("Press F to dash, breaking %0.0f blocks", value);
			}
			else
			{
				return format_text("Dash can break an extra block");
			}
		} break;

		case e_upgrade_dash_cd:
		{
			return format_text("-%.0f%% dash cooldown", value);
		} break;

		invalid_default_case;
	}
	return null;
}

func float get_upgrade_value(int id, int level)
{
	switch(id)
	{
		case e_upgrade_dig_speed:
		{
			return 50.0f + 10 * level;
		} break;

		case e_upgrade_dig_range:
		{
			return 30.0f + 5 * level;
		} break;

		case e_upgrade_movement_speed:
		{
			return 25.0f + 5 * level;
		} break;

		case e_upgrade_health:
		{
			return 1;
		} break;

		case e_upgrade_extra_jump:
		{
			return 1;
		} break;

		case e_upgrade_slower_kill_area:
		{
			return 5;
		} break;

		case e_upgrade_dash:
		{
			if(level == 0) { return 3; }
			else { return 1; }
		} break;

		case e_upgrade_dash_cd:
		{
			return 10;
		} break;

		invalid_default_case;
	}
	return 0;
}

func int pick_from_weights(s64* weights, int count)
{
	s64 total = 0;
	for(int i = 0; i < count; i++)
	{
		s64 weight = weights[i];
		total += weight;
	}
	for(int i = 0; i < count; i++)
	{
		s64 weight = weights[i];
		s64 rand = game->rng.randu() % total;
		if(rand < weight) { return i; }
		total -= weight;
	}
	return -1;
}

func void add_exp(int exp)
{
	s_player* player = &game->transient.player;
	player->exp += exp;
	int required_exp = get_required_exp_to_level_up(player->level);
	game->transient.exp_gained_time = game->total_time;
	while(player->exp >= required_exp)
	{
		player->level += 1;
		player->exp -= required_exp;
		required_exp = get_required_exp_to_level_up(player->level);
		add_upgrade_to_queue();
	}
}

func int get_required_exp_to_level_up(int level)
{
	float result = 10;
	result += (level - 1) * 10;
	result = powf(result, 1.1f);
	return ceilfi(result);
}

func float get_max_y_vel()
{
	return 1000.0f * (game->high_gravity ? 5 : 1);
}

func void begin_winning()
{
	assert(!game->transient.winning);
	assert(!game->transient.won);
	game->transient.winning = true;
	game->transient.won = true;
	play_sound_if_supported(e_sound_win);

}

func s_v2 world_to_screen(s_v2 pos, s_camera cam)
{
	s_v2 result;
	result.x = c_base_res.x / 2 - (cam.center.x - pos.x);
	result.y = c_base_res.y / 2 - (cam.center.y - pos.y);
	return result;
}

func s_v2 get_camera_wanted_center(s_player player)
{
	s_v2 result = player.pos;

	if(game->camera_bounds)
	{
		s_bounds cam_bounds = get_camera_bounds(game->camera);
		float right_limit = c_tiles_right * c_tile_size;
		result.x = at_least(c_half_res.x, result.x);
		result.x = at_most(right_limit - c_half_res.x, result.x);
	}
	return result;

}

