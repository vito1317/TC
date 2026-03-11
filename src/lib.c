#include "lib.h"

static const char* keywordStrings[keywordCount] = {"import", "from", "func", "class", "method", "self", "if", "elif", "else", "while", "for", "true", "false", "return", "break", "continue", "int", "float", "string", "bool", "void", "var"};
string keywordList[keywordCount] = {0};
static const char* symbolStrings[symbolCount] = {"(", ")", "{", "}", ",", "!", ".", "[", "]", ";", "_", "+", "-", "*", "/", "%", "<", ">", "=", "==", "!=", "<=", ">=", "+=", "-=", "*=", "/=", "%=", "&&", "||"};
string symbolList[symbolCount] = {0};

MemoryBlock* struct_memory = NULL;
MemoryBlock* string_memory = NULL;
bool initialized = false;

StringList* all_string_list = 0;

string CONSTRUCTOR_NAME = 0;

string IMPORT_KEYWORD = 0;
string FROM_KEYWORD = 0;
string FUNC_KEYWORD = 0;
string CLASS_KEYWORD = 0;
string METHOD_KEYWORD = 0;
string SELF_KEYWORD = 0;
string IF_KEYWORD = 0;
string ELIF_KEYWORD = 0;
string ELSE_KEYWORD = 0;
string WHILE_KEYWORD = 0;
string FOR_KEYWORD = 0;
string TRUE_KEYWORD = 0;
string FALSE_KEYWORD = 0;
string RETURN_KEYWORD = 0;
string BREAK_KEYWORD = 0;
string CONTINUE_KEYWORD = 0;
string INT_KEYWORD = 0;
string FLOAT_KEYWORD = 0;
string STRING_KEYWORD = 0;
string BOOL_KEYWORD = 0;
string VOID_KEYWORD = 0;
string VAR_KEYWORD = 0;
string L_PAREN_SYMBOL = 0;
string R_PAREN_SYMBOL = 0;
string L_BRACE_SYMBOL = 0;
string R_BRACE_SYMBOL = 0;
string COMMA_SYMBOL = 0;
string NOT_SYMBOL = 0;
string DOT_SYMBOL = 0;
string L_BRACKET_SYMBOL = 0;
string R_BRACKET_SYMBOL = 0;
string SEMICOLON_SYMBOL = 0;
string UNDERLINE_SYMBOL = 0;
string ADD_SYMBOL = 0;
string SUB_SYMBOL = 0;
string MUL_SYMBOL = 0;
string DIV_SYMBOL = 0;
string MOD_SYMBOL = 0;
string LT_SYMBOL = 0;
string GT_SYMBOL = 0;
string ASSIGN_SYMBOL = 0;
string EQ_SYMBOL = 0;
string NE_SYMBOL = 0;
string LE_SYMBOL = 0;
string GE_SYMBOL = 0;
string ADD_ASSIGN_SYMBOL = 0;
string SUB_ASSIGN_SYMBOL = 0;
string MUL_ASSIGN_SYMBOL = 0;
string DIV_ASSIGN_SYMBOL = 0;
string MOD_ASSIGN_SYMBOL = 0;
string AND_SYMBOL = 0;
string OR_SYMBOL = 0;
Name* name_void = 0;
Name* name_int = 0;
Name* name_float = 0;
Name* name_string = 0;
Name* name_bool = 0;
Scope* builtin_scope = 0;

static size_t struct_memory_used = 0;
static size_t string_memory_used = 0;
static size_t struct_memory_count = 0;
static size_t string_memory_count = 0;

static void increase_memory_size(bool for_struct) {
    MemoryBlock* new_block = (MemoryBlock*)malloc(sizeof(MemoryBlock));
    if (new_block == NULL) {
        fprintf(stderr, "Fatal: Cannot allocate memory\n");
        MemoryBlock* current = string_memory;
        while (current != NULL) {
            MemoryBlock* next = current->next;
            free(current->block);
            free(current);
            current = next;
        }
        initialized = false;
        exit(1);
    }
    new_block->block = (pointer)malloc(defaultMemorySize);
    new_block->size = defaultMemorySize;
    new_block->used = 0;
    new_block->next = NULL;
    if (for_struct) {
        struct_memory_used += struct_memory->used;
        new_block->next = struct_memory;
        struct_memory = new_block;
        struct_memory_count += defaultMemorySize;
#ifdef DEBUG
        fprintf(stderr, "[DEBUG]: Add new memory block for struct\n");
#endif
    } else {
        string_memory_used += string_memory->used;
        new_block->next = string_memory;
        string_memory = new_block;
        string_memory_count += defaultMemorySize;
#ifdef DEBUG
        fprintf(stderr, "[DEBUG]: Add new memory block for string\n");
#endif
    }
}

static string alloc_big_memory(size_t size) {
    string_memory_count += size;
    string_memory_used += size;
    char* block = (char*)malloc(size);
    fprintf(stderr, "Info: Allocate big memory block of size %zu bytes\n", size);
    if (block == NULL) {
        fprintf(stderr, "Fatal: Cannot allocate memory\n");
        exit(1);
    }
    return block;
}

static string create_string_check(const char* data, size_t length, bool check) {
    if (!initialized) init();
    if (data == NULL || length == 0) return 0;
    if (check) {
        StringList* current = all_string_list;
        string existing = NULL;
        while (current != NULL) {
            if (current->length == length && current->str != 0 && strncmp(current->str, data, length) == 0)
                existing = current->str;
            current = current->next;
        }
        if (existing != NULL)
            return existing;
    }
    char* str;
    if (length >= defaultMemorySize - 1)
        str = alloc_big_memory(length + 1);
    else {
        if (string_memory->used + length >= string_memory->size)
            increase_memory_size(false);
        str = &((char*)(string_memory->block))[string_memory->used];
        string_memory->used += length + 1;
    }
    strncpy(str, data, length);
    str[length] = '\0';
    StringList* new_str = (StringList*)alloc_memory(sizeof(StringList));
    new_str->str = str;
    new_str->length = length;
    new_str->next = all_string_list;
    all_string_list = new_str;
    return str;
}

string create_string(const char* data, size_t length) {
    return create_string_check(data, length, true);
}

void init(void) {
    if (initialized) return;
    if (struct_memory == NULL) {
        struct_memory = (MemoryBlock*)malloc(sizeof(MemoryBlock));
        if (struct_memory == NULL) {
            fprintf(stderr, "Fatal: Cannot allocate memory\n");
            initialized = false;
            exit(1);
        }
        struct_memory->block = (size_t*)malloc(defaultMemorySize);
        struct_memory->size = defaultMemorySize;
        struct_memory->used = 0;
        struct_memory->next = NULL;
        struct_memory_count = defaultMemorySize;
    }
    if (string_memory == NULL) {
        string_memory = (MemoryBlock*)malloc(sizeof(MemoryBlock));
        if (string_memory == NULL) {
            fprintf(stderr, "Fatal: Cannot allocate memory\n");
            initialized = false;
            exit(1);
        }
        string_memory->block = (pointer)malloc(defaultMemorySize);
        string_memory->size = defaultMemorySize;
        string_memory->used = 0;
        string_memory->next = NULL;
        string_memory_count = defaultMemorySize;
    }
    initialized = true;
    for (size_t i = 0; i < keywordCount; ++i)
        keywordList[i] = create_string_check(keywordStrings[i], strlen(keywordStrings[i]), false);
    for (size_t i = 0; i < symbolCount; ++i)
        symbolList[i] = create_string_check(symbolStrings[i], strlen(symbolStrings[i]), false);
    CONSTRUCTOR_NAME = create_string_check("init", 4, false);
    IMPORT_KEYWORD = keywordList[0];
    FROM_KEYWORD = keywordList[1];
    FUNC_KEYWORD = keywordList[2];
    CLASS_KEYWORD = keywordList[3];
    METHOD_KEYWORD = keywordList[4];
    SELF_KEYWORD = keywordList[5];
    IF_KEYWORD = keywordList[6];
    ELIF_KEYWORD = keywordList[7];
    ELSE_KEYWORD = keywordList[8];
    WHILE_KEYWORD = keywordList[9];
    FOR_KEYWORD = keywordList[10];
    TRUE_KEYWORD = keywordList[11];
    FALSE_KEYWORD = keywordList[12];
    RETURN_KEYWORD = keywordList[13];
    BREAK_KEYWORD = keywordList[14];
    CONTINUE_KEYWORD = keywordList[15];
    INT_KEYWORD = keywordList[16];
    FLOAT_KEYWORD = keywordList[17];
    STRING_KEYWORD = keywordList[18];
    BOOL_KEYWORD = keywordList[19];
    VOID_KEYWORD = keywordList[20];
    VAR_KEYWORD = keywordList[21];
    L_PAREN_SYMBOL = symbolList[0];
    R_PAREN_SYMBOL = symbolList[1];
    L_BRACE_SYMBOL = symbolList[2];
    R_BRACE_SYMBOL = symbolList[3];
    COMMA_SYMBOL = symbolList[4];
    NOT_SYMBOL = symbolList[5];
    DOT_SYMBOL = symbolList[6];
    L_BRACKET_SYMBOL = symbolList[7];
    R_BRACKET_SYMBOL = symbolList[8];
    SEMICOLON_SYMBOL = symbolList[9];
    UNDERLINE_SYMBOL = symbolList[10];
    ADD_SYMBOL = symbolList[11];
    SUB_SYMBOL = symbolList[12];
    MUL_SYMBOL = symbolList[13];
    DIV_SYMBOL = symbolList[14];
    MOD_SYMBOL = symbolList[15];
    LT_SYMBOL = symbolList[16];
    GT_SYMBOL = symbolList[17];
    ASSIGN_SYMBOL = symbolList[18];
    EQ_SYMBOL = symbolList[19];
    NE_SYMBOL = symbolList[20];
    LE_SYMBOL = symbolList[21];
    GE_SYMBOL = symbolList[22];
    ADD_ASSIGN_SYMBOL = symbolList[23];
    SUB_ASSIGN_SYMBOL = symbolList[24];
    MUL_ASSIGN_SYMBOL = symbolList[25];
    DIV_ASSIGN_SYMBOL = symbolList[26];
    MOD_ASSIGN_SYMBOL = symbolList[27];
    AND_SYMBOL = symbolList[28];
    OR_SYMBOL = symbolList[29];
}

static size_t memoryBlockCount = 0;

pointer alloc_memory(size_t size) {
    if (!initialized) init();
    if (struct_memory->used + size >= struct_memory->size)
        increase_memory_size(true);
    size = (size + ALIGN_SIZE - 1) & ~(ALIGN_SIZE - 1);
    assert(struct_memory->used % ALIGN_SIZE == 0);
    size_t* ptr = struct_memory->block + (struct_memory->used / ALIGN_SIZE);
    struct_memory->used += size;
    ++memoryBlockCount;
    return ptr;
}

bool is_keyword(const string str) {
    if (!initialized) init();
    for (size_t i = 0; i < keywordCount; ++i)
        if (string_equal(str, keywordList[i]))
            return true;
    return false;
}

bool string_equal(string a, string b) {
    return a == b;
}

#define MEM_USED_STR_MAX 48
#define INFO_STR_MAX 240

string get_info(void) {
    size_t stringCount = 0;
    StringList* current = all_string_list;
    while (current != NULL) {
        stringCount++;
        current = current->next;
    }
    // max: 47 char
    string struct_memory_used_str = create_string_check("", MEM_USED_STR_MAX, false);
    if (snprintf(struct_memory_used_str, MEM_USED_STR_MAX + 1, "%zu/%zu bytes", struct_memory_used + struct_memory->used, struct_memory_count) >= (int)(MEM_USED_STR_MAX + 1)) {
        // Truncation is acceptable for info reporting, but let's be safe
    }
    // max: 47 char
    string string_memory_used_str = create_string_check("", MEM_USED_STR_MAX, false);
    if (snprintf(string_memory_used_str, MEM_USED_STR_MAX + 1, "%zu/%zu bytes", string_memory_used + string_memory->used, string_memory_count) >= (int)(MEM_USED_STR_MAX + 1)) {
        // Truncation is acceptable for info reporting, but let's be safe
    }
    // max: 239 char
    string info = (string)create_string_check("", INFO_STR_MAX, false);
    if (snprintf(info, INFO_STR_MAX + 1, "Platform: %d, Structure Memory Used: %s, String Memory Used: %s, stringCount: %zu, Memory Block Count: %zu", PLATFORM, struct_memory_used_str, string_memory_used_str, stringCount, memoryBlockCount) >= (int)(INFO_STR_MAX + 1)) {
        // Truncation is acceptable for info reporting, but let's be safe
    }
    return info;
}
