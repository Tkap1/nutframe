

global constexpr s_v2 c_base_res = {64*12, 64*12};
global constexpr s_v2 c_half_res = {c_base_res.x / 2.0f, c_base_res.y / 2.0f};

global constexpr int c_num_channels = 2;
global constexpr int c_sample_rate = 44100;
global constexpr int c_max_concurrent_sounds = 32;

global constexpr int c_updates_per_second = 60;
global constexpr f64 c_update_delay = 1.0 / c_updates_per_second;

global constexpr s_v2 c_origin_topleft = {1.0f, -1.0f};
global constexpr s_v2 c_origin_bottomleft = {1.0f, 1.0f};
global constexpr s_v2 c_origin_center = {0, 0};

global constexpr float c_delta = 1.0f / c_updates_per_second;

global constexpr int c_base_resolution_index = 5;
global constexpr s_v2i c_resolutions[] = {
	v2i(640, 360),
	v2i(854, 480),
	v2i(960, 540),
	v2i(1024, 576),
	v2i(1280, 720),
	v2i(1366, 768),
	v2i(1600, 900),
	v2i(1920, 1080),
	v2i(2560, 1440),
	v2i(3200, 1800),
	v2i(3840, 2160),
	v2i(5120, 2880),
	v2i(7680, 4320),
};
