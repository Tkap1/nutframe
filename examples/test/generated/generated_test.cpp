func s_v2i pos_to_index(s_v2 pos, int tile_size);
func s_v2 index_to_pos(s_v2i index, int tile_size);
func void do_particles(int count, s_v2 pos, int z, b8 attached_to_player, s_particle_data data);
func void on_leaderboard_received(s_json* json);
func void on_our_leaderboard_received(s_json* json);
func void on_leaderboard_score_submitted();
func s_m4 get_camera_view(s_camera3d cam);
func s_button_interaction ui_button_interaction(s_len_str id_str, s_v2 pos, s_ui_optional optional);
func b8 ui_button(s_len_str id_str, s_v2 pos, s_ui_optional optional);
func b8 ui_texture_button(s_len_str id_str, s_v2 pos, s_texture texture, s_ui_optional optional);
func void do_button_tooltip(s_len_str description, float font_size);
func b8 ui_button_with_confirmation(s_len_str id_str, s_len_str confirmation_str, s_v2 pos, s_ui_optional optional);
func void on_set_leaderboard_name(b8 success);
func int make_entity(b8* active, int* id, s_entity_index_data* index_data, int max_entities);
func int make_creature(s_v2 pos, int tier, b8 boss);
func int make_deposit(s_v2 pos, int tier);
func int make_bot(s_v2 pos);
func void pick_target_for_bot(int bot);
func int get_creature(s_entity_index index);
func s_damage_creature damage_creature(int creature, int damage);
func void remove_entity(int entity, b8* active, s_entity_index_data* index_data);
func s_get_closest_creature get_closest_creature(s_v2 pos);
func int get_closest_creature2(s_v2 pos, float radius, s_cells* cells, s_lin_arena* arena, s_sarray<int, c_max_player_hits> blacklist);
func s_dynamic_array<int> query_creatures_circle(s_v2 pos, float radius, s_cells* cells, s_lin_arena* frame_arena);
func s_v2 get_center(s_v2 pos, s_v2 size);
func s_pos_area make_pos_area(s_v2 pos, s_v2 size, s_v2 element_size, float spacing, int count, int flags);
func s_pos_area make_horizontal_layout(s_v2 pos, s_v2 element_size, float spacing, int flags);
func s_pos_area make_vertical_layout(s_v2 pos, s_v2 element_size, float spacing, int flags);
func s_v2 pos_area_get_advance(s_pos_area* area);
func s_v2 pos_area_get_advance(s_pos_area* area, float advance_x, float advance_y);
func int get_player_damage();
func int get_bot_damage();
func float get_player_movement_speed();
func float get_bot_movement_speed();
func f64 get_creature_spawn_delay();
func int get_creature_spawn_tier();
func float get_player_harvest_range();
func float get_bot_harvest_range();
func int get_creature_resource_reward(int tier, b8 boss);
func int get_creature_exp_reward(int tier, b8 boss);
func b8 set_state_next_frame(e_state new_state);
func void set_state_next_frame_temporary(e_state new_state);
func int count_alive_creatures();
func void draw_light(s_v2 pos, float radius, s_v4 color, float smoothness);
func void draw_shadow(s_v2 pos, float radius, float strength, float smoothness);
func int get_bot_max_cargo_count();
func s_v2 get_creature_size(int creature);
func float get_creature_size_multi(int creature);
func s_entity_index creature_to_entity_index(int creature);
func s_v2i get_cell_index(s_v2 pos);
func s_bounds get_map_bounds();
func s_bounds get_cam_bounds(s_camera2d cam);
func s_bounds get_cam_bounds_snap_to_tile_size(s_camera2d cam);
func int get_player_hits();
func int get_bot_hits();
func void make_pickup(s_v2 pos, e_pickup type);
func void add_buff(s_player* player, e_pickup pickup);
func b8 has_buff(e_pickup type);
func s_particle_data multiply_particle_data(s_particle_data data, s_particle_multiplier multi);
func int get_player_multi_target();
func void play_sound_group(e_sound_group group_id);
func int count_alive_bots();
func s_len_str get_upgrade_tooltip(e_upgrade id);
func s_v2 wxy(float x, float y);
func s64 get_required_exp_to_level(int level);
func int add_exp(s_player* player, int to_add);
func b8 game_is_paused();
func b8 can_pause();
func b8 can_lose();
func b8 can_go_to_level_up_state();
func b8 should_show_ui();
func int pick_weighted(f64* arr, int count, s_rng* rng);
func float ticks_to_seconds(int ticks);
func s_carray<float, 3> ticks_to_seconds2(int ticks, float interp_dt);
func void draw_laser(s_laser_target target, float laser_light_radius, s_v4 laser_color, float interp_dt);
func void do_options_menu(b8 in_play_mode);
func void do_controls_menu(b8 in_play_mode);
func e_state get_state();
func void go_back_to_prev_state();
func void add_resource(int amount);
func b8 is_mouse_clicked();
func s_ui_data* get_or_create_ui_data(u32 id);
func bool for_ui_data(s_ui_iterator* it);
func s_v2 get_random_creature_spawn_pos();
func b8 counts_towards_defeat(e_creature type);
func b8 can_creature_move(e_creature type);
func int get_creature_max_health(e_creature type, int tier, b8 is_boss);
func float get_nectar_per_second();
func s_auto_timer make_auto_timer(float curr, float duration);
func s_carray<s_v2, 8> get_broken_bot_pos_arr(s_rng* rng);
func int get_upgrade_cost(e_upgrade id);
func void do_leaderboard_stuff();
func void draw_progress_bar(s_v2 pos, s_v2 size, s_v4 under_size, s_v4 over_size, s_len_str str, float progress);
func float get_multiplier(int level, float per_level);
func s_timer make_timer(float curr, float duration);
