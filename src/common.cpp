
#define gl(...) __VA_ARGS__; {int error = glGetError(); if(error != 0) printf("%s error: %i\n", #__VA_ARGS__, error);}

func void init_opengl(s_platform_renderer* platform_renderer, s_lin_arena* arena)
{
	gl(glGenVertexArrays(1, &platform_renderer->default_vao));
	gl(glBindVertexArray(platform_renderer->default_vao));

	gl(glGenBuffers(1, &platform_renderer->default_vbo));
	gl(glBindBuffer(GL_ARRAY_BUFFER, platform_renderer->default_vbo));

	s_foo foo = zero;
	add_int(&foo, 1);
	add_int(&foo, 1);
	add_int(&foo, 1);
	add_int(&foo, 1);
	add_float(&foo, 1);
	add_float(&foo, 2);
	add_float(&foo, 2);
	add_float(&foo, 2);
	add_float(&foo, 2);
	add_float(&foo, 2);
	add_float(&foo, 2);
	add_float(&foo, 4);
	add_float(&foo, 4);
	finish(&foo);

	// @Fixme(tkap, 07/10/2023): proper size. we have to basically set this to the maximum of things that we have ever drawn
	gl(glBufferData(GL_ARRAY_BUFFER, sizeof(s_transform) * 1024, null, GL_DYNAMIC_DRAW));

	// @Fixme(tkap, 08/10/2023):
	// glDebugMessageCallback(gl_debug_callback, null);
	// glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	for(int shader_i = 0; shader_i < e_shader_count; shader_i++)
	{
		platform_renderer->programs[shader_i] = load_shader(shader_paths[shader_i].vertex_path, shader_paths[shader_i].fragment_path, arena);
	}

	gl(glUseProgram(platform_renderer->programs[e_shader_default]));
}

func void add_int(s_foo* foo, int count)
{
	s_attrib attrib = zero;
	attrib.type = GL_INT;
	attrib.size = sizeof(int);
	attrib.count = count;
	foo->attribs.add(attrib);
}

func void add_float(s_foo* foo, int count)
{
	s_attrib attrib = zero;
	attrib.type = GL_FLOAT;
	attrib.size = sizeof(float);
	attrib.count = count;
	foo->attribs.add(attrib);
}

func void finish(s_foo* foo)
{
	u8* offset = 0;
	int stride = 0;
	foreach_raw(attrib_i, attrib, foo->attribs)
	{
		stride += attrib.size * attrib.count;
	}
	assert(stride == sizeof(s_transform));
	foreach_raw(attrib_i, attrib, foo->attribs)
	{
		if(attrib.type == GL_FLOAT)
		{
			glVertexAttribPointer(attrib_i, attrib.count, attrib.type, GL_FALSE, stride, offset);
		}
		else
		{
			glVertexAttribIPointer(attrib_i, attrib.count, attrib.type, stride, offset);
		}
		glEnableVertexAttribArray(attrib_i);
		glVertexAttribDivisor(attrib_i, 1);
		offset += attrib.size * attrib.count;
	}

}