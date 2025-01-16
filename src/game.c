#include "game.h"
#include <raylib.h>
#include <stdlib.h>

int main() {
	InitWindow(game.width, game.height, "Cool game!");
	init();
	atexit(when_exit);
	while(!WindowShouldClose()) {
		activate_event(&e_falling);
		if(!e_falling_bevent.active) {
			e_falling_bevent.active = true;
			e_falling.active = false;
			e_jump.active = true;
		}
		activate_event(&e_jump);
		BeginDrawing();
		update_me(Vector3Scale(me.speed, GetFrameTime()));
		BeginMode3D(me.camera_third);
		ClearBackground(BACK_COLOR);
		DrawCubeV(pos_to_cpos(obs[OBJECT_FLOOR].cb.pos, obs[OBJECT_FLOOR].cb.size), obs[OBJECT_FLOOR].cb.size, DARKPURPLE);
		static const float gridSize = 10.0f;
		DrawGrid(obs[OBJECT_FLOOR].cb.size.x / gridSize, gridSize);
		EndMode3D();
		EndDrawing();
	}
	return 0;
}
