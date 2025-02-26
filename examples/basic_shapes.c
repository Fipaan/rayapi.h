#include <math.h>
#define RAYF_IMPLEMENTATION
#include <rayapi.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>
#define ARENA_CAPACITY (8*1024)

char ARENA[ARENA_CAPACITY] = {0};
size_t ptr = 0;

void *arena_alloc(size_t size) {
	void *res = &ARENA[ptr];
	ptr += size;
	if(ptr <= ARENA_CAPACITY) {
		printf("arena_alloc: I'm filled on %.2f%% 😳\n", ptr * 100.0f / ARENA_CAPACITY);
		return res;
	} else {
		printf("arena_alloc: I can't take anymore... 😭😭😭\n");
		return NULL;
	}
}

void arena_free(void *ptr) {
	(void) ptr;
	return;
}

int main()
{
	Vector2 dpi = {16, 9};
	float SCALE = 100.0f;
	int width = dpi.x * SCALE;
	int height = dpi.y * SCALE;
	Vector2 screen_size = {width, height};
	RayF_Objects objects = {0};
	// TODO: provide example for custom object
	// rayf_objects_append(&objects, rayf_ocustom_init(arena_alloc));
	Vector2 dv = Vector2Scale(screen_size, 0.001f);
	Vector2 size = Vector2Scale(dv, 150.0f);
	float thick = 5.0f;
	Vector2 cursor = Vector2Scale(dv, 10.0f);
	float radius = MIN(size.x, size.y) * 0.5f;

	rayf_objects_append(&objects, rayf_opixel_init(arena_alloc, cursor, WHITE));
	cursor.x += 10 * dv.x;

	rayf_objects_append(&objects, rayf_oline_init(arena_alloc, cursor, Vector2Add(cursor, size), GREEN));
	cursor.x += size.x * 1.1f;

	rayf_objects_append(&objects, rayf_oline_ex_init(arena_alloc, cursor, Vector2Add(cursor, size), thick, YELLOW));
	cursor.x += size.x * 1.1f;

	rayf_objects_append(&objects, rayf_oline_bezier_init(arena_alloc, cursor, Vector2Add(cursor, size), thick, RED));
	cursor.x += size.x * 1.1f;
{
	RayF_Object *ostrip = rayf_oline_strip_init(arena_alloc, 4, PURPLE);
	RayF_OLine_Strip *olstrip = ostrip->as.line_strip;
	olstrip->points[0] = Vector2Add(cursor, Vector2Multiply(size, (Vector2){0.0f , 0.0f }));
	olstrip->points[1] = Vector2Add(cursor, Vector2Multiply(size, (Vector2){0.25f, 0.75f}));
	olstrip->points[2] = Vector2Add(cursor, Vector2Multiply(size, (Vector2){0.75f, 0.25f}));
	olstrip->points[3] = Vector2Add(cursor, Vector2Multiply(size, (Vector2){1.0f , 1.0f }));
	rayf_objects_append(&objects, ostrip);
	cursor.x += size.x * 1.1f;
}
{
	Vector2 center = Vector2Add(cursor, Vector2Scale(size, 0.55f));
	rayf_objects_append(&objects, rayf_ocircle_init(arena_alloc, center, radius, BLACK));
	rayf_objects_append(&objects, rayf_ocircle_lines_init(arena_alloc, center, radius, WHITE));
	cursor.x += size.x * 1.1f;
}
{
	Vector2 center = Vector2Add(cursor, Vector2Scale(size, 0.55f));
	rayf_objects_append(&objects, rayf_ocircle_grad_init(arena_alloc, center, radius, BLACK, WHITE));
	cursor.x = 10 * dv.x;
	cursor.y += size.y * 1.1f;
}
{
	RayF_OCircle_Sector sec = {
		.center = Vector2Add(cursor, Vector2Scale(size, 0.55f)),
		.radius = radius,
		.color = LIME,
		.startAngle = 30,
		.endAngle = 120,
		.segments = 100,
	};
	rayf_objects_append(&objects, rayf_ocircle_sector_init(arena_alloc,
				sec.center, sec.radius, sec.color,
				sec.startAngle, sec.endAngle,
				sec.segments));
	sec.color = ORANGE;
	rayf_objects_append(&objects, rayf_ocircle_sector_lines_init(arena_alloc,
				sec.center, sec.radius, sec.color,
				sec.startAngle, sec.endAngle,
				sec.segments));
	
	cursor.x += size.x * 1.1f;
}
{
	Vector2 center = Vector2Add(cursor, Vector2Scale(size, 0.55f));
	Vector2 radiusxy = Vector2Scale(size, 0.5f);
	rayf_objects_append(&objects, rayf_oellipse_init(arena_alloc, center, radiusxy, RED));
	rayf_objects_append(&objects, rayf_oellipse_lines_init(arena_alloc, center, radiusxy, BLACK));
	cursor.x += size.x * 1.1f;
}
{
	RayF_ORing ring = {
		.center = Vector2Add(cursor, Vector2Scale(size, 0.55f)),
		.innerRadius = radius * 0.6f,
		.outerRadius = radius * 0.8f,
		.startAngle = 70,
		.endAngle = 70 + 180,
		.segments = 100,
		.color = MAROON,
	};
	rayf_objects_append(&objects, rayf_oring_init(arena_alloc,
				ring.center, ring.innerRadius, ring.outerRadius,
				ring.startAngle, ring.endAngle,
				ring.segments, ring.color));
	ring.color = SKYBLUE;
	rayf_objects_append(&objects, rayf_oring_lines_init(arena_alloc,
				ring.center, ring.innerRadius, ring.outerRadius,
				ring.startAngle, ring.endAngle,
				ring.segments, ring.color));

	cursor.x += size.x * 1.1f;
}
{
	Rectangle rec = {
		.x = cursor.x + size.x * 0.05f,
		.y = cursor.y + size.y * 0.05f,
		.width = size.x,
		.height = size.y,
	};
	rayf_objects_append(&objects, rayf_orectangle_init(arena_alloc, rec, BLUE));
	rayf_objects_append(&objects, rayf_orectangle_lines_init(arena_alloc, rec, RED));
	cursor.x += size.x * 1.1f;
	rec.x += size.x * 1.1f;
	rayf_objects_append(&objects, rayf_orectangle_lines_ex_init(arena_alloc, rec, thick, YELLOW));
	cursor.x += size.x * 1.1f;
	rec.x += size.x * 1.1f;
	rayf_objects_append(&objects, rayf_orectangle_gradv_init(arena_alloc, rec, WHITE, BLACK));
	cursor.x = 10 * dv.x;
	cursor.y += size.y * 1.1f;
	rec.x = cursor.x + size.x * 0.05f,
	rec.y = cursor.y + size.y * 0.05f,
	rayf_objects_append(&objects, rayf_orectangle_gradh_init(arena_alloc, rec, WHITE, BLACK));
	cursor.x += size.x * 1.1f;
	rec.x += size.x * 1.1f;
	rayf_objects_append(&objects, rayf_orectangle_grad_init(arena_alloc, rec, LIME, PURPLE, RED, BLUE));
	cursor.x += size.x * 1.1f;
	rec.x += size.x * 1.1f;
}
{
	RayF_ORectangle_Pro *rectangle_pro = arena_alloc(sizeof(RayF_ORectangle_Pro));
	Rectangle rec = {
		.x = cursor.x + size.x * 0.15f,
		.y = cursor.y + size.y * 0.15f,
		.width = size.x * 0.8f,
		.height = size.y * 0.8f,
	};
	Vector2 origin = {rec.width * 0.5f, rec.height * 0.5f};
	rec.x += origin.x;
	rec.y += origin.y;
	*rectangle_pro = (RayF_ORectangle_Pro) {
		.rec = rec,
		.origin = origin,
		.rotation = 30,
		.color = RAYWHITE,
	};
	rayf_objects_append(&objects, rayf_object_init(arena_alloc, rectangle_pro, RAYF_ORECTANGLE_PRO));
	cursor.x += size.x * 1.1f;
}
{
	Rectangle rec = {
		.x = cursor.x + size.x * 0.1f,
		.y = cursor.y + size.y * 0.1f,
		.width = size.x,
		.height = size.y,
	};
	float roundness = 0.6f;
	int segments = 100;
	rayf_objects_append(&objects, rayf_orectangle_rounded_init(arena_alloc, rec, roundness, segments, BLACK));
	rayf_objects_append(&objects, rayf_orectangle_rounded_lines_init(arena_alloc, rec, roundness, segments, LIME));
	cursor.x += size.x * 1.1f;
	rec.x += size.x * 1.1f;
	rayf_objects_append(&objects, 
		rayf_orectangle_rounded_lines_ex_init(arena_alloc, rec, roundness, segments, PURPLE, thick)
	);
	cursor.x += size.x * 1.1f;
}
{
	Vector2 p1 = Vector2Add(cursor, Vector2Scale(size, 0.1f));
	Vector2 p2 = Vector2Add(p1, (Vector2) {MIN(size.x, size.y) * 2.0f / sqrtf(3.0f), 0.0f});
	Vector2 p3 = Vector2Add(p1, Vector2Rotate(Vector2Subtract(p2, p1), PI / 3.0f));
	rayf_objects_append(&objects, rayf_otriangle_init(arena_alloc, p3, p2, p1, GRAY));
	rayf_objects_append(&objects, rayf_otriangle_lines_init(arena_alloc, p3, p2, p1, WHITE));
	cursor.x = 10 * dv.x;
	cursor.y += size.y * 1.1f;
}
{
	rayf_objects_append(&objects, rayf_otriangle_fan_init(arena_alloc, 4, YELLOW));
	RayF_OTriangle_Fan *tf = objects.items[objects.count - 1]->as.triangle_fan;
	tf->points[0] = Vector2Add(cursor, Vector2Scale(size, 0.5f));
	tf->points[1] = Vector2Add(cursor, (Vector2) {size.x * 0.7f, size.y});
	tf->points[2] = Vector2Add(cursor, (Vector2) {size.x,   0.0f});
	tf->points[3] = Vector2Add(cursor, (Vector2) {  0.0f,  0.0f});
	cursor.x += size.x * 1.1f;
	rayf_objects_append(&objects, rayf_otriangle_strip_init(arena_alloc, 4, GREEN));
	RayF_OTriangle_Fan *ts = objects.items[objects.count - 1]->as.triangle_fan;
	ts->points[0] = Vector2Add(cursor, (Vector2) {  0.0f,   0.0f});
	ts->points[1] = Vector2Add(cursor, (Vector2) {size.x * 0.2f, size.y * 0.8f});
	ts->points[2] = Vector2Add(cursor, (Vector2) {size.x * 0.8f, size.y * 0.2f});
	ts->points[3] = Vector2Add(cursor, (Vector2) {size.x, size.y});
	cursor.x += size.x * 1.1f;
}
{
	RayF_OPoly poly = {
		.center = Vector2Add(cursor, Vector2Scale(size, 0.55f)),
		.sides = 7,
		.radius = radius * 0.8,
		.rotation = 60.0f,
		.color = BROWN,
	};
	rayf_objects_append(&objects, rayf_opoly_init(arena_alloc,
				poly.center, poly.sides, poly.radius,
				poly.rotation, poly.color));
	poly.color = SKYBLUE;
	rayf_objects_append(&objects, rayf_opoly_lines_init(arena_alloc,
				poly.center, poly.sides, poly.radius,
				poly.rotation, poly.color));
	cursor.x += size.x * 1.1f;
	poly.center.x += size.x * 1.1f;
	RayF_OPoly_Lines_Ex *poly_lines_ex = arena_alloc(sizeof(RayF_OPoly_Lines_Ex));
	*poly_lines_ex = (RayF_OPoly_Lines_Ex) {
		.center = poly.center,
		.sides = 6,
		.radius = poly.radius,
		.rotation = poly.rotation,
		.color = YELLOW,
		.thick = thick,
	};
	rayf_objects_append(&objects, rayf_object_init(arena_alloc, poly_lines_ex, RAYF_OPOLY_LINES_EX));
	cursor.x += size.x * 1.1f;
}

	InitWindow(width, height, "Cool Example!");
	while(!WindowShouldClose()) {
		BeginDrawing();
		ClearBackground((Color) {0x18, 0x18, 0x18, 0xFF});
		rayf_objects_draw_all(objects);
		EndDrawing();
	}
	CloseWindow();
	rayf_objects_destroy_all(arena_free, objects);
	free(objects.items);
	return 0;
}
