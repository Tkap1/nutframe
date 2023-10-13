
global constexpr int c_str_builder_size = 1 * c_gb;

struct s_str_builder
{
	int tab_count;
	int len;
	char data[c_str_builder_size];
};

func void builder_add_line(s_str_builder* builder, const char* what, ...);
func void builder_add(s_str_builder* builder, const char* what, ...);