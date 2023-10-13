

#define invalid_default_case default: { assert(false); }
#define invalid_else else { assert(false); }

#define error(b) do { if(!(b)) { printf("ERROR\n"); exit(1); }} while(0)
#define assert(cond) do { if(!(cond)) { on_failed_assert(#cond, __FILE__, __LINE__); } } while(0)
#define check(cond) do { if(!(cond)) { error(false); }} while(0)
#define unreferenced(thing) (void)thing;

#define breakable_block__(a, b) for(int a##b = 1; a##b--;)
#define breakable_block_(a) breakable_block__(tkinternal_condblock, a)
#define breakable_block breakable_block_(__LINE__)

global constexpr s64 c_kb = 1024;
global constexpr s64 c_mb = 1024 * c_kb;
global constexpr s64 c_gb = 1024 * c_mb;
global constexpr s64 c_tb = 1024 * c_gb;

#ifndef _WIN32
#define _declspec(x)
#endif

#define array_count(arr) (sizeof((arr)) / sizeof((arr)[0]))

#ifndef _WIN32
#define max(a,b) (a)<(b)?(b):(a)
#define min(a,b) (a)<(b)?(a):(b)
#endif

#define log(...) printf(__VA_ARGS__); printf("\n")
#define log_info(...) printf(__VA_ARGS__); printf("\n")

func void on_failed_assert(const char* cond, const char* file, int line);

func void* buffer_read(u8** cursor, u64 size)
{
	void* result = *cursor;
	*cursor += size;
	return result;
}

func void buffer_write(u8** cursor, void* data, u64 size)
{
	memcpy(*cursor, data, size);
	*cursor += size;
}

func char* format_text(const char* text, ...)
{
	global constexpr int max_format_text_buffers = 16;
	global constexpr int max_text_buffer_length = 256;

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

func void on_failed_assert(const char* cond, const char* file, int line)
{
	printf("FAILED ASSERT: %s\n", cond);
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


#define foreach_ptr__(a, index_name, element_name, array) if(0) finished##a: ; else for(auto element_name = &(array).elements[0];;) if(1) goto body##a; else while(1) if(1) goto finished##a; else body##a: for(int index_name = 0; index_name < (array).count && (bool)(element_name = &(array)[index_name]); index_name++)
#define foreach_ptr_(a, index_name, element_name, array) foreach_ptr__(a, index_name, element_name, array)
#define foreach_ptr(index_name, element_name, array) foreach_ptr_(__LINE__, index_name, element_name, array)

#define foreach_val__(a, index_name, element_name, array) if(0) finished##a: ; else for(auto element_name = (array).elements[0];;) if(1) goto body##a; else while(1) if(1) goto finished##a; else body##a: for(int index_name = 0; index_name < (array).count && (void*)&(element_name = (array)[index_name]); index_name++)
#define foreach_val_(a, index_name, element_name, array) foreach_val__(a, index_name, element_name, array)
#define foreach_val(index_name, element_name, array) foreach_val_(__LINE__, index_name, element_name, array)

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
			memcpy(elements + index, elements + index + 1, to_move * sizeof(T));
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

template <typename T>
func void swap(T* a, T* b)
{
	T temp = *a;
	*a = *b;
	*b = temp;
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

[[nodiscard]]
func s_v4 rgb(float r, float g, float b)
{
	return v4(r, g, b, 1);
}

[[nodiscard]]
func constexpr s_v4 rgb(int hex)
{
	s_v4 result;
	result.x = ((hex & 0xFF0000) >> 16) / 255.0f;
	result.y = ((hex & 0x00FF00) >> 8) / 255.0f;
	result.z = ((hex & 0x0000FF)) / 255.0f;
	result.w = 1;
	return result;
}

[[nodiscard]]
func constexpr s_v4 rgba(int hex)
{
	s_v4 result;
	result.x = ((hex & 0xFF000000) >> 24) / 255.0f;
	result.y = ((hex & 0x00FF0000) >> 16) / 255.0f;
	result.z = ((hex & 0x0000FF00) >> 8) / 255.0f;
	result.w = ((hex & 0x000000FF) >> 0) / 255.0f;
	return result;
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

func int double_until_greater_or_equal(int current, int target)
{
	assert(target > 0);
	if(current <= 0) { current = 1; }
	while(current < target) { current *= 2; }
	return current;
}