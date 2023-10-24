
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <stdarg.h>
#include <math.h>

#ifdef _WIN32
#ifdef m_build_dll
#define m_dll_export __declspec(dllexport)
#else // m_build_dll
#define m_dll_export
#endif // m_build_dll
#endif // _WIN32

#ifdef __GNUC__
// @TODO(tkap, 13/10/2023): stackoverflow copy paste
#ifdef m_build_dll
#define m_dll_export __attribute__((visibility("default")))
#else // m_build_dll
#define m_dll_export
#endif // m_build_dll
#endif // __linux__

#ifdef __linux__
// @TODO(tkap, 13/10/2023): I don't know. Nothing??
#define m_dll_export
#endif // __linux__

typedef int64_t s64;
typedef int32_t s32;
typedef int16_t s16;
typedef int8_t s8;

typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef uint8_t b8;
typedef uint32_t b32;

typedef double f64;

#ifdef __EMSCRIPTEN__
#define m_gl_single_channel GL_LUMINANCE
#else // __EMSCRIPTEN__
#define m_gl_single_channel GL_RED
#endif // __EMSCRIPTEN__

#define assert(cond) do { if(!(cond)) { on_failed_assert(#cond, __FILE__, __LINE__); } } while(0)
#define unreferenced(thing) (void)thing;
#define check(cond) do { if(!(cond)) { error(false); }} while(0)
#define invalid_default_case default: { assert(false); }
#define invalid_else else { assert(false); }
#define error(b) do { if(!(b)) { printf("ERROR\n"); exit(1); }} while(0)
#define array_count(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define log(...) printf(__VA_ARGS__); printf("\n")
#define log_info(...) printf(__VA_ARGS__); printf("\n")
#define log_error(...) printf(__VA_ARGS__); printf("\n")

#define breakable_block__(a, b) for(int a##b = 1; a##b--;)
#define breakable_block_(a) breakable_block__(tkinternal_condblock, a)
#define breakable_block breakable_block_(__LINE__)

#define foreach_ptr__(a, index_name, element_name, array) if(0) finished##a: ; else for(auto element_name = &(array).elements[0];;) if(1) goto body##a; else while(1) if(1) goto finished##a; else body##a: for(int index_name = 0; index_name < (array).count && (bool)(element_name = &(array)[index_name]); index_name++)
#define foreach_ptr_(a, index_name, element_name, array) foreach_ptr__(a, index_name, element_name, array)
#define foreach_ptr(index_name, element_name, array) foreach_ptr_(__LINE__, index_name, element_name, array)

#define foreach_val__(a, index_name, element_name, array) if(0) finished##a: ; else for(auto element_name = (array).elements[0];;) if(1) goto body##a; else while(1) if(1) goto finished##a; else body##a: for(int index_name = 0; index_name < (array).count && (void*)&(element_name = (array)[index_name]); index_name++)
#define foreach_val_(a, index_name, element_name, array) foreach_val__(a, index_name, element_name, array)
#define foreach_val(index_name, element_name, array) foreach_val_(__LINE__, index_name, element_name, array)

static constexpr u64 c_max_u64 = UINT64_MAX;
static constexpr float c_max_f32 = 999999999.0f;

static constexpr s64 c_kb = 1024;
static constexpr s64 c_mb = 1024 * c_kb;
static constexpr s64 c_gb = 1024 * c_mb;
static constexpr s64 c_tb = 1024 * c_gb;

static constexpr float pi = 3.1415926f;
static constexpr float tau = 6.283185f;
static constexpr float epsilon = 0.000001f;

static void on_failed_assert(const char* cond, const char* file, int line);

template<typename t0, typename t1>
struct is_same_ { static constexpr b8 is_it = false; };
template<typename t>
struct is_same_<t, t> { static constexpr b8 is_it = true; };
template <typename t0, typename t1>
constexpr b8 is_same = is_same_<t0, t1>::is_it;

struct s_game_renderer;

struct s_v2
{
	float x;
	float y;
};

struct s_v2i
{
	int x;
	int y;
};

struct s_v3
{
	float x;
	float y;
	float z;
};

struct s_v4
{
	float x;
	float y;
	float z;
	float w;
};


#ifndef m_game
#ifndef m_debug
#include "embed.h"
#endif // m_debug
#endif // m_game


template <typename T, int N>
struct s_sarray
{
	static_assert(N > 0);
	int count = 0;
	T elements[N];

	constexpr T& operator[](int index)
	{
		assert(index >= 0);
		assert(index < count);
		return elements[index];
	}

	constexpr T get(int index)
	{
		return (*this)[index];
	}

	T pop()
	{
		assert(count > 0);
		return elements[--count];
	}

	constexpr void remove_and_swap(int index)
	{
		assert(index >= 0);
		assert(index < count);
		count -= 1;
		elements[index] = elements[count];
	}

	constexpr T remove_and_shift(int index)
	{
		assert(index >= 0);
		assert(index < count);
		T result = elements[index];
		count -= 1;

		int to_move = count - index;
		if(to_move > 0)
		{
			// @Note(tkap, 13/10/2023): memcpy is good enough here, but the sanitizer complains.
			memmove(elements + index, elements + index + 1, to_move * sizeof(T));
		}
		return result;
	}

	constexpr T* get_ptr(int index)
	{
		return &(*this)[index];
	}

	constexpr void swap(int index0, int index1)
	{
		assert(index0 >= 0);
		assert(index1 >= 0);
		assert(index0 < count);
		assert(index1 < count);
		assert(index0 != index1);
		T temp = elements[index0];
		elements[index0] = elements[index1];
		elements[index1] = temp;
	}

	constexpr T get_last()
	{
		assert(count > 0);
		return elements[count - 1];
	}

	constexpr T* get_last_ptr()
	{
		assert(count > 0);
		return &elements[count - 1];
	}

	constexpr int add(T element)
	{
		assert(count < N);
		elements[count] = element;
		count += 1;
		return count - 1;
	}

	constexpr b8 add_checked(T element)
	{
		if(count < N)
		{
			add(element);
			return true;
		}
		return false;
	}

	constexpr b8 contains(T what)
	{
		for(int element_i = 0; element_i < count; element_i++)
		{
			if(what == elements[element_i])
			{
				return true;
			}
		}
		return false;
	}

	constexpr void insert(int index, T element)
	{
		assert(index >= 0);
		assert(index < N);
		assert(index <= count);

		int to_move = count - index;
		count += 1;
		if(to_move > 0)
		{
			memmove(&elements[index + 1], &elements[index], to_move * sizeof(T));
		}
		elements[index] = element;
	}

	constexpr int max_elements()
	{
		return N;
	}

	constexpr b8 is_last(int index)
	{
		assert(index >= 0);
		assert(index < count);
		return index == count - 1;
	}

	constexpr b8 is_full()
	{
		return count >= N;
	}

	b8 is_empty()
	{
		return count <= 0;
	}

	void small_sort()
	{
		// @Note(tkap, 25/06/2023): Let's not get crazy with insertion sort, bro
		assert(count < 256);

		for(int i = 1; i < count; i++)
		{
			for(int j = i; j > 0; j--)
			{
				T* a = &elements[j];
				T* b = &elements[j - 1];

				if(*a > *b) {
					break;
				}
				T temp = *a;
				*a = *b;
				*b = temp;
			}
		}
	}
};

template <typename t, int n>
struct s_carray
{
	t elements[n];

	t& operator[](int index)
	{
		assert(index >= 0);
		assert(index < n);
		return elements[index];
	}
};

template <typename t, int n1, int n2>
struct s_carray2
{
	s_carray<t, n2> elements[n1];

	s_carray<t, n2>& operator[](int index)
	{
		assert(index >= 0);
		assert(index < n1);
		return elements[index];
	}
};

struct s_texture
{
	b8 comes_from_framebuffer;
	u32 gpu_id;
	int game_id;
	s_v2 size;
	s_v2 sub_size;
	const char* path;
};

struct s_glyph
{
	int advance_width;
	int width;
	int height;
	int x0;
	int y0;
	int x1;
	int y1;
	s_v2 uv_min;
	s_v2 uv_max;
};

struct s_font
{
	float size;
	float scale;
	int ascent;
	int descent;
	int line_gap;
	s_texture texture;
	s_carray<s_glyph, 1024> glyph_arr;
};

[[nodiscard]]
static u32 hash(const char* text)
{
	assert(text);
	u32 hash = 5381;
	while(true)
	{
		int c = *text;
		text += 1;
		if(!c) { break; }
		hash = ((hash << 5) + hash) + c;
	}
	return hash;
}

[[nodiscard]]
static constexpr s_v4 rgb(int hex)
{
	s_v4 result;
	result.x = ((hex & 0xFF0000) >> 16) / 255.0f;
	result.y = ((hex & 0x00FF00) >> 8) / 255.0f;
	result.z = ((hex & 0x0000FF)) / 255.0f;
	result.w = 1;
	return result;
}

[[nodiscard]]
static constexpr s_v4 rgba(int hex)
{
	s_v4 result;
	result.x = ((hex & 0xFF000000) >> 24) / 255.0f;
	result.y = ((hex & 0x00FF0000) >> 16) / 255.0f;
	result.z = ((hex & 0x0000FF00) >> 8) / 255.0f;
	result.w = ((hex & 0x000000FF) >> 0) / 255.0f;
	return result;
}

template <typename t>
static t at_least(t a, t b)
{
	return a > b ? a : b;
}

template <typename t>
static t at_most(t a, t b)
{
	return b > a ? a : b;
}

static b8 floats_equal(float a, float b)
{
	return (a >= b - epsilon && a <= b + epsilon);
}

static float ilerp(float start, float end, float val)
{
	float b = end - start;
	if(floats_equal(b, 0)) { return val; }
	return (val - start) / b;
}

template <typename t>
static t clamp(t current, t min_val, t max_val)
{
	return at_most(max_val, at_least(min_val, current));
}

static s_v4 brighter(s_v4 color, float val)
{
	color.x = clamp(color.x * val, 0.0f, 1.0f);
	color.y = clamp(color.y * val, 0.0f, 1.0f);
	color.z = clamp(color.z * val, 0.0f, 1.0f);
	return color;
}

template <typename T>
static constexpr s_v2 v2(T x, T y)
{
	s_v2 result;
	result.x = (float)x;
	result.y = (float)y;
	return result;
}

static constexpr s_v2 v2(s_v2i v)
{
	return v2(v.x, v.y);
}

template <typename T>
static constexpr s_v2 v2(T v)
{
	s_v2 result;
	result.x = (float)v;
	result.y = (float)v;
	return result;
}

static s_v4 v4(s_v3 v3, float a)
{
	return {v3.x, v3.y, v3.z, a};
}

static constexpr s_v3 v3(float x, float y, float z)
{
	s_v3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

static s_v4 hsv_to_rgb(s_v3 color)
{
	s_v4 rgba;
	rgba.w = 1;

	if(color.y <= 0.0f)
	{
		rgba.x = color.z;
		rgba.y = color.z;
		rgba.z = color.z;
		return rgba;
	}

	color.x *= 360.0f;
	if(color.x < 0.0f || color.x >= 360.0f)
		color.x = 0.0f;
	color.x /= 60.0f;

	u32 i = (u32)color.x;
	float ff = color.x - i;
	float p = color.z * (1.0f - color.y );
	float q = color.z * (1.0f - (color.y * ff));
	float t = color.z * (1.0f - (color.y * (1.0f - ff)));

	switch(i)
	{
	case 0:
		rgba.x = color.z;
		rgba.y = t;
		rgba.z = p;
		break;

	case 1:
		rgba.x = q;
		rgba.y = color.z;
		rgba.z = p;
		break;

	case 2:
		rgba.x = p;
		rgba.y = color.z;
		rgba.z = t;
		break;

	case 3:
		rgba.x = p;
		rgba.y = q;
		rgba.z = color.z;
		break;

	case 4:
		rgba.x = t;
		rgba.y = p;
		rgba.z = color.z;
		break;

	default:
		rgba.x = color.z;
		rgba.y = p;
		rgba.z = q;
		break;
	}
	return rgba;
}

static char* format_text(const char* text, ...)
{
	static constexpr int max_format_text_buffers = 16;
	static constexpr int max_text_buffer_length = 256;

	static char buffers[max_format_text_buffers][max_text_buffer_length] = {};
	static int index = 0;

	char* current_buffer = buffers[index];
	memset(current_buffer, 0, max_text_buffer_length);

	va_list args;
	va_start(args, text);
	#ifdef m_debug
	int written = vsnprintf(current_buffer, max_text_buffer_length, text, args);
	assert(written > 0 && written < max_text_buffer_length);
	#else
	vsnprintf(current_buffer, max_text_buffer_length, text, args);
	#endif
	va_end(args);

	index += 1;
	if(index >= max_format_text_buffers) { index = 0; }

	return current_buffer;
}

static constexpr int c_max_arena_push = 16;
struct s_lin_arena
{
	s_sarray<u64, c_max_arena_push> push;
	u64 used;
	u64 capacity;
	void* memory;
};

static void* la_get(s_lin_arena* arena, u64 in_requested)
{
	assert(arena);
	assert(in_requested > 0);
	u64 requested = (in_requested + 7) & ~7;
	assert(arena->used + requested <= arena->capacity);
	void* result = (u8*)arena->memory + arena->used;
	arena->used += requested;
	return result;
}

static void* la_get_zero(s_lin_arena* arena, u64 in_requested)
{
	void* result = la_get(arena, in_requested);
	memset(result, 0, in_requested);
	return result;
}

static void la_push(s_lin_arena* arena)
{
	assert(arena->push.count < c_max_arena_push);
	arena->push.add(arena->used);
}

static void la_pop(s_lin_arena* arena)
{
	assert(arena->push.count > 0);
	arena->used = arena->push.pop();
}


template <int max_chars>
struct s_str_builder
{
	int tab_count;
	int len;
	char data[max_chars];

	void add_(const char* what, b8 use_tabs, va_list args);
	void add(const char* what, ...);
	void add_char(char c);
	void add_with_tabs(const char* what, ...);
	void add_line(const char* what, ...);
	void add_line_with_tabs(const char* what, ...);
	void add_tabs();
	void line();
	void push_tab();
	void pop_tab();
};

template <int max_chars>
void s_str_builder<max_chars>::add_(const char* what, b8 use_tabs, va_list args)
{
	if(use_tabs)
	{
		for(int tab_i = 0; tab_i < tab_count; tab_i++)
		{
			data[len++] = '\t';
		}
	}
	char* where_to_write = &data[len];
	int written = vsnprintf(where_to_write, max_chars + 1 - len, what, args);
	assert(written > 0 && written < max_chars);
	len += written;
	assert(len < max_chars);
	data[len] = 0;
}

template <int max_chars>
void s_str_builder<max_chars>::add(const char* what, ...)
{
	va_list args;
	va_start(args, what);
	add_(what, false, args);
	va_end(args);
}

template <int max_chars>
void s_str_builder<max_chars>::add_char(char c)
{
	assert(len < max_chars);
	data[len++] = c;
	data[len] = 0;
}

template <int max_chars>
void s_str_builder<max_chars>::add_with_tabs(const char* what, ...)
{
	va_list args;
	va_start(args, what);
	builder_add_(this, what, true, args);
	va_end(args);
}

template <int max_chars>
void s_str_builder<max_chars>::add_line(const char* what, ...)
{
	va_list args;
	va_start(args, what);
	add_(what, false, args);
	va_end(args);
	add("\n");
}

template <int max_chars>
void s_str_builder<max_chars>::add_line_with_tabs(const char* what, ...)
{
	va_list args;
	va_start(args, what);
	builder_add_(this, what, true, args);
	va_end(args);
	builder_add(this, "\n");
}

template <int max_chars>
void s_str_builder<max_chars>::add_tabs()
{
	for(int tab_i = 0; tab_i < tab_count; tab_i++)
	{
		data[len++] = '\t';
	}
}

template <int max_chars>
void s_str_builder<max_chars>::line()
{
	builder_add(this, "\n");
}

template <int max_chars>
void s_str_builder<max_chars>::push_tab()
{
	assert(tab_count <= 64);
	tab_count++;
}

template <int max_chars>
void s_str_builder<max_chars>::pop_tab()
{
	assert(tab_count > 0);
	tab_count--;
}

#ifndef m_game

static s_v2 g_base_res = {};

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
	s_carray<u32, e_shader_count> programs;
};
static s_platform_renderer g_platform_renderer = {};

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

static void add_int_attrib(s_attrib_handler* handler, int count);
static void add_float_attrib(s_attrib_handler* handler, int count);
static void finish_attribs(s_attrib_handler* handler);
static u32 load_shader_from_str(const char* vertex_src, const char* fragment_src);
static u32 load_shader_from_file(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena);
static void after_making_framebuffer(int index, s_game_renderer* game_renderer);
static s_font load_font_from_file(const char* path, int font_size, s_lin_arena* arena);
static s_font load_font_from_data(u8* file_data, int font_size, s_lin_arena* arena);
static s_texture load_texture(s_game_renderer* game_renderer, const char* path);
static s_texture load_texture_from_data(void* data, int width, int height, u32 filtering, int format);
static s_font* load_font(s_game_renderer* game_renderer, const char* path, int font_size, s_lin_arena* arena);

static char* read_file(const char* path, s_lin_arena* arena, u64* out_file_size = NULL)
{
	FILE* file = fopen(path, "rb");
	if(!file) { return NULL; }

	fseek(file, 0, SEEK_END);
	u64 file_size = ftell(file);
	fseek(file, 0, SEEK_SET);

	char* data = (char*)la_get(arena, file_size + 1);
	fread(data, file_size, 1, file);
	data[file_size] = 0;
	fclose(file);

	if(out_file_size) { *out_file_size = file_size; }

	return data;
}

static b8 write_file(const char* path, void* data, u64 size)
{
	assert(size > 0);
	FILE* file = fopen(path, "wb");
	if(!file) { return false; }

	fwrite(data, size, 1, file);
	fclose(file);
	return true;
}


#endif // m_game

struct s_rng
{
	u32 seed;

	u32 randu()
	{
		seed = seed * 2147001325 + 715136305;
		return 0x31415926 ^ ((seed >> 16) + (seed << 16));
	}

	b8 rand_bool()
	{
		return randu() & 1;
	}

	f64 randf()
	{
		return (f64)randu() / (f64)4294967295;
	}

	float randf32()
	{
		return (float)randu() / (float)4294967295;
	}

	f64 randf2()
	{
		return randf() * 2 - 1;
	}

	u64 randu64()
	{
		return (u64)(randf() * (f64)c_max_u64);
	}


	// min inclusive, max inclusive
	int rand_range_ii(int min, int max)
	{
		if(min > max)
		{
			int temp = min;
			min = max;
			max = temp;
		}

		return min + ((int)(randu() % (max - min + 1)));
	}

	// min inclusive, max exclusive
	int rand_range_ie(int min, int max)
	{
		if(min > max)
		{
			int temp = min;
			min = max;
			max = temp;
		}

		return min + ((int)(randu() % (max - min)));
	}

	float randf_range(float min_val, float max_val)
	{
		if(min_val > max_val)
		{
			float temp = min_val;
			min_val = max_val;
			max_val = temp;
		}

		float r = (float)randf();
		return min_val + (max_val - min_val) * r;
	}

	b8 chance100(int chance)
	{
		return rand_range_ii(1, 100) <= chance;
	}

};

static constexpr s_v2i v2i(int x, int y)
{
	s_v2i result;
	result.x = x;
	result.y = y;
	return result;
}

static s_v4 v41f(float v)
{
	s_v4 result;
	result.x = v;
	result.y = v;
	result.z = v;
	result.w = v;
	return result;
}


static s_v2 operator+(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

static s_v2i operator+(s_v2i a, s_v2i b)
{
	s_v2i result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

static s_v2 operator+(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x + b;
	result.y = a.y + b;
	return result;
}

static s_v2 operator-(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

static s_v2 operator-(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x - b;
	result.y = a.y - b;
	return result;
}

static s_v2 operator*(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	return result;
}

static s_v2 operator*(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x * b;
	result.y = a.y * b;
	return result;
}

static s_v2i operator*(s_v2i a, int b)
{
	s_v2i result;
	result.x = a.x * b;
	result.y = a.y * b;
	return result;
}

static s_v2 operator/(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x / b;
	result.y = a.y / b;
	return result;
}

static s_v2 operator/(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	return result;
}

static void operator-=(s_v2& a, s_v2 b)
{
	a.x -= b.x;
	a.y -= b.y;
}

static void operator*=(s_v2& a, s_v2 b)
{
	a.x *= b.x;
	a.y *= b.y;
}

static void operator*=(s_v2& a, float b)
{
	a.x *= b;
	a.y *= b;
}

static s_v2 v2_rotate_around(s_v2 v, s_v2 pivot, float angle)
{
	s_v2 p = v;

	float s = sinf(angle);
	float c = cosf(angle);

	// translate point back to origin
	p.x -= pivot.x;
	p.y -= pivot.y;

	// rotate point
	float xnew = p.x * c - p.y * s;
	float ynew = p.x * s + p.y * c;

	// translate point back
	p.x = xnew + pivot.x;
	p.y = ynew + pivot.y;

	return p;
}

static void operator+=(s_v2& left, s_v2 right)
{
	left.x += right.x;
	left.y += right.y;
}

static void operator+=(s_v2i& left, s_v2i right)
{
	left.x += right.x;
	left.y += right.y;
}

[[nodiscard]]
static int circular_index(int index, int size)
{
	assert(size > 0);
	if(index >= 0)
	{
		return index % size;
	}
	return (size - 1) - ((-index - 1) % size);
}

static float v2_angle(s_v2 v)
{
	return atan2f(v.y, v.x);
}

static float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

template <typename t>
static t max(t a, t b)
{
	return a >= b ? a : b;
}

template <typename t>
static t min(t a, t b)
{
	return a <= b ? a : b;
}

static int roundfi(float x)
{
	return (int)roundf(x);
}

static float sinf2(float t)
{
	return sinf(t) * 0.5f + 0.5f;
}

static b8 operator==(s_v2i a, s_v2i b)
{
	return a.x == b.x && a.y == b.y;
}

template <typename t>
static constexpr s_v4 make_color(t v)
{
	s_v4 result;
	result.x = (float)v;
	result.y = (float)v;
	result.z = (float)v;
	result.w = 1;
	return result;
}

template <typename t0, typename t1, typename t2>
static constexpr s_v4 make_color(t0 r, t1 g, t2 b)
{
	s_v4 result;
	result.x = (float)r;
	result.y = (float)g;
	result.z = (float)b;
	result.w = 1;
	return result;
}

static s_v2 v2_from_angle(float angle)
{
	return v2(
		cosf(angle),
		sinf(angle)
	);
}

static float sign(float x)
{
	return x >= 0 ? 1.0f : -1.0f;
}

static float deg_to_rad(float d)
{
	return d * (pi / 180.f);
}


static b8 rect_collides_circle_topleft(s_v2 rect_pos, s_v2 rect_size, s_v2 center, float radius)
{
	s_v2 rect_center = rect_pos + rect_size * 0.5f;
	b8 collision = false;

	float dx = fabsf(center.x - rect_center.x);
	float dy = fabsf(center.y - rect_center.y);

	if(dx > (rect_size.x/2.0f + radius)) { return false; }
	if(dy > (rect_size.y/2.0f + radius)) { return false; }

	if(dx <= (rect_size.x/2.0f)) { return true; }
	if(dy <= (rect_size.y/2.0f)) { return true; }

	float cornerDistanceSq = (dx - rect_size.x/2.0f)*(dx - rect_size.x/2.0f) +
													(dy - rect_size.y/2.0f)*(dy - rect_size.y/2.0f);

	collision = (cornerDistanceSq <= (radius*radius));

	return collision;
}

static b8 rect_collides_rect_topleft(s_v2 pos0, s_v2 size0, s_v2 pos1, s_v2 size1)
{
	return pos0.x + size0.x > pos1.x && pos0.x < pos1.x + size1.x &&
		pos0.y + size0.y > pos1.y && pos0.y < pos1.y + size1.y;
}

static b8 mouse_collides_rect_topleft(s_v2 mouse, s_v2 pos, s_v2 size)
{
	return rect_collides_rect_topleft(mouse, v2(1, 1), pos, size);
}


static int double_until_greater_or_equal(int current, int target)
{
	assert(target > 0);
	if(current <= 0) { current = 1; }
	while(current < target) { current *= 2; }
	return current;
}

static int floorfi(float x)
{
	return (int)floorf(x);
}

static int ceilfi(float x)
{
	return (int)ceilf(x);
}

static float fract(float x)
{
	return x - (int)x;
}

static float v2_length(s_v2 a)
{
	return sqrtf(a.x * a.x + a.y * a.y);
}

static float v2_distance(s_v2 a, s_v2 b)
{
	return v2_length(a - b);
}


static s_v2 lerp(s_v2 a, s_v2 b, float t)
{
	s_v2 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	return result;
}

static s_v2 lerp_snap(s_v2 a, s_v2 b, float t)
{
	s_v2 result;
	float dist = v2_distance(a, b);
	if(dist < 1.0f)
	{
		t = 1;
	}
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	return result;
}

static s_v4 lerp(s_v4 a, s_v4 b, float t)
{
	s_v4 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	result.z = lerp(a.z, b.z, t);
	result.w = lerp(a.w, b.w, t);
	return result;
}

static s_v2 v2_normalized(s_v2 v)
{
	s_v2 result;
	float length = v2_length(v);
	if(length != 0)
	{
		result.x = v.x / length;
		result.y = v.y / length;
	}
	else
	{
		result = v;
	}
	return result;
}

static s_v2 v2_rand_normalized(s_rng* rng)
{
	return v2_normalized(v2(
		(float)rng->randf2(),
		(float)rng->randf2()
	));
}

static float v2_dot(s_v2 a, s_v2 b)
{
	float dot = a.x * b.x + a.y * b.y;
	return dot;
}

static s_v2 v2_reflect(s_v2 v, s_v2 normal)
{
	s_v2 result;

	float dotProduct = v2_dot(v, normal);

	result.x = v.x - (2.0f * normal.x) * dotProduct;
	result.y = v.y - (2.0f * normal.y) * dotProduct;

	return result;
}


static float range_lerp(float input_val, float input_start, float input_end, float output_start, float output_end)
{
	return output_start + ((output_end - output_start) / (input_end - input_start)) * (input_val - input_start);
}

static int str_find_from_left(const char* haystack, int haystack_len, const char* needle, int needle_len)
{
	if(needle_len > haystack_len) { return -1; }

	for(int haystack_i = 0; haystack_i < haystack_len - (needle_len - 1); haystack_i++)
	{
		b8 found = true;
		for(int needle_i = 0; needle_i < needle_len; needle_i++)
		{
			char haystack_c = haystack[haystack_i + needle_i];
			char needle_c = needle[needle_i];
			if(haystack_c != needle_c)
			{
				found = false;
				break;
			}
		}
		if(found)
		{
			return haystack_i;
		}
	}
	return -1;
}

static b8 str_replace(char* str, const char* needle, const char* replacement)
{
	int str_len = (int)strlen(str);
	int needle_len = (int)strlen(needle);
	int replacement_len = (int)strlen(replacement);

	if(needle_len > str_len) { return false; }

	int index = str_find_from_left(str, str_len, needle, needle_len);
	if(index == -1) { return false; }

	int to_copy = str_len - (index + 1);
	if(to_copy > 0 && needle_len != replacement_len)
	{
		memmove(&str[index + replacement_len], &str[index + needle_len], str_len - index);
	}
	memmove(&str[index], replacement, replacement_len);
	str[str_len + to_copy] = 0;

	return true;

}


static constexpr int c_key_backspace = 0x08;
static constexpr int c_key_tab = 0x09;
static constexpr int c_key_enter = 0x0D;
static constexpr int c_key_alt = 0x12;
static constexpr int c_key_left_alt = 0xA4;
static constexpr int c_key_right_alt = 0xA5;
static constexpr int c_key_escape = 0x1B;
static constexpr int c_key_space = 0x20;
static constexpr int c_key_end = 0x23;
static constexpr int c_key_home = 0x24;
static constexpr int c_key_left = 0x25;
static constexpr int c_key_up = 0x26;
static constexpr int c_key_right = 0x27;
static constexpr int c_key_down = 0x28;
static constexpr int c_key_delete = 0x2E;
static constexpr int c_key_0 = 0x30;
static constexpr int c_key_1 = 0x31;
static constexpr int c_key_2 = 0x32;
static constexpr int c_key_3 = 0x33;
static constexpr int c_key_4 = 0x34;
static constexpr int c_key_5 = 0x35;
static constexpr int c_key_6 = 0x36;
static constexpr int c_key_7 = 0x37;
static constexpr int c_key_8 = 0x38;
static constexpr int c_key_9 = 0x39;
static constexpr int c_key_a = 0x41;
static constexpr int c_key_b = 0x42;
static constexpr int c_key_c = 0x43;
static constexpr int c_key_d = 0x44;
static constexpr int c_key_e = 0x45;
static constexpr int c_key_f = 0x46;
static constexpr int c_key_g = 0x47;
static constexpr int c_key_h = 0x48;
static constexpr int c_key_i = 0x49;
static constexpr int c_key_j = 0x4A;
static constexpr int c_key_k = 0x4B;
static constexpr int c_key_l = 0x4C;
static constexpr int c_key_m = 0x4D;
static constexpr int c_key_n = 0x4E;
static constexpr int c_key_o = 0x4F;
static constexpr int c_key_p = 0x50;
static constexpr int c_key_q = 0x51;
static constexpr int c_key_r = 0x52;
static constexpr int c_key_s = 0x53;
static constexpr int c_key_t = 0x54;
static constexpr int c_key_u = 0x55;
static constexpr int c_key_v = 0x56;
static constexpr int c_key_w = 0x57;
static constexpr int c_key_x = 0x58;
static constexpr int c_key_y = 0x59;
static constexpr int c_key_z = 0x5A;
static constexpr int c_key_add = 0x6B;
static constexpr int c_key_subtract = 0x6D;
static constexpr int c_key_f1 = 0x70;
static constexpr int c_key_f2 = 0x71;
static constexpr int c_key_f3 = 0x72;
static constexpr int c_key_f4 = 0x73;
static constexpr int c_key_f5 = 0x74;
static constexpr int c_key_f6 = 0x75;
static constexpr int c_key_f7 = 0x76;
static constexpr int c_key_f8 = 0x77;
static constexpr int c_key_f9 = 0x78;
static constexpr int c_key_f10 = 0x79;
static constexpr int c_key_f11 = 0x7A;
static constexpr int c_key_f12 = 0x7B;
static constexpr int c_key_left_shift = 0xA0;
static constexpr int c_key_right_shift = 0xA1;
static constexpr int c_key_left_ctrl = 0xA2;
static constexpr int c_key_right_ctrl = 0xA3;
static constexpr int c_left_mouse = 1020;
static constexpr int c_right_mouse = 1021;
static constexpr int c_max_keys = 1024;

static constexpr int c_game_memory = 1 * c_mb;

static constexpr s_v2 c_origin_topleft = {1.0f, -1.0f};
static constexpr s_v2 c_origin_bottomleft = {1.0f, 1.0f};
static constexpr s_v2 c_origin_center = {0, 0};

static constexpr int c_base_resolution_index = 5;
static constexpr s_v2i c_resolutions[] = {
	v2i(640, 360),
	v2i(854, 480),
	v2i(960, 540),
	v2i(1024, 576),
	v2i(1280, 720),
	v2i(1366, 768),
	v2i(1600, 900),
	v2i(1920, 1080),
	v2i(2560, 1440),
	v2i(3200, 1800),
	v2i(3840, 2160),
	v2i(5120, 2880),
	v2i(7680, 4320),
};

enum e_render_flags
{
	e_render_flag_use_texture = 1 << 0,
	e_render_flag_flip_x = 1 << 1,
	e_render_flag_circle = 1 << 2,
	e_render_flag_text = 1 << 3,
};

struct s_sound
{
	int index;
	int sample_count;
	s16* samples;
};

struct s_framebuffer;
struct s_platform_data;

typedef b8 (*t_play_sound)(s_sound*);
typedef void (*t_set_vsync)(b8);
typedef int (*t_show_cursor)(b8);
typedef int (*t_cycle_between_available_resolutions)(int);
typedef u32 (*t_get_random_seed)();
typedef s_framebuffer* (*t_make_framebuffer)(s_game_renderer*, b8);
typedef s_sound* (*t_load_sound)(s_platform_data*, const char*, s_lin_arena*);
typedef b8 (*t_set_shader_float)(const char*, float);
typedef b8 (*t_set_shader_v2)(const char*, s_v2);
typedef char* (*t_read_file)(const char*, s_lin_arena*, u64*);
typedef b8 (*t_write_file)(const char*, void*, u64);

// @Note(tkap, 08/10/2023): We have a bug with this. If we ever go from having never drawn anything to drawing 64*16+1 things we will
// exceed the max bucket count (16 currently). To fix this, I guess we have to allow merging in the middle of a frame?? Seems messy...
static constexpr int c_bucket_capacity = 64;

template <typename t>
struct s_bucket_array
{
	int bucket_count;
	s_carray<int, 16> capacity;
	s_carray<int, 16> element_count;
	s_carray<t*, 16> elements;
};

#pragma pack(push, 1)
struct s_transform
{
	int flags;
	int layer;
	int sublayer;
	int effect_id;
	float mix_weight;
	float rotation;
	s_v2 pos;
	s_v2 origin_offset;
	s_v2 draw_size;
	s_v2 texture_size;
	s_v2 uv_min;
	s_v2 uv_max;
	s_v4 color;
	s_v4 mix_color;
};
#pragma pack(pop)

struct s_framebuffer
{
	u32 gpu_id;
	int game_id;
	s_texture texture;
	b8 do_depth;

	// @Note(tkap, 08/10/2023): We esentially want s_bucket_array<s_transform> transforms[e_texture_count][e_blend_mode_count];
	// but we don't know how many textures there will be at compile time, because the game code may load any amount
	s_bucket_array<s_transform>* transforms;
};

struct s_game_window
{
	int width;
	int height;

	// @Note(tkap, 24/06/2023): Set by the game
	s_v2 size;
	s_v2 center;
};

struct s_stored_input
{
	b8 is_down;
	int key;
};

struct s_key
{
	b8 is_down;
	int count;
};

struct s_input
{
	s_carray<s_key, c_max_keys> keys;
};

enum e_blend_mode
{
	e_blend_mode_normal,
	e_blend_mode_additive,
	e_blend_mode_count,
};

struct s_render_data
{
	e_blend_mode blend_mode;
	s_framebuffer* framebuffer;
};

enum e_var_type
{
	e_var_type_int,
	e_var_type_float,
	e_var_type_bool,
};

union s_var_value
{
	float val_float;
	int val_int;
};

struct s_var
{
	b8 display;
	e_var_type type;
	void* ptr;
	const char* name;
	s_var_value min_val;
	s_var_value max_val;
};

enum e_ui
{
	e_ui_nothing,
	e_ui_hovered,
	e_ui_pressed,
	e_ui_active,
};

struct s_ui_interaction
{
	b8 pressed_this_frame;
	e_ui state;
};

struct s_platform_data
{
	b8 recompiled;
	b8 quit_after_this_frame;
	b8 any_key_pressed;
	b8 is_window_active;
	b8 window_resized;

	#ifdef m_debug
	b8 loaded_a_state;
	#endif // m_debug

	int window_width;
	int window_height;
	s_input* input;
	s_input* logic_input;
	s_lin_arena* frame_arena;
	s_v2 mouse;
	f64 frame_time;
	t_get_random_seed get_random_seed;
	t_load_sound load_sound;
	t_play_sound play_sound;
	s_sarray<s_sound, 16> sounds;
	t_show_cursor show_cursor;
	t_cycle_between_available_resolutions cycle_between_available_resolutions;
	t_read_file read_file;
	t_write_file write_file;
	void (*reset_ui)();
	s_ui_interaction (*ui_button)(s_game_renderer*, const char*, s_v2, s_v2, s_font*, float, s_input*, s_v2);
	void (*set_window_size)(int, int);
	char* variables_path;

	#ifdef m_debug
	s_v2 vars_pos;
	s_v2 vars_pos_offset;
	b8 show_live_vars;
	s_sarray<s_var, 128> vars;
	#endif // m_debug
};

#ifndef m_game
static s_platform_data g_platform_data = {};
#endif // m_game

typedef s_texture (*t_load_texture)(s_game_renderer*, const char*);
typedef s_font* (*t_load_font)(s_game_renderer*, const char*, int, s_lin_arena*);
struct s_game_renderer
{
	b8 did_we_alloc;
	t_set_vsync set_vsync;
	t_load_texture load_texture;
	t_load_font load_font;
	t_make_framebuffer make_framebuffer;
	t_set_shader_float set_shader_float;
	t_set_shader_v2 set_shader_v2;
	f64 total_time;

	s_texture checkmark_texture;

	int transform_arena_index;
	s_carray<s_lin_arena, 2> transform_arenas;
	int arena_index;
	s_carray<s_lin_arena, 2> arenas;
	s_sarray<s_texture, 16> textures;
	s_sarray<s_framebuffer, 4> framebuffers;
	s_sarray<s_font, 4> fonts;
};

#define m_init_game(name) void name(s_platform_data* platform_data)
#define m_update_game(name) void name(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
#ifdef m_build_dll
typedef m_init_game(t_init_game);
typedef m_update_game(t_update_game);
#else // m_build_dll
m_update_game(update_game);
m_init_game(init_game);
#endif


static int get_render_offset(int texture, int blend_mode);
static s_v2 get_text_size_with_count(const char* text, s_font* font, float font_size, int count);
static s_v2 get_text_size(const char* text, s_font* font, float font_size);
template <typename t>
static void bucket_add(s_bucket_array<t>* arr, t new_element, s_lin_arena* arena, b8* did_we_alloc);
template <typename t>
static void bucket_merge(s_bucket_array<t>* arr, s_lin_arena* arena);




// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------

static void apply_event_to_input(s_input* in_input, s_stored_input event)
{
	in_input->keys[event.key].is_down = event.is_down;
	in_input->keys[event.key].count += 1;
}

static b8 is_key_down(s_input* input, int key) {
	assert(key < c_max_keys);
	return input->keys[key].is_down || input->keys[key].count >= 2;
}

static b8 is_key_up(s_input* input, int key) {
	assert(key < c_max_keys);
	return !input->keys[key].is_down;
}

static b8 is_key_pressed(s_input* input, int key) {
	assert(key < c_max_keys);
	return (input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

static b8 is_key_released(s_input* input, int key) {
	assert(key < c_max_keys);
	return (!input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

static int get_render_offset(int texture, int blend_mode)
{
	return texture * e_blend_mode_count + blend_mode;
}

// @Note(tkap, 15/10/2023): Should this always return font_size for y (like it does now), or actually get the tallest char?
static s_v2 get_text_size_with_count(const char* text, s_font* font, float font_size, int count)
{
	assert(count >= 0);
	if(count <= 0) { return {}; }

	s_v2 size = {};
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

static s_v2 get_text_size(const char* text, s_font* font, float font_size)
{
	return get_text_size_with_count(text, font, font_size, (int)strlen(text));
}

static void draw_rect(s_game_renderer* game_renderer, s_v2 pos, int layer, s_v2 size, s_v4 color, s_render_data render_data = {}, s_transform t = {})
{
	if(!render_data.framebuffer) {
		render_data.framebuffer = &game_renderer->framebuffers[0];
	}

	t.pos = pos;
	t.layer = layer;
	t.draw_size = size;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	t.mix_color = v41f(1);
	bucket_add(&render_data.framebuffer->transforms[get_render_offset(0, render_data.blend_mode)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_texture(s_game_renderer* game_renderer, s_v2 pos, int layer, s_v2 size, s_v4 color, s_texture texture, s_render_data render_data = {}, s_transform t = {})
{
	if(!render_data.framebuffer) {
		render_data.framebuffer = &game_renderer->framebuffers[0];
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
	bucket_add(&render_data.framebuffer->transforms[get_render_offset(texture.game_id, render_data.blend_mode)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_text(s_game_renderer* game_renderer, const char* text, s_v2 in_pos, int layer, float font_size, s_v4 color, b8 centered, s_font* font, s_render_data render_data = {}, s_transform t = {})
{
	if(!render_data.framebuffer) {
		render_data.framebuffer = &game_renderer->framebuffers[0];
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
		t.flags |= e_render_flag_use_texture | e_render_flag_text;
		t.pos = glyph_pos;

		s_v2 center = t.pos + t.draw_size / 2 * v2(1, -1);
		s_v2 bottomleft = t.pos;

		// s_v2 topleft = t.pos + t.draw_size * v2(0, -1);
		// draw_rect(t.pos, 1, t.draw_size, make_color(0.4f, 0,0), {}, {.origin_offset = c_origin_bottomleft});
		// draw_rect(center, 75, v2(4), make_color(0, 1,0), {});
		// draw_rect(topleft, 75, v2(4), make_color(0, 0,1), {}, {.origin_offset = c_origin_topleft});
		// draw_rect(bottomleft, 75, v2(4), make_color(1, 1,0), {}, {.origin_offset = c_origin_bottomleft});
		// draw_rect(in_pos, 77, v2(4), make_color(0, 1,1), {}, {.origin_offset = c_origin_topleft});

		t.pos = v2_rotate_around(center, in_pos, t.rotation) + (bottomleft - center);

		t.color = color;
		t.uv_min = glyph.uv_min;
		t.uv_max = glyph.uv_max;
		t.origin_offset = c_origin_bottomleft;

		bucket_add(
			&render_data.framebuffer->transforms[get_render_offset(font->texture.game_id, render_data.blend_mode)], t,
			&game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc
		);

		pos.x += glyph.advance_width * scale;

	}
}

static s_lin_arena make_lin_arena(u64 capacity)
{
	assert(capacity > 0);
	capacity = (capacity + 7) & ~7;
	s_lin_arena result = {};
	result.capacity = capacity;
	result.memory = malloc(capacity);
	return result;
}

static s_lin_arena make_lin_arena_from_memory(u64 capacity, void* memory)
{
	assert(capacity > 0);
	assert(memory);

	capacity = (capacity + 7) & ~7;
	s_lin_arena result = {};
	result.capacity = capacity;
	result.memory = memory;
	return result;
}


template <typename t>
static void bucket_add(s_bucket_array<t>* arr, t new_element, s_lin_arena* arena, b8* did_we_alloc)
{
	// printf("bucket_add at %p, bucket_count = %i\n", arr, arr->bucket_count);
	for(int i = 0; i < arr->bucket_count; i++)
	{
		int* count = &arr->element_count[i];
		// printf("count = %i\n", *count);
		if(*count < arr->capacity[i])
		{
			arr->elements[i][*count] = new_element;
			*count += 1;
			return;
		}
	}
	assert(arr->bucket_count < 16);
	constexpr int capacity = c_bucket_capacity;
	arr->elements[arr->bucket_count] = (t*)la_get(arena, sizeof(t) * capacity);
	arr->capacity[arr->bucket_count] = capacity;
	arr->elements[arr->bucket_count][0] = new_element;
	arr->element_count[arr->bucket_count] = 1;
	arr->bucket_count += 1;

	if(arr->bucket_count > 1)
	{
		*did_we_alloc = true;
	}
}

template <typename t>
static void bucket_merge(s_bucket_array<t>* arr, s_lin_arena* arena)
{
	int capacity = 0;
	int count = 0;
	constexpr int element_size = sizeof(t);
	t* elements;

	if(arr->element_count[0] <= 0) { return; }

	for(int i = 0; i < arr->bucket_count; i++)
	{
		capacity += arr->capacity[i];
	}

	elements = (t*)la_get(arena, element_size * capacity);

	for(int i = 0; i < arr->bucket_count; i++)
	{
		assert(arr->element_count[i] > 0);
		memcpy(&elements[count], arr->elements[i], element_size * arr->element_count[i]);
		count += arr->element_count[i];
	}

	arr->elements[0] = elements;
	arr->capacity[0] = capacity;
	arr->element_count[0] = count;
	arr->bucket_count = 1;
}

static void on_failed_assert(const char* cond, const char* file, int line)
{
	printf("FAILED ASSERT: %s\n%s (%i)\n", cond, file, line);
	#ifndef __EMSCRIPTEN__
	__debugbreak();
	#endif // __EMSCRIPTEN__
	printf("Press ENTER to exit...");
	// *(char*)1 = 0;
	getchar();
	exit(1);
	// @Fixme(tkap, 05/10/2023):
	// char* text = format_text("FAILED ASSERT IN %s (%i)\n%s\n", file, line, cond);
	// printf("%s\n", text);
	// int result = MessageBox(NULL, text, "Assertion failed", MB_RETRYCANCEL | MB_TOPMOST);
	// if(result != IDRETRY)
	// {
	// 	if(IsDebuggerPresent())
	// 	{
	// 		__debugbreak();
	// 	}
	// 	else
	// 	{
	// 		exit(1);
	// 	}
	// }
}

#ifndef m_game
static u32 load_shader(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena);
static b8 check_for_shader_errors(u32 id, char* out_error);
static s_texture load_texture_from_file(const char* path, u32 filtering);
static void after_loading_texture(s_game_renderer* game_renderer);

static void set_window_size(int width, int height)
{
	g_base_res.x = (float)width;
	g_base_res.y = (float)height;
}

// @TODO(tkap, 17/10/2023): This has to go away in favor of a dynamic system when we add support for custom shaders
static constexpr s_shader_paths c_shader_paths[e_shader_count] = {
	{
		.vertex_path = "shaders/vertex.vertex",
		.fragment_path = "shaders/fragment.fragment",
	},
};

struct s_ui_id
{
	u32 id;
};

struct s_ui
{
	b8 pressed_present;
	s_ui_id hovered;
	s_ui_id pressed;
};


static s_ui g_ui = {};

static void ui_request_hovered(s_ui* ui, u32 id)
{
	if(ui->pressed.id != 0) { return; }
	ui->hovered.id = id;
}

static void ui_request_pressed(s_ui* ui, u32 id)
{
	ui->hovered.id = 0;
	ui->pressed.id = id;
}

static void ui_request_active(s_ui* ui, u32 id)
{
	unreferenced(id);
	ui->hovered.id = 0;
	ui->pressed.id = 0;
}

static s_ui_interaction ui_button(
	s_game_renderer* game_renderer, const char* text, s_v2 pos, s_v2 size, s_font* font, float font_size, s_input* input, s_v2 mouse
)
{
	s_ui_interaction result = {};
	u32 id = hash(text);
	s_v4 button_color = rgb(0x217278);
	b8 hovered = mouse_collides_rect_topleft(mouse, pos, size);
	if(hovered) {
		ui_request_hovered(&g_ui, id);
	}
	if(g_ui.hovered.id == id) {
		result.state = e_ui_hovered;
		button_color = brighter(button_color, 1.4f);
		if(hovered && is_key_pressed(input, c_left_mouse)) {
			result.pressed_this_frame = true;
			ui_request_pressed(&g_ui, id);
		}
		else if(!hovered) {
			ui_request_hovered(&g_ui, 0);
		}
	}
	if(g_ui.pressed.id == id) {
		result.state = e_ui_pressed;
		g_ui.pressed_present = true;
		button_color = brighter(button_color, 0.6f);
		if(is_key_released(input, c_left_mouse)) {
			if(hovered) {
				result.state = e_ui_active;
				ui_request_active(&g_ui, id);
			}
			else {
				ui_request_pressed(&g_ui, 0);
			}
		}
	}

	draw_rect(game_renderer, pos, 10, size, button_color, {}, {.origin_offset = c_origin_topleft});
	if(font) {
		s_v2 text_pos = pos;
		text_pos += size / 2;
		draw_text(game_renderer, text, text_pos, 11, font_size, rgb(0xFB9766), true, font);
	}
	return result;
}

template <typename t>
static t ui_slider(
	s_game_renderer* game_renderer, const char* text, s_v2 pos, s_v2 size, s_font* font, float font_size, t min_val, t max_val, t curr_val, s_input* input, s_v2 mouse
)
{
	constexpr b8 is_int = is_same<t, int>;
	constexpr b8 is_float = is_same<t, float>;
	static_assert(is_int || is_float);

	t result = curr_val;
	u32 id = hash(text);
	s_v4 button_color = rgb(0x217278);
	s_v4 handle_color = rgb(0xEA5D58);
	s_v2 handle_size = v2(size.y);
	b8 hovered = mouse_collides_rect_topleft(mouse, pos, size);
	if(hovered) {
		ui_request_hovered(&g_ui, id);
	}
	if(g_ui.hovered.id == id) {
		button_color = brighter(button_color, 1.4f);
		if(hovered && is_key_pressed(input, c_left_mouse)) {
			ui_request_pressed(&g_ui, id);
		}
		else if(!hovered) {
			ui_request_hovered(&g_ui, 0);
		}
	}
	if(g_ui.pressed.id == id) {
		g_ui.pressed_present = true;
		button_color = brighter(button_color, 0.6f);
		if(is_key_released(input, c_left_mouse)) {
			if(hovered) {
				ui_request_active(&g_ui, id);
			}
			else {
				ui_request_pressed(&g_ui, 0);
			}
		}
		float percent;
		if(is_key_down(input, c_key_left_ctrl)) {
			percent = ilerp(handle_size.x * 0.5f, g_base_res.x - handle_size.x * 0.5f, mouse.x);
		}
		else {
			percent = ilerp(pos.x + handle_size.x * 0.5f, pos.x + size.x - handle_size.x * 0.5f, mouse.x);
		}
		result = (t)lerp((float)min_val, (float)max_val, percent);
	}
	result = clamp(result, min_val, max_val);

	s_v2 handle_pos = v2(
		pos.x + ilerp((float)min_val, (float)max_val, (float)result) * (size.x - handle_size.y),
		pos.y - handle_size.y / 2 + size.y / 2
	);

	draw_rect(game_renderer, pos, 10, size, button_color, {}, {.origin_offset = c_origin_topleft});
	draw_rect(game_renderer, handle_pos, 11, handle_size, handle_color, {}, {.flags = e_render_flag_circle, .origin_offset = c_origin_topleft});
	s_v2 text_pos = pos;
	text_pos += size / 2;
	if(is_int) {
		draw_text(game_renderer, format_text("%i", result), text_pos, 15, font_size, rgb(0xFB9766), true, font);
	}
	else if(is_float) {
		draw_text(game_renderer, format_text("%.2f", result), text_pos, 15, font_size, rgb(0xFB9766), true, font);
	}
	return result;
}

static void ui_checkbox(s_game_renderer* game_renderer, const char* text, s_v2 pos, s_v2 size, b8* val, s_input* input, s_v2 mouse)
{
	assert(val);
	u32 id = hash(text);
	s_v4 button_color = rgb(0x217278);
	b8 hovered = mouse_collides_rect_topleft(mouse, pos, size);
	if(hovered) {
		ui_request_hovered(&g_ui, id);
	}
	if(g_ui.hovered.id == id) {
		button_color = brighter(button_color, 1.4f);
		if(hovered && is_key_pressed(input, c_left_mouse)) {
			ui_request_pressed(&g_ui, id);
		}
		else if(!hovered) {
			ui_request_hovered(&g_ui, 0);
		}
	}
	if(g_ui.pressed.id == id) {
		g_ui.pressed_present = true;
		button_color = brighter(button_color, 0.6f);
		if(is_key_released(input, c_left_mouse)) {
			if(hovered) {
				*val = !(*val);
				ui_request_active(&g_ui, id);
			}
			else {
				ui_request_pressed(&g_ui, 0);
			}
		}
	}

	draw_rect(game_renderer, pos, 10, size, button_color, {}, {.origin_offset = c_origin_topleft});
	if(*val) {
		draw_texture(game_renderer, pos, 11, size, make_color(0,1,0), game_renderer->checkmark_texture, {}, {.origin_offset = c_origin_topleft});
	}
}

static b8 g_do_embed = false;
static s_sarray<const char*, 128> g_to_embed;
static int g_asset_index = 0;

static void write_embed_file()
{
	constexpr int max_chars = 100 * c_mb;
	assert(g_do_embed);
	s_str_builder<max_chars>* builder = (s_str_builder<max_chars>*)malloc(sizeof(s_str_builder<max_chars>));
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

		builder->add_line("static constexpr u8 embed%i[%u] = {", embed_i, file_size);
		for(u64 i = 0; i < file_size; i++) {
			builder->add("%u,", *cursor);
			cursor++;
		}
		builder->add_line("\n};");

		fclose(file);
		free(data);
	}

	builder->add_line("static constexpr u8* embed_data[%i] = {", g_to_embed.count);
	foreach_val(embed_i, embed, g_to_embed) {
		builder->add("(u8*)embed%i,", embed_i);
	}
	builder->add_line("\n};");

	builder->add_line("static constexpr int embed_sizes[%i] = {", g_to_embed.count);
	foreach_val(embed_i, embed, g_to_embed) {
		builder->add("array_count(embed%i),", embed_i);
	}
	builder->add_line("\n};");

	{
		FILE* file = fopen("src/embed.h", "wb");
		fwrite(builder->data, 1, builder->len, file);
		fclose(file);
	}

	printf("Successfully created embed.h!\n");

	exit(0);
}

static void do_game_layer(
	s_game_renderer* game_renderer, void* game_memory
	#ifndef m_sdl
	, t_update_game update_game
	#endif
)
{
	#ifdef m_debug

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		save states start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		char* path = "save_state";
		if(is_key_pressed(g_platform_data.input, c_key_f3)) {
			write_file(path, game_memory, c_game_memory);
		}
		if(is_key_pressed(g_platform_data.input, c_key_f4)) {
			u8* data = (u8*)read_file(path, g_platform_data.frame_arena, NULL);
			if(data) {
				memcpy(game_memory, data, c_game_memory);
				g_platform_data.loaded_a_state = true;
			}
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		save states end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	if(is_key_pressed(g_platform_data.input, c_key_f1)) {
		g_platform_data.show_live_vars = !g_platform_data.show_live_vars;
	}

	if(g_platform_data.show_live_vars) {
		s_v2 pos = g_platform_data.vars_pos;
		s_ui_interaction interaction = ui_button(game_renderer, "Move", pos, v2(32), NULL, 32, g_platform_data.input, g_platform_data.mouse);
		if(interaction.state == e_ui_pressed) {
			if(interaction.pressed_this_frame) {
				g_platform_data.vars_pos_offset = g_platform_data.mouse - pos;
			}
			s_v2 m = g_platform_data.mouse;
			m -= g_platform_data.vars_pos_offset;
			m.x = clamp(m.x, 0.0f, g_base_res.x - 32);
			m.y = clamp(m.y, 0.0f, g_base_res.y - 32);
			g_platform_data.vars_pos = m;
			pos = m;
		}
		pos += v2(100);

		constexpr float font_size = 24;
		constexpr float button_height = 32;

		foreach_val(var_i, var, g_platform_data.vars) {
			if(!var.display) { continue; }
			s_v2 text_pos = pos;
			text_pos.x -= 300;
			text_pos.y += button_height / 2.0f - font_size * 0.5f;
			s_v2 slider_pos = pos;
			draw_text(game_renderer, var.name, text_pos, 15, font_size, rgb(0xffffff), false, &game_renderer->fonts[0]);
			if(var.type == e_var_type_int) {
				*(int*)var.ptr = ui_slider(
					game_renderer, var.name, slider_pos, v2(200.0f, button_height), &game_renderer->fonts[0], font_size,
					var.min_val.val_int, var.max_val.val_int, *(int*)var.ptr, g_platform_data.input, g_platform_data.mouse
				);
			}
			else if(var.type == e_var_type_float) {
				*(float*)var.ptr = ui_slider(
					game_renderer, var.name, slider_pos, v2(200.0f, button_height), &game_renderer->fonts[0], font_size,
					var.min_val.val_float, var.max_val.val_float, *(float*)var.ptr, g_platform_data.input, g_platform_data.mouse
				);
			}
			else if(var.type == e_var_type_bool) {
				s_v2 temp = slider_pos;
				temp.x += 100 - button_height / 2.0f;
				ui_checkbox(game_renderer, var.name, temp, v2(button_height), (b8*)var.ptr, g_platform_data.input, g_platform_data.mouse);
			}
			pos.y += button_height + 4.0f;
		}

		if(ui_button(
				game_renderer, "Save", pos, v2(200.0f, button_height), &game_renderer->fonts[0], font_size, g_platform_data.input, g_platform_data.mouse
			).state == e_ui_active) {
			s_str_builder<10 * c_kb> builder;
			foreach_val(var_i, var, g_platform_data.vars) {
				if(var.type == e_var_type_int) {
					builder.add_line("static int %s = %i;", var.name, *(int*)var.ptr);
				}
				else if(var.type == e_var_type_float) {
					builder.add_line("static float %s = %ff;", var.name, *(float*)var.ptr);
				}
				else if(var.type == e_var_type_bool) {
					builder.add_line("static b8 %s = %s;", var.name, *(b8*)var.ptr ? "true" : "false");
				}
				invalid_else;
			}
			if(g_platform_data.variables_path == NULL) {
				printf("Path to variables file is not set!!\n");
			}
			else {
				write_file(g_platform_data.variables_path, builder.data, builder.len);
			}
		}
	}

	g_platform_data.vars.count = 0;
	#endif // m_debug

	update_game(&g_platform_data, game_memory, game_renderer);
	g_platform_data.recompiled = false;

	#ifdef m_debug
	g_platform_data.loaded_a_state = false;
	#endif // m_debug

	if(g_do_embed) {
		write_embed_file();
	}

	g_platform_data.frame_arena->used = 0;
}

static void on_gl_error(const char* expr, int error)
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

static void init_gl(s_platform_renderer* platform_renderer, s_game_renderer* game_renderer, s_lin_arena* arena)
{
	gl(glGenVertexArrays(1, &platform_renderer->default_vao));
	gl(glBindVertexArray(platform_renderer->default_vao));

	gl(glGenBuffers(1, &platform_renderer->default_vbo));
	gl(glBindBuffer(GL_ARRAY_BUFFER, platform_renderer->default_vbo));

	s_attrib_handler handler = {};
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
	gl(glBufferData(GL_ARRAY_BUFFER, sizeof(s_transform) * platform_renderer->max_elements, NULL, GL_DYNAMIC_DRAW));

	for(int shader_i = 0; shader_i < e_shader_count; shader_i++)
	{
		u32 program = load_shader(c_shader_paths[shader_i].vertex_path, c_shader_paths[shader_i].fragment_path, arena);
		assert(program);
		platform_renderer->programs[shader_i] = program;
	}

	// @Fixme(tkap, 20/10/2023): path
	game_renderer->checkmark_texture = load_texture(game_renderer, "assets/checkmark.png");

	load_font(game_renderer, "assets/consola.ttf", 128, arena);

	gl(glUseProgram(platform_renderer->programs[e_shader_default]));

	s_framebuffer framebuffer = {};
	framebuffer.do_depth = true;
	game_renderer->framebuffers.add(framebuffer);
	after_making_framebuffer(framebuffer.game_id, game_renderer);

}

static void add_int_attrib(s_attrib_handler* handler, int count)
{
	s_attrib attrib = {};
	attrib.type = GL_INT;
	attrib.size = sizeof(int);
	attrib.count = count;
	handler->attribs.add(attrib);
}

static void add_float_attrib(s_attrib_handler* handler, int count)
{
	s_attrib attrib = {};
	attrib.type = GL_FLOAT;
	attrib.size = sizeof(float);
	attrib.count = count;
	handler->attribs.add(attrib);
}

static void finish_attribs(s_attrib_handler* handler)
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

static void gl_render(s_platform_renderer* platform_renderer, s_game_renderer* game_renderer)
{
	gl(glUseProgram(platform_renderer->programs[e_shader_default]));

	{
		int location = gl(glGetUniformLocation(platform_renderer->programs[e_shader_default], "window_size"));
		s_v2 window_size = v2(g_window.width, g_window.height);
		gl(glUniform2fv(location, 1, &window_size.x));
	}
	{
		int location = gl(glGetUniformLocation(platform_renderer->programs[e_shader_default], "base_res"));
		gl(glUniform2fv(location, 1, &g_base_res.x));
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
					gl(glBufferData(GL_ARRAY_BUFFER, sizeof(s_transform) * platform_renderer->max_elements, NULL, GL_DYNAMIC_DRAW));
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

static u32 load_shader(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena)
{
	if(g_do_embed) {
		g_to_embed.add(vertex_path);
		g_to_embed.add(fragment_path);
	}

	#ifndef m_debug

	u32 program = load_shader_from_str((char*)embed_data[g_asset_index], (char*)embed_data[g_asset_index + 1]);
	g_asset_index += 2;
	return program;

	#else

	return load_shader_from_file(vertex_path, fragment_path, frame_arena);

	#endif

}

static u32 load_shader_from_file(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena)
{
	char* vertex_src = read_file(vertex_path, frame_arena);
	if(!vertex_src || !vertex_src[0]) { return 0; }
	char* fragment_src = read_file(fragment_path, frame_arena);
	if(!fragment_src || !fragment_src[0]) { return 0; }

	return load_shader_from_str(vertex_src, fragment_src);
}

static u32 load_shader_from_str(const char* vertex_src, const char* fragment_src)
{
	u32 vertex = glCreateShader(GL_VERTEX_SHADER);
	u32 fragment = glCreateShader(GL_FRAGMENT_SHADER);

	#ifdef __EMSCRIPTEN__
	const char* header = "#version 300 es\nprecision highp float;\n";
	#else
	const char* header = "#version 330 core\n";
	#endif

	const char* vertex_src_arr[] = {header, vertex_src};
	const char* fragment_src_arr[] = {header, fragment_src};
	gl(glShaderSource(vertex, array_count(vertex_src_arr), (const GLchar * const *)vertex_src_arr, NULL));
	gl(glShaderSource(fragment, array_count(fragment_src_arr), (const GLchar * const *)fragment_src_arr, NULL));
	gl(glCompileShader(vertex));
	char buffer[1024] = {};
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

static b8 check_for_shader_errors(u32 id, char* out_error)
{
	int compile_success;
	char info_log[1024];
	gl(glGetShaderiv(id, GL_COMPILE_STATUS, &compile_success));

	if(!compile_success) {
		gl(glGetShaderInfoLog(id, 1024, NULL, info_log));
		log("Failed to compile shader:\n%s", info_log);

		if(out_error)
		{
			strcpy(out_error, info_log);
		}

		return false;
	}
	return true;
}

static s_texture load_texture(s_game_renderer* game_renderer, const char* path)
{
	if(g_do_embed) {
		g_to_embed.add(path);
	}

	#ifndef m_debug

	int width, height, num_channels;
	void* data = stbi_load_from_memory(embed_data[g_asset_index], embed_sizes[g_asset_index], &width, &height, &num_channels, 4);
	s_texture result = load_texture_from_data(data, width, height, GL_LINEAR);
	g_asset_index += 1;

	#else

	s_texture result = load_texture_from_file(path, GL_LINEAR);
	#endif

	result.game_id = game_renderer->textures.count;
	result.path = path;
	game_renderer->textures.add(result);
	after_loading_texture(game_renderer);
	return result;
}

static s_texture load_texture_from_data(void* data, int width, int height, u32 filtering, int format)
{
	assert(data);
	u32 id;
	gl(glGenTextures(1, &id));
	gl(glBindTexture(GL_TEXTURE_2D, id));
	gl(glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, filtering));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, filtering));

	s_texture texture = {};
	texture.gpu_id = id;
	texture.size = v2(width, height);
	return texture;
}

static s_texture load_texture_from_file(const char* path, u32 filtering)
{
	int width, height, num_channels;
	void* data = stbi_load(path, &width, &height, &num_channels, 4);
	assert(data);
	s_texture texture = load_texture_from_data(data, width, height, filtering, GL_RGBA);
	stbi_image_free(data);
	return texture;
}

static void after_loading_texture(s_game_renderer* game_renderer)
{
	int old_index = game_renderer->transform_arena_index;
	int new_index = (game_renderer->transform_arena_index + 1) % 2;
	int size = sizeof(*game_renderer->framebuffers[0].transforms) * game_renderer->textures.count * e_blend_mode_count;

	foreach_ptr(framebuffer_i, framebuffer, game_renderer->framebuffers) {
		s_bucket_array<s_transform>* new_transforms = (s_bucket_array<s_transform>*)la_get_zero(
			&game_renderer->transform_arenas[new_index], size
		);

		// @Note(tkap, 08/10/2023): The first time we add a texture, transforms is NULL, so we can't memcpy from it
		if(framebuffer->transforms) {
			memcpy(new_transforms, framebuffer->transforms, size);
		}
		framebuffer->transforms = new_transforms;
	}

	game_renderer->transform_arenas[old_index].used = 0;
	game_renderer->transform_arena_index = new_index;
}

static s_framebuffer* make_framebuffer(s_game_renderer* game_renderer, b8 do_depth)
{
	// @Fixme(tkap, 11/10/2023): handle this
	assert(!do_depth);

	s_framebuffer result = {};

	gl(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	gl(glGenFramebuffers(1, &result.gpu_id));
	gl(glBindFramebuffer(GL_FRAMEBUFFER, result.gpu_id));

	gl(glGenTextures(1, &result.texture.gpu_id));
	gl(glBindTexture(GL_TEXTURE_2D, result.texture.gpu_id));
	gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, g_window.width, g_window.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
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

static void after_making_framebuffer(int index, s_game_renderer* game_renderer)
{
	int size = sizeof(*game_renderer->framebuffers[0].transforms) * game_renderer->textures.count * e_blend_mode_count;
	s_bucket_array<s_transform>* new_transforms = (s_bucket_array<s_transform>*)la_get_zero(
		&game_renderer->transform_arenas[game_renderer->transform_arena_index], size
	);
	s_framebuffer* framebuffer = &game_renderer->framebuffers[index];
	framebuffer->transforms = new_transforms;

	after_loading_texture(game_renderer);

}

static s_font* load_font(s_game_renderer* game_renderer, const char* path, int font_size, s_lin_arena* arena)
{
	if(g_do_embed) {
		g_to_embed.add(path);
	}

	#ifdef m_debug

	s_font font = load_font_from_file(path, font_size, arena);

	#else // m_debug

	s_font font = load_font_from_data(embed_data[g_asset_index], font_size, arena);
	g_asset_index += 1;

	#endif // m_debug

	font.texture.game_id = game_renderer->textures.count;
	game_renderer->textures.add(font.texture);
	after_loading_texture(game_renderer);
	int index = game_renderer->fonts.add(font);
	return &game_renderer->fonts[index];
}

static s_font load_font_from_file(const char* path, int font_size, s_lin_arena* arena)
{
	u8* file_data = (u8*)read_file(path, arena);
	return load_font_from_data(file_data, font_size, arena);
}

static s_font load_font_from_data(u8* file_data, int font_size, s_lin_arena* arena)
{
	s_font font = {};
	font.size = (float)font_size;

	assert(file_data);

	stbtt_fontinfo info = {};
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
	// @Note(tkap, 20/10/2023): We need to align the texture width to 4 bytes! Very important! Thanks to tk_dev
	total_width = (total_width + 3) & ~3;
	int total_height = floorfi((float)(rows * (font_size + padding)));

	for(int char_i = 0; char_i < max_chars; char_i++)
	{
		s_glyph glyph = {};
		u8* bitmap = stbtt_GetCodepointBitmap(&info, 0, font.scale, char_i, &glyph.width, &glyph.height, 0, 0);
		stbtt_GetCodepointBox(&info, char_i, &glyph.x0, &glyph.y0, &glyph.x1, &glyph.y1);
		stbtt_GetGlyphHMetrics(&info, char_i, &glyph.advance_width, NULL);

		font.glyph_arr[char_i] = glyph;
		bitmap_arr[bitmap_count++] = bitmap;
	}

	u8* gl_bitmap = (u8*)la_get_zero(arena, sizeof(u8) * 1 * total_width * total_height);

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
				u8* dst_pixel = &gl_bitmap[((current_x + x) + (current_y + y) * total_width)];
				dst_pixel[0] = src_pixel;
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
		stbtt_FreeBitmap(bitmap_arr[bitmap_i], NULL);
	}

	font.texture = load_texture_from_data(gl_bitmap, total_width, total_height, GL_LINEAR, m_gl_single_channel);

	return font;
}

static b8 set_shader_float(const char* uniform_name, float val)
{
	// @TODO(tkap, 18/10/2023): Change this when we support multiple shaders (if ever)
	int location = gl(glGetUniformLocation(g_platform_renderer.programs[e_shader_default], uniform_name));
	if(location < 0) { return false; }
	gl(glUniform1f(location, val));
	return true;
}

static b8 set_shader_v2(const char* uniform_name, s_v2 val)
{
	// @TODO(tkap, 18/10/2023): Change this when we support multiple shaders (if ever)
	int location = gl(glGetUniformLocation(g_platform_renderer.programs[e_shader_default], uniform_name));
	if(location < 0) { return false; }
	gl(glUniform2fv(location, 1, &val.x));
	return true;
}

#endif // m_game

// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		live variable start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
#ifdef m_game

#ifdef m_debug
#define live_variable(vars, var, min_val, max_val, display) live_variable_(vars, &var, #var, min_val, max_val, display)
#else // m_debug
#define live_variable(vars, var, min_val, max_val, display)
#endif // m_debug

#endif // m_game

#if defined(m_debug)

static s_var* get_var_by_ptr(s_sarray<s_var, 128>* vars, void* ptr);

template <typename t>
static void live_variable_(s_sarray<s_var, 128>* vars, t* ptr, const char* name, t min_val, t max_val, b8 display)
{
	constexpr b8 is_int = is_same<t, int>;
	constexpr b8 is_float = is_same<t, float>;
	constexpr b8 is_bool = is_same<t, b8>;
	static_assert(is_int || is_float || is_bool);
	s_var var = {};
	var.display = display;
	var.ptr = ptr;
	var.name = name;
	if constexpr(is_int) {
		var.type = e_var_type_int;
		var.min_val.val_int = min_val;
		var.max_val.val_int = max_val;
	}
	else if constexpr(is_float) {
		var.type = e_var_type_float;
		var.min_val.val_float = min_val;
		var.max_val.val_float = max_val;
	}
	else if constexpr(is_bool) {
		var.type = e_var_type_bool;
	}
	s_var* temp = get_var_by_ptr(vars, ptr);
	if(temp) {
		*temp = var;
	}
	else {
		vars->add(var);
	}
}

static s_var* get_var_by_ptr(s_sarray<s_var, 128>* vars, void* ptr)
{
	for(int var_i = 0; var_i < vars->count; var_i++)
	{
		s_var* var = vars->get_ptr(var_i);
		if(var->ptr == ptr) { return var; }
	}
	return NULL;
}

#endif
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		live variable end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		ui start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv

#ifndef m_game

static void reset_ui()
{
	g_ui.hovered.id = 0;
	if(!g_ui.pressed_present && g_ui.pressed.id != 0) {
		ui_request_pressed(&g_ui, 0);
	}
	g_ui.pressed_present = false;
}

#endif // m_game

// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		ui end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^