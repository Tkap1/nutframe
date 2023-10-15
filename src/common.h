

#ifdef m_debug
#define gl(...) __VA_ARGS__; {int error = glGetError(); if(error != 0) { on_gl_error(#__VA_ARGS__, error); }}
#else // m_debug
#define gl(...) __VA_ARGS__
#endif // m_debug


enum e_shader
{
	e_shader_default,
	e_shader_count
};

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
	const char* vertex_path;
	const char* fragment_path;
};

struct s_platform_renderer
{
	int max_elements;
	u32 default_vao;
	u32 default_vbo;
	u32 programs[e_shader_count];
};

#pragma pack(push, 1)
struct s_riff_chunk
{
	u32 chunk_id;
	u32 chunk_size;
	u32 format;
};

struct s_fmt_chunk
{
	u32 sub_chunk1_id;
	u32 sub_chunk1_size;
	u16 audio_format;
	u16 num_channels;
	u32 sample_rate;
	u32 byte_rate;
	u16 block_align;
	u16 bits_per_sample;
};

struct s_data_chunk
{
	u32 sub_chunk2_id;
	u32 sub_chunk2_size;
};
#pragma pack(pop)


func void add_int_attrib(s_attrib_handler* handler, int count);
func void add_float_attrib(s_attrib_handler* handler, int count);
func void finish_attribs(s_attrib_handler* handler);
func u32 load_shader_from_str(const char* vertex_src, const char* fragment_src);
func u32 load_shader_from_file(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena);
func void after_making_framebuffer(int index, s_game_renderer* game_renderer);