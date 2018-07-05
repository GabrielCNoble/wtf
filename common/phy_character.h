#ifndef PHY_CHARACTER_H
#define PHY_CHARACTER_H

#include "phy_common.h"


#ifdef __cplusplus
extern "C"
{
#endif

void physics_Jump(int character_collider_index, float jump_force);

void physics_Move(int character_collider_index, vec3_t direction);

void physics_UpdateCharacterCollider(int character_collider_index);

#ifdef __cplusplus
}
#endif

#endif
