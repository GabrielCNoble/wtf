/*
	This file exists to allow for quick addition
	and name changes of every global variable used
	by the editor. MAIN_EDITOR_FILE should
	NOT be defined anywhere or the vars won't
	have external linkage, which won't allow them
	to have their values properly updated by other
	parts of the editor...
*/


#ifdef MAIN_EDITOR_FILE
	#define EDITOR_GLOBAL(x) x
#else
	#define EDITOR_GLOBAL(x) extern x
#endif

/* shaders... */
EDITOR_GLOBAL(int ed_brush_pick_shader);
EDITOR_GLOBAL(int ed_pick_shader);
EDITOR_GLOBAL(int ed_pick_brush_face_shader);
EDITOR_GLOBAL(int ed_light_pick_shader);
EDITOR_GLOBAL(int ed_spawn_point_pick_shader);
EDITOR_GLOBAL(int ed_brush_dist_shader);
EDITOR_GLOBAL(int ed_draw_cursors_shader);
EDITOR_GLOBAL(int ed_model_thumbnail_shader);
EDITOR_GLOBAL(int ed_forward_pass_brush_shader);

EDITOR_GLOBAL(int ed_draw_3d_handle);

EDITOR_GLOBAL(int ed_max_selections);
EDITOR_GLOBAL(int ed_selection_count);
EDITOR_GLOBAL(pick_record_t *ed_selections);
EDITOR_GLOBAL(vec3_t ed_3d_cursor_position);
EDITOR_GLOBAL(vec3_t ed_3d_handle_position);
EDITOR_GLOBAL(int ed_3d_handle_flags);
EDITOR_GLOBAL(int ed_3d_handle_pivot_mode);
EDITOR_GLOBAL(int ed_3d_handle_transform_mode);
EDITOR_GLOBAL(int ed_editing_mode);
EDITOR_GLOBAL(float ed_3d_rotation_handle_angles_lut[ROTATION_HANDLE_DIVS][2]);

//EDITOR_GLOBAL(unsigned int ed_pick_framebuffer_id);
//EDITOR_GLOBAL(unsigned int ed_pick_color_texture_id);
//EDITOR_GLOBAL(unsigned int ed_pick_depth_texture_id);

EDITOR_GLOBAL(struct framebuffer_t ed_pick_framebuffer);
EDITOR_GLOBAL(struct framebuffer_t ed_cursors_framebuffer);


