#pragma once
#include <raylib.h>
#define Oy (Vector3) {0.0f, 1.0f, 0.0f}
#define CAMERA_FOVY 120
#define BETWEENSS(min, x, max) ((min) <= (x) && (x) <= (max))
#define BETWEEN(min, x, max) ((min) < (x) && (x) < (max)), max) ((min) < (x) && (x) < (max))
#define XOR(a, b) (((a) || (b)) && !((a) && (b)))
#define MAX(n, m) ((n) > (m) ? (n) : (m))
#define MAXSQ(n, m) MAX(n*n, m*m)
#define MAX3(a, b, c) ((a) > (b) ? MAX(a, c) : MAX(b, c))
#define MIN(n, m) ((n) < (m) ? (n) : (m))
#define MIN3(a, b, c) ((a) < (b) ? MIN(a, c) : MIN(a, c))
#define MINSQ(n, m) MIN(n*n, m*m)
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
	int staying_on_name;
};
