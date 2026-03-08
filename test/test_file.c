#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include "file.h"
#include "lib.h"

int main() {
    init(); // initialize compiler memory system

    printf("Testing change_file_extension with new_extension == NULL...\n");

    // Test 1: File with extension in a directory
    File* f = create_file("test/dir/myfile.txt");
    assert(f != NULL);

    change_file_extension(f, NULL);

    // The extension should be NULL
    assert(f->extension == NULL);

    // The path should no longer have the extension
    // Because create_file calls absolute_path, the path might be absolute.
    // So we just check if it ends with "test/dir/myfile" instead of .txt
    size_t len = strlen(f->path);
    const char* expected_suffix = "test/dir/myfile";
    size_t suffix_len = strlen(expected_suffix);

    // Ensure path is long enough
    assert(len >= suffix_len);

    // Check suffix
    assert(strcmp(f->path + len - suffix_len, expected_suffix) == 0);

    // Test 2: File with extension, no directory (just name)
    File* f2 = create_file("myfile.txt");
    assert(f2 != NULL);

    change_file_extension(f2, NULL);
    assert(f2->extension == NULL);

    len = strlen(f2->path);
    expected_suffix = "myfile";
    suffix_len = strlen(expected_suffix);
    assert(len >= suffix_len);
    assert(strcmp(f2->path + len - suffix_len, expected_suffix) == 0);

    printf("Test passed!\n");

    return 0;
}
