#include <stdio.h>
#include <stdlib.h>
#include <time.h>

double get_time() {
    struct timespec ts;
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return ts.tv_sec + ts.tv_nsec * 1e-9;
}

int main() {
    const int runs = 30;
    double total_time = 0;
	
    for (int i = 0; i < runs; ++i) {
        double start = get_time();
        
        system("./game");

        double end = get_time();
        total_time += (end - start);
    }

    printf("Average time: %.6f seconds\n", total_time / runs);
    return 0;
}
