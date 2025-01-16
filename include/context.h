#pragma once
#include <raylib.h>
#define Oy (Vector3) {0.0f, 1.0f, 0.0f}
#define CAMERA_FOVY 120
// Map between name of keybind, and corresponding raylib key
typedef struct Keybind {
	int key;
	char* name;
} Keybind;

typedef struct CollisionBox {
	enum CollisionBoxType {
		COLLISION_BOX_AABB,
		COLLISION_BOX_COUNT,
	} type;
	Vector3 pos;
	Vector3 size;
} CollisionBox;

typedef struct Vars {
	char** lines; // array of lines, delimited by '\n'
	int count_lines; // count of lines
	int curr_line; // current line
	const char* key; // current key
	int key_cap; // cap key size
	const char* value; // current value
	int value_cap;  // cap value size
} Vars;

typedef struct Player {
	int name;
	Vector3 speed;
	Camera camera_first, camera_third;
} Player;

typedef struct ObjectBind {
	CollisionBox cb;
	char* name;
} ObjectBind;

struct Context {
	Keybind *kb_context;
	ObjectBind *ob_context;
	int obs_count;
	Vars *keybinds_v, *objects_v;
};
