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
    // We check that the extension is removed and the name matches.
    assert(strstr(f->path, ".txt") == NULL);
    assert(strcmp(f->name, "myfile") == 0);

    // Test 2: File with extension, no directory (just name)
    File* f2 = create_file("myfile.txt");
    assert(f2 != NULL);

    change_file_extension(f2, NULL);
    assert(f2->extension == NULL);

    assert(strstr(f2->path, ".txt") == NULL);
    assert(strcmp(f2->name, "myfile") == 0);

    printf("Test passed!\n");

    free_file(f);
    free_file(f2);

    return 0;
}
