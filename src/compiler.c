#include "compiler.h"

#include "file.h"
#include "helper.h"
#include "lexer.h"

void string_append(string dest, const size_t dest_length, const string src, const string new) {
    size_t src_length = strlen(src), new_length = strlen(new);
    if (dest_length <= src_length + new_length) {
        size_t max_src_length = dest_length - new_length - 1;
        snprintf(dest, dest_length, "%.*s%s", (int)max_src_length, src, new);
    } else if (src == dest)
        snprintf(dest + src_length, dest_length - src_length, "%s", new);
    else
        snprintf(dest, dest_length, "%s%s", src, new);
}
string read_source(FILE* file, size_t* length) {
    fseek(file, 0, SEEK_END);
    *length = (size_t)ftell(file);
    fseek(file, 0, SEEK_SET);

    string source = create_string("", *length + 1);
    size_t bytes_read = fread(source, 1, *length, file);
    source[bytes_read] = '\0';
    *length = bytes_read;

    for (size_t i = 0; i < *length; ++i) {
        if (source[i] == '\r' || source[i] == '\t')
            source[i] = ' ';
    }
    return source;
}
void output_token(FILE* file, Lexer* lexer) {
    for (Token* current = get_next_token(lexer, false); current != 0; current = get_next_token(lexer, false)) {
        Token* token = current;
        if (token->type == EOF_TOKEN) {
            fprintf(file, "Token(Type: EOF,         Line: %zu, Column: %zu)\n", token->line + 1, token->column + 1);
            break;
        } else if (token->type == IDENTIFIER)
            fprintf(file, "Token(Type: identifier,  Line: ");
        else if (token->type == INTEGER)
            fprintf(file, "Token(Type: integer,     Line: ");
        else if (token->type == FLOAT)
            fprintf(file, "Token(Type: float,       Line: ");
        else if (token->type == STRING)
            fprintf(file, "Token(Type: string,      Line: ");
        else if (token->type == SYMBOL)
            fprintf(file, "Token(Type: symbol,      Line: ");
        else if (token->type == KEYWORD)
            fprintf(file, "Token(Type: keyword,     Line: ");
        else if (token->type == COMMENT)
            fprintf(file, "Token(Type: comment,     Line: ");
        fprintf(file, "%zu, Column: %zu)\tLexeme: '", token->line + 1, token->column + 1);
        string lexeme_ptr = token->lexeme;
        for (size_t i = 0; i < strlen(lexeme_ptr); ++i) {
            char c = lexeme_ptr[i];
            if (c == '\0')
                fputs("\\0", file);
            else if (c == '\n')
                fputs("\\n", file);
            else if (c == '\t')
                fputs("\\t", file);
            else if (c == '\r')
                fputs("\\r", file);
            else
                fputc(c, file);
        }
        fprintf(file, "'\n");
    }
    fprintf(file, "\ninfo by lib:\n    %s\n", get_info());
}
void output_ast(FILE* file, Lexer* lexer, Parser* parser) {
    Code* ast_root = parse_code(lexer, builtin_scope, parser);
    output_code(ast_root, file, 0, parser);
    fprintf(file, "\ninfo by lib:\n    %s\n", get_info());
}
void parse_file(const string name, bool o_token, bool o_ast) {
    File* file = create_file(name);
    string filename = get_full_path(file);
    size_t length = 0;
    FILE* source_file = fopen(filename, "r");
    if (source_file == NULL) {
        fprintf(stderr, "Error opening file: %s", filename);
        return;
    }
    string source = read_source(source_file, &length);
    fclose(source_file);
    Lexer* lexer = create_lexer(source, length);
    if (o_token) {
        change_file_extension(file, create_string(".token", 6));
        string out_token_name = get_full_path(file);
        FILE* out_token_file = fopen(out_token_name, "w");
        if (out_token_file == NULL)
            fprintf(stderr, "Error opening file: %s\n", out_token_name);
        else {
            output_token(out_token_file, lexer);
            fclose(out_token_file);
        }
    }
    reset_lexer(lexer);
    Parser* parser = create_parser();
    if (o_ast) {
        change_file_extension(file, create_string(".ast", 4));
        string out_ast_name = get_full_path(file);
        FILE* out_ast_file = fopen(out_ast_name, "w");
        if (out_ast_file == NULL)
            fprintf(stderr, "Error opening file: %s\n", out_ast_name);
        else {
            output_ast(out_ast_file, lexer, parser);
            fclose(out_ast_file);
        }
    }
}
