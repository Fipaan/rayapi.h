#include <raylib.h>
#include <stddef.h>
#include <string.h>

#ifndef RAYF_MALLOC
#define RAYF_MALLOC malloc
#endif // RAYF_MALLOC

#ifndef RAYF_REALLOC
#define RAYF_REALLOC realloc
#endif // RAYF_REALLOC

#ifndef RAYF_FREE
#define RAYF_FREE free
#endif // RAYF_FREE

#ifndef RAYF_CALLOC
#define RAYF_CALLOC calloc
#endif // RAYF_CALLOC

#ifndef RAYF_MEMCPY
#define RAYF_MEMCPY memcpy
#endif // RAYF_MEMCPY

#ifndef RAYF_MEMSET
#define RAYF_MEMSET memset
#endif // RAYF_MEMSET

#ifndef RAYF_MEMCMP
#define RAYF_MEMCMP memcmp
#endif // RAYF_MEMSET

#ifdef STRIP_PREFIXES
#define RAYF_MALLOC MALLOC
#define RAYF_REALLOC REALLOC
#define RAYF_FREE FREE
#define RAYF_CALLOC CALLOC
#define RAYF_MEMCPY MEMCPY
#define RAYF_MEMSET MEMSET
#define RAYF_MEMCMP MEMCMP
#endif // STRIP_PREFIXES

// NOTE:
// @optional -
//     Usually I put optional parameters at the end
//     (via pointers), sometimes all parameters are optional.
//     NULL indicates omission. Not all trailing parameters
//     are optional - depends on context.

#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define da_append(da, item) \
	do { \
		if((da)->count * sizeof(*(da)->items) >= (da)->capacity) { \
			(da)->capacity = MAX(2 * (da)->capacity, sizeof(*(da)->items)); \
			(da)->items = RAYF_REALLOC((da)->items, (da)->capacity); \
		} \
		(da)->items[(da)->count++] = (item); \
	} while(0)

typedef struct RayF_String {
	char* buf;
	size_t length;
} RayF_String;
#define str_to_cp(str) (str).buf
#define cp_to_str(cp) (RayF_String) {(cp), strlen(cp)}
#define NULL_STR (RayF_String) {NULL, 0}
#define str_update_len(str) (str)->length = strlen((str)->buf)
#define str_append(str, ch) (str)->buf[(str)->length++] = (ch)
#define str_null_terminate(str) (str)->buf[(str)->length] = '\0'
#define fstrcpy(dst, src) \
	do {\
		RAYF_MEMCPY((dst)->buf, (src).buf, (src).length+1);\
		(dst)->length = (src).length;\
	} while(0)
#define array_str_static_alloc(name, buf_capacity, count)							\
	RayF_String name[(count)];														\
	{																				\
		char __##name##_internal_buffer[(buf_capacity)*(count)];					\
		RAYF_MEMSET(__##name##_internal_buffer, '\0', (buf_capacity)*(count));		\
		for(int __i_##name = 0; __i_##name < (count); ++__i_##name) {				\
			name[__i_##name] = (RayF_String) {										\
				.buf = &__##name##_internal_buffer[(buf_capacity)*__i_##name],		\
			};																		\
		}																			\
	}
#define str_free(str) \
	do { \
		RAYF_FREE((void*)(str)->buf); \
		RAYF_MEMSET((str), '\0', sizeof(RayF_String));\
	} while(0)

typedef struct RayF_StringBuilder {
	char* buf;
	size_t length;
	size_t capacity;
} RayF_StringBuilder;
#define sb_to_str(sb) (RayF_String) {(sb).buf, (sb).length}
#define sb_to_cp(sb) (sb).buf
#define str_to_sb(str, capacity) (RayF_StringBuilder) {(sb).buf, (sb).length, (capacity)}
#define cp_to_sb(cp, capacity) (RayF_StringBuilder) {(cp), strlen(cp), (capacity)}
#define SB_INC_STEP 128
#define sb_init() (RayF_StringBuilder) {.buf = RAYF_CALLOC(SB_INC_STEP, sizeof(char)), .capacity = SB_INC_STEP}
#define sb_append(sb, c) \
	do { \
		if((sb)->length >= (sb)->capacity) { \
			(sb)->capacity += SB_INC_STEP; \
			(sb)->buf = RAYF_REALLOC((sb)->buf, (sb)->capacity); \
		} \
		(sb)->buf[(sb)->length++] = (c); \
	} while(0)
#define sb_free(sb)\
	do {\
		RAYF_FREE((void*)(sb)->buf);\
		RAYF_MEMSET((sb), '\0', sizeof(RayF_StringBuilder));\
	} while(0)
#define sb_append_str(sb, str) \
	do { \
		if((sb)->capacity < (sb)->length + (str).length) { \
			(sb)->capacity = MAX(2 * (sb)->capacity, (sb)->length + (str).length); \
			(sb)->buf = RAYF_REALLOC((sb)->buf, (sb)->capacity); \
		} \
		RAYF_MEMCPY((sb)->buf + (sb)->length, (str).buf, (str).length); \
		(sb)->length += (str).length; \
	} while(0)
#define sb_append_sv(sb, sv) \
	do { \
		if((sb)->capacity < (sb)->length + (sv).length) { \
			(sb)->capacity = MAX(2 * (sb)->capacity, (sb)->length + (sv).length); \
			(sb)->buf = RAYF_REALLOC((sb)->buf, (sb)->capacity); \
		} \
		RAYF_MEMCPY((sb)->buf + (sb)->length, (sv).view, (sv).length); \
		(sb)->length += (sv).length; \
	} while(0)
typedef struct RayF_StringView {
	const char* view;
	size_t length;
} RayF_StringView;
#define cp_to_sv(cp) (RayF_StringView) {(cp), strlen(cp)}
#define str_to_sv(str) (RayF_StringView) {(str).buf, (str).length}
#define sb_to_sv(sb) (RayF_StringView) {(sb).buf, (sb).length}
// NOTE: I don't have macros for casting sv to something,
// because usually it's not what you want to do
#define NULL_SV (RayF_StringView) {NULL, 0}

typedef struct RayF_Vars {
	RayF_String* lines; // array of lines, delimited by '\n'
	int count_lines; // count of lines
	int curr_line; // current line
	union {
		struct {
			RayF_StringBuilder key;
			RayF_StringBuilder value;
		};
		RayF_StringBuilder pair[2];
	};
} RayF_Vars;

// Shifts position from inital state to end state. For example: \end
// |___| __ |___|												\end
// |___| -> |_._|												\end
// |.__| __ |___|												\end
// Here start_shift = {0, 0, 0} and end_shift = {0.5, 0.5, 0.5} \end
// pos + (end - start) * size
Vector3 position_lerp(Vector3 pos, Vector3 size, Vector3 start_shift, Vector3 end_shift);
#define LERP_POS (Vector3) {0.0f, 0.0f, 0.0f}
#define LERP_CPOS (Vector3) {0.5f, 0.5f, 0.5f}
// Initialize Vars structure
RayF_Vars rayf_vars_init(void);
// Free strings and array itself
void rayf_free_strings(RayF_String* strings, int count);
// RAYF_MALLOC + RAYF_MEMCPY for RayF_String structure
RayF_String rayf_strcpyloc(RayF_StringView text);
// RAYF_MALLOC + RAYF_MEMCPY but from char* to RayF_String
RayF_String rayf_strcpyloc2(const char* text);
// Split |text| with length |text_len| by |delim|, that can be escaped by
// |escape_symbol|. Saves words count in |count|, and provides them through return.
// NOTE: needs freeing
// NOTE: @optional
RayF_String* rayf_split(RayF_StringView text, char delim, char escape_symbol, int* count);
// Split |text| with length |text_len| by |delim|, that can be escaped by
// |escape_symbol. Saves words count in |count|, stores result into buffer, and returns it
// NOTE: @optional
RayF_String* rayf_snsplit(RayF_StringView text, char delim, char escape_symbol, int* count, RayF_String* buffer);
// Same as snsplit, but allocates memory if necessary for RayF_StringBuilder
// NOTE: @optional
RayF_StringBuilder* rayf_sbsplit(RayF_StringView text, char delim, char escape_symbol, int* count, RayF_StringBuilder* buffer);



#define WORDS_DELIM ':'
#define WORDS_ESCAPING '\\'

// Read file into |vars|, passing only correct lines
void rayf_read_file(RayF_Vars* vars, const char* fileName);
// Write file from |vars|
// NOTE: rayf_free_vars should be called separately
void rayf_write_file(RayF_Vars vars, const char* fileName);
// Deallocates necessary structs in |vars|, and clean up it for new usage
void rayf_free_vars(RayF_Vars* vars);
// Usage:
// while(rayf_next_var(&vars)) {
//     // Do something with vars.key, vars.value
//     // For reset you can do |vars.curr_line = 0|
// }
bool rayf_next_var(RayF_Vars* vars);
// Get index of key, -1 on non-existing key
int rayf_index_by_key(RayF_Vars vars, RayF_StringView key);
// Get value of key, NULL_SV on non-existing key
RayF_StringView rayf_value_by_key(RayF_Vars vars, RayF_StringView key);
// Updates certain line of |vars| by |index|,
// modifying value of that pair.
void rayf_update_var_by_index(RayF_Vars* vars, int index, RayF_StringView new_value);
// Adds new pair |key| ~ |value|
// NOTE: Unexpected behavior if key already exists
void rayf_add_pair(RayF_Vars* vars, RayF_StringView key, RayF_StringView new_value);
// Updates existing |key|, or adds it with |new_value|
// return true if added new key
bool rayf_upsert_var_by_key(RayF_Vars* vars, RayF_StringView key, RayF_StringView new_value);
// Adds pair |key| ~ |fallback_value| if |key| does not exists,
// return value by key, or |fallback_value| if it didn't existed
RayF_StringView rayf_value_fallback(RayF_Vars* vars, RayF_StringView key, RayF_StringView fallback_value);

// ---------------------------------------------------------------------------------------
// 						Objects
// RayF_O<Object_Name> - structure name
// RAYF_O<OBJECT_NAME> - enum name
// rayf_o<object_name>_init - Initialize certain object
// rayf_o<object_name>_draw_raw - draw raw object
// rayf_o<object_name>_draw - links with raw version
// rayf_o<object_name>_destroy - Destroy/free certain object
// <object> - human readable "interface" of object
// NOTE: In init functions I use macro RAYF_OBJECT_LINK:
// 		RAYF_OBJECT_LINK(self, object_name) - links rayf_o<object_name> functions with |self|
// How to add new object?
// Implement:
//     typedef struct RayF_O<Object_Name> {...} RayF_O<Object_Name>;
//     RayF_Object *rayf_o<object_name>_init(RayF_Allocator allocator, ...);
//     void rayf_o<object_name>_draw_raw(RayF_O<object_name> self); // Not so important, could be embeded inside draw function
//     void rayf_o<object_name>_draw(RayF_Object *self);
//     void rayf_o<object_name>_destroy(RayF_Free custom_free, RayF_Object *self);
// Modify:
// 	   enum RayF_ObjectLike: add RAYF_O<OBJECT_NAME>
// 	   union RayF_ObjectAs: add RayF_O<Object_Name> *<object_name>;
// 	   function rayf_object_init: handle linking for new type, general case:
// 	   		case RayF_ObjectLike: add RAYF_O<OBJECT>: {RAYF_OBJECT_LINK(res, <object>);} break;
// ---------------------------------------------------------------------------------------

#define RAYF_OBJECT_LINK(self, object)				   \
    do {                                               \
		(self)->do_.draw = rayf_o##object##_draw;	   \
    	(self)->do_.destroy = rayf_o##object##_destroy;\
	} while (0)

#define RAYF_OBJECT_INIT_BASIC(OBJECT_NAME, Object_Name, object_name, object_binding) \
	RayF_Object *object = allocator(sizeof(RayF_Object)); \
	*object = (RayF_Object) { \
		.like = RAYF_O##OBJECT_NAME, \
		.as.object_name = allocator(sizeof(RayF_O##Object_Name)), \
	}; \
	RAYF_OBJECT_LINK(object, object_name); \
	*object->as.object_name = object_binding; \
	return object

// TODO: make aliases for "same" types (RAYF_OCIRCLE_SECTOR and RAYF_OCIRCLE_SECTOR_LINES for example)
// TODO: Handle images
// TODO: Handle 3D

typedef void* (*RayF_Allocator)(size_t size);
typedef void (*RayF_Free)(void *ptr);

typedef struct RayF_Object RayF_Object;

typedef struct RayF_Points {
	Vector2* points;
	// TODO: this without ruining structure
	// struct RayF_Points_As {
	// 	Vector2 center;
	// 	Vector2 points[];
	// } *fan;
	int count;
	Color color;
} RayF_Points;

// Behavior of most simple objects at least
void rayf_object_destroy_default(RayF_Free custom_free, RayF_Object* self);

// [BASIC SHAPES INTERFACES]

typedef struct RayF_OPixel {
	Vector2 position;
	Color color;
} RayF_OPixel;

RayF_Object *rayf_opixel_init(RayF_Allocator allocator, Vector2 position, Color color);
void rayf_opixel_draw_raw(RayF_OPixel self);
void rayf_opixel_draw(RayF_Object *self);
void rayf_opixel_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_OLine {
    Vector2 start, end;
    Color color;
} RayF_OLine;

RayF_Object *rayf_oline_init(RayF_Allocator allocator, Vector2 start, Vector2 end, Color color);
void rayf_oline_draw_raw(RayF_OLine self);
void rayf_oline_draw(RayF_Object *self);
void rayf_oline_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_OLine_Ex {
    Vector2 start, end;
    float thick;
    Color color;
} RayF_OLine_Ex;

RayF_Object *rayf_oline_ex_init(RayF_Allocator allocator,
		Vector2 start, Vector2 end, float thick, Color color);
void rayf_oline_ex_draw_raw(RayF_OLine_Ex self);
void rayf_oline_ex_draw(RayF_Object *self);
void rayf_oline_ex_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_OLine_Ex RayF_OLine_Bezier;

RayF_Object *rayf_oline_bezier_init(RayF_Allocator allocator,
		Vector2 start, Vector2 end, float thick, Color color);
void rayf_oline_bezier_draw_raw(RayF_OLine_Bezier self);
void rayf_oline_bezier_draw(RayF_Object *self);
void rayf_oline_bezier_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_Points RayF_OLine_Strip;

// NOTE: You need assign each self->point[i] by yourself
RayF_Object *rayf_oline_strip_init(RayF_Allocator allocator, int count, Color color);
void rayf_oline_strip_draw_raw(RayF_OLine_Strip self);
void rayf_oline_strip_draw(RayF_Object *self);
void rayf_oline_strip_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_OCircle {
	Vector2 center;
	float radius;
	Color color;
} RayF_OCircle;

RayF_Object *rayf_ocircle_init(RayF_Allocator allocator, Vector2 center, float radius, Color color);
void rayf_ocircle_draw_raw(RayF_OCircle self);
void rayf_ocircle_draw(RayF_Object *self);
void rayf_ocircle_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_OCircle RayF_OCircle_Lines;

RayF_Object *rayf_ocircle_lines_init(RayF_Allocator allocator, Vector2 center, float radius, Color color);
void rayf_ocircle_lines_draw_raw(RayF_OCircle_Lines self);
void rayf_ocircle_lines_draw(RayF_Object *self);
void rayf_ocircle_lines_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_OCircle_Grad {
	Vector2 center;
	float radius;
	Color inner;
	Color outer;
} RayF_OCircle_Grad;

RayF_Object *rayf_ocircle_grad_init(RayF_Allocator allocator,
		Vector2 center, float radius, Color inner, Color outer);
void rayf_ocircle_grad_draw_raw(RayF_OCircle_Grad self);
void rayf_ocircle_grad_draw(RayF_Object *self);
void rayf_ocircle_grad_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_OCircle_Sector {
	Vector2 center;
	float radius;
	Color color;
	float startAngle, endAngle;
	int segments;
} RayF_OCircle_Sector;

RayF_Object *rayf_ocircle_sector_init(RayF_Allocator allocator,
		Vector2 center, float radius, Color color,
		float startAngle, float endAngle, int segments);
void rayf_ocircle_sector_draw_raw(RayF_OCircle_Sector self);
void rayf_ocircle_sector_draw(RayF_Object *self);
void rayf_ocircle_sector_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_OCircle_Sector RayF_OCircle_Sector_Lines;

RayF_Object *rayf_ocircle_sector_lines_init(RayF_Allocator allocator,
		Vector2 center, float radius, Color color,
		float startAngle, float endAngle, int segments);
void rayf_ocircle_sector_lines_draw_raw(RayF_OCircle_Sector_Lines self);
void rayf_ocircle_sector_lines_draw(RayF_Object *self);
void rayf_ocircle_sector_lines_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_OEllipse {
	Vector2 center;
	Vector2 radius;
	Color color;
} RayF_OEllipse;

RayF_Object *rayf_oellipse_init(RayF_Allocator allocator, Vector2 center, Vector2 radius, Color color);
void rayf_oellipse_draw_raw(RayF_OEllipse self);
void rayf_oellipse_draw(RayF_Object *self);
void rayf_oellipse_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_OEllipse RayF_OEllipse_Lines;

RayF_Object *rayf_oellipse_lines_init(RayF_Allocator allocator, Vector2 center, Vector2 radius, Color color);
void rayf_oellipse_lines_draw_raw(RayF_OEllipse_Lines self);
void rayf_oellipse_lines_draw(RayF_Object *self);
void rayf_oellipse_lines_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_ORing {
	Vector2 center;
	float innerRadius, outerRadius;
	float startAngle, endAngle;
	int segments;
	Color color;
} RayF_ORing;

RayF_Object *rayf_oring_init(RayF_Allocator allocator,
		Vector2 center,
		float innerRadius, float outerRadius,
		float startAngle, float endAngle,
		int segments, Color color);
void rayf_oring_draw_raw(RayF_ORing self);
void rayf_oring_draw(RayF_Object *self);
void rayf_oring_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_ORing RayF_ORing_Lines;

RayF_Object *rayf_oring_lines_init(RayF_Allocator allocator,
		Vector2 center,
		float innerRadius, float outerRadius,
		float startAngle, float endAngle,
		int segments, Color color);
void rayf_oring_lines_draw_raw(RayF_ORing_Lines self);
void rayf_oring_lines_draw(RayF_Object *self);
void rayf_oring_lines_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_ORectangle {
	Rectangle rec;
	Color color;
} RayF_ORectangle;

RayF_Object *rayf_orectangle_init(RayF_Allocator allocator, Rectangle rec, Color color);
void rayf_orectangle_draw_raw(RayF_ORectangle self);
void rayf_orectangle_draw(RayF_Object *self);
void rayf_orectangle_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_ORectangle RayF_ORectangle_Lines;

RayF_Object *rayf_orectangle_lines_init(RayF_Allocator allocator, Rectangle rec, Color color);
void rayf_orectangle_lines_draw_raw(RayF_ORectangle_Lines self);
void rayf_orectangle_lines_draw(RayF_Object *self);
void rayf_orectangle_lines_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_ORectangle_Lines_Ex {
	Rectangle rec;
	float thick;
	Color color;
} RayF_ORectangle_Lines_Ex;

RayF_Object *rayf_orectangle_lines_ex_init(RayF_Allocator allocator, Rectangle rec, float thick, Color color);
void rayf_orectangle_lines_ex_draw_raw(RayF_ORectangle_Lines_Ex self);
void rayf_orectangle_lines_ex_draw(RayF_Object *self);
void rayf_orectangle_lines_ex_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_ORectangle_Pro {
	Rectangle rec;
	Vector2 origin;
	float rotation;
	Color color;
} RayF_ORectangle_Pro;

RayF_Object *rayf_orectangle_pro_init(RayF_Allocator allocator, Rectangle rec, Vector2 origin, float rotation, Color color);
void rayf_orectangle_pro_draw_raw(RayF_ORectangle_Pro self);
void rayf_orectangle_pro_draw(RayF_Object *self);
void rayf_orectangle_pro_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_ORectangle_GradV {
	Rectangle rec;
	Color top, bottom;
} RayF_ORectangle_GradV;

RayF_Object *rayf_orectangle_gradv_init(RayF_Allocator allocator, Rectangle rec, Color top, Color bottom);
void rayf_orectangle_gradv_draw_raw(RayF_ORectangle_GradV self);
void rayf_orectangle_gradv_draw(RayF_Object *self);
void rayf_orectangle_gradv_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_ORectangle_GradH {
	Rectangle rec;
	Color left, right;
} RayF_ORectangle_GradH;

RayF_Object *rayf_orectangle_gradh_init(RayF_Allocator allocator, Rectangle rec, Color left, Color right);
void rayf_orectangle_gradh_draw_raw(RayF_ORectangle_GradH self);
void rayf_orectangle_gradh_draw(RayF_Object *self);
void rayf_orectangle_gradh_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_ORectangle_Grad {
	Rectangle rec;
	Color topLeft, bottomLeft;
	Color topRight, bottomRight;
} RayF_ORectangle_Grad;

RayF_Object *rayf_orectangle_grad_init(RayF_Allocator allocator, Rectangle rec,
		Color topLeft , Color bottomLeft,
		Color topRight, Color bottomRight);
void rayf_orectangle_grad_draw_raw(RayF_ORectangle_Grad self);
void rayf_orectangle_grad_draw(RayF_Object *self);
void rayf_orectangle_grad_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_ORectangle_Rounded {
	Rectangle rec;
	float roundness;
	int segments;
	Color color;
} RayF_ORectangle_Rounded;

RayF_Object *rayf_orectangle_rounded_init(RayF_Allocator allocator, Rectangle rec,
		float roundness, int segments,
		Color color);
void rayf_orectangle_rounded_draw_raw(RayF_ORectangle_Rounded self);
void rayf_orectangle_rounded_draw(RayF_Object *self);
void rayf_orectangle_rounded_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_ORectangle_Rounded RayF_ORectangle_Rounded_Lines;

RayF_Object *rayf_orectangle_rounded_lines_init(RayF_Allocator allocator, Rectangle rec,
		float roundness, int segments,
		Color color);
void rayf_orectangle_rounded_lines_draw_raw(RayF_ORectangle_Rounded_Lines self);
void rayf_orectangle_rounded_lines_draw(RayF_Object *self);
void rayf_orectangle_rounded_lines_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_ORectangle_Rounded_Lines_Ex {
	Rectangle rec;
	float roundness;
	int segments;
	Color color;
	float thick;
} RayF_ORectangle_Rounded_Lines_Ex;

RayF_Object *rayf_orectangle_rounded_lines_ex_init(RayF_Allocator allocator, Rectangle rec,
		float roundness, int segments,
		Color color, float thick);
void rayf_orectangle_rounded_lines_ex_draw_raw(RayF_ORectangle_Rounded_Lines_Ex self);
void rayf_orectangle_rounded_lines_ex_draw(RayF_Object *self);
void rayf_orectangle_rounded_lines_ex_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_OTriangle {
	Vector2 v1, v2, v3;
	Color color;
} RayF_OTriangle;

RayF_Object *rayf_otriangle_init(RayF_Allocator allocator,
		Vector2 v1, Vector2 v2, Vector2 v3,
		Color color);
void rayf_otriangle_draw_raw(RayF_OTriangle self);
void rayf_otriangle_draw(RayF_Object *self);
void rayf_otriangle_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_OTriangle RayF_OTriangle_Lines;

RayF_Object *rayf_otriangle_lines_init(RayF_Allocator allocator,
		Vector2 v1, Vector2 v2, Vector2 v3,
		Color color);
void rayf_otriangle_lines_draw_raw(RayF_OTriangle_Lines self);
void rayf_otriangle_lines_draw(RayF_Object *self);
void rayf_otriangle_lines_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_Points RayF_OTriangle_Fan;

// NOTE: You need assign each self->points[i] by yourself
RayF_Object *rayf_otriangle_fan_init(RayF_Allocator allocator, int count, Color color);
void rayf_otriangle_fan_draw_raw(RayF_OTriangle_Fan self);
void rayf_otriangle_fan_draw(RayF_Object *self);
void rayf_otriangle_fan_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_Points RayF_OTriangle_Strip;

// NOTE: You need assign each self->points[i] by yourself
RayF_Object *rayf_otriangle_strip_init(RayF_Allocator allocator, int count, Color color);
void rayf_otriangle_strip_draw_raw(RayF_OTriangle_Strip self);
void rayf_otriangle_strip_draw(RayF_Object *self);
void rayf_otriangle_strip_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_OPoly {
	Vector2 center;
	int sides;
	float radius;
	float rotation;
	Color color;
} RayF_OPoly;

RayF_Object *rayf_opoly_init(RayF_Allocator allocator,
	Vector2 center, int sides, float radius,
	float rotation, Color color);
void rayf_opoly_draw_raw(RayF_OPoly self);
void rayf_opoly_draw(RayF_Object *self);
void rayf_opoly_destroy(RayF_Free custom_free, RayF_Object *self);

typedef RayF_OPoly RayF_OPoly_Lines;

RayF_Object *rayf_opoly_lines_init(RayF_Allocator allocator,
	Vector2 center, int sides, float radius,
	float rotation, Color color);
void rayf_opoly_lines_draw_raw(RayF_OPoly_Lines self);
void rayf_opoly_lines_draw(RayF_Object *self);
void rayf_opoly_lines_destroy(RayF_Free custom_free, RayF_Object *self);

typedef struct RayF_OPoly_Lines_Ex {
	Vector2 center;
	int sides;
	float radius;
	float rotation;
	float thick;
	Color color;
} RayF_OPoly_Lines_Ex;

RayF_Object *rayf_opoly_lines_ex_init(RayF_Allocator allocator,
	Vector2 center, int sides, float radius,
	float rotation, float thick, Color color);
void rayf_opoly_lines_ex_draw_raw(RayF_OPoly_Lines_Ex self);
void rayf_opoly_lines_ex_draw(RayF_Object *self);
void rayf_opoly_lines_ex_destroy(RayF_Free custom_free, RayF_Object *self);

// [BASIC SHAPES INTERFACES]

typedef enum RayF_ObjectLike {
	RAYF_OCUSTOM,
	RAYF_OPIXEL,
	RAYF_OLINE,
	RAYF_OLINE_EX,
	RAYF_OLINE_BEZIER,
	RAYF_OLINE_STRIP,
	RAYF_OCIRCLE,
	RAYF_OCIRCLE_LINES,
	RAYF_OCIRCLE_GRAD,
	RAYF_OCIRCLE_SECTOR,
	RAYF_OCIRCLE_SECTOR_LINES,
	RAYF_OELLIPSE,
	RAYF_OELLIPSE_LINES,
	RAYF_ORING,
	RAYF_ORING_LINES,
	RAYF_ORECTANGLE,
	RAYF_ORECTANGLE_LINES,
	RAYF_ORECTANGLE_LINES_EX,
	RAYF_ORECTANGLE_PRO,
	RAYF_ORECTANGLE_GRADV,
	RAYF_ORECTANGLE_GRADH,
	RAYF_ORECTANGLE_GRAD,
	RAYF_ORECTANGLE_ROUNDED,
	RAYF_ORECTANGLE_ROUNDED_LINES,
	RAYF_ORECTANGLE_ROUNDED_LINES_EX,
	RAYF_OTRIANGLE,
	RAYF_OTRIANGLE_LINES,
	RAYF_OTRIANGLE_FAN,
	RAYF_OTRIANGLE_STRIP,
	RAYF_OPOLY,
	RAYF_OPOLY_LINES,
	RAYF_OPOLY_LINES_EX,
} RayF_ObjectLike;

// NOTE: Instead of this you should use actual object type under the hood
typedef void RayF_OVoid;

struct RayF_Object {
	RayF_ObjectLike like;
	union RayF_ObjectAs {
		RayF_OVoid                        *custom;
		RayF_OPixel                       *pixel;
		RayF_OLine                        *line;
		RayF_OLine_Ex                     *line_ex;
		RayF_OLine_Bezier                 *line_bezier;
		RayF_OLine_Strip                  *line_strip;
		RayF_OCircle                      *circle;
		RayF_OCircle_Lines                *circle_lines;
		RayF_OCircle_Grad                 *circle_grad;
		RayF_OCircle_Sector               *circle_sector;
		RayF_OCircle_Sector_Lines         *circle_sector_lines;
		RayF_OEllipse                     *ellipse;
		RayF_OEllipse_Lines               *ellipse_lines;
		RayF_ORing                        *ring;
		RayF_ORing_Lines                  *ring_lines;
		RayF_ORectangle                   *rectangle;
		RayF_ORectangle_Lines             *rectangle_lines;
		RayF_ORectangle_Lines_Ex          *rectangle_lines_ex;
		RayF_ORectangle_Pro               *rectangle_pro;
		RayF_ORectangle_GradV             *rectangle_gradv;
		RayF_ORectangle_GradH             *rectangle_gradh;
		RayF_ORectangle_Grad              *rectangle_grad;
		RayF_ORectangle_Rounded           *rectangle_rounded;
		RayF_ORectangle_Rounded_Lines     *rectangle_rounded_lines;
		RayF_ORectangle_Rounded_Lines_Ex  *rectangle_rounded_lines_ex;
		RayF_OTriangle                    *triangle;
		RayF_OTriangle_Lines              *triangle_lines;
		RayF_OTriangle_Fan                *triangle_fan;
		RayF_OTriangle_Strip              *triangle_strip;
		RayF_OPoly                        *poly;
		RayF_OPoly_Lines                  *poly_lines;
		RayF_OPoly_Lines_Ex               *poly_lines_ex;
	} as;
	struct RayF_Object_Methods {
		void (*draw)(RayF_Object *self);
		void (*destroy)(RayF_Free custom_free, RayF_Object *self);
	} do_;
};

typedef struct RayF_Objects {
	RayF_Object **items;
	size_t count;
	size_t capacity;
} RayF_Objects;

// NOTE: object should be allocated the same way as provided allocator
// NOTE: object->do_ should be linked manually
RayF_Object *rayf_object_init(RayF_Allocator allocator, RayF_OVoid *object, RayF_ObjectLike like);
void rayf_objects_append(RayF_Objects *objects, RayF_Object *object);
void rayf_objects_draw_all(RayF_Objects objects);
void rayf_objects_destroy_all(RayF_Free custom_free, RayF_Objects objects);

#ifdef RAYF_IMPLEMENTATION

#include <stdlib.h>
#include <raymath.h>
#include <assert.h>

Vector3 position_lerp(Vector3 pos, Vector3 size, Vector3 start_shift, Vector3 end_shift)
{
	return Vector3Add(pos, Vector3Multiply(Vector3Subtract(end_shift, start_shift), size));
}

RayF_Vars rayf_vars_init(void)
{
	return (RayF_Vars) {
		.key = sb_init(),
		.value = sb_init(),
	};
}

void rayf_free_strings(RayF_String* strings, int count)
{
	for(int i = 0; i < count; ++i) {
		RAYF_FREE(strings[i].buf);
	}
	RAYF_FREE(strings);
}

int rayf_strcmp2(RayF_StringView s1, RayF_StringView s2) {
	if(s1.length == s2.length) {
		return RAYF_MEMCMP(s1.view, s2.view, s1.length);
	}
	return s1.length - s2.length;
}

RayF_String rayf_strcpyloc(RayF_StringView text)
{
	RayF_String res = {
		.buf = RAYF_MALLOC(text.length + 1),
		.length = text.length
	};
	RAYF_MEMCPY(res.buf, text.view, text.length);
	res.buf[text.length] = '\0';
	return res;
}

RayF_String rayf_strcpyloc2(const char* text)
{
	return rayf_strcpyloc((RayF_StringView) {
		(char*)text,
		strlen(text)
	});
}

RayF_String* rayf_split(RayF_StringView text, char delim, char escape_symbol, int* count)
{
	// In case of empty string return NULL 
	if(text.length == 0) {
		return NULL;
	}
	int _count = 1;
	assert(escape_symbol != delim);
	// Calculate count of words, based on count delimiters and escaping symbols
	if(text.view[0] == delim) _count++;
	for(size_t i = 1; i < text.length; ++i) {
		if(text.view[i] == delim && text.view[i-1] != escape_symbol) {
			_count++;
		}
	}
	if(count) *count = _count;
	char __tmp_text_buf[text.length+1];
	RayF_String tmp_text = {.buf = (char*)__tmp_text_buf};
	RayF_String* res = RAYF_MALLOC(_count * sizeof(RayF_String));
	int current = 0;
	bool escaped = false;
	for(size_t i = 0; i < text.length; ++i) {
		char ch = text.view[i];
		if(!escaped) {
			if(ch == escape_symbol) {
				escaped = true;
			} else if(ch == delim) {
				res[current] = rayf_strcpyloc(str_to_sv(tmp_text));
				tmp_text.length = 0;
				current++;
			} else {
				str_append(&tmp_text, ch);
			}
			continue;
		}
		if(ch == escape_symbol || ch == delim) {
			str_append(&tmp_text, ch);
		} else {
			goto on_error;
		}
		escaped = false;
	}
	// Unclosed escaping
	if (escaped) {
		goto on_error;
	}
	res[current] = rayf_strcpyloc(str_to_sv(tmp_text));
	return res;
on_error:
	rayf_free_strings(res, current+1);
	return NULL;
}

RayF_String* rayf_snsplit(RayF_StringView text, char delim, char escape_symbol, int* count, RayF_String* buffer)
{
	// In case of empty string return NULL 
	if(text.length == 0) {
		return NULL;
	}
	int _count = 1;
	assert(escape_symbol != delim);
	// Calculate count of words, based on count delimiters and escaping symbols
	if(text.view[0] == delim) _count++;
	for(size_t i = 1; i < text.length; ++i) {
		if(text.view[i] == delim && text.view[i-1] != escape_symbol) {
			_count++;
		}
	}
	if(count) *count = _count;
	int current = 0;
	bool escaped = false;
	for(size_t i = 0; i < text.length; ++i) {
		char ch = text.view[i];
		if(!escaped) {
			if(ch == escape_symbol) {
				escaped = true;
			} else if(ch == delim) {
				str_null_terminate(&buffer[current]);
				buffer[++current].length = 0;
			} else {
				str_append(&buffer[current], ch);
			}
			continue;
		}
		if(ch == escape_symbol || ch == delim) {
			str_append(&buffer[current], ch);
		} else {
			goto on_error;
		}
		escaped = false;
	}
	// Unclosed escaping
	if (escaped) {
		goto on_error;
	}
	str_null_terminate(&buffer[current]);
	return buffer;
on_error:
	return NULL;
}

RayF_StringBuilder* rayf_sbsplit(RayF_StringView text, char delim, char escape_symbol, int* count, RayF_StringBuilder* buffer)
{
	// In case of empty string return NULL 
	if(text.length == 0) {
		return NULL;
	}
	int _count = 1;
	assert(escape_symbol != delim);
	// Calculate count of words, based on count delimiters and escaping symbols
	if(text.view[0] == delim) _count++;
	for(size_t i = 1; i < text.length; ++i) {
		if(text.view[i] == delim && text.view[i-1] != escape_symbol) {
			_count++;
		}
	}
	if(count) *count = _count;
	int current = 0;
	bool escaped = false;
	buffer[0].length = 0;
	for(size_t i = 0; i < text.length; ++i) {
		char ch = text.view[i];
		if(!escaped) {
			if(ch == escape_symbol) {
				escaped = true;
			} else if(ch == delim) {
				str_null_terminate(&buffer[current]);
				buffer[++current].length = 0;
			} else {
				sb_append(&buffer[current], ch);
			}
			continue;
		}
		if(ch == escape_symbol || ch == delim) {
			sb_append(&buffer[current], ch);
		} else {
			goto on_error;
		}
		escaped = false;
	}
	// Unclosed escaping
	if (escaped) {
		goto on_error;
	}
	str_null_terminate(&buffer[current]);
	return buffer;
on_error:
	return NULL;
}

void rayf_read_file(RayF_Vars* vars, const char* fileName)
{
	{
		char* cp_tmp = LoadFileText(fileName);
		vars->lines = rayf_split(cp_to_sv(cp_tmp), '\n', WORDS_ESCAPING, &vars->count_lines);
		UnloadFileText(cp_tmp);
	}
	for(int i = 0; i < vars->count_lines; ++i) {
		int count_words = 0;
		RayF_String* words = rayf_split(str_to_sv(vars->lines[i]), WORDS_DELIM, WORDS_ESCAPING, &count_words);
		if(count_words != 2) {
			RAYF_FREE(vars->lines[i].buf);
			vars->lines[i] = vars->lines[--vars->count_lines];
			i--;
		}
		rayf_free_strings(words, count_words);
	}
	vars->lines = RAYF_REALLOC(vars->lines, vars->count_lines * sizeof(*vars->lines));
}

void rayf_write_file(RayF_Vars vars, const char* fileName)
{
	int size = 0;
	for(int i = 0; i < vars.count_lines; ++i) {
		size += vars.lines[i].length + 1;
	}
	if(size == 0) {
		SaveFileText(fileName, "");
		return;
	}
	char* file = RAYF_MALLOC(size);
	int ptr = 0;
	for(int i = 0; i < vars.count_lines; ++i) {
		if(i > 0) file[ptr++] = '\n';
		RAYF_MEMCPY(file + ptr, vars.lines[i].buf, vars.lines[i].length);
		ptr += vars.lines[i].length;
	}
	file[size - 1] = '\0';
	SaveFileText(fileName, file);
	RAYF_FREE(file);
}

void rayf_free_vars(RayF_Vars* vars)
{
	sb_free(&vars->key);
	sb_free(&vars->value);
	for(int i = 0;i < vars->count_lines; ++i) {
		str_free(&vars->lines[i]);
	}
	RAYF_FREE(vars->lines);
	vars->lines = NULL;
	vars->count_lines = 0;
}

bool rayf_next_var(RayF_Vars* vars)
{
	if(vars->count_lines == vars->curr_line) return false;
	rayf_sbsplit(str_to_sv(vars->lines[vars->curr_line]), WORDS_DELIM, WORDS_ESCAPING, NULL, vars->pair);
	vars->curr_line++;
	return true;
}

int rayf_index_by_key(RayF_Vars vars, RayF_StringView key)
{
	for(int i = 0; i < vars.count_lines; ++i) {
		if(vars.lines[i].length <= key.length) continue;
		if(RAYF_MEMCMP(vars.lines[i].buf, key.view, key.length) == 0 &&
				   vars.lines[i].buf[key.length] == WORDS_DELIM
		) return i;
	}
	return -1;
}

RayF_StringView rayf_value_by_key(RayF_Vars vars, RayF_StringView key)
{
	int index = rayf_index_by_key(vars, key);
	if(index == -1) return NULL_SV;
	rayf_sbsplit(str_to_sv(vars.lines[index]), WORDS_DELIM, WORDS_ESCAPING, NULL, vars.pair);
	return sb_to_sv(vars.value);
}

void rayf_update_var_by_index(RayF_Vars* vars, int index, RayF_StringView new_value)
{
	rayf_sbsplit(str_to_sv(vars->lines[index]), WORDS_DELIM, WORDS_ESCAPING, NULL, vars->pair);
	
	vars->lines[index].buf = RAYF_REALLOC(vars->lines[index].buf, vars->key.length + 1 + new_value.length + 1);
	RAYF_MEMCPY(vars->lines[index].buf, vars->key.buf, vars->key.length);
	vars->lines[index].buf[vars->key.length] = ':';
	RAYF_MEMCPY(vars->lines[index].buf + vars->key.length + 1, new_value.view, new_value.length + 1);
	str_update_len(&vars->lines[index]);
	
	vars->value.length = 0;
	sb_append_sv(&vars->value, new_value);
}

void rayf_add_pair(RayF_Vars* vars, RayF_StringView key, RayF_StringView value)
{
	vars->lines = RAYF_REALLOC(vars->lines, (vars->count_lines + 1) * sizeof(*vars->lines));
	RayF_String last_line = {
		.buf = RAYF_MALLOC(key.length + 1 + value.length + 1),
		.length = key.length + 1 + value.length,
	};
	vars->lines[vars->count_lines++] = last_line;
	RAYF_MEMCPY(last_line.buf, key.view, key.length);
	last_line.buf[key.length] = ':';
	RAYF_MEMCPY(last_line.buf + key.length + 1, value.view, value.length + 1);
}

bool rayf_upsert_var_by_key(RayF_Vars* vars, RayF_StringView key, RayF_StringView new_value)
{
	int index = rayf_index_by_key(*vars, key);
	if(index != -1) {
		rayf_update_var_by_index(vars, index, new_value);
		return false;
	} else {
		rayf_add_pair(vars, key, new_value);
		return true;
	}
}

RayF_StringView rayf_value_fallback(RayF_Vars* vars, RayF_StringView key, RayF_StringView fallback_value)
{
	RayF_StringView value = rayf_value_by_key(*vars, key);
	if(!value.view) {
		rayf_add_pair(vars, key, fallback_value);
		value = rayf_value_by_key(*vars, key);
	}
	return value;
}

void rayf_object_destroy_default(RayF_Free custom_free, RayF_Object* self)
{
	custom_free(self->as.custom);
	custom_free(self);
}

// [BASIC SHAPES INTERFACES]
RayF_Object *rayf_opixel_init(RayF_Allocator allocator, Vector2 position, Color color)
{
	RayF_OPixel object_binding = (RayF_OPixel) {
		.position = position,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(PIXEL, Pixel, pixel, object_binding);
}
void rayf_opixel_draw_raw(RayF_OPixel self)
{
	DrawPixelV(self.position, self.color);
}
void rayf_opixel_draw(RayF_Object *self)
{
	rayf_opixel_draw_raw(*self->as.pixel);
}
void rayf_opixel_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_oline_init(RayF_Allocator allocator, Vector2 start, Vector2 end, Color color)
{
	RayF_OLine object_binding = (RayF_OLine) {
		.start = start,
		.end = end,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(LINE, Line, line, object_binding);
}
void rayf_oline_draw_raw(RayF_OLine self)
{
	DrawLineV(self.start, self.end, self.color);
}
void rayf_oline_draw(RayF_Object *self)
{
	rayf_oline_draw_raw(*self->as.line);
}
void rayf_oline_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_oline_ex_init(RayF_Allocator allocator,
		Vector2 start, Vector2 end, float thick, Color color)
{
	RayF_OLine_Ex object_binding = (RayF_OLine_Ex) {
		.start = start,
		.end = end,
		.thick = thick,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(LINE_EX, Line_Ex, line_ex, object_binding);
}
void rayf_oline_ex_draw_raw(RayF_OLine_Ex self)
{
	DrawLineEx(self.start, self.end, self.thick, self.color);
}
void rayf_oline_ex_draw(RayF_Object *self)
{
	rayf_oline_ex_draw_raw(*self->as.line_ex);
}
void rayf_oline_ex_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_oline_bezier_init(RayF_Allocator allocator,
		Vector2 start, Vector2 end, float thick, Color color)
{
	RayF_OLine_Bezier object_binding = (RayF_OLine_Bezier) {
		.start = start,
		.end = end,
		.thick = thick,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(LINE_BEZIER, Line_Bezier, line_bezier, object_binding);
}
void rayf_oline_bezier_draw_raw(RayF_OLine_Bezier self)
{
	DrawLineBezier(self.start, self.end, self.thick, self.color);
}
void rayf_oline_bezier_draw(RayF_Object *self)
{
	rayf_oline_bezier_draw_raw(*self->as.line_bezier);
}
void rayf_oline_bezier_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_oline_strip_init(RayF_Allocator allocator, int count, Color color)
{
	RAYF_OBJECT_INIT_BASIC(LINE_STRIP, Line_Strip, line_strip, ((RayF_OLine_Strip) {
		.points = allocator(count * sizeof(*object->as.line_strip->points)),
		.count = count,
		.color = color,
	}));
}
void rayf_oline_strip_draw_raw(RayF_OLine_Strip self)
{
	DrawLineStrip(self.points, self.count, self.color);
}
void rayf_oline_strip_draw(RayF_Object *self)
{
	rayf_oline_strip_draw_raw(*self->as.line_strip);
}
void rayf_oline_strip_destroy(RayF_Free custom_free, RayF_Object *self)
{
	custom_free(self->as.line_strip->points);
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_ocircle_init(RayF_Allocator allocator, Vector2 center, float radius, Color color)
{
	RayF_OCircle object_binding = (RayF_OCircle) {
		.center = center,
		.radius = radius,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(CIRCLE, Circle, circle, object_binding);
}
void rayf_ocircle_draw_raw(RayF_OCircle self)
{
	DrawCircleV(self.center, self.radius, self.color);
}
void rayf_ocircle_draw(RayF_Object *self)
{
	rayf_ocircle_draw_raw(*self->as.circle);
}
void rayf_ocircle_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_ocircle_lines_init(RayF_Allocator allocator, Vector2 center, float radius, Color color)
{
	RayF_OCircle_Lines object_binding = (RayF_OCircle_Lines) {
		.center = center,
		.radius = radius,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(CIRCLE_LINES, Circle_Lines, circle_lines, object_binding);
}
void rayf_ocircle_lines_draw_raw(RayF_OCircle_Lines self)
{
	DrawCircleLinesV(self.center, self.radius, self.color);
}
void rayf_ocircle_lines_draw(RayF_Object *self)
{
	rayf_ocircle_lines_draw_raw(*self->as.circle_lines);
}
void rayf_ocircle_lines_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_ocircle_grad_init(RayF_Allocator allocator,
		Vector2 center, float radius, Color inner, Color outer)
{
	RayF_OCircle_Grad object_binding = (RayF_OCircle_Grad) {
		.center = center,
		.radius = radius,
		.inner = inner,
		.outer = outer,
	};
	RAYF_OBJECT_INIT_BASIC(CIRCLE_GRAD, Circle_Grad, circle_grad, object_binding);
}
void rayf_ocircle_grad_draw_raw(RayF_OCircle_Grad self)
{
	DrawCircleGradient((int)self.center.x, (int)self.center.y, self.radius, self.inner, self.outer);
}
void rayf_ocircle_grad_draw(RayF_Object *self)
{
	rayf_ocircle_grad_draw_raw(*self->as.circle_grad);
}
void rayf_ocircle_grad_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_ocircle_sector_init(RayF_Allocator allocator,
		Vector2 center, float radius, Color color,
		float startAngle, float endAngle, int segments)
{
	RayF_OCircle_Sector object_binding = (RayF_OCircle_Sector) {
		.center = center,
		.radius = radius,
		.color = color,
		.startAngle = startAngle,
		.endAngle = endAngle,
		.segments = segments,
	};
	RAYF_OBJECT_INIT_BASIC(CIRCLE_SECTOR, Circle_Sector, circle_sector, object_binding);
}
void rayf_ocircle_sector_draw_raw(RayF_OCircle_Sector self)
{
	DrawCircleSector(self.center, self.radius, self.startAngle, self.endAngle, self.segments, self.color);
}
void rayf_ocircle_sector_draw(RayF_Object *self)
{
	rayf_ocircle_sector_draw_raw(*self->as.circle_sector);
}
void rayf_ocircle_sector_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_ocircle_sector_lines_init(RayF_Allocator allocator,
		Vector2 center, float radius, Color color,
		float startAngle, float endAngle, int segments)
{
	RayF_OCircle_Sector_Lines object_binding = (RayF_OCircle_Sector_Lines) {
		.center = center,
		.radius = radius,
		.color = color,
		.startAngle = startAngle,
		.endAngle = endAngle,
		.segments = segments,
	};
	RAYF_OBJECT_INIT_BASIC(CIRCLE_SECTOR_LINES, Circle_Sector_Lines, circle_sector_lines, object_binding);
}
void rayf_ocircle_sector_lines_draw_raw(RayF_OCircle_Sector_Lines self)
{
	DrawCircleSectorLines(self.center, self.radius, self.startAngle, self.endAngle, self.segments, self.color);
}
void rayf_ocircle_sector_lines_draw(RayF_Object *self)
{
	rayf_ocircle_sector_lines_draw_raw(*self->as.circle_sector_lines);
}
void rayf_ocircle_sector_lines_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_oellipse_init(RayF_Allocator allocator, Vector2 center, Vector2 radius, Color color)
{
	RayF_OEllipse object_binding = (RayF_OEllipse) {
		.center = center,
		.radius = radius,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(ELLIPSE, Ellipse, ellipse, object_binding);
}
void rayf_oellipse_draw_raw(RayF_OEllipse self)
{
	DrawEllipse((int)self.center.x, (int)self.center.y, self.radius.x, self.radius.y, self.color);
}
void rayf_oellipse_draw(RayF_Object *self)
{
	rayf_oellipse_draw_raw(*self->as.ellipse);
}
void rayf_oellipse_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_oellipse_lines_init(RayF_Allocator allocator, Vector2 center, Vector2 radius, Color color)
{
	RayF_OEllipse_Lines object_binding = (RayF_OEllipse_Lines) {
		.center = center,
		.radius = radius,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(ELLIPSE_LINES, Ellipse_Lines, ellipse_lines, object_binding);
}
void rayf_oellipse_lines_draw_raw(RayF_OEllipse_Lines self)
{
	DrawEllipseLines((int)self.center.x, (int)self.center.y, self.radius.x, self.radius.y, self.color);
}
void rayf_oellipse_lines_draw(RayF_Object *self)
{
	rayf_oellipse_lines_draw_raw(*self->as.ellipse_lines);
}
void rayf_oellipse_lines_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_oring_init(RayF_Allocator allocator,
		Vector2 center,
		float innerRadius, float outerRadius,
		float startAngle, float endAngle,
		int segments, Color color)
{
	RayF_ORing object_binding = (RayF_ORing) {
		.center = center,
		.innerRadius = innerRadius,
		.outerRadius = outerRadius,
		.startAngle = startAngle,
		.endAngle = endAngle,
		.segments = segments,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(RING, Ring, ring, object_binding);
}
void rayf_oring_draw_raw(RayF_ORing self)
{
	DrawRing(self.center, self.innerRadius, self.outerRadius, self.startAngle, self.endAngle, self.segments, self.color);
}
void rayf_oring_draw(RayF_Object *self)
{
	rayf_oring_draw_raw(*self->as.ring);
}
void rayf_oring_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_oring_lines_init(RayF_Allocator allocator,
		Vector2 center,
		float innerRadius, float outerRadius,
		float startAngle, float endAngle,
		int segments, Color color)
{
	RayF_ORing_Lines object_binding = (RayF_ORing_Lines) {
		.center = center,
		.innerRadius = innerRadius,
		.outerRadius = outerRadius,
		.startAngle = startAngle,
		.endAngle = endAngle,
		.segments = segments,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(RING_LINES, Ring_Lines, ring_lines, object_binding);
}
void rayf_oring_lines_draw_raw(RayF_ORing_Lines self)
{
	DrawRingLines(self.center, self.innerRadius, self.outerRadius, self.startAngle, self.endAngle, self.segments, self.color);
}
void rayf_oring_lines_draw(RayF_Object *self)
{
	rayf_oring_lines_draw_raw(*self->as.ring_lines);
}
void rayf_oring_lines_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_orectangle_init(RayF_Allocator allocator, Rectangle rec, Color color)
{
	RayF_ORectangle object_binding = (RayF_ORectangle) {
		.rec = rec,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(RECTANGLE, Rectangle, rectangle, object_binding);
}
void rayf_orectangle_draw_raw(RayF_ORectangle self)
{
	DrawRectangleRec(self.rec, self.color);
}
void rayf_orectangle_draw(RayF_Object *self)
{
	rayf_orectangle_draw_raw(*self->as.rectangle);
}
void rayf_orectangle_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_orectangle_lines_init(RayF_Allocator allocator, Rectangle rec, Color color)
{
	RayF_ORectangle_Lines object_binding = (RayF_ORectangle_Lines) {
		.rec = rec,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(RECTANGLE_LINES, Rectangle_Lines, rectangle_lines, object_binding);
}
void rayf_orectangle_lines_draw_raw(RayF_ORectangle_Lines self)
{
	DrawRectangleLines((int)self.rec.x, (int)self.rec.y, (int)self.rec.width, (int)self.rec.height, self.color);
}
void rayf_orectangle_lines_draw(RayF_Object *self)
{
	rayf_orectangle_lines_draw_raw(*self->as.rectangle_lines);
}
void rayf_orectangle_lines_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_orectangle_lines_ex_init(RayF_Allocator allocator, Rectangle rec, float thick, Color color)
{
	RayF_ORectangle_Lines_Ex object_binding = (RayF_ORectangle_Lines_Ex) {
		.rec = rec,
		.thick = thick,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(RECTANGLE_LINES_EX, Rectangle_Lines_Ex, rectangle_lines_ex, object_binding);
}
void rayf_orectangle_lines_ex_draw_raw(RayF_ORectangle_Lines_Ex self)
{
	DrawRectangleLinesEx(self.rec, self.thick, self.color);
}
void rayf_orectangle_lines_ex_draw(RayF_Object *self)
{
	rayf_orectangle_lines_ex_draw_raw(*self->as.rectangle_lines_ex);
}
void rayf_orectangle_lines_ex_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_orectangle_pro_init(RayF_Allocator allocator, Rectangle rec, Vector2 origin, float rotation, Color color)
{
	RayF_ORectangle_Pro object_binding = (RayF_ORectangle_Pro) {
		.rec = rec,
		.origin = origin,
		.rotation = rotation,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(RECTANGLE_PRO, Rectangle_Pro, rectangle_pro, object_binding);
}
void rayf_orectangle_pro_draw_raw(RayF_ORectangle_Pro self)
{
	DrawRectanglePro(self.rec, self.origin, self.rotation, self.color);
}
void rayf_orectangle_pro_draw(RayF_Object *self)
{
	rayf_orectangle_pro_draw_raw(*self->as.rectangle_pro);
}
void rayf_orectangle_pro_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_orectangle_gradv_init(RayF_Allocator allocator, Rectangle rec, Color top, Color bottom)
{
	RayF_ORectangle_GradV object_binding = (RayF_ORectangle_GradV) {
		.rec = rec,
		.top = top,
		.bottom = bottom,
	};
	RAYF_OBJECT_INIT_BASIC(RECTANGLE_GRADV, Rectangle_GradV, rectangle_gradv, object_binding);
}
void rayf_orectangle_gradv_draw_raw(RayF_ORectangle_GradV self)
{
	DrawRectangleGradientV((int)self.rec.x, (int)self.rec.y, (int)self.rec.width, (int)self.rec.height, self.top, self.bottom);
}
void rayf_orectangle_gradv_draw(RayF_Object *self)
{
	rayf_orectangle_gradv_draw_raw(*self->as.rectangle_gradv);
}
void rayf_orectangle_gradv_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_orectangle_gradh_init(RayF_Allocator allocator, Rectangle rec, Color left, Color right)
{
	RayF_ORectangle_GradH object_binding = (RayF_ORectangle_GradH) {
		.rec = rec,
		.left = left,
		.right = right,
	};
	RAYF_OBJECT_INIT_BASIC(RECTANGLE_GRADH, Rectangle_GradH, rectangle_gradh, object_binding);
}
void rayf_orectangle_gradh_draw_raw(RayF_ORectangle_GradH self)
{
	DrawRectangleGradientH((int)self.rec.x, (int)self.rec.y, (int)self.rec.width, (int)self.rec.height, self.left, self.right);
}
void rayf_orectangle_gradh_draw(RayF_Object *self)
{
	rayf_orectangle_gradh_draw_raw(*self->as.rectangle_gradh);
}
void rayf_orectangle_gradh_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_orectangle_grad_init(RayF_Allocator allocator, Rectangle rec,
		Color topLeft , Color bottomLeft,
		Color topRight, Color bottomRight)
{
	RayF_ORectangle_Grad object_binding = (RayF_ORectangle_Grad) {
		.rec = rec,
		.topLeft = topLeft,
		.bottomLeft = bottomLeft,
		.topRight = topRight,
		.bottomRight = bottomRight,
	};
	RAYF_OBJECT_INIT_BASIC(RECTANGLE_GRAD, Rectangle_Grad, rectangle_grad, object_binding);
}
void rayf_orectangle_grad_draw_raw(RayF_ORectangle_Grad self)
{
	DrawRectangleGradientEx(self.rec, self.topLeft, self.bottomLeft, self.topRight, self.bottomRight);
}
void rayf_orectangle_grad_draw(RayF_Object *self)
{
	rayf_orectangle_grad_draw_raw(*self->as.rectangle_grad);
}
void rayf_orectangle_grad_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_orectangle_rounded_init(RayF_Allocator allocator, Rectangle rec,
		float roundness, int segments,
		Color color)
{
	RayF_ORectangle_Rounded object_binding = (RayF_ORectangle_Rounded) {
		.rec = rec,
		.roundness = roundness,
		.segments = segments,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(RECTANGLE_ROUNDED, Rectangle_Rounded, rectangle_rounded, object_binding);
}
void rayf_orectangle_rounded_draw_raw(RayF_ORectangle_Rounded self)
{
	DrawRectangleRounded(self.rec, self.roundness, self.segments, self.color);
}
void rayf_orectangle_rounded_draw(RayF_Object *self)
{
	rayf_orectangle_rounded_draw_raw(*self->as.rectangle_rounded);
}
void rayf_orectangle_rounded_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_orectangle_rounded_lines_init(RayF_Allocator allocator, Rectangle rec,
		float roundness, int segments,
		Color color)
{
	RayF_ORectangle_Rounded_Lines object_binding = (RayF_ORectangle_Rounded_Lines) {
		.rec = rec,
		.roundness = roundness,
		.segments = segments,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(RECTANGLE_ROUNDED_LINES, Rectangle_Rounded_Lines, rectangle_rounded_lines, object_binding);
}
void rayf_orectangle_rounded_lines_draw_raw(RayF_ORectangle_Rounded_Lines self)
{
	DrawRectangleRoundedLines(self.rec, self.roundness, self.segments, self.color);
}
void rayf_orectangle_rounded_lines_draw(RayF_Object *self)
{
	rayf_orectangle_rounded_lines_draw_raw(*self->as.rectangle_rounded_lines);
}
void rayf_orectangle_rounded_lines_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_orectangle_rounded_lines_ex_init(RayF_Allocator allocator, Rectangle rec,
		float roundness, int segments,
		Color color, float thick)
{
	RayF_ORectangle_Rounded_Lines_Ex object_binding = (RayF_ORectangle_Rounded_Lines_Ex) {
		.rec = rec,
		.roundness = roundness,
		.segments = segments,
		.color = color,
		.thick = thick,
	};
	RAYF_OBJECT_INIT_BASIC(RECTANGLE_ROUNDED_LINES_EX, Rectangle_Rounded_Lines_Ex, rectangle_rounded_lines_ex, object_binding);
}
void rayf_orectangle_rounded_lines_ex_draw_raw(RayF_ORectangle_Rounded_Lines_Ex self)
{
	DrawRectangleRoundedLinesEx(self.rec, self.roundness, self.segments, self.thick, self.color);
}
void rayf_orectangle_rounded_lines_ex_draw(RayF_Object *self)
{
	rayf_orectangle_rounded_lines_ex_draw_raw(*self->as.rectangle_rounded_lines_ex);
}
void rayf_orectangle_rounded_lines_ex_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_otriangle_init(RayF_Allocator allocator,
		Vector2 v1, Vector2 v2, Vector2 v3,
		Color color)
{
	RayF_OTriangle object_binding = (RayF_OTriangle) {
		.v1 = v1,
		.v2 = v2,
		.v3 = v3,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(TRIANGLE, Triangle, triangle, object_binding);
}
void rayf_otriangle_draw_raw(RayF_OTriangle self)
{
	DrawTriangle(self.v1, self.v2, self.v3, self.color);
}
void rayf_otriangle_draw(RayF_Object *self)
{
	rayf_otriangle_draw_raw(*self->as.triangle);
}
void rayf_otriangle_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_otriangle_lines_init(RayF_Allocator allocator,
		Vector2 v1, Vector2 v2, Vector2 v3,
		Color color)
{
	RayF_OTriangle_Lines object_binding = (RayF_OTriangle_Lines) {
		.v1 = v1,
		.v2 = v2,
		.v3 = v3,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(TRIANGLE_LINES, Triangle_Lines, triangle_lines, object_binding);
}
void rayf_otriangle_lines_draw_raw(RayF_OTriangle_Lines self)
{
	DrawTriangleLines(self.v1, self.v2, self.v3, self.color);
}
void rayf_otriangle_lines_draw(RayF_Object *self)
{
	rayf_otriangle_lines_draw_raw(*self->as.triangle_lines);
}
void rayf_otriangle_lines_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_otriangle_fan_init(RayF_Allocator allocator, int count, Color color)
{
	RAYF_OBJECT_INIT_BASIC(TRIANGLE_FAN, Triangle_Fan, triangle_fan, ((RayF_OTriangle_Fan) {
		.points = allocator(count * sizeof(*object->as.triangle_fan->points)),
		.count = count,
		.color = color,
	}));
}
void rayf_otriangle_fan_draw_raw(RayF_OTriangle_Fan self)
{
	DrawTriangleFan(self.points, self.count, self.color);
}
void rayf_otriangle_fan_draw(RayF_Object *self)
{
	rayf_otriangle_fan_draw_raw(*self->as.triangle_fan);
}
void rayf_otriangle_fan_destroy(RayF_Free custom_free, RayF_Object *self)
{
	custom_free(self->as.triangle_fan->points);
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_otriangle_strip_init(RayF_Allocator allocator, int count, Color color)
{
	RAYF_OBJECT_INIT_BASIC(TRIANGLE_STRIP, Triangle_Strip, triangle_strip, ((RayF_OTriangle_Strip) {
		.points = allocator(count * sizeof(*object->as.triangle_strip->points)),
		.count = count,
		.color = color,
	}));
}
void rayf_otriangle_strip_draw_raw(RayF_OTriangle_Strip self)
{
	DrawTriangleStrip(self.points, self.count, self.color);
}
void rayf_otriangle_strip_draw(RayF_Object *self)
{
	rayf_otriangle_strip_draw_raw(*self->as.triangle_strip);
}
void rayf_otriangle_strip_destroy(RayF_Free custom_free, RayF_Object *self)
{
	custom_free(self->as.triangle_strip->points);
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_opoly_init(RayF_Allocator allocator,
	Vector2 center, int sides, float radius,
	float rotation, Color color)
{
	RayF_OPoly object_binding = (RayF_OPoly) {
		.center = center,
		.sides = sides,
		.radius = radius,
		.rotation = rotation,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(POLY, Poly, poly, object_binding);
}
void rayf_opoly_draw_raw(RayF_OPoly self)
{
	DrawPoly(self.center, self.sides, self.radius, self.rotation, self.color);
}
void rayf_opoly_draw(RayF_Object *self)
{
	rayf_opoly_draw_raw(*self->as.poly);
}
void rayf_opoly_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_opoly_lines_init(RayF_Allocator allocator,
	Vector2 center, int sides, float radius,
	float rotation, Color color)
{
	RayF_OPoly_Lines object_binding = (RayF_OPoly_Lines) {
		.center = center,
		.sides = sides,
		.radius = radius,
		.rotation = rotation,
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(POLY_LINES, Poly_Lines, poly_lines, object_binding);
}
void rayf_opoly_lines_draw_raw(RayF_OPoly_Lines self)
{
	DrawPolyLines(self.center, self.sides, self.radius, self.rotation, self.color);
}
void rayf_opoly_lines_draw(RayF_Object *self)
{
	rayf_opoly_lines_draw_raw(*self->as.poly_lines);
}
void rayf_opoly_lines_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

RayF_Object *rayf_opoly_lines_ex_init(RayF_Allocator allocator,
	Vector2 center, int sides, float radius,
	float rotation, float thick, Color color)
{
	RayF_OPoly_Lines_Ex object_binding = (RayF_OPoly_Lines_Ex) {
		.center = center,
		.sides = sides,
		.radius = radius,
		.thick = thick,
		.rotation = rotation,
		
		.color = color,
	};
	RAYF_OBJECT_INIT_BASIC(POLY_LINES_EX, Poly_Lines_Ex, poly_lines_ex, object_binding);
}

void rayf_opoly_lines_ex_draw_raw(RayF_OPoly_Lines_Ex self)
{
	DrawPolyLinesEx(self.center, self.sides, self.radius, self.rotation, self.thick, self.color);
}
void rayf_opoly_lines_ex_draw(RayF_Object *self)
{
	rayf_opoly_lines_ex_draw_raw(*self->as.poly_lines_ex);
}
void rayf_opoly_lines_ex_destroy(RayF_Free custom_free, RayF_Object *self)
{
	rayf_object_destroy_default(custom_free, self);
}

// [BASIC SHAPES INTERFACES]

RayF_Object *rayf_object_init(RayF_Allocator allocator, RayF_OVoid *object, RayF_ObjectLike like)
{
	RayF_Object *res = allocator(sizeof(RayF_Object));
	res->as.custom = object;
	switch(like) {
		case RAYF_OCUSTOM: {
		} break;
		case RAYF_OPIXEL: {
			RAYF_OBJECT_LINK(res, pixel);
		} break;
		case RAYF_OLINE: {
			RAYF_OBJECT_LINK(res, line);
		} break;
		case RAYF_OLINE_EX: {
			RAYF_OBJECT_LINK(res, line_ex);
		} break;
		case RAYF_OLINE_BEZIER: {
			RAYF_OBJECT_LINK(res, line_bezier);
		} break;
		case RAYF_OLINE_STRIP: {
			RAYF_OBJECT_LINK(res, line_strip);
		} break;
		case RAYF_OCIRCLE: {
			RAYF_OBJECT_LINK(res, circle);
		} break;
		case RAYF_OCIRCLE_LINES: {
			RAYF_OBJECT_LINK(res, circle_lines);
		} break;
		case RAYF_OCIRCLE_GRAD: {
			RAYF_OBJECT_LINK(res, circle_grad);
		} break;
		case RAYF_OCIRCLE_SECTOR: {
			RAYF_OBJECT_LINK(res, circle_sector);
		} break;
		case RAYF_OCIRCLE_SECTOR_LINES: {
			RAYF_OBJECT_LINK(res, circle_sector_lines);
		} break;
		case RAYF_OELLIPSE: {
			RAYF_OBJECT_LINK(res, ellipse);
		} break;
		case RAYF_OELLIPSE_LINES: {
			RAYF_OBJECT_LINK(res, ellipse_lines);
		} break;
		case RAYF_ORING: {
			RAYF_OBJECT_LINK(res, ring);
		} break;
		case RAYF_ORING_LINES: {
			RAYF_OBJECT_LINK(res, ring_lines);
		} break;
		case RAYF_ORECTANGLE: {
			RAYF_OBJECT_LINK(res, rectangle);
		} break;
		case RAYF_ORECTANGLE_LINES: {
			RAYF_OBJECT_LINK(res, rectangle_lines);
		} break;
		case RAYF_ORECTANGLE_LINES_EX: {
			RAYF_OBJECT_LINK(res, rectangle_lines_ex);
		} break;
		case RAYF_ORECTANGLE_PRO: {
			RAYF_OBJECT_LINK(res, rectangle_pro);
		} break;
		case RAYF_ORECTANGLE_GRADV: {
			RAYF_OBJECT_LINK(res, rectangle_gradv);
		} break;
		case RAYF_ORECTANGLE_GRADH: {
			RAYF_OBJECT_LINK(res, rectangle_gradh);
		} break;
		case RAYF_ORECTANGLE_GRAD: {
			RAYF_OBJECT_LINK(res, rectangle_grad);
		} break;
		case RAYF_ORECTANGLE_ROUNDED: {
			RAYF_OBJECT_LINK(res, rectangle_rounded);
		} break;
		case RAYF_ORECTANGLE_ROUNDED_LINES: {
			RAYF_OBJECT_LINK(res, rectangle_rounded_lines);
		} break;
		case RAYF_ORECTANGLE_ROUNDED_LINES_EX: {
			RAYF_OBJECT_LINK(res, rectangle_rounded_lines_ex);
		} break;
		case RAYF_OTRIANGLE: {
			RAYF_OBJECT_LINK(res, triangle);
		} break;
		case RAYF_OTRIANGLE_LINES: {
			RAYF_OBJECT_LINK(res, triangle_lines);
		} break;
		case RAYF_OTRIANGLE_FAN: {
			RAYF_OBJECT_LINK(res, triangle_fan);
		} break;
		case RAYF_OTRIANGLE_STRIP: {
			RAYF_OBJECT_LINK(res, triangle_strip);
		} break;
		case RAYF_OPOLY: {
			RAYF_OBJECT_LINK(res, poly);
		} break;
		case RAYF_OPOLY_LINES: {
			RAYF_OBJECT_LINK(res, poly_lines);
		} break;
		case RAYF_OPOLY_LINES_EX: {
			RAYF_OBJECT_LINK(res, poly_lines_ex);
		} break;
	}
	return res;
}
void rayf_objects_append(RayF_Objects *objects, RayF_Object *object)
{
	da_append(objects, object);
}
void rayf_objects_draw_all(RayF_Objects objects)
{
	for(size_t i = 0; i < objects.count; ++i) {
			objects.items[i]->do_.draw(objects.items[i]);
	}
}
void rayf_objects_destroy_all(RayF_Free custom_free, RayF_Objects objects)
{
	for(size_t i = 0; i < objects.count; ++i) {
			objects.items[i]->do_.destroy(custom_free, objects.items[i]);
	}
}

#endif
