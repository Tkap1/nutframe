

#include "tklib.h"


struct s_file_info
{
	char* name;
	char* path;
	char* name_and_path;
};

struct s_dir_search_result
{
	s_sarray<s_file_info, 1024> file_arr;
};


func s_dir_search_result get_all_files_in_directory(char* path, s_lin_arena* arena);

int main()
{
	s_lin_arena arena = make_lin_arena(100 * c_mb, true);
	s_dir_search_result dir = get_all_files_in_directory("examples/speedjam5", &arena);
	foreach_val(file_i, file, dir.file_arr) {
		char* data = read_file_quick(file.name_and_path, &arena);
		printf("%s\n", data);
	}
}

func s_dir_search_result get_all_files_in_directory(char* path, s_lin_arena* arena)
{
	s_dir_search_result result = zero;
	WIN32_FIND_DATAA find_data = zero;
	s_str_sbuilder<MAX_PATH> path_with_star;
	path_with_star.add("%s/*.h", path);
	HANDLE first = FindFirstFileA(path_with_star.cstr(), &find_data);

	if(first == INVALID_HANDLE_VALUE) {
		return result;
	}

	while(true) {
		b8 is_directory = (b8)(find_data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY);
		if(strcmp(find_data.cFileName, ".") == 0) { goto next; }
		if(strcmp(find_data.cFileName, "..") == 0) { goto next; }

		if(is_directory) {
			// s_str_sbuilder<1024> builder;
			// builder.add("%s/%s", directory_path, find_data.cFileName);
			// get_all_files_in_directory_(dynamic_arr, builder.cstr(), arena);
		}
		else {
			s_file_info info = zero;
			int name_len = (int)strlen(find_data.cFileName);
			int path_len = (int)strlen(path);
			{
				info.name = (char*)arena->get_zero(name_len + 1);
				memcpy(info.name, find_data.cFileName, name_len);
			}
			{
				info.path = (char*)arena->get_zero(path_len + 1);
				memcpy(info.path, path, path_len);
			}
			info.name_and_path = (char*)arena->get_zero(name_len + path_len + 1);
			memcpy(info.name_and_path, info.path, path_len);
			strcpy(info.name_and_path + path_len, "/");
			memcpy(info.name_and_path + path_len + 1, info.name, name_len);
			result.file_arr.add(info);
		}

		next:
		if(FindNextFileA(first, &find_data) == 0) {
			break;
		}
	}

	FindClose(first);
	return result;
}
