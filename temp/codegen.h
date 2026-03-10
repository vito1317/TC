#ifndef CODEGEN_H
#define CODEGEN_H

#include "parser.h"

typedef struct Id {
    string name;
    size_t id;
} Id;
typedef struct TAC {
    list(Design*) designs;
    Id* entry_point;
    list(Subroutine*) subroutines;
} TAC;
typedef struct Design {
    Id* name;
    size_t size;
    list(Attribute*) attributes;
} Design;
typedef struct Attribute {
    Id* type;
    Id* name;
    size_t offset;
} Attribute;
typedef struct Subroutine {
    Id* name;
    list(Var*) parameters;
    list(Var*) local_vars;
    list(Block*) blocks;
} Subroutine;
typedef struct Block {
    Id* label;
    list(Instruction*) instructions;
} Block;
typedef struct Var {
    Id* name;
    Id* type;
} Var;
typedef enum ArgType {
    ARG_VARIABLE,
    ARG_CONSTANT,
    ARG_LABEL,
    ARG_DESIGN,
    ARG_NONE
} ArgType;
typedef struct Arg {
    union {
        Id* variable;
        string constant;
        Id* label;
        Design* design;
    } value;
    ArgType type;
} Arg;
typedef enum InstructionType {
    INST_ADD,       // +
    INST_SUB,       // -
    INST_MUL,       // *
    INST_DIV,       // /
    INST_MOD,       // %
    INST_EQ,        // ==
    INST_NE,        // !=
    INST_LT,        // <
    INST_GT,        // >
    INST_LE,        // <=
    INST_GE,        // >=
    INST_AND,       // &&
    INST_OR,        // ||
    INST_NOT,       // !
    INST_ASSIGN,    // =
    INST_SET_ATTR,  // set object attribute
    INST_GET_ATTR,  // get object attribute
    INST_SET_ELEM,  // set array element
    INST_GET_ELEM,  // get array element
    INST_PARAM,     // set parameter for function/method call
    INST_ALLOC,     // allocate size memory for variable
    INST_JMP_T,     // jump to label if condition is true
    INST_JMP_F,     // jump to label if condition is false
    INST_JMP,       // unconditional jump to label
    INST_RET,       // return from subroutine
    INST_CALL,      // call subroutine
    // INST_PUSH_FRAME,  // push a new frame
    // INST_POP_FRAME,   // pop the current frame
    INST_NONE
} InstructionType;
typedef struct Instruction {
    Arg* arg1;
    Arg* arg2;
    Arg* result;
    InstructionType type;
} Instruction;

typedef struct TACStatus {
    list(Design*) designs;
    Subroutine* current_subroutine;
    Block* current_block;
    size_t var_count;
    size_t if_count;
    size_t else_if_count;
    size_t for_count;
    size_t while_count;
    size_t label_count;
} TACStatus;

TAC* codegen_code(Code* code);
void codegen_code_member(CodeMember* code_member, TACStatus* status);
void codegen_import(Import* import, TACStatus* status);
void codegen_function(Function* function, TACStatus* status);
void codegen_method(Method* method, TACStatus* status);
void codegen_class_member(ClassMember* class_member, TACStatus* status);
void codegen_class(Class* class, TACStatus* status);
void codegen_variable(Variable* variable, TACStatus* status);
void codegen_statement(Statement* statement, TACStatus* status);
void codegen_if(If* if_, TACStatus* status);
void codegen_else_if(ElseIf* else_if, TACStatus* status);
void codegen_for(For* for_, TACStatus* status);
void codegen_while(While* while_, TACStatus* status);
Arg* codegen_expression(Expression* expression, TACStatus* status);
Arg* codegen_primary(Primary* primary, TACStatus* status);
Arg* codegen_variable_access(VariableAccess* variable_access, TACStatus* status);
Id* codegen_name(Name* name);

#endif  // CODEGEN_H
