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
