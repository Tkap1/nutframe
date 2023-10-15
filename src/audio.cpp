
func b8 play_sound_if_supported(e_sound sound_id)
{
	if(!g_platform_funcs.play_sound) { return false; }
	f64 time_passed = game->total_time - game->sound_times[sound_id];
	if(time_passed < 0.1f) { return false; }

	s_sound sound = game->sounds[sound_id];
	b8 result = g_platform_funcs.play_sound(sound);
	if(result)
	{
		game->sound_times[sound_id] = game->total_time;
	}
	return result;
}
