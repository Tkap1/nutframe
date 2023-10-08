
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

global s_sarray<s_transform, 16384> particles;
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

global s_shader_paths shader_paths[e_shader_count] = {
	{
		.vertex_path = "shaders/vertex.vertex",
		.fragment_path = "shaders/fragment.fragment",
	},
};

global s_game_renderer* g_r;


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
	game = (s_game*)game_memory;
	g_r = rendering;
	if(!game->initialized)
	{
		game->initialized = true;
		g_r->set_vsync(true);
		game->noise = g_r->load_texture(rendering, "assets/noise.png");
	}

	g_input = platform_data->input;
	for(int i = 0; i < c_max_keys; i++)
	{
		g_input->keys[i].count = 0;
	}

	draw_rect(c_half_res, 0, c_base_res, make_color(121/255.0f * 0.2f, 241/255.0f * 0.2f, 255/255.0f * 0.2f));
	draw_texture(platform_data->mouse, 1, v2(1024), make_color(1), game->noise, {.effect_id = 1});
}

#ifdef m_build_dll
}
#endif // m_build_dll

func void update()
{

}

func void render(float dt)
{
	s_camera interpolated_camera = game->camera;
	switch(game->state)
	{
		case e_state_main_menu:
		{
			s_v2i res = c_resolutions[game->current_resolution_index];
		} break;

		case e_state_game:
		{
		} break;

		case e_state_victory:
		{
			if(game->best_time < game->transient.beat_time)
			{
			}
			else
			{
			}

			if(is_key_pressed(g_input, c_key_enter))
			{
				game->state = e_state_game;
				game->transient.winning = false;
			}
			if(is_key_pressed(g_input, c_key_r))
			{
				game->state = e_state_game;
			}
			if(is_key_pressed(g_input, c_key_escape))
			{
				// ExitProcess(0);
			}
		} break;

		invalid_default_case;
	}

	foreach_raw(particle_i, particle, game->particles)
	{
		s_v2 pos = lerp(particle.prev_pos, particle.pos, dt);
		s_v4 color = particle.color;
		float percent = (particle.time / particle.duration);
		float percent_left = 1.0f - percent;
		if(particle.fade)
		{
			color.w *= powf(percent_left, 0.5f);
		}
		float radius = particle.radius;
		if(particle.shrink)
		{
			radius *= range_lerp(percent, 0, 1, 1, 0.2f);
		}
	}

	#ifdef m_debug
	hot_reload_shaders();
	#endif // m_debug
}

func s_font load_font(const char* path, float font_size, s_lin_arena* arena)
{
	s_font font = zero;
	font.size = font_size;

	u8* file_data = (u8*)read_file(path, arena);
	assert(file_data);

	stbtt_fontinfo info = zero;
	stbtt_InitFont(&info, file_data, 0);

	stbtt_GetFontVMetrics(&info, &font.ascent, &font.descent, &font.line_gap);

	font.scale = stbtt_ScaleForPixelHeight(&info, font_size);
	constexpr int max_chars = 128;
	int bitmap_count = 0;
	u8* bitmap_arr[max_chars];
	const int padding = 10;

	int columns = floorfi(4096 / (font_size + padding));
	int rows = ceilfi((max_chars - columns) / (float)columns) + 1;

	int total_width = floorfi(columns * (font_size + padding));
	int total_height = floorfi(rows * (font_size + padding));

	for(int char_i = 0; char_i < max_chars; char_i++)
	{
		s_glyph glyph = zero;
		u8* bitmap = stbtt_GetCodepointBitmap(&info, 0, font.scale, char_i, &glyph.width, &glyph.height, 0, 0);
		stbtt_GetCodepointBox(&info, char_i, &glyph.x0, &glyph.y0, &glyph.x1, &glyph.y1);
		stbtt_GetGlyphHMetrics(&info, char_i, &glyph.advance_width, null);

		font.glyph_arr[char_i] = glyph;
		bitmap_arr[bitmap_count++] = bitmap;
	}

	// @Fixme(tkap, 23/06/2023): Use arena
	u8* gl_bitmap = (u8*)calloc(1, sizeof(u8) * 4 * total_width * total_height);

	for(int char_i = 0; char_i < max_chars; char_i++)
	{
		s_glyph* glyph = &font.glyph_arr[char_i];
		u8* bitmap = bitmap_arr[char_i];
		int column = char_i % columns;
		int row = char_i / columns;
		for(int y = 0; y < glyph->height; y++)
		{
			for(int x = 0; x < glyph->width; x++)
			{
				int current_x = floorfi(column * (font_size + padding));
				int current_y = floorfi(row * (font_size + padding));
				u8 src_pixel = bitmap[x + y * glyph->width];
				u8* dst_pixel = &gl_bitmap[((current_x + x) + (current_y + y) * total_width) * 4];
				dst_pixel[0] = 255;
				dst_pixel[1] = 255;
				dst_pixel[2] = 255;
				dst_pixel[3] = src_pixel;
			}
		}

		glyph->uv_min.x = column / (float)columns;
		glyph->uv_max.x = glyph->uv_min.x + (glyph->width / (float)total_width);

		glyph->uv_min.y = row / (float)rows;

		// @Note(tkap, 17/05/2023): For some reason uv_max.y is off by 1 pixel (checked the texture in renderoc), which causes the text to be slightly miss-positioned
		// in the Y axis. "glyph->height - 1" fixes it.
		glyph->uv_max.y = glyph->uv_min.y + (glyph->height / (float)total_height);

		// @Note(tkap, 17/05/2023): Otherwise the line above makes the text be cut off at the bottom by 1 pixel...
		// glyph->uv_max.y += 0.01f;
	}

	// font.texture = load_texture_from_data(gl_bitmap, total_width, total_height, GL_LINEAR);

	for(int bitmap_i = 0; bitmap_i < bitmap_count; bitmap_i++)
	{
		stbtt_FreeBitmap(bitmap_arr[bitmap_i], null);
	}

	free(gl_bitmap);

	return font;
}

func s_texture load_texture_from_data(void* data, int width, int height, u32 filtering)
{
	// assert(data);
	// u32 id;
	// glGenTextures(1, &id);
	// glBindTexture(GL_TEXTURE_2D, id);
	// glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
	// glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);

	s_texture texture = zero;
	// texture.id = id;
	// texture.size = v22i(width, height);
	return texture;
}

func s_texture load_texture_from_file(char* path, u32 filtering)
{
	// int width, height, num_channels;
	// void* data = stbi_load(path, &width, &height, &num_channels, 4);
	// s_texture texture = load_texture_from_data(data, width, height, filtering);
	// stbi_image_free(data);
	return zero;
}

func s_v2 get_text_size_with_count(const char* text, e_font font_id, int count)
{
	assert(count >= 0);
	if(count <= 0) { return zero; }
	s_font* font = &game->font_arr[font_id];

	s_v2 size = zero;
	size.y = font->size;

	for(int char_i = 0; char_i < count; char_i++)
	{
		char c = text[char_i];
		s_glyph glyph = font->glyph_arr[c];
		size.x += glyph.advance_width * font->scale;
	}

	return size;
}

func s_v2 get_text_size(const char* text, e_font font_id)
{
	return get_text_size_with_count(text, font_id, (int)strlen(text));
}


#ifdef m_debug
func void hot_reload_shaders(void)
{
	// for(int shader_i = 0; shader_i < e_shader_count; shader_i++)
	// {
	// 	s_shader_paths* sp = &shader_paths[shader_i];

	// 	WIN32_FIND_DATAA find_data = zero;
	// 	HANDLE handle = FindFirstFileA(sp->fragment_path, &find_data);
	// 	if(handle == INVALID_HANDLE_VALUE) { continue; }

	// 	if(CompareFileTime(&sp->last_write_time, &find_data.ftLastWriteTime) == -1)
	// 	{
	// 		// @Note(tkap, 23/06/2023): This can fail because text editor may be locking the file, so we check if it worked
	// 		u32 new_program = load_shader(sp->vertex_path, sp->fragment_path);
	// 		if(new_program)
	// 		{
	// 			if(game->programs[shader_i])
	// 			{
	// 				glUseProgram(0);
	// 				glDeleteProgram(game->programs[shader_i]);
	// 			}
	// 			game->programs[shader_i] = load_shader(sp->vertex_path, sp->fragment_path);
	// 			sp->last_write_time = find_data.ftLastWriteTime;
	// 		}
	// 	}

	// 	FindClose(handle);
	// }

}
#endif // m_debug

func u32 load_shader(const char* vertex_path, const char* fragment_path)
{
	return 0;
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

func void spawn_particles(int count, s_particle_spawn_data data)
{
	for(int i = 0; i < count; i++)
	{
		s_particle p = zero;
		p.duration = data.duration * (1 - data.duration_rand * game->rng.randf32());
		p.speed = data.speed * (1 - data.speed_rand * game->rng.randf32());
		p.radius = data.radius * (1 - data.radius_rand * game->rng.randf32());
		p.pos = data.pos;
		p.prev_pos = p.pos;

		float foo = (float)game->rng.randf2() * data.angle_rand * tau;
		float angle = data.angle + foo;
		p.dir = v2_from_angle(angle);
		p.color.x = data.color.x * (1 - data.color_rand * game->rng.randf32());
		p.color.y = data.color.y * (1 - data.color_rand * game->rng.randf32());
		p.color.z = data.color.z * (1 - data.color_rand * game->rng.randf32());
		p.color.w = data.color.w;
		p.fade = data.fade;
		p.shrink = data.shrink;
		game->particles.add_checked(p);
	}
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

func s_tile_collision get_tile_collision(s_v2 pos, s_v2 size)
{
	s_tile_collision result = zero;
	int x_index = floorfi(pos.x / (float)c_tile_size);
	int y_index = floorfi(pos.y / (float)c_tile_size);

	for(int y = -1; y <= 1; y++)
	{
		int yy = y_index + y;
		for(int x = -1; x <= 1; x++)
		{
			int xx = x_index + x;
			if(!is_valid_tile_index(xx, yy)) { continue; }
			if(!is_tile_active(xx, yy)) { continue; }
			s_v2 tile_center = v2(xx * c_tile_size, yy * c_tile_size);
			tile_center += v2(c_tile_size) / 2;
			if(rect_collides_rect_center(pos, size, tile_center, v2(c_tile_size)))
			{
				result.collided = true;
				result.index = v2i(xx, yy);
				return result;
			}
		}
	}

	return result;
}

func b8 is_valid_tile_index(int x, int y)
{
	return x >= 0 && x < c_tiles_right && y >= 0 && y < c_tiles_down;
}

func b8 is_valid_tile_index(s_v2i p)
{
	return is_valid_tile_index(p.x, p.y);
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

func b8 is_tile_active(int x, int y)
{
	assert(is_valid_tile_index(x, y));
	return game->tiles_active[y][x];
}

func b8 is_tile_active(s_v2i index)
{
	return is_tile_active(index.x, index.y);
}

func s_v2i get_closest_tile_to_mouse(s_camera camera)
{
	s_v2 mouse = get_world_mouse(camera);
	int mx = floorfi(mouse.x / c_tile_size);
	int my = floorfi(mouse.y / c_tile_size);

	// @Note(tkap, 01/10/2023): Let's just check on a 17x17 tile radius
	float shortest_dist = 999999.0f;
	s_v2i closest = v2i(-1, -1);
	for(int y = -8; y <= 8; y++)
	{
		int yy = my + y;
		for(int x = -8; x <= 8; x++)
		{
			int xx = mx + x;
			if(is_valid_tile_index(xx, yy) && is_tile_active(xx, yy))
			{
				float dist = v2_distance(mouse, get_tile_center(xx, yy));
				if(dist < shortest_dist)
				{
					shortest_dist = dist;
					closest = v2i(xx, yy);
				}
			}
		}
	}

	return closest;
}

func s_v2 get_tile_center(int x, int y)
{
	assert(is_valid_tile_index(x, y));
	return v2(
		x * c_tile_size + c_tile_size / 2.0f,
		y * c_tile_size + c_tile_size / 2.0f
	);
}

func s_v2 get_tile_center(s_v2i index)
{
	return get_tile_center(index.x, index.y);
}

func s_v2 world_to_screen(s_v2 pos, s_camera cam)
{
	s_v2 result;
	result.x = c_base_res.x / 2 - (cam.center.x - pos.x);
	result.y = c_base_res.y / 2 - (cam.center.y - pos.y);
	return result;
}

func s_v2 get_tile_pos(s_v2i index)
{
	assert(is_valid_tile_index(index));
	return v2(
		(float)(index.x * c_tile_size),
		(float)(index.y * c_tile_size)
	);
}


func void do_tile_particles(s_v2 pos, int tile_type, int type)
{
	s_v4 color = g_tile_data[tile_type].particle_color;
	color.w *= type == 0 ? 0.5f : 1.0f;
	spawn_particles(type == 0 ? 3 : 20, {
		.speed = 75.0f,
		.speed_rand = 0.5f,
		.radius = type == 0 ? 8.0f : 16.0f,
		.radius_rand = 0.5f,
		.duration = 0.5f,
		.duration_rand = 0.5f,
		.angle = 0,
		.angle_rand = 1,
		.pos = pos,
		.color = color,
	});
}

func s_label_group begin_label_group(s_v2 pos, e_font font_type, int selected, float spacing)
{
	s_label_group result = zero;
	result.pos = pos;
	result.font_type = font_type;
	result.default_selected = selected;
	result.spacing = spacing;
	return result;
}

func s_ui_state add_label(s_label_group* group, char* text)
{
	s_ui_state result = zero;
	u32 id = hash(text);
	int index = group->ids.count;

	if(index == group->default_selected && g_ui.selected.id == 0 && g_ui.hovered.id == 0 && g_ui.pressed.id == 0)
	{
		ui_request_selected(id, index);
	}

	group->ids.add(id);
	s_v4 color = rgb(0.7f, 0.7f, 0.7f);
	s_v2 label_size = get_text_size(text, group->font_type);
	s_v2 label_topleft = group->pos - label_size / 2;
	b8 hovered = mouse_collides_rect_topleft(g_platform_data->mouse, label_topleft, label_size);
	b8 selected = id == g_ui.selected.id;
	if(hovered || (selected && g_ui.hovered.id == 0))
	{
		ui_request_hovered(id, index);
	}
	if(g_ui.hovered.id == id)
	{
		color = rgb(1, 1, 0);
		if(hovered)
		{
			if(is_key_pressed(g_input, c_left_mouse))
			{
				ui_request_pressed(id, index);
			}
		}
		else
		{
			g_ui.hovered = zero;
		}
		if(is_key_pressed(g_input, c_key_enter))
		{
			result.clicked = true;
		}
	}
	if(g_ui.pressed.id == id)
	{
		color = rgb(0.7f, 0.7f, 0);
		if(is_key_released(g_input, c_left_mouse))
		{
			if(hovered)
			{
				ui_request_active(id);
				result.clicked = true;
			}
			else
			{
				g_ui.pressed = zero;
			}
		}
	}

	group->pos.y += group->spacing;

	if(result.clicked)
	{
		g_ui = zero;
	}

	return result;
}

func int end_label_group(s_label_group* group)
{
	if(g_ui.hovered.id != 0) { return g_ui.hovered.index; }
	if(g_ui.pressed.id != 0) { return g_ui.pressed.index; }
	int selected = g_ui.selected.index;
	if(is_key_pressed(g_input, c_key_up))
	{
		selected = circular_index(selected - 1, group->ids.count);
	}
	if(is_key_pressed(g_input, c_key_down))
	{
		selected = circular_index(selected + 1, group->ids.count);
	}
	ui_request_selected(group->ids[selected], selected);
	return selected;
}

func u32 hash(const char* text)
{
	assert(text);
	u32 hash = 5381;
	while(true)
	{
		int c = *text;
		text += 1;
		if(!c) { break; }
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

func void ui_request_selected(u32 id, int index)
{
	if(g_ui.hovered.id != 0) { return; }
	if(g_ui.pressed.id != 0) { return; }
	g_ui.selected.id = id;
	g_ui.selected.index = index;
}

func void ui_request_hovered(u32 id, int index)
{
	if(g_ui.pressed.id != 0) { return; }
	g_ui.hovered.id = id;
	g_ui.hovered.index = index;
	g_ui.selected.id = id;
	g_ui.selected.index = index;
}

func void ui_request_pressed(u32 id, int index)
{
	g_ui.hovered = zero;
	g_ui.pressed.id = id;
	g_ui.pressed.index = index;
}

func void ui_request_active(u32 id)
{
	unreferenced(id);
	g_ui.hovered = zero;
	g_ui.pressed = zero;
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

func float get_kill_area_speed()
{
	float result = game->transient.kill_area_speed;

	float dec = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_slower_kill_area]; i++)
	{
		dec -= get_upgrade_value(e_upgrade_slower_kill_area, i) / 100;
	}
	dec = at_least(0.0f, dec);
	result *= dec;

	return result;
}

func void damage_tile(s_v2i index, int damage)
{
	assert(is_valid_tile_index(index));
	assert(is_tile_active(index));
	s_tile tile = game->tiles[index.y][index.x];
	int health = g_tile_data[tile.type].health;
	assert(health > 0);
	game->tiles[index.y][index.x].damage_taken += damage;
	if(game->tiles[index.y][index.x].damage_taken >= health)
	{
		add_exp(g_tile_data[tile.type].exp);

		do_tile_particles(
			random_point_in_rect_topleft(get_tile_pos(index), v2(c_tile_size), &game->rng),
			tile.type, 1
		);
		play_sound_if_supported(g_tile_data[tile.type].break_sound);
		game->tiles_active[index.y][index.x] = false;

		s_v2 tile_pos = get_tile_pos(index);
		s_v2 tile_center = get_tile_center(index);
		for(int y = 0; y < c_tile_pieces; y++)
		{
			for(int x = 0; x < c_tile_pieces; x++)
			{
				s_broken_tile piece = zero;
				piece.type = tile.type;
				piece.pos = tile_pos + v2(x * (c_tile_size / c_tile_pieces), y * (c_tile_size / c_tile_pieces));
				piece.prev_pos = piece.pos;
				piece.index = v2i(x, y);
				piece.sub_size = v2(1.0f / (float)c_tile_pieces);
				s_v2 center = piece.pos + v2(c_tile_size / (c_tile_pieces * 2));
				piece.dir = v2_normalized(center - tile_center) * game->rng.randf32();
				game->transient.broken_tiles.add_checked(piece);
			}
		}

	}
	else
	{
		s_v2 pos;
		if(get_hovered_tile(game->camera) == index) { pos = get_world_mouse(game->camera); }
		else { pos = random_point_in_rect_topleft(get_tile_pos(index), v2(c_tile_size), &game->rng); }
		do_tile_particles(
			pos, tile.type, 0
		);
		play_sound_if_supported(e_sound_dig);
	}
}

func float get_dash_cd()
{
	float result = c_dash_cd;

	float dec = 1;
	for(int i = 0; i < game->transient.upgrades_chosen[e_upgrade_dash_cd]; i++)
	{
		dec -= get_upgrade_value(e_upgrade_dash_cd, i) / 100;
	}
	dec = at_least(0.0f, dec);
	result *= dec;

	return result;
}

func s_sprite_data get_animation_frame(int animation_id, float time)
{
	float delay = g_animation[animation_id].delay;
	int index = roundfi(time / delay * g_animation[animation_id].frame_count);
	index %= g_animation[animation_id].frame_count;
	return g_animation[animation_id].frames[index];
}

func void set_animation(s_player* player, int animation_id)
{
	if(player->current_animation == animation_id) { return; }
	player->animation_timer = 0;
	player->current_animation = animation_id;
}