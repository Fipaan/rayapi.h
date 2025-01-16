#pragma once
#include <raylib.h>
#include <raymath.h>
#include "context.h"
extern struct Context ctx;
// General macros
#define MAX(n, m) ((n) > (m) ? (n) : (m))
#define MIN(n, m) ((n) < (m) ? (n) : (m)) 
#define FLAG(flags, flag) (((flags) & (flag)) == (flag))
#define SI_OPT static inline
#define BETWEENSS(min, x, max) ((min) <= (x) && (x) <= (max))
#define BETWEEN(min, x, max) ((min) < (x) && (x) < (max))
#define for_betweenss(min, i, max) for (int i = min; i <= max; ++i)
// Lib needed
#define mstrcpy(p1, p2) memcpy((p1), (p2), strlen(p2) + 1)
#define mstrcpyu(p1, p2) memcpy((p1), (p2), strlen(p2))
// Headers specified
#define CAP_KEY_LENGTH 8
#define COUNT_SPECIAL_KEYS 93
#define WORDS_DELIM ':'
#define IN_VALUE_DELIM ';'
#define FLOOR_PERC 1.0e-3f
#define BACK_COLOR (Color) {0x18, 0x18, 0x18, 0xFF}
#define CAMERA_HEIGHT 0.8f
#define CONST_G 0.098f
// Macros that use other macros
#define is_alphanumeric_key(key) BETWEENSS(KEY_APOSTROPHE, (key), KEY_GRAVE)
#define is_alphanumeric_key_or_space(key) (is_alphanumeric_key(key) || ((key) == KEY_SPACE))

#define is_function_key_no_special(key) BETWEENSS(KEY_SPACE, (key), KEY_F12)
#define is_function_key_special(key) BETWEENSS(KEY_LEFT_SHIFT, (key), KEY_KB_MENU)
#define is_function_key(key) (is_function_key_no_special(key) || is_function_key_special(key))

#define is_function_key_no_special_no_space(key) BETWEENSS(KEY_ESCAPE, (key), KEY_F12)
#define is_function_key_no_space(key) (is_function_key_no_special_no_space(key) || is_function_key_special(key))

#define is_keypad_key(key) BETWEENSS(KEY_KP_0, (key), KEY_KP_EQUAL)

#define is_special_key(key) (is_function_key(key) || is_keypad_key(key))
#define is_special_key_no_space(key) (is_function_key_no_space(key) || is_keypad_key(key))

#define V3DEF(v3) v3 x, v3 y, v3 z
#define V3(v3) V3DEF((v3).)
#define V3U(v3) V3DEF(v3.)
#define V3R(v3) V3DEF((v3)->)
#define V3UR(v3) V3DEF(v3->)
#define V3_FORMAT "%f,%f,%f"
#define CAMERA_FORMAT V3_FORMAT "," V3_FORMAT "," V3_FORMAT ",%f,%d"
#define CAMERADEF(camera) V3U(camera position), V3U(camera target), V3U(camera up), camera fovy, camera projection
#define CAMERAU(camera) CAMERADEF(camera.)
#define CAMERA(camera) CAMERADEF((camera).)

typedef int Name;
typedef char* str;
typedef const char* c_str;
typedef char* const str_c;
typedef const char* const c_str_c;

str get_object_name(str_c buf, c_str_c name); // 'name' -> 'bin/objects/name'
void scan_struct_no_ptr(void* data, c_str_c fileName, int size);
#define SCAN_STRUCT_NO_PTR(type, data, fileName) scan_struct_no_ptr((data), (fileName), sizeof(type))
void save_struct_no_ptr(void* data, c_str_c fileName, int size);
#define SAVE_STRUCT_NO_PTR(type, data, fileName) save_struct_no_ptr((data), (fileName), sizeof(type))
/*
int scan_v3(c_str_c s, Vector3* v);
int save_v3(str_c result, Vector3 v); // assuming, that result - char[64]
int scan_player(c_str_c s, Player* v);
int save_player(str_c result, Player v); // assuming, that result - char[512]
*/
Vector3 pos_to_cpos(Vector3 pos, Vector3 size);
Vector3 cpos_to_pos(Vector3 cpos, Vector3 size);

void swap(void* p1, void* p2, int size); // Swap data between p1 and p2
void pull_clear(void* dest, void* src, int size); // Pull data from src to dest, and then clear src
void pull_free(void* dest, void* src, int size); // Pull data from src to deest, and then free src
char* strcpyloc(c_str_c str, int len); // malloc + memcpy(..., str, len)
void free_strings(str* strings, int count); // Free strings and array itself
void pop_cs(str_c str, int index, int len, int count); // Pop |count| chars start from |index| in |str| with length |len|
// Splits text by |delim| and return array of pointers on strings (they could be not properly aligned in memory)
//
// NOTE: need freeing (result and all result[[i]])
//
// NOTE: delim = '\' - undefined behavior
str* split(c_str_c text, char delim, int* count);
void snsplit(c_str text, char delim, int* count, str* result); // Splits text by |delim| and store it in |result|

void read_file(Vars* vars, const char* fileName); // Read file into |vars|, passing only correct lines
void write_file(Vars vars, const char* fileName);
bool next_var(Vars* vars); // Goes to next var by |vars->curr_line|, useful for while/for loops
int index_by_key(Vars vars, const char* key);
c_str value_by_key(Vars vars, const char* key);
void update_var_by_index(Vars* vars, int index, c_str_c new_value);
bool upsert_var_by_key(Vars* vars, c_str_c key, c_str_c new_value); // return true if added new key
void free_vars(Vars* vars);
const char* value_fallback(Vars* vars, const char* key, const char* fallback_value); // Returns vars[key]. Key not exists -> vars[key] = fallback_value
bool remove_key_by_index(Vars* vars, int index); // returns success of operation

// skey - string key ("+", "-", "ArrowUp", "Num-", ...)
// rkey - raylib key (KEY_DOWN, KEY_APOSTROPHE, ...)
// ikey - index of keybind (i ~ kb_context[i])

const char* rkey_to_skey(int rkey, str_c buf);
int skey_to_rkey(c_str_c skey);
bool IsKeybindDown(Name ikey); // Checks for certain keybind to be holded
bool IsKeybindPressed(Name ikey); // Checks for certain keybind to be pressed
bool IsKeybindDownSoft(Name ikey); // Checks for certain keybind to be pressed or holded
float IsKeybindDownToggle(Name ikey_plus, Name ikey_minus); // 0.0f, 1.0f, -1.0f, 0.0f depending on keybinds holded
float IsKeybindDownToggleSoft(Name ikey_plus, Name ikey_minus); // 0.0f, 1.0f, -1.0f, 0.0f depending on keybinds holded or pressed
void skb_bind(Vars* vars, Keybind* kb, const char* fallback_skey); // If kb->name exists in vars -> gets keybind, otherwise saves fallback_skey
void rkb_bind(Vars* vars, Keybind* kb, int fallback_rkey); // If kb->name exists in vars -> gets keybind, otherwise saves fallback_rkey

void get_cb_data(Name index, Vector3* pos, Vector3* size, Vector3* cpos, Vector3* epos); // cpos - center pos, epos - end pos. NULL for ignore
#include "collision.h"
extern ConvMovFunc convert_mov_vector[COLLISION_BOX_COUNT][COLLISION_BOX_COUNT]; // new_pos[t1][t2](cb1, cb2, moving_vector) moves cb1 without collision with cb2, according to their types
#define CONVERT_MOV_VECTOR(cb1, cb2, moving_vector, is_collided) convert_mov_vector[(cb1).type][(cb2).type]((cb1), (cb2), (moving_vector), (is_collided))
Vector3 get_moving_vector(Name index, Vector3 moving_vector, int count, bool* is_collided);
void mov_by_index(Name index, Vector3 moving_vector, int count);

extern Player me;
bool is_falling(Name name);
float get_falling_floor(Name name);
void read_cb_aabb(Name name, Vector3 default_pos, Vector3 default_size, bool ignoreFile);
void write_cb_aabb(Name name);
void read_player(Player* player, Player default_player, Vector3 default_pos, Vector3 default_size, bool ignoreFile);
void write_player(Player player);

typedef struct Event {
	enum EventTType {
		ET_NOT_ON_FLOOR = (1 << 0),
		ET_ON_FLOOR = (1 << 1),
		ET_CLICK = (1 << 2),
		ET_DOWN = (1 << 3),
		ET_CLICK_OR_DOWN = ET_CLICK | ET_DOWN,
		ET_ALWAYS = (1 << 4),
	} ttype; // Trigger type
	enum EventBType {
		EB_NONE = 0,
		EB_COOLDOWN = (1 << 0),
		EB_EVENT_SUCCESS = (1 << 1), // On success triggers another event
		EB_EVENT_FAIL = (1 << 2), // On fail triggers another event
	} btype; // Behavior type
	struct EventOnFloorData {
		Name name;
	} on_floor_data;
	struct EventKeyData {
		Name name;
	} key_data;
	struct EventCooldownData {
		float ellapsed_time;
		float cooldown_time;
		void (*on_cooldown)(struct Event* self);
	} cooldown_data;
	struct EventBEventData {
		struct Event* event;
	} bevent_data;
	bool active;
	bool (*on_action)(struct Event* self);
} Event;

bool activate_event(Event* event);
