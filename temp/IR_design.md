```ebnf
TAC ::= { design } entry_point { subroutine }
design ::= identifier(class_name) { attribute }
entry_point ::= identifier(subroutine_name)
subroutine ::= identifier(subroutine_name) { parameter } { local } { block }
block ::= label_name { instruction }
attribute ::= identifier(type) attribute_name offset
parameter ::= identifier(type) parameter_name
local ::= identifier(type) var_name
attribute_name ::= "a" integer
parameter_name ::= "p" integer
var_name ::= "v" integer
temp_var_name ::= "t" integer
block_name ::= "b" integer
instruction ::= add arg(result) arg(var_a) arg(var_b)
              | sub arg(result) arg(var_a) arg(var_b)
              | mul arg(result) arg(var_a) arg(var_b)
              | div arg(result) arg(var_a) arg(var_b)
              | mod arg(result) arg(var_a) arg(var_b)
              | eq  arg(result) arg(var_a) arg(var_b)
              | ne  arg(result) arg(var_a) arg(var_b)
              | lt  arg(result) arg(var_a) arg(var_b)
              | gt  arg(result) arg(var_a) arg(var_b)
              | le  arg(result) arg(var_a) arg(var_b)
              | ge  arg(result) arg(var_a) arg(var_b)
              | and arg(result) arg(var_a) arg(var_b)
              | or  arg(result) arg(var_a) arg(var_b)
              | not arg(result) arg(operand)
              | assign arg(result) arg(value)
              | set_attr arg(object) identifier(attribute_name) arg(value)
              | get_attr arg(result) arg(object) identifier(attribute_name)
              | set_elem arg(array) arg(index) arg(value)
              | get_elem arg(result) arg(array) arg(index)
              | param integer(size) arg(value)
              | alloc arg(var) identifier(type) integer(size)
              | jmp_t identifier(label) arg(condition)
              | jmp_f identifier(label) arg(condition)
              | jmp identifier(label)
              | ret arg(value)
              | call arg(var) identifier(subroutine_name) argument_count
arg ::= var_name | temp_var_name | parameter_name | literal
literal ::= integer | float | string | boolean | void
subroutine ::= class_name "." method_name | function_name
```
