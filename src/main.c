#include "compiler.h"

int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <filename>\n", argv[0]);
        return 1;
    }
    string filename = argv[1];

    parse_file(filename, true, true);

    return 0;
}
