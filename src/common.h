
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

func void add_int(s_attrib_handler* handler, int count);
func void add_float(s_attrib_handler* handler, int count);
func void finish(s_attrib_handler* handler);