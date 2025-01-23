#include <raylib.h>
#include <raymath.h>
// #include <stdio.h>
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
	OBJECT_CUBE1,
	OBJECT_CUBE2,
	OBJECT_CUBE3,
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
	[OBJECT_CUBE1] = {
		.name = "Cube1",
	},
	[OBJECT_CUBE2] = {
		.name = "Cube2",
	},
	[OBJECT_CUBE3] = {
		.name = "Cube3",
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
	bool is_camera_first;
} game = {
	.width = 1600,
	.height = 900,
	.is_camera_first = true,
};

void update_me(Vector3 moving_vector) {
	mov_by_index(me.name, moving_vector, OBJECT_COUNT);
	Vector3 bcpos = pos_to_cpos(obs[me.name].cb.pos, obs[me.name].cb.size);
	bcpos.y = obs[me.name].cb.pos.y;
	Vector3 dcamera = Vector3Subtract(me.camera_first.target, me.camera_first.position);
	Vector2 dcamera2d = {dcamera.x, dcamera.z};
	dcamera2d = Vector2Scale(Vector2Normalize(dcamera2d), MIN(obs[me.name].cb.size.x, obs[me.name].cb.size.z) * CAMERA_DISTANCE);
	me.camera_first.position = (Vector3) {bcpos.x + dcamera2d.x, bcpos.y + obs[me.name].cb.size.y * CAMERA_HEIGHT, bcpos.z};
	me.camera_first.target = Vector3Add(me.camera_first.position, dcamera);
	me.camera_third.position = Vector3Subtract(pos_to_cpos(obs[me.name].cb.pos, obs[me.name].cb.size), Vector3Scale(dcamera, 30.0f));
	me.camera_third.target = pos_to_cpos(obs[me.name].cb.pos, obs[me.name].cb.size);
}

void set_me_pos(Vector3 new_pos) {
	update_me(Vector3Subtract(new_pos, obs[me.name].cb.pos));
}

typedef enum EventName {
	EVENT_FALLING_ON_FAIL,
	EVENT_FALLING,
	EVENT_FRICTION,
	EVENT_FORWARD,
	EVENT_BACKWARD,
	EVENT_LEFT,
	EVENT_RIGHT,
	EVENT_JUMP,
	EVENT_ROTATE_CAMERA,
	EVENT_COUNT,
} EventName;

bool trigger_mouse_move(Event* self) {
	(void) self;
	return Vector2LengthSqr(GetMouseDelta()) > 0.0f;
}

bool trigger_moving(Event* self) {
	(void) self;
	return me.speed.x * me.speed.x + me.speed.z * me.speed.z > 0.0f;
}

bool e_falling_bevent_on_action(Event* self) {
	(void) self;
	update_me((Vector3){0.0f, get_falling_floor(me.name) - obs[me.name].cb.pos.y, 0.0f});
	me.speed.y = 0.0f;
	return true;
}

bool e_jump_on_action(Event* self) {
	(void) self;
	me.speed.y += 100.0f;
	return true;
}

bool e_friction_on_action(Event* self) {
	(void) self;
	if(me.speed.x * me.speed.x + me.speed.z * me.speed.z == 0.0f) return false;
	me.speed.x *= FRICTION_SCALE;
	me.speed.z *= FRICTION_SCALE;
	if(me.speed.x * me.speed.x + me.speed.z * me.speed.z < ZERO_SPEED) {
		me.speed.x = 0.0f;
		me.speed.z = 0.0f;
	}
	return true;
}

bool e_falling_on_action(Event* self) {
	(void) self;
	me.speed.y -= CONST_G;	
	return true;
}

bool e_forward_on_action(Event* self) {
	(void) self;
	Vector3 dxz = Vector3Subtract(me.camera_first.target, me.camera_first.position);
	dxz.y = 0.0f;
	Vector3 dv = Vector3Scale(Vector3Normalize(dxz), MOVING_SPEED * (1.0f + SPRINT_FACTOR * IsKeybindDownSoft(KB_SPRINT)));
	me.speed.x += dv.x;
	me.speed.z += dv.z;
	return true;
}

bool e_backward_on_action(Event* self) {
	(void) self;
	Vector3 dxz = Vector3Subtract(me.camera_first.target, me.camera_first.position);
	dxz.y = 0.0f;
	Vector3 dv = Vector3Scale(Vector3Normalize(dxz), -MOVING_SPEED * (1.0f + SPRINT_FACTOR * IsKeybindDownSoft(KB_SPRINT)));
	me.speed.x += dv.x;
	me.speed.z += dv.z;
	return true;
}

bool e_left_on_action(Event* self) {
	(void) self;
	Vector3 dxz = Vector3CrossProduct(Oy, Vector3Subtract(me.camera_first.target, me.camera_first.position));
	dxz.y = 0.0f;
	Vector3 dv = Vector3Scale(Vector3Normalize(dxz), MOVING_SPEED * (1.0f + SPRINT_FACTOR * IsKeybindDownSoft(KB_SPRINT)));
	me.speed.x += dv.x;
	me.speed.z += dv.z;
	return true;
}

bool e_right_on_action(Event* self) {
	(void) self;
	Vector3 dxz = Vector3CrossProduct(Oy, Vector3Subtract(me.camera_first.target, me.camera_first.position));
	dxz.y = 0.0f;
	Vector3 dv = Vector3Scale(Vector3Normalize(dxz), -MOVING_SPEED * (1.0f + SPRINT_FACTOR * IsKeybindDownSoft(KB_SPRINT)));
	me.speed.x += dv.x;
	me.speed.z += dv.z;
	return true;
}

bool e_rotate_camera_on_action(Event* self) {
	(void) self;
	Vector2 dpos = Vector2Scale(GetMouseDelta(), 0.5f);
	UpdateCameraPro(&me.camera_first, Vector3Zero(), (Vector3) {dpos.x, dpos.y, 0.0f}, 0.0f);
	return true;
}

Event event[EVENT_COUNT] = {
	[EVENT_FALLING_ON_FAIL] = {
		.ttype = ET_ON_FLOOR,
		.on_action = e_falling_bevent_on_action,
		.active = true,
	},
	[EVENT_FALLING] = {
		.ttype = ET_NOT_ON_FLOOR,
		.btype = EB_EVENT_FAIL,
		.on_floor_data.name = OBJECT_PLAYER,
		.bevent_data = {
			.event = event + EVENT_FALLING_ON_FAIL,
		},
		.on_action = e_falling_on_action,
		.active = true,
	},
	[EVENT_JUMP] = {
		.ttype = ET_CLICK_OR_DOWN | ET_ON_FLOOR,
		.on_floor_data.name = OBJECT_PLAYER,
		.key_data.name = KB_JUMP,
		.on_action = e_jump_on_action,
		.active = true,
	},
	[EVENT_FRICTION] = {
		.ttype = ET_CUSTOM,
		.trigger_custom_data.trigger = trigger_moving,
		.on_action = e_friction_on_action,
		.active = true,
	},
	[EVENT_FORWARD] = {
		.ttype = ET_CLICK_OR_DOWN,
		.key_data.name = KB_FORWARD,
		.on_action = e_forward_on_action,
		.active = true,
	},
	[EVENT_BACKWARD] = {
		.ttype = ET_CLICK_OR_DOWN,
		.key_data.name = KB_BACKWARD,
		.on_action = e_backward_on_action,
		.active = true,
	},
	[EVENT_LEFT] = {
		.ttype = ET_CLICK_OR_DOWN,
		.key_data.name = KB_LEFT,
		.on_action = e_left_on_action,
		.active = true,
	},
	[EVENT_RIGHT] = {
		.ttype = ET_CLICK_OR_DOWN,
		.key_data.name = KB_RIGHT,
		.on_action = e_right_on_action,
		.active = true,
	},
	[EVENT_ROTATE_CAMERA] = {
		.ttype = ET_CUSTOM,
		.trigger_custom_data.trigger = trigger_mouse_move,
		.on_action = e_rotate_camera_on_action,
		.active = true,
	}
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
	default_dcamera2d = Vector2Scale(Vector2Normalize(default_dcamera2d), MIN(default_size.x, default_size.z) * CAMERA_DISTANCE);
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
	read_cb_aabb(OBJECT_FLOOR, (Vector3) {-100, -1, -100}, (Vector3) {200, 1, 200}, true);
	read_cb_aabb(OBJECT_CUBE1, (Vector3) {-5, 0, -5}, (Vector3) {10, 10, 10}, true);
	read_cb_aabb(OBJECT_CUBE2, (Vector3) {15, 12, 15}, (Vector3) {10, 10, 10}, true);
	read_cb_aabb(OBJECT_CUBE3, (Vector3) {30, 24, -5}, (Vector3) {10, 10, 10}, true);
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
