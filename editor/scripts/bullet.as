


void main()
{
	if(entity_GetLife() > 150)
	{
		entity_Die();
	}
}

void OnSpawn()
{
	entity_handle_t current_entity = entity_GetCurrentEntity();
	entity_AddEntityProp1i(current_entity, "bullet");
}

void OnDie()
{
	
}

void OnCollision(array<entity_handle_t> @collided_entities)
{
	int i;
	
	for(i = 0; i < collided_entities.count; i++)
	{
		if(entity_EntityHasProp(collided_entities[i], "enemy") != 0)
		{
			entity_Die();
		}
	}
	
}
