
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
