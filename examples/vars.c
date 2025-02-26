#define RAYF_IMPLEMENTATION
#define RAYF_STRIP_PREFIXES
#include <rayapi.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#ifndef __SRC_LOC__
#define __SRC_LOC__ "./"
#endif

#define WORDS_CAPACITY 10
int main() {
	// Initializing structure
	RayF_Vars vars = rayf_vars_init();
	rayf_read_file(&vars, __SRC_LOC__ "hotkeys.ini");

{
	// Dynamic changing of variable
	RayF_StringView key = cp_to_sv("Test");
	int index = rayf_index_by_key(vars, key);
	RayF_String old_value = rayf_strcpyloc((rayf_value_by_key(vars, key)));
	const char* cp_new = strncmp(old_value.buf, "Foo", sizeof("Foo")) == 0 ? "Bar" : "Foo";
	rayf_update_var_by_index(&vars, index, cp_to_sv(cp_new));
	RayF_String new_value = rayf_strcpyloc((rayf_value_by_key(vars, key)));
	printf("Old: '%s', New: '%s'\n", old_value.buf, new_value.buf);
}
{
	// Usage of rayf_next_var
	while(rayf_next_var(&vars)) {
		printf("Key: '%s', Value: '%s', i: %d\n", vars.key.buf, vars.value.buf, rayf_index_by_key(vars, sb_to_sv(vars.key)));
	}
}
{
	// Trying to access non-existing key
	RayF_StringView non_existing_key = cp_to_sv("Forwardi");
	RayF_StringView non_existing_value = rayf_value_by_key(vars, non_existing_key);
	printf("Key (%d): '%s', founded value: '%s'\n",
			rayf_index_by_key(vars, non_existing_key),
			non_existing_key.view,
			non_existing_value.view ? non_existing_value.view : "Not Found"
	);
}
{
	// Freeing stage
	rayf_write_file(vars, __SRC_LOC__ "hotkeys.ini");
	rayf_free_vars(&vars);
}
	return 0;
}
