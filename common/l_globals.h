#ifdef LIGHT_MAIN_FILE
	#define GLOBAL(x) x
#else
	#define GLOBAL(x) extern x
#endif



//GLOBAL(int l_light_list_cursor);
GLOBAL(int l_light_count);
//GLOBAL(light_position_t *l_light_positions);
//GLOBAL(light_params_t *l_light_params);
//GLOBAL(struct bsp_striangle_t *l_light_visible_triangles);
//GLOBAL(char **l_light_names);
//GLOBAL(unsigned int l_cluster_texture);
//GLOBAL(cluster_t *l_clusters);

GLOBAL(unsigned int l_light_uniform_buffer);
GLOBAL(unsigned int l_light_cache_element_buffer);
GLOBAL(unsigned int l_light_cache_shadow_element_buffer);
GLOBAL(unsigned int l_light_cache_shadow_maps[LIGHT_UNIFORM_BUFFER_SIZE]);
GLOBAL(int *l_light_cache_index_buffer_base);
GLOBAL(int *l_light_cache_groups_next);
GLOBAL(int *l_light_cache_frustum_counts);
GLOBAL(int *l_light_cache_index_buffers[LIGHT_UNIFORM_BUFFER_SIZE]);
GLOBAL(light_cache_slot_t l_light_cache[LIGHT_UNIFORM_BUFFER_SIZE]);

GLOBAL(unsigned int l_shadow_map_frame_buffer);
GLOBAL(unsigned int l_shared_shadow_map);
GLOBAL(unsigned int l_indirection_texture);
GLOBAL(unsigned int l_shadow_maps_array);

GLOBAL(mat4_t l_shadow_map_mats[6]);
GLOBAL(mat4_t l_shadow_map_projection_matrix);
GLOBAL(int l_allocd_shadow_map_count);
GLOBAL(struct shadow_map_t *l_shadow_maps);

//GLOBAL(int visible_light_count);
//GLOBAL(int visible_lights[MAX_WORLD_LIGHTS]);
