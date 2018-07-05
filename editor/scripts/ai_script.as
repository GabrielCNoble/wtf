

entity_handle_t entity;

int jump_timer = 0;

void OnFirstRun()
{
	script_DebugPrint();
	vec3_t end_pos;
	
	/*end_pos[0] = 0.0;
	end_pos[1] = 0.0;
	end_pos[2] = 10.0;*/
	entity_FindPath(vec3_t(0.0, 0.0, 10.0));
}

float pitch = 0.0;
float yaw = 0.0;

void main()
{
	vec3_t waypoint_direction;
	entity_GetWaypointDirection(waypoint_direction);
	entity_Move(waypoint_direction * 4.0);
	
	/*vec3_t direction;
	mat3_t orientation;
	
	float mouse_dx;
	float mouse_dy;
	
	direction[0] = 0.0;
	direction[1] = 0.0;
	direction[2] = 0.0;
	
	if((input_GetKeyStatus(SDL_SCANCODE_W) & KEY_PRESSED) == KEY_PRESSED)
	{
		direction.x += 10.0;
	}
	
	if((input_GetKeyStatus(SDL_SCANCODE_S) & KEY_PRESSED) == KEY_PRESSED)
	{
		direction.x -= 10.0;
	}
	
	if((input_GetKeyStatus(SDL_SCANCODE_A) & KEY_PRESSED) == KEY_PRESSED)
	{
		direction.z -= 10.0;
	}
	
	if((input_GetKeyStatus(SDL_SCANCODE_D) & KEY_PRESSED) == KEY_PRESSED)
	{
		direction.z += 10.0;
	}
	
	input_GetMouseDelta(mouse_dx, mouse_dy);
	
	pitch += mouse_dy;
	
	if(pitch > 0.5) pitch = 0.5;
	else if(pitch < -0.5) pitch = -0.5;
	
	yaw += mouse_dx;
	
	
	
	entity_Rotate(vec3_t(0.0, 0.0, 1.0), pitch, 1);
	entity_Rotate(vec3_t(0.0, 1.0, 0.0), -yaw, 0);
	
	
	orientation = entity_GetOrientation();
	direction = orientation * direction;
	
	entity_Move(direction);
	
	if((input_GetKeyStatus(SDL_SCANCODE_SPACE) & KEY_JUST_PRESSED) == KEY_JUST_PRESSED)
	{
		entity_Jump(250.0);
	}*/
	
	/*jump_timer++;

	if(jump_timer > 60)
	{
		jump_timer = 0;
		entity_Jump(entity, 500.0);
	}*/
}


