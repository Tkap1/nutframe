
struct s_attrib
{
	int type;
	int size;
	int count;
};


struct s_foo
{
	s_sarray<s_attrib, 32> attribs;
};

func void add_int(s_foo* foo, int count);
func void add_float(s_foo* foo, int count);
func void finish(s_foo* foo);
