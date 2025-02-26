#define RAYF_IMPLEMENTATION
#include <rayapi.h>
#include <raylib.h>
#include <raymath.h>
#include <stdio.h>

#define BUF_CAPACITY 128
#define WORDS_CAPACITY 10
int main() {
	RayF_StringView text = cp_to_sv(
		"Foo, bar, baz\\, biz\\, booz\\,, and so on"
	);
	array_str_static_alloc(buffer, BUF_CAPACITY, WORDS_CAPACITY);
	int count = 0;
	rayf_snsplit(text, ',', '\\', &count, buffer);
	printf("For \"%s\"\n", text.view);
	for(int i = 0; i < count; ++i) {
		printf("s[%d] = \"%s\"\n", i, buffer[i].buf);
	}
	return 0;
}
