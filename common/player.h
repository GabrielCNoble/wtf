#ifndef PLAYER_H
#define PLAYER_H

#include "matrix_types.h"
#include "vector_types.h"
#include "camera_types.h"
#include "mesh.h"

#define PLAYER_CAMERA_HEIGHT 0.0
#define PLAYER_MAX_SLOPE_ANGLE 0.707

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



typedef struct
{
	mat3_t player_orientation;
	vec3_t collision_box_position;
	vec3_t player_position;
	vec3_t delta;
	
	
	camera_t *player_camera;
	char *player_name;
	int weapon_start;
	int weapon_count;
	int body_start;
	int body_count;
	float weapon_x_shift;
	float weapon_y_shift;
	float weapon_z_shift;
	
	float pitch;
	float yaw;
	float max_slope;						/* max slope angle cosine */
	float health;
	
	int bm_movement;
	int fire_timer;
	int player_type;
}player_t;


void player_Init();

void player_Finish();

void player_CreatePlayer(char *name, vec3_t position, mat3_t *orientation);

player_t *player_GetPlayer(char *name);

player_t *player_GetActivePlayer();

void player_SetPlayerAsActive(player_t *player);

void player_ProcessActivePlayer(float delta_time);

void player_ProcessAI(float delta_time);

void player_UpdatePlayers(double delta_time);

void player_Move(player_t *player);

void player_TransformPlayers();

#endif
