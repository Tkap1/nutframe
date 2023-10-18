


#ifndef _WIN32
#define max(a,b) (a)<(b)?(b):(a)
#define min(a,b) (a)<(b)?(a):(b)
#endif

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


template <typename T>
func void swap(T* a, T* b)
{
	T temp = *a;
	*a = *b;
	*b = temp;
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
