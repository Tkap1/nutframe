
#include <stdint.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <math.h>

#ifdef _WIN32
#define m_dll_export __declspec(dllexport)
#endif // _WIN32

#ifdef __GNUC__
// @TODO(tkap, 13/10/2023): stackoverflow copy paste
#define m_dll_export __attribute__((visibility("default")))
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

#define zero {}
#define func static
#define global static
#define null NULL
#define assert(cond) do { if(!(cond)) { on_failed_assert(#cond, __FILE__, __LINE__); } } while(0)
#define unreferenced(thing) (void)thing;
#define check(cond) do { if(!(cond)) { error(false); }} while(0)
#define invalid_default_case default: { assert(false); }
#define invalid_else else { assert(false); }
#define error(b) do { if(!(b)) { printf("ERROR\n"); exit(1); }} while(0)
#define array_count(arr) (sizeof((arr)) / sizeof((arr)[0]))
#define log(...) printf(__VA_ARGS__); printf("\n")
#define log_info(...) printf(__VA_ARGS__); printf("\n")

#define breakable_block__(a, b) for(int a##b = 1; a##b--;)
#define breakable_block_(a) breakable_block__(tkinternal_condblock, a)
#define breakable_block breakable_block_(__LINE__)

#define foreach_ptr__(a, index_name, element_name, array) if(0) finished##a: ; else for(auto element_name = &(array).elements[0];;) if(1) goto body##a; else while(1) if(1) goto finished##a; else body##a: for(int index_name = 0; index_name < (array).count && (bool)(element_name = &(array)[index_name]); index_name++)
#define foreach_ptr_(a, index_name, element_name, array) foreach_ptr__(a, index_name, element_name, array)
#define foreach_ptr(index_name, element_name, array) foreach_ptr_(__LINE__, index_name, element_name, array)

#define foreach_val__(a, index_name, element_name, array) if(0) finished##a: ; else for(auto element_name = (array).elements[0];;) if(1) goto body##a; else while(1) if(1) goto finished##a; else body##a: for(int index_name = 0; index_name < (array).count && (void*)&(element_name = (array)[index_name]); index_name++)
#define foreach_val_(a, index_name, element_name, array) foreach_val__(a, index_name, element_name, array)
#define foreach_val(index_name, element_name, array) foreach_val_(__LINE__, index_name, element_name, array)

global constexpr u64 c_max_u64 = UINT64_MAX;
global constexpr float c_max_f32 = 999999999.0f;

global constexpr s64 c_kb = 1024;
global constexpr s64 c_mb = 1024 * c_kb;
global constexpr s64 c_gb = 1024 * c_mb;
global constexpr s64 c_tb = 1024 * c_gb;

global constexpr float pi = 3.1415926f;
global constexpr float tau = 6.283185f;
global constexpr float epsilon = 0.000001f;

func void on_failed_assert(const char* cond, const char* file, int line);

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
	s_glyph glyph_arr[1024];
};


global constexpr int c_max_arena_push = 16;
struct s_lin_arena
{
	int push_count;
	u64 push[c_max_arena_push];
	u64 used;
	u64 capacity;
	void* memory;
};

func void* la_get(s_lin_arena* arena, u64 in_requested)
{
	assert(arena);
	assert(in_requested > 0);
	u64 requested = (in_requested + 7) & ~7;
	assert(arena->used + requested <= arena->capacity);
	void* result = (u8*)arena->memory + arena->used;
	arena->used += requested;
	return result;
}

func void* la_get_zero(s_lin_arena* arena, u64 in_requested)
{
	void* result = la_get(arena, in_requested);
	memset(result, 0, in_requested);
	return result;
}

func void la_push(s_lin_arena* arena)
{
	assert(arena->push_count < c_max_arena_push);
	arena->push[arena->push_count++] = arena->used;
}

func void la_pop(s_lin_arena* arena)
{
	assert(arena->push_count > 0);
	arena->used = arena->push[--arena->push_count];
}



#ifndef m_game

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
func s_font load_font_from_file(const char* path, int font_size, s_lin_arena* arena);
func s_font load_font_from_data(u8* file_data, int font_size, s_lin_arena* arena);


global constexpr int c_str_builder_size = 1 * c_gb;

struct s_str_builder
{
	int tab_count;
	int len;
	char data[c_str_builder_size];
};

func void builder_add_line(s_str_builder* builder, const char* what, ...);
func void builder_add(s_str_builder* builder, const char* what, ...);

func void builder_add_(s_str_builder* builder, const char* what, b8 use_tabs, va_list args)
{
	if(use_tabs)
	{
		for(int tab_i = 0; tab_i < builder->tab_count; tab_i++)
		{
			builder->data[builder->len++] = '\t';
		}
	}
	char* where_to_write = &builder->data[builder->len];
	int written = vsnprintf(where_to_write, c_str_builder_size + 1 - builder->len, what, args);
	assert(written > 0 && written < c_str_builder_size);
	builder->len += written;
	assert(builder->len < c_str_builder_size);
	builder->data[builder->len] = 0;
}

func void builder_add(s_str_builder* builder, const char* what, ...)
{
	va_list args;
	va_start(args, what);
	builder_add_(builder, what, false, args);
	va_end(args);
}

func void builder_add_char(s_str_builder* builder, char c)
{
	assert(builder->len < c_str_builder_size);
	builder->data[builder->len++] = c;
	builder->data[builder->len] = 0;
}

func void builder_add_with_tabs(s_str_builder* builder, const char* what, ...)
{
	va_list args;
	va_start(args, what);
	builder_add_(builder, what, true, args);
	va_end(args);
}

func void builder_add_line(s_str_builder* builder, const char* what, ...)
{
	va_list args;
	va_start(args, what);
	builder_add_(builder, what, false, args);
	va_end(args);
	builder_add(builder, "\n");
}

func void builder_add_line_with_tabs(s_str_builder* builder, const char* what, ...)
{
	va_list args;
	va_start(args, what);
	builder_add_(builder, what, true, args);
	va_end(args);
	builder_add(builder, "\n");
}

func void builder_add_tabs(s_str_builder* builder)
{
	for(int tab_i = 0; tab_i < builder->tab_count; tab_i++)
	{
		builder->data[builder->len++] = '\t';
	}
}

func void builder_line(s_str_builder* builder)
{
	builder_add(builder, "\n");
}

func void builder_push_tab(s_str_builder* builder)
{
	assert(builder->tab_count <= 64);
	builder->tab_count++;
}

func void builder_pop_tab(s_str_builder* builder)
{
	assert(builder->tab_count > 0);
	builder->tab_count--;
}

func char* read_file(const char* path, s_lin_arena* arena, u64* out_file_size = null)
{
	FILE* file = fopen(path, "rb");
	if(!file) { return null; }

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

func b8 write_file(const char* path, void* data, u64 size)
{
	assert(size > 0);
	FILE* file = fopen(path, "wb");
	if(!file) { return false; }

	fwrite(data, size, 1, file);
	fclose(file);
	return true;
}


#endif // m_game

template <typename t>
func t at_least(t a, t b)
{
	return a > b ? a : b;
}

template <typename t>
func t at_most(t a, t b)
{
	return b > a ? a : b;
}


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

		return min + (randu() % (max - min + 1));
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

		return min + (randu() % (max - min));
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


template <typename T>
func constexpr s_v2 v2(T x, T y)
{
	s_v2 result;
	result.x = (float)x;
	result.y = (float)y;
	return result;
}

func constexpr s_v2 v2(s_v2i v)
{
	return v2(v.x, v.y);
}

template <typename T>
func constexpr s_v2 v2(T v)
{
	s_v2 result;
	result.x = (float)v;
	result.y = (float)v;
	return result;
}

func constexpr s_v2i v2i(int x, int y)
{
	s_v2i result;
	result.x = x;
	result.y = y;
	return result;
}

func s_v4 v41f(float v)
{
	s_v4 result;
	result.x = v;
	result.y = v;
	result.z = v;
	result.w = v;
	return result;
}


func s_v2 operator+(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

func s_v2 operator+(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x + b;
	result.y = a.y + b;
	return result;
}

func s_v2 operator-(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	return result;
}

func s_v2 operator-(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x - b;
	result.y = a.y - b;
	return result;
}

func s_v2 operator*(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	return result;
}

func s_v2 operator*(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x * b;
	result.y = a.y * b;
	return result;
}

func s_v2i operator*(s_v2i a, int b)
{
	s_v2i result;
	result.x = a.x * b;
	result.y = a.y * b;
	return result;
}

func s_v2 operator/(s_v2 a, float b)
{
	s_v2 result;
	result.x = a.x / b;
	result.y = a.y / b;
	return result;
}

func s_v2 operator/(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	return result;
}

func void operator-=(s_v2& a, s_v2 b)
{
	a.x -= b.x;
	a.y -= b.y;
}

func void operator*=(s_v2& a, s_v2 b)
{
	a.x *= b.x;
	a.y *= b.y;
}

func void operator*=(s_v2& a, float b)
{
	a.x *= b;
	a.y *= b;
}

func s_v2 v2_rotate_around(s_v2 v, s_v2 pivot, float angle)
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

func void operator+=(s_v2& left, s_v2 right)
{
	left.x += right.x;
	left.y += right.y;
}

func void operator+=(s_v2i& left, s_v2i right)
{
	left.x += right.x;
	left.y += right.y;
}

[[nodiscard]]
func int circular_index(int index, int size)
{
	assert(size > 0);
	if(index >= 0)
	{
		return index % size;
	}
	return (size - 1) - ((-index - 1) % size);
}

func float v2_angle(s_v2 v)
{
	return atan2f(v.y, v.x);
}

func float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

func int roundfi(float x)
{
	return (int)roundf(x);
}

func float sinf2(float t)
{
	return sinf(t) * 0.5f + 0.5f;
}

func b8 operator==(s_v2i a, s_v2i b)
{
	return a.x == b.x && a.y == b.y;
}

template <typename t>
func constexpr s_v4 make_color(t v)
{
	s_v4 result;
	result.x = (float)v;
	result.y = (float)v;
	result.z = (float)v;
	result.w = 1;
	return result;
}

template <typename t0, typename t1, typename t2>
func constexpr s_v4 make_color(t0 r, t1 g, t2 b)
{
	s_v4 result;
	result.x = (float)r;
	result.y = (float)g;
	result.z = (float)b;
	result.w = 1;
	return result;
}

func s_v2 v2_from_angle(float angle)
{
	return v2(
		cosf(angle),
		sinf(angle)
	);
}

func int double_until_greater_or_equal(int current, int target)
{
	assert(target > 0);
	if(current <= 0) { current = 1; }
	while(current < target) { current *= 2; }
	return current;
}

func int floorfi(float x)
{
	return (int)floorf(x);
}

func int ceilfi(float x)
{
	return (int)ceilf(x);
}

func float fract(float x)
{
	return x - (int)x;
}

func float v2_length(s_v2 a)
{
	return sqrtf(a.x * a.x + a.y * a.y);
}

func float v2_distance(s_v2 a, s_v2 b)
{
	return v2_length(a - b);
}


func s_v2 lerp(s_v2 a, s_v2 b, float t)
{
	s_v2 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	return result;
}

func s_v2 lerp_snap(s_v2 a, s_v2 b, float t)
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

func s_v4 lerp(s_v4 a, s_v4 b, float t)
{
	s_v4 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	result.z = lerp(a.z, b.z, t);
	result.w = lerp(a.w, b.w, t);
	return result;
}

func s_v2 v2_normalized(s_v2 v)
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

func float range_lerp(float input_val, float input_start, float input_end, float output_start, float output_end)
{
	return output_start + ((output_end - output_start) / (input_end - input_start)) * (input_val - input_start);
}

func int str_find_from_left(const char* haystack, int haystack_len, const char* needle, int needle_len)
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

func b8 str_replace(char* str, const char* needle, const char* replacement)
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


global constexpr int c_key_backspace = 0x08;
global constexpr int c_key_tab = 0x09;
global constexpr int c_key_enter = 0x0D;
global constexpr int c_key_alt = 0x12;
global constexpr int c_key_left_alt = 0xA4;
global constexpr int c_key_right_alt = 0xA5;
global constexpr int c_key_escape = 0x1B;
global constexpr int c_key_space = 0x20;
global constexpr int c_key_end = 0x23;
global constexpr int c_key_home = 0x24;
global constexpr int c_key_left = 0x25;
global constexpr int c_key_up = 0x26;
global constexpr int c_key_right = 0x27;
global constexpr int c_key_down = 0x28;
global constexpr int c_key_delete = 0x2E;
global constexpr int c_key_0 = 0x30;
global constexpr int c_key_1 = 0x31;
global constexpr int c_key_2 = 0x32;
global constexpr int c_key_3 = 0x33;
global constexpr int c_key_4 = 0x34;
global constexpr int c_key_5 = 0x35;
global constexpr int c_key_6 = 0x36;
global constexpr int c_key_7 = 0x37;
global constexpr int c_key_8 = 0x38;
global constexpr int c_key_9 = 0x39;
global constexpr int c_key_a = 0x41;
global constexpr int c_key_b = 0x42;
global constexpr int c_key_c = 0x43;
global constexpr int c_key_d = 0x44;
global constexpr int c_key_e = 0x45;
global constexpr int c_key_f = 0x46;
global constexpr int c_key_g = 0x47;
global constexpr int c_key_h = 0x48;
global constexpr int c_key_i = 0x49;
global constexpr int c_key_j = 0x4A;
global constexpr int c_key_k = 0x4B;
global constexpr int c_key_l = 0x4C;
global constexpr int c_key_m = 0x4D;
global constexpr int c_key_n = 0x4E;
global constexpr int c_key_o = 0x4F;
global constexpr int c_key_p = 0x50;
global constexpr int c_key_q = 0x51;
global constexpr int c_key_r = 0x52;
global constexpr int c_key_s = 0x53;
global constexpr int c_key_t = 0x54;
global constexpr int c_key_u = 0x55;
global constexpr int c_key_v = 0x56;
global constexpr int c_key_w = 0x57;
global constexpr int c_key_x = 0x58;
global constexpr int c_key_y = 0x59;
global constexpr int c_key_z = 0x5A;
global constexpr int c_key_add = 0x6B;
global constexpr int c_key_subtract = 0x6D;
global constexpr int c_key_f1 = 0x70;
global constexpr int c_key_f2 = 0x71;
global constexpr int c_key_f3 = 0x72;
global constexpr int c_key_f4 = 0x73;
global constexpr int c_key_f5 = 0x74;
global constexpr int c_key_f6 = 0x75;
global constexpr int c_key_f7 = 0x76;
global constexpr int c_key_f8 = 0x77;
global constexpr int c_key_f9 = 0x78;
global constexpr int c_key_f10 = 0x79;
global constexpr int c_key_f11 = 0x7A;
global constexpr int c_key_f12 = 0x7B;
global constexpr int c_key_left_shift = 0xA0;
global constexpr int c_key_right_shift = 0xA1;
global constexpr int c_key_left_ctrl = 0xA2;
global constexpr int c_key_right_ctrl = 0xA3;
global constexpr int c_left_mouse = 1020;
global constexpr int c_right_mouse = 1021;
global constexpr int c_max_keys = 1024;

global constexpr int c_game_memory = 1 * c_mb;

global constexpr s_v2 c_origin_topleft = {1.0f, -1.0f};
global constexpr s_v2 c_origin_bottomleft = {1.0f, 1.0f};
global constexpr s_v2 c_origin_center = {0, 0};

global constexpr s_v2 c_base_res = {64*12, 64*12};
global constexpr s_v2 c_half_res = {c_base_res.x / 2.0f, c_base_res.y / 2.0f};

global constexpr int c_base_resolution_index = 5;
global constexpr s_v2i c_resolutions[] = {
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

// @Note(tkap, 08/10/2023): We have a bug with this. If we ever go from having never drawn anything to drawing 64*16+1 things we will
// exceed the max bucket count (16 currently). To fix this, I guess we have to allow merging in the middle of a frame?? Seems messy...
global constexpr int c_bucket_capacity = 64;

template <typename t>
struct s_bucket_array
{
	int bucket_count;
	int capacity[16];
	int element_count[16];
	t* elements[16];
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
	s_key keys[c_max_keys];
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

struct s_platform_data
{
	b8 recompiled;
	b8 quit_after_this_frame;
	b8 any_key_pressed;
	b8 is_window_active;
	b8 window_resized;
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
};

typedef s_texture (*t_load_texture)(s_game_renderer*, const char*);
typedef s_font* (*t_load_font)(s_game_renderer*, const char*, int, s_lin_arena*);
struct s_game_renderer
{
	b8 did_we_alloc;
	t_set_vsync set_vsync;
	t_load_texture load_texture;
	t_load_font load_font;
	t_make_framebuffer make_framebuffer;
	f64 total_time;

	int transform_arena_index;
	s_lin_arena transform_arenas[2];
	int arena_index;
	s_lin_arena arenas[2];
	s_sarray<s_texture, 16> textures;
	s_sarray<s_framebuffer, 4> framebuffers;
	s_sarray<s_font, 4> fonts;
};

#define m_update_game(name) void name(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer)
#ifdef m_build_dll
typedef m_update_game(t_update_game);
#else // m_build_dll
m_update_game(update_game);
#endif


func int get_render_offset(int texture, int blend_mode);
func s_v2 get_text_size_with_count(const char* text, s_font* font, float font_size, int count);
func s_v2 get_text_size(const char* text, s_font* font, float font_size);
template <typename t>
func void bucket_add(s_bucket_array<t>* arr, t new_element, s_lin_arena* arena, b8* did_we_alloc);
template <typename t>
func void bucket_merge(s_bucket_array<t>* arr, s_lin_arena* arena);




// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------
// ------------------------------------------------------------------------------------

func void apply_event_to_input(s_input* in_input, s_stored_input event)
{
	in_input->keys[event.key].is_down = event.is_down;
	in_input->keys[event.key].count += 1;
}

func b8 is_key_down(s_input* input, int key) {
	assert(key < c_max_keys);
	return input->keys[key].is_down || input->keys[key].count >= 2;
}

func b8 is_key_up(s_input* input, int key) {
	assert(key < c_max_keys);
	return !input->keys[key].is_down;
}

func b8 is_key_pressed(s_input* input, int key) {
	assert(key < c_max_keys);
	return (input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

func b8 is_key_released(s_input* input, int key) {
	assert(key < c_max_keys);
	return (!input->keys[key].is_down && input->keys[key].count == 1) || input->keys[key].count > 1;
}

func int get_render_offset(int texture, int blend_mode)
{
	return texture * e_blend_mode_count + blend_mode;
}

// @Note(tkap, 15/10/2023): Should this always return font_size for y (like it does now), or actually get the tallest char?
func s_v2 get_text_size_with_count(const char* text, s_font* font, float font_size, int count)
{
	assert(count >= 0);
	if(count <= 0) { return zero; }

	s_v2 size = zero;
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

func s_v2 get_text_size(const char* text, s_font* font, float font_size)
{
	return get_text_size_with_count(text, font, font_size, (int)strlen(text));
}

func void draw_rect(s_game_renderer* game_renderer, s_v2 pos, int layer, s_v2 size, s_v4 color, s_render_data render_data = zero, s_transform t = zero)
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

func void draw_texture(s_game_renderer* game_renderer, s_v2 pos, int layer, s_v2 size, s_v4 color, s_texture texture, s_render_data render_data = zero, s_transform t = zero)
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

func void draw_text(s_game_renderer* game_renderer, const char* text, s_v2 in_pos, int layer, float font_size, s_v4 color, b8 centered, s_font* font, s_render_data render_data = zero, s_transform t = zero)
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
			&game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc
		);

		pos.x += glyph.advance_width * scale;

	}
}

func s_lin_arena make_lin_arena(u64 capacity)
{
	assert(capacity > 0);
	capacity = (capacity + 7) & ~7;
	s_lin_arena result = zero;
	result.capacity = capacity;
	result.memory = malloc(capacity);
	return result;
}

func s_lin_arena make_lin_arena_from_memory(u64 capacity, void* memory)
{
	assert(capacity > 0);
	assert(memory);

	capacity = (capacity + 7) & ~7;
	s_lin_arena result = zero;
	result.capacity = capacity;
	result.memory = memory;
	return result;
}


template <typename t>
func void bucket_add(s_bucket_array<t>* arr, t new_element, s_lin_arena* arena, b8* did_we_alloc)
{
	for(int i = 0; i < arr->bucket_count; i++)
	{
		int* count = &arr->element_count[i];
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
func void bucket_merge(s_bucket_array<t>* arr, s_lin_arena* arena)
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

func void on_failed_assert(const char* cond, const char* file, int line)
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
	// int result = MessageBox(null, text, "Assertion failed", MB_RETRYCANCEL | MB_TOPMOST);
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