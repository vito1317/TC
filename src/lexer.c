#include "lexer.h"

Lexer* create_lexer(string source, size_t length) {
    Lexer* lexer = (Lexer*)alloc_memory(sizeof(Lexer));
    lexer->source = source;
    lexer->position = 0;
    lexer->length = length;
    lexer->line = 0;
    lexer->column = 0;
    lexer->peeked_token = 0;
    lexer->peeked_position = 0;
    lexer->peeked_line = 0;
    lexer->peeked_column = 0;
    lexer->current_token = 0;
    return lexer;
}

static Token* create_token(TokenType type, string lexeme, size_t line, size_t column) {
    Token* token = (Token*)alloc_memory(sizeof(Token));
    token->type = type;
    token->lexeme = lexeme;
    token->line = line;
    token->column = column;
    return token;
}

static void lexer_error(const string message, size_t line, size_t column) {
    fprintf(stderr, "Lexer Error at Line %zu, Column %zu: %s\n", line + 1, column + 1, message);
}

static char get_current_char(Lexer* lexer) {
    if (lexer->position >= lexer->length)
        return '\0';
    lexer->column++;
    return lexer->source[lexer->position++];
}

static char peek_next_char(Lexer* lexer) {
    if (lexer->position >= lexer->length)
        return '\0';
    return lexer->source[lexer->position];
}

static void newline(Lexer* lexer) {
    lexer->line++;
    lexer->column = 0;
}

static void move_position(Lexer* lexer, int count) {
    if (count >= 0) {
        lexer->position += (size_t)count;
        lexer->column += (size_t)count;
    } else {
        lexer->position -= (size_t)(-count);
        lexer->column -= (size_t)(-count);
    }
}

static Token* next_token(Lexer* lexer, bool skip_comment) {
    char c = get_current_char(lexer);
    if (c == '\0')
        return create_token(EOF_TOKEN, 0, lexer->line, lexer->column);
    else if (c == ' ' || c == '\t' || c == '\r')
        return next_token(lexer, skip_comment);
    else if (c == '\n') {
        newline(lexer);
        return next_token(lexer, skip_comment);
    } else if (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || c == '_') {
        size_t start = lexer->position - 1;
        size_t column_start = lexer->column - 1;
        while (('a' <= c && c <= 'z') || ('A' <= c && c <= 'Z') || ('0' <= c && c <= '9') || c == '_')
            c = get_current_char(lexer);
        move_position(lexer, -1);
        string content = create_string(&lexer->source[start], lexer->position - start);
        if (is_keyword(content))
            return create_token(KEYWORD, content, lexer->line, column_start);
        else
            return create_token(IDENTIFIER, content, lexer->line, column_start);
    } else if ('0' <= c && c <= '9') {
        size_t start = lexer->position - 1;
        size_t column_start = lexer->column - 1;
        while ('0' <= c && c <= '9')
            c = get_current_char(lexer);
        TokenType type = INTEGER;
        char p = peek_next_char(lexer);
        if (c == '.' && ('0' <= p && p <= '9')) {
            c = get_current_char(lexer);
            while ('0' <= c && c <= '9')
                c = get_current_char(lexer);
            type = FLOAT;
        }
        move_position(lexer, -1);
        return create_token(type, create_string(&lexer->source[start], lexer->position - start), lexer->line, column_start);
    } else if (c == '"') {
        size_t start = lexer->position;
        size_t column_start = lexer->column - 1;
        c = get_current_char(lexer);
        while (c != '"' && c != '\0' && c != '\n')
            c = get_current_char(lexer);
        if (c != '"') {
            lexer_error("Unterminated string literal", lexer->line, start - 1);
            if (c == '\n') newline(lexer);
            return create_token(STRING, create_string(&lexer->source[start], lexer->position - start - 1), lexer->line, column_start);
        }
        return create_token(STRING, create_string(&lexer->source[start], lexer->position - start - 1), lexer->line, column_start);
    } else {
        char p = peek_next_char(lexer);
        if (c == '/' && p == '/') {
            size_t start = lexer->position + 1;
            size_t column_start = lexer->column - 1;
            while (c != '\n' && c != '\0')
                c = get_current_char(lexer);
            move_position(lexer, -1);
            if (skip_comment)
                return next_token(lexer, skip_comment);
            return create_token(COMMENT, create_string(&lexer->source[start], lexer->position - start), lexer->line, column_start);
        } else if (c == '/' && p == '*') {
            size_t start = lexer->position + 1;
            size_t column_start = lexer->column - 1;
            while (!(c == '*' && p == '/')) {
                c = get_current_char(lexer);
                p = peek_next_char(lexer);
                if (c == '\n') newline(lexer);
                if (p == '\0') break;
                assert(c != '\0');
            }
            if (p == '\0') {
                if (c == '\0') move_position(lexer, -1);
                lexer_error("Unterminated comment", lexer->line, start);
                if (skip_comment)
                    return next_token(lexer, skip_comment);
                return create_token(COMMENT, create_string(&lexer->source[start], lexer->position - start), lexer->line, column_start);
            }
            c = get_current_char(lexer);
            if (skip_comment)
                return next_token(lexer, skip_comment);
            return create_token(COMMENT, create_string(&lexer->source[start], lexer->position - start - 2), lexer->line, column_start);
        } else if (c == '=' && p == '=') {
            get_current_char(lexer);
            return create_token(SYMBOL, EQ_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '!' && p == '=') {
            get_current_char(lexer);
            return create_token(SYMBOL, NE_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '<' && p == '=') {
            get_current_char(lexer);
            return create_token(SYMBOL, LE_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '>' && p == '=') {
            get_current_char(lexer);
            return create_token(SYMBOL, GE_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '+' && p == '=') {
            get_current_char(lexer);
            return create_token(SYMBOL, ADD_ASSIGN_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '-' && p == '=') {
            get_current_char(lexer);
            return create_token(SYMBOL, SUB_ASSIGN_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '*' && p == '=') {
            get_current_char(lexer);
            return create_token(SYMBOL, MUL_ASSIGN_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '/' && p == '=') {
            get_current_char(lexer);
            return create_token(SYMBOL, DIV_ASSIGN_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '%' && p == '=') {
            get_current_char(lexer);
            return create_token(SYMBOL, MOD_ASSIGN_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '&' && p == '&') {
            get_current_char(lexer);
            return create_token(SYMBOL, AND_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '|' && p == '|') {
            get_current_char(lexer);
            return create_token(SYMBOL, OR_SYMBOL, lexer->line, lexer->column - 2);
        } else if (c == '(')
            return create_token(SYMBOL, L_PAREN_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == ')')
            return create_token(SYMBOL, R_PAREN_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '{')
            return create_token(SYMBOL, L_BRACE_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '}')
            return create_token(SYMBOL, R_BRACE_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == ',')
            return create_token(SYMBOL, COMMA_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '!')
            return create_token(SYMBOL, NOT_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '.')
            return create_token(SYMBOL, DOT_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '[')
            return create_token(SYMBOL, L_BRACKET_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == ']')
            return create_token(SYMBOL, R_BRACKET_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == ';')
            return create_token(SYMBOL, SEMICOLON_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '_')
            return create_token(SYMBOL, UNDERLINE_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '+')
            return create_token(SYMBOL, ADD_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '-')
            return create_token(SYMBOL, SUB_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '*')
            return create_token(SYMBOL, MUL_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '/')
            return create_token(SYMBOL, DIV_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '%')
            return create_token(SYMBOL, MOD_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '<')
            return create_token(SYMBOL, LT_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '>')
            return create_token(SYMBOL, GT_SYMBOL, lexer->line, lexer->column - 1);
        else if (c == '=')
            return create_token(SYMBOL, ASSIGN_SYMBOL, lexer->line, lexer->column - 1);
        else {
            lexer_error("Unexpected character", lexer->line, lexer->column - 1);
            return create_token(EOF_TOKEN, 0, 0, 0);
        }
    }
}

Token* get_next_token(Lexer* lexer, bool skip_comment) {
    if (lexer->peeked_token != 0) {
        lexer->current_token = lexer->peeked_token;
        lexer->position = lexer->peeked_position;
        lexer->line = lexer->peeked_line;
        lexer->column = lexer->peeked_column;
        lexer->peeked_token = 0;
        return lexer->current_token;
    }
    Token* token = next_token(lexer, skip_comment);
    return lexer->current_token = token;
}

Token* peek_next_token(Lexer* lexer, bool skip_comment) {
    if (lexer->peeked_token != 0)
        return lexer->peeked_token;
    size_t saved_position = lexer->position;
    size_t saved_line = lexer->line;
    size_t saved_column = lexer->column;
    Token* token = get_next_token(lexer, skip_comment);
    lexer->peeked_position = lexer->position;
    lexer->peeked_line = lexer->line;
    lexer->peeked_column = lexer->column;
    lexer->position = saved_position;
    lexer->line = saved_line;
    lexer->column = saved_column;
    lexer->peeked_token = token;
    return token;
}

inline Token* peek_current_token(Lexer* lexer) {
    return lexer->current_token;
}

void reset_lexer(Lexer* lexer) {
    lexer->position = 0;
    lexer->line = 0;
    lexer->column = 0;
    lexer->peeked_token = 0;
    lexer->peeked_position = 0;
    lexer->peeked_line = 0;
    lexer->peeked_column = 0;
    lexer->current_token = 0;
}
