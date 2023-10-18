
func void apply_event_to_input(s_input* in_input, s_stored_input event)
{
	in_input->keys[event.key].is_down = event.is_down;
	in_input->keys[event.key].count += 1;
}

func b8 is_key_down(s_input* input, int key) {
	assert(key < c_max_keys);
	return input->keys[key].is_down || input->keys[key].count >= 2;
}

func b8 is_key_up(s_input* input, int key) {
	assert(key < c_max_keys);
	return !input->keys[key].is_down;
}

func b8 is_key_pressed(s_input* input, int key) {
	assert(key < c_max_keys);
	return (input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

func b8 is_key_released(s_input* input, int key) {
	assert(key < c_max_keys);
	return (!input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

func int get_render_offset(int texture, int blend_mode)
{
	return texture * e_blend_mode_count + blend_mode;
}

// @Note(tkap, 15/10/2023): Should this always return font_size for y (like it does now), or actually get the tallest char?
func s_v2 get_text_size_with_count(const char* text, s_font* font, float font_size, int count)
{
	assert(count >= 0);
	if(count <= 0) { return zero; }

	s_v2 size = zero;
	float scale = font->scale * (font_size / font->size);

	for(int char_i = 0; char_i < count; char_i++)
	{
		char c = text[char_i];
		s_glyph glyph = font->glyph_arr[c];
		s_v2 draw_size = v2((glyph.x1 - glyph.x0) * scale, (glyph.y1 - glyph.y0) * scale);
		if(char_i == count - 1) {
			size.x += (glyph.x1 - glyph.x0) * scale;
		}
		else {
			size.x += glyph.advance_width * scale;
		}
		size.y = at_least(draw_size.y, size.y);
	}

	return size;
}

func s_v2 get_text_size(const char* text, s_font* font, float font_size)
{
	return get_text_size_with_count(text, font, font_size, (int)strlen(text));
}

func void draw_rect(s_game_renderer* game_renderer, s_v2 pos, int layer, s_v2 size, s_v4 color, s_render_data render_data = zero, s_transform t = zero)
{
	if(!render_data.framebuffer) {
		render_data.framebuffer = &game_renderer->framebuffers[0];
	}

	t.pos = pos;
	t.layer = layer;
	t.draw_size = size;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	t.mix_color = v41f(1);
	bucket_add(&render_data.framebuffer->transforms[get_render_offset(0, render_data.blend_mode)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

func void draw_texture(s_game_renderer* game_renderer, s_v2 pos, int layer, s_v2 size, s_v4 color, s_texture texture, s_render_data render_data = zero, s_transform t = zero)
{
	if(!render_data.framebuffer) {
		render_data.framebuffer = &game_renderer->framebuffers[0];
	}

	t.layer = layer;
	t.flags |= e_render_flag_use_texture;
	t.pos = pos;
	t.draw_size = size;
	t.color = color;
	if(texture.comes_from_framebuffer) {
		t.uv_min = v2(0, 1);
		t.uv_max = v2(1, 0);
	}
	else {
		t.uv_min = v2(0);
		t.uv_max = v2(1);
	}
	t.mix_color = v41f(1);
	bucket_add(&render_data.framebuffer->transforms[get_render_offset(texture.game_id, render_data.blend_mode)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

func void draw_text(s_game_renderer* game_renderer, const char* text, s_v2 in_pos, int layer, float font_size, s_v4 color, b8 centered, s_font* font, s_render_data render_data = zero, s_transform t = zero)
{
	if(!render_data.framebuffer) {
		render_data.framebuffer = &game_renderer->framebuffers[0];
	}

	t.layer = layer;

	float scale = font->scale * (font_size / font->size);

	int len = (int)strlen(text);
	assert(len > 0);
	s_v2 pos = in_pos;
	s_v2 text_size = get_text_size(text, font, font_size);
	if(centered) {
		pos.x -= text_size.x / 2;
		pos.y -= text_size.y / 2;
	}
	// pos.y += font->ascent * scale;
	for(int char_i = 0; char_i < len; char_i++) {
		int c = text[char_i];
		if(c <= 0 || c >= 128) { continue; }

		s_glyph glyph = font->glyph_arr[c];
		t.draw_size = v2((glyph.x1 - glyph.x0) * scale, (glyph.y1 - glyph.y0) * scale);

		s_v2 glyph_pos = pos;
		glyph_pos.x += glyph.x0 * scale;
		glyph_pos.y += -glyph.y0 * scale;

		glyph_pos.y += text_size.y;
		t.flags |= e_render_flag_use_texture;
		t.pos = glyph_pos;

		s_v2 center = t.pos + t.draw_size / 2 * v2(1, -1);
		s_v2 bottomleft = t.pos;

		// s_v2 topleft = t.pos + t.draw_size * v2(0, -1);
		// draw_rect(t.pos, 1, t.draw_size, make_color(0.4f, 0,0), zero, {.origin_offset = c_origin_bottomleft});
		// draw_rect(center, 75, v2(4), make_color(0, 1,0), zero);
		// draw_rect(topleft, 75, v2(4), make_color(0, 0,1), zero, {.origin_offset = c_origin_topleft});
		// draw_rect(bottomleft, 75, v2(4), make_color(1, 1,0), zero, {.origin_offset = c_origin_bottomleft});
		// draw_rect(in_pos, 77, v2(4), make_color(0, 1,1), zero, {.origin_offset = c_origin_topleft});

		t.pos = v2_rotate_around(center, in_pos, t.rotation) + (bottomleft - center);

		t.color = color;
		t.uv_min = glyph.uv_min;
		t.uv_max = glyph.uv_max;
		t.origin_offset = c_origin_bottomleft;

		bucket_add(
			&render_data.framebuffer->transforms[get_render_offset(font->texture.game_id, render_data.blend_mode)], t,
			&game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc
		);

		pos.x += glyph.advance_width * scale;

	}
}

func s_lin_arena make_lin_arena(u64 capacity)
{
	assert(capacity > 0);
	capacity = (capacity + 7) & ~7;
	s_lin_arena result = zero;
	result.capacity = capacity;
	result.memory = malloc(capacity);
	return result;
}

func s_lin_arena make_lin_arena_from_memory(u64 capacity, void* memory)
{
	assert(capacity > 0);
	assert(memory);

	capacity = (capacity + 7) & ~7;
	s_lin_arena result = zero;
	result.capacity = capacity;
	result.memory = memory;
	return result;
}

func void* la_get(s_lin_arena* arena, u64 in_requested)
{
	assert(arena);
	assert(in_requested > 0);
	u64 requested = (in_requested + 7) & ~7;
	assert(arena->used + requested <= arena->capacity);
	void* result = (u8*)arena->memory + arena->used;
	arena->used += requested;
	return result;
}

func void* la_get_zero(s_lin_arena* arena, u64 in_requested)
{
	void* result = la_get(arena, in_requested);
	memset(result, 0, in_requested);
	return result;
}

func void la_push(s_lin_arena* arena)
{
	assert(arena->push_count < c_max_arena_push);
	arena->push[arena->push_count++] = arena->used;
}

func void la_pop(s_lin_arena* arena)
{
	assert(arena->push_count > 0);
	arena->used = arena->push[--arena->push_count];
}



template <typename t>
func void bucket_add(s_bucket_array<t>* arr, t new_element, s_lin_arena* arena, b8* did_we_alloc)
{
	for(int i = 0; i < arr->bucket_count; i++)
	{
		int* count = &arr->element_count[i];
		if(*count < arr->capacity[i])
		{
			arr->elements[i][*count] = new_element;
			*count += 1;
			return;
		}
	}
	assert(arr->bucket_count < 16);
	constexpr int capacity = c_bucket_capacity;
	arr->elements[arr->bucket_count] = (t*)la_get(arena, sizeof(t) * capacity);
	arr->capacity[arr->bucket_count] = capacity;
	arr->elements[arr->bucket_count][0] = new_element;
	arr->element_count[arr->bucket_count] = 1;
	arr->bucket_count += 1;

	if(arr->bucket_count > 1)
	{
		*did_we_alloc = true;
	}
}

template <typename t>
func void bucket_merge(s_bucket_array<t>* arr, s_lin_arena* arena)
{
	int capacity = 0;
	int count = 0;
	constexpr int element_size = sizeof(t);
	t* elements;

	if(arr->element_count[0] <= 0) { return; }

	for(int i = 0; i < arr->bucket_count; i++)
	{
		capacity += arr->capacity[i];
	}

	elements = (t*)la_get(arena, element_size * capacity);

	for(int i = 0; i < arr->bucket_count; i++)
	{
		assert(arr->element_count[i] > 0);
		memcpy(&elements[count], arr->elements[i], element_size * arr->element_count[i]);
		count += arr->element_count[i];
	}

	arr->elements[0] = elements;
	arr->capacity[0] = capacity;
	arr->element_count[0] = count;
	arr->bucket_count = 1;
}

func void on_failed_assert(const char* cond, const char* file, int line)
{
	printf("FAILED ASSERT: %s\n%s (%i)\n", cond, file, line);
	#ifndef __EMSCRIPTEN__
	__debugbreak();
	#endif // __EMSCRIPTEN__
	printf("Press ENTER to exit...");
	// *(char*)1 = 0;
	getchar();
	exit(1);
	// @Fixme(tkap, 05/10/2023):
	// char* text = format_text("FAILED ASSERT IN %s (%i)\n%s\n", file, line, cond);
	// printf("%s\n", text);
	// int result = MessageBox(null, text, "Assertion failed", MB_RETRYCANCEL | MB_TOPMOST);
	// if(result != IDRETRY)
	// {
	// 	if(IsDebuggerPresent())
	// 	{
	// 		__debugbreak();
	// 	}
	// 	else
	// 	{
	// 		exit(1);
	// 	}
	// }
}