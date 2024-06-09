
#define m_multisample 0

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

typedef bool b8;
typedef uint32_t b32;

typedef double f64;

#ifdef __EMSCRIPTEN__
#define m_gl_single_channel GL_LUMINANCE
#else // __EMSCRIPTEN__
#define m_gl_single_channel GL_RED
#endif // __EMSCRIPTEN__

#ifndef m_game

#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#define NOMINMAX
#include <windows.h>
#endif // _WIN32

#if defined(__EMSCRIPTEN__)
#include <gl/GL.h>
#include "external/glcorearb.h"
#elif defined(m_sdl)
#include <gl/GL.h>
#include "external/glcorearb.h"
#else
#include <gl/GL.h>
#include "external/glcorearb.h"
#include "external/wglext.h"
#endif

#ifdef __EMSCRIPTEN__
#define m_gl_funcs \
X(PFNGLBUFFERDATAPROC, glBufferData) \
X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
X(PFNGLGENBUFFERSPROC, glGenBuffers) \
X(PFNGLBINDBUFFERPROC, glBindBuffer) \
X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer) \
X(PFNGLVERTEXATTRIBIPOINTERPROC, glVertexAttribIPointer) \
X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
X(PFNGLCREATESHADERPROC, glCreateShader) \
X(PFNGLSHADERSOURCEPROC, glShaderSource) \
X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
X(PFNGLATTACHSHADERPROC, glAttachShader) \
X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
X(PFNGLCOMPILESHADERPROC, glCompileShader) \
X(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor) \
X(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced) \
X(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced) \
X(PFNGLUNIFORM1FVPROC, glUniform1fv) \
X(PFNGLUNIFORM2FVPROC, glUniform2fv) \
X(PFNGLUNIFORM3FVPROC, glUniform3fv) \
X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
X(PFNGLUSEPROGRAMPROC, glUseProgram) \
X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
X(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) \
X(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) \
X(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) \
X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) \
X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
X(PFNGLDELETESHADERPROC, glDeleteShader) \
X(PFNGLUNIFORM1IPROC, glUniform1i) \
X(PFNGLUNIFORM1FPROC, glUniform1f) \
X(PFNGLDETACHSHADERPROC, glDetachShader) \
X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
X(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers) \
X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv) \

#else // __EMSCRIPTEN__

#define m_gl_funcs \
X(PFNGLBUFFERDATAPROC, glBufferData) \
X(PFNGLBUFFERSUBDATAPROC, glBufferSubData) \
X(PFNGLGENVERTEXARRAYSPROC, glGenVertexArrays) \
X(PFNGLBINDVERTEXARRAYPROC, glBindVertexArray) \
X(PFNGLGENBUFFERSPROC, glGenBuffers) \
X(PFNGLBINDBUFFERPROC, glBindBuffer) \
X(PFNGLVERTEXATTRIBPOINTERPROC, glVertexAttribPointer) \
X(PFNGLVERTEXATTRIBIPOINTERPROC, glVertexAttribIPointer) \
X(PFNGLENABLEVERTEXATTRIBARRAYPROC, glEnableVertexAttribArray) \
X(PFNGLCREATESHADERPROC, glCreateShader) \
X(PFNGLSHADERSOURCEPROC, glShaderSource) \
X(PFNGLCREATEPROGRAMPROC, glCreateProgram) \
X(PFNGLATTACHSHADERPROC, glAttachShader) \
X(PFNGLLINKPROGRAMPROC, glLinkProgram) \
X(PFNGLCOMPILESHADERPROC, glCompileShader) \
X(PFNGLVERTEXATTRIBDIVISORPROC, glVertexAttribDivisor) \
X(PFNGLDRAWARRAYSINSTANCEDPROC, glDrawArraysInstanced) \
X(PFNGLDRAWELEMENTSINSTANCEDPROC, glDrawElementsInstanced) \
X(PFNGLUNIFORM1FVPROC, glUniform1fv) \
X(PFNGLUNIFORM2FVPROC, glUniform2fv) \
X(PFNGLUNIFORM3FVPROC, glUniform3fv) \
X(PFNGLGETUNIFORMLOCATIONPROC, glGetUniformLocation) \
X(PFNGLUSEPROGRAMPROC, glUseProgram) \
X(PFNGLGETSHADERIVPROC, glGetShaderiv) \
X(PFNGLGETSHADERINFOLOGPROC, glGetShaderInfoLog) \
X(PFNGLGENFRAMEBUFFERSPROC, glGenFramebuffers) \
X(PFNGLBINDFRAMEBUFFERPROC, glBindFramebuffer) \
X(PFNGLFRAMEBUFFERTEXTURE2DPROC, glFramebufferTexture2D) \
X(PFNGLCHECKFRAMEBUFFERSTATUSPROC, glCheckFramebufferStatus) \
X(PFNGLACTIVETEXTUREPROC, glActiveTexture) \
X(PFNGLDELETEPROGRAMPROC, glDeleteProgram) \
X(PFNGLDELETESHADERPROC, glDeleteShader) \
X(PFNGLUNIFORM1IPROC, glUniform1i) \
X(PFNGLUNIFORM1FPROC, glUniform1f) \
X(PFNGLDETACHSHADERPROC, glDetachShader) \
X(PFNGLGETPROGRAMIVPROC, glGetProgramiv) \
X(PFNGLGETPROGRAMINFOLOGPROC, glGetProgramInfoLog) \
X(PFNGLDELETEFRAMEBUFFERSPROC, glDeleteFramebuffers) \
X(PFNGLUNIFORMMATRIX4FVPROC, glUniformMatrix4fv) \

#endif // __EMSCRIPTEN__

#define X(type, name) static type name = NULL;
m_gl_funcs
#undef X



#endif // m_game

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
static constexpr f64 epsilon64 = 0.000000001;

#define rad2deg (pi * 180)
#define deg2rad (pi / 180)

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
	union
	{
		struct
		{
			float x;
			float y;
		};
		s_v2 xy;
	};
	float z;
};

struct s_v4
{
	union
	{
		struct
		{
			float x;
			float y;
			float z;
		};
		s_v3 xyz;
	};
	float w;
};

union s_m4
{
	float all[16];
	float elements[4][4];
	s_v4 v[4];
};

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

	float randf32_11()
	{
		return randf32() * 2 - 1;
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

template <typename T, int N>
struct s_sarray
{
	static_assert(N > 0);
	int count = 0;
	T elements[N];

	constexpr void shuffle(s_rng* rng)
	{
		assert(count > 0);
		for(int i = 0; i < count; i++) {
			swap(i, rng->rand_range_ie(i, count));
		}
	}

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
		T temp = elements[index0];
		elements[index0] = elements[index1];
		elements[index1] = temp;
	}

	constexpr T& get_last()
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

	// @TODO(tkap, 15/11/2023): Currently, if the array if full and you call this, the last element will get removed.
	// Not sure if that is what we want...
	constexpr void insert(int index, T element)
	{
		assert(index >= 0);
		assert(index < N);
		assert(index <= count);

		if(count >= N) {
			count -= 1;
		}

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

	void small_sort(bool (*compare_func)(T, T) = NULL, b8 reverse = false)
	{
		// @Note(tkap, 25/06/2023): Let's not get crazy with insertion sort, bro
		assert(count < 256);

		for(int i = 1; i < count; i++)
		{
			for(int j = i; j > 0; j--)
			{
				T* a = &elements[j];
				T* b = &elements[j - 1];

				if(compare_func) {
					if(reverse) {
						if(!compare_func(*a, *b)) { break; }
					}
					else {
						if(compare_func(*a, *b)) { break; }
					}
				}
				else {
					if(reverse) {
						if(*a < *b) { break; }
					}
					else {
						if(*a > *b) { break; }
					}
				}
				T temp = *a;
				*a = *b;
				*b = temp;
			}
		}
	}
};

static constexpr int c_max_arena_push = 16;
struct s_lin_arena
{
	s_sarray<u64, c_max_arena_push> push;
	u64 used;
	u64 capacity;
	void* memory;
};

struct s_len_str
{
	char* str;
	int len;

	char operator[](int index)
	{
		assert(index >= 0);
		return str[index];
	}
};


template <typename t>
struct s_maybe
{
	b8 valid;
	t value;
};

template <typename t>
static s_maybe<t> maybe(t value)
{
	return {.valid = true, .value = value};
}

template <typename t>
static s_maybe<t> maybe()
{
	return {};
}

template <int n>
struct s_str
{
	int len;
	char data[n];

	char& operator[](int i)
	{
		assert(i >= 0);
		assert(i <= len);
		return data[i];
	}

	void null_terminate()
	{
		assert(len >= 0);
		assert(len < n);
		data[len] = 0;
	}

	void from_cstr(char* str)
	{
		int in_len = (int)strlen(str);
		assert(in_len < n);
		memcpy(data, str, in_len);
		data[in_len] = 0;
		len = in_len;
	}

	void from_data(char* str, int in_len)
	{
		assert(in_len < n);
		memcpy(data, str, in_len);
		len = in_len;
		data[in_len] = 0;
	}

	b8 is_full() { return len >= n - 1; }

	void insert(int index, char c)
	{
		assert(index >= 0);
		assert(index < n);
		assert(index <= len);
		assert(!is_full());

		memmove(&data[index + 1], &data[index], len - index);
		data[index] = c;
		len += 1;
		data[len] = 0;
	}

	void remove_char_at(int index)
	{
		assert(index >= 0);
		assert(index < len);

		len -= 1;
		memmove(&data[index], &data[index + 1], len - index);
		data[len] = 0;
	}

};

template <int n>
struct s_input_str
{
	b8 visual_pos_initialized;
	s_v2 cursor_visual_pos;
	float last_edit_time;
	float last_action_time;
	s_maybe<int> cursor;
	s_str<n> str;

	int get_max_chars()
	{
		return n - 1;
	}
};

#define m_strlit(s) {.str = s, .len = sizeof(s) - 1}

static s_len_str strlit(char* s)
{
	return {.str = s, .len = (int)strlen(s)};
}

[[nodiscard]] static s_len_str get_substr_from(s_len_str x, s_len_str needle)
{
	char* match = strstr(x.str, needle.str);
	if(match) {
		return {.str = match, .len = (int)(match - x.str)};
	}
	else {
		return {};
	}
}

[[nodiscard]] static s_len_str substr_from_to_exclusive(s_len_str x, int start, int end)
{
	assert(start >= 0);
	assert(end > start);
	return {.str = x.str + start, .len = end - start};
}

[[nodiscard]] static s_len_str advance(s_len_str x, int n)
{
	assert(n > 0);
	assert(x.len - n >= 0);
	return {.str = x.str + n, .len = x.len - n};
}

struct s_recti
{
	union
	{
		struct
		{
			int x;
			int y;
			int width;
			int height;
		};
		struct
		{
			int x0;
			int y0;
			int x1;
			int y1;
		};
	};
};

struct s_string_parse
{
	b8 success;
	char* first_char;
	char* last_char;
	char* continuation;
	char* result;
	int len;
};


enum e_json
{
	e_json_object,
	e_json_integer,
	e_json_string,
	e_json_bool,
	e_json_array,
	e_json_null,
};

struct s_json
{
	e_json type;
	char* key;
	s_json* next;

	union
	{
		b8 bool_val;
		s_json* object;
		s_json* array;
		int integer;
		char* str;
	};
};

enum e_input_modifier
{
	e_input_modifier_none = 0,
	e_input_modifier_ctrl = 1 << 1,
};


#ifndef m_game
#ifndef m_debug
#include "embed.h"
#endif // m_debug
#endif // m_game


struct s_ray
{
	s_v3 pos;
	s_v3 dir;
};

struct s_ray_collision
{
	b8 hit;
	float distance;
	s_v3 normal;
	s_v3 point;
};

enum e_blend_mode
{
	e_blend_mode_disable,
	e_blend_mode_normal,
	e_blend_mode_additive,
	e_blend_mode_multiply,
	e_blend_mode_count,
};

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

	void swap(int a, int b)
	{
		assert(a >= 0 && a < n);
		assert(b >= 0 && b < n);
		t temp = elements[a];
		elements[a] = elements[b];
		elements[b] = temp;
	}

	constexpr int max_elements()
	{
		return n;
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

	void clear()
	{
		memset(elements, 0, sizeof(elements));
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

struct s_camera3d
{
	s_v3 pos;
	s_v3 target;
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
static u32 hash(u32 value)
{
	return value;
}

struct s_parse_ui_id
{
	u32 id;
	s_len_str text;
};

[[nodiscard]]
static s_parse_ui_id parse_ui_id(s_len_str str)
{
	s_parse_ui_id result = {};
	s_len_str id_part = get_substr_from(str, strlit("##"));
	if(id_part.len > 2) {
		result.text = substr_from_to_exclusive(str, 0, (int)(id_part.str - str.str));
		id_part = advance(id_part, 2);
		result.id = hash(id_part.str);
	}
	else {
		result.text = str;
		result.id = hash(str.str);
	}
	return result;
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

static b8 floats_equal(float a, float b)
{
	return (a >= b - epsilon && a <= b + epsilon);
}

static b8 floats64_equal(f64 a, f64 b)
{
	return (a >= b - epsilon64 && a <= b + epsilon64);
}

static b8 is_zero(float x)
{
	return floats_equal(x, 0);
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
	return {.x = v3.x, .y = v3.y, .z = v3.z, .w = a};
}

static s_v4 v4(float x, float y, float z, float w)
{
	return {.x = x, .y = y, .z = z, .w = w};
}

static constexpr s_v3 v3(float x, float y, float z)
{
	s_v3 result;
	result.x = x;
	result.y = y;
	result.z = z;
	return result;
}

static constexpr s_v3 v3(float x)
{
	s_v3 result;
	result.x = x;
	result.y = x;
	result.z = x;
	return result;
}

static constexpr s_v3 v3(s_v2 v, float z)
{
	s_v3 result;
	result.x = v.x;
	result.y = v.y;
	result.z = z;
	return result;
}

static constexpr s_v3 v3(s_v4 v)
{
	s_v3 result;
	result.x = v.x;
	result.y = v.y;
	result.z = v.z;
	return result;
}

static float v3_dot(s_v3 a, s_v3 b)
{
	float Result = (a.x * b.x) + (a.y * b.y) + (a.z * b.z);
	return (Result);
}

static s_m4 m4_identity()
{
	return {{
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1,
	}};
}

static s_m4 m4_inverse(const s_m4 m)
{
	s_m4 result = {};
	float det;
	int i;

	result.all[0] = m.all[5]  * m.all[10] * m.all[15] -
				m.all[5]  * m.all[11] * m.all[14] -
				m.all[9]  * m.all[6]  * m.all[15] +
				m.all[9]  * m.all[7]  * m.all[14] +
				m.all[13] * m.all[6]  * m.all[11] -
				m.all[13] * m.all[7]  * m.all[10];

	result.all[4] = -m.all[4]  * m.all[10] * m.all[15] +
				m.all[4]  * m.all[11] * m.all[14] +
				m.all[8]  * m.all[6]  * m.all[15] -
				m.all[8]  * m.all[7]  * m.all[14] -
				m.all[12] * m.all[6]  * m.all[11] +
				m.all[12] * m.all[7]  * m.all[10];

	result.all[8] = m.all[4]  * m.all[9] * m.all[15] -
				m.all[4]  * m.all[11] * m.all[13] -
				m.all[8]  * m.all[5] * m.all[15] +
				m.all[8]  * m.all[7] * m.all[13] +
				m.all[12] * m.all[5] * m.all[11] -
				m.all[12] * m.all[7] * m.all[9];

	result.all[12] = -m.all[4]  * m.all[9] * m.all[14] +
					m.all[4]  * m.all[10] * m.all[13] +
					m.all[8]  * m.all[5] * m.all[14] -
					m.all[8]  * m.all[6] * m.all[13] -
					m.all[12] * m.all[5] * m.all[10] +
					m.all[12] * m.all[6] * m.all[9];

	result.all[1] = -m.all[1]  * m.all[10] * m.all[15] +
				m.all[1]  * m.all[11] * m.all[14] +
				m.all[9]  * m.all[2] * m.all[15] -
				m.all[9]  * m.all[3] * m.all[14] -
				m.all[13] * m.all[2] * m.all[11] +
				m.all[13] * m.all[3] * m.all[10];

	result.all[5] = m.all[0]  * m.all[10] * m.all[15] -
				m.all[0]  * m.all[11] * m.all[14] -
				m.all[8]  * m.all[2] * m.all[15] +
				m.all[8]  * m.all[3] * m.all[14] +
				m.all[12] * m.all[2] * m.all[11] -
				m.all[12] * m.all[3] * m.all[10];

	result.all[9] = -m.all[0]  * m.all[9] * m.all[15] +
				m.all[0]  * m.all[11] * m.all[13] +
				m.all[8]  * m.all[1] * m.all[15] -
				m.all[8]  * m.all[3] * m.all[13] -
				m.all[12] * m.all[1] * m.all[11] +
				m.all[12] * m.all[3] * m.all[9];

	result.all[13] = m.all[0]  * m.all[9] * m.all[14] -
				m.all[0]  * m.all[10] * m.all[13] -
				m.all[8]  * m.all[1] * m.all[14] +
				m.all[8]  * m.all[2] * m.all[13] +
				m.all[12] * m.all[1] * m.all[10] -
				m.all[12] * m.all[2] * m.all[9];

	result.all[2] = m.all[1]  * m.all[6] * m.all[15] -
				m.all[1]  * m.all[7] * m.all[14] -
				m.all[5]  * m.all[2] * m.all[15] +
				m.all[5]  * m.all[3] * m.all[14] +
				m.all[13] * m.all[2] * m.all[7] -
				m.all[13] * m.all[3] * m.all[6];

	result.all[6] = -m.all[0]  * m.all[6] * m.all[15] +
				m.all[0]  * m.all[7] * m.all[14] +
				m.all[4]  * m.all[2] * m.all[15] -
				m.all[4]  * m.all[3] * m.all[14] -
				m.all[12] * m.all[2] * m.all[7] +
				m.all[12] * m.all[3] * m.all[6];

	result.all[10] = m.all[0]  * m.all[5] * m.all[15] -
				m.all[0]  * m.all[7] * m.all[13] -
				m.all[4]  * m.all[1] * m.all[15] +
				m.all[4]  * m.all[3] * m.all[13] +
				m.all[12] * m.all[1] * m.all[7] -
				m.all[12] * m.all[3] * m.all[5];

	result.all[14] = -m.all[0]  * m.all[5] * m.all[14] +
					m.all[0]  * m.all[6] * m.all[13] +
					m.all[4]  * m.all[1] * m.all[14] -
					m.all[4]  * m.all[2] * m.all[13] -
					m.all[12] * m.all[1] * m.all[6] +
					m.all[12] * m.all[2] * m.all[5];

	result.all[3] = -m.all[1] * m.all[6] * m.all[11] +
				m.all[1] * m.all[7] * m.all[10] +
				m.all[5] * m.all[2] * m.all[11] -
				m.all[5] * m.all[3] * m.all[10] -
				m.all[9] * m.all[2] * m.all[7] +
				m.all[9] * m.all[3] * m.all[6];

	result.all[7] = m.all[0] * m.all[6] * m.all[11] -
				m.all[0] * m.all[7] * m.all[10] -
				m.all[4] * m.all[2] * m.all[11] +
				m.all[4] * m.all[3] * m.all[10] +
				m.all[8] * m.all[2] * m.all[7] -
				m.all[8] * m.all[3] * m.all[6];

	result.all[11] = -m.all[0] * m.all[5] * m.all[11] +
					m.all[0] * m.all[7] * m.all[9] +
					m.all[4] * m.all[1] * m.all[11] -
					m.all[4] * m.all[3] * m.all[9] -
					m.all[8] * m.all[1] * m.all[7] +
					m.all[8] * m.all[3] * m.all[5];

	result.all[15] = m.all[0] * m.all[5] * m.all[10] -
				m.all[0] * m.all[6] * m.all[9] -
				m.all[4] * m.all[1] * m.all[10] +
				m.all[4] * m.all[2] * m.all[9] +
				m.all[8] * m.all[1] * m.all[6] -
				m.all[8] * m.all[2] * m.all[5];

	det = m.all[0] * result.all[0] + m.all[1] * result.all[4] + m.all[2] * result.all[8] + m.all[3] * result.all[12];

	if(floats_equal(det, 0))
		return result;

	det = 1.0f / det;

	for (i = 0; i < 16; i++)
		result.all[i] = result.all[i] * det;

	return result;
}


static s_m4 m4_translate(s_v3 v)
{
	return {{
		1,0,0,0,
		0,1,0,0,
		0,0,1,0,
		v.x,v.y,v.z,1
	}};
}

static s_m4 m4_scale(s_v3 scale)
{
	return {{
		scale.x,	0,			0,			0,
		0,			scale.y,	0,			0,
		0,			0,			scale.z,	0,
		0,			0,			0,			1
	}};
}

static s_m4 m4_perspective(float FOV, float AspectRatio, float Near, float Far)
{
	s_m4 Result = {0};

	// See https://www.khronos.org/registry/OpenGL-Refpages/gl2.1/xhtml/gluPerspective.xml

	float Cotangent = 1.0f / tanf((FOV * deg2rad) / 2.0f);
	Result.elements[0][0] = Cotangent / AspectRatio;
	Result.elements[1][1] = Cotangent;
	Result.elements[2][3] = -1.0f;

	Result.elements[2][2] = (Near + Far) / (Near - Far);
	Result.elements[3][2] = (2.0f * Near * Far) / (Near - Far);

	return Result;
}

static s_m4 m4_orthographic(float Left, float Right, float Bottom, float Top, float Near, float Far)
{
	s_m4 Result = {};

	Result.elements[0][0] = 2.0f / (Right - Left);
	Result.elements[1][1] = 2.0f / (Top - Bottom);
	Result.elements[2][2] = 2.0f / (Near - Far);
	Result.elements[3][3] = 1.0f;

	Result.elements[3][0] = (Left + Right) / (Left - Right);
	Result.elements[3][1] = (Bottom + Top) / (Bottom - Top);
	Result.elements[3][2] = (Near + Far) / (Near - Far);

	return (Result);
}

static float lerp(float a, float b, float t)
{
	return a + (b - a) * t;
}

static s_v2 lerp(s_v2 a, s_v2 b, float t)
{
	s_v2 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	return result;
}

static s_v3 lerp(s_v3 a, s_v3 b, float t)
{
	s_v3 result;
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	result.z = lerp(a.z, b.z, t);
	return result;
}


static float lerp_snap(float a, float b, float t, float max_diff)
{
	float result = a + (b - a) * t;
	if(fabsf(result - b) < max_diff) {
		result = b;
	}
	return result;
}

static s_m4 lerp_m4(s_m4 a, s_m4 b, float t)
{
	s_m4 result;
	for(int i = 0; i < 16; i++) {
		result.all[i] = lerp(a.all[i], b.all[i], t);
	}
	return result;
}

static s_v3 v3_cross(s_v3 a, s_v3 b)
{
	s_v3 Result;

	Result.x = (a.y * b.z) - (a.z * b.y);
	Result.y = (a.z * b.x) - (a.x * b.z);
	Result.z = (a.x * b.y) - (a.y * b.x);

	return (Result);
}


static s_v3 operator-(s_v3 a, s_v3 b)
{
	s_v3 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
	result.z = a.z - b.z;
	return result;
}


static s_m4 look_at_internal(s_v3 F, s_v3 S, s_v3 U, s_v3 Eye)
{
	s_m4 Result;

	Result.elements[0][0] = S.x;
	Result.elements[0][1] = U.x;
	Result.elements[0][2] = -F.x;
	Result.elements[0][3] = 0.0f;

	Result.elements[1][0] = S.y;
	Result.elements[1][1] = U.y;
	Result.elements[1][2] = -F.y;
	Result.elements[1][3] = 0.0f;

	Result.elements[2][0] = S.z;
	Result.elements[2][1] = U.z;
	Result.elements[2][2] = -F.z;
	Result.elements[2][3] = 0.0f;

	Result.elements[3][0] = -v3_dot(S, Eye);
	Result.elements[3][1] = -v3_dot(U, Eye);
	Result.elements[3][2] = v3_dot(F, Eye);
	Result.elements[3][3] = 1.0f;

	return Result;
}

static float v3_length_squared(s_v3 v)
{
	return v.x * v.x + v.y * v.y + v.z * v.z;
}

static float v3_length(s_v3 v)
{
	return sqrtf(v3_length_squared(v));
}


static s_v3 v3_normalized(s_v3 v)
{
	s_v3 result = v;
	float length = v3_length(v);
	if(!floats_equal(length, 0))
	{
		result.x /= length;
		result.y /= length;
		result.z /= length;
	}
	return result;
}


static s_m4 look_at(s_v3 Eye, s_v3 Center, s_v3 Up)
{
	s_v3 F = v3_normalized(Center - Eye);
	s_v3 S = v3_normalized(v3_cross(F, Up));
	s_v3 U = v3_cross(S, F);

	return look_at_internal(F, S, U, Eye);
}

static s_m4 m4_rotate(float angle, s_v3 axis)
{

	s_m4 Result = m4_identity();

	axis = v3_normalized(axis);

	float SinTheta = sinf(angle);
	float CosTheta = cosf(angle);
	float CosValue = 1.0f - CosTheta;

	Result.elements[0][0] = (axis.x * axis.x * CosValue) + CosTheta;
	Result.elements[0][1] = (axis.x * axis.y * CosValue) + (axis.z * SinTheta);
	Result.elements[0][2] = (axis.x * axis.z * CosValue) - (axis.y * SinTheta);

	Result.elements[1][0] = (axis.y * axis.x * CosValue) - (axis.z * SinTheta);
	Result.elements[1][1] = (axis.y * axis.y * CosValue) + CosTheta;
	Result.elements[1][2] = (axis.y * axis.z * CosValue) + (axis.x * SinTheta);

	Result.elements[2][0] = (axis.z * axis.x * CosValue) + (axis.y * SinTheta);
	Result.elements[2][1] = (axis.z * axis.y * CosValue) - (axis.x * SinTheta);
	Result.elements[2][2] = (axis.z * axis.z * CosValue) + CosTheta;

	return (Result);
}

static s_m4 m4_multiply(s_m4 left, s_m4 right)
{

	s_m4 result;

	int Columns;
	for(Columns = 0; Columns < 4; ++Columns)
	{
		int Rows;
		for(Rows = 0; Rows < 4; ++Rows)
		{
			float sum = 0;
			int CurrentMatrice;
			for(CurrentMatrice = 0; CurrentMatrice < 4; ++CurrentMatrice)
			{
				sum += left.elements[CurrentMatrice][Columns] * right.elements[Rows][CurrentMatrice];
			}

			result.elements[Rows][Columns] = sum;
		}
	}

	return result;
}


template <typename t>
static void swap(t* a, t* b)
{
	t temp = *a;
	*a = *b;
	*b = temp;
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

static s_len_str format_text(const char* text, ...)
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

	return strlit(current_buffer);
}

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


static b8 is_number(char c)
{
	return c >= '0' && c <= '9';
}

static b8 is_alpha(char c)
{
	return (c >= 'A' && c <= 'Z') || (c >= 'a' && c <= 'z');
}

static b8 is_alpha_numeric(char c)
{
	return is_number(c) || is_alpha(c);
}

static b8 can_start_identifier(char c)
{
	return is_alpha(c) || c == '_';
}

static b8 can_continue_identifier(char c)
{
	return is_alpha(c) || is_number(c) || c == '_';
}

struct s_parse_identifier
{
	char* start;
	char* end;
};

static s_parse_identifier parse_identifier(char* str)
{
	assert(str);
	s_parse_identifier result = {};
	while(*str == ' ') { str += 1; }
	char* cursor = str;
	result.start = str;
	b8 found_something = false;
	if(can_start_identifier(*cursor)) {
		found_something = true;
		cursor += 1;
		while(can_continue_identifier(*cursor)) { cursor += 1; }
	}
	if(found_something) {
		result.end = cursor;
	}
	return result;
}


template <int max_chars>
struct s_str_builder
{
	int tab_count;
	int len = 0;
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

static s_v2 g_base_res = {0, 0};

#ifdef m_debug
#define gl(...) __VA_ARGS__; {int error = glGetError(); if(error != 0) { on_gl_error(#__VA_ARGS__, __FILE__, __LINE__, error); }}
#else // m_debug
#define gl(...) __VA_ARGS__
#endif // m_debug

enum e_shader
{
	e_shader_default,
	e_shader_basic_3d,
	e_shader_3d_flat,
	e_shader_threshold,
	e_shader_blur,
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
	u32 index_buffer_2d;
	u32 index_buffer_3d;
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
static u32 load_shader_from_str(const char* vertex_src, const char* fragment_src, char* out_buffer = NULL);
static u32 load_shader_from_file(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena);
static void after_making_framebuffer(int index, s_game_renderer* game_renderer);
static s_font load_font_from_file(const char* path, int font_size, s_lin_arena* arena);
static s_font load_font_from_data(u8* file_data, int font_size, s_lin_arena* arena);
static s_texture load_texture(s_game_renderer* game_renderer, const char* path);
static s_texture load_texture_from_data(void* data, int width, int height, u32 filtering, int format);
static s_font* load_font(s_game_renderer* game_renderer, const char* path, int font_size, s_lin_arena* arena);
static void reset_ui();

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


template <typename t>
[[nodiscard]]
static u8* buffer_write(u8* cursor, t value)
{
	*(t*)cursor = value;
	cursor += sizeof(t);
	return cursor;
}

[[nodiscard]]
static u8* buffer_write2(u8* cursor, void* data, u64 size)
{
	memcpy(cursor, data, size);
	cursor += size;
	return cursor;
}


template <typename t>
[[nodiscard]]
static t buffer_read(u8** out_cursor)
{
	u8* cursor = *out_cursor;
	t result = *(t*)cursor;
	cursor += sizeof(t);
	*out_cursor = cursor;
	return result;
}

[[nodiscard]]
static u8* buffer_read2(u8* cursor, void* dst, u64 size)
{
	memcpy(dst, cursor, size);
	cursor += size;
	return cursor;
}


static constexpr s_v2i v2i(int val)
{
	s_v2i result;
	result.x = val;
	result.y = val;
	return result;
}

static constexpr s_v2i v2i(s_v2 v)
{
	s_v2i result;
	result.x = (int)v.x;
	result.y = (int)v.y;
	return result;
}

static constexpr s_v2i v2i(int x, int y)
{
	s_v2i result;
	result.x = x;
	result.y = y;
	return result;
}

static s_v2i index_1d_to_2d(int val, int width)
{
	return v2i(
		val % width,
		val / width
	);
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

static s_v3 operator+(s_v3 a, s_v3 b)
{
	s_v3 result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	result.z = a.z + b.z;
	return result;
}

static s_v2i operator+(s_v2i a, s_v2i b)
{
	s_v2i result;
	result.x = a.x + b.x;
	result.y = a.y + b.y;
	return result;
}

static s_v2 operator-(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x - b.x;
	result.y = a.y - b.y;
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

static s_v3 operator*(s_v3 a, float b)
{
	s_v3 result;
	result.x = a.x * b;
	result.y = a.y * b;
	result.z = a.z * b;
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

static s_v3 operator/(s_v3 a, float b)
{
	s_v3 result;
	result.x = a.x / b;
	result.y = a.y / b;
	result.z = a.z / b;
	return result;
}

static s_v2 operator/(s_v2 a, s_v2 b)
{
	s_v2 result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	return result;
}

static s_v3 operator/(s_v3 a, s_v3 b)
{
	s_v3 result;
	result.x = a.x / b.x;
	result.y = a.y / b.y;
	result.z = a.z / b.z;
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


static s_v3 operator*(s_v3 a, s_v3 b)
{
	s_v3 result;
	result.x = a.x * b.x;
	result.y = a.y * b.y;
	result.z = a.z * b.z;
	return result;
}

static s_v3 ray_at_z(s_ray ray, float z)
{
	float t = (z - ray.pos.z) / ray.dir.z;
	return ray.pos + ray.dir * t;
}

static s_v4 v4_multiply_m4(s_v4 v, s_m4 m)
{
	s_v4 result;

	result.x = m.all[0] * v.x + m.all[4] * v.y + m.all[8]  * v.z + m.all[12] * v.w;
	result.y = m.all[1] * v.x + m.all[5] * v.y + m.all[9]  * v.z + m.all[13] * v.w;
	result.z = m.all[2] * v.x + m.all[6] * v.y + m.all[10] * v.z + m.all[14] * v.w;
	result.w = m.all[3] * v.x + m.all[7] * v.y + m.all[11] * v.z + m.all[15] * v.w;

	return result;
}


static s_v4 operator*(s_m4 a, s_v4 b)
{
	return v4_multiply_m4(b, a);
}

static s_ray get_ray(s_v2 mouse, s_v2 window_size, s_camera3d cam, s_m4 view, s_m4 projection)
{
	float x = (2.0f * mouse.x) / window_size.x - 1.0f;
	float y = 1.0f - (2.0f * mouse.y) / window_size.y;
	float z = 1.0f;
	s_v3 ray_nds = v3(x, y, z);
	s_v4 ray_clip = v4(ray_nds.x, ray_nds.y, -1.0f, 1.0f);
	s_v4 ray_eye = m4_inverse(projection) * ray_clip;
	ray_eye = v4(ray_eye.x, ray_eye.y, -1.0f, 0.0f);

	s_v4 temp = m4_inverse(view) * ray_eye;
	s_v3 ray_wor = v3(temp.x, temp.y, temp.z);
	ray_wor = v3_normalized(ray_wor);
	return {.pos = cam.pos, .dir = ray_wor};
}



static float smoothstep(float edge0, float edge1, float x)
{
	// Scale, bias and saturate x to 0..1 range
	x = clamp((x - edge0) / (edge1 - edge0), 0.0f, 1.0f);
	// Evaluate polynomial
	return x * x * (3 - 2 * x);
}


static s_v2 v2_rotated(s_v2 v, float angle)
{

	float x = v.x;
	float y = v.y;

	float cs = cosf(angle);
	float sn = sinf(angle);

	v.x = x * cs - y * sn;
	v.y = x * sn + y * cs;

	return v;
}

static float sin2(float t)
{
	return sinf(t) * 0.5f + 0.5f;
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

static void operator+=(s_v3& left, s_v3 right)
{
	left.x += right.x;
	left.y += right.y;
	left.z += right.z;
}

static void operator-=(s_v3& left, s_v3 right)
{
	left.x -= right.x;
	left.y -= right.y;
	left.z -= right.z;
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

static s_v2 sign(s_v2 v)
{
	s_v2 result;
	result.x = v.x >= 0 ? 1.0f : -1.0f;
	result.y = v.y >= 0 ? 1.0f : -1.0f;
	return result;
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

static b8 rect_collides_rect_center(s_v2 pos0, s_v2 size0, s_v2 pos1, s_v2 size1)
{
	return rect_collides_rect_topleft(pos0 - size0 * 0.5f, size0, pos1 - size1 * 0.5f, size1);
}

static b8 mouse_collides_rect_topleft(s_v2 mouse, s_v2 pos, s_v2 size)
{
	return rect_collides_rect_topleft(mouse, v2(1, 1), pos, size);
}

static b8 mouse_collides_rect_center(s_v2 mouse, s_v2 pos, s_v2 size)
{
	return rect_collides_rect_center(mouse, v2(1, 1), pos, size);
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

static s_v2 lerp_snap(s_v2 a, s_v2 b, float t)
{
	s_v2 result;
	float dist = v2_distance(a, b);
	if(dist < 1.0f) {
		t = 1;
	}
	result.x = lerp(a.x, b.x, t);
	result.y = lerp(a.y, b.y, t);
	return result;
}


static s_v2 lerp_snap(s_v2 a, s_v2 b, float t, s_v2 max_diff)
{
	s_v2 result;
	result.x = lerp_snap(a.x, b.x, t, max_diff.x);
	result.y = lerp_snap(a.y, b.y, t, max_diff.y);
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

static s_v2 v2_rand_0_to_1(s_rng* rng)
{
	return v2(
		rng->randf32(),
		rng->randf32()
	);
}

static s_v2 v2_rand_minus_1_to_1(s_rng* rng)
{
	return v2(
		(float)rng->randf2(),
		(float)rng->randf2()
	);
}

static s_v2 v2_rand_minus_1_to_1_normalized(s_rng* rng)
{
	return v2_normalized(v2_rand_minus_1_to_1(rng));
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

static s_v2 random_point_rect_topleft(s_v2 pos, s_v2 size, s_rng* rng)
{
	s_v2 result;
	result.x = pos.x + size.x * rng->randf32();
	result.y = pos.y + size.y * rng->randf32();
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

static char lower(char c)
{
	if(c >= 'A' && c <= 'Z') { return c + ('a' - 'A'); }
	return c;
}

static char to_upper_case(char c)
{
	if(c >= 'a' && c <= 'z') { return c - ('a' - 'A'); }
	return c;
}

static b8 strncmp_ignore_case(const char* haystack, const char* needle, int count)
{
	int haystack_len = (int)strlen(haystack);
	int needle_len = (int)strlen(needle);
	assert(count <= haystack_len);
	assert(count <= needle_len);
	if(needle_len > haystack_len) { return false; }

	for(int haystack_i = 0; haystack_i < count; haystack_i++) {
		b8 found = true;
		for(int needle_i = 0; needle_i < count; needle_i++) {
			char haystack_c = lower(haystack[haystack_i + needle_i]);
			char needle_c = lower(needle[needle_i]);
			if(haystack_c != needle_c) {
				found = false;
				break;
			}
		}
		if(found) {
			return true;
		}
	}
	return false;
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

 // @TODO(tkap, 13/11/2023): handle this on windows
static constexpr int c_middle_mouse = 1022;

static constexpr int c_max_keys = 1024;

static constexpr int c_game_memory = 20 * c_mb;

static constexpr s_v2 c_origin_topleft = {1.0f, 1.0f};
static constexpr s_v2 c_origin_bottomleft = {1.0f, -1.0f};
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
	e_render_flag_line = 1 << 4,
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
typedef s_framebuffer* (*t_make_framebuffer)(s_game_renderer*, s_v2i);
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
	int effect_id;
	float mix_weight;
	float rotation;
	s_v2 origin_offset;
	s_v2 draw_size;
	s_v2 texture_size;
	s_v2 uv_min;
	s_v2 uv_max;
	s_v3 pos;
	s_v4 color;
	s_v4 mix_color;
	s_m4 model;
};
#pragma pack(pop)

struct s_framebuffer
{
	b8 is_main;
	b8 depth_is_borrowed;
	u32 gpu_id;
	int game_id;
	s_texture texture;
	s_texture depth;
};


struct s_render_pass
{
	b8 do_clear;
	b8 do_depth;
	b8 do_cull;
	b8 dont_write_depth;
	e_blend_mode blend_mode;
	s_v3 cam_pos;
	s_m4 view_projection;
	s_framebuffer* framebuffer;
	s_v4 clear_color;
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

#ifdef m_debug
struct s_foo
{
	int update_count;
	s_stored_input input;
};

struct s_recorded_input
{
	int starting_update;
	s_sarray<s_v2, 10240> mouse;
	s_sarray<s_foo, 1024> keys;
};
#endif // m_debug

struct s_key
{
	b8 is_down;
	int count;
};

struct s_key_event
{
	b8 went_down;
	int modifiers;
	int key;
};

struct s_input
{
	float wheel_movement;

	s_sarray<s_key_event, 128> key_events;
	s_sarray<char, 128> char_events;
	s_carray<s_key, c_max_keys> keys;
};

struct s_render_data
{
	b8 flip_x;
	int shader;
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
	char* name;
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

#ifdef m_debug
#define add_console_command(...) add_console_command_(__VA_ARGS__)
#define add_msg_to_console(...) add_msg_to_console_(__VA_ARGS__)
struct s_console;
typedef void(*t_console_func)(s_console*, char*);
struct s_console_command
{
	char* name;
	t_console_func func_ptr;
};

struct s_console
{
	float curr_y;
	float target_y;
	int cursor;
	int last_command_index;
	float cursor_visual_x;
	s_str<128> input;
	s_sarray<s_str<128>, 32> buffer;
	s_sarray<s_str<128>, 32> prev_commands;
	s_sarray<s_console_command, 128> commands;
};
#else // m_debug
#define add_console_command(...)
#define add_msg_to_console(...)
#endif // m_debug

typedef void (*t_submit_leaderboard_score_callback)(void);
typedef void (*t_get_leaderboard_callback)(s_json*);
typedef void (*t_get_our_leaderboard_callback)(s_json*);
typedef void (*t_set_leaderboard_name_callback)(b8);

struct s_platform_data
{
	b8 recompiled;
	b8 quit_after_this_frame;
	b8 any_key_pressed;
	b8 is_window_active;
	b8 window_resized;
	b8 game_called_set_window_size;

	#ifdef m_debug
	int recorded_input_index;
	s_recorded_input recorded_input;
	b8 recording_input;
	b8 replaying_input;
	b8 loaded_a_state;
	u32 program_that_failed;
	char program_error[1024];

	s_console console;
	#endif // m_debug

	void (*submit_leaderboard_score)(int, int, t_submit_leaderboard_score_callback);
	void (*get_leaderboard)(int, t_get_leaderboard_callback);
	void (*get_our_leaderboard)(int, t_get_our_leaderboard_callback);
	void (*register_leaderboard_client)(void);
	void (*set_leaderboard_name)(s_len_str, t_set_leaderboard_name_callback);
	char* leaderboard_session_token;
	t_submit_leaderboard_score_callback submit_leaderboard_score_callback;
	t_get_leaderboard_callback get_leaderboard_callback;
	t_get_our_leaderboard_callback get_our_leaderboard_callback;
	t_set_leaderboard_name_callback set_leaderboard_name_callback;
	s_str<64> leaderboard_public_uid;
	s_str<64> leaderboard_nice_name;
	s_str<256> leaderboard_player_identifier;
	int leaderboard_player_id;

	int update_count;
	int render_count;

	int window_width;
	int window_height;
	s_input logic_input;
	s_input render_input;
	s_lin_arena* frame_arena;
	s_v2 mouse;
	f64 frame_time;
	f64 update_time;
	f64 update_delay;
	t_get_random_seed get_random_seed;
	t_load_sound load_sound;
	t_play_sound play_sound;
	s_sarray<s_sound, 16> sounds;
	t_show_cursor show_cursor;
	t_cycle_between_available_resolutions cycle_between_available_resolutions;
	t_read_file read_file;
	t_write_file write_file;
	void (*reset_ui)();
	s_ui_interaction (*ui_button)(s_game_renderer*, s_len_str, s_v2, s_v2, s_font*, float, s_input*, s_v2);
	void (*ui_checkbox)(s_game_renderer*, s_len_str, s_v2, s_v2, b8*, s_input*, s_v2);
	void (*set_window_size)(int, int);
	void (*set_base_resolution)(int, int);
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

static constexpr int c_max_framebuffers = 8;
typedef s_texture (*t_load_texture)(s_game_renderer*, const char*);
typedef s_font* (*t_load_font)(s_game_renderer*, const char*, int, s_lin_arena*);
struct s_game_renderer
{
	b8 did_we_alloc;
	b8 in_render_pass;
	t_set_vsync set_vsync;
	t_load_texture load_texture;
	t_load_font load_font;
	t_make_framebuffer make_framebuffer;
	t_set_shader_float set_shader_float;
	t_set_shader_v2 set_shader_v2;
	void (*end_render_pass)(s_game_renderer*, s_render_pass);
	void (*clear_framebuffer)(s_framebuffer*, s_v4);
	s_framebuffer* (*make_framebuffer_with_existing_depth)(s_game_renderer*, s_v2i, s_texture);
	// void (*delete_framebuffer)(s_game_renderer*, s_framebuffer*);
	f64 total_time;

	s_texture checkmark_texture;

	// @Note(tkap, 08/10/2023): We esentially want s_bucket_array<s_transform> transforms[e_shader_count][e_texture_count];
	// but we don't know how many textures and shaders there will be at compile time, because the game code may load any amount
	s_bucket_array<s_transform>* transforms;

	int transform_arena_index;
	s_carray<s_lin_arena, 2> transform_arenas;
	int arena_index;
	s_carray<s_lin_arena, 2> arenas;
	s_sarray<s_texture, 32> textures;
	s_carray<b8, c_max_framebuffers> framebuffer_active_arr;
	s_carray<s_framebuffer, c_max_framebuffers> framebuffers;
	s_sarray<s_font, 4> fonts;
};

typedef void (t_update)(s_platform_data*, void*, s_game_renderer*);
typedef void (t_render)(s_platform_data*, void*, s_game_renderer*, float);
#ifdef m_build_dll
typedef void (t_init_game)(s_platform_data*);
#else // m_build_dll
void init_game(s_platform_data* platform_data);
void update(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer);
void render(s_platform_data* platform_data, void* game_memory, s_game_renderer* renderer, float interp_dt);
#endif


#if !defined(m_game)

static s_framebuffer* make_framebuffer(s_game_renderer* game_renderer, s_v2i size);
static b8 set_shader_v2(const char* uniform_name, s_v2 val);
static b8 set_shader_float(const char* uniform_name, float val);
static void end_render_pass(s_game_renderer* game_renderer, s_render_pass render_pass = {});
static void clear_framebuffer(s_framebuffer* fbo, s_v4 clear_color);
static s_framebuffer* make_framebuffer_with_existing_depth(s_game_renderer* game_renderer, s_v2i size, s_texture depth);
// static void delete_framebuffer(s_game_renderer* game_renderer, s_framebuffer* fbo);
#endif

// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		function headers start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
static int get_render_offset(s_game_renderer* game_renderer, int shader, int texture);
static s_v2 get_text_size_with_count(s_len_str text, s_font* font, float font_size, int count, int in_column = 0);
static s_v2 get_text_size(s_len_str text, s_font* font, float font_size);
template <typename t>
static void bucket_add(s_bucket_array<t>* arr, t new_element, s_lin_arena* arena, b8* did_we_alloc);
template <typename t>
static void bucket_merge(s_bucket_array<t>* arr, s_lin_arena* arena);
static s_json* parse_json(char* str);
static void print_json(s_json* json);
static s_json* json_get(s_json* json, char* key_name, e_json in_type);
static s_string_parse parse_string(char* str, b8 do_alloc);
static s_len_str alloc_string(void* data, int len);
static char* to_cstr(s_len_str str, s_lin_arena* arena);
// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		function headers end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^



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

static int get_render_offset(s_game_renderer* game_renderer, int shader, int texture)
{
	int result = shader * game_renderer->textures.count + texture;
	return result;
}

static int get_spaces_for_column(int column)
{
	constexpr int tab_size = 4;
	if(tab_size <= 0) { return 0; }
	return tab_size - (column % tab_size);
}

// @TODO(tkap, 31/10/2023): Handle new lines
static s_v2 get_text_size_with_count(s_len_str text, s_font* font, float font_size, int count, int in_column)
{
	assert(count >= 0);
	if(count <= 0) { return {}; }

	int column = in_column;

	s_v2 size = {};
	float scale = font->scale * (font_size / font->size);
	size.y = font_size;

	for(int char_i = 0; char_i < count; char_i++)
	{
		char c = text[char_i];
		s_glyph glyph = font->glyph_arr[c];
		if(c == '\t')
		{
			int spaces = get_spaces_for_column(column);
			size.x += glyph.advance_width * scale * spaces;
			column += spaces;
		}
		else
		{
			size.x += glyph.advance_width * scale;
			column += 1;
		}
	}

	return size;
}

static s_v2 get_text_size(s_len_str text, s_font* font, float font_size)
{
	return get_text_size_with_count(text, font, font_size, text.len);
}

static void draw_rect(s_game_renderer* game_renderer, s_v2 pos, int layer, s_v2 size, s_v4 color, s_render_data render_data = {}, s_transform t = {})
{
	s_m4 model = m4_translate(v3(pos, -99.0f + layer * 2));
	model = m4_multiply(model, m4_scale(v3(size, 1)));
	if(!is_zero(t.rotation)) {
		model = m4_multiply(model, m4_rotate(t.rotation, v3(0, 0, 1)));
	}
	t.model = model;

	t.pos = v3(pos, 0);
	t.draw_size = size;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	t.mix_color = v41f(1);
	bucket_add(&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, 0)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_rect_3d(s_game_renderer* game_renderer, s_v3 pos, s_v2 size, s_v4 color, s_render_data render_data = {}, s_transform t = {})
{
	s_m4 model = m4_translate(pos);
	model = m4_multiply(model, m4_scale(v3(size, 1)));
	if(!is_zero(t.rotation)) {
		model = m4_multiply(model, m4_rotate(t.rotation, v3(0, 0, 1)));
	}
	t.model = model;

	t.draw_size = size;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	t.mix_color = v41f(1);
	bucket_add(&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, 0)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_cube(s_game_renderer* game_renderer, s_v3 pos, s_v3 size, s_v4 color, s_render_data render_data = {}, s_transform t = {})
{

	if(render_data.shader == 0) { render_data.shader = 1; }

	s_m4 model = m4_translate(pos);
	model = m4_multiply(model, m4_scale(size));
	t.model = model;
	t.pos = pos;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	t.mix_color = v41f(1);
	bucket_add(&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, 0)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_textured_cube(s_game_renderer* game_renderer, s_v3 pos, s_v3 size, s_v4 color, s_texture texture, s_render_data render_data = {}, s_transform t = {})
{
	if(render_data.shader == 0) { render_data.shader = 1; }

	s_m4 model = m4_translate(pos);
	model = m4_multiply(model, m4_scale(size));
	t.flags |= e_render_flag_use_texture;
	t.model = model;
	t.pos = pos;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	t.mix_color = v41f(1);
	bucket_add(&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, texture.game_id)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_texture_3d(s_game_renderer* game_renderer, s_v3 pos, s_v2 size, s_v4 color, s_texture texture, s_render_data render_data = {}, s_transform t = {})
{

	if(render_data.shader == 0) { render_data.shader = 2; }

	s_m4 model = m4_translate(pos);
	model = m4_multiply(model, m4_scale(v3(size, 1)));
	t.model = model;
	t.flags |= e_render_flag_use_texture;
	t.pos = pos;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	if(render_data.flip_x) {
		swap(&t.uv_min.x, &t.uv_max.x);
	}
	t.mix_color = v41f(1);
	bucket_add(&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, texture.game_id)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_line(s_game_renderer* game_renderer, s_v2 from, s_v2 to, int layer, float thickness, s_v4 color, s_render_data render_data = {}, s_transform t = {})
{
	t.flags |= e_render_flag_line;
	t.pos = v3(from, 0);
	t.draw_size = to;
	t.texture_size.x = thickness;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	t.mix_color = v41f(1);
	bucket_add(&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, 0)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_texture(s_game_renderer* game_renderer, s_v2 pos, int layer, s_v2 size, s_v4 color, s_texture texture, s_render_data render_data = {}, s_transform t = {})
{

	s_m4 model = m4_translate(v3(pos, 0));
	model = m4_multiply(model, m4_scale(v3(size, 1)));
	if(!is_zero(t.rotation)) {
		model = m4_multiply(model, m4_rotate(t.rotation, v3(0, 0, 1)));
	}
	t.model = model;
	t.flags |= e_render_flag_use_texture;
	t.pos = v3(pos, 0);
	t.draw_size = size;
	t.color = color;
	t.uv_min = v2(0, 1);
	t.uv_max = v2(1, 0);

	// @Note(tkap, 31/05/2024): Let's use draw_framebuffer for now
	assert(!texture.comes_from_framebuffer);
	// if(texture.comes_from_framebuffer) {
	// 	swap(&t.uv_min.y, &t.uv_max.y);
	// }
	t.mix_color = v41f(1);
	bucket_add(&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, texture.game_id)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_framebuffer(s_game_renderer* game_renderer, s_v2 pos, int layer, s_v2 size, s_v4 color, s_framebuffer* framebuffer, s_render_data render_data = {}, s_transform t = {})
{
	s_m4 model = m4_translate(v3(pos, 0));
	model = m4_multiply(model, m4_scale(v3(size, 1)));
	if(!is_zero(t.rotation)) {
		model = m4_multiply(model, m4_rotate(t.rotation, v3(0, 0, 1)));
	}
	t.model = model;
	t.flags |= e_render_flag_use_texture;
	t.pos = v3(pos, 0);
	t.draw_size = size;
	t.color = color;
	t.uv_min = v2(0, 0);
	t.uv_max = v2(1, 1);
	t.mix_color = v41f(1);
	bucket_add(&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, framebuffer->texture.game_id)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_atlas(s_game_renderer* game_renderer, s_v2 pos, int layer, s_v2 size, s_v4 color, s_texture texture, s_v2i sprite_pos, s_v2i sprite_size, s_render_data render_data = {}, s_transform t = {})
{

	s_m4 model = m4_translate(v3(pos, 0));
	model = m4_multiply(model, m4_scale(v3(size, 1)));
	if(!is_zero(t.rotation)) {
		model = m4_multiply(model, m4_rotate(t.rotation, v3(0, 0, 1)));
	}
	t.model = model;

	t.flags |= e_render_flag_use_texture;
	t.pos = v3(pos, 0);
	t.draw_size = size;
	t.color = color;
	t.uv_min = v2(
		(float)sprite_pos.x / texture.size.x,
		(float)sprite_pos.y / texture.size.y
	);
	t.uv_max = v2(
		t.uv_min.x + sprite_size.x / (float)texture.size.x,
		t.uv_min.y + sprite_size.y / (float)texture.size.y
	);
	swap(&t.uv_min.y, &t.uv_max.y);

	// @Note(tkap, 31/05/2024): Let's use draw_framebuffer for now
	assert(!texture.comes_from_framebuffer);
	// if(texture.comes_from_framebuffer) {
	// 	// @Note(tkap, 13/11/2023): If this triggers it means that we do not set texture size for framebuffer textures. Should do that...
	// 	assert(texture.size.x > 0 && texture.size.y > 0);
	// 	swap(&t.uv_min.y, &t.uv_max.y);
	// }
	t.mix_color = v41f(1);
	bucket_add(&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, texture.game_id)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static void draw_atlas_3d(s_game_renderer* game_renderer, s_v3 pos, s_v2 size, s_v4 color, s_texture texture, s_v2i sprite_pos, s_v2i sprite_size, s_render_data render_data = {}, s_transform t = {})
{

	if(render_data.shader == 0) { render_data.shader = 2; }

	s_m4 model = m4_translate(pos);
	model = m4_multiply(model, m4_scale(v3(size, 1)));
	if(!is_zero(t.rotation)) {
		model = m4_multiply(model, m4_rotate(t.rotation, v3(0, 0, 1)));
	}
	t.model = model;
	t.flags |= e_render_flag_use_texture;
	t.pos = pos;
	t.color = color;
	t.uv_min = v2(
		(float)sprite_pos.x / texture.size.x,
		(float)sprite_pos.y / texture.size.y
	);
	t.uv_max = v2(
		t.uv_min.x + sprite_size.x / (float)texture.size.x,
		t.uv_min.y + sprite_size.y / (float)texture.size.y
	);
	if(render_data.flip_x) {
		swap(&t.uv_min.x, &t.uv_max.x);
	}
	// @Note(tkap, 31/05/2024): Let's use draw_framebuffer for now
	assert(!texture.comes_from_framebuffer);
	// if(texture.comes_from_framebuffer) {
	// 	// @Note(tkap, 13/11/2023): If this triggers it means that we do not set texture size for framebuffer textures. Should do that...
	// 	assert(texture.size.x > 0 && texture.size.y > 0);
	// 	swap(&t.uv_min.y, &t.uv_max.y);
	// }
	t.mix_color = v41f(1);
	bucket_add(&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, texture.game_id)], t, &game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc);
}

static s_v2 draw_text(s_game_renderer* game_renderer, s_len_str text, s_v2 in_pos, int layer, float font_size, s_v4 color, b8 centered, s_font* font, s_render_data render_data = {}, s_transform t = {})
{
	float scale = font->scale * (font_size / font->size);

	assert(text.len > 0);
	if(centered) {
		s_v2 text_size = get_text_size(text, font, font_size);
		in_pos.x -= text_size.x / 2;
		in_pos.y -= text_size.y / 2;
	}
	s_v2 pos = in_pos;
	pos.y += font->ascent * scale;
	for(int char_i = 0; char_i < text.len; char_i++) {
		int c = text[char_i];
		if(c <= 0 || c >= 128) { continue; }

		if(c == '\n' || c == '\r') {
			pos.x = in_pos.x;
			pos.y += font_size;
			continue;
		}

		s_glyph glyph = font->glyph_arr[c];
		t.draw_size = v2((glyph.x1 - glyph.x0) * scale, (glyph.y1 - glyph.y0) * scale);

		s_v2 glyph_pos = pos;
		glyph_pos.x += glyph.x0 * scale;
		glyph_pos.y += -glyph.y0 * scale;

		// glyph_pos.y += font->ascent * scale;
		// glyph_pos.y += font_size;
		t.flags |= e_render_flag_use_texture | e_render_flag_text;
		t.pos = v3(glyph_pos, 0);

		s_v2 center = t.pos.xy + t.draw_size / 2 * v2(1, -1);
		s_v2 bottomleft = t.pos.xy;

		// s_v2 topleft = t.pos + t.draw_size * v2(0, -1);
		// draw_rect(t.pos, 1, t.draw_size, make_color(0.4f, 0,0), {}, {.origin_offset = c_origin_bottomleft});
		// draw_rect(center, 75, v2(4), make_color(0, 1,0), {});
		// draw_rect(topleft, 75, v2(4), make_color(0, 0,1), {}, {.origin_offset = c_origin_topleft});
		// draw_rect(bottomleft, 75, v2(4), make_color(1, 1,0), {}, {.origin_offset = c_origin_bottomleft});
		// draw_rect(in_pos, 77, v2(4), make_color(0, 1,1), {}, {.origin_offset = c_origin_topleft});

		t.pos.xy = v2_rotate_around(center, in_pos, t.rotation) + (bottomleft - center);

		s_m4 model = m4_translate(v3(t.pos.xy, -99.0f + layer * 2));
		model = m4_multiply(model, m4_scale(v3(t.draw_size, 1)));
		// model = m4_multiply(model, m4_scale(v3(t.draw_size.x, t.draw_size.y * -1, 1)));
		// if(!is_zero(t.rotation)) {
		// 	model = m4_multiply(model, m4_rotate(t.rotation, v3(0, 0, 1)));
		// }
		t.model = model;

		t.color = color;
		t.uv_min = glyph.uv_min;
		t.uv_max = glyph.uv_max;
		swap(&t.uv_min.y, &t.uv_max.y);
		t.origin_offset = c_origin_bottomleft;

		bucket_add(
			&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, font->texture.game_id)], t,
			&game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc
		);

		pos.x += glyph.advance_width * scale;

	}
	return v2(pos.x, in_pos.y);
}

static s_v2 draw_text_3d(s_game_renderer* game_renderer, s_len_str text, s_v3 in_pos, float font_size, s_v4 color, b8 centered, s_font* font, s_render_data render_data = {}, s_transform t = {})
{
	float scale = font->scale * (font_size / font->size);

	assert(text.len > 0);
	if(centered) {
		s_v2 text_size = get_text_size(text, font, font_size);
		in_pos.x -= text_size.x / 2;
		in_pos.y -= text_size.y / 2;
	}
	s_v2 pos = in_pos.xy;
	pos.y += font->ascent * scale;
	for(int char_i = 0; char_i < text.len; char_i++) {
		int c = text[char_i];
		if(c <= 0 || c >= 128) { continue; }

		if(c == '\n' || c == '\r') {
			pos.x = in_pos.x;
			pos.y += font_size;
			continue;
		}

		s_glyph glyph = font->glyph_arr[c];
		t.draw_size = v2((glyph.x1 - glyph.x0) * scale, (glyph.y1 - glyph.y0) * scale);

		s_v2 glyph_pos = pos;
		glyph_pos.x += glyph.x0 * scale;
		glyph_pos.y += -glyph.y0 * scale;

		// glyph_pos.y += font->ascent * scale;
		// glyph_pos.y += font_size;
		t.flags |= e_render_flag_use_texture | e_render_flag_text;
		t.pos = v3(glyph_pos, 0);

		s_v2 center = t.pos.xy + t.draw_size / 2 * v2(1, -1);
		s_v2 bottomleft = t.pos.xy;

		// s_v2 topleft = t.pos + t.draw_size * v2(0, -1);
		// draw_rect(t.pos, 1, t.draw_size, make_color(0.4f, 0,0), {}, {.origin_offset = c_origin_bottomleft});
		// draw_rect(center, 75, v2(4), make_color(0, 1,0), {});
		// draw_rect(topleft, 75, v2(4), make_color(0, 0,1), {}, {.origin_offset = c_origin_topleft});
		// draw_rect(bottomleft, 75, v2(4), make_color(1, 1,0), {}, {.origin_offset = c_origin_bottomleft});
		// draw_rect(in_pos, 77, v2(4), make_color(0, 1,1), {}, {.origin_offset = c_origin_topleft});

		t.pos.xy = v2_rotate_around(center, in_pos.xy, t.rotation) + (bottomleft - center);

		s_m4 model = m4_translate(v3(t.pos.xy, in_pos.z));
		model = m4_multiply(model, m4_scale(v3(t.draw_size, 1)));
		// model = m4_multiply(model, m4_scale(v3(t.draw_size.x, t.draw_size.y * -1, 1)));
		// if(!is_zero(t.rotation)) {
		// 	model = m4_multiply(model, m4_rotate(t.rotation, v3(0, 0, 1)));
		// }
		t.model = model;

		t.color = color;
		t.uv_min = glyph.uv_min;
		t.uv_max = glyph.uv_max;
		swap(&t.uv_min.y, &t.uv_max.y);
		t.origin_offset = c_origin_bottomleft;

		bucket_add(
			&game_renderer->transforms[get_render_offset(game_renderer, render_data.shader, font->texture.game_id)], t,
			&game_renderer->arenas[game_renderer->arena_index], &game_renderer->did_we_alloc
		);

		pos.x += glyph.advance_width * scale;

	}
	return v2(pos.x, in_pos.y);
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
	assert(arr);
	assert(arena);
	assert(did_we_alloc);


	for(int bucket_i = 0; bucket_i < arr->bucket_count; bucket_i++) {
		int* count = &arr->element_count[bucket_i];
		assert(*count <= arr->capacity[bucket_i]);

		assert((u8*)&arr->elements[bucket_i][0] >= (u8*)arena->memory);
		assert((u8*)&arr->elements[bucket_i][0] < (u8*)arena->memory + arena->used);

		if(*count < arr->capacity[bucket_i]) {
			arr->elements[bucket_i][*count] = new_element;
			*count += 1;
			return;
		}
	}

	// assert(arr->bucket_count < 16);
	if(arr->bucket_count >= 16) {
		return;
	}

	constexpr int capacity = c_bucket_capacity;
	arr->elements[arr->bucket_count] = (t*)la_get(arena, sizeof(t) * capacity);
	arr->capacity[arr->bucket_count] = capacity;
	arr->elements[arr->bucket_count][0] = new_element;
	arr->element_count[arr->bucket_count] = 1;
	arr->bucket_count += 1;

	if(arr->bucket_count > 1) {
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

	assert(arr->bucket_count <= 16);

	if(arr->capacity[0] <= 0) { return; }

	for(int i = 0; i < arr->bucket_count; i++) {
		capacity += arr->capacity[i];
	}

	elements = (t*)la_get(arena, element_size * capacity);

	for(int i = 0; i < arr->bucket_count; i++) {
		assert((count + arr->element_count[i]) * element_size <= element_size * capacity);
		memcpy(&elements[count], arr->elements[i], element_size * arr->element_count[i]);
		count += arr->element_count[i];

		// @Note(tkap, 27/10/2023): Unnecessary
		arr->elements[i] = NULL;
		arr->element_count[i] = 0;
		arr->capacity[i] = 0;
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

#ifdef m_debug
static void add_console_command_(s_console* cn, char* name, t_console_func func_ptr)
{
	s_console_command new_command = {};
	new_command.name = name;
	new_command.func_ptr = func_ptr;

	foreach_val(command_i, command, cn->commands) {
		if(strcmp(command.name, name) == 0) {
			cn->commands[command_i] = new_command;
			return;
		}
	}
	cn->commands.add(new_command);
}

static void add_msg_to_console_(s_console* cn, s_len_str text)
{
	s_str<128> str;
	assert(text.len > 0);
	assert(text.len < 128);
	str.len = text.len;
	memcpy(str.data, text.str, text.len + 1);
	cn->buffer.insert(0, str);
}
#endif // m_debug

#ifndef m_game
static u32 load_shader(const char* vertex_path, const char* fragment_path, s_lin_arena* frame_arena);
static b8 check_for_shader_errors(u32 id, char* out_error);
static s_texture load_texture_from_file(const char* path, u32 filtering);
static void after_loading_texture(s_game_renderer* game_renderer);

static void set_base_resolution(int width, int height)
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
	{
		.vertex_path = "shaders/basic_3d.vertex",
		.fragment_path = "shaders/basic_3d.fragment",
	},
	{
		.vertex_path = "shaders/3d_flat.vertex",
		.fragment_path = "shaders/3d_flat.fragment",
	},
	{
		.vertex_path = "shaders/vertex.vertex",
		.fragment_path = "shaders/threshold.fragment",
	},
	{
		.vertex_path = "shaders/vertex.vertex",
		.fragment_path = "shaders/blur.fragment",
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
	s_game_renderer* game_renderer, s_len_str text, s_v2 pos, s_v2 size, s_font* font, float font_size, s_input* input, s_v2 mouse
)
{
	s_ui_interaction result = {};
	u32 id = hash(text.str);
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
	if constexpr(is_int) {
		draw_text(game_renderer, format_text("%i", result), text_pos, 15, font_size, rgb(0xFB9766), true, font);
	}
	else if constexpr(is_float) {
		draw_text(game_renderer, format_text("%.2f", result), text_pos, 15, font_size, rgb(0xFB9766), true, font);
	}
	return result;
}

static void ui_checkbox(s_game_renderer* game_renderer, s_len_str text, s_v2 pos, s_v2 size, b8* val, s_input* input, s_v2 mouse)
{
	assert(val);
	u32 id = hash(text.str);
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
		u8* data = (u8*)malloc(file_size + 1);
		fread(data, 1, file_size, file);
		data[file_size] = 0;
		u8* cursor = data;

		builder->add_line("static constexpr u8 embed%i[%u] = {", embed_i, file_size + 1);
		for(u64 i = 0; i < file_size + 1; i++) {
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

	log_info("Successfully created embed.h!\n");

	exit(0);
}

#ifdef m_debug
static void begin_replaying_input()
{
	assert(!g_platform_data.recording_input);
	g_platform_data.replaying_input = true;
	s_recorded_input* recorded_input = (s_recorded_input*)read_file("recorded_input", g_platform_data.frame_arena, NULL);
	memcpy(&g_platform_data.recorded_input, recorded_input, sizeof(*recorded_input));
	g_platform_data.recorded_input_index = 0;
}
#endif // m_debug

static s_recti do_letter_boxing(int base_width, int base_height, int window_width, int window_height);

#ifdef m_debug
static void update_console(s_console* cn, s_game_renderer* game_renderer)
{
	b8 is_open = cn->target_y > 0;

	s_input* input = &g_platform_data.render_input;

	if(is_key_pressed(input, c_key_f7)) {
		if(is_open) {
			cn->target_y = 0;
		}
		else {
			if(is_key_down(input, c_key_left_ctrl)) {
				cn->target_y = 1.0f;
			}
			else {
				cn->target_y = 0.5f;
			}
		}
	}

	if(cn->input.len <= 0) {
		cn->last_command_index = -1;
	}

	if(is_open) {
		foreach_val(c_i, c, input->char_events) {
			if(c == '\r') {
				if(cn->input.len > 0) {
					cn->prev_commands.insert(0, cn->input);
					b8 found_command = false;
					s_parse_identifier result = parse_identifier(cn->input.data);
					if(result.end) {
						foreach_val(command_i, command, cn->commands) {
							int len = (int)strlen(command.name);
							if(len > cn->input.len) { continue; }
							if(strncmp(cn->input.data, command.name, len) == 0) {
								found_command = true;
								command.func_ptr(cn, result.end);
								break;
							}
						}
					}
					if(!found_command) {
						if(result.end) {
							add_msg_to_console(cn, format_text("Command '%.*s' not found", result.end - result.start, result.start));
						}
						else {
							add_msg_to_console(cn, strlit("Expected identifier"));
						}
					}
				}
				cn->cursor = 0;
				cn->input.len = 0;
				cn->input.null_terminate();
			}
			// escape
			else if(c == 27) {
				cn->cursor = 0;
				cn->input.len = 0;
				cn->input.null_terminate();
			}
			else if(c == '\b') {
				if(cn->cursor > 0) {
					cn->cursor -= 1;
					int chars_right = cn->input.len - cn->cursor;
					if(chars_right > 0) {
						memmove(&cn->input[cn->cursor], &cn->input[cn->cursor + 1], chars_right);
					}
					cn->input.len -= 1;
					cn->input.null_terminate();
				}
			}
			else {
				int chars_right = cn->input.len - cn->cursor;
				if(chars_right > 0) {
					memmove(&cn->input[cn->cursor + 1], &cn->input[cn->cursor], chars_right);
				}
				cn->input.data[cn->cursor] = c;
				cn->cursor += 1;
				cn->input.len += 1;
				cn->input.null_terminate();
			}
		}

		if(is_key_pressed(input, c_key_left)) {
			cn->cursor = max(0, cn->cursor - 1);
		}
		if(is_key_pressed(input, c_key_right)) {
			cn->cursor = at_most(cn->input.len, cn->cursor + 1);
		}
		if(is_key_pressed(input, c_key_home)) {
			cn->cursor = 0;
		}
		if(is_key_pressed(input, c_key_end)) {
			cn->cursor = cn->input.len;
		}
		if(is_key_pressed(input, c_key_up)) {
			if(cn->prev_commands.count > 0) {
				cn->last_command_index = at_most(cn->prev_commands.count - 1, cn->last_command_index + 1);
				cn->input = cn->prev_commands[cn->last_command_index];
				cn->cursor = cn->prev_commands[cn->last_command_index].len;
			}
		}
		if(is_key_pressed(input, c_key_down)) {
			if(cn->prev_commands.count > 0) {
				cn->last_command_index = at_least(-1, cn->last_command_index - 1);
				if(cn->last_command_index == -1) {
					cn->cursor = 0;
					cn->input.len = 0;
					cn->input.null_terminate();
				}
				else {
					cn->input = cn->prev_commands[cn->last_command_index];
					cn->cursor = cn->prev_commands[cn->last_command_index].len;
				}
			}
		}
	}

	float delta = (float)g_platform_data.frame_time;
	s_font* font = &game_renderer->fonts[0];
	cn->curr_y = lerp_snap(cn->curr_y, cn->target_y, delta * 20.0f, 0.001f);
	constexpr float input_height = 32.0f;
	float input_y = g_base_res.y * cn->curr_y - input_height;
	float cursor_target_x = get_text_size_with_count(strlit(cn->input.data), font, input_height, cn->cursor).x;
	cn->cursor_visual_x = lerp_snap(cn->cursor_visual_x, cursor_target_x, delta * 20, 0.1f);
	draw_rect(game_renderer, v2(0, 0), 50, v2(g_base_res.x, g_base_res.y * cn->curr_y), rgb(0x44736B), {}, {.origin_offset = c_origin_topleft});
	draw_rect(game_renderer, v2(0.0f, input_y), 51, v2(g_base_res.x, input_height), rgb(0x5CA79A), {}, {.origin_offset = c_origin_topleft});
	draw_rect(game_renderer, v2(cn->cursor_visual_x, input_y), 55, v2(input_height * 0.33f, input_height), rgb(0x571E38), {}, {.origin_offset = c_origin_topleft});
	if(cn->input.len > 0) {
		draw_text(game_renderer, strlit(cn->input.data), v2(0.0f, input_y), 52, input_height, make_color(1), false, font);
	}

	foreach_val(msg_i, msg, cn->buffer) {
		draw_text(game_renderer, strlit(msg.data), v2(0.0f, input_y - (msg_i + 1) * input_height), 52, input_height, make_color(1), false, font);
	}
}

static void console_quit(s_console* cn, char* text)
{
	exit(1);
}
#endif // m_debug

static void do_game_layer(
	s_game_renderer* game_renderer, void* game_memory
	#ifndef m_sdl
	, t_update update,
	t_render render
	#endif
)
{

	#ifdef m_debug
	if(g_platform_data.update_count <= 0) {
		add_console_command(&g_platform_data.console, "quit", console_quit);
		add_console_command(&g_platform_data.console, "exit", console_quit);
	}
	#endif // m_debug

	// @Note(tkap, 13/11/2023): Adjust mouse coordinates based on window size. We want mouse to always in the [0, base_res] space
	{
		s_recti rect = do_letter_boxing((int)g_base_res.x, (int)g_base_res.y, g_platform_data.window_width, g_platform_data.window_height);
		g_platform_data.mouse.x = range_lerp(
			g_platform_data.mouse.x,
			(float)rect.x, (float)rect.x + rect.width,
			0.0f, g_base_res.x
		);
		g_platform_data.mouse.y = range_lerp(
			g_platform_data.mouse.y,
			(float)rect.y, (float)rect.y + rect.height,
			0.0f, g_base_res.y
		);
	}

	if(is_key_down(&g_platform_data.logic_input, c_key_left_alt) && is_key_down(&g_platform_data.logic_input, c_key_f4)) {
		exit(0);
	}

	#ifdef m_debug
	char* save_state_path = "save_state";

	// vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		save states start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{
		if(is_key_pressed(&g_platform_data.render_input, c_key_f10)) {
			if(g_platform_data.recording_input) {
				g_platform_data.recording_input = false;
				write_file("recorded_input", &g_platform_data.recorded_input, sizeof(g_platform_data.recorded_input));
				log_info("Recording stopped\n");
			}
			else {
				write_file(save_state_path, game_memory, c_game_memory);
				if(is_key_down(&g_platform_data.logic_input, c_key_left_ctrl)) {
					g_platform_data.recording_input = true;
					g_platform_data.recorded_input.starting_update = g_platform_data.update_count;
					log_info("Recording started\n");
				}
			}
		}
		if(is_key_pressed(&g_platform_data.render_input, c_key_f11)) {
			u8* data = (u8*)read_file(save_state_path, g_platform_data.frame_arena, NULL);
			if(data) {
				memcpy(game_memory, data, c_game_memory);
				g_platform_data.loaded_a_state = true;
				if(is_key_down(&g_platform_data.logic_input, c_key_left_ctrl)) {
					begin_replaying_input();
					log_info("Begin replay\n");
				}
				else {
					g_platform_data.replaying_input = false;
				}
			}
		}
	}
	// ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		save states end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	if(is_key_pressed(&g_platform_data.render_input, c_key_f8)) {
		g_platform_data.show_live_vars = !g_platform_data.show_live_vars;
	}

	if(g_platform_data.show_live_vars) {
		s_v2 pos = g_platform_data.vars_pos;
		s_ui_interaction interaction = ui_button(game_renderer, strlit("Move"), pos, v2(32), NULL, 32, &g_platform_data.logic_input, g_platform_data.mouse);
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
			draw_text(game_renderer, strlit(var.name), text_pos, 15, font_size, rgb(0xffffff), false, &game_renderer->fonts[0]);
			if(var.type == e_var_type_int) {
				*(int*)var.ptr = ui_slider(
					game_renderer, var.name, slider_pos, v2(200.0f, button_height), &game_renderer->fonts[0], font_size,
					var.min_val.val_int, var.max_val.val_int, *(int*)var.ptr, &g_platform_data.logic_input, g_platform_data.mouse
				);
			}
			else if(var.type == e_var_type_float) {
				*(float*)var.ptr = ui_slider(
					game_renderer, var.name, slider_pos, v2(200.0f, button_height), &game_renderer->fonts[0], font_size,
					var.min_val.val_float, var.max_val.val_float, *(float*)var.ptr, &g_platform_data.logic_input, g_platform_data.mouse
				);
			}
			else if(var.type == e_var_type_bool) {
				s_v2 temp = slider_pos;
				temp.x += 100 - button_height / 2.0f;
				ui_checkbox(game_renderer, strlit(var.name), temp, v2(button_height), (b8*)var.ptr, &g_platform_data.logic_input, g_platform_data.mouse);
			}
			pos.y += button_height + 4.0f;
		}

		if(ui_button(
				game_renderer, strlit("Save"), pos, v2(200.0f, button_height), &game_renderer->fonts[0], font_size, &g_platform_data.logic_input, g_platform_data.mouse
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

	g_platform_data.update_time += g_platform_data.frame_time;

	if(g_platform_data.update_delay <= 0) {
		g_platform_data.update_delay = 1.0 / 60.0;
	}
	f64 delay = g_platform_data.update_delay;
	f64 time = at_most(0.2, g_platform_data.update_time);
	while(time > delay || g_platform_data.update_count <= 0) {
		g_platform_data.update_time -= delay;
		time -= delay;

		#ifdef m_debug

		if(g_platform_data.recording_input) {
			g_platform_data.recorded_input.mouse.add(g_platform_data.mouse);
		}

		if(g_platform_data.replaying_input) {
			if(g_platform_data.recorded_input.mouse.count <= 0) {
				u8* data = (u8*)read_file(save_state_path, g_platform_data.frame_arena, NULL);
				assert(data);
				memcpy(game_memory, data, c_game_memory);
				begin_replaying_input();
				log_info("Replay restarted\n");
			}
			int frame = g_platform_data.recorded_input.starting_update + g_platform_data.recorded_input_index;
			g_platform_data.mouse = g_platform_data.recorded_input.mouse[0];
			g_platform_data.recorded_input.mouse.remove_and_shift(0);

			set_cursor_pos((int)g_platform_data.mouse.x, (int)g_platform_data.mouse.y);

			foreach_val(key_i, key, g_platform_data.recorded_input.keys) {
				if(key.update_count > frame) {
					break;
				}
				apply_event_to_input(&g_platform_data.logic_input, key.input);
				apply_event_to_input(&g_platform_data.render_input, key.input);
				g_platform_data.recorded_input.keys.remove_and_shift(key_i--);
			}

			g_platform_data.recorded_input_index += 1;
		}
		#endif // m_debug

		update(&g_platform_data, game_memory, game_renderer);
		g_platform_data.update_count += 1;
		g_platform_data.recompiled = false;
		g_platform_data.logic_input.wheel_movement = 0;

		for(int i = 0; i < c_max_keys; i++) {
			g_platform_data.logic_input.keys[i].count = 0;
		}
		g_platform_data.logic_input.char_events.count = 0;
		g_platform_data.logic_input.key_events.count = 0;

		#ifdef m_debug
		g_platform_data.loaded_a_state = false;
		#endif // m_debug
	}

	#ifdef m_debug
	update_console(&g_platform_data.console, game_renderer);
	#endif // m_debug

	float interp_dt = (float)(time / delay);
	render(&g_platform_data, game_memory, game_renderer, interp_dt);
	g_platform_data.render_count += 1;
	g_platform_data.render_input.wheel_movement = 0;

	reset_ui();
	for(int i = 0; i < c_max_keys; i++) {
		g_platform_data.render_input.keys[i].count = 0;
	}
	g_platform_data.render_input.char_events.count = 0;
	g_platform_data.render_input.key_events.count = 0;

	if(g_do_embed) {
		write_embed_file();
	}

	g_platform_data.frame_arena->used = 0;
}

static void on_gl_error(const char* expr, char* file, int line, int error)
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
	printf("  %s(%i)\n", file, line);
	printf("--------\n");
}


#ifdef __EMSCRIPTEN__

static void on_store_success(void* arg) { printf("stored\n"); }
static void on_store_error(void* arg) { printf("error\n"); }

static void success(emscripten_fetch_t *fetch) {
	((char*)fetch->data)[fetch->numBytes] = 0;
	// We're done with the fetch, so free it
	emscripten_fetch_close(fetch);
}

static void register_leaderboard_client_success(emscripten_fetch_t *fetch) {
	char buffer[4096] = {};
	memcpy(buffer, fetch->data, fetch->numBytes);
	s_json* json = parse_json(buffer);
	s_json* temp = json_get(json, "session_token", e_json_string);
	if(temp) {
		g_platform_data.leaderboard_session_token = temp->str;
		temp = json_get(json, "public_uid", e_json_string);
		g_platform_data.leaderboard_public_uid.from_cstr(temp->str);
		char* player_identifier = json_get(json, "player_identifier", e_json_string)->str;
		char* nice_name = json_get(json, "player_name", e_json_string)->str;
		g_platform_data.leaderboard_player_identifier.from_cstr(player_identifier);
		g_platform_data.leaderboard_player_id = json_get(json, "player_id", e_json_integer)->integer;
		g_platform_data.leaderboard_nice_name.from_cstr(nice_name);
	}
	emscripten_idb_async_store("leaderboard", "id", (void*)g_platform_data.leaderboard_player_identifier.data, g_platform_data.leaderboard_player_identifier.len, NULL, on_store_success, on_store_error);
	// We're done with the fetch, so free it
	emscripten_fetch_close(fetch);
}


static void failure(emscripten_fetch_t *fetch) {
	emscripten_fetch_close(fetch);
}

static void on_leaderboard_id_load_success(void* arg, void* in_data, int data_len)
{

	char data[1024] = {};
	memcpy(data, in_data, data_len);

	emscripten_fetch_attr_t attr = {};
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "POST");
	attr.onsuccess = register_leaderboard_client_success;
	attr.onerror = failure;
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

	s_len_str body = format_text("{\"game_key\": \"dev_ae7c0ca6ad2047e1890f76fe7836a5e3\", \"player_identifier\": \"%s\", \"game_version\": \"0.0.0.1\", \"development_mode\": true}", (char*)data);
	attr.requestData = body.str;
	attr.requestDataSize = body.len;
	emscripten_fetch(&attr, "https://api.lootlocker.io/game/v2/session/guest");

}

static void on_leaderboard_id_load_error(void* arg)
{
	emscripten_fetch_attr_t attr = {};
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "POST");
	attr.onsuccess = register_leaderboard_client_success;
	attr.onerror = failure;
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

	const char* body = "{\"game_key\": \"dev_ae7c0ca6ad2047e1890f76fe7836a5e3\", \"game_version\": \"0.0.0.1\", \"development_mode\": true}";
	attr.requestData = body;
	attr.requestDataSize = strlen(body);
	emscripten_fetch(&attr, "https://api.lootlocker.io/game/v2/session/guest");
}

static void register_leaderboard_client()
{
	if(g_platform_data.leaderboard_session_token) { return; }
	emscripten_idb_async_load("leaderboard", "id", NULL, on_leaderboard_id_load_success, on_leaderboard_id_load_error);
}


static void set_leaderboard_name_success(emscripten_fetch_t *fetch)
{
	// char buffer[4096] = {};
	// memcpy(buffer, fetch->data, fetch->numBytes);
	// s_json* json = parse_json(buffer);
	// print_json(json);
	// s_json* temp = json_get(json, "session_token", e_json_string);
	g_platform_data.set_leaderboard_name_callback(true);
	emscripten_fetch_close(fetch);
}

static void set_leaderboard_name_fail(emscripten_fetch_t *fetch)
{
	g_platform_data.set_leaderboard_name_callback(false);
	emscripten_fetch_close(fetch);
}

static void set_leaderboard_name(s_len_str name, t_set_leaderboard_name_callback callback)
{
	assert(g_platform_data.leaderboard_session_token);
	g_platform_data.set_leaderboard_name_callback = callback;

	emscripten_fetch_attr_t attr = {};
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "PATCH");
	attr.onsuccess = set_leaderboard_name_success;
	attr.onerror = set_leaderboard_name_fail;
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

	s_len_str data = format_text("{\"name\": \"%s\"}", to_cstr(name, g_platform_data.frame_arena));
	char* headers[] = {"x-session-token", g_platform_data.leaderboard_session_token, NULL};
	attr.requestHeaders = headers;
	attr.requestData = data.str;
	attr.requestDataSize = data.len;
	s_len_str url = strlit("https://api.lootlocker.io/game/player/name");
	emscripten_fetch(&attr, url.str);
}

#endif // __EMSCRIPTEN__

static void init_gl(s_platform_renderer* platform_renderer, s_game_renderer* game_renderer, s_lin_arena* arena)
{

	game_renderer->set_vsync = set_vsync;
	game_renderer->load_texture = load_texture;
	game_renderer->load_font = load_font;
	game_renderer->make_framebuffer = make_framebuffer;
	game_renderer->set_shader_float = set_shader_float;
	game_renderer->set_shader_v2 = set_shader_v2;
	game_renderer->end_render_pass = end_render_pass;
	game_renderer->clear_framebuffer = clear_framebuffer;
	game_renderer->make_framebuffer_with_existing_depth = make_framebuffer_with_existing_depth;
	// game_renderer->delete_framebuffer = delete_framebuffer;

	// // vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv		http start		vvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvvv
	{

		{
			// char* str = read_file("test.json", arena);
			// s_json* foo = parse_json(str);
			// print_json(foo);
		}
	}
	// // ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^		http end		^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^

	gl(glGenVertexArrays(1, &platform_renderer->default_vao));
	gl(glBindVertexArray(platform_renderer->default_vao));

	gl(glGenBuffers(1, &platform_renderer->default_vbo));
	gl(glBindBuffer(GL_ARRAY_BUFFER, platform_renderer->default_vbo));

	gl(glGenBuffers(1, &platform_renderer->index_buffer_2d));
	gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, platform_renderer->index_buffer_2d));

	{
		u32 index_arr[6] = {
			0, 1, 2,
			0, 2, 3
		};
		gl(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_arr), index_arr, GL_STATIC_DRAW));
	}

	gl(glGenBuffers(1, &platform_renderer->index_buffer_3d));
	gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, platform_renderer->index_buffer_3d));

	{
		u32 index_arr[36] = {
			// Front face
			0, 1, 2,
			0, 2, 3,

			// Back face
			4, 5, 6,
			4, 6, 7,

			// top face
			8, 9, 10,
			8, 10, 11,

			// bottom face
			12, 13, 14,
			12, 14, 15,

			// left face
			16, 17, 18,
			16, 18, 19,

			// right face
			20, 21, 22,
			20, 22, 23,
		};
		gl(glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_arr), index_arr, GL_STATIC_DRAW));
	}



	s_attrib_handler handler = {};
	add_int_attrib(&handler, 2);
	add_float_attrib(&handler, 1);
	add_float_attrib(&handler, 1);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 2);
	add_float_attrib(&handler, 3);
	add_float_attrib(&handler, 4);
	add_float_attrib(&handler, 4);

	// @Note(tkap, 25/05/2024): model matrix
	add_float_attrib(&handler, 4);
	add_float_attrib(&handler, 4);
	add_float_attrib(&handler, 4);
	add_float_attrib(&handler, 4);
	finish_attribs(&handler);

	platform_renderer->max_elements = 64;
	gl(glBufferData(GL_ARRAY_BUFFER, sizeof(s_transform) * platform_renderer->max_elements, NULL, GL_DYNAMIC_DRAW));

	for(int shader_i = 0; shader_i < e_shader_count; shader_i++) {
		u32 program = load_shader(c_shader_paths[shader_i].vertex_path, c_shader_paths[shader_i].fragment_path, arena);
		assert(program);
		platform_renderer->programs[shader_i] = program;
	}

	// @Fixme(tkap, 20/10/2023): path
	game_renderer->checkmark_texture = load_texture(game_renderer, "assets/checkmark.png");

	load_font(game_renderer, "assets/consola.ttf", 128, arena);

	// @TODO(tkap, 25/05/2024): dont think we need this
	// gl(glUseProgram(platform_renderer->programs[e_shader_default]));

	s_framebuffer framebuffer = {.is_main = true};
	for(int i = 0; i < c_max_framebuffers; i++) {
		if(game_renderer->framebuffer_active_arr[i]) { continue; }
		game_renderer->framebuffers[i] = framebuffer;
		game_renderer->framebuffer_active_arr[i] = true;
		break;
	}
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

static s_recti do_letter_boxing(int base_width, int base_height, int window_width, int window_height)
{
	s_recti result = {.x = 0, .y = 0, .width = window_width, .height = window_height};
	float base_ar = base_width / (float)base_height;
	float window_ar = window_width / (float)window_height;
	if(!floats_equal(base_ar, window_ar)) {
		b8 shrink_x = window_width / base_ar >= window_height;
		if(shrink_x) {
			int diff_from_width = window_width - base_width;
			int diff_from_height = (int)((base_height - window_height) * base_ar);
			result.x = diff_from_height / 2 + diff_from_width / 2;
			result.width = window_width - diff_from_height - diff_from_width;
		}
		else {
			int diff_from_height = window_height - base_height;
			int diff_from_width = (int)((base_width - window_width) / base_ar);
			result.y = diff_from_width / 2 + diff_from_height / 2;
			result.height = window_height - diff_from_width - diff_from_height;
		}
	}
	return result;
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

static u32 load_shader_from_str(const char* vertex_src, const char* fragment_src, char* out_error)
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
	b8 vertex_compiled = check_for_shader_errors(vertex, buffer);
	if(!vertex_compiled) {
		if(out_error) {
			memcpy(out_error, buffer, 1024);
		}
		return 0;
	}

	gl(glCompileShader(fragment));
	b8 fragment_compiled = check_for_shader_errors(fragment, buffer);
	if(!fragment_compiled) {
		if(out_error) {
			memcpy(out_error, buffer, 1024);
		}
		return 0;
	}
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

			if(out_error) {
				memcpy(out_error, buffer, 1024);
			}

			// @TODO(tkap, 27/10/2023): Delete shader and vertex?
			return 0;
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
	s_texture result = load_texture_from_data(data, width, height, GL_NEAREST, GL_RGBA);
	g_asset_index += 1;

	#else

	s_texture result = load_texture_from_file(path, GL_NEAREST);
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
	int size = sizeof(*game_renderer->transforms) * (e_shader_count * game_renderer->textures.count) + (game_renderer->textures.count);

	s_bucket_array<s_transform>* new_transforms = (s_bucket_array<s_transform>*)la_get_zero(
		&game_renderer->transform_arenas[new_index], size
	);

	// @Note(tkap, 08/10/2023): The first time we add a texture, transforms is NULL, so we can't memcpy from it
	if(game_renderer->transforms) {
		memcpy(new_transforms, game_renderer->transforms, size);
	}
	game_renderer->transforms = new_transforms;

	game_renderer->transform_arenas[old_index].used = 0;
	game_renderer->transform_arena_index = new_index;
}

static s_framebuffer* make_framebuffer_with_existing_depth(s_game_renderer* game_renderer, s_v2i size, s_texture depth)
{
	s_framebuffer result = {};
	result.depth_is_borrowed = true;

	if(size.x <= 0) {
		size.x = g_platform_data.window_width;
	}
	if(size.y <= 0) {
		size.y = g_platform_data.window_height;
	}

	gl(glBindFramebuffer(GL_FRAMEBUFFER, 0));
	gl(glGenFramebuffers(1, &result.gpu_id));
	gl(glBindFramebuffer(GL_FRAMEBUFFER, result.gpu_id));

	gl(glGenTextures(1, &result.texture.gpu_id));
	gl(glBindTexture(GL_TEXTURE_2D, result.texture.gpu_id));
	gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, size.x, size.y, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR));
	gl(glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR));
	gl(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, result.texture.gpu_id, 0));

	result.depth = depth;
	gl(glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, depth.gpu_id, 0));

	assert(glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE);
	gl(glBindFramebuffer(GL_FRAMEBUFFER, 0));

	result.texture.size.x = (float)size.x;
	result.texture.size.y = (float)size.y;
	result.texture.game_id = game_renderer->textures.count;
	result.texture.comes_from_framebuffer = true;
	game_renderer->textures.add(result.texture);

	int index = 0;
	for(index = 0; index < c_max_framebuffers; index++) {
		if(game_renderer->framebuffer_active_arr[index]) { continue; }
		result.game_id = index;
		game_renderer->framebuffers[index] = result;
		game_renderer->framebuffer_active_arr[index] = true;
		break;
	}

	after_making_framebuffer(result.game_id, game_renderer);

	return &game_renderer->framebuffers[index];
}

static s_framebuffer* make_framebuffer(s_game_renderer* game_renderer, s_v2i size)
{
	if(size.x <= 0) {
		size.x = g_platform_data.window_width;
	}
	if(size.y <= 0) {
		size.y = g_platform_data.window_height;
	}

	s_texture depth = {};
	gl(glGenTextures(1, &depth.gpu_id));
	gl(glBindTexture(GL_TEXTURE_2D, depth.gpu_id));
	gl(glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, size.x, size.y, 0, GL_DEPTH_STENCIL, GL_UNSIGNED_INT_24_8, NULL));
	s_framebuffer* result = make_framebuffer_with_existing_depth(game_renderer, size, depth);
	result->depth_is_borrowed = false;
	return result;
}

// static void delete_framebuffer(s_game_renderer* game_renderer, s_framebuffer* fbo)
// {
// 	glDeleteTextures(1, &fbo->texture.gpu_id);
// 	if(!fbo->depth_is_borrowed) {
// 		glDeleteTextures(1, &fbo->depth.gpu_id);
// 	}
// 	glDeleteFramebuffers(1, &fbo->gpu_id);

// 	b8 found = false;

// 	for(int i = 0; i < c_max_framebuffers; i++) {
// 		if(!game_renderer->framebuffer_active_arr[i]) { continue; }
// 		if(&game_renderer->framebuffers[i] != fbo) { continue; }
// 		game_renderer->framebuffer_active_arr[i] = false;
// 		found = true;
// 		break;
// 	}
// 	assert(found);

// 	found = false;
// 	// @Note(tkap, 09/06/2024): Delete old texture
// 	foreach_val(texture_i, texture, game_renderer->textures) {
// 		if(texture.game_id == fbo->texture.game_id) {
// 			game_renderer->textures.remove_and_swap(texture_i--);
// 			found = true;
// 			break;
// 		}
// 	}
// 	assert(found);
// }

static void after_making_framebuffer(int index, s_game_renderer* game_renderer)
{
	// @Note TODO(tkap, 27/10/2023): Is this even needed???
	// int size = sizeof(*game_renderer->framebuffers[0].transforms) * game_renderer->textures.count * e_blend_mode_count;
	// s_bucket_array<s_transform>* new_transforms = (s_bucket_array<s_transform>*)la_get_zero(
		// &game_renderer->transform_arenas[game_renderer->transform_arena_index], size
	// );
	// @Note(tkap, 26/10/2023): Aren't we already doing this in after loading texture??
	// s_framebuffer* framebuffer = &game_renderer->framebuffers[index];
	// framebuffer->transforms = new_transforms;

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
static void live_variable_(s_sarray<s_var, 128>* vars, t* ptr, char* name, t min_val, t max_val, b8 display)
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

struct s_time_data
{
	int minutes;
	int seconds;
	int ms;
};

static s_time_data process_time(f64 time)
{
	s_time_data data = {};
	data.minutes = (int)floor(time / 60);
	time -= data.minutes * 60;
	data.seconds = (int)floor(time);
	time -= data.seconds;
	data.ms = (int)floor(time * 1000);
	return data;
}

static b8 is_bit_index_set(int a, int b)
{
	assert(b <= 31);
	return (a & (1 << b)) != 0;
}

static int to_bit(int a)
{
	return 1 << a;
}

static void start_render_pass(s_game_renderer* game_renderer)
{
	assert(!game_renderer->in_render_pass);
	game_renderer->in_render_pass = true;
}

#ifndef m_game

static void handle_key_event(int key, b8 is_down, b8 is_repeat)
{
	if(is_down) {
		g_platform_data.any_key_pressed = true;
	}
	if(key < c_max_keys) {
		if(!is_repeat) {
			s_stored_input si = {};
			si.key = key;
			si.is_down = is_down;
			apply_event_to_input(&g_platform_data.logic_input, si);
			apply_event_to_input(&g_platform_data.render_input, si);
			g_platform_data.any_key_pressed = true;
		}

		#ifdef m_debug
		if(g_platform_data.recording_input && key != c_key_f10 && !is_repeat) {
			s_foo ri = {};
			ri.input = {.is_down = is_down, .key = key};
			ri.update_count = g_platform_data.update_count;
			g_platform_data.recorded_input.keys.add(ri);
		}
		#endif // m_debug

		{
			s_key_event key_event = {};
			key_event.went_down = is_down;
			key_event.key = key;
			key_event.modifiers |= e_input_modifier_ctrl * is_key_down(&g_platform_data.logic_input, c_key_left_ctrl);
			g_platform_data.logic_input.key_events.add(key_event);
			g_platform_data.render_input.key_events.add(key_event);
		}
	}
}

#ifdef __EMSCRIPTEN__


static void submit_leaderboard_success(emscripten_fetch_t *fetch) {
	// printf("Finished downloading %llu bytes\n", fetch->numBytes);
	// ((char*)fetch->data)[fetch->numBytes] = 0;
	// printf("Data: %s\n", fetch->data);
	// s_json* json = parse_json((char*)fetch->data);
	// // We're done with the fetch, so free it
	emscripten_fetch_close(fetch);
	g_platform_data.submit_leaderboard_score_callback();
}


static void submit_leaderboard_score(int time, int leaderboard_id, t_submit_leaderboard_score_callback submit_leaderboard_score_callback)
{
	if(!g_platform_data.leaderboard_session_token) { return; }

	g_platform_data.submit_leaderboard_score_callback = submit_leaderboard_score_callback;

	emscripten_fetch_attr_t attr = {};
	emscripten_fetch_attr_init(&attr);
	strcpy(attr.requestMethod, "POST");
	attr.onsuccess = submit_leaderboard_success;
	attr.onerror = failure;
	attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

	s_len_str data = format_text("{\"score\": %i}", time);
	char* headers[] = {"x-session-token", g_platform_data.leaderboard_session_token, NULL};
	attr.requestHeaders = headers;
	attr.requestData = data.str;
	attr.requestDataSize = data.len;
	s_len_str url = format_text("https://api.lootlocker.io/game/leaderboards/%i/submit", leaderboard_id);
	emscripten_fetch(&attr, url.str);
}

static void get_leaderboard_success(emscripten_fetch_t *fetch) {
	((char*)fetch->data)[fetch->numBytes] = 0;
	s_json* json = parse_json((char*)fetch->data);
	// We're done with the fetch, so free it
	emscripten_fetch_close(fetch);
	g_platform_data.get_leaderboard_callback(json);
}

static void get_our_leaderboard_success(emscripten_fetch_t *fetch) {
	((char*)fetch->data)[fetch->numBytes] = 0;
	s_json* json = parse_json((char*)fetch->data);
	// We're done with the fetch, so free it
	emscripten_fetch_close(fetch);
	g_platform_data.get_our_leaderboard_callback(json);
}


static void get_leaderboard(int leaderboard_id, t_get_leaderboard_callback get_leaderboard_callback)
{
	if(!g_platform_data.leaderboard_session_token) { return; }

	{
		g_platform_data.get_leaderboard_callback = get_leaderboard_callback;
		emscripten_fetch_attr_t attr = {};
		emscripten_fetch_attr_init(&attr);
		strcpy(attr.requestMethod, "GET");
		attr.onsuccess = get_leaderboard_success;
		attr.onerror = failure;
		attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

		char* headers[] = {"x-session-token", g_platform_data.leaderboard_session_token, NULL};
		attr.requestHeaders = headers;
		s_len_str url = format_text("https://api.lootlocker.io/game/leaderboards/%i/list?count=10", leaderboard_id);
		emscripten_fetch(&attr, url.str);
	}
}

static void get_our_leaderboard(int leaderboard_id, t_get_our_leaderboard_callback get_our_leaderboard_callback)
{
	if(!g_platform_data.leaderboard_session_token) { return; }

	{
		g_platform_data.get_our_leaderboard_callback = get_our_leaderboard_callback;
		emscripten_fetch_attr_t attr = {};
		emscripten_fetch_attr_init(&attr);
		strcpy(attr.requestMethod, "GET");
		attr.onsuccess = get_our_leaderboard_success;
		attr.onerror = failure;
		attr.attributes = EMSCRIPTEN_FETCH_LOAD_TO_MEMORY;

		char* headers[] = {"x-session-token", g_platform_data.leaderboard_session_token, NULL};
		attr.requestHeaders = headers;
		s_len_str url = format_text("https://api.lootlocker.io/game/leaderboards/%i/member/%i", leaderboard_id, g_platform_data.leaderboard_player_id);
		emscripten_fetch(&attr, url.str);
	}
}

#endif // __EMSCRIPTEN__

static void end_render_pass(s_game_renderer* game_renderer, s_render_pass render_pass)
{
	assert(game_renderer->in_render_pass);
	s_framebuffer* framebuffer = render_pass.framebuffer;
	if(!framebuffer) {
		framebuffer = &game_renderer->framebuffers[0];
	}
	s_platform_renderer* platform_renderer = &g_platform_renderer;
	game_renderer->in_render_pass = false;
	if(render_pass.do_depth) {
		gl(glEnable(GL_DEPTH_TEST));
	}
	else {
		gl(glDisable(GL_DEPTH_TEST));
	}
	if(render_pass.blend_mode == e_blend_mode_normal) {
		gl(glEnable(GL_BLEND));
		gl(glBlendFunc(GL_ONE, GL_ONE_MINUS_SRC_ALPHA));
	}
	else if(render_pass.blend_mode == e_blend_mode_additive) {
		gl(glEnable(GL_BLEND));
		gl(glBlendFunc(GL_ONE, GL_ONE));
	}
	else if(render_pass.blend_mode == e_blend_mode_multiply) {
		gl(glEnable(GL_BLEND));
		gl(glBlendFunc(GL_DST_COLOR, GL_ZERO));
	}
	else {
		gl(glDisable(GL_BLEND));
	}
	if(render_pass.do_cull) {
		gl(glEnable(GL_CULL_FACE));
		glCullFace(GL_BACK);
	}
	else {
		gl(glDisable(GL_CULL_FACE));
	}

	#if m_multisample
	glEnable(GL_MULTISAMPLE);
	#else
	glDisable(GL_MULTISAMPLE);
	#endif

	if(game_renderer->did_we_alloc) {
		int new_index = (game_renderer->arena_index + 1) % 2;
		assert(game_renderer->arenas[new_index].used == 0);
		for(int shader_i = 0; shader_i < e_shader_count; shader_i++) {
			for(int texture_i = 0; texture_i < game_renderer->textures.count; texture_i++) {
				int offset = get_render_offset(game_renderer, shader_i, texture_i);
				bucket_merge(&game_renderer->transforms[offset], &game_renderer->arenas[new_index]);
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

	gl(glBindFramebuffer(GL_FRAMEBUFFER, framebuffer->gpu_id));
	if(render_pass.do_clear) {
		if(framebuffer->is_main) {
			s_recti rect = do_letter_boxing((int)g_base_res.x, (int)g_base_res.y, g_platform_data.window_width, g_platform_data.window_height);
			gl(glViewport(rect.x, rect.y, rect.width, rect.height));
		}
		else {
			gl(glViewport(0, 0, (int)framebuffer->texture.size.x, (int)framebuffer->texture.size.y));
		}
		glDepthMask(GL_TRUE);
		gl(glClearColor(render_pass.clear_color.x, render_pass.clear_color.y, render_pass.clear_color.z, render_pass.clear_color.w));
		gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
	}

	if(render_pass.dont_write_depth) {
		glDepthMask(GL_FALSE);
	}
	else {
		glDepthMask(GL_TRUE);
	}

	gl(glBindVertexArray(platform_renderer->default_vao));
	gl(glBindBuffer(GL_ARRAY_BUFFER, platform_renderer->default_vbo));

	for(int shader_i = 0; shader_i < e_shader_count; shader_i++) {
		gl(glUseProgram(platform_renderer->programs[shader_i]));

		{
			int location = gl(glGetUniformLocation(platform_renderer->programs[shader_i], "window_size"));
			s_v2 window_size = v2(g_platform_data.window_width, g_platform_data.window_height);
			gl(glUniform2fv(location, 1, &window_size.x));
		}
		{
			int location = gl(glGetUniformLocation(platform_renderer->programs[shader_i], "base_res"));
			gl(glUniform2fv(location, 1, &g_base_res.x));
		}
		{
			int location = gl(glGetUniformLocation(platform_renderer->programs[shader_i], "time"));
			gl(glUniform1f(location, (float)game_renderer->total_time));
		}
		{
			int location = gl(glGetUniformLocation(platform_renderer->programs[shader_i], "mouse"));
			gl(glUniform2fv(location, 1, &g_platform_data.mouse.x));
		}

		{
			int location = gl(glGetUniformLocation(platform_renderer->programs[shader_i], "view_projection"));
			gl(glUniformMatrix4fv(location, 1, GL_FALSE, &render_pass.view_projection.elements[0][0]));
		}

		{
			int location = gl(glGetUniformLocation(platform_renderer->programs[shader_i], "cam_pos"));
			if(location != -1) {
				gl(glUniform3fv(location, 1, &render_pass.cam_pos.x));
			}
		}

		for(int texture_i = 0; texture_i < game_renderer->textures.count; texture_i++) {
			gl(glActiveTexture(GL_TEXTURE0));
			gl(glBindTexture(GL_TEXTURE_2D, game_renderer->textures[texture_i].gpu_id));

			int offset = get_render_offset(game_renderer, shader_i, texture_i);
			int count = game_renderer->transforms[offset].element_count[0];

			if(count > platform_renderer->max_elements) {
				platform_renderer->max_elements = double_until_greater_or_equal(platform_renderer->max_elements, count);
				gl(glBufferData(GL_ARRAY_BUFFER, sizeof(s_transform) * platform_renderer->max_elements, NULL, GL_DYNAMIC_DRAW));
			}

			if(count <= 0) { continue; }

			int size = sizeof(*game_renderer->transforms[offset].elements[0]);

			gl(glBufferSubData(GL_ARRAY_BUFFER, 0, size * count, game_renderer->transforms[offset].elements[0]));

			int num_vertices = 0;
			u32 index_buffer = 0;
			if(shader_i == e_shader_default) {
				num_vertices = 6;
				index_buffer = platform_renderer->index_buffer_2d;
			}
			else if(shader_i == e_shader_basic_3d) {
				num_vertices = 36;
				index_buffer = platform_renderer->index_buffer_3d;
			}
			else if(shader_i == e_shader_3d_flat) {
				num_vertices = 6;
				index_buffer = platform_renderer->index_buffer_2d;
			}
			else if(shader_i == e_shader_threshold) {
				num_vertices = 6;
				index_buffer = platform_renderer->index_buffer_2d;
			}
			else if(shader_i == e_shader_blur) {
				num_vertices = 6;
				index_buffer = platform_renderer->index_buffer_2d;
			}
			invalid_else;

			// gl(glDrawArraysInstanced(GL_TRIANGLES, 0, num_vertices, count));
			gl(glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, index_buffer));
			gl(glDrawElementsInstanced(GL_TRIANGLES, num_vertices, GL_UNSIGNED_INT, NULL, count));
			memset(&game_renderer->transforms[offset].element_count, 0, sizeof(game_renderer->transforms[offset].element_count));

			assert(game_renderer->transforms[offset].bucket_count == 1);
		}
	}
}

static void clear_framebuffer(s_framebuffer* fbo, s_v4 clear_color)
{
	assert(!fbo->is_main);
	gl(glBindFramebuffer(GL_FRAMEBUFFER, fbo->gpu_id));
	gl(glViewport(0, 0, (int)fbo->texture.size.x, (int)fbo->texture.size.y));
	glDepthMask(GL_TRUE);
	gl(glClearColor(clear_color.x, clear_color.y, clear_color.z, clear_color.w));
	gl(glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT));
}

#endif // m_game

static float sin_range(float min_val, float max_val, float x)
{
	return lerp(min_val, max_val, sinf(x) * 0.5f + 0.5f);
}

static char* skip_whitespace(char* str)
{
	while(true) {
		if(*str == '0') { break; }
		else if(*str <= ' ') { str += 1; }
		else { break; }
	}
	return str;
}

static s_string_parse parse_string(char* str, b8 do_alloc)
{
	s_string_parse result = {};
	if(*str != '"') { return {}; }
	str += 1;
	result.first_char = str;
	while(true) {
		if(*str == '\0') { return {}; }
		else if(*str == '"' && str[-1] != '\\') {
			result.success = true;
			result.last_char = str - 1;
			result.len = (int)(result.last_char - result.first_char) + 1;
			result.continuation = str + 1;
			if(do_alloc && result.last_char >= result.first_char) {
				result.result = (char*)malloc(result.len + 1);
				memcpy(result.result, result.first_char, result.len);
				result.result[result.len] = 0;
			}
			break;
		}
		else { str += 1; }
	}
	return result;
}

static char* parse_json_key(char** out_str)
{
	char* str = *out_str;
	str = skip_whitespace(str);
	assert(*str == '"');
	str += 1;
	char* start = str;
	char* key = NULL;
	while(true) {
		if(*str == '\0') { assert(false); }
		else if(*str == '"' && str[-1] != '\\') {
			u64 len = str - start;
			key = (char*)malloc(len + 1);
			memcpy(key, start, len);
			key[len] = 0;
			break;
		}
		else { str += 1; }
	}
	str += 1;
	str = skip_whitespace(str);
	assert(*str == ':');
	str += 1;
	*out_str = str;
	return key;
}

static s_json* parse_json_object(char** out_str)
{
	char* str = *out_str;
	str = skip_whitespace(str);
	s_json* result = NULL;
	if(*str == '{') {
		str += 1;
		result = (s_json*)calloc(1, sizeof(*result));
		result->type = e_json_object;
		s_json** curr = &result->object;
		while(true) {
			str = skip_whitespace(str);
			if(*str == '}') { break; }
			char* key = parse_json_key(&str);
			s_json* child = parse_json_object(&str);
			assert(child);
			child->key = key;
			*curr = child;
			curr = &child->next;
			str = skip_whitespace(str);
			if(*str != ',') { break; }
			str += 1;
		}
		str += 1;
	}
	else if(strncmp(str, "true", 4) == 0) {
		result = (s_json*)calloc(1, sizeof(*result));
		result->type = e_json_bool;
		result->bool_val = true;
		str += 4;
	}
	else if(strncmp(str, "false", 5) == 0) {
		result = (s_json*)calloc(1, sizeof(*result));
		result->type = e_json_bool;
		result->bool_val = false;
		str += 5;
	}
	else if(strncmp(str, "null", 4) == 0) {
		result = (s_json*)calloc(1, sizeof(*result));
		result->type = e_json_null;
		str += 4;
	}
	else if(*str == '"') {
		s_string_parse parse = parse_string(str, true);
		assert(parse.success);
		str = parse.continuation;
		result = (s_json*)calloc(1, sizeof(*result));
		result->type = e_json_string;
		result->str = parse.result;
	}
	else if(is_number(*str)) {
		result = (s_json*)calloc(1, sizeof(*result));
		result->type = e_json_integer;
		result->integer = atoi(str);
		while(is_number(*str)) {
			str += 1;
		}
	}
	else if('[') {
		result = (s_json*)calloc(1, sizeof(*result));
		result->type = e_json_array;
		str += 1;
		s_json** curr = &result->array;
		while(true) {
			str = skip_whitespace(str);
			if(*str == ']') { break; }
			s_json* child = parse_json_object(&str);
			assert(child);
			*curr = child;
			curr = &child->next;
			str = skip_whitespace(str);
			if(*str != ',') { break; }
			str += 1;
		}

		assert(*str == ']');
		str += 1;
	}
	*out_str = str;
	return result;
}

static s_json* parse_json(char* str)
{
	return parse_json_object(&str);
}

static void print_json(s_json* json)
{
	assert(json);
	for(s_json* j = json; j; j = j->next) {
		if(j->key) {
			printf("%s: ", j->key);
		}
		switch(j->type) {
			case e_json_bool: {
				printf("%s\n", j->bool_val ? "true" : "false");
			} break;
			case e_json_integer: {
				printf("%i\n", j->integer);
			} break;
			case e_json_string: {
				printf("\"%s\"\n", j->str);
			} break;
			case e_json_object: {
				printf("{\n");
				print_json(j->object);
				printf("}\n");
			} break;
			case e_json_array: {
				printf("[\n");
				print_json(j->array);
				printf("]\n");
			} break;
			case e_json_null: {
				printf("null\n");
			} break;
			invalid_default_case;
		}
	}
}

static s_json* json_get(s_json* json, char* key_name, e_json in_type)
{
	assert(json);
	for(s_json* j = json; j; j = j->next) {
		if(!j->key) {
			if(j->object) {
				return json_get(j->object, key_name, in_type);
			}
		}
		if(j->type == in_type && strcmp(j->key, key_name) == 0) {
			return j;
		}
	}
	return NULL;
}

struct s_percent_data
{
	float percent;
	float percent_inv;
};

static s_percent_data get_percent_data(float time, float duration)
{
	s_percent_data result = {};
	result.percent = time / duration;
	result.percent_inv = 1.0f - result.percent;
	return result;
}

template <typename key_type, typename value_type, int n>
struct s_hashmap
{
	s_carray<b8, n> used;
	s_carray<key_type, n> keys;
	s_carray<value_type, n> values;

	value_type* set(key_type key, value_type value);
	value_type* get(key_type key);
	b8 remove(key_type key);
};

template <typename key_type, typename value_type, int n>
value_type* s_hashmap<key_type, value_type, n>::set(key_type key, value_type value)
{
	u32 base = hash(key);
	int best_index = -1;
	for(int i = 0; i < n; i++) {
		int index = (base + i) % n;
		if(used[index] && keys[index] == key) {
			best_index = index;
			break;
		}
		else if(best_index < 0 && !used[index]) {
			best_index = index;
		}
	}
	assert(best_index >= 0);
	used[best_index] = true;
	keys[best_index] = key;
	values[best_index] = value;
	return &values[best_index];
}

template <typename key_type, typename value_type, int n>
value_type* s_hashmap<key_type, value_type, n>::get(key_type key)
{
	u32 base = hash(key);
	for(int i = 0; i < n; i++) {
		int index = (base + i) % n;
		if(used[index] && keys[index] == key) {
			return &values[index];
		}
	}
	return NULL;
}

// @Note(tkap, 01/05/2024): Returns true if we found and removed it
template <typename key_type, typename value_type, int n>
b8 s_hashmap<key_type, value_type, n>::remove(key_type key)
{
	u32 base = hash(key);
	for(int i = 0; i < n; i++) {
		int index = (base + i) % n;
		if(used[index] && keys[index] == key) {
			used[index] = false;
			return true;
		}
	}
	return false;
}

static s_v2 center_text_on_rect(s_v2 rect_pos, s_v2 rect_size, float font_size, b8 center_x, b8 center_y)
{
	assert(center_x || center_y);
	s_v2 result = rect_pos;
	if(center_x) {
		result.x += rect_size.x * 0.5f;
	}
	if(center_y) {
		result.y += rect_size.y * 0.5f;
		result.y -= font_size * 0.5f;
	}
	return result;
}

static s_len_str alloc_string(void* data, int len)
{
	s_len_str result = {};
	result.len = len;
	result.str = (char*)malloc(len + 1);
	memcpy(result.str, data, len);
	result.str[len] = 0;
	return result;
}

[[nodiscard]]
static float ilerp_clamp(float start, float end, float value)
{
	return ilerp(start, end, clamp(value, start, end));
}


static float handle_advanced_easing(float x, float x_start, float x_end, float duration)
{
	assert(duration > 0);
	assert(duration <= 1);
	x = clamp(ilerp_clamp(x_start, x_end, x) / duration, 0.0f, 1.0f);
	return x;
}

static float ease_in_expo(float x)
{
	if(floats_equal(x, 0)) { return 0; }
	return powf(2, 10 * x - 10);
}

static float ease_in_quad(float x)
{
	return x * x;
}

static float ease_out_quad(float x)
{
	float x2 = 1 - x;
	return 1 - x2 * x2;
}

static float ease_out_expo(float x)
{
	if(floats_equal(x, 1)) { return 1; }
	return 1 - powf(2, -10 * x);
}

static float ease_out_elastic(float x)
{
	constexpr float c4 = (2 * pi) / 3;
	if(floats_equal(x, 0) || floats_equal(x, 1)) { return x; }
	return powf(2, -5 * x) * sinf((x * 5 - 0.75f) * c4) + 1;
}

static float ease_out_elastic2(float x)
{
	constexpr float c4 = (2 * pi) / 3;
	if(floats_equal(x, 0) || floats_equal(x, 1)) { return x; }
	return powf(2, -10 * x) * sinf((x * 10 - 0.75f) * c4) + 1;
}

#define m_advanced_easings \
X(ease_in_expo) \
X(ease_in_quad) \
X(ease_out_quad) \
X(ease_out_expo) \
X(ease_out_elastic) \
X(ease_out_elastic2) \

#define X(name) \
static float name##_advanced(float x, float x_start, float x_end, float target_start, float target_end, float duration) \
{ \
	x = handle_advanced_easing(x, x_start, x_end, duration); \
	return lerp(target_start, target_end, name(x)); \
}
m_advanced_easings
#undef X

static void add_clamp(float* ptr, float to_add, float min_val, float max_val)
{
	*ptr = clamp(*ptr + to_add, min_val, max_val);
}

static void add_clamp_min(float* ptr, float to_add, float min_val)
{
	*ptr = at_least(min_val, *ptr + to_add);
}

static void add_clamp_max(float* ptr, float to_add, float max_val)
{
	*ptr = at_most(max_val, *ptr + to_add);
}

template <int n>
static b8 handle_string_input(s_input_str<n>* str, s_input* input, float time)
{
	b8 result = false;
	if(!str->cursor.valid) {
		str->cursor = maybe(0);
	}
	foreach_val(c_i, c, input->char_events) {
		if(is_alpha_numeric(c) || c == '_') {
			if(!str->str.is_full()) {
				str->str.insert(str->cursor.value, c);
				str->cursor.value += 1;
				str->last_edit_time = time;
				str->last_action_time = str->last_edit_time;
			}
		}
		else if(c == '\b') {
			if(str->cursor.value > 0) {
				str->cursor.value -= 1;
				str->str.remove_char_at(str->cursor.value);
				str->last_edit_time = time;
				str->last_action_time = str->last_edit_time;
			}
		}
	}

	foreach_val(event_i, event, input->key_events) {
		if(!event.went_down) { continue; }
		if(event.key == c_key_enter) {
			result = true;
			str->last_action_time = time;
		}
		else if(event.key == c_key_escape) {
			str->cursor.value = 0;
			str->str.len = 0;
			str->str[0] = 0;
			str->last_edit_time = time;
			str->last_action_time = str->last_edit_time;
		}
	}
	return result;
}

static char* to_cstr(s_len_str str, s_lin_arena* arena)
{
	char* result = (char*)la_get(arena, str.len + 1);
	memcpy(result, str.str, str.len);
	result[str.len] = 0;
	return result;
}