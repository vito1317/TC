#include "helper.h"

static void output_code_member(CodeMember* code_member, FILE* outfile, size_t indent, Parser* parser);
static void output_import(Import* import, FILE* outfile, size_t indent, Parser* parser);
static void output_function(Function* function, FILE* outfile, size_t indent, Parser* parser);
static void output_method(Method* method, FILE* outfile, size_t indent, Parser* parser);
static void output_class_member(ClassMember* class_member, FILE* outfile, size_t indent, Parser* parser);
static void output_class(Class* class, FILE* outfile, size_t indent, Parser* parser);
static void output_variable(Variable* variable, FILE* outfile, size_t indent, Parser* parser);
static void output_statement(Statement* statement, FILE* outfile, size_t indent, Parser* parser);
static void output_if(If* if_, FILE* outfile, size_t indent, Parser* parser);
static void output_else_if(ElseIf* else_if, FILE* outfile, size_t indent, Parser* parser);
static void output_for(For* for_, FILE* outfile, size_t indent, Parser* parser);
static void output_while(While* while_, FILE* outfile, size_t indent, Parser* parser);
static void output_expression(Expression* expression, FILE* outfile, size_t indent, Parser* parser);
static void output_primary(Primary* primary, FILE* outfile, size_t indent, Parser* parser);
static void output_variable_access(VariableAccess* variable_access, FILE* outfile, size_t indent, Parser* parser);
static void output_name(Name* name, FILE* outfile, size_t indent, Parser* parser);

void output_code_member(CodeMember* code_member, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_code_member\n");
#endif
    switch (code_member->type) {
        case CODE_IMPORT:
            OUT(0, false, "import\n");
            output_import(code_member->content.import, outfile, indent + 1, parser);
            break;
        case CODE_FUNCTION:
            OUT(0, false, "function\n");
            output_function(code_member->content.function, outfile, indent + 1, parser);
            break;
        case CODE_CLASS:
            OUT(0, false, "class\n");
            output_class(code_member->content.class_, outfile, indent + 1, parser);
            break;
        default:
            OUT(0, false, "unknown_CodeMemberType: %u\n", code_member->type);
            break;
    }
}
void output_code(Code* code, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_code\n");
#endif
    list(CodeMember*) members = list_copy(code->members);
    CodeMember* member;
    while ((member = (CodeMember*)list_pop(members)) != 0)
        output_code_member(member, outfile, indent + 1, parser);
}
void output_import(Import* import, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_import\n");
#endif
    OUT(0, false, "name\n");
    output_name(import->name, outfile, indent + 1, parser);
    OUT(0, true, "source: \"%s\"\n", import->source != 0 ? import->source : "NULL");
}
void output_function(Function* function, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_function\n");
#endif
    OUT(0, false, "name\n");
    output_name(function->name, outfile, indent + 1, parser);
    OUT(0, false, "return_type\n");
    output_name(function->return_type, outfile, indent + 1, parser);
    OUT(0, false, "parameters\n");
    list(Variable*) parameters = list_copy(function->parameters);
    list(Statement*) body = list_copy(function->body);
    Variable* parameter;
    int index = -1;
    while ((parameter = (Variable*)list_pop(parameters)) != 0) {
        OUT(1, false, "parameters[%d]\n", ++index);
        output_variable(parameter, outfile, indent + 2, parser);
    }
    OUT(0, true, "body\n");
    Statement* statement;
    while ((statement = (Statement*)list_pop(body)) != 0)
        output_statement(statement, outfile, indent + 1, parser);
}
void output_method(Method* method, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_method\n");
#endif
    OUT(0, false, "name\n");
    output_name(method->name, outfile, indent + 1, parser);
    OUT(0, false, "return_type\n");
    output_name(method->return_type, outfile, indent + 1, parser);
    OUT(0, false, "parameters\n");
    list(Variable*) parameters = list_copy(method->parameters);
    list(Statement*) body = list_copy(method->body);
    Variable* parameter;
    int index = -1;
    while ((parameter = (Variable*)list_pop(parameters)) != 0) {
        OUT(1, false, "parameters[%d]\n", ++index);
        output_variable(parameter, outfile, indent + 2, parser);
    }
    OUT(0, true, "body\n");
    Statement* statement;
    while ((statement = (Statement*)list_pop(body)) != 0)
        output_statement(statement, outfile, indent + 1, parser);
}
void output_class_member(ClassMember* class_member, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_class_member\n");
#endif
    switch (class_member->type) {
        case CLASS_METHOD:
            OUT(0, false, "method\n");
            output_method(class_member->content.method, outfile, indent + 1, parser);
            break;
        case CLASS_VARIABLE:
            OUT(0, false, "variable\n");
            output_variable(class_member->content.variable, outfile, indent + 1, parser);
            break;
        default:
            OUT(0, false, "unknown_ClassMemberType: %u\n", class_member->type);
            break;
    }
}
void output_class(Class* class, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_class\n");
#endif
    OUT(0, false, "name\n");
    output_name(class->name, outfile, indent + 1, parser);
    OUT(0, true, "members\n");
    list(ClassMember*) members = list_copy(class->members);
    ClassMember* member;
    while ((member = (ClassMember*)list_pop(members)) != 0)
        output_class_member(member, outfile, indent + 1, parser);
}
void output_variable(Variable* variable, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_variable\n");
#endif
    OUT(0, false, "type\n");
    output_name(variable->type, outfile, indent + 1, parser);
    OUT(0, false, "name\n");
    output_name(variable->name, outfile, indent + 1, parser);
    if (variable->value != 0) {
        OUT(0, true, "value\n");
        output_expression(variable->value, outfile, indent + 1, parser);
    } else
        OUT(0, true, "value: \"NULL\"\n");
}
void output_statement(Statement* statement, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_statement\n");
#endif
    switch (statement->type) {
        case IF_STATEMENT:
            OUT(0, false, "if_statement\n");
            output_if(statement->stmt.if_stmt, outfile, indent + 1, parser);
            break;
        case FOR_STATEMENT:
            OUT(0, false, "for_statement\n");
            output_for(statement->stmt.for_stmt, outfile, indent + 1, parser);
            break;
        case WHILE_STATEMENT:
            OUT(0, false, "while_statement\n");
            output_while(statement->stmt.while_stmt, outfile, indent + 1, parser);
            break;
        case VARIABLE_STATEMENT:
            OUT(0, false, "variable_declaration_statement\n");
            output_variable(statement->stmt.var, outfile, indent + 1, parser);
            break;
        case RETURN_STATEMENT:
            if (statement->stmt.expr == 0) {
                OUT(0, false, "return_statement: \"NULL\"\n");
                return;
            }
            OUT(0, false, "return_statement\n");
            output_expression(statement->stmt.expr, outfile, indent + 1, parser);
            break;
        case BREAK_STATEMENT:
            OUT(0, false, "break_statement: \"NULL\"\n");
            return;
        case CONTINUE_STATEMENT:
            OUT(0, false, "continue_statement: \"NULL\"\n");
            return;
        case EXPRESSION_STATEMENT:
            if (statement->stmt.expr == 0) {
                OUT(0, false, "expression_statement: \"NULL\"\n");
                return;
            }
            OUT(0, false, "expression_statement\n");
            output_expression(statement->stmt.expr, outfile, indent + 1, parser);
            break;
        default:
            OUT(0, false, "unknown_StatementType: %u\n", statement->type);
            return;
    }
}
void output_if(If* if_, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_if\n");
#endif
    list(Statement*) body = list_copy(if_->body);
    list(ElseIf*) else_if_list = list_copy(if_->else_if);
    list(Statement*) else_body = list_copy(if_->else_body);
    OUT(0, false, "condition\n");
    output_expression(if_->condition, outfile, indent + 1, parser);
    OUT(0, false, "body\n");
    Statement* statement;
    while ((statement = (Statement*)list_pop(body)) != 0)
        output_statement(statement, outfile, indent + 1, parser);
    OUT(0, false, "else_if\n");
    ElseIf* else_if;
    while ((else_if = (ElseIf*)list_pop(else_if_list)) != 0)
        output_else_if(else_if, outfile, indent + 1, parser);
    OUT(0, true, "else_body\n");
    while ((statement = (Statement*)list_pop(else_body)) != 0)
        output_statement(statement, outfile, indent + 1, parser);
}
void output_else_if(ElseIf* else_if, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_else_if\n");
#endif
    list(Statement*) body = list_copy(else_if->body);
    OUT(0, false, "condition\n");
    output_expression(else_if->condition, outfile, indent + 1, parser);
    OUT(0, true, "body\n");
    Statement* statement;
    while ((statement = (Statement*)list_pop(body)) != 0)
        output_statement(statement, outfile, indent + 1, parser);
}
void output_for(For* for_, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_for\n");
#endif
    list(Statement*) body = list_copy(for_->body);
    if (for_->initializer != 0) {
        OUT(0, false, "initializer\n");
        output_variable(for_->initializer, outfile, indent + 1, parser);
    } else
        OUT(0, false, "initializer: \"NULL\"\n");
    if (for_->condition != 0) {
        OUT(0, false, "condition\n");
        output_expression(for_->condition, outfile, indent + 1, parser);
    } else
        OUT(0, false, "condition: \"NULL\"\n");
    if (for_->increment != 0) {
        OUT(0, false, "increment\n");
        output_expression(for_->increment, outfile, indent + 1, parser);
    } else
        OUT(0, false, "increment: \"NULL\"\n");
    OUT(0, true, "body\n");
    Statement* statement;
    while ((statement = (Statement*)list_pop(body)) != 0)
        output_statement(statement, outfile, indent + 1, parser);
}
void output_while(While* while_, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_while\n");
#endif
    list(Statement*) body = list_copy(while_->body);
    OUT(0, false, "condition\n");
    output_expression(while_->condition, outfile, indent + 1, parser);
    OUT(0, true, "body\n");
    Statement* statement;
    while ((statement = (Statement*)list_pop(body)) != 0)
        output_statement(statement, outfile, indent + 1, parser);
}
void output_expression(Expression* expression, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_expression\n");
#endif
    if (expression->operator == OP_NONE) {
        OUT(0, true, "primary\n");
        output_primary(expression->prim_left, outfile, indent + 1, parser);
    } else {
        string op_str = operator_to_string(expression->operator);
        OUT(0, false, "operator: \"%s\"\n", op_str ? (op_str) : "UNKNOWN_OPERATOR");
        OUT(0, false, "left\n");
        output_expression(expression->expr_left, outfile, indent + 1, parser);
        OUT(0, true, "right\n");
        output_expression(expression->right, outfile, indent + 1, parser);
    }
}
void output_primary(Primary* primary, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_primary\n");
#endif
    switch (primary->type) {
        case PRIM_INTEGER:
            OUT(0, false, "type: \"integer\"\n");
            OUT(0, true, "value: %s\n", (primary->value.literal_value));
            break;
        case PRIM_FLOAT:
            OUT(0, false, "type: \"float\"\n");
            OUT(0, true, "value: %s\n", (primary->value.literal_value));
            break;
        case PRIM_STRING:
            OUT(0, false, "type: \"string\"\n");
            OUT(0, true, "value: \"%s\"\n", (primary->value.literal_value));
            break;
        case PRIM_TRUE:
            OUT(0, false, "type: \"boolean\"\n");
            OUT(0, true, "value: \"true\"\n");
            break;
        case PRIM_FALSE:
            OUT(0, false, "type: \"boolean\"\n");
            OUT(0, true, "value: \"false\"\n");
            break;
        case PRIM_EXPRESSION:
            OUT(0, false, "type: \"expression\"\n");
            OUT(0, true, "value\n");
            output_expression(primary->value.expr, outfile, indent + 1, parser);
            break;
        case PRIM_NOT_OPERAND:
            OUT(0, false, "type: \"!\"\n");
            OUT(0, true, "value\n");
            output_primary(primary->value.operand, outfile, indent + 1, parser);
            break;
        case PRIM_NEG_OPERAND:
            OUT(0, false, "type: \"-\"\n");
            OUT(0, true, "value\n");
            output_primary(primary->value.operand, outfile, indent + 1, parser);
            break;
        case PRIM_VARIABLE_ACCESS:
            OUT(0, false, "type: \"variable\"\n");
            OUT(0, true, "value\n");
            output_variable_access(primary->value.var, outfile, indent + 1, parser);
            break;
        default:
            OUT(0, true, "unknown_PrimaryType: %u\n", primary->type);
            break;
    }
}
void output_variable_access(VariableAccess* variable_access, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_variable_access\n");
#endif
    list(Expression*) args;
    int index;
    switch (variable_access->type) {
        case VAR_NAME:
            OUT(0, false, "type: \"name\"\n");
            OUT(0, true, "name\n");
            output_name(variable_access->content.name, outfile, indent + 1, parser);
            break;
        case VAR_FUNC_CALL:
            args = list_copy(variable_access->content.args);
            OUT(0, false, "type: \"function_call\"\n");
            OUT(0, false, "function\n");
            output_variable_access(variable_access->base, outfile, indent + 1, parser);
            OUT(0, true, "arguments\n");
            Expression* arg;
            index = -1;
            while ((arg = (Expression*)list_pop(args)) != 0) {
                OUT(1, false, "arguments[%d]\n", ++index);
                output_expression(arg, outfile, indent + 2, parser);
            }
            break;
        case VAR_GET_SEQ:
            OUT(0, false, "type: \"get sequence_element\"\n");
            OUT(0, false, "sequence\n");
            output_variable_access(variable_access->base, outfile, indent + 1, parser);
            OUT(0, true, "index\n");
            output_expression(variable_access->content.index, outfile, indent + 1, parser);
            break;
        case VAR_GET_ATTR:
            OUT(0, false, "type: \"get_attribute\"\n");
            OUT(0, false, "object\n");
            output_variable_access(variable_access->base, outfile, indent + 1, parser);
            OUT(0, true, "attribute_name\n");
            output_name(variable_access->content.attr_name, outfile, indent + 1, parser);
            break;
        default:
            OUT(0, true, "unknown_VariableAccessType: %u\n", variable_access->type);
            break;
    }
}
void output_name(Name* name, FILE* outfile, size_t indent, Parser* parser) {
#ifdef DEBUG
    fprintf(stderr, "into output_name\n");
#endif
    if (name == NULL) {
        OUT(0, true, "Name pointer: \"NULL\"\n");
        return;
    }
    OUT(0, false, "name: \"%s\"\n", (name->name));
    OUT(0, false, "id: %zu\n", name->id);
    switch (name->kind) {
        case NAME_TYPE:
            OUT(0, true, "kind: \"type\"\n");
            break;
        case NAME_VARIABLE:
            OUT(0, false, "kind: \"variable\"\n");
            OUT(0, true, "type\n");
            output_name(name->info.type, outfile, indent + 1, parser);
            break;
        case NAME_FUNCTION:
            OUT(0, false, "kind: \"function\"\n");
            OUT(0, true, "return_type\n");
            output_name(name->info.type, outfile, indent + 1, parser);
            break;
        case NAME_METHOD:
            OUT(0, false, "kind: \"method\"\n");
            OUT(0, true, "return_type\n");
            output_name(name->info.type, outfile, indent + 1, parser);
            break;
        case NAME_CLASS:
            OUT(0, true, "kind: \"class\"\n");
            break;
        case NAME_ATTRIBUTE:
            OUT(0, false, "kind: \"attribute\"\n");
            OUT(0, true, "type\n");
            output_name(name->info.type, outfile, indent + 1, parser);
            break;
        default:
            OUT(0, true, "kind: \"unknown_NameType\"\n");
            break;
    }
}
