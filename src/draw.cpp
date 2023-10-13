

enum e_render_flags
{
	e_render_flag_use_texture = 1 << 0,
	e_render_flag_flip_x = 1 << 1,
};


func void draw_rect(s_v2 pos, int layer, s_v2 size, s_v4 color, s_render_data render_data = zero, s_transform t = zero)
{
	assert(render_data.framebuffer_index >= 0);
	assert(render_data.framebuffer_index < g_r->framebuffers.count);
	s_framebuffer* framebuffer = &g_r->framebuffers[render_data.framebuffer_index];

	t.pos = pos;
	t.layer = layer;
	t.draw_size = size;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	t.mix_color = v41f(1);
	bucket_add(&framebuffer->transforms[get_render_offset(0, render_data.blend_mode)], t, &g_r->arenas[g_r->arena_index], &g_r->did_we_alloc);
}

// func void draw_circle(s_v2 pos, int layer, float radius, s_v4 color, s_transform t = zero)
// {
// 	t.do_circle = true;
// 	t.pos = pos;
// 	t.layer = layer;
// 	t.draw_size = v2(radius * 2, radius * 2);
// 	t.color = color;
// 	t.uv_min = v2(0, 0);
// 	t.uv_max = v2(1, 1);
// 	t.mix_color = v41f(1);
// 	bucket_add(&g_r->transforms, t, &g_r->arenas[g_r->arena_index], &g_r->did_we_alloc);
// }

// func void draw_circle_p(s_v2 pos, int layer, float radius, s_v4 color, s_camera* camera, s_transform t = zero)
// {
// 	t.do_circle = true;
// 	if(camera)
// 	{
// 		t.pos = world_to_screen(pos, *camera);
// 	}
// 	else
// 	{
// 		t.pos = pos;
// 	}
// 	t.layer = layer;
// 	t.draw_size = v2(radius * 2, radius * 2);
// 	t.color = color;
// 	t.uv_min = v2(0, 0);
// 	t.uv_max = v2(1, 1);
// 	t.mix_color = v41f(1);
// 	bucket_add(&g_r->transforms, t, &g_r->arenas[g_r->arena_index], &g_r->did_we_alloc);
// }

func void draw_texture(s_v2 pos, int layer, s_v2 size, s_v4 color, s_texture texture, s_render_data render_data = zero, s_transform t = zero)
{
	assert(render_data.framebuffer_index >= 0);
	assert(render_data.framebuffer_index < g_r->framebuffers.count);
	s_framebuffer* framebuffer = &g_r->framebuffers[render_data.framebuffer_index];

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
	bucket_add(&framebuffer->transforms[get_render_offset(texture.game_id, render_data.blend_mode)], t, &g_r->arenas[g_r->arena_index], &g_r->did_we_alloc);
}

// func void draw_fbo(u32 texture, s_transform t = zero)
// {
// 	t.layer = e_layer_particles;
// 	t.texture_id = texture;
// 	t.pos = c_half_res;
// 	t.draw_size = c_base_res;
// 	t.color = make_color(1);
// 	t.uv_min = v2(0, 1);
// 	t.uv_max = v2(1, 0);
// 	bucket_add(&g_r->transforms, t, &g_r->arenas[g_r->arena_index], &g_r->did_we_alloc);
// }
