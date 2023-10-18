
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

global constexpr int c_max_arena_push = 16;

struct s_lin_arena
{
	int push_count;
	u64 push[c_max_arena_push];
	u64 used;
	u64 capacity;
	void* memory;
};

struct s_framebuffer;
struct s_game_renderer;
struct s_platform_data;

typedef b8 (*t_play_sound)(s_sound*);
typedef void (*t_set_vsync)(b8);
typedef int (*t_show_cursor)(b8);
typedef int (*t_cycle_between_available_resolutions)(int);
typedef u32 (*t_get_random_seed)();
typedef s_framebuffer* (*t_make_framebuffer)(s_game_renderer*, b8);
typedef s_sound* (*t_load_sound)(s_platform_data*, const char*, s_lin_arena*);

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