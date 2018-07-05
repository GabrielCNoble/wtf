#ifndef PLAYER_H
#define PLAYER_H

#include "matrix_types.h"
#include "vector_types.h"
#include "camera_types.h"
#include "model.h"
#include "scr_common.h"
#include "ent_common.h"

#include "physics.h"

#define PLAYER_CAMERA_HEIGHT 0.75
//#define PLAYER_CAMERA_HEIGHT 0.0
#define PLAYER_MAX_SLOPE_ANGLE 0.707

#define PLAYER_X_EXTENT 0.5
#define PLAYER_Y_EXTENT 1.0
#define PLAYER_Z_EXTENT 0.5

enum PLAYER_TYPE
{
	PLAYER_ACTIVE,				/* controlled by player... */
	PLAYER_NPC,					/* AI... */
};


enum PLAYER_MOVEMENT
{
	MOVE_FORWARD = 1,
	MOVE_BACKWARD = 1 << 1,
	MOVE_STRAFE_LEFT = 1 << 2,
	MOVE_STRAFE_RIGHT = 1 << 3,
	MOVE_JUMP = 1 << 4,
	
	PLAYER_FLYING = 1 << 5,
	PLAYER_JUMPED = 1 << 6,
	PLAYER_FIRED = 1 << 7,
	PLAYER_ON_GROUND = 1 << 8,
	PLAYER_STEPPING_UP = 1 << 9,
};


enum PLAYER_FLAGS
{
	PLAYER_IN_WORLD = 1,
	PLAYER_DEAD = 1 << 1,
	PLAYER_INVALID = 1 << 2,
};

enum SPAWN_POINT_FLAGS
{
	SPAWN_POINT_INVALID = 1,
};


typedef struct player_def_t
{
	struct player_def_t *next;
	struct player_def_t *prev;
	
	char *name;
	
	collider_def_t *collider_def;
}player_def_t;


struct ai_script_t
{
	struct script_t script;
	void *ai_controller;
	
};



typedef struct
{
	mat3_t player_orientation;
	//vec3_t collision_box_position;
	vec3_t player_position;
	
	int collider_index;
	//vec3_t delta;
	
	
	camera_t *player_camera;
	char *player_name;
	
	int gun_entity_index;
	//int weapon_start;
	//int weapon_count;
	//int body_start;
	//int body_count;
	float weapon_x_shift;
	float weapon_y_shift;
	float weapon_z_shift;
	
	float pitch;
	float yaw;
	float max_slope;						/* max slope angle cosine */
	float health;
	
	short bm_movement;
	short bm_flags;
	int fire_timer;
	int player_type;
}player_t;


typedef struct
{
	vec3_t position;
	int bm_flags;
	char *name;
}spawn_point_t;

#ifdef __cplusplus
extern "C"
{
#endif

int player_Init();

void player_Finish();

/*
========================================================================================
========================================================================================
========================================================================================
*/

player_def_t *player_CreatePlayerDef(char *name, collider_def_t *collider_def);

void player_DestroyPlayerDef(char *name);

player_def_t *player_GetPlayerDefPointer(char *name);

/*
========================================================================================
========================================================================================
========================================================================================
*/

int player_SpawnPlayer(mat3_t *orientation, vec3_t position, vec3_t scale, player_def_t *def);


struct ai_script_t *player_LoadAIScript(char *file_name, char *script_name);

//int player_CreatePlayer(char *name, vec3_t position, mat3_t *orientation);

//void player_DestroyPlayer(char *name);

//void player_DestroyPlayerIndex(int player_index);

//int player_CreateSpawnPoint(vec3_t position, char *name);

//void player_DestroySpawnPoint(int spawn_point_index);

//void player_DestroyAllSpawnPoints();

//void player_SpawnPlayer(int player_index, int spawn_point_index);

//void player_RemovePlayer(int player_index);

//player_t *player_GetPlayer(char *name);

//player_t *player_GetActivePlayer();

//void player_SetPlayerAsActive(player_t *player);

void player_SetPlayerAsActiveIndex(struct entity_handle_t player);

void player_UpdateActivePlayer(double delta_time);

void player_ProcessAI(float delta_time);

void player_UpdatePlayers(double delta_time);

void player_PostUpdatePlayers(double delta_time);

void player_Move(player_t *player, float delta_time);

void player_TransformPlayers();




#ifdef __cplusplus
}
#endif

#endif
