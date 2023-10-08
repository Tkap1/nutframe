
func void init_opengl(s_platform_renderer* platform_renderer, s_lin_arena* arena)
{
	glGenVertexArrays(1, &platform_renderer->default_vao);
	glBindVertexArray(platform_renderer->default_vao);

	glGenBuffers(1, &platform_renderer->default_ssbo);
	glBindBuffer(GL_SHADER_STORAGE_BUFFER, platform_renderer->default_ssbo);
	glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, platform_renderer->default_ssbo);

	// @Fixme(tkap, 07/10/2023): proper size. we have to basically set this to the maximum of things that we have ever drawn
	glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(s_transform) * 1024, null, GL_DYNAMIC_DRAW);

	glDebugMessageCallback(gl_debug_callback, null);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	for(int shader_i = 0; shader_i < e_shader_count; shader_i++)
	{
		platform_renderer->programs[shader_i] = load_shader(shader_paths[shader_i].vertex_path, shader_paths[shader_i].fragment_path, arena);
	}

	glUseProgram(platform_renderer->programs[e_shader_default]);
}
