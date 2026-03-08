#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include "../include/file.h"
#include "../include/lib.h"

int main() {
    init();

    int num_components = 20000;
    char* path = malloc(num_components * 10 + 100);
    path[0] = '\0';

    strcat(path, "/root");
    for (int i = 0; i < num_components; i++) {
        strcat(path, "/comp");
    }
    strcat(path, "/file.txt");

    printf("Benchmarking with %d components...\n", num_components);

    clock_t start, end;
    double cpu_time_used;

    // Benchmark normalize_path (called inside create_file)
    start = clock();
    File* file = create_file(path);
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("create_file (includes normalize_path) took %f seconds\n", cpu_time_used);

    // Benchmark get_file_dir
    start = clock();
    for (int i = 0; i < 100; i++) {
        string dir = get_file_dir(file);
    }
    end = clock();
    cpu_time_used = ((double) (end - start)) / CLOCKS_PER_SEC;
    printf("get_file_dir (100 iterations) took %f seconds\n", cpu_time_used);

    free(path);
    return 0;
}
