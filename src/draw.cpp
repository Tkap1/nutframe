

enum e_render_flags
{
	e_render_flag_use_texture = 1 << 0,
	e_render_flag_flip_x = 1 << 1,
};

func void draw_rect(s_v2 pos, int layer, s_v2 size, s_v4 color, s_render_data render_data = zero, s_transform t = zero)
{
	if(!render_data.framebuffer) {
		render_data.framebuffer = &g_r->framebuffers[0];
	}

	t.pos = pos;
	t.layer = layer;
	t.draw_size = size;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	t.mix_color = v41f(1);
	bucket_add(&render_data.framebuffer->transforms[get_render_offset(0, render_data.blend_mode)], t, &g_r->arenas[g_r->arena_index], &g_r->did_we_alloc);
}

func void draw_texture(s_v2 pos, int layer, s_v2 size, s_v4 color, s_texture texture, s_render_data render_data = zero, s_transform t = zero)
{
	if(!render_data.framebuffer) {
		render_data.framebuffer = &g_r->framebuffers[0];
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
	bucket_add(&render_data.framebuffer->transforms[get_render_offset(texture.game_id, render_data.blend_mode)], t, &g_r->arenas[g_r->arena_index], &g_r->did_we_alloc);
}

func void draw_text(const char* text, s_v2 in_pos, int layer, float font_size, s_v4 color, b8 centered, s_font* font, s_render_data render_data = zero, s_transform t = zero)
{
	if(!render_data.framebuffer) {
		render_data.framebuffer = &g_r->framebuffers[0];
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
			&g_r->arenas[g_r->arena_index], &g_r->did_we_alloc
		);

		pos.x += glyph.advance_width * scale;

	}
}