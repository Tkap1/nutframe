
#ifndef m_debug
#include "embed.h"
#endif // m_debug

global constexpr s_shader_paths c_shader_paths[e_shader_count] = {
	{
		.vertex_path = "shaders/vertex.vertex",
		.fragment_path = "shaders/fragment.fragment",
	},
};

global b8 g_do_embed = false;
global s_sarray<const char*, 128> g_to_embed;
global int g_load_texture_index = 0;

func void on_gl_error(const char* expr, int error)
{
	#define m_gl_errors \
	X(GL_INVALID_ENUM, "GL_INVALID_ENUM") \
	X(GL_INVALID_VALUE, "GL_INVALID_VALUE") \
	X(GL_INVALID_OPERATION, "GL_INVALID_OPERATION") \
	X(GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW") \
	X(GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW") \
	X(GL_OUT_OF_MEMORY, "GL_STACK_OUT_OF_MEMORY") \
	X(GL_INVALID_FRAMEBUFFER_OPERATION, "GL_STACK_INVALID_FRAME_BUFFER_OPERATION")

	const char* error_str;
	#define X(a, b) case a: { error_str = b; } break;
	switch(error)
	{
		m_gl_errors
		default: {
			error_str = "unknown error";
		} break;
	}
	#undef X
	#undef m_gl_errors

	printf("GL ERROR: %s - %i (%s)\n", expr, error, error_str);
}

func void init_gl(s_platform_renderer* platform_renderer, s_game_renderer* game_renderer, s_lin_arena* arena)
{
	gl(glGenVertexArrays(1, &platform_renderer->default_vao));
	gl(glBindVertexArray(platform_renderer->default_vao));

	gl(glGenBuffers(1, &platform_renderer->default_vbo));
	gl(glBindBuffer(GL_ARRAY_BUFFER, platform_renderer->default_vbo));

	s_attrib_handler handler = zero;
	add_int_attrib(&handler, 1);
	add_int_attrib(&handler, 1);
	add_int_attrib(&handler, 1);
	add_int_attrib(&handler, 1);
	add_float_attrib(&handler, 1);
	add_float_attrib(&handler, 1);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 4);
	add_float_attrib(&handler, 4);
	finish_attribs(&handler);

	platform_renderer->max_elements = 64;
	gl(glBufferData(GL_ARRAY_BUFFER, sizeof(s_transform) * platform_renderer->max_elements, null, GL_DYNAMIC_DRAW));

	for(int shader_i = 0; shader_i < e_shader_count; shader_i++)
	{
		u32 program = load_shader_from_file(c_shader_paths[shader_i].vertex_path, c_shader_paths[shader_i].fragment_path, arena);
		assert(program);
		platform_renderer->programs[shader_i] = program;
	}

	gl(glUseProgram(platform_renderer->programs[e_shader_default]));

	s_framebuffer framebuffer = zero;
	framebuffer.do_depth = true;
	game_renderer->framebuffers.add(framebuffer);
	after_making_framebuffer(framebuffer.game_id, game_renderer);

}

func void add_int_attrib(s_attrib_handler* handler, int count)
{
	s_attrib attrib = zero;
	attrib.type = GL_INT;
	attrib.size = sizeof(int);
	attrib.count = count;
	handler->attribs.add(attrib);
}

func void add_float_attrib(s_attrib_handler* handler, int count)
{
	s_attrib attrib = zero;
	attrib.type = GL_FLOAT;
	attrib.size = sizeof(float);
	attrib.count = count;
	handler->attribs.add(attrib);
}

func void finish_attribs(s_attrib_handler* handler)
{
	u8* offset = 0;
	int stride = 0;
	foreach_val(attrib_i, attrib, handler->attribs)
	{
		stride += attrib.size * attrib.count;
	}
	assert(stride == sizeof(s_transform));
	foreach_val(attrib_i, attrib, handler->attribs)
	{
		if(attrib.type == GL_FLOAT)
		{
			gl(glVertexAttribPointer(attrib_i, attrib.count, attrib.type, GL_FALSE, stride, offset));
		}
		else
		{
			gl(glVertexAttribIPointer(attrib_i, attrib.count, attrib.type, stride, offset));
		}
		gl(glEnableVertexAttribArray(attrib_i));
		gl(glVertexAttribDivisor(attrib_i, 1));
		offset += attrib.size * attrib.count;
	}
}

func void gl_render(s_platform_renderer* platform_renderer, s_game_renderer* game_renderer)
{
	gl(glUseProgram(platform_renderer->programs[e_shader_default]));

	{
		int location = gl(glGetUniformLocation(platform_renderer->programs[e_shader_default], "window_size"));
		s_v2 window_size = v2(g_window.width, g_window.height);
		gl(glUniform2fv(location, 1, &window_size.x));
	}
	{
		int location = gl(glGetUniformLocation(platform_renderer->programs[e_shader_default], "base_res"));
		gl(glUniform2fv(location, 1, &c_base_res.x));
	}
	{
		int location = gl(glGetUniformLocation(platform_renderer->programs[e_shader_default], "time"));
		gl(glUniform1f(location, (float)game_renderer->total_time));
	}
	{
		int location = gl(glGetUniformLocation(platform_renderer->programs[e_shader_default], "mouse"));
		gl(glUniform2fv(location, 1, &g_platform_data.mouse.x));
	}

	gl(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	gl(glClearDepth(0.0f));
	gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	if(game_renderer->did_we_alloc) {
		foreach_ptr(framebuffer_i, framebuffer, game_renderer->framebuffers) {
			for(int texture_i = 0; texture_i < game_renderer->textures.count; texture_i++) {
				for(int blend_i = 0; blend_i < e_blend_mode_count; blend_i++) {
					int new_index = (game_renderer->arena_index + 1) % 2;
					int offset = get_render_offset(texture_i, blend_i);
					bucket_merge(&framebuffer->transforms[offset], &game_renderer->arenas[new_index]);
				}
			}
		}
	}

	if(game_renderer->did_we_alloc) {
		int old_index = game_renderer->arena_index;
		int new_index = (game_renderer->arena_index + 1) % 2;
		game_renderer->arenas[old_index].used = 0;
		game_renderer->arena_index = new_index;
		game_renderer->did_we_alloc = false;
	}

	foreach_ptr(framebuffer_i, framebuffer, game_renderer->framebuffers) {
		gl(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->gpu_id));
		gl(glViewport(0, 0, g_window.width, g_window.height));
		gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
		if(framebuffer->do_depth) {
			gl(glEnable(GL_DEPTH_TEST));
			gl(glDepthFunc(GL_GREATER));
		}
		else {
			gl(glDisable(GL_DEPTH_TEST));
		}

		gl(glEnable(GL_BLEND));

		gl(glBindVertexArray(platform_renderer->default_vao));
		gl(glBindBuffer(GL_ARRAY_BUFFER, platform_renderer->default_vbo));

		for(int texture_i = 0; texture_i < game_renderer->textures.count; texture_i++) {
			gl(glActiveTexture(GL_TEXTURE0));
			gl(glBindTexture(GL_TEXTURE_2D, game_renderer->textures[texture_i].gpu_id));

			if(game_renderer->textures[texture_i].comes_from_framebuffer) { continue; }
			for(int blend_i = 0; blend_i < e_blend_mode_count; blend_i++) {
				int offset = get_render_offset(texture_i, blend_i);
				int count = framebuffer->transforms[offset].element_count[0];

				if(count > platform_renderer->max_elements) {
					platform_renderer->max_elements = double_until_greater_or_equal(platform_renderer->max_elements, count);
					gl(glBufferData(GL_ARRAY_BUFFER, sizeof(s_transform) * platform_renderer->max_elements, null, GL_DYNAMIC_DRAW));
				}

				if(count <= 0) { continue; }

				if(blend_i == e_blend_mode_normal) {
						gl(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
				}
				else if(blend_i == e_blend_mode_additive) {
					gl(glBlendFunc(GL_ONE, GL_ONE));
				}
				invalid_else;

				// glActiveTexture(GL_TEXTURE1);
				// glBindTexture(GL_TEXTURE_2D, game->noise.id);
				// glUniform1i(1, 1);

				// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				int size = sizeof(*framebuffer->transforms[offset].elements[0]);

				gl(glBufferSubData(GL_ARRAY_BUFFER, 0, size * count, framebuffer->transforms[offset].elements[0]));
				gl(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, count));
				memset(&framebuffer->transforms[offset].element_count, 0, sizeof(framebuffer->transforms[offset].element_count));

				assert(framebuffer->transforms[offset].bucket_count == 1);
			}
		}
	}

	foreach_ptr(framebuffer_i, framebuffer, game_renderer->framebuffers) {
		gl(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->gpu_id));
		if(framebuffer->do_depth) {
			gl(glEnable(GL_DEPTH_TEST));
			gl(glDepthFunc(GL_GREATER));
		}
		else {
			gl(glDisable(GL_DEPTH_TEST));
		}

		gl(glEnable(GL_BLEND));

		gl(glBindVertexArray(platform_renderer->default_vao));
		gl(glBindBuffer(GL_ARRAY_BUFFER, platform_renderer->default_vbo));

		for(int texture_i = 0; texture_i < game_renderer->textures.count; texture_i++) {
			gl(glActiveTexture(GL_TEXTURE0));
			gl(glBindTexture(GL_TEXTURE_2D, game_renderer->textures[texture_i].gpu_id));

			if(!game_renderer->textures[texture_i].comes_from_framebuffer) { continue; }
			for(int blend_i = 0; blend_i < e_blend_mode_count; blend_i++) {
				int offset = get_render_offset(texture_i, blend_i);
				if(framebuffer->transforms[offset].element_count[0] <= 0) { continue; }

				if(blend_i == e_blend_mode_normal) {
						gl(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
				}
				else if(blend_i == e_blend_mode_additive) {
					gl(glBlendFunc(GL_ONE, GL_ONE));
				}
				invalid_else;

				// glActiveTexture(GL_TEXTURE1);
				// glBindTexture(GL_TEXTURE_2D, game->noise.id);
				// glUniform1i(1, 1);

				// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

				int count = framebuffer->transforms[offset].element_count[0];
				int size = sizeof(*framebuffer->transforms[offset].elements[0]);

				gl(glBufferSubData(GL_ARRAY_BUFFER, 0, size * count, framebuffer->transforms[offset].elements[0]));
				gl(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, count));
				memset(&framebuffer->transforms[offset].element_count, 0, sizeof(framebuffer->transforms[offset].element_count));

				assert(framebuffer->transforms[offset].bucket_count == 1);
			}
		}
	}
}

func u32 load_shader_from_file(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena)
{
	char* vertex_src = read_file(vertex_path, frame_arena);
	if(!vertex_src || !vertex_src[0]) { return 0; }
	char* fragment_src = read_file(fragment_path, frame_arena);
	if(!fragment_src || !fragment_src[0]) { return 0; }

	return load_shader_from_str(vertex_src, fragment_src);
}

func u32 load_shader_from_str(const char* vertex_src, const char* fragment_src)
{
	u32 vertex = glCreateShader(GL_VERTEX_SHADER);
	u32 fragment = glCreateShader(GL_FRAGMENT_SHADER);

	#ifdef __EMSCRIPTEN__
	const char* header = "#version 300 es\nprecision highp float;";
	#else
	const char* header = "#version 330 core\n";
	#endif

	const char* vertex_src_arr[] = {header, vertex_src};
	const char* fragment_src_arr[] = {header, fragment_src};
	gl(glShaderSource(vertex, array_count(vertex_src_arr), (const GLchar * const *)vertex_src_arr, null));
	gl(glShaderSource(fragment, array_count(fragment_src_arr), (const GLchar * const *)fragment_src_arr, null));
	gl(glCompileShader(vertex));
	char buffer[1024] = zero;
	check_for_shader_errors(vertex, buffer);
	gl(glCompileShader(fragment));
	check_for_shader_errors(fragment, buffer);
	u32 program = gl(glCreateProgram());
	gl(glAttachShader(program, vertex));
	gl(glAttachShader(program, fragment));
	gl(glLinkProgram(program));

	{
		int length = 0;
		int linked = 0;
		gl(glGetProgramiv(program, GL_LINK_STATUS, &linked));
		if(!linked) {
			gl(glGetProgramInfoLog(program, sizeof(buffer), &length, buffer));
			printf("FAILED TO LINK: %s\n", buffer);
		}
	}

	gl(glDetachShader(program, vertex));
	gl(glDetachShader(program, fragment));
	gl(glDeleteShader(vertex));
	gl(glDeleteShader(fragment));
	return program;
}

func b8 check_for_shader_errors(u32 id, char* out_error)
{
	int compile_success;
	char info_log[1024];
	gl(glGetShaderiv(id, GL_COMPILE_STATUS, &compile_success));

	if(!compile_success) {
		gl(glGetShaderInfoLog(id, 1024, null, info_log));
		log("Failed to compile shader:\n%s", info_log);

		if(out_error)
		{
			strcpy(out_error, info_log);
		}

		return false;
	}
	return true;
}

func s_texture load_texture(s_game_renderer* game_renderer, const char* path)
{
	if(g_do_embed) {
		g_to_embed.add(path);
	}

	#ifndef m_debug

	int width, height, num_channels;
	void* data = stbi_load_from_memory(embed_data[g_load_texture_index], embed_sizes[g_load_texture_index], &width, &height, &num_channels, 4);
	s_texture result = load_texture_from_data(data, width, height, GL_LINEAR);
	g_load_texture_index += 1;

	#else // m_debug

	s_texture result = load_texture_from_file(path, GL_LINEAR);
	#endif

	result.game_id = game_renderer->textures.count;
	result.path = path;
	game_renderer->textures.add(result);
	after_loading_texture(game_renderer);
	return result;
}

func s_texture load_texture_from_data(void* data, int width, int height, u32 filtering)
{
	assert(data);
	u32 id;
	gl(glGenTextures(1, &id));
	gl(glBindTexture(GL_TEXTURE_2D, id));
	gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering));

	s_texture texture = zero;
	texture.gpu_id = id;
	texture.size = v2(width, height);
	return texture;
}

func s_texture load_texture_from_file(const char* path, u32 filtering)
{
	int width, height, num_channels;
	void* data = stbi_load(path, &width, &height, &num_channels, 4);
	assert(data);
	s_texture texture = load_texture_from_data(data, width, height, filtering);
	stbi_image_free(data);
	return texture;
}

func void after_loading_texture(s_game_renderer* game_renderer)
{
	int old_index = game_renderer->transform_arena_index;
	int new_index = (game_renderer->transform_arena_index + 1) % 2;
	int size = sizeof(*game_renderer->framebuffers[0].transforms) * game_renderer->textures.count * e_blend_mode_count;

	foreach_ptr(framebuffer_i, framebuffer, game_renderer->framebuffers) {
		s_bucket_array<s_transform>* new_transforms = (s_bucket_array<s_transform>*)la_get_zero(
			&game_renderer->transform_arenas[new_index], size
		);

		// @Note(tkap, 08/10/2023): The first time we add a texture, transforms is null, so we can't memcpy from it
		if(framebuffer->transforms) {
			memcpy(new_transforms, framebuffer->transforms, size);
		}
		framebuffer->transforms = new_transforms;
	}

	game_renderer->transform_arenas[old_index].used = 0;
	game_renderer->transform_arena_index = new_index;
}

func s_framebuffer* make_framebuffer(s_game_renderer* game_renderer, b8 do_depth)
{
	// @Fixme(tkap, 11/10/2023): handle this
	assert(!do_depth);

	s_framebuffer result = zero;

	gl(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	gl(glGenFramebuffers(1, &result.gpu_id));
	gl(glBindFramebuffer(GL_FRAMEBUFFER, result.gpu_id));

	gl(glGenTextures(1, &result.texture.gpu_id));
	gl(glBindTexture(GL_TEXTURE_2D, result.texture.gpu_id));
	gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_window.width, g_window.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, null));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	gl(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result.texture.gpu_id, 0));

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	gl(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	result.texture.game_id = game_renderer->textures.count;
	result.texture.comes_from_framebuffer = true;
	game_renderer->textures.add(result.texture);

	result.game_id = game_renderer->framebuffers.count;
	result.do_depth = do_depth;
	int index = game_renderer->framebuffers.add(result);
	after_making_framebuffer(result.game_id, game_renderer);

	return &game_renderer->framebuffers[index];
}

func void after_making_framebuffer(int index, s_game_renderer* game_renderer)
{
	int size = sizeof(*game_renderer->framebuffers[0].transforms) * game_renderer->textures.count * e_blend_mode_count;
	s_bucket_array<s_transform>* new_transforms = (s_bucket_array<s_transform>*)la_get_zero(
		&game_renderer->transform_arenas[game_renderer->transform_arena_index], size
	);
	s_framebuffer* framebuffer = &game_renderer->framebuffers[index];
	framebuffer->transforms = new_transforms;

	after_loading_texture(game_renderer);

}

func void write_embed_file()
{
	assert(g_do_embed);
	s_str_builder* builder = (s_str_builder*)malloc(sizeof(s_str_builder));
	builder->tab_count = 0;
	builder->len = 0;
	foreach_val(embed_i, embed, g_to_embed) {
		FILE* file = fopen(embed, "rb");
		assert(file);
		fseek(file, 0, SEEK_END);
		u64 file_size = ftell(file);
		fseek(file, 0, SEEK_SET);
		u8* data = (u8*)malloc(file_size);
		fread(data, 1, file_size, file);
		u8* cursor = data;

		builder_add_line(builder, "global constexpr u8 embed%i[%u] = {", embed_i, file_size);
		for(u64 i = 0; i < file_size; i++) {
			builder_add(builder, "%u,", *cursor);
			cursor++;
		}
		builder_add_line(builder, "\n};");

		fclose(file);
		free(data);
	}

	builder_add_line(builder, "global constexpr u8* embed_data[%i] = {", g_to_embed.count);
	foreach_val(embed_i, embed, g_to_embed) {
		builder_add(builder, "(u8*)embed%i,", embed_i);
	}
	builder_add_line(builder, "\n};");

	builder_add_line(builder, "global constexpr int embed_sizes[%i] = {", g_to_embed.count);
	foreach_val(embed_i, embed, g_to_embed) {
		builder_add(builder, "array_count(embed%i),", embed_i);
	}
	builder_add_line(builder, "\n};");

	{
		FILE* file = fopen("src/embed.h", "wb");
		fwrite(builder->data, 1, builder->len, file);
		fclose(file);
	}

	exit(0);
}

func s_font* load_font(s_game_renderer* game_renderer, const char* path, int font_size, s_lin_arena* arena)
{
	s_font font = zero;
	font.size = (float)font_size;

	u8* file_data = (u8*)read_file(path, arena);
	assert(file_data);

	stbtt_fontinfo info = zero;
	stbtt_InitFont(&info, file_data, 0);

	stbtt_GetFontVMetrics(&info, &font.ascent, &font.descent, &font.line_gap);

	font.scale = stbtt_ScaleForPixelHeight(&info, (float)font_size);
	constexpr int max_chars = 128;
	int bitmap_count = 0;
	u8* bitmap_arr[max_chars];
	const int padding = 10;

	int columns = floorfi((float)(4096 / (font_size + padding)));
	int rows = ceilfi((max_chars - columns) / (float)columns) + 1;

	int total_width = floorfi((float)(columns * (font_size + padding)));
	int total_height = floorfi((float)(rows * (font_size + padding)));

	for(int char_i = 0; char_i < max_chars; char_i++)
	{
		s_glyph glyph = zero;
		u8* bitmap = stbtt_GetCodepointBitmap(&info, 0, font.scale, char_i, &glyph.width, &glyph.height, 0, 0);
		stbtt_GetCodepointBox(&info, char_i, &glyph.x0, &glyph.y0, &glyph.x1, &glyph.y1);
		stbtt_GetGlyphHMetrics(&info, char_i, &glyph.advance_width, null);

		font.glyph_arr[char_i] = glyph;
		bitmap_arr[bitmap_count++] = bitmap;
	}

	u8* gl_bitmap = (u8*)la_get_zero(arena, sizeof(u8) * 4 * total_width * total_height);

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
				int current_x = floorfi((float)(column * (font_size + padding)));
				int current_y = floorfi((float)(row * (font_size + padding)));
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

	for(int bitmap_i = 0; bitmap_i < bitmap_count; bitmap_i++)
	{
		stbtt_FreeBitmap(bitmap_arr[bitmap_i], null);
	}

	font.texture = load_texture_from_data(gl_bitmap, total_width, total_height, GL_LINEAR);
	font.texture.game_id = game_renderer->textures.count;
	game_renderer->textures.add(font.texture);
	after_loading_texture(game_renderer);

	int index = game_renderer->fonts.add(font);
	return &game_renderer->fonts[index];
}