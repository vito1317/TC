#ifndef COMPILER_H
#define COMPILER_H

#include "lib.h"

#define MAX_FILENAME_SIZE 1024

typedef struct Lexer Lexer;
typedef struct Parser Parser;
void string_append(string dest, const size_t dest_length, const string src, const string new);
string read_source(FILE* file, size_t* length);
void output_token(FILE* file, Lexer* lexer);
void output_ast(FILE* file, Lexer* lexer, Parser* parser);
void parse_file(const string name, bool o_token, bool o_ast);

#endif  // COMPILER_H
