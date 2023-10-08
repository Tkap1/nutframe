
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

template <typename t>
func void bucket_add(s_bucket_array<t>* arr, t new_element, s_lin_arena* arena, b8* did_we_alloc);

template <typename t>
func void bucket_merge(s_bucket_array<t>* arr, s_lin_arena* arena);