#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "matrix.h"
#include "GL/glew.h"

#include "gui.h"

#include "path.h"


#include "ed_ui.h"
#include "ed_ui_entity.h"
#include "ed_ui_explorer.h"


#include "model.h"
#include "camera.h"
#include "r_main.h"
#include "gpu.h"
#include "shader.h"
#include "entity.h"

#define ITEM_WIDTH 100
#define ITEM_HEIGHT 120

#define ENTITY_DEF_THUMBNAIL_WIDTH 90
#define ENTITY_DEF_THUMBNAIL_HEIGHT 90

#define ENTITY_DEF_PROPERTIES_VIEWER_THUMBNAIL_WIDTH 250
#define ENTITY_DEF_PROPERTIES_VIEWER_THUMBNAIL_HEIGHT 250

#define ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_WIDTH 182
#define ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_HEIGHT 50


/* from model.c */
extern int model_list_cursor;
extern model_t *models;

/* from entity.c */
extern int ent_entity_def_list_cursor;
extern entity_def_t *ent_entity_defs;

int model_thumbnail_shader;



widget_t *entity_def_viewer_window = NULL;
int ed_entity_def_viewer_window_open = 0;
item_list_t *entity_def_viewer_item_list = NULL;
button_t *entity_def_viewer_load_model_button = NULL;
button_t *entity_def_viewer_spawn_selected_entity_button = NULL;


widget_t *entity_def_properties_viewer_window = NULL;
int ed_entity_def_properties_viewer_window_open = 0;
button_t *entity_def_properties_viewer_delete_def_button = NULL;
button_t *entity_def_properties_viewer_done_button = NULL;
widget_t *entity_def_properties_viewer_thumbnail = NULL;
wsurface_t *entity_def_properties_viewer_thumbnail_surface = NULL;




camera_t *thumbnail_camera = NULL;
int thumbnail_camera_index = -1;

vec3_t thumbnail_camera_position_vector = {1.2, 1.2, 1.2};
mat3_t thumbnail_camera_orientation;

/* from r_main.c */
extern int r_window_width;
extern int r_window_height;

extern vec3_t cursor_3d_position;



void editor_EntityDefViewerLoadModelButtonCallback(widget_t *widget)
{
	button_t *button;
	
	if(widget->type == WIDGET_BUTTON)
	{
		button = (button_t *)widget;
		
		editor_OpenModelExplorer();
		
	}
}

void editor_EntityDefViewerSpawnEntityButtonCallback(widget_t *widget)
{
	button_t *button;
	
	if(widget->type == WIDGET_BUTTON)
	{
		button = (button_t *)widget;
		
		if(entity_def_viewer_item_list->selected_item_index < 0xffff)
		{
			editor_SpawnEntity((int)entity_def_viewer_item_list->selected_item->data);
		}
		
		
	}
}

void editor_ModelExplorerFileClickCallback()
{
	char *file;
	char *name;
	editor_CloseModelExplorer();
	
	while(file = editor_GetExplorerNextSelectedFile())
	{
		name = path_GetFileNameFromPath(file);
		entity_LoadModel(file, name, name, ENTITY_TYPE_MOVABLE);
	}
	
	editor_OpenEntityDefViewerWindow();
	//entity_LoadModel(ed_)
}

void editor_EntityDefViewerItemListCallback(widget_t *widget)
{
	item_list_t *list = (item_list_t *)widget;
	
	if(widget->type == WIDGET_ITEM_LIST)
	{
		if(list->selected_item_index < 0xffff)
		{
			editor_CloseEntityDefViewerWindow();
			editor_OpenEntityDefPropertiesWindow();
		}
	}
}



void editor_InitEntityUI()
{
	
	entity_def_viewer_window = gui_AddWidget(NULL, "Entity defs", 0, -r_window_height * 0.5 + ENTITY_DEF_VIEWER_WINDOW_HEIGHT * 0.5, ENTITY_DEF_VIEWER_WINDOW_WIDTH, ENTITY_DEF_VIEWER_WINDOW_HEIGHT);
	entity_def_viewer_window->bm_flags |= WIDGET_SHOW_NAME | WIDGET_RENDER_NAME;
	entity_def_viewer_load_model_button = gui_AddButton(entity_def_viewer_window, "Load model", ENTITY_DEF_VIEWER_WINDOW_WIDTH * 0.5 - ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_WIDTH * 0.5 - 10, ENTITY_DEF_VIEWER_WINDOW_HEIGHT * 0.5 - ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_HEIGHT * 0.5 - 10, ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_WIDTH, ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_HEIGHT, 0, editor_EntityDefViewerLoadModelButtonCallback);
	entity_def_viewer_spawn_selected_entity_button = gui_AddButton(entity_def_viewer_window, "Spawn entity", ENTITY_DEF_VIEWER_WINDOW_WIDTH * 0.5 - ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_WIDTH * 0.5 - 10, ENTITY_DEF_VIEWER_WINDOW_HEIGHT * 0.5 - ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_HEIGHT * 1.5 - 20, ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_WIDTH, ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_HEIGHT, 0, editor_EntityDefViewerSpawnEntityButtonCallback);
	entity_def_viewer_item_list = gui_AddItemList(entity_def_viewer_window, "Entity defs", 0, 0, ITEM_WIDTH * 5, ENTITY_DEF_VIEWER_WINDOW_HEIGHT, ITEM_LIST_HORIZONTAL_ORDER | ITEM_LIST_DOUBLE_CLICK_SELECTION, editor_EntityDefViewerItemListCallback);
	
	
	widget_t *widget = gui_CreateWidget("item template", 0, 0, ITEM_WIDTH, ITEM_HEIGHT, WIDGET_BASE);
	mat3_t_rotate(&thumbnail_camera_orientation, vec3(1.0, 0.0, 0.0), -0.2, 1);
	mat3_t_rotate(&thumbnail_camera_orientation, vec3(0.0, 1.0, 0.0), 0.25, 0);
	
	thumbnail_camera_index = camera_CreateCamera("thumbnail camera", thumbnail_camera_position_vector, &thumbnail_camera_orientation, 0.5, ENTITY_DEF_THUMBNAIL_WIDTH, ENTITY_DEF_THUMBNAIL_HEIGHT, 0.1, 100.0, 0);
	gui_AddItemToList(widget, entity_def_viewer_item_list);
	
	
	
	entity_def_properties_viewer_window = gui_AddWidget(NULL, "Entity def properties", 0, -r_window_height * 0.5 + ENTITY_DEF_VIEWER_WINDOW_HEIGHT * 0.5, ENTITY_DEF_VIEWER_WINDOW_WIDTH, ENTITY_DEF_VIEWER_WINDOW_HEIGHT);
	entity_def_properties_viewer_window->bm_flags |= WIDGET_SHOW_NAME | WIDGET_RENDER_NAME;
	
	entity_def_properties_viewer_thumbnail = gui_AddWidget(entity_def_properties_viewer_window, "Entity def thumbnail", -ENTITY_DEF_VIEWER_WINDOW_WIDTH * 0.5 + ENTITY_DEF_PROPERTIES_VIEWER_THUMBNAIL_WIDTH * 0.5, ENTITY_DEF_VIEWER_WINDOW_HEIGHT * 0.5 - ENTITY_DEF_PROPERTIES_VIEWER_THUMBNAIL_HEIGHT * 0.5 - 10, ENTITY_DEF_PROPERTIES_VIEWER_THUMBNAIL_WIDTH, ENTITY_DEF_PROPERTIES_VIEWER_THUMBNAIL_HEIGHT);
	entity_def_properties_viewer_thumbnail_surface = gui_AddSurface(entity_def_properties_viewer_thumbnail, "Entity def thumbnail surface", 0, 0, ENTITY_DEF_PROPERTIES_VIEWER_THUMBNAIL_WIDTH, ENTITY_DEF_PROPERTIES_VIEWER_THUMBNAIL_HEIGHT, 0, NULL);
	entity_def_properties_viewer_thumbnail_surface->clear_color = vec4(0.76, 0.76, 1.0, 1.0);
	
	//entity_def_viewer_load_model_button = gui_AddButton(entity_def_viewer_window, "Load model", ENTITY_DEF_VIEWER_WINDOW_WIDTH * 0.5 - ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_WIDTH * 0.5 - 10, ENTITY_DEF_VIEWER_WINDOW_HEIGHT * 0.5 - ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_HEIGHT * 0.5 - 10, ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_WIDTH, ENTITY_DEF_VIEWER_LOAD_MODEL_BUTTON_HEIGHT, 0, editor_EntityDefViewerLoadModelButtonCallback);
	//entity_def_viewer_item_list = gui_AddItemList(entity_def_viewer_window, "Entity defs", 0, 0, ITEM_WIDTH * 5, ENTITY_DEF_VIEWER_WINDOW_HEIGHT, ITEM_LIST_HORIZONTAL_ORDER, NULL);
	
	
	editor_CloseEntityDefViewerWindow();
	editor_CloseEntityDefPropertiesWindow();
}

void editor_OpenEntityDefViewerWindow()
{
	if(entity_def_viewer_window)
	{
		editor_CloseEntityDefPropertiesWindow();
		gui_SetVisible(entity_def_viewer_window);
		editor_EnumerateEntityDefs();
		ed_entity_def_viewer_window_open = 1;
	}
}

void editor_CloseEntityDefViewerWindow()
{
	if(entity_def_viewer_window)
	{
		editor_CloseEntityDefPropertiesWindow();
		gui_SetInvisible(entity_def_viewer_window);
		ed_entity_def_viewer_window_open = 0;
	}
}

void editor_ToggleEntityDefViewerWindow()
{
	if(ed_entity_def_viewer_window_open)
	{
		editor_CloseEntityDefViewerWindow();
	}
	else
	{
		editor_OpenEntityDefViewerWindow();
	}
}

void editor_EnumerateEntityDefs()
{
	int i;
	widget_t *item;
	wsurface_t *surface;
	
	gui_RemoveAllItems(entity_def_viewer_item_list);
	
	
 	for(i = 0; i < ent_entity_def_list_cursor; i++)
	{
		
		if(ent_entity_defs[i].type == ENTITY_TYPE_INVALID)
			continue;
		
		
		item = gui_AddItemToList(NULL, entity_def_viewer_item_list);
		
		if(!item)
		{
			//item = gui_AddWidget((widget_t *)entity_def_viewer_window, "item widget", 0, 0, ITEM_WIDTH, ITEM_HEIGHT);
			item = gui_CreateWidget("item widget", 0, 0, ITEM_WIDTH, ITEM_HEIGHT, WIDGET_BASE);
			item->bm_flags |= WIDGET_NOT_AS_TOP;
			gui_AddItemToList(item, entity_def_viewer_item_list);
			//gui_AddWidget((widget_t *)entity_def_viewer_window, "item widget", 0, 0, ITEM_WIDTH, ITEM_HEIGHT);
		}
		
		if(!item->nestled)
		{
			surface = gui_AddSurface(item, "thumbnail surface", 0, 5, ENTITY_DEF_THUMBNAIL_WIDTH, ENTITY_DEF_THUMBNAIL_HEIGHT, 0, NULL);
			surface->clear_color = vec4(0.76, 0.76, 1.0, 1.0);
		}
		else
		{
			surface = (wsurface_t *)item->nestled;
		}
		
		item->data = (void *)i;
		
		editor_UpdateThumbnail(item);
	}
	
}

void editor_UpdateThumbnail(widget_t *thumbnail)
{
	
	wsurface_t *surface;
	camera_t *camera;
	surface = (wsurface_t *)thumbnail->nestled;
	mat4_t model_view_matrix;
	vec3_t cam_vec;
	vec3_t center;
	entity_def_t *def;
	model_t *model;
	mesh_t *mesh;
	int model_index;
	int entity_def_index;
	int i;
	
	/*float max_x = -9999999999.9;
	float max_y = -9999999999.9;
	float max_z = -9999999999.9;*/
	
	vec3_t max = {-9999999999.9, -9999999999.9, -9999999999.9};
	float d;
	
	
	
	camera = camera_GetCameraByIndex(thumbnail_camera_index);
	//model_index = (int)thumbnail->data;
	entity_def_index = (int)thumbnail->data;
	def = entity_GetEntityDefPointerIndex(entity_def_index);
	model = model_GetModelPointerIndex(def->model_index);
	mesh = model->mesh;
	
	
	center.x = 0.0;
	center.y = 0.0;
	center.z = 0.0;
	
	for(i = 0; i < mesh->vert_count; i++)
	{
		if(mesh->vertices[i].position.x > max.x) max.x = mesh->vertices[i].position.x; 
		if(mesh->vertices[i].position.y > max.y) max.y = mesh->vertices[i].position.y;
		if(mesh->vertices[i].position.z > max.z) max.z = mesh->vertices[i].position.z;
		
		center.x += mesh->vertices[i].position.x;
		center.y += mesh->vertices[i].position.y;
		center.z += mesh->vertices[i].position.z;
		
	}
	
	
	center.x /= mesh->vert_count;
	center.y /= mesh->vert_count;
	center.z /= mesh->vert_count;
	
	d = length3(max);
	
	camera->world_position.x = thumbnail_camera_position_vector.x * d;
	camera->world_position.y = thumbnail_camera_position_vector.y * d;
	camera->world_position.z = thumbnail_camera_position_vector.z * d;
	
	
	cam_vec.x = -center.x + camera->world_position.x;
	cam_vec.y = -center.y + camera->world_position.y;
	cam_vec.z = -center.z + camera->world_position.z;
	
	cam_vec = normalize3(cam_vec);
	
	
	
	//d = acos(dot3(cam_vec, vec3(0.0, 1.0, 0.0))) / 3.14159265;
	
	mat3_t_rotate(&camera->world_orientation, vec3(0.0, 1.0, 0.0), 0.25, 1);
	
	d = acos(dot3(cam_vec, camera->world_orientation.f_axis)) / 3.14159265;
		
	mat3_t_rotate(&camera->world_orientation, camera->world_orientation.r_axis, -d, 0);
	
	
	
	camera_ComputeWorldToCameraMatrix(camera);
	
	
	
	
	
	
	gui_SetUpStates((widget_t *)surface);
	
	renderer_SetShader(-1);
	
	glMatrixMode(GL_PROJECTION);
	glPushMatrix();
	glLoadMatrixf(&camera->projection_matrix.floats[0][0]);
	glMatrixMode(GL_MODELVIEW);
	glLoadMatrixf(&camera->world_to_camera_matrix.floats[0][0]);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
	
	glDisable(GL_CULL_FACE);
	glBegin(GL_QUADS);
	glColor3f(1.0, 1.0, 1.0);
	
	for(i = 0; i <= 10; i++)
	{
		glVertex3f(-i, 0.0,-10.0);
		glVertex3f(-i, 0.0, 10.0);
		glVertex3f( i, 0.0, 10.0);
		glVertex3f( i, 0.0,-10.0);
	}
	
	for(i = 0; i <= 10; i++)
	{
		glVertex3f(-10.0, 0.0, i);
		glVertex3f( 10.0, 0.0, i);
		glVertex3f( 10.0, 0.0,-i);
		glVertex3f(-10.0, 0.0,-i);
	}
	
	glEnd();
	glEnable(GL_CULL_FACE);
	glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	
	gpu_BindGpuHeap();
	
	renderer_SetShader(model_thumbnail_shader);
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_POSITION, 3, (int)&((vertex_t *)0)->position, sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_NORMAL, 3, (int)&((vertex_t *)0)->normal, sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TANGENT, 3, (int)&((vertex_t *)0)->tangent, sizeof(vertex_t));
	renderer_SetVertexAttribPointer(VERTEX_ATTRIB_TEX_COORDS, 2, (int)&((vertex_t *)0)->tex_coord, sizeof(vertex_t));
	
	renderer_SetUniform4fv(UNIFORM_active_camera_position, &camera->world_position.floats[0]);
	
	
	renderer_SetProjectionMatrix(&camera->projection_matrix);
	renderer_SetViewMatrix(&camera->world_to_camera_matrix);
	renderer_SetModelMatrix(NULL);
	renderer_UpdateMatrices();
	
	
	for(i = 0; i < model->triangle_group_count; i++)
	{
		renderer_SetMaterial(model->triangle_groups[i].material_index);
		glDrawArrays(GL_TRIANGLES, model->vert_start + model->triangle_groups[i].start, model->triangle_groups[i].next);
	}

	gpu_UnbindGpuHeap();
	
	glMatrixMode(GL_PROJECTION);
	glPopMatrix();
	glMatrixMode(GL_MODELVIEW);
	
	gui_RestorePrevStates((widget_t *)surface);
	
}

void editor_OpenEntityDefPropertiesWindow()
{
	if(entity_def_properties_viewer_window)
	{
		gui_SetVisible(entity_def_properties_viewer_window);
		gui_SetAsTop(entity_def_properties_viewer_window);
		ed_entity_def_properties_viewer_window_open = 1;
		
		entity_def_properties_viewer_thumbnail->data = entity_def_viewer_item_list->selected_item->data;
		
		editor_UpdateThumbnail(entity_def_properties_viewer_thumbnail);
	}
}

void editor_CloseEntityDefPropertiesWindow()
{
	if(entity_def_properties_viewer_window)
	{
		gui_SetInvisible(entity_def_properties_viewer_window);
		ed_entity_def_properties_viewer_window_open = 0;
	}
}


void editor_OpenModelExplorer()
{
	editor_SetExplorerFileClickCallback(editor_ModelExplorerFileClickCallback);
	editor_ClearExplorerExtensionFilters();
	editor_AddExplorerExtensionFilter(".mpk");
	editor_ExplorerMultiFileSelection(1);
	
	editor_OpenExplorerWindow(NULL);
}


void editor_CloseModelExplorer()
{
	editor_CloseExplorerWindow();
}


void editor_UpdateEntityUI()
{
	entity_def_viewer_window->y = -r_window_height * 0.5 + ENTITY_DEF_VIEWER_WINDOW_HEIGHT * 0.5;
	entity_def_viewer_window->x = ((-r_window_width * 0.5 + TEXTURE_WINDOW_WIDTH * 0.5) + (r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH * 0.5));
	entity_def_viewer_window->w = fabs((-r_window_width * 0.5 + TEXTURE_WINDOW_WIDTH) - (r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH)) / 2.0 - 2;
	
	entity_def_properties_viewer_window->y = -r_window_height * 0.5 + ENTITY_DEF_VIEWER_WINDOW_HEIGHT * 0.5;
	entity_def_properties_viewer_window->x = ((-r_window_width * 0.5 + TEXTURE_WINDOW_WIDTH * 0.5) + (r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH * 0.5));
	entity_def_properties_viewer_window->w = fabs((-r_window_width * 0.5 + TEXTURE_WINDOW_WIDTH) - (r_window_width * 0.5 - MATERIAL_WINDOW_WIDTH)) / 2.0 - 2;
	//entity_def_properties_viewer_thumbnail->w = entity_def_properties_viewer_window->w;
	
	entity_def_properties_viewer_thumbnail->x = -entity_def_properties_viewer_window->w + entity_def_properties_viewer_thumbnail->w + 10;
	
	
	entity_def_viewer_item_list->widget.w = entity_def_viewer_window->w - 100;
	entity_def_viewer_item_list->widget.x = -100;
	entity_def_viewer_item_list->flags |= ITEM_LIST_UPDATE;
	
	
	entity_def_viewer_load_model_button->widget.x = entity_def_viewer_window->w - entity_def_viewer_load_model_button->widget.w - 10;
	entity_def_viewer_load_model_button->widget.y = entity_def_viewer_window->h - entity_def_viewer_load_model_button->widget.h - 10;
	
	entity_def_viewer_spawn_selected_entity_button->widget.x = entity_def_viewer_window->w - entity_def_viewer_spawn_selected_entity_button->widget.w - 10;
	entity_def_viewer_spawn_selected_entity_button->widget.y = entity_def_viewer_window->h - entity_def_viewer_load_model_button->widget.h - 10 - entity_def_viewer_load_model_button->widget.h - entity_def_viewer_load_model_button->widget.h - 10;
}


void editor_SpawnEntity(int entity_def_index)
{
	mat3_t id = mat3_t_id();
	entity_CreateEntity("entity", cursor_3d_position, vec3(1.0, 1.0, 1.0), &id, entity_def_index);
}















