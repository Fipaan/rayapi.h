#include "raylib.h"
#define GAME_H
#include <context.h>
typedef enum KeybindName {
	KB_CAMERA_TOGGLE,
	KB_FORWARD,
	KB_BACKWARD,
	KB_LEFT,
	KB_RIGHT,
	KB_JUMP,
	KB_SPRINT,
	KB_ZOOMIN3,
	KB_ZOOMOUT3,
	KB_COUNT,
} KeybindName;

Keybind keybind[KB_COUNT] = {
	[KB_CAMERA_TOGGLE].name = "Camera Toggle",
	[KB_FORWARD].name = "Forward",
	[KB_BACKWARD].name = "Backward",
	[KB_LEFT].name = "Left",
	[KB_RIGHT].name = "Right",
	[KB_JUMP].name = "Jump",
	[KB_SPRINT].name = "Sprint",
	[KB_ZOOMIN3].name = "Zoom In(Camera3)",
	[KB_ZOOMOUT3].name = "Zoom Out(Camera3)",
};

typedef enum ObjectName {
	OBJECT_PLAYER,
	OBJECT_FLOOR,
	OBJECT_COUNT,
} ObjectName;

Vars keybinds_v, objects_v;
ObjectBind obs[OBJECT_COUNT] = {
	[OBJECT_PLAYER] = {
		.name = "Player",
	},
	[OBJECT_FLOOR] = {
		.name = "Floor",
	},
};

struct Context ctx = {
	.kb_context = keybind,
	.ob_context = obs,
	.obs_count = OBJECT_COUNT,
	.keybinds_v = &keybinds_v,
	.objects_v = &objects_v,
};

Player me;

#include <rayapi.h>

struct Game {
	int width, height;
} game = {
	.width = 1600,
	.height = 900,
};

void update_me(Vector3 moving_vector) {
	mov_by_index(me.name, moving_vector, OBJECT_COUNT);
	Vector3 bcpos = pos_to_cpos(obs[me.name].cb.pos, obs[me.name].cb.size);
	bcpos.y = obs[me.name].cb.pos.y;
	Vector3 dcamera = Vector3Normalize(Vector3Subtract(me.camera_first.target, me.camera_first.position));
	Vector2 dcamera2d = {dcamera.x, dcamera.z};
	dcamera2d = Vector2Scale(Vector2Normalize(dcamera2d), MIN(obs[me.name].cb.size.x, obs[me.name].cb.size.z) * 0.5f);
	me.camera_first.position = (Vector3) {bcpos.x + dcamera2d.x, bcpos.y + obs[me.name].cb.size.y * CAMERA_HEIGHT, bcpos.z};
	me.camera_first.target = Vector3Add(me.camera_first.position, dcamera);
	me.camera_third.position = Vector3Subtract(pos_to_cpos(obs[me.name].cb.pos, obs[me.name].cb.size), Vector3Scale(dcamera, 30.0f));
	me.camera_third.target = pos_to_cpos(obs[me.name].cb.pos, obs[me.name].cb.size);
}

void set_me_pos(Vector3 new_pos) {
	update_me(Vector3Subtract(new_pos, obs[me.name].cb.pos));
}

bool e_falling_bevent_on_action(Event* self) {
	update_me((Vector3){0.0f, get_falling_floor(me.name) - obs[me.name].cb.pos.y, 0.0f});
	me.speed.y = 0.0f;
	self->active = false;
	return true;
}

Event e_falling_bevent = {
	.ttype = ET_ALWAYS,
	.on_action = e_falling_bevent_on_action,
	.active = true,
};

bool e_falling_on_action(Event* self) {
	(void) self;
	me.speed.y -= CONST_G;	
	return true;
}

Event e_falling = {
	.ttype = ET_NOT_ON_FLOOR,
	.btype = EB_EVENT_FAIL,
	.on_floor_data.name = OBJECT_PLAYER,
	.bevent_data = {
		.event = &e_falling_bevent,
	},
	.on_action = e_falling_on_action,
	.active = true,
};

#include <stdio.h>

bool e_jump_on_action(Event* self) {
	me.speed.y += 100.0f;
	e_falling.active = true;
	self->active = false;
	printf("JUMP!\n");
	return true;
}


Event e_jump = {
	.ttype = ET_CLICK_OR_DOWN | ET_ON_FLOOR,
	.on_floor_data.name = OBJECT_PLAYER,
	.key_data.name = KB_JUMP,
	.on_action = e_jump_on_action,
	.active = true,
};

SI_OPT Player get_default_me(Vector3* rdefault_pos, Vector3* rdefault_size) {
	Vector3 default_size = {10, 20, 10};
	Vector3 default_pos = {45, 200, 45};
	*rdefault_pos = default_pos;
	*rdefault_size = default_size;
	Vector3 default_bcpos = pos_to_cpos(default_pos, default_size);
	default_bcpos.y = default_pos.y;
	Vector3 default_dcamera = Vector3Normalize(Vector3Subtract(Vector3Zero(), (Vector3){50, 200, 50}));
	Vector2 default_dcamera2d = {default_dcamera.x, default_dcamera.z};
	default_dcamera2d = Vector2Scale(Vector2Normalize(default_dcamera2d), MIN(default_size.x, default_size.z) * 0.5f);
	Vector3 default_first_camera_pos = {default_bcpos.x + default_dcamera2d.x, default_bcpos.y + default_size.y * CAMERA_HEIGHT, default_bcpos.z};
	return (Player) {
		.name = OBJECT_PLAYER,
		.speed = {0.0f, 0.0f, 0.0f},
		.camera_first = {
			.position = default_first_camera_pos,
			.target = Vector3Add(default_first_camera_pos, default_dcamera),
			.up = Oy,
			.fovy = CAMERA_FOVY,
			.projection = CAMERA_PERSPECTIVE,
		},
		.camera_third = {
			.position = Vector3Subtract(pos_to_cpos(default_pos, default_size), Vector3Scale(default_dcamera, 30.0f)),
			.target = pos_to_cpos(default_pos, default_size),
			.up = Oy,
			.fovy = CAMERA_FOVY,
			.projection = CAMERA_PERSPECTIVE,
		}
	};
}

void init(void) {
	Vector3 default_pos = {}, default_size = {};
	Player default_me = get_default_me(&default_pos, &default_size);
	read_player(&me, default_me, default_pos, default_size, true);
	read_cb_aabb(OBJECT_FLOOR , (Vector3) {-100, -1, -100}, (Vector3) {200, 1, 200}, true);
	read_file(&keybinds_v, "bin/hotkeys.ini");
	rkb_bind(&keybinds_v, keybind + KB_CAMERA_TOGGLE, KEY_NULL		 );
	rkb_bind(&keybinds_v, keybind + KB_FORWARD		, KEY_W			 );
	rkb_bind(&keybinds_v, keybind + KB_BACKWARD		, KEY_S			 );
	rkb_bind(&keybinds_v, keybind + KB_LEFT			, KEY_A			 );
	rkb_bind(&keybinds_v, keybind + KB_RIGHT		, KEY_D			 );
	rkb_bind(&keybinds_v, keybind + KB_JUMP			, KEY_SPACE		 );
	rkb_bind(&keybinds_v, keybind + KB_SPRINT		, KEY_LEFT_SHIFT );
	rkb_bind(&keybinds_v, keybind + KB_ZOOMIN3		, KEY_KP_ADD	 );
	rkb_bind(&keybinds_v, keybind + KB_ZOOMOUT3		, KEY_KP_SUBTRACT);
}

void when_exit(void) {
	for(int i = 0; i < OBJECT_COUNT; ++i) {
		if(i == OBJECT_PLAYER) continue;
		write_cb_aabb(i);
	}
	write_player(me);
	write_file(keybinds_v, "bin/hotkeys.ini");
	free_vars(&objects_v);
	free_vars(&keybinds_v);
	CloseWindow();
}
