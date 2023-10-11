

#ifdef m_debug
#define gl(...) __VA_ARGS__; {int error = glGetError(); if(error != 0) { on_gl_error(#__VA_ARGS__, error); }}
#else // m_debug
#define gl(...) __VA_ARGS__
#endif // m_debug

struct s_attrib
{
	int type;
	int size;
	int count;
};

struct s_attrib_handler
{
	s_sarray<s_attrib, 32> attribs;
};

struct s_shader_paths
{
	char* vertex_path;
	char* fragment_path;
};

func void add_int(s_attrib_handler* handler, int count);
func void add_float(s_attrib_handler* handler, int count);
func void finish(s_attrib_handler* handler);
func u32 load_shader_from_str(const char* vertex_src, const char* fragment_src);
func u32 load_shader_from_file(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena);