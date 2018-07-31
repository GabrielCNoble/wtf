

entity_handle_t entity;

entity_handle_t entity_def;

int jump_timer = 0;

float pitch = 0.0;
float yaw = 0.0;


int spawn_timer = 0;

int fire_timer = 10;
int gun = 0;

void main()
{	
	component_handle_t camera_transform;
	entity_handle_t camera_entity;
	
	vec3_t direction;
	mat3_t orientation;
	mat3_t pitch_matrix;
	vec3_t weapon_offset;
	vec3_t weapon_position;
	vec2_t mouse_delta;
	
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
	
	mouse_delta = input_GetMouseDelta();
	
	pitch += mouse_delta.y;
	
	if(pitch > 0.5) pitch = 0.5;
	else if(pitch < -0.5) pitch = -0.5;
	
	yaw += mouse_delta.x;

	mat3_t_rotate(pitch_matrix, vec3_t(0.0, 1.0, 0.0), -0.5, 1);
	mat3_t_rotate(pitch_matrix, vec3_t(0.0, 0.0, 1.0), pitch, 0);
	
	
	camera_entity = entity_GetChildEntity("camera entity");
	
	camera_transform = entity_GetEntityComponent(camera_entity, COMPONENT_TYPE_TRANSFORM);
	entity_SetComponentValue33f(camera_transform, "orientation", pitch_matrix);
	
	entity_Rotate(vec3_t(0.0, 1.0, 0.0), -yaw, 1);
	
	orientation = entity_GetOrientation(0);
	direction = orientation * direction;

	entity_Move(direction);
	
	component_handle_t weapon_transform;
	entity_handle_t right_gun = entity_GetEntityChildEntity(camera_entity, "right gun");
	entity_handle_t left_gun = entity_GetEntityChildEntity(camera_entity, "left gun");
	
	weapon_transform = entity_GetEntityComponent(right_gun, COMPONENT_TYPE_TRANSFORM);
	weapon_offset = entity_GetEntityProp3f(right_gun, "weapon offset");
	weapon_position = entity_GetEntityProp3f(right_gun, "weapon pos");
	weapon_offset.x *= 0.9;
	weapon_offset.y *= 0.9;
	weapon_offset.x -= mouse_delta.x * 0.3;
	weapon_offset.y -= mouse_delta.y * 0.3;
	weapon_position.x += weapon_offset.x;
	weapon_position.y += weapon_offset.y;
	entity_SetEntityProp3f(right_gun, "weapon offset", weapon_offset);
	entity_SetComponentValue3f(weapon_transform, "position", weapon_position);
	
	weapon_transform = entity_GetEntityComponent(left_gun, COMPONENT_TYPE_TRANSFORM);
//	weapon_offset = entity_GetEntityProp3f(left_gun, "weapon offset");
	weapon_position = entity_GetEntityProp3f(left_gun, "weapon pos");	
//	weapon_offset.x *= 0.9;
//	weapon_offset.y *= 0.9;
//	weapon_offset.x -= mouse_delta.x * 0.3;
//	weapon_offset.y -= mouse_delta.y * 0.3;
	weapon_position.x += weapon_offset.x;
	weapon_position.y += weapon_offset.y;
	entity_SetEntityProp3f(left_gun, "weapon offset", weapon_offset);
	entity_SetComponentValue3f(weapon_transform, "position", weapon_position);
	
/*	entity_handle_t weapon_entity = entity_GetChildEntity("weapon entity");
	component_handle_t weapon_transform = entity_GetEntityComponent(weapon_entity, COMPONENT_TYPE_TRANSFORM);
	
	weapon_offset = entity_GetEntityProp3f(weapon_entity, "weapon offset");
	weapon_position = entity_GetEntityProp3f(weapon_entity, "weapon pos");
	weapon_offset.x *= 0.9;
	weapon_offset.y *= 0.9;
	
	weapon_offset.x -= mouse_delta.x * 0.2;
	weapon_offset.y -= mouse_delta.y * 0.2;
	
	entity_SetEntityProp3f(weapon_entity, "weapon offset", weapon_offset);
	
	weapon_position.x += weapon_offset.x;
	weapon_position.y += weapon_offset.y;
	
	entity_SetComponentValue3f(weapon_transform, "position", weapon_position);*/
	
	
	
	
	if((input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_JUST_PRESSED) == KEY_JUST_PRESSED)
	{
		entity_Jump(10.0);
	}
	
	
	if(fire_timer < 10)
	{
		fire_timer++;
	}
	
	if((input_GetMouseStatus() & MOUSE_LEFT_BUTTON_CLICKED) != 0 && fire_timer >= 1)
	{
		fire_timer = 0;
		
		entity_handle_t bullet_def = entity_GetEntityDef("bullet");
		entity_handle_t spawn_entity;
		
		switch(gun)
		{
			case 0:
				gun = 1;
				spawn_entity = entity_GetEntityChildEntity(right_gun, "spawn entity");
			break;
			
			case 1:
				gun = 0;
				spawn_entity = entity_GetEntityChildEntity(left_gun, "spawn entity");
			break;
		}
		
		 
		
		vec3_t spawn_position = entity_GetEntityPosition(spawn_entity, 0);
		
		direction = vec3_t(0.0, 0.0, -1.0);
		direction = pitch_matrix * direction;
		direction = orientation * direction;
		
		orientation.identity();
		
		entity_handle_t bullet_instance = entity_SpawnEntity(orientation, spawn_position, vec3_t(0.2, 0.2, 0.2), bullet_def, "bullet");
		
		entity_SetEntityVelocity(bullet_instance, direction * 80.0);
		
	}
	
/*	if(spawn_timer <= 0)
	{		
		spawn_timer = 10;
		entity_def = entity_GetEntityDef("enemy");
		
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
	
			entity_SpawnEntity(orientation, position, scale, entity_def, "enemy");
		}
		
		
	}
	
	spawn_timer--;*/
}


void OnFirstRun()
{
	
	
}

void OnSpawn()
{
	component_handle_t component;
	mat3_t camera_orientation;
	camera_orientation.identity();
	vec3_t prop_value;
	
	int i;
	
	entity_handle_t body_entity;
	entity_handle_t camera_entity;
	entity_handle_t weapon_entity;
	
	
	body_entity = entity_GetCurrentEntity();
	
	entity_AddEntityProp1i(body_entity, "player");

	camera_entity = entity_GetChildEntity("camera entity");
	
	component = entity_GetEntityComponent(camera_entity, COMPONENT_TYPE_TRANSFORM);
	entity_SetComponentValue3f(component, "position", vec3_t(0.1, 0.35, 0.0));
	mat3_t_rotate(camera_orientation, vec3_t(0.0, 1.0, 0.0), -0.5, 1);
	entity_SetComponentValue33f(component, "orientation", camera_orientation);

	vec3_t weapon_pos;
	entity_handle_t right_gun = entity_GetEntityChildEntity(camera_entity, "right gun");
	entity_handle_t left_gun = entity_GetEntityChildEntity(camera_entity, "left gun");
	
	entity_AddEntityProp3f(right_gun, "weapon offset");
	entity_AddEntityProp3f(right_gun, "weapon pos");
	component = entity_GetEntityComponent(right_gun, COMPONENT_TYPE_TRANSFORM);
	entity_GetComponentValue3f(component, "position", weapon_pos);
	entity_SetEntityProp3f(right_gun, "weapon offset", vec3_t(0.0, 0.0, 0.0));
	entity_SetEntityProp3f(right_gun, "weapon pos", weapon_pos);
	
	
	entity_AddEntityProp3f(left_gun, "weapon offset");
	entity_AddEntityProp3f(left_gun, "weapon pos");
	component = entity_GetEntityComponent(left_gun, COMPONENT_TYPE_TRANSFORM);
	entity_GetComponentValue3f(component, "position", weapon_pos);
	entity_SetEntityProp3f(left_gun, "weapon offset", vec3_t(0.0, 0.0, 0.0));
	entity_SetEntityProp3f(left_gun, "weapon pos", weapon_pos);
	
	/*weapon_entity = entity_GetEntity("weapon entity", 0);
	
	entity_AddEntityProp3f(weapon_entity, "weapon offset");
	entity_AddEntityProp3f(weapon_entity, "weapon pos");
	
	
	component = entity_GetEntityComponent(weapon_entity, COMPONENT_TYPE_TRANSFORM);
	
	vec3_t weapon_pos;
	
	entity_GetComponentValue3f(component, "position", weapon_pos);
	entity_SetEntityProp3f(weapon_entity, "weapon offset", vec3_t(0.0, 0.0, 0.0));
	entity_SetEntityProp3f(weapon_entity, "weapon pos", weapon_pos);
	*/
	
	component = entity_GetEntityComponent(camera_entity, COMPONENT_TYPE_CAMERA);
	
	entity_SetCamera(component);
	
	pitch = 0.0;
	
	
	entity_handle_t entity_def = entity_GetEntityDef("enemy");
		
	mat3_t orientation;
	
	orientation.identity();
	
	for(i = 0; i < 150; i++)
	{
		entity_handle_t enemy = entity_SpawnEntity(orientation, vec3_t(8.0, 2.0 + i * 2, 0.0), vec3_t(1.0, 1.0, 1.0), entity_def, "enemy");
		entity_SetEntityVelocity(enemy, vec3_t(0.0, 0.0, 0.0));
	}
		
	
}


void OnDie()
{
	
}

/*void OnCollision(array<entity_handle_t> @collided_entities)
{

}*/



