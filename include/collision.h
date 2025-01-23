#pragma once
#include "context.h"
#include <raymath.h>
typedef Vector3 (*ConvMovFunc)(CollisionBox cb1, CollisionBox cb2, Vector3 moving_vector, bool *is_collided);

#ifdef GAME_H
Vector3 convert_mov_vector_aabb_aabb(CollisionBox cb1, CollisionBox cb2, Vector3 moving_vector, bool *is_collided) {
	Vector3 prev_pos = cb1.pos;
	cb1.pos = Vector3Add(cb1.pos, moving_vector);
	Vector3 cb1_max = Vector3Add(cb1.pos, cb1.size);
    Vector3 cb2_max = Vector3Add(cb2.pos, cb2.size);
	*is_collided = false;
	Vector3 inters = {
		MIN(cb1_max.x, cb2_max.x) - MAX(cb1.pos.x, cb2.pos.x),
		MIN(cb1_max.y, cb2_max.y) - MAX(cb1.pos.y, cb2.pos.y),
		MIN(cb1_max.z, cb2_max.z) - MAX(cb1.pos.z, cb2.pos.z),
	};
	if(inters.x <= 0.0f || inters.y <= 0.0f || inters.z <= 0.0f) return moving_vector;
	Vector3 areas = {inters.y * inters.z, inters.x * inters.z, inters.x * inters.y};
	float max_area = MAX3(areas.x, areas.y, areas.z);
	if (max_area == areas.x && (BETWEENSS(cb2.pos.x, cb1.pos.x, cb2_max.x) || BETWEENSS(cb2.pos.x, cb1_max.x, cb2_max.x)))
	{
		*is_collided = true;
		if (moving_vector.x > 0 && cb2.pos.x > prev_pos.x) {
            moving_vector.x = cb2.pos.x - cb1_max.x; // [cb1] -> [cb2]
        } else {
            moving_vector.x = cb2_max.x - prev_pos.x; // [cb2] <- [cb1]
        }
    } else if (max_area == areas.y && (BETWEENSS(cb2.pos.y, cb1.pos.y, cb2_max.y) || BETWEENSS(cb2.pos.y, cb1_max.y, cb2_max.y)))
	{
		*is_collided = true;
		if (moving_vector.y > 0 && cb2.pos.y > prev_pos.y) {
            moving_vector.y = cb2.pos.y - cb1_max.y; // [cb1] -> [cb2]
        } else {
            moving_vector.y = cb2_max.y - prev_pos.y; // [cb2] <- [cb1]
        }
    }
	if (max_area == areas.z && (BETWEENSS(cb2.pos.z, cb1.pos.z, cb2_max.z) || BETWEENSS(cb2.pos.z, cb1_max.z, cb2_max.z)))
	{
		*is_collided = true;
		if (moving_vector.z > 0 && cb2.pos.z > prev_pos.z) {
            moving_vector.z = cb2.pos.z - cb1_max.z; // [cb1] -> [cb2]
        } else {
            moving_vector.z = cb2_max.z - prev_pos.z; // [cb2] <- [cb1]
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
