#include "ed_ui_brush.h"
#include "gui.h"
#include "ed_ui.h"

#include "brush.h"
#include "material.h"
#include "bsp_cmp.h"
#include "input.h"

#include "GL/glew.h"
#include "editor.h"

#include <stdio.h>

int ed_brush_properties_window_open = 0;
widget_t *brush_properties_window = NULL;
option_list_t *brush_faces = NULL;

widget_t *brush_face_properties_window = NULL;
dropdown_t *brush_face_properties_material_dropdown = NULL;
option_list_t *brush_face_properties_material_list = NULL;
button_t *brush_face_properties_toggle_uv_edit_window_button = NULL;

mat4_t ed_edit_uv_window_projection_matrix;
float ed_edit_uv_window_zoom = 2.0;
int ed_edit_uv_window_open = 0;
wsurface_t *edit_uv_window = NULL;
 

/* from editor.c */
extern brush_t *ed_selected_brush;
extern int max_selections;
extern int selection_count;
extern pick_record_t *selections;
extern int ed_selected_brush_polygon_index;
extern int ed_selected_brush_polygon_vertex_index;
extern int ed_selected_brush_polygon_edge_index;
extern int ed_selected_brush_selection_index;
extern int ed_editing_mode;


extern int pick_brush_face_shader;

/* from material.c */
extern int material_count;
extern material_t *materials;
extern char **material_names;
extern material_t default_material;
extern char *default_material_name;

/* from input.c */
extern int bm_mouse;

/* from r_main.c */
extern int r_window_width;
extern int r_window_height;

/* from brush.c */
extern brush_t *brushes;



/*##########################################################################################*/

//void brush_face_material_option_list_callback(widget_t *widget)
void editor_BrushFacePropertiesWindowMaterialOptionListCallback(widget_t *widget)
{
	option_t *option;
	brush_t *brush;
	int material_index;
	
	if(widget->type == WIDGET_OPTION)
	{
		option = (option_t *)widget;
		material_index = (int )option->widget.data;
			
		brush_SetFaceMaterial(ed_selected_brush, ed_selected_brush_polygon_index, material_index);
		
		/*if(ed_selected_brush->polygons[ed_selected_brush_polygon_index].material_index < 0)
		{
			brush_face_properties_material_dropdown->widget.var->addr = &default_material_name;
		}
		else
		{*/
		brush_face_properties_material_dropdown->widget.var->addr = &material_names[ed_selected_brush->polygons[ed_selected_brush_polygon_index].material_index];
		//}
		
		//brush_face_properties_material_dropdown->widget.var->addr = &material_names[ed_selected_brush->polygons[ed_selected_brush_polygon_index].material_index];
	}
}

//void brush_face_material_open_uv_window_button_callback(widget_t *widget)
void editor_BrushFacePropertiesWindowOpenBrushUVWindowButtonCallback(widget_t *widget)
{
	
	if(widget->type == WIDGET_BUTTON)
	{
		if(ed_edit_uv_window_open)
		{
			editor_CloseBrushFaceUVWindow();
		}
		else
		{
			editor_OpenBrushFaceUVWindow();
		}
	}
}


vec4_t tex_coord_handle_position = {0.0, 0.0, 0.0, 0.0};

float tex_coord_handle_grab_offset_x = 0.0;
float tex_coord_handle_grab_offset_y = 0.0;

int tex_coord_vert_selected = 0;
int grabbed_handle = 0;

float view_offset_x = 0.0;
float view_offset_y = 0.0;

extern float mouse_dx;
extern float mouse_dy;

//void surface_callback(widget_t *widget)
void editor_BrushFaceUVWindowSurfaceCallback(widget_t *widget)
{
	wsurface_t *surface;
	bsp_polygon_t *polygon;
	int i;
	int j;
	int x;
	int y;
	int index;
	int type;
	float dx;
	float dy;
	pick_record_t pick_record;
	float value[4];
	vec4_t pos;
	//static float a = 0.0;
	if(widget->type == WIDGET_SURFACE)
	{
		
		
		
		if(!ed_selected_brush)
			return;
			
		if(ed_selected_brush_polygon_index < 0)
			return;	
		
		
		if(bm_mouse & MOUSE_WHEEL_UP)
		{
			ed_edit_uv_window_zoom -= 0.5;
			if(ed_edit_uv_window_zoom < 1.0)
			{
				ed_edit_uv_window_zoom = 1.0;
			}
			
		}
		else if(bm_mouse & MOUSE_WHEEL_DOWN)
		{
			ed_edit_uv_window_zoom += 0.5;
		}
			
		
		CreateOrthographicMatrix(&ed_edit_uv_window_projection_matrix, -ed_edit_uv_window_zoom, ed_edit_uv_window_zoom, ed_edit_uv_window_zoom, -ed_edit_uv_window_zoom, -10.0, 10.0, NULL);
				
		polygon = ed_selected_brush->polygons + ed_selected_brush_polygon_index;
		surface = (wsurface_t *)widget;
		
		//gui_SetUpStates(widget);
		
		glDisable(GL_DEPTH_TEST);
			
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadMatrixf(&ed_edit_uv_window_projection_matrix.floats[0][0]);
		
		glMatrixMode(GL_MODELVIEW);
		glPushMatrix();
		glLoadIdentity();
		//glTranslatef(view_offset_x, view_offset_y, 0.0);
		
		if(widget->bm_flags & (WIDGET_JUST_RECEIVED_RIGHT_MOUSE_BUTTON | WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON | WIDGET_HAS_MIDDLE_MOUSE_BUTTON))
		{		
			glLineWidth(8.0);
			glPointSize(16.0);
			
			renderer_SetShader(pick_brush_face_shader);
			glClearColor(0.0, 0.0, 0.0, 0.0);
			gui_EnablePicking(widget);	
			
			
			
			if(widget->bm_flags & WIDGET_JUST_RECEIVED_LEFT_MOUSE_BUTTON)
			{
				
				if(tex_coord_vert_selected)
				{
					glMatrixMode(GL_PROJECTION);
					glLoadIdentity();
					
					glPointSize(12.0);
					
					value[0] = 1.0;
					value[1] = 1.0;
					value[2] = 1.0;
					value[3] = 0.0;
					
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, value);
					
					glBegin(GL_POINTS);
					glVertex3f(tex_coord_handle_position.x + view_offset_x, tex_coord_handle_position.y + view_offset_y, -0.5);
					glEnd();
					
					
					
					value[0] = 1.0;
					value[1] = 0.0;
					value[2] = 0.0;
					value[3] = 0.0;
					
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, value);
					
					#define TEX_COOR_HANDLE_SIZE 0.25
					glBegin(GL_LINES);
					glVertex3f(tex_coord_handle_position.x, tex_coord_handle_position.y, -0.5);
					glVertex3f(tex_coord_handle_position.x + TEX_COOR_HANDLE_SIZE, tex_coord_handle_position.y, -0.5);
					glEnd();
					
									
					value[0] = 0.0;
					value[1] = 1.0;
					value[2] = 0.0;
					value[3] = 0.0;
					
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, value);
					
					glBegin(GL_LINES);			
					glVertex3f(tex_coord_handle_position.x, tex_coord_handle_position.y, -0.5);
					glVertex3f(tex_coord_handle_position.x, tex_coord_handle_position.y + TEX_COOR_HANDLE_SIZE, -0.5);
					glEnd();
					#undef TEX_COORD_HANDLE_SIZE
					
					
					x = widget->w * 2.0 * (widget->relative_mouse_x * 0.5 + 0.5);
					y = widget->h * 2.0 * (widget->relative_mouse_y * 0.5 + 0.5);
					glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, value);	
					glLoadMatrixf(&ed_edit_uv_window_projection_matrix.floats[0][0]);
					
					tex_coord_handle_grab_offset_x = widget->relative_mouse_x - tex_coord_handle_position.x;
					tex_coord_handle_grab_offset_y = widget->relative_mouse_y - tex_coord_handle_position.y;
					
					
					if(value[0] != 0.0)
					{
						grabbed_handle |= 1;
					}
					
					if(value[1] != 0.0)
					{
						grabbed_handle |= 2;
					}
					
				}
				
				
				
			}
			else if(widget->bm_flags & WIDGET_HAS_MIDDLE_MOUSE_BUTTON)
			{
				view_offset_x += mouse_dx * ed_edit_uv_window_zoom * 2.0;
				view_offset_y += mouse_dy * ed_edit_uv_window_zoom * 2.0;
				//printf("%f %f\n", view_offset_x, view_offset_y);
			}
			else
			{
				*(int *)&value[1] = 0;
				value[2] = 0.0;
				value[3] = 0.0;
				
				for(i = 0; i < polygon->vert_count;)
				{
					*(int *)&value[0] = i + 1;
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, value);
					
					glBegin(GL_LINES);
					glVertex3f(polygon->vertices[i].tex_coord.x * 2.0 - 1.0 + view_offset_x, polygon->vertices[i].tex_coord.y * 2.0 - 1.0 + view_offset_y, -0.5);
					i++;
					glVertex3f(polygon->vertices[i].tex_coord.x * 2.0 - 1.0 + view_offset_x, polygon->vertices[i].tex_coord.y * 2.0 - 1.0 + view_offset_y, -0.5);
					glEnd();
				}
				
				*(int *)&value[1] = 1;
				value[2] = 0.0;
				value[3] = 0.0;
				
				for(i = 0; i < polygon->vert_count; i++)
				{
					*(int *)&value[0] = i + 1;
					glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, value);
					
					glBegin(GL_POINTS);
					glVertex3f(polygon->vertices[i].tex_coord.x * 2.0 - 1.0 + view_offset_x, polygon->vertices[i].tex_coord.y * 2.0 - 1.0 + view_offset_y, -0.5);
					glEnd();
				}
				
				
				
				x = widget->w * 2.0 * (widget->relative_mouse_x * 0.5 + 0.5);
				y = widget->h * 2.0 * (widget->relative_mouse_y * 0.5 + 0.5);
				glReadPixels(x, y, 1, 1, GL_RGBA, GL_FLOAT, value);
				
			
				//printf("%d %d\n", (*(int *)&value[0]) - 1, *(int *)&value[1]);
				index = *(int *)&value[0] - 1;
				type = *(int *)&value[1];
				
				if(index > -1)
				{
					
					if(!(input_GetKeyStatus(SDL_SCANCODE_LSHIFT) & KEY_PRESSED))
					{
						editor_ClearSelection();
					}
					
					tex_coord_vert_selected = 0;
					grabbed_handle = 0;
					
					switch(type)
					{						
						case 1:
							pick_record.type = PICK_UV_VERTEX;
							pick_record.index0 = index;
							editor_AddSelection(&pick_record);			
							tex_coord_vert_selected = 1;		
						break;
					}
				}
			}
							
			gui_DisablePicking(widget);
		}
		
		
		
		
		
		renderer_SetShader(-1);
		gui_SetUpStates(widget);
		
		glClearColor(0.2, 0.2, 0.2, 1.0);
		glClear(GL_COLOR_BUFFER_BIT);
		
		glClearColor(0.0, 0.0, 0.0, 1.0);
		
		glColor3f(1.0, 0.8, 0.0);
		
		glLineWidth(4.0);
		glPointSize(8.0);
		
		
		glBegin(GL_LINE_LOOP);
		for(i = 0; i < polygon->vert_count; i++)
		{
			glVertex3f(polygon->vertices[i].tex_coord.x * 2.0 - 1.0 + view_offset_x, polygon->vertices[i].tex_coord.y * 2.0 - 1.0 + view_offset_y, -0.5);
		}
		glEnd();
		
	
		glBegin(GL_POINTS);
		for(i = 0; i < polygon->vert_count; i++)
		{
			glVertex3f(polygon->vertices[i].tex_coord.x * 2.0 - 1.0 + view_offset_x, polygon->vertices[i].tex_coord.y * 2.0 - 1.0 + view_offset_y, -0.5);
		}
		glEnd();
		
		glMatrixMode(GL_PROJECTION);
		glPopMatrix();
		
		
		if(widget->bm_flags & WIDGET_HAS_LEFT_MOUSE_BUTTON && tex_coord_vert_selected)
		{	
		
			dx = 0.0;
			dy = 0.0;
			
			if(grabbed_handle & 1)
			{
				dx = widget->relative_mouse_x - tex_coord_handle_position.x - tex_coord_handle_grab_offset_x;
			}		
			
			if(grabbed_handle & 2)
			{
				dy = widget->relative_mouse_y - tex_coord_handle_position.y - tex_coord_handle_grab_offset_y;
			}
			
			
			for(i = ed_selected_brush_selection_index + 1; i < selection_count; i++)
			{
				polygon->vertices[selections[i].index0].tex_coord.x += dx;
				polygon->vertices[selections[i].index0].tex_coord.y += dy;
			}
			
			brush_UploadBrushVertices(ed_selected_brush);
			
		}
		else
		{
			grabbed_handle = 0;
		}
		
		
		if(tex_coord_vert_selected)
		//if(ed_selected_brush_polygon_index > -1)
		{
			
			glMatrixMode(GL_PROJECTION);
			glPushMatrix();
			glLoadIdentity();
			
			glPointSize(16.0);	
			glColor3f(0.0, 1.0, 0.0);	
			glBegin(GL_POINTS);
			
			tex_coord_handle_position.x = 0.0;
			tex_coord_handle_position.y = 0.0;
			tex_coord_handle_position.z = 0.0;
			tex_coord_handle_position.w = 0.0;
			
			j = 0;
			
			for(i = ed_selected_brush_selection_index + 1; i < selection_count; i++)
			{
					
				pos.x = polygon->vertices[selections[i].index0].tex_coord.x * 2.0 - 1.0 + view_offset_x;
				pos.y = polygon->vertices[selections[i].index0].tex_coord.y * 2.0 - 1.0 + view_offset_y;
				pos.z = -0.5;
				pos.w = 1.0;
				
				mat4_t_vec4_t_mult(&ed_edit_uv_window_projection_matrix, &pos);
				
				pos.x /= pos.w;
				pos.y /= pos.w;	
				glVertex3f(pos.x, pos.y, -0.5);
				
				tex_coord_handle_position.x += pos.x;
				tex_coord_handle_position.y += pos.y;
		
				j++;
				
			}
			glEnd();
			
			
			tex_coord_handle_position.x /= j;				
			tex_coord_handle_position.y /= j;
			
			
			#define TEX_COOR_HANDLE_SIZE 0.25
			glBegin(GL_LINES);
			
			glColor3f(1.0, 0.0, 0.0);
			glVertex3f(tex_coord_handle_position.x, tex_coord_handle_position.y, -0.5);
			glVertex3f(tex_coord_handle_position.x + TEX_COOR_HANDLE_SIZE, tex_coord_handle_position.y, -0.5);
			
			glColor3f(0.0, 1.0, 0.0);
			glVertex3f(tex_coord_handle_position.x, tex_coord_handle_position.y, -0.5);
			glVertex3f(tex_coord_handle_position.x, tex_coord_handle_position.y + TEX_COOR_HANDLE_SIZE, -0.5);
			
			glEnd();
			#undef TEX_COORD_HANDLE_SIZE 
			
			
			glPointSize(12.0);
			glBegin(GL_POINTS);
			glColor3f(1.0, 1.0, 1.0);
			glVertex3f(tex_coord_handle_position.x, tex_coord_handle_position.y, -0.5);
			glEnd();
			
			
			
			
			
			
			glPopMatrix();	
		}
			
		
		glMatrixMode(GL_MODELVIEW);
		glPopMatrix();
		glEnable(GL_DEPTH_TEST);
		glLineWidth(1.0);
		glPointSize(1.0);
		
		
		gui_RestorePrevStates(widget);
		
		
	}
}

/*##########################################################################################*/






void editor_InitBrushUI()
{
	//brush_properties_window = gui_CreateWidget("Brush properties", r_window_width * 0.5 - LIGHT_PROPERTIES_WINDOW_WIDTH * 0.5, 0, LIGHT_PROPERTIES_WINDOW_WIDTH, LIGHT_PROPERTIES_WINDOW_HEIGHT);
	//brush_properties_window->bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	//brush_face_properties_window = gui_CreateWidget("Brush face", r_window_width * 0.5 - BRUSH_FACE_PROPERTIES_WINDOW_WIDTH * 0.5, r_window_height * 0.5 - MENU_BAR_HEIGHT - BRUSH_FACE_PROPERTIES_WINDOW_HEIGHT, BRUSH_FACE_PROPERTIES_WINDOW_WIDTH, BRUSH_FACE_PROPERTIES_WINDOW_HEIGHT);
	brush_face_properties_window = gui_AddWidget(NULL, "Brush face", r_window_width * 0.5 - BRUSH_FACE_PROPERTIES_WINDOW_WIDTH * 0.5, r_window_height * 0.5 - MENU_BAR_HEIGHT - BRUSH_FACE_PROPERTIES_WINDOW_HEIGHT, BRUSH_FACE_PROPERTIES_WINDOW_WIDTH, BRUSH_FACE_PROPERTIES_WINDOW_HEIGHT);
	brush_face_properties_window->bm_flags |= WIDGET_RENDER_NAME | WIDGET_SHOW_NAME;
	
	brush_face_properties_material_dropdown = gui_AddDropdown(brush_face_properties_window, "Face material", "Face material", 0, BRUSH_FACE_PROPERTIES_WINDOW_HEIGHT * 0.5 - OPTION_HEIGHT * 0.5 - 15, 180, 0, NULL);
	brush_face_properties_material_list = gui_AddOptionList((widget_t *)brush_face_properties_material_dropdown, "Materials", 0, -OPTION_HEIGHT * 0.5, 180, 0, 8, editor_BrushFacePropertiesWindowMaterialOptionListCallback); 
	brush_face_properties_toggle_uv_edit_window_button = gui_AddButton(brush_face_properties_window, "Edit UV", -BRUSH_FACE_PROPERTIES_WINDOW_WIDTH * 0.5 + 45, -BRUSH_FACE_PROPERTIES_WINDOW_HEIGHT * 0.5 + 25, 70, 30, 0, editor_BrushFacePropertiesWindowOpenBrushUVWindowButtonCallback);
	
	gui_var_t *face_material_var = gui_CreateVar("Brush face material", GUI_VAR_STRING, NULL, NULL, 0);
	gui_TrackVar(face_material_var, (widget_t *)brush_face_properties_material_dropdown);
	
	edit_uv_window = gui_AddSurface(NULL, "Edit UV window", r_window_width * 0.5 - UV_WINDOW_WIDTH * 0.5, -r_window_height * 0.5 + UV_WINDOW_HEIGHT * 0.5, UV_WINDOW_WIDTH, UV_WINDOW_HEIGHT, 0, editor_BrushFaceUVWindowSurfaceCallback);
	
	editor_CloseBrushFaceUVWindow();
	editor_CloseBrushFacePropertiesWindow();
	
	
}

void editor_OpenBrushPropertiesWindow()
{
	if(brush_properties_window)
	{
		gui_SetVisible(brush_properties_window);
		ed_brush_properties_window_open = 1;
	}
}
 
void editor_CloseBrushPropertiesWindow()
{
	if(brush_properties_window)
	{
		gui_SetInvisible(brush_properties_window);
		ed_brush_properties_window_open = 0;
	}
}
 
void editor_OpenBrushFacePropertiesWindow(int brush_index, int face_index)
{
	
	brush_t *brush;
	option_t *option;
	int i;
	if(brush_face_properties_window)
	{
		gui_SetVisible(brush_face_properties_window);
		brush = &brushes[brush_index];
		
		gui_RemoveAllOptions(brush_face_properties_material_list);
		
		//option = gui_AddOptionToList(brush_face_properties_material_list, default_material_name, default_material_name);
		//option->widget.data = (void *)-1;
		
		for(i = -1; i < material_count; i++)
		{
			if(materials[i].flags & MATERIAL_INVALID)
			
				continue;
			
			option = gui_AddOptionToList(brush_face_properties_material_list, material_names[i], material_names[i]);
			option->widget.data = (void *)i;
		}
		
		/*if(brush->polygons[face_index].material_index < 0)
		{
			brush_face_properties_material_dropdown->widget.var->addr = &default_material_name;
		}
		else
		{*/
		brush_face_properties_material_dropdown->widget.var->addr = &material_names[brush->polygons[face_index].material_index];
		//}
		
		//
	}
}

void editor_CloseBrushFacePropertiesWindow()
{
	if(brush_face_properties_window)
	{
		gui_SetInvisible(brush_face_properties_window);
		brush_face_properties_material_dropdown->widget.var->addr = NULL;
		editor_CloseBrushFaceUVWindow();
	}
}

void editor_OpenBrushFaceUVWindow()
{
	if(edit_uv_window)
	{
		gui_SetVisible((widget_t *)edit_uv_window);
		ed_edit_uv_window_open = 1;
		editor_SetEditingMode(EDITING_MODE_UV);
		gui_SetAsTop((widget_t *)edit_uv_window);
	}
}

void editor_CloseBrushFaceUVWindow()
{
	if(edit_uv_window)
	{
		gui_SetInvisible((widget_t *)edit_uv_window);	
		ed_edit_uv_window_open = 0;
		ed_selected_brush_polygon_vertex_index = -1;
		tex_coord_vert_selected = 0;
		
		if(ed_editing_mode == EDITING_MODE_UV)
		{
			editor_ClearSelection();
		}
		
		editor_SetEditingMode(EDITING_MODE_BRUSH);
	}
}







