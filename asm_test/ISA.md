# ISA design
## grammar
```bnf
code ::= { constant } { func_name ":" { [ label_name ":" ] instruction } }
constant ::= "str"  const_name ":" string_literal
           | "num"  const_name ":" number_literal
           | "arr"  const_name ":" "{" number_literal { [ "," number_literal ] } "}"
instruction ::= "add"  [ size ] target source source  # target = source + source
              | "sub"  [ size ] target source source  # target = source - source
              | "mul"  [ size ] target source source  # target = source * source
              | "div"  [ size ] target source source  # target = source / source
              | "and"  [ size ] target source source  # target = source & source
              | "or"   [ size ] target source source  # target = source | source
              | "xor"  [ size ] target source source  # target = source ^ source
              | "not"  [ size ] target source         # target = ~source
              | "sll"  [ size ] target source immediate  # target = source << immediate
              | "srl"  [ size ] target source immediate  # target = source >> immediate (logical shift)
              | "sra"  [ size ] target source immediate  # target = source >> immediate (arithmetic shift)
              | "load" [ size ] target source  # target = memory[source]
              | "save" [ size ] source source  # memory[source_a] = source_v
              | "mov"  [ size ] target source  # target = source
              | "cmpu" [ size ] source source  # compare source and source, set condition flags, unsigned compare
              | "cmps" [ size ] source source  # compare source and source, set condition flags, signed compare
              | "jmp"  label_name condition  # jump to label_name if condition is true
              | syntactic_sugar
syntactic_sugar ::= "call" func_name immediate # call function at func_name
                  | "ret"  # return from function
                  | "push" [ size ] source     # push source onto stack
                  | "pop"  [ size ] target     # pop from stack into target

label_name ::= "l." identifier
const_name ::= "c." identifier
func_name ::= "f." identifier
target ::= register
source ::= register | immediate | const_name
condition ::= "eq" | "ne" | "gt" | "lt" | "ge" | "le" | "always" | "never"
size ::= "byte" | "half" | "word" | "dword" | "custom" immediate  # 1 byte, 2 bytes, 4 bytes, 8 bytes, or custom size in bytes
string_literal ::= '"' { character } '"'
number_literal ::= digit { digit }
immediate ::= number_literal
register ::= special_register | general_register
general_register ::= "r" immediate | "a" immediate
special_register ::= "sp" | "fp" | "ip" | "rv" | "ap" | "lp" | "fg" | "t0" ~ "t8"  # total 16 registers
comment ::= ";" { character }
```
## semantics
### registers
`sp`: stack pointer  
`fp`: frame pointer  
`ip`: instruction pointer  
`rv`: return value  
`ap`: argument pointer  
`lp`: local variable pointer  
`fg`: flags for condition and comparison results  
`t0` ~ `t8`: temporary registers for calculations  
`rN`: general-purpose registers for variables and intermediate results  
`aN`: argument registers for callee to access arguments passed by caller
### stack frame layout
```
|----------------------------| <- higher address
|                            | <- ap
| aN register mapping area   |
|                            |
|----------------------------|
| return address             | <- fp ( return address == old instruction pointer)
|----------------------------|
| old stack pointer          |
|----------------------------|
| old frame pointer          |
|----------------------------|
| old argument pointer       |
|----------------------------|
| old local variable pointer |
|----------------------------|
| old flags                  |
|----------------------------|
|                            | <- lp
| rN register mapping area   |
|                            |
|----------------------------|
|                            |
| general stack area         |
|                            | <- sp
|----------------------------| <- smaller address
```
### function call convention
1. Caller push arguments onto stack.
2. Caller push return address onto stack and set `ap` to point to the first argument and set `ip` to the instruction after the call instruction.
3. Caller push old `fp`, `sp`, `ip`(= return address), `ap`, `lp`, `fg` on stack and set `fp`, `lp` to the new values for callee.
4. Callee allocate space for local variables.
5. Callee execute function body, using `rN` registers for intermediate results and variables.
6. Callee store return value in `rv` register.
7. Callee restore old `fp`, `sp`, `ip`, `ap`, `lp`, and `fg` from stack, and return to caller by jumping to the return address.  
`call` instruction: 2, 3  
`ret` instruction: 7
user: 1, 4, 5, 6
### condition flags
`fg` register layout:
```
| mean   | bit | value |
|--------|-----|-------|
| eq     | 010 | 2     |
| ne     | 101 | 5     |
| gt     | 001 | 1     |
| lt     | 100 | 4     |
| ge     | 011 | 3     |
| le     | 110 | 6     |
| always | 111 | 7     |
| never  | 000 | 0     |
```
### `aN` and `rN` register
`aN` and `rN` registers are syntactic sugar for stack access.  
They are mapped to the stack frame by `ap` and `lp` pointers.  
`aN = memory[ap + N * size]`  
get `aN` value:
```asm
mov  t0 ap
sub  t0 t0 N * size
load aN t0
```
save value to `aN`:
```asm
mov  t0 ap
sub  t0 t0 N * size
save aN t0
```
`rN = memory[lp + N * size]`  
get `rN` value:
```asm
mov  t0 lp
sub  t0 t0 N * size
load rN t0
```
save value to `rN`:
```asm
mov  t0 lp
sub  t0 t0 N * size
save rN t0
```
### `ret` and `call` instructions
N is the number of arguments.  
`call f.name N`:
```asm
add  t0 sp N * size
mov  t1 sp
push l.f_name  ; return address
push t0  ; stack pointer
push fp  ; frame pointer
push ap  ; argument pointer
push lp  ; local variable pointer
push fg  ; flags
mov  fp t1
mov  ap t0
mov  lp sp
jmp  f.name always
l.f_name:
```
`ret` instruction:
```asm
mov  sp lp
pop  fg
pop  lp
pop  ap
pop  fp
pop  t1
pop  t0
mov  sp t1
jmp  t0 always
```
### `push` and `pop` instructions
`push source`:
```asm
save source sp
sub  sp sp size
```
`pop target`:
```asm
add  sp sp size
load target sp
```
### size
- if compute is 32-bit, default size is `word`, `word` is 4 bytes, `half` is 2 bytes, `byte` is 1 byte, `dword` is not supported.
- if compute is 64-bit, default size if `dword`, `dword` is 8 bytes, `word` is 4 bytes, `half` is 2 bytes, `byte` is 1 byte.
- for `load` and `save` instructions, the size determines how many bytes to read/write from/to memory.
- for arithmetic and logic instructions, the size determines how to interpret the source operands and how to store the result in target. For example, if size is `byte`, the instruction will only operate on the least significant byte of the source operands and store the result in the least significant byte of the target, while keeping the other bytes unchanged.
### comments
- comments start with `;` and continue until the end of the line.
- comments can be placed anywhere in the code and are ignored by the assembler.
### labels
- labels are defined by a name followed by a colon, e.g. `l.loop_start:`.
- labels can be used as jump targets for `jmp` instructions.
- labels must be unique within anywhere in the code.
### constants
- constants are defined in the constant section at the beginning of the code.
- constants can be string literals, number literals, or array literals.
- constants are identified by a name, e.g. `c.str_1`, and can be used as source operands in instructions.
- string literals are enclosed in double quotes, e.g. `"Hello, World!"`.
- number literals are sequences of digits, e.g. `12345`.
- array literals are enclosed in curly braces and contain number literals separated by commas, e.g. `{1, 2, 3, 4}`.
### functions
- functions are defined by a name followed by a colon, e.g. `f.my_function:`.
- functions contain a sequence of instructions and can be called by `call` instructions.
- functions must be unique within anywhere in the code.
- the entry point of the program is the `main` function.
