#pragma once
#include "context.h"
#include <raymath.h>
typedef Vector3 (*ConvMovFunc)(CollisionBox cb1, CollisionBox cb2, Vector3 moving_vector, bool *is_collided);

#ifdef GAME_H
#ifndef BETWEEN
#define BETWEEN(min, x, max) ((min) < (x) && (x) < (max))
#endif
Vector3 convert_mov_vector_aabb_aabb(CollisionBox cb1, CollisionBox cb2, Vector3 moving_vector, bool *is_collided) {
    Vector3 cb1_max = Vector3Add(cb1.pos, cb1.size);
    
    Vector3 cb2_max = Vector3Add(cb2.pos, cb2.size);
	
	*is_collided = false;
	if (BETWEEN(cb2.pos.x - cb1_max.x, moving_vector.x, cb2_max.x - cb1.pos.x) &&
		BETWEEN(cb2.pos.y - cb1_max.y, 0.0f, cb2_max.y - cb1.pos.y) &&
		BETWEEN(cb2.pos.z - cb1_max.z, 0.0f, cb2_max.z - cb1.pos.z)
		) {
        *is_collided = true;
		if (moving_vector.x > 0) {
            moving_vector.x = cb2.pos.x - cb1_max.x; // [cb1] -> [cb2]
        } else {
            moving_vector.x = cb2_max.x - cb1.pos.x; // [cb2] <- [cb1]
        }
    }
	
	if (BETWEEN(cb2.pos.x - cb1_max.x, 0.0f, cb2_max.x - cb1.pos.x) &&
		BETWEEN(cb2.pos.y - cb1_max.y, moving_vector.y, cb2_max.y - cb1.pos.y) &&
		BETWEEN(cb2.pos.z - cb1_max.z, 0.0f, cb2_max.z - cb1.pos.z)) {
        *is_collided = true;
		if (moving_vector.y > 0) {
            moving_vector.y = cb2.pos.y - cb1_max.y; // [cb1] -> [cb2]
        } else {
            moving_vector.y = cb2_max.y - cb1.pos.y; // [cb2] <- [cb1]
        }
    }

	if (BETWEEN(cb2.pos.x - cb1_max.x, 0.0f, cb2_max.x - cb1.pos.x) &&
		BETWEEN(cb2.pos.y - cb1_max.y, 0.0f, cb2_max.y - cb1.pos.y) &&
		BETWEEN(cb2.pos.z - cb1_max.z, moving_vector.z, cb2_max.z - cb1.pos.z)) {
        *is_collided = true;
        if (moving_vector.z > 0) {
            moving_vector.z = cb2.pos.z - cb1_max.z; // [cb1] -> [cb2]
        } else {
            moving_vector.z = cb2_max.z - cb1.pos.z; // [cb2] <- [cb1]
        }
    }

    return moving_vector;
}
ConvMovFunc convert_mov_vector[COLLISION_BOX_COUNT][COLLISION_BOX_COUNT] = {
	[COLLISION_BOX_AABB] = {
		[COLLISION_BOX_AABB] = convert_mov_vector_aabb_aabb,
	},
};
#else
extern ConvMovFunc new_pos[COLLISION_BOX_COUNT][COLLISION_BOX_COUNT];
#endif // GAME_H
