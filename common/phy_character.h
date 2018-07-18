#ifndef PHY_CHARACTER_H
#define PHY_CHARACTER_H

#include "phy_common.h"


#ifdef __cplusplus
extern "C"
{
#endif

void physics_Jump(struct collider_handle_t character_collider, float jump_force);

void physics_Move(struct collider_handle_t character_collider, vec3_t direction);

void physics_UpdateCharacterCollider(struct collider_handle_t character_collider);

#ifdef __cplusplus
}
#endif

#endif
