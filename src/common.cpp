
#ifdef m_debug
#define gl(...) __VA_ARGS__; {int error = glGetError(); if(error != 0) { on_gl_error(#__VA_ARGS__, error); }}
#else // m_debug
#define gl(...) __VA_ARGS__
#endif // m_debug

func void on_gl_error(char* expr, int error)
{
	#define m_gl_errors \
	X(GL_INVALID_ENUM, "GL_INVALID_ENUM") \
	X(GL_INVALID_VALUE, "GL_INVALID_VALUE") \
	X(GL_INVALID_OPERATION, "GL_INVALID_OPERATION") \
	X(GL_STACK_OVERFLOW, "GL_STACK_OVERFLOW") \
	X(GL_STACK_UNDERFLOW, "GL_STACK_UNDERFLOW") \
	X(GL_OUT_OF_MEMORY, "GL_STACK_OUT_OF_MEMORY") \
	X(GL_INVALID_FRAMEBUFFER_OPERATION, "GL_STACK_INVALID_FRAME_BUFFER_OPERATION")

	char* error_str;
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

func void init_gl(s_platform_renderer* platform_renderer, s_lin_arena* arena)
{
	gl(glGenVertexArrays(1, &platform_renderer->default_vao));
	gl(glBindVertexArray(platform_renderer->default_vao));

	gl(glGenBuffers(1, &platform_renderer->default_vbo));
	gl(glBindBuffer(GL_ARRAY_BUFFER, platform_renderer->default_vbo));

	s_attrib_handler handler = zero;
	add_int(&handler, 1);
	add_int(&handler, 1);
	add_int(&handler, 1);
	add_int(&handler, 1);
	add_float(&handler, 1);
	add_float(&handler, 2);
	add_float(&handler, 2);
	add_float(&handler, 2);
	add_float(&handler, 2);
	add_float(&handler, 2);
	add_float(&handler, 2);
	add_float(&handler, 4);
	add_float(&handler, 4);
	finish(&handler);

	// @Fixme(tkap, 07/10/2023): proper size. we have to basically set this to the maximum of things that we have ever drawn
	gl(glBufferData(GL_ARRAY_BUFFER, sizeof(s_transform) * 1024, null, GL_DYNAMIC_DRAW));

	for(int shader_i = 0; shader_i < e_shader_count; shader_i++)
	{
		platform_renderer->programs[shader_i] = load_shader(shader_paths[shader_i].vertex_path, shader_paths[shader_i].fragment_path, arena);
	}

	gl(glUseProgram(platform_renderer->programs[e_shader_default]));
}

func void add_int(s_attrib_handler* handler, int count)
{
	s_attrib attrib = zero;
	attrib.type = GL_INT;
	attrib.size = sizeof(int);
	attrib.count = count;
	handler->attribs.add(attrib);
}

func void add_float(s_attrib_handler* handler, int count)
{
	s_attrib attrib = zero;
	attrib.type = GL_FLOAT;
	attrib.size = sizeof(float);
	attrib.count = count;
	handler->attribs.add(attrib);
}

func void finish(s_attrib_handler* handler)
{
	u8* offset = 0;
	int stride = 0;
	foreach_raw(attrib_i, attrib, handler->attribs)
	{
		stride += attrib.size * attrib.count;
	}
	assert(stride == sizeof(s_transform));
	foreach_raw(attrib_i, attrib, handler->attribs)
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
		gl(glUniform1f(location, game_renderer->total_time));
	}
	{
		int location = gl(glGetUniformLocation(platform_renderer->programs[e_shader_default], "mouse"));
		gl(glUniform2fv(location, 1, &g_platform_data.mouse.x));
	}

	gl(glClearColor(0.0f, 0.0f, 0.0f, 0.0f));
	gl(glClearDepth(0.0f));
	gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));

	if(game_renderer->did_we_alloc)
	{
		for(int texture_i = 0; texture_i < game_renderer->textures.count; texture_i++)
		{
			int new_index = (game_renderer->arena_index + 1) % 2;
			bucket_merge(&game_renderer->transforms[texture_i], &game_renderer->arenas[new_index]);
		}
	}
	if(game_renderer->did_we_alloc)
	{
		int old_index = game_renderer->arena_index;
		int new_index = (game_renderer->arena_index + 1) % 2;
		game_renderer->arenas[old_index].used = 0;
		game_renderer->arena_index = new_index;
		game_renderer->did_we_alloc = false;
	}

	for(int texture_i = 0; texture_i < game_renderer->textures.count; texture_i++)
	{
		if(game_renderer->transforms[texture_i].element_count[0] > 0)
		{
			gl(glBindFramebuffer(GL_FRAMEBUFFER, 0));
			gl(glBindVertexArray(platform_renderer->default_vao));
			gl(glBindBuffer(GL_ARRAY_BUFFER, platform_renderer->default_vbo));

			gl(glActiveTexture(GL_TEXTURE0));
			gl(glBindTexture(GL_TEXTURE_2D, game_renderer->textures[texture_i].gpu_id));

			// glActiveTexture(GL_TEXTURE1);
			// glBindTexture(GL_TEXTURE_2D, game->noise.id);
			// glUniform1i(1, 1);

			gl(glEnable(GL_DEPTH_TEST));
			gl(glDepthFunc(GL_GREATER));
			gl(glEnable(GL_BLEND));
			// if(render_type == 0)
			{
				// glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
				// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
			}
			// else if(render_type == 1)
			// {
				gl(glBlendFunc(GL_ONE, GL_ONE));
			// }
			// invalid_else;

			int count = game_renderer->transforms[texture_i].element_count[0];
			int size = sizeof(*game_renderer->transforms[texture_i].elements[0]);

			gl(glBufferSubData(GL_ARRAY_BUFFER, 0, size * count, game_renderer->transforms[texture_i].elements[0]));
			gl(glDrawArraysInstanced(GL_TRIANGLES, 0, 6, count));
			memset(&game_renderer->transforms[texture_i].element_count, 0, sizeof(game_renderer->transforms[texture_i].element_count));

			assert(game_renderer->transforms[texture_i].bucket_count == 1);
		}
	}
}