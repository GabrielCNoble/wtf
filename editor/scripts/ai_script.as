

entity_handle_t entity;

entity_handle_t entity_def;

int jump_timer = 0;

float pitch = 0.0;
float yaw = 0.0;

class script_class
{
	float member0;
	float member1;
	float member2;
	float member3;
};

void OnFirstRun()
{
	//script_DebugPrint();
	//script_TestPrint("motherfucker");
	//entity_Print("entity motherfucker");
	//vec3_t end_pos;
	
	script_class sc;
	mat3_t camera_orientation;
	camera_orientation.identity();
	vec3_t prop_value;
	/*end_pos[0] = 0.0;
	end_pos[1] = 0.0;
	end_pos[2] = 10.0;*/
	//entity_FindPath(vec3_t(0.0, 0.0, 25.0));
	
	entity_handle_t camera_entity;
	entity_handle_t weapon_entity;
	
	camera_entity = entity_GetEntity("camera entity", 0);
	
	component_handle_t component = entity_GetEntityComponent(camera_entity, COMPONENT_TYPE_TRANSFORM);
	entity_SetComponentValue3f(component, "position", vec3_t(0.1, 0.15, 0.0));
	entity_SetComponentValue33f(component, "orientation", camera_orientation);
	
	component = entity_GetEntityComponent(camera_entity, COMPONENT_TYPE_CAMERA);
	
	weapon_entity = entity_GetEntity("weapon entity", 0);
	
	//entity_AddEntityProp(weapon_entity, "script class", );
	
	entity_AddEntityProp3f(weapon_entity, "weapon offset");
	
	prop_value[0] = 0.0;
	prop_value[1] = 0.0;
	prop_value[2] = 0.0;
	
	entity_SetEntityPropValue3f(weapon_entity, "weapon offset", prop_value);
	
	entity_SetCamera(component);
	
	pitch = 0.0;
	
}

void OnSpawn()
{

}


void OnDie()
{

}


int spawn_timer = 0;


void main()
{
	/*vec3_t waypoint_direction;
	entity_GetWaypointDirection(waypoint_direction);
	entity_Move(waypoint_direction * 4.0);*/
	
	component_handle_t camera;
	entity_handle_t camera_entity;
	
	vec3_t direction;
	mat3_t orientation;
	mat3_t pitch_matrix;
	vec3_t weapon_offset;
	vec3_t weapon_position;
	
	float mouse_dx;
	float mouse_dy;
	int move = 0;
	int i;
	
	direction[0] = 0.0;
	direction[1] = 0.0;
	direction[2] = 0.0;
	
	if((input_GetKeyStatus(SDL_SCANCODE_W) & KEY_PRESSED) == KEY_PRESSED)
	{
		direction.x += 35.0;
		move = 1;
	}
	
	if((input_GetKeyStatus(SDL_SCANCODE_S) & KEY_PRESSED) == KEY_PRESSED)
	{
		direction.x -= 35.0;
		move = 1;
	}
	
	if((input_GetKeyStatus(SDL_SCANCODE_A) & KEY_PRESSED) == KEY_PRESSED)
	{
		direction.z -= 35.0;
		move = 1;
	}
	
	if((input_GetKeyStatus(SDL_SCANCODE_D) & KEY_PRESSED) == KEY_PRESSED)
	{
		direction.z += 35.0;
		move = 1;
	}
	
	input_GetMouseDelta(mouse_dx, mouse_dy);
	
	pitch += mouse_dy;
	
	if(pitch > 0.5) pitch = 0.5;
	else if(pitch < -0.5) pitch = -0.5;
	
	yaw += mouse_dx;
	
	
	
	//entity_Rotate(vec3_t(0.0, 0.0, 1.0), pitch, 1);
	
	//mat3_t_rotate(pitch_matrix, vec3_t(0.0, 1.0, 0.0), -0.5, 1);
	mat3_t_rotate(pitch_matrix, vec3_t(0.0, 0.0, 1.0), pitch, 1);
	
	
	camera_entity = entity_GetEntity("camera entity", 0);
	
	camera = entity_GetEntityComponent(camera_entity, COMPONENT_TYPE_TRANSFORM);
	entity_SetComponentValue33f(camera, "orientation", pitch_matrix);
	
	entity_Rotate(vec3_t(0.0, 1.0, 0.0), -yaw, 1);
	
	orientation = entity_GetOrientation();
	direction = orientation * direction;

	entity_Move(direction);
	
	entity_handle_t weapon_entity = entity_GetEntity("weapon entity", 0);
	component_handle_t weapon_transform = entity_GetEntityComponent(weapon_entity, COMPONENT_TYPE_TRANSFORM);
	
	weapon_position = vec3_t(0.3, 0.0, 0.15);
	
	entity_GetEntityPropValue3f(weapon_entity, "weapon offset", weapon_offset);
	weapon_offset.x *= 0.9;
	weapon_offset.y *= 0.9;
	
	weapon_offset.x -= mouse_dx * 0.2;
	weapon_offset.y -= mouse_dy * 0.2;
	
	entity_SetEntityPropValue3f(weapon_entity, "weapon offset", weapon_offset);
	
	weapon_position.z += weapon_offset.x;
	weapon_position.y += weapon_offset.y;
	
	entity_SetComponentValue3f(weapon_transform, "position", weapon_position);
	entity_SetComponentValue3f(weapon_transform, "scale", vec3_t(0.3, 0.1, 0.1));
	
	

	//entity_Print("script executed!\n");
	
	if((input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_PRESSED) == KEY_PRESSED && spawn_timer <= 0)
	{
		entity_Jump(250.0);
		/*spawn_timer = 10;
		entity_def = entity_GetEntityDef("cube");
		
		orientation.identity();
		vec3_t scale;
		vec3_t position;
		
		for(i = 0; i < 1; i++)
		{
			scale[0] = 0.1 + randfloat() * 2.0;
			scale[1] = 0.1 + randfloat() * 2.0;
			scale[2] = 0.1 + randfloat() * 2.0;
			
			position[0] = (randfloat() * 2.0 - 1.0) * 10.0;
			position[1] = 20.0;
			position[2] = (randfloat() * 2.0 - 1.0) * 10.0;
	
			entity_SpawnEntity(orientation, position, scale, entity_def, "cube");
		}
		*/
		
	}
	
	//spawn_timer--;
}


