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

    // Store original path to verify suffix is correctly removed
    size_t orig_len1 = strlen(f->path);
    size_t orig_ext_len1 = strlen(f->extension);

    change_file_extension(f, NULL);

    // The extension should be NULL
    assert(f->extension == NULL);

    // The new path should be exactly the old path minus the extension length
    size_t new_len1 = strlen(f->path);
    assert(new_len1 == orig_len1 - orig_ext_len1);

    // Test 2: File with extension, no directory (just name)
    File* f2 = create_file("myfile.txt");
    assert(f2 != NULL);

    // Store original path to verify suffix is correctly removed
    size_t orig_len2 = strlen(f2->path);
    size_t orig_ext_len2 = strlen(f2->extension);

    change_file_extension(f2, NULL);
    assert(f2->extension == NULL);

    size_t new_len2 = strlen(f2->path);
    assert(new_len2 == orig_len2 - orig_ext_len2);

    printf("Test passed!\n");

    // Test objects are allocated using alloc_memory() from lib.h
    // which manages memory internally in big blocks.
    // There is no free_file() in the codebase.

    return 0;
}
