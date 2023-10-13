
global constexpr int c_max_arena_push = 16;

struct s_lin_arena
{
	int push_count;
	u64 push[c_max_arena_push];
	u64 used;
	u64 capacity;
	void* memory;
};

