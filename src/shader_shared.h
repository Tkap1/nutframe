
// @Fixme(tkap, 08/10/2023): This is not even shared anymore
#pragma pack(push, 1)
struct s_transform
{
	int flags;
	int layer;
	int sublayer;
	int effect_id;
	float mix_weight;
	s_v2 pos;
	s_v2 origin_offset;
	s_v2 draw_size;
	s_v2 texture_size;
	s_v2 uv_min;
	s_v2 uv_max;
	s_v4 color;
	s_v4 mix_color;
};
#pragma pack(pop)
