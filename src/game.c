#include "game.h"
#include <raylib.h>
#include <stdlib.h>

int main() {
	InitWindow(game.width, game.height, "Cool game!");
	init();
	atexit(when_exit);
	float ticker = 0.0f;
	while(!WindowShouldClose()) {
		for(int i = 0; i < EVENT_COUNT; ++i) {
			activate_event(event + i);
		}
		update_me(Vector3Scale(me.speed, GetFrameTime()));
		if(IsKeybindPressed(KB_CAMERA_TOGGLE)) {
			game.is_camera_first = !game.is_camera_first;
		}
		SetMousePosition(game.width * 0.5f, game.height * 0.5f);
		BeginDrawing();
		BeginMode3D(game.is_camera_first ? me.camera_first : me.camera_third);
		ClearBackground(BACK_COLOR);
		DrawCubeV(pos_to_cpos(obs[OBJECT_FLOOR].cb.pos, obs[OBJECT_FLOOR].cb.size), obs[OBJECT_FLOOR].cb.size, DARKPURPLE);
		static const float gridSize = 10.0f;
		DrawGrid(obs[OBJECT_FLOOR].cb.size.x / gridSize, gridSize);
		DrawCubeV(pos_to_cpos(obs[OBJECT_CUBE1].cb.pos, obs[OBJECT_CUBE1].cb.size), obs[OBJECT_CUBE1].cb.size, GREEN);
		DrawCubeV(pos_to_cpos(obs[OBJECT_CUBE2].cb.pos, obs[OBJECT_CUBE2].cb.size), obs[OBJECT_CUBE2].cb.size, MAGENTA);
		ticker += GetFrameTime();
		static const float TICKER_CAP = 3.0f;
		static const float TICKER_PERIOD = 2 * PI / TICKER_CAP;
		const float speed = GetFrameTime() * cosf(PI / 2 + ticker * TICKER_PERIOD) * 20.0f * TICKER_PERIOD;
		obs[OBJECT_CUBE3].cb.pos.z += speed;
		if(ticker >= TICKER_CAP) {
			ticker = 0.0f;
		}
		if(ctx.staying_on_name == OBJECT_CUBE3) {
			update_me((Vector3){0.0f, 0.0f, speed});
		}
		DrawCubeV(pos_to_cpos(obs[OBJECT_CUBE3].cb.pos, obs[OBJECT_CUBE3].cb.size), obs[OBJECT_CUBE3].cb.size, PINK);
		if(!game.is_camera_first) {
			DrawCubeV(pos_to_cpos(obs[OBJECT_PLAYER].cb.pos, obs[OBJECT_PLAYER].cb.size), obs[OBJECT_PLAYER].cb.size, RED);
			DrawCubeWiresV(pos_to_cpos(obs[OBJECT_PLAYER].cb.pos, obs[OBJECT_PLAYER].cb.size), obs[OBJECT_PLAYER].cb.size, BLACK);
		}
		EndMode3D();
		EndDrawing();
	}
	return 0;
}
