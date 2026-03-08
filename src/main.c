#include "compiler.h"
#include <stdbool.h>

void print_usage(const char* prog_name) {
    printf("Usage: %s <source_file> [output_path] [options]\n", prog_name);
    printf("Parameters:\n");
    printf("  <source_file>  The TC source file you want to compile\n");
    printf("  [output_path]  (Optional) The directory where output files will be saved. Default is current directory.\n");
    printf("  [options]      (Optional) Compilation flags.\n");
    printf("Available Options:\n");
    printf("  -o             Output the compiled result (not yet implemented).\n");
    printf("  -a             Output the Abstract Syntax Tree (AST) to output_path/file_name.ast\n");
    printf("  -l             Output the lexer result to output_path/file_name.lex\n");
    printf("  -s             Output the symbol table to output_path/file_name.sym (not yet implemented)\n");
    printf("  -h             Display this help information.\n");
}

int main(int argc, char* argv[]) {
    if (argc < 2) {
        print_usage(argv[0]);
        return 1;
    }

    string source_file = NULL;
    string output_path = NULL;
    bool o_ast = false;
    bool o_token = false;

    // Parse arguments
    for (int i = 1; i < argc; i++) {
        if (argv[i][0] == '-') {
            // It's a flag
            for (int j = 1; argv[i][j] != '\0'; j++) {
                switch (argv[i][j]) {
                    case 'o': fprintf(stderr, "Error: Option '-o' is not yet implemented.\n"); return 1;
                    case 'a': o_ast = true; break;
                    case 'l': o_token = true; break;
                    case 's': fprintf(stderr, "Error: Option '-s' is not yet implemented.\n"); return 1;
                    case 'h': print_usage(argv[0]); return 0;
                    default:
                        fprintf(stderr, "Unknown option: -%c\n", argv[i][j]);
                        print_usage(argv[0]);
                        return 1;
                }
            }
        } else {
            // It's a positional argument
            if (source_file == NULL) {
                source_file = argv[i];
            } else if (output_path == NULL) {
                output_path = argv[i];
            } else {
                fprintf(stderr, "Too many positional arguments: %s\n", argv[i]);
                print_usage(argv[0]);
                return 1;
            }
        }
    }

    if (source_file == NULL) {
        fprintf(stderr, "Error: Missing <source_file>\n");
        print_usage(argv[0]);
        return 1;
    }

    // Call parse_file with the appropriate flags
    parse_file(source_file, output_path, o_token, o_ast);

    return 0;
}
