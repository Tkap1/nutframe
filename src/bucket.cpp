

template <typename t>
func void bucket_add(s_bucket_array<t>* arr, t new_element, s_lin_arena* arena, b8* did_we_alloc)
{
	for(int i = 0; i < arr->bucket_count; i++)
	{
		int* count = &arr->element_count[i];
		if(*count < arr->capacity[i])
		{
			arr->elements[i][*count] = new_element;
			*count += 1;
			return;
		}
	}
	assert(arr->bucket_count < 16);
	constexpr int capacity = c_bucket_capacity;
	arr->elements[arr->bucket_count] = (t*)la_get(arena, sizeof(t) * capacity);
	arr->capacity[arr->bucket_count] = capacity;
	arr->elements[arr->bucket_count][0] = new_element;
	arr->element_count[arr->bucket_count] = 1;
	arr->bucket_count += 1;

	if(arr->bucket_count > 1)
	{
		*did_we_alloc = true;
	}
}

template <typename t>
func void bucket_merge(s_bucket_array<t>* arr, s_lin_arena* arena)
{
	int capacity = 0;
	int count = 0;
	constexpr int element_size = sizeof(t);
	t* elements;

	if(arr->element_count[0] <= 0) { return; }

	for(int i = 0; i < arr->bucket_count; i++)
	{
		capacity += arr->capacity[i];
	}

	elements = (t*)la_get(arena, element_size * capacity);

	for(int i = 0; i < arr->bucket_count; i++)
	{
		assert(arr->element_count[i] > 0);
		memcpy(&elements[count], arr->elements[i], element_size * arr->element_count[i]);
		count += arr->element_count[i];
	}

	arr->elements[0] = elements;
	arr->capacity[0] = capacity;
	arr->element_count[0] = count;
	arr->bucket_count = 1;
}