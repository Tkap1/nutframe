


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
