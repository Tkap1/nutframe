#pragma comment(lib, "user32.lib")
#pragma comment(lib, "gdi32.lib")
#pragma comment(lib, "opengl32.lib")
#pragma comment(lib, "Ole32.lib")

#include "pch_platform.h"

#pragma warning(push, 0)
#define STB_IMAGE_IMPLEMENTATION
#define STBI_ASSERT assert
#include "external/stb_image.h"
#pragma warning(pop)

#include "resource.h"
#include "memory.h"
#include "config.h"
#include "bucket.h"
#include "shader_shared.h"
#include "platform_shared.h"
#include "sdl_platform.h"

global s_window g_window;
global s_input g_input;
global s_input g_logic_input;
global u64 g_cycle_frequency;
global u64 g_start_cycles;
global s_platform_data g_platform_data = zero;
global SDL_GLContext gContext;
global SDL_Window* gWindow = null;
global GLuint gProgramID = 0;
global GLint gVertexPos2DLocation = -1;
global GLuint gVBO = 0;
global GLuint gIBO = 0;

global s_shader_paths shader_paths[e_shader_count] = {
	{
		.vertex_path = "shaders/vertex.vertex",
		.fragment_path = "shaders/fragment.fragment",
	},
};



#include "memory.cpp"
#include "platform_shared.cpp"
#include "file.cpp"
#include "bucket.cpp"
#include "common.cpp"

int main(int argc, char** argv)
{

	if(SDL_Init(SDL_INIT_VIDEO) < 0)
	{
		printf("SDL could not initialize! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MAJOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_MINOR_VERSION, 3 );
	SDL_GL_SetAttribute( SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE );

	gWindow = SDL_CreateWindow(
		"SDL Tutorial", SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED,
		c_resolutions[c_base_resolution_index].x, c_resolutions[c_base_resolution_index].y, SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
	);
	if(gWindow == NULL)
	{
		printf("Window could not be created! SDL_Error: %s\n", SDL_GetError());
		return 1;
	}

	gContext = SDL_GL_CreateContext( gWindow );
	if( gContext == NULL )
	{
		printf( "OpenGL context could not be created! SDL Error: %s\n", SDL_GetError() );
		return 1;
	}

	glewExperimental = GL_TRUE;
	GLenum glewError = glewInit();
	if( glewError != GLEW_OK )
	{
		printf( "Error initializing GLEW! %s\n", glewGetErrorString( glewError ) );
		return 1;
	}

	s_platform_renderer platform_renderer = zero;
	s_game_renderer* game_renderer = null;
	s_lin_arena platform_frame_arena = zero;
	s_lin_arena game_frame_arena = zero;

	#ifdef m_debug
	unreferenced(argc);
	unreferenced(argv);
	#endif

	s_platform_funcs platform_funcs = zero;
	// platform_funcs.play_sound = play_sound;
	// platform_funcs.show_cursor = ShowCursor;
	// platform_funcs.cycle_between_available_resolutions = cycle_between_available_resolutions;

	void* game_memory = null;

	{
		s_lin_arena all = zero;
		all.capacity = 100 * c_mb;

		// @Note(tkap, 26/06/2023): We expect this memory to be zero'd
		all.memory = malloc(all.capacity);
		memset(all.memory, 0, all.capacity);

		game_renderer = (s_game_renderer*)la_get(&all, sizeof(s_game_renderer));

		game_memory = la_get(&all, c_game_memory);
		platform_frame_arena = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		game_frame_arena = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		g_platform_data.frame_arena = &game_frame_arena;

		game_renderer->arenas[0] = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		game_renderer->arenas[1] = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		game_renderer->transform_arenas[0] = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		game_renderer->transform_arenas[1] = make_lin_arena_from_memory(5 * c_mb, la_get(&all, 5 * c_mb));
		game_renderer->textures.add(zero);
		after_loading_texture(game_renderer);
	}

	game_renderer->set_vsync = set_vsync;
	game_renderer->load_texture = load_texture;
	init_opengl(&platform_renderer, &platform_frame_arena);

	b8 running = true;
	f64 time_passed = 0;

	#ifndef m_debug
	g_platform_data.recompiled = true;
	#endif // m_debug

	while(running)
	{
		f64 start_of_frame_ms = SDL_GetTicks();

		SDL_Event e;
		while(SDL_PollEvent(&e) != 0)
		{
			if(e.type == SDL_QUIT)
			{
				running = false;
			}
		}

		g_platform_data.input = &g_input;
		g_platform_data.logic_input = &g_logic_input;
		g_platform_data.quit_after_this_frame = !running;
		g_platform_data.window_width = g_window.width;
		g_platform_data.window_height = g_window.height;
		g_platform_data.time_passed = time_passed;

		{
			int x, y;
			SDL_GetMouseState(&x, &y);
			g_platform_data.mouse.x = (float)x;
			g_platform_data.mouse.y = (float)y;
		}
		// g_platform_data.is_window_active = GetActiveWindow() == g_window.handle;

		update_game(&g_platform_data, platform_funcs, game_memory, game_renderer);
		g_platform_data.recompiled = false;

		// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		render start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
		{
			int location = glGetUniformLocation(platform_renderer.programs[e_shader_default], "window_size");
			s_v2 window_size = v2(g_window.width, g_window.height);
			glUniform2fv(location, 1, &window_size.x);
		}
		{
			int location = glGetUniformLocation(platform_renderer.programs[e_shader_default], "base_res");
			glUniform2fv(location, 1, &c_base_res.x);
		}
		{
			static float time = 0;
			time += time_passed;
			int location = glGetUniformLocation(platform_renderer.programs[e_shader_default], "time");
			glUniform1f(location, time);
		}
		{
			int location = glGetUniformLocation(platform_renderer.programs[e_shader_default], "mouse");
			glUniform2fv(location, 1, &g_platform_data.mouse.x);
		}

		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		glClearDepth(0.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

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
				glBindFramebuffer(GL_FRAMEBUFFER, 0);
				glBindVertexArray(platform_renderer.default_vao);
				glActiveTexture(GL_TEXTURE0);
				glBindTexture(GL_TEXTURE_2D, game_renderer->textures[texture_i].gpu_id);

				// glActiveTexture(GL_TEXTURE1);
				// glBindTexture(GL_TEXTURE_2D, game->noise.id);
				// glUniform1i(1, 1);

				glEnable(GL_DEPTH_TEST);
				glDepthFunc(GL_GREATER);
				glEnable(GL_BLEND);
				// if(render_type == 0)
				{
					// glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA);
					// glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
				}
				// else if(render_type == 1)
				// {
					glBlendFunc(GL_ONE, GL_ONE);
				// }
				// invalid_else;

				int count = game_renderer->transforms[texture_i].element_count[0];
				int size = sizeof(*game_renderer->transforms[texture_i].elements[0]);

				glBufferSubData(GL_SHADER_STORAGE_BUFFER, 0, size * count, game_renderer->transforms[texture_i].elements[0]);
				glDrawArraysInstanced(GL_TRIANGLES, 0, 6, count);
				memset(&game_renderer->transforms[texture_i].element_count, 0, sizeof(game_renderer->transforms[texture_i].element_count));

				assert(game_renderer->transforms[texture_i].bucket_count == 1);
			}
		}

		// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		render end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

		SDL_GL_SwapWindow(gWindow);

		time_passed = (SDL_GetTicks() - start_of_frame_ms) / 1000;
	}

	return 0;

}



void gl_debug_callback(GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, const GLchar* message, const void* userParam)
{
	unreferenced(userParam);
	unreferenced(length);
	unreferenced(id);
	unreferenced(type);
	unreferenced(source);
	if(severity >= GL_DEBUG_SEVERITY_HIGH)
	{
		printf("GL ERROR: %s\n", message);
		assert(false);
	}
}

func u32 load_shader(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena)
{
	u32 vertex = glCreateShader(GL_VERTEX_SHADER);
	u32 fragment = glCreateShader(GL_FRAGMENT_SHADER);
	const char* header = "#version 330 core\n";
	char* vertex_src = read_file(vertex_path, frame_arena);
	if(!vertex_src || !vertex_src[0]) { return 0; }
	char* fragment_src = read_file(fragment_path, frame_arena);
	if(!fragment_src || !fragment_src[0]) { return 0; }

	#ifdef m_debug
	const char* vertex_src_arr[] = {header, read_file("src/shader_shared.h", frame_arena), vertex_src};
	const char* fragment_src_arr[] = {header, read_file("src/shader_shared.h", frame_arena), fragment_src};
	#else // m_debug
	const char* vertex_src_arr[] = {header, vertex_src};
	const char* fragment_src_arr[] = {header, fragment_src};
	#endif // m_debug
	glShaderSource(vertex, array_count(vertex_src_arr), (const GLchar * const *)vertex_src_arr, null);
	glShaderSource(fragment, array_count(fragment_src_arr), (const GLchar * const *)fragment_src_arr, null);
	glCompileShader(vertex);
	char buffer[1024] = zero;
	check_for_shader_errors(vertex, buffer);
	glCompileShader(fragment);
	check_for_shader_errors(fragment, buffer);
	u32 program = glCreateProgram();
	glAttachShader(program, vertex);
	glAttachShader(program, fragment);
	glLinkProgram(program);
	glDeleteShader(vertex);
	glDeleteShader(fragment);
	return program;
}

func b8 check_for_shader_errors(u32 id, char* out_error)
{
	int compile_success;
	char info_log[1024];
	glGetShaderiv(id, GL_COMPILE_STATUS, &compile_success);

	if(!compile_success)
	{
		glGetShaderInfoLog(id, 1024, null, info_log);
		log("Failed to compile shader:\n%s", info_log);

		if(out_error)
		{
			strcpy(out_error, info_log);
		}

		return false;
	}
	return true;
}

func s_texture load_texture(s_game_renderer* game_renderer, char* path)
{
	s_texture result = load_texture_from_file(path, GL_LINEAR);
	result.game_id = game_renderer->textures.count;
	game_renderer->textures.add(result);
	after_loading_texture(game_renderer);
	return result;
}

func s_texture load_texture_from_data(void* data, int width, int height, u32 filtering)
{
	assert(data);
	u32 id;
	glGenTextures(1, &id);
	glBindTexture(GL_TEXTURE_2D, id);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, data);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering);

	s_texture texture = zero;
	texture.gpu_id = id;
	texture.size = v22i(width, height);
	return texture;
}

func s_texture load_texture_from_file(char* path, u32 filtering)
{
	int width, height, num_channels;
	void* data = stbi_load(path, &width, &height, &num_channels, 4);
	s_texture texture = load_texture_from_data(data, width, height, filtering);
	stbi_image_free(data);
	return texture;
}

func void after_loading_texture(s_game_renderer* game_renderer)
{
	int old_index = game_renderer->transform_arena_index;
	int new_index = (game_renderer->transform_arena_index + 1) % 2;
	int size = sizeof(*game_renderer->transforms) * game_renderer->textures.count;
	s_bucket_array<s_transform>* new_transforms = (s_bucket_array<s_transform>*)la_get_zero(
		&game_renderer->transform_arenas[new_index], size
	);

	// @Note(tkap, 08/10/2023): The first time we add a texture, transforms is null, so we can't memcpy from it
	if(game_renderer->transforms)
	{
		memcpy(new_transforms, game_renderer->transforms, size);
	}
	game_renderer->transforms = new_transforms;
	game_renderer->transform_arenas[old_index].used = 0;
	game_renderer->transform_arena_index = new_index;
}

func void render()
{

}

func void printShaderLog( GLuint shader )
{
	//Make sure name is shader
	if( glIsShader( shader ) )
	{
		//Shader log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetShaderiv( shader, GL_INFO_LOG_LENGTH, &maxLength );

		//Allocate string
		char* infoLog = new char[ maxLength ];

		//Get info log
		glGetShaderInfoLog( shader, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
		{
			//Print Log
			printf( "%s\n", infoLog );
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf( "Name %d is not a shader\n", shader );
	}
}

void printProgramLog( GLuint program )
{
	//Make sure name is shader
	if( glIsProgram( program ) )
	{
		//Program log length
		int infoLogLength = 0;
		int maxLength = infoLogLength;

		//Get info string length
		glGetProgramiv( program, GL_INFO_LOG_LENGTH, &maxLength );

		//Allocate string
		char* infoLog = new char[ maxLength ];

		//Get info log
		glGetProgramInfoLog( program, maxLength, &infoLogLength, infoLog );
		if( infoLogLength > 0 )
		{
			//Print Log
			printf( "%s\n", infoLog );
		}

		//Deallocate string
		delete[] infoLog;
	}
	else
	{
		printf( "Name %d is not a program\n", program );
	}
}

func void set_vsync(b8 val)
{
	SDL_GL_SetSwapInterval(val ? 1 : 0);
}
