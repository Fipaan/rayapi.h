#include "rayapi.h"
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define DEBUG(fmt, ...) printf("[DEBUG] " fmt "\n", ##__VA_ARGS__)


str get_object_name(str_c buf, c_str_c name) {
	mstrcpyu(buf, "bin/objects/");
	mstrcpy(buf + strlen("bin/objects/"), name);
	return buf;
}

void scan_struct_no_ptr(void* data, c_str_c fileName, int size) {
	int _size;
	memcpy(data, LoadFileData(fileName, &_size), size);
}

void save_struct_no_ptr(void* data, c_str_c fileName, int size) {
	if(!DirectoryExists("bin/objects/")) {
		MakeDirectory("bin/objects/");
	}
	SaveFileData(fileName, data, size);
}

/*
int scan_v3(c_str_c s, Vector3* v) {
	int res = sscanf(s, V3_FORMAT, V3UR(&v));
	DEBUG("Scanning...: " V3_FORMAT, V3R(v));
	return res;
}
int save_v3(str_c result, Vector3 v) {
	DEBUG("Saving...: " V3_FORMAT "\n", V3(v));
	return snprintf(result, 64, V3_FORMAT, V3(v));
}
int scan_player(c_str_c s, Player* p) {
	int res = sscanf(s, V3_FORMAT ";" V3_FORMAT ";" CAMERA_FORMAT ";" CAMERA_FORMAT,
			V3U(&ctx.ob_context[p->name].cb.pos),
			V3U(&ctx.ob_context[p->name].cb.size),
			CAMERAU(&p->camera_first), 
			CAMERAU(&p->camera_third));
	DEBUG("Scanning...: " V3_FORMAT ";" V3_FORMAT ";" CAMERA_FORMAT ";" CAMERA_FORMAT,
			V3(ctx.ob_context[p->name].cb.pos),
			V3(ctx.ob_context[p->name].cb.size),
			CAMERA(p->camera_first), 
			CAMERA(p->camera_third));
	return res;
}
int save_player(str_c result, Player p) {
	DEBUG("Saving...: \\\n" V3_FORMAT ";\\\n" V3_FORMAT ";\\\n" CAMERA_FORMAT ";\\\n" CAMERA_FORMAT,
			V3(ctx.ob_context[p.name].cb.pos),
			V3(ctx.ob_context[p.name].cb.size),
			CAMERA(p.camera_first), 
			CAMERA(p.camera_third));
	return snprintf(result, 512, "\\\n" V3_FORMAT ";\\\n" V3_FORMAT ";\\\n" CAMERA_FORMAT ";\\\n" CAMERA_FORMAT,
			V3(ctx.ob_context[p.name].cb.pos),
			V3(ctx.ob_context[p.name].cb.size),
			CAMERA(p.camera_first), 
			CAMERA(p.camera_third));
}
*/
Vector3 pos_to_cpos(Vector3 pos, Vector3 size) {
	return Vector3Add(pos, Vector3Scale(size, 0.5f));
}
Vector3 cpos_to_pos(Vector3 cpos, Vector3 size) {
	return Vector3Subtract(cpos, Vector3Scale(size, 0.5f));
}

// Swap data between p1 and p2
void swap(void* p1, void* p2, int size) {
	void* tmp = malloc(size);
	memcpy(tmp, p2, size);
	memcpy(p2, p1, size);
	memcpy(p1, tmp, size);
	free(tmp);
}
// Pull data from src to dest, and then clear src
void pull_clear(void* dest, void* src, int size) {
	memcpy(dest, src, size);
	memset(src, '\0', size);
}

// Pull data from src to dest, and then free src
void pull_free(void* dest, void* src, int size) {
	memcpy(dest, src, size);
	free(src);
}

// malloc + memcpy(..., str, len)
char* strcpyloc(c_str_c str, int len) {
	if(!len) len = strlen(str);
	char* res = malloc(len + 1);
	memcpy(res, str, len);
	res[len] = '\0';
	return res;
}

// Free strings and array itself
void free_strings(str* strings, int count) {
	for(int i = 0; i < count; ++i) {
		free(strings[i]);
	}
	free(strings);
}

// Pop |count| chars start from |index| in |str| with length |len|
void pop_cs(str_c str, int index, int len, int count) {
	for(int i = index; i + count < len; ++i) {
		str[i] = str[i+count];
	}
	memset(str + (len - count), '\0', count);
}

// Handle current character, modify if needed next character, returns if word ended
void __check_c(str_c str, int* index, int* len, const int delim, int* count) {
	char c0 = str[*index];
	char c1 = str[*index + 1];
	if (c0 == '\\') {
		if(c1 == '\\') {
			pop_cs(str, *index, *len--, 1);
		} else if(c1 == delim) {
			pop_cs(str, *index, *len--, 1);
		} else {
			*len = -1;
		}
	} else {
		if(c0 == delim) {
			str[(*index)++] = '\0';
			*count += 1;
		}
		*index += 1;
	}
}

// Splits text by |delim| and return array of pointers on strings (they could be not properly aligned in memory)
//
// NOTE: need freeing (result and all result[[i]])
//
// NOTE: delim = '\' - undefined behavior
str* split(c_str_c text, char delim, int* count) {
	if(!text) return NULL;
	int len = strlen(text);
	if(!len) return NULL;
	int _count = 1;
	str tmp_text = strcpyloc(text, len);
	int index = 0;
	while(index < len) __check_c(tmp_text, &index, &len, delim, &_count);
	if(count) *count = _count;
	str* res = calloc(_count, sizeof(str));
	int ptr = 0;
	for(int i = 0; i < _count; ++i) {
		res[i] = calloc(strlen(tmp_text + ptr) + 1, sizeof(char));
		memcpy(res[i], tmp_text + ptr, strlen(tmp_text + ptr) + 1);
		ptr += strlen(res[i]) + 1;
	}
	free(tmp_text);
	return res;
}

// Splits text into the |result| by |delim|
void snsplit(c_str text, char delim, int* count, str* result) {
	int len = strlen(text);
	if(!len) return;
	int _count = 1;
	str tmp_text = strcpyloc(text, len);
	int index = 0;
	while(index < len) __check_c(tmp_text, &index, &len, delim, &_count);
	if(count) *count = _count;
	int ptr = 0;
	for(int i = 0; i < _count; ++i) {
		memcpy(result[i], tmp_text + ptr, strlen(tmp_text + ptr) + 1);
		ptr += strlen(result[i]) + 1;
	}
	free(tmp_text);
}

// Read file into |vars|, passing only correct lines
void read_file(Vars* vars, const char* fileName) {
	vars->lines = split(LoadFileText(fileName), '\n', &vars->count_lines);
	for(int i = 0; i < vars->count_lines; ++i) {
		int count_words = 0;
		str* words = split(vars->lines[i], WORDS_DELIM, &count_words);
		if(count_words != 2) {
			free(vars->lines[i]);
			vars->lines[i] = vars->lines[--vars->count_lines];
			i--;
		} else {
			if(strlen(words[0]) > (size_t)vars->key_cap) vars->key_cap = strlen(words[0]);
			if(strlen(words[1]) > (size_t)vars->value_cap) vars->value_cap = strlen(words[1]);
		}
		free_strings(words, count_words);
	}
	vars->key = calloc(vars->key_cap + 1, sizeof(char));
	vars->value = calloc(vars->value_cap + 1, sizeof(char));
}

void write_file(Vars vars, const char* fileName) {
	int size = 0;
	for(int i = 0; i < vars.count_lines; ++i) {
		size += strlen(vars.lines[i]) + 1;
	}
	if(size == 0) {
		SaveFileText(fileName, "");
		return;
	}
	char* file = malloc(size);
	file[size - 1] = '\0';
	int ptr = 0;
	for(int i = 0; i < vars.count_lines; ++i) {
		if(i > 0) file[ptr++] = '\n';
		memcpy(file + ptr, vars.lines[i], strlen(vars.lines[i]));
		ptr += strlen(vars.lines[i]);
	}
	SaveFileText(fileName, file);
	free(file);
}

// Goes to next var by |vars->curr_line|, useful for while/for loops
bool next_var(Vars* vars) {
	if(vars->count_lines == vars->curr_line) return false;
	c_str_c kv_link[2] = {vars->key, vars->value};
	snsplit(vars->lines[vars->curr_line], WORDS_DELIM, NULL, (str*)kv_link);
	vars->curr_line++;
	return true;
}

int index_by_key(Vars vars, const char* key) {
	for(int i = 0; i < vars.count_lines; ++i) {
		if(strncmp(vars.lines[i], key, strlen(key)) == 0 &&
				   vars.lines[i][strlen(key)] == WORDS_DELIM
		) return i;
	}
	return -1;
}

c_str value_by_key(Vars vars, const char* key) {
	int index = index_by_key(vars, key);
	if(index == -1) return NULL;
	c_str_c kv_link[2] = {vars.key, vars.value};
	snsplit(vars.lines[index], WORDS_DELIM, NULL, (str*)kv_link);
	return vars.value;
}

void update_var_by_index(Vars* vars, int index, c_str_c new_value) {
	c_str kv_link[2] = {vars->key, vars->value};
	snsplit(vars->lines[index], ':', NULL, (str*)kv_link);
	
	vars->lines[index] = realloc(vars->lines[index], strlen(vars->key) + 1 + strlen(new_value) + 1);
	memcpy(vars->lines[index], vars->key, strlen(vars->key));
	vars->lines[index][strlen(vars->key)] = ':';
	memcpy(vars->lines[index] + strlen(vars->key) + 1, new_value, strlen(new_value) + 1);
	
	if((size_t)vars->value_cap < strlen(new_value)) {
		vars->value_cap = strlen(new_value);
		vars->value = realloc((void*)vars->value, vars->value_cap + 1);
	}
}

void __add_key(Vars* vars, const char* key, const char* value) {
	vars->lines = realloc(vars->lines, (vars->count_lines + 1) * sizeof(char*));
	int len_key = strlen(key);
	int len_value = strlen(value);
	
	char* last_line = malloc(len_key + 1 + len_value + 1);
	vars->lines[vars->count_lines++] = last_line;
	memcpy(last_line, key, len_key);
	last_line[len_key] = ':';
	memcpy(last_line + len_key + 1, value, len_value + 1);
	
	if(len_key > vars->key_cap) {
		vars->key_cap = len_key;
		vars->key = realloc((void*)vars->key, vars->key_cap + 1);
	}
	if(len_value > vars->value_cap) {
		vars->value_cap = len_value;
		vars->value = realloc((void*)vars->value, vars->value_cap + 1);
	}
}

// return true if added new key
bool upsert_var_by_key(Vars* vars, c_str_c key, c_str_c new_value) {
	int index = index_by_key(*vars, key);
	if(index != -1) {
		update_var_by_index(vars, index, new_value);
		return false;
	} else {
		__add_key(vars, key, new_value);
		return true;
	}
}

void free_vars(Vars* vars) {
	free((void*)vars->key);
	free((void*)vars->value);
	for(int i = 0;i < vars->count_lines; ++i) {
		free(vars->lines[i]);
	}
	free(vars->lines);
	vars->lines = NULL;
	vars->count_lines = 0;
}

// Returns vars[key]. Key not exists -> vars[key] = fallback_value
const char* value_fallback(Vars* vars, const char* key, const char* fallback_value) {
	const char* value = value_by_key(*vars, key);
	if(!value) {
		__add_key(vars, key, fallback_value);
		value = value_by_key(*vars, key);
	}
	return value;
}

// returns success of operation
bool remove_key_by_index(Vars* vars, int index) {
	if(index >= vars->count_lines) return false;
	free(vars->lines[index]);
	for(int i = index; i < vars->count_lines - 1; ++i) {
		vars->lines[i] = vars->lines[i + 1];
	}
	vars->lines[vars->count_lines - 1] = NULL;
	vars->lines = realloc(vars->lines, (--vars->count_lines) * sizeof(char*));
	return true;
}

// skey - string key ("+", "-", "ArrowUp", "Num-", ...)
// rkey - raylib key (KEY_DOWN, KEY_APOSTROPHE, ...)
// ikey - index of keybind (i ~ kb_context[i])

#define get_special_key(key) __special_keys[key - KEY_ESCAPE]
#define _S(i) [i - KEY_ESCAPE]
c_str_c __special_keys[COUNT_SPECIAL_KEYS] = {
	// NOTE: no KEY_SPACE, it located on other place
	_S(KEY_ESCAPE) = "Esc",
	_S(KEY_ENTER) = "Enter",
	_S(KEY_TAB) = "Tab",
	_S(KEY_BACKSPACE) = "Back ",
	_S(KEY_INSERT) = "Insert",
	_S(KEY_DELETE) = "Delete",
	_S(KEY_RIGHT) = "Right",
	_S(KEY_LEFT) = "Left",
	_S(KEY_DOWN) = "Down",
	_S(KEY_UP) = "Up",
	_S(KEY_PAGE_UP) = "PgUp",
	_S(KEY_PAGE_DOWN) = "PgDown",
	_S(KEY_HOME) = "Home",
	_S(KEY_END) = "End",
	_S(KEY_CAPS_LOCK) = "CapsL",
	_S(KEY_SCROLL_LOCK) = "ScrollL",
	_S(KEY_NUM_LOCK) = "NumL",
	_S(KEY_PRINT_SCREEN) = "PrtSc",
	_S(KEY_PAUSE) = "Pause",
	_S(KEY_F1) = "F1",
	_S(KEY_F2) = "F2",
	_S(KEY_F3) = "F3",
	_S(KEY_F4) = "F4",
	_S(KEY_F5) = "F5",
	_S(KEY_F6) = "F6",
	_S(KEY_F7) = "F7",
	_S(KEY_F8) = "F8",
	_S(KEY_F9) = "F9",
	_S(KEY_F10) = "F10",
	_S(KEY_F11) = "F11",
	_S(KEY_F12) = "F12",
	_S(KEY_LEFT_SHIFT) = "LShift",
	_S(KEY_LEFT_CONTROL) = "LControl",
	_S(KEY_LEFT_ALT) = "LAlt",
	_S(KEY_LEFT_SUPER) = "LSuper",
	_S(KEY_RIGHT_SHIFT) = "RShift",
	_S(KEY_RIGHT_CONTROL) = "RControl",
	_S(KEY_RIGHT_ALT) = "RAlt",
	_S(KEY_RIGHT_SUPER) = "RSuper",
	_S(KEY_KB_MENU) = "Menu",
	_S(KEY_KP_0) = "Num0",
	_S(KEY_KP_1) = "Num1",
	_S(KEY_KP_2) = "Num2",
	_S(KEY_KP_3) = "Num3",
	_S(KEY_KP_4) = "Num4",
	_S(KEY_KP_5) = "Num5",
	_S(KEY_KP_6) = "Num6",
	_S(KEY_KP_7) = "Num7",
	_S(KEY_KP_8) = "Num8",
	_S(KEY_KP_9) = "Num9",
	_S(KEY_KP_DECIMAL) = "Num.",
	_S(KEY_KP_DIVIDE) = "Num/",
	_S(KEY_KP_MULTIPLY) = "Num*",
	_S(KEY_KP_SUBTRACT) = "Num-",
	_S(KEY_KP_ADD) = "Num+",
	_S(KEY_KP_ENTER) = "NumEnter",
	_S(KEY_KP_EQUAL) = "Num=",
};

const char* rkey_to_skey(int rkey, str_c buf) {
	memset(buf, '\0', CAP_KEY_LENGTH + 1);
	if(rkey == KEY_NULL) {
		mstrcpy(buf, "NULL");
	}
	if (is_alphanumeric_key_or_space(rkey)) {
		*buf = rkey;
	} else if (is_special_key_no_space(rkey)) {
		mstrcpy(buf, get_special_key(rkey));
	} else {
		mstrcpy(buf, "UNDEFINED");
	}
	return buf;
}

int skey_to_rkey(c_str_c skey) {
	if(skey[1] == '\0') {
		if(is_alphanumeric_key_or_space(skey[0])) return skey[0];
		return -1;
	}
	int len = strlen(skey);
	if(strncmp("NULL", skey, len) == 0) return KEY_NULL;
	for_betweenss(KEY_ESCAPE, key, KEY_END) {
		if(strncmp(get_special_key(key), skey, len) == 0) return key;
	}
	for_betweenss(KEY_CAPS_LOCK, key, KEY_PAUSE) {
		if(strncmp(get_special_key(key), skey, len) == 0) return key;
	}
	for_betweenss(KEY_F1, key, KEY_F12) {
		if(strncmp(get_special_key(key), skey, len) == 0) return key;
	}
	for_betweenss(KEY_LEFT_SHIFT, key, KEY_KB_MENU) {
		if(strncmp(get_special_key(key), skey, len) == 0) return key;
	}
	return -1;
}

// Checks for certain keybind to be holded
bool IsKeybindDown(Name ikey) {
	return IsKeyDown(ctx.kb_context[ikey].key);
};

// Checks for certain keybind to be pressed
bool IsKeybindPressed(Name ikey) {
	return IsKeyPressed(ctx.kb_context[ikey].key);
}

bool IsKeybindDownSoft(Name ikey) {
	return IsKeybindPressed(ikey) || IsKeybindDown(ikey);
}

// Both keys down cancel each other
//
// Otherwise key_plus returns 1.0f and key_minus returns -1.0f
float IsKeybindDownToggle(Name ikey_plus, Name ikey_minus) {
	return (IsKeybindDown(ikey_plus) - IsKeybindDown(ikey_minus)) * 0.5f;
}

float IsKeybindDownToggleSoft(Name ikey_plus, Name ikey_minus) {
	return (IsKeybindDownSoft(ikey_plus) - IsKeybindDownSoft(ikey_minus)) * 0.5f;
}
// Tries to get key (as value in vars) by name (as key in vars) and save it in kb
// 
// If it couldn't find it, save in kb key fallback_skey converted to rkey
void skb_bind(Vars* vars, Keybind* kb, const char* fallback_skey) {
	kb->key = skey_to_rkey(value_fallback(vars, kb->name, fallback_skey));
}

// Add kb->name: fallback_key pair to vars, and returns fallback_key
//
// NOTE: Don't call this function, it's helper function for ikb_bind
int __rkb_bind_helper(Vars* vars, Keybind* kb, int fallback_key) {
	char buf[CAP_KEY_LENGTH + 1] = {};
	upsert_var_by_key(vars, kb->name, (char*)rkey_to_skey(fallback_key, buf));
	return fallback_key;
}

// Tries to get key (as value in vars) by name (as key in vars) and save it in kb
// 
// If it couldn't find it, save in kb key fallback_key
void rkb_bind(Vars* vars, Keybind* kb, int fallback_rkey) {
	const char* skey = value_by_key(*vars, kb->name);
	kb->key = skey ? skey_to_rkey(skey) : __rkb_bind_helper(vars, kb, fallback_rkey);
}

void get_cb_data(Name index, Vector3* pos, Vector3* size, Vector3* cpos, Vector3* epos) {
	if(pos) *pos = ctx.ob_context[index].cb.pos;
	if(size) *size = ctx.ob_context[index].cb.size;
	if(cpos) *cpos = Vector3Add(ctx.ob_context[index].cb.pos, Vector3Scale(ctx.ob_context[index].cb.size, 0.5f));
	if(epos) *epos = Vector3Add(ctx.ob_context[index].cb.pos, ctx.ob_context[index].cb.size);
}

Vector3 get_moving_vector(Name index, Vector3 moving_vector, int count, bool* is_collided) {
	for(int i = 0; i < count; ++i) {
		if(i == index) continue;
		moving_vector = CONVERT_MOV_VECTOR(ctx.ob_context[index].cb, ctx.ob_context[i].cb, moving_vector, is_collided);
		if(*is_collided) goto end_sec;
	}
end_sec:
	return moving_vector;
}

void mov_by_index(Name index, Vector3 moving_vector, int count) {
	bool is_collided = false;
	do {
		moving_vector = get_moving_vector(index, moving_vector, count, &is_collided);
	} while (is_collided);
	ctx.ob_context[index].cb.pos = Vector3Add(ctx.ob_context[index].cb.pos, moving_vector);
}

bool is_falling(Name name) {
	Vector3 pos = ctx.ob_context[name].cb.pos;
	for(int i = 0; i < ctx.obs_count; ++i) {
		if(name == i) continue;
		Vector3 ipos = ctx.ob_context[i].cb.pos;
		Vector3 isize = ctx.ob_context[i].cb.size;
		if(BETWEENSS(ipos.y + isize.y, pos.y, ipos.y + isize.y + FLOOR_PERC) &&
		   BETWEENSS(ipos.x, pos.x, ipos.x + isize.x) &&
		   BETWEENSS(ipos.z, pos.z, ipos.z + isize.z)
		) return false;
	}
	return true;
}

float get_falling_floor(Name name) {
	Vector3 pos = ctx.ob_context[name].cb.pos;
	for(int i = 0; i < ctx.obs_count; ++i) {
		if(name == i) continue;
		Vector3 ipos = ctx.ob_context[i].cb.pos;
		Vector3 isize = ctx.ob_context[i].cb.size;
		if(BETWEENSS(ipos.y + isize.y, pos.y, ipos.y + isize.y + FLOOR_PERC) &&
		   BETWEENSS(ipos.x, pos.x, ipos.x + isize.x) &&
		   BETWEENSS(ipos.z, pos.z, ipos.z + isize.z)
		) return ipos.y + isize.y;
	}
	return pos.y;
}

struct AABB {
	Vector3 pos;
	Vector3 size;
};

void read_cb_aabb(Name name, Vector3 default_pos, Vector3 default_size, bool ignoreFile) {
	ctx.ob_context[me.name].cb.type = COLLISION_BOX_AABB;
	char fileName[sizeof("bin/objects/") + strlen(ctx.ob_context[name].name)];
	if(!ignoreFile && FileExists(get_object_name(fileName, ctx.ob_context[name].name))) {
		struct AABB aabb;
		SCAN_STRUCT_NO_PTR(struct AABB, &aabb, fileName);
		ctx.ob_context[name].cb.pos = aabb.pos;
		ctx.ob_context[name].cb.size = aabb.size;
	} else {
		ctx.ob_context[name].cb.pos = default_pos;
		ctx.ob_context[name].cb.size = default_size;
	}
}
void write_cb_aabb(Name name) {
	char fileName[sizeof("bin/objects/") + strlen(ctx.ob_context[name].name)];
	get_object_name(fileName, ctx.ob_context[name].name);
	struct AABB aabb = {
		.pos = ctx.ob_context[name].cb.pos,
		.size = ctx.ob_context[name].cb.size,
	};
	SAVE_STRUCT_NO_PTR(struct AABB, &aabb, fileName);
}

struct PlayerData {
	Player player;
	Vector3 pos;
	Vector3 size;
};

void read_player(Player* player, Player default_player, Vector3 default_pos, Vector3 default_size, bool ignoreFile) {
	char fileName[sizeof("bin/objects/") + strlen(ctx.ob_context[default_player.name].name)];
	if(!ignoreFile && FileExists(get_object_name(fileName, ctx.ob_context[default_player.name].name))) {
		struct PlayerData player_data;
		SCAN_STRUCT_NO_PTR(struct PlayerData, &player_data, fileName);
		*player = player_data.player;
		ctx.ob_context[default_player.name].cb.pos = player_data.pos;
		ctx.ob_context[default_player.name].cb.size = player_data.size;
	} else {
		*player = default_player;
		ctx.ob_context[default_player.name].cb.pos = default_pos;
		ctx.ob_context[default_player.name].cb.size = default_size;
	}
}
void write_player(Player player) {
	char fileName[sizeof("bin/objects/") + strlen(ctx.ob_context[player.name].name)];
	get_object_name(fileName, ctx.ob_context[player.name].name);
	struct PlayerData player_data = {
		.player = player,
		.pos = ctx.ob_context[player.name].cb.pos,
		.size = ctx.ob_context[player.name].cb.size,
	};
	SAVE_STRUCT_NO_PTR(struct PlayerData, &player_data, fileName);
}

bool activate_event(Event* event) {
	if(!event->active) return false;
	if(FLAG(event->btype, ET_ALWAYS)) goto on_action_true_sec;
	if(FLAG(event->btype, EB_COOLDOWN)) {
		if(event->cooldown_data.ellapsed_time > 0.0f) {
			event->cooldown_data.cooldown_time += GetFrameTime();
			if(event->cooldown_data.ellapsed_time >= event->cooldown_data.cooldown_time) {
				event->cooldown_data.ellapsed_time = 0.0f;
				if(event->cooldown_data.on_cooldown) event->cooldown_data.on_cooldown(event);
				if(FLAG(event->btype, EB_EVENT_SUCCESS)) return activate_event(event->bevent_data.event);
			} else {
				return false;
			}
		}
	}
	if(FLAG(event->ttype, ET_ON_FLOOR)) {
		if(is_falling(event->on_floor_data.name)) goto on_action_false_sec;
	}
	if(FLAG(event->ttype, ET_NOT_ON_FLOOR)) {
		if(!is_falling(event->on_floor_data.name)) goto on_action_false_sec;
	}
	if(FLAG(event->ttype, ET_CLICK_OR_DOWN)) {
		if(!IsKeybindDownSoft(event->key_data.name)) goto on_action_false_sec;
	} else if(FLAG(event->ttype, ET_CLICK)) {
		if(!IsKeybindPressed(event->key_data.name)) goto on_action_false_sec;
	} else if(FLAG(event->ttype, ET_DOWN)) {
		if(!IsKeybindDown(event->key_data.name)) goto on_action_false_sec;
	}
on_action_true_sec:
	if(!event->on_action(event)) goto on_action_false_sec;
	if(FLAG(event->btype, EB_COOLDOWN)) event->cooldown_data.cooldown_time += GetFrameTime();
	else if(FLAG(event->btype, EB_EVENT_SUCCESS)) return activate_event(event->bevent_data.event);
	return true;
on_action_false_sec:
	if(FLAG(event->btype, EB_EVENT_FAIL)) return activate_event(event->bevent_data.event);
	return false;
}
