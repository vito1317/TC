#include "create.h"
#include "helper.h"
#include "lexer.h"

// parser functions
static Import* parse_import(Lexer* lexer, Scope* now_scope);
static Function* parse_function(Lexer* lexer, Scope* now_scope, Parser* parser);
static Method* parse_method(Lexer* lexer, Scope* now_scope, Name* class_name, Parser* parser);
static Class* parse_class(Lexer* lexer, Scope* now_scope, Parser* parser);
static Variable* parse_variable(Lexer* lexer, Scope* now_scope, Parser* parser);
static Statement* parse_statement(Lexer* lexer, Scope* now_scope, Parser* parser);
static If* parse_if(Lexer* lexer, Scope* now_scope, Parser* parser);
static For* parse_for(Lexer* lexer, Scope* now_scope, Parser* parser);
static While* parse_while(Lexer* lexer, Scope* now_scope, Parser* parser);
static Expression* parse_expression(Lexer* lexer, Scope* now_scope, Parser* parser);
static Primary* parse_primary(Lexer* lexer, Scope* now_scope, Parser* parser);
static VariableAccess* parse_variable_access(Lexer* lexer, Scope* now_scope, Parser* parser);

Code* parse_code(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_code\n");
#endif
    if (builtin_scope == 0) {
        builtin_scope = create_scope(0);
        name_void = create_name(VOID_KEYWORD, NAME_TYPE, NULL, NULL, builtin_scope);
        name_int = create_name(INT_KEYWORD, NAME_TYPE, NULL, NULL, builtin_scope);
        name_float = create_name(FLOAT_KEYWORD, NAME_TYPE, NULL, NULL, builtin_scope);
        name_string = create_name(STRING_KEYWORD, NAME_TYPE, NULL, NULL, builtin_scope);
        name_bool = create_name(BOOL_KEYWORD, NAME_TYPE, NULL, NULL, builtin_scope);
    }
    if (now_scope == 0)
        now_scope = builtin_scope;
    Token* token = 0;
    list(CodeMember*) members = create_list();
    Scope* global_scope = create_scope(now_scope);
    token = get_next_token(lexer, true);
    while (token != 0 && token->type != EOF_TOKEN) {
        if (token->type == KEYWORD && string_equal(token->lexeme, IMPORT_KEYWORD)) {
            Import* import = parse_import(lexer, global_scope);
            if (import == 0)
                parser_error("Failed to parse import statement", token);
            list_append(members, (pointer)create_code_member(CODE_IMPORT, import, NULL, NULL));
        } else if (token->type == KEYWORD && string_equal(token->lexeme, FUNC_KEYWORD)) {
            Function* function = parse_function(lexer, global_scope, parser);
            if (function == 0)
                parser_error("Failed to parse function declaration", token);
            list_append(members, (pointer)create_code_member(CODE_FUNCTION, NULL, function, NULL));
        } else if (token->type == KEYWORD && string_equal(token->lexeme, CLASS_KEYWORD)) {
            Class* class_ = parse_class(lexer, global_scope, parser);
            if (class_ == 0)
                parser_error("Failed to parse class declaration", token);
            list_append(members, (pointer)create_code_member(CODE_CLASS, NULL, NULL, class_));
        } else
            parser_error("Unexpected token in code member", token);
        token = get_next_token(lexer, true);
    }
    return create_code(members, global_scope);
}
Import* parse_import(Lexer* lexer, Scope* now_scope) {
#ifdef DEBUG
    fprintf(stderr, "into parse_import\n");
#endif
    Token* token = 0;
    token = get_next_token(lexer, true);
    if (token->type != IDENTIFIER) {
        parser_error("Expected identifier after 'import'", token);
        return NULL;
    }
    string import_name = token->lexeme;
    string source = 0;
    token = get_next_token(lexer, true);
    if (token->type == KEYWORD && string_equal(token->lexeme, FROM_KEYWORD)) {
        token = get_next_token(lexer, true);
        if (token->type != STRING) {
            parser_error("Expected string literal after 'from'", token);
            return NULL;
        }
        source = token->lexeme;
        token = get_next_token(lexer, true);
    }
    if (token->type != SYMBOL || !string_equal(token->lexeme, SEMICOLON_SYMBOL)) {
        parser_error("Expected ';' at end of import statement", token);
        return NULL;
    }
    Name* name;
    name = parse_import_file(import_name, source, now_scope);
    if (name == 0)
        name = create_name(import_name, NAME_VARIABLE, name_void, NULL, now_scope);
    return create_import(name, source);
}
Function* parse_function(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_function\n");
#endif
    Token* token = 0;
    Scope* function_scope = create_scope(now_scope);
    token = get_next_token(lexer, true);
    if (token->type != IDENTIFIER && !(token->type == KEYWORD && is_builtin_type(token->lexeme))) {
        parser_error("Expected function return type after 'func'", token);
        return NULL;
    }
    Name* return_type = search(now_scope, token->lexeme);
    if (return_type == 0)

    {
        parser_error("Unknown function return type", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    if (token->type != IDENTIFIER) {
        parser_error("Expected function name after return type", token);
        return NULL;
    }
    Name* name = create_name(token->lexeme, NAME_FUNCTION, return_type, NULL, now_scope);
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_PAREN_SYMBOL)) {
        parser_error("Expected '(' after function name", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    list(Variable*) parameters = create_list();
    while (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL)) {
        Variable* parameter = parse_variable(lexer, function_scope, parser);
        if (parameter == 0)
            parser_error("Failed to parse function parameter", token);
        else if (parameter->value != 0)
            parser_error("Function parameters cannot have default values", token);
        else
            list_append(parameters, (pointer)parameter);
        token = get_next_token(lexer, true);
        if (token->type == SYMBOL && string_equal(token->lexeme, COMMA_SYMBOL)) {
            token = get_next_token(lexer, true);
        } else if (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL)) {
            parser_error("Expected ',' or ')' after function parameter", token);
            return NULL;
        }
    }
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_BRACE_SYMBOL)) {
        parser_error("Expected '{' to start function body", token);
        return NULL;
    }
    list(Statement*) body = create_list();
    parser->in_function = true;
    bool have_return = false;
    token = get_next_token(lexer, true);
    while (token->type != SYMBOL || !string_equal(token->lexeme, R_BRACE_SYMBOL)) {
        Statement* statement = parse_statement(lexer, function_scope, parser);
        if (statement == 0)
            parser_error("Failed to parse function body statement", token);
        else if (have_return)
            parser_error("Unreachable code after return statement", token);
        if (((statement))->type == RETURN_STATEMENT)
            have_return = true;
        list_append(body, (pointer)statement);
        token = get_next_token(lexer, true);
    }
    parser->in_function = false;
    if (!have_return && return_type != name_void)
        parser_error("Function missing return statement", token);
    return create_function(name, return_type, parameters, body, function_scope);
}
Method* parse_method(Lexer* lexer, Scope* now_scope, Name* class_name, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_method\n");
#endif
    Token* token = 0;
    Scope* method_scope = create_scope(now_scope);
    Name* self = create_name(SELF_KEYWORD, NAME_VARIABLE, class_name, NULL, method_scope);
    token = get_next_token(lexer, true);
    if (token->type != IDENTIFIER && !(token->type == KEYWORD && is_builtin_type(token->lexeme))) {
        parser_error("Expected method return type after 'method'", token);
        return NULL;
    }
    Name* return_type = search(now_scope, token->lexeme);
    if (return_type == 0) {
        parser_error("Unknown return type for method", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    if (token->type != IDENTIFIER) {
        parser_error("Expected method name after return type", token);
        return NULL;
    }
    Name* name = create_name(token->lexeme, NAME_METHOD, return_type, NULL, now_scope);
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_PAREN_SYMBOL)) {
        parser_error("Expected '(' after method name", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    if (token->type != KEYWORD || !string_equal(token->lexeme, SELF_KEYWORD))
        parser_error("Expected 'self' as first parameter of method", token);
    token = get_next_token(lexer, true);
    list(Variable*) parameters = create_list();
    list_append(parameters, (pointer)create_variable(class_name, self, 0));
    while (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL)) {
        if (token->type == SYMBOL && string_equal(token->lexeme, COMMA_SYMBOL)) {
            token = get_next_token(lexer, true);
        } else if (token->type == SYMBOL && string_equal(token->lexeme, R_PAREN_SYMBOL)) break;
        else {
            parser_error("Expected ',' or ')' after method parameter", token);
            return NULL;
        }
        Variable* parameter = parse_variable(lexer, method_scope, parser);
        if (parameter == 0)
            parser_error("Failed to parse method parameter", token);
        else if (((parameter))->value != 0)
            parser_error("Method parameters cannot have default values", token);
        else
            list_append(parameters, (pointer)parameter);
        token = get_next_token(lexer, true);
    }
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_BRACE_SYMBOL)) {
        parser_error("Expected '{' to start method body", token);
        return NULL;
    }
    list(Statement*) body = create_list();
    parser->in_function = true;
    parser->in_method = true;
    bool have_return = false;
    token = get_next_token(lexer, true);
    while (token->type != SYMBOL || !string_equal(token->lexeme, R_BRACE_SYMBOL)) {
        Statement* statement = parse_statement(lexer, method_scope, parser);
        if (statement == 0)
            parser_error("Failed to parse method body statement", token);
        else if (have_return)
            parser_error("Unreachable code after return statement", token);
        if (((statement))->type == RETURN_STATEMENT)
            have_return = true;
        list_append(body, (pointer)statement);
        token = get_next_token(lexer, true);
    }
    parser->in_function = false;
    parser->in_method = false;
    if (!have_return && return_type != name_void)
        parser_error("Method missing return statement", token);
    return create_method(name, return_type, parameters, body, method_scope);
}
Class* parse_class(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_class\n");
#endif
    Token* token = 0;
    Scope* class_scope = create_scope(now_scope);
    token = get_next_token(lexer, true);
    if (token->type != IDENTIFIER) {
        parser_error("Expected class name after 'class'", token);
        return NULL;
    }
    Name* name = create_name(token->lexeme, NAME_CLASS, NULL, class_scope, now_scope);
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_BRACE_SYMBOL)) {
        parser_error("Expected '{' to start class body", token);
        return NULL;
    }
    list(ClassMember*) members = create_list();
    token = get_next_token(lexer, true);
    while (token->type != SYMBOL || !string_equal(token->lexeme, R_BRACE_SYMBOL)) {
        if (token->type == KEYWORD && string_equal(token->lexeme, METHOD_KEYWORD)) {
            Method* method = parse_method(lexer, class_scope, name, parser);
            if (method == 0)
                parser_error("Failed to parse class method", token);
            list_append(members, (pointer)create_class_member(CLASS_METHOD, method, NULL));
        } else if (token->type == KEYWORD && string_equal(token->lexeme, VAR_KEYWORD)) {
            token = get_next_token(lexer, true);
            Variable* variable = parse_variable(lexer, class_scope, parser);
            if (variable == 0)
                parser_error("Failed to parse class variable", token);
            list_append(members, (pointer)create_class_member(CLASS_VARIABLE, NULL, variable));
            token = get_next_token(lexer, true);
            if (token->type != SYMBOL || !string_equal(token->lexeme, SEMICOLON_SYMBOL))
                parser_error("Expected ';' after class variable declaration", token);
        } else
            parser_error("Unexpected token in class member", token);
        token = get_next_token(lexer, true);
    }
    Name* constructor = search(class_scope, CONSTRUCTOR_NAME);
    if (constructor == NULL)
        constructor = create_name(CONSTRUCTOR_NAME, NAME_METHOD, name, NULL, now_scope);
    if (constructor->kind != NAME_METHOD)
        parser_error("Constructor name conflicts with existing member", token);
    return create_class(name, members, class_scope);
}
Variable* parse_variable(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_variable\n");
#endif
    Token* token = 0;
    token = peek_current_token(lexer);
    if (token->type != IDENTIFIER && !(token->type == KEYWORD && is_builtin_type(token->lexeme))) {
        parser_error("Expected variable type", token);
        return NULL;
    }
    Name* type = search(now_scope, token->lexeme);
    if (type != 0) {
        Name* type_ptr = (type);
        if (type_ptr->kind != NAME_TYPE && type_ptr->kind != NAME_CLASS)
            parser_error("Expected a type for variable declaration", token);
    } else
        parser_error("Unknown variable type", token);
    token = get_next_token(lexer, true);
    if (token->type != IDENTIFIER)
        parser_error("Expected variable name", token);
    Name* name = create_name(token->lexeme, NAME_VARIABLE, type, NULL, now_scope);
    Expression* value = 0;
    token = peek_next_token(lexer, true);
    if (token->type == SYMBOL && string_equal(token->lexeme, ASSIGN_SYMBOL)) {
        token = get_next_token(lexer, true);  // consume '='
        token = get_next_token(lexer, true);
        value = parse_expression(lexer, now_scope, parser);
        if (value == 0)
            parser_error("Failed to parse variable initializer", token);
    }
    return create_variable(type, name, value);
}
Statement* parse_statement(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_statement\n");
#endif
    Token* token = 0;
    token = peek_current_token(lexer);
    Statement* statement = 0;
    if (token->type == KEYWORD) {
        if (string_equal(token->lexeme, IF_KEYWORD))
            return create_statement(IF_STATEMENT, parse_if(lexer, now_scope, parser), NULL, NULL, NULL, NULL);
        else if (string_equal(token->lexeme, FOR_KEYWORD))
            return create_statement(FOR_STATEMENT, NULL, NULL, parse_for(lexer, now_scope, parser), NULL, NULL);
        else if (string_equal(token->lexeme, WHILE_KEYWORD))
            return create_statement(WHILE_STATEMENT, NULL, parse_while(lexer, now_scope, parser), NULL, NULL, NULL);
        else if (string_equal(token->lexeme, VAR_KEYWORD)) {
            get_next_token(lexer, true);
            statement = create_statement(VARIABLE_STATEMENT, NULL, NULL, NULL, NULL, parse_variable(lexer, now_scope, parser));
        } else if (string_equal(token->lexeme, RETURN_KEYWORD)) {
            token = get_next_token(lexer, true);
            statement = create_statement(RETURN_STATEMENT, NULL, NULL, NULL, parse_expression(lexer, now_scope, parser), NULL);
        } else if (string_equal(token->lexeme, BREAK_KEYWORD)) {
            if (!(parser->in_loop))

            {
                parser_error("Cannot use 'break' outside of a loop", token);
                return NULL;
            }
            statement = create_statement(BREAK_STATEMENT, NULL, NULL, NULL, NULL, NULL);
        } else if (string_equal(token->lexeme, CONTINUE_KEYWORD)) {
            if (!parser->in_loop) {
                parser_error("Cannot use 'continue' outside of a loop", token);
                return NULL;
            }
            statement = create_statement(CONTINUE_STATEMENT, NULL, NULL, NULL, NULL, NULL);
        } else
            statement = create_statement(EXPRESSION_STATEMENT, NULL, NULL, NULL, parse_expression(lexer, now_scope, parser), NULL);
    } else
        statement = create_statement(EXPRESSION_STATEMENT, NULL, NULL, NULL, parse_expression(lexer, now_scope, parser), NULL);
    token = peek_current_token(lexer);
    if (statement == 0)
        parser_error("Failed to parse statement", token);
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, SEMICOLON_SYMBOL))
        parser_error("Expected ';' after statement", token);
    return statement;
}
If* parse_if(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_if\n");
#endif
    Token* token = 0;
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_PAREN_SYMBOL)) {
        parser_error("Expected '(' after 'if'", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    Expression* condition = parse_expression(lexer, now_scope, parser);
    if (condition == 0)
        parser_error("Failed to parse if condition", token);
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL)) {
        parser_error("Expected ')' after if condition", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_BRACE_SYMBOL)) {
        parser_error("Expected '{' to start if body", token);
        return NULL;
    }
    list(Statement*) body = create_list();
    token = get_next_token(lexer, true);
    while (token->type != SYMBOL || !string_equal(token->lexeme, R_BRACE_SYMBOL)) {
        Statement* statement = parse_statement(lexer, now_scope, parser);
        if (statement == 0)
            parser_error("Failed to parse if body statement", token);
        list_append(body, (pointer)statement);
        token = get_next_token(lexer, true);
    }
    list(ElseIf*) else_if = create_list();
    list(Statement*) else_body = create_list();
    token = peek_next_token(lexer, true);
    while (token->type == KEYWORD && string_equal(token->lexeme, ELIF_KEYWORD)) {
        token = get_next_token(lexer, true);  // consume 'elif'
        token = get_next_token(lexer, true);
        if (token->type != SYMBOL || !string_equal(token->lexeme, L_PAREN_SYMBOL)) {
            parser_error("Expected '(' after 'elif'", token);
            return NULL;
        }
        token = get_next_token(lexer, true);
        Expression* elif_condition = parse_expression(lexer, now_scope, parser);
        if (elif_condition == 0)
            parser_error("Failed to parse else-if condition", token);
        token = get_next_token(lexer, true);
        if (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL)) {
            parser_error("Expected ')' after else-if condition", token);
            return NULL;
        }
        token = get_next_token(lexer, true);
        if (token->type != SYMBOL || !string_equal(token->lexeme, L_BRACE_SYMBOL)) {
            parser_error("Expected '{' to start else-if body", token);
            return NULL;
        }
        list(Statement*) elif_body = create_list();
        token = get_next_token(lexer, true);
        while (token->type != SYMBOL || !string_equal(token->lexeme, R_BRACE_SYMBOL)) {
            Statement* statement = parse_statement(lexer, now_scope, parser);
            if (statement == 0)
                parser_error("Failed to parse else-if body statement", token);
            list_append(elif_body, (pointer)statement);
            token = get_next_token(lexer, true);
        }
        list_append(else_if, (pointer)create_else_if(elif_condition, elif_body));
        token = peek_next_token(lexer, true);
    }
    if (token->type == KEYWORD && string_equal(token->lexeme, ELSE_KEYWORD)) {
        token = get_next_token(lexer, true);  // consume 'else'
        token = get_next_token(lexer, true);
        if (token->type != SYMBOL || !string_equal(token->lexeme, L_BRACE_SYMBOL)) {
            parser_error("Expected '{' to start else body", token);
            return NULL;
        }
        token = get_next_token(lexer, true);
        while (token->type != SYMBOL || !string_equal(token->lexeme, R_BRACE_SYMBOL)) {
            Statement* statement = parse_statement(lexer, now_scope, parser);
            if (statement == 0)
                parser_error("Failed to parse else body statement", token);
            list_append(else_body, (pointer)statement);
            token = get_next_token(lexer, true);
        }
    }
    token = peek_current_token(lexer);
    return create_if(condition, body, else_if, else_body);
}
For* parse_for(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_for\n");
#endif
    Token* token = 0;
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_PAREN_SYMBOL)) {
        parser_error("Expected '(' after 'for'", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    Variable* initializer = 0;
    if (token->type != SYMBOL || !string_equal(token->lexeme, SEMICOLON_SYMBOL)) {
        initializer = parse_variable(lexer, now_scope, parser);
        if (initializer == 0)
            parser_error("Failed to parse for loop initializer", token);
        token = get_next_token(lexer, true);
    }
    if (token->type != SYMBOL || !string_equal(token->lexeme, SEMICOLON_SYMBOL))

    {
        parser_error("Expected ';' after for loop initializer", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    Expression* condition = 0;
    if (token->type != SYMBOL || !string_equal(token->lexeme, SEMICOLON_SYMBOL)) {
        condition = parse_expression(lexer, now_scope, parser);
        if (condition == 0)
            parser_error("Failed to parse for loop condition", token);
        token = get_next_token(lexer, true);
    }
    if (token->type != SYMBOL || !string_equal(token->lexeme, SEMICOLON_SYMBOL))

    {
        parser_error("Expected ';' after for loop condition", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    Expression* increment = 0;
    if (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL)) {
        increment = parse_expression(lexer, now_scope, parser);
        if (increment == 0)
            parser_error("Failed to parse for loop increment", token);
        token = get_next_token(lexer, true);
    }
    if (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL))

    {
        parser_error("Expected ')' after for loop increment", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_BRACE_SYMBOL)) {
        parser_error("Expected '{' to start for loop body", token);
        return NULL;
    }
    list(Statement*) body = create_list();
    parser->in_loop = true;
    token = get_next_token(lexer, true);
    while (token->type != SYMBOL || !string_equal(token->lexeme, R_BRACE_SYMBOL)) {
        Statement* statement = parse_statement(lexer, now_scope, parser);
        if (statement == 0)
            parser_error("Failed to parse for loop body statement", token);
        list_append(body, (pointer)statement);
        token = get_next_token(lexer, true);
    }
    parser->in_loop = false;
    return create_for(initializer, condition, increment, body);
}
While* parse_while(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_while\n");
#endif
    Token* token = 0;
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_PAREN_SYMBOL)) {
        parser_error("Expected '(' after 'while'", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    Expression* condition = parse_expression(lexer, now_scope, parser);
    if (condition == 0)
        parser_error("Failed to parse while condition", token);
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL)) {
        parser_error("Expected ')' after while condition", token);
        return NULL;
    }
    token = get_next_token(lexer, true);
    if (token->type != SYMBOL || !string_equal(token->lexeme, L_BRACE_SYMBOL)) {
        parser_error("Expected '{' to start while body", token);
        return NULL;
    }
    list(Statement*) body = create_list();
    parser->in_loop = true;
    token = get_next_token(lexer, true);
    while (token->type != SYMBOL || !string_equal(token->lexeme, R_BRACE_SYMBOL)) {
        Statement* statement = parse_statement(lexer, now_scope, parser);
        if (statement == 0)
            parser_error("Failed to parse while body statement", token);
        list_append(body, (pointer)statement);
        token = get_next_token(lexer, true);
    }
    parser->in_loop = false;
    return create_while(condition, body);
}
static Expression* parse_expr_prec(Lexer* lexer, Expression* expr_left, int min_prec, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_expr_prec\n");
#endif
    Token* token = 0;
    token = peek_next_token(lexer, true);
    while (token->type == SYMBOL) {
        OperatorType op = string_to_operator(token->lexeme);
        int op_prec = operator_precedence(op);
        if (op == OP_NONE || op_prec < min_prec)
            break;
        token = get_next_token(lexer, true);  // consume operator
        token = get_next_token(lexer, true);
        Primary* right_primary = parse_primary(lexer, now_scope, parser);
        if (right_primary == 0) {
            parser_error("Failed to parse right operand", token);
            return NULL;
        }
        Expression* right = create_expression(OP_NONE, NULL, right_primary, 0);
        token = peek_next_token(lexer, true);
        while (token->type == SYMBOL) {
            OperatorType next_op = string_to_operator(token->lexeme);
            int next_prec = operator_precedence(next_op);
            if (next_op == OP_NONE || next_prec <= op_prec)
                break;
            right = parse_expr_prec(lexer, right, next_prec, now_scope, parser);
            if (right == 0)
                return NULL;
            token = peek_next_token(lexer, true);
        }
        expr_left = create_expression(op, expr_left, NULL, right);
        token = peek_next_token(lexer, true);
    }
    return expr_left;
}
Expression* parse_expression(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_expression\n");
#endif
    Primary* left_primary = parse_primary(lexer, now_scope, parser);
    if (left_primary == 0) {
        parser_error("Failed to parse expression primary", peek_current_token(lexer));
        return NULL;
    }
    return parse_expr_prec(lexer, create_expression(OP_NONE, NULL, left_primary, NULL), 0, now_scope, parser);
}
Primary* parse_primary(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_primary\n");
#endif
    Token* token = 0;
    token = peek_current_token(lexer);
    PrimaryType type;
    string str_value = NULL;
    Expression* expr_value = NULL;
    Primary* prim_value = NULL;
    VariableAccess* variable_value = NULL;
    if (token->type == INTEGER) {
        type = PRIM_INTEGER;
        str_value = token->lexeme;
    } else if (token->type == FLOAT) {
        type = PRIM_FLOAT;
        str_value = token->lexeme;
    } else if (token->type == STRING) {
        type = PRIM_STRING;
        str_value = token->lexeme;
    } else if (token->type == KEYWORD && string_equal(token->lexeme, TRUE_KEYWORD)) {
        type = PRIM_TRUE;
        str_value = token->lexeme;
    } else if (token->type == KEYWORD && string_equal(token->lexeme, FALSE_KEYWORD)) {
        type = PRIM_FALSE;
        str_value = token->lexeme;
    } else if (token->type == SYMBOL && string_equal(token->lexeme, L_PAREN_SYMBOL)) {
        type = PRIM_EXPRESSION;
        token = get_next_token(lexer, true);
        expr_value = parse_expression(lexer, now_scope, parser);
        if (expr_value == NULL) {
            parser_error("Failed to parse parenthesized expression", token);
            return NULL;
        }
        token = get_next_token(lexer, true);
        if (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL)) {
            parser_error("Expected ')' after expression", token);
            return NULL;
        }
    } else if (token->type == SYMBOL && string_equal(token->lexeme, NOT_SYMBOL)) {
        type = PRIM_NOT_OPERAND;
        token = get_next_token(lexer, true);
        prim_value = parse_primary(lexer, now_scope, parser);
        if (prim_value == 0) {
            parser_error("Failed to parse operand of unary '!'", token);
            return NULL;
        }
    } else if (token->type == SYMBOL && string_equal(token->lexeme, SUB_SYMBOL)) {
        type = PRIM_NEG_OPERAND;
        token = get_next_token(lexer, true);
        prim_value = parse_primary(lexer, now_scope, parser);
        if (prim_value == 0) {
            parser_error("Failed to parse operand of unary '-'", token);
            return NULL;
        }
    } else if (token->type == IDENTIFIER || (token->type == KEYWORD && string_equal(token->lexeme, SELF_KEYWORD) && parser->in_method)) {
        type = PRIM_VARIABLE_ACCESS;
        variable_value = parse_variable_access(lexer, now_scope, parser);
        if (variable_value == 0) {
            parser_error("Failed to parse variable access", token);
            return NULL;
        }
    } else {
        parser_error("Unexpected token in primary expression", token);
        return NULL;
    }
    return create_primary(type, str_value, expr_value, prim_value, variable_value);
}
VariableAccess* parse_variable_access(Lexer* lexer, Scope* now_scope, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into parse_variable_access\n");
#endif
    Token* token = peek_current_token(lexer);
    if (token->type != IDENTIFIER && !(token->type == KEYWORD && string_equal(token->lexeme, SELF_KEYWORD))) {
        parser_error("Expected variable name in variable access", token);
        return NULL;
    }
    Name* current_type = 0;
    Name* base_name = 0;
    Scope* var_scope = 0;
    base_name = search(now_scope, token->lexeme);
    VariableAccess* base = create_variable_access(VAR_NAME, 0, base_name, NULL, NULL);
    token = peek_next_token(lexer, true);
    while (token->type == SYMBOL) {
        if (base_name != NULL) {
            if (base_name->kind == NAME_VARIABLE || base_name->kind == NAME_ATTRIBUTE || base_name->kind == NAME_FUNCTION || base_name->kind == NAME_METHOD)
                current_type = base_name->info.type;
            else if (base_name->kind == NAME_CLASS || base_name->kind == NAME_TYPE)
                current_type = base_name;
        }
        if (var_scope == NULL && current_type != NULL) {
            Name* type_ptr = (current_type);
            if (type_ptr->kind == NAME_CLASS)
                var_scope = type_ptr->info.scope;
        }
        if (string_equal(token->lexeme, L_PAREN_SYMBOL)) {
            token = get_next_token(lexer, true);  // consume '('
            if (base_name == 0)
                parser_error("Cannot call undefined variable", token);
            else if (base_name->kind == NAME_CLASS) {
                base_name = search(base_name->info.scope, CONSTRUCTOR_NAME);
                base = create_variable_access(VAR_GET_ATTR, base, base_name, NULL, NULL);
            }
            if (base_name->kind != NAME_FUNCTION && base_name->kind != NAME_METHOD)
                parser_error("Cannot call non-function variable", token);
            token = get_next_token(lexer, true);
            list(Expression*) args = create_list();
            while (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL)) {
                Expression* arg = parse_expression(lexer, now_scope, parser);
                if (arg == 0)
                    parser_error("Failed to parse function call argument", token);
                list_append(args, (pointer)arg);
                token = get_next_token(lexer, true);
                if (token->type == SYMBOL && string_equal(token->lexeme, COMMA_SYMBOL)) {
                    token = get_next_token(lexer, true);
                } else if (token->type != SYMBOL || !string_equal(token->lexeme, R_PAREN_SYMBOL)) {
                    parser_error("Expected ',' or ')' after function call argument", token);
                    return NULL;
                }
            }
            base = create_variable_access(VAR_FUNC_CALL, base, NULL, NULL, args);
            base_name = base_name->info.type;
            current_type = 0;
            var_scope = 0;
        } else if (string_equal(token->lexeme, L_BRACKET_SYMBOL)) {
            token = get_next_token(lexer, true);  // consume '['
            token = get_next_token(lexer, true);
            Expression* index = parse_expression(lexer, now_scope, parser);
            if (index == 0)
                parser_error("Failed to parse sequence index", token);
            token = get_next_token(lexer, true);
            if (token->type != SYMBOL || !string_equal(token->lexeme, R_BRACKET_SYMBOL)) {
                parser_error("Expected ']' after sequence index", token);
                return NULL;
            }
            base = create_variable_access(VAR_GET_SEQ, base, NULL, index, NULL);
        } else if (string_equal(token->lexeme, DOT_SYMBOL)) {
            token = get_next_token(lexer, true);  // consume '.'
            token = get_next_token(lexer, true);
            if (var_scope == 0) {
                parser_error("Cannot access attribute without a valid scope", token);
                return NULL;
            }
            if (token->type != IDENTIFIER) {
                parser_error("Expected attribute name after '.'", token);
                return NULL;
            }
            base_name = search(var_scope, token->lexeme);
            if (base_name == 0) {
                parser_error("Unknown attribute name", token);
                return NULL;
            }
            base = create_variable_access(VAR_GET_ATTR, base, base_name, NULL, NULL);
            current_type = 0;
            var_scope = 0;
        } else
            break;
        token = peek_next_token(lexer, true);
    }
    return base;
}
