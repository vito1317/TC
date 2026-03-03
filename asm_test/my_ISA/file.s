str c.str_0: ""
str c.str_1: "%s/%s"
str c.str_2: "/"
str c.str_3: "%s"

; string get_cwd(void) {
f.get_cwd:
    ; return getcwd(NULL, 0);  // MinGW or LINUX or MACOS
    push 0
    push 0
    call f.getcwd 2
    ret
; }

; File* create_file(const string path) {
f.create_file:
    ; a0: path
    ; File* file = (File*)alloc_memory(sizeof(File));
    push 4 * size  ; size = sizeof(File)
    call f.alloc_memory 1
    mov  r0 rv  ; r0: file
    ; file->path = absolute_path(path);
    push a0
    call f.absolute_path 1
    add  r1 r0 3 * size  ; size = sizeof(size_t), offset of path in File struct
    save r1 rv
    ; normalize_path(file);
    push r0
    call f.normalize_path 1
    ; return file;
    mov rv r0
    ret
; }

; string absolute_path(string path) {
f.absolute_path:
    ; a0: path
    ; size_t path_len = strlen(path);
    push a0
    call f.strlen 1
    mov r0 rv  ; r0: path_len
    ; for (size_t i = 0; i < path_len; i++) {
l.for_start_0:
    mov  r1 0  ; r1: i
    cmpu r1 r0
    jmp  l.for_end_0 ge
    ; if (path[i] == '\\') {
    add  r2 a0 r1
    load r3 r2
    cmpu byte r3 92  ; '\\' == 92
    jmp  l.if_end_0 ne
    ; path[i] = '/';
    save byte r2 47  ; '/' == 47
    ; }
l.if_end_0:
    ; if (i > 1 && path[i] == '/' && path[i - 1] == '.' && path[i - 2] == '/') {
    cmpu r1 1
    jmp  l.if_end_1 le
    add  r2 a0 r1
    load r2 r2
    cmpu byte r2 47  ; '/' == 47
    jmp  l.if_end_1 ne
    add  r2 a0 r1
    sub  r2 r2 1
    load r2 r2
    cmpu byte r2 46  ; '.' == 46
    jmp  l.if_end_1 ne
    add  r2 a0 r1
    sub  r2 r2 2
    load r2 r2
    cmpu byte r2 47  ; '/' == 47
    jmp  l.if_end_1 ne
    ; memmove(path + i - 1, path + i + 1, path_len - i);
    add  r2 a0 r1
    sub  r2 r2 1
    push r2
    add  r2 a0 r1
    add  r2 r2 1
    push r2
    sub  r2 r0 r1
    push r2
    call f.memmove 3
    ; path_len -= 2;
    sub  r0 r0 2
    ; i -= 2;
    sub  r1 r1 2
    ; }
l.if_end_1:
    ; if (i > 0 && path[i] == '/' && path[i - 1] == '/') {
    cmpu r1 0
    jmp  l.if_end_2 le
    add  r2 a0 r1
    load r2 r2
    cmpu byte r2 47  ; '/' == 47
    jmp  l.if_end_2 ne
    add  r2 a0 r1
    sub  r2 r2 1
    load r2 r2
    cmpu byte r2 47  ; '/' == 47
    jmp  l.if_end_2 ne
    ; memmove(path + i, path + i + 1, path_len - i);
    add  r2 a0 r1
    push r2
    add  r2 r2 1
    push r2
    sub  r2 r0 r1
    push r2
    call f.memmove 3
    ; path_len--;
    sub  r0 r0 1
    ; i--;
    sub  r1 r1 1
    ; }
l.if_end_2:
    add  r1 r1 1
    ; }
l.for_end_0:
    ; if (path_len > 0 && path[0] == '/') {
    cmpu r0 0
    jmp  l.if_end_3 le
    load r1 a0
    cmpu byte r1 47  ; '/' == 47
    ; return path;
    mov  rv a0
    ret
    ; }
l.if_end_3:
    ; string cwd = get_cwd();
    call f.get_cwd 0
    mov  r1 rv  ; r1: cwd
    ; if (cwd == NULL) {
    cmpu r1 0
    jmp  l.if_end_4 ne
    ; return path;
    mov  rv a0
    ret
    ; }
l.if_end_4:
    ; size_t total_len = strlen(cwd) + 1 + path_len + 1;
    push r1
    call f.strlen 1
    add  r2 rv 1
    add  r2 r2 r0
    add  r2 r2 1  ; r2: total_len
    ; string abs_path = create_string("", total_len);
    push c.str_0
    push r2
    call f.create_string 2
    mov  r3 rv  ; r3: abs_path
    ; sprintf(abs_path, "%s/%s", cwd, path);
    push r3
    push c.str_1
    push r1
    push a0
    call f.sprintf 4
    ; free(cwd);
    push r1
    call f.free 1
    ; return create_string(abs_path, total_len);
    push r3
    push r2
    call f.create_string 2
    ret
; }

; string get_file_name(File* path) {
f.get_file_name:
    ; a0: path
    ; return path->name;
    add  r0 a0 2 * size  ; size = sizeof(size_t), offset of name in File struct
    load rv r0
    ret
; }

; string get_file_extension(File* path) {
f.get_file_extension:
    ; a0: path
    ; return path->extension;
    add  r0 a0 size  ; size = sizeof(size_t), offset of extension in File struct
    load rv r0
    ret
; }

; string get_file_dir(File* path) {
f.get_file_dir:
    ; a0: path
    ; if (path->dirs == 0) {
    load r0 a0
    cmpu byte r0 0
    jmp  l.if_end5 ne
    ; return 0;
    mov  rv 0
    ret
    ; }
l.if_end5:
    ; size_t total_len = 0;
    mov  r0 0  ; r0: total_len
    ; size_t node_count = 0;
    mov  r1 0  ; r1: node_count
    ; StrNode* current = path->dirs;
    load r2 a0  ; r2: current
    ; while (current != 0) {
l.while_start_0:
    cmpu r2 0
    jmp  l.while_end_0 eq
    ; if (current->next != 0) {
    add  r3 r2 size  ; size = sizeof(size_t), offset of next in StrNode struct
    load r3 r3
    cmpu r3 0
    jmp  l.if_end_6 eq
    ; size_t dir_len = strlen(current->dir);
    load r3 r2
    push r3
    call f.strlen 1
    mov  r3 rv  ; r3: dir_len
    ; total_len += dir_len;
    add  r0 r0 r3
    ; node_count++;
    add  r1 r1 1
    ; }
l.if_end_6:
    ; current = current->next;
    add  r2 r2 size  ; size = sizeof(size_t), offset of next in StrNode struct
    load r2 r2
    ; }
l.while_end_0:
    ; if (node_count == 0) {
    cmpu r1 0
    jmp  l.if_end_7 ne
    ; return 0;
    mov  rv 0
    ret
    ; }
l.if_end_7:
    ; if (node_count > 1) {
    cmpu r1 1
    jmp  l.if_end_8 le
    ; total_len += node_count - 1;
    sub  r3 r1 1
    add  r0 r0 r3
    ; }
l.if_end_8:
    ; string dir_path = create_string("", total_len + 1);
    push c.str_0
    add  r3 r0 1
    push r3
    call f.create_string 2
    mov  r3 rv  ; r3: dir_path
    ; dir_path[0] = '\0';
    save byte r3 0  ; '\0' == 0
    ; current = path->dirs;
    load r2 a0
    ; bool first = true;
    mov  r4 1  ; r4: first
    ; while (current != 0) {
l.while_start_1:
    cmpu r2 0
    jmp  l.while_end_1 eq
    ; if (current->next != 0) {
    add  r5 r2 size  ; size = sizeof(size_t), offset of next in StrNode struct
    load r5 r5
    cmpu r5 0
    jmp  l.if_end_9 eq
    ; if (!first && strcmp(current->dir, "/") != 0) {
    cmpu r4 0
    jmp  l.if_end_10 ne
    load r5 r2
    push r5
    push c.str_2
    call f.strcmp 2
    cmps rv 0
    jmp  l.if_end_10 eq
    ; if (strlen(dir_path) > 0 && dir_path[strlen(dir_path) - 1] != '/') {
    push r3
    call f.strlen 1
    cmpu rv 0
    jmp  l.if_end_11 le
    sub  r5 rv 1
    add  r5 r3 r5
    load r5 r5
    cmpu byte r5 47  ; '/' == 47
    jmp  l.if_end_11 eq
    ; strcat(dir_path, "/");
    push r3
    push c.str_2
    call f.strcat 2
    ; }
l.if_end_11:
    ; }
l.if_end_10:
    ; strcat(dir_path, current->dir);
    push r3
    load r5 r2
    push r5
    call f.strcat 2
    ; first = false;
    mov  r4 0
    ; }
l.if_end_9:
    ; current = current->next;
    add  r2 r2 size  ; size = sizeof(size_t), offset of next in StrNode struct
    load r2 r2
    ; }
l.while_end_1:
    ; return create_string(dir_path, strlen(dir_path));
    push r3
    push r0
    call f.strlen 1
    push rv
    call f.create_string 2
    ret
; }

; string get_full_path(File* path) {
f.get_full_path:
    ; a0: path
    ; return path->path;
    add  r0 a0 3 * size  ; size = sizeof(size_t), offset of path in File struct
    load rv r0
    ret
; }

; void change_file_extension(File* file, const string new_extension) {
f.change_file_extension:
    ; a0: file
    ; a1: new_extension
    ; file->extension = new_extension;
    add  r0 a0 size  ; size = sizeof(size_t), offset of extension in File struct
    save r0 a1
    ; string dir = get_file_dir(file);
    push a0
    call f.get_file_dir 1
    mov  r0 rv  ; r0: dir
    ; string dir_cstr = dir != NULL ? dir : "";
    cmpu r0 0
    jmp  l.if_else_12 ne
    mov  r1 c.str_0  ; r1: dir_cstr
    jmp  l.if_end_12 always
l.if_else_12:
    mov  r1 r0  ; r1: dir_cstr
l.if_end_12:
    ; string ext_cstr = new_extension != NULL ? new_extension : "";
    cmpu a1 0
    jmp  l.if_else_13 ne
    mov  r2 c.str_0  ; r2: ext_cstr
    jmp  l.if_end_13 always
l.if_else_13:
    mov  r2 a1  ; r2: ext_cstr
l.if_end_13:
    ; size_t path_len = strlen(dir_cstr) + 1 + strlen(file->name);
    push r1
    call f.strlen 1
    add  r3 rv 1
    add  r4 a0 2 * size  ; size = sizeof(size_t), offset of name in File struct
    load r4 r4
    push r4
    call f.strlen 1
    add  r3 r3 rv  ; r3: path_len
    ; if (new_extension != 0) {
    cmpu a1 0
    jmp  l.if_end_14 eq
    ; path_len += strlen(ext_cstr);
    push r2
    call f.strlen 1
    add  r3 r3 rv
    ; }
l.if_end_14:
    ; string new_path = create_string("", path_len + 1);
    push c.str_0
    add  r4 r3 1
    push r4
    call f.create_string 2
    mov  r4 rv  ; r4: new_path
    ; if (dir != 0 && strlen(dir_cstr) > 0) {
    cmpu r0 0
    jmp  l.if_else_15 eq
    push r1
    call f.strlen 1
    cmpu rv 0
    jmp  l.if_else_15 le
    ; sprintf(new_path, "%s/%s", dir_cstr, file->name);
    push r4
    push c.str_1
    push r1
    add  r5 a0 2 * size  ; size = sizeof(size_t), offset of name in File struct
    load r5 r5
    push r5
    call f.sprintf 4
    jmp  l.if_end_15 always
    ; } else {
l.if_else_15:
    ; sprintf(new_path, "%s", file->name);
    push r4
    push c.str_3
    add  r5 a0 2 * size  ; size = sizeof(size_t), offset of name in File struct
    load r5 r5
    push r5
    call f.sprintf 2
    ; }
l.if_end_15:
    ; if (new_extension != 0) {
    cmpu a1 0
    jmp  l.if_end_16 eq
    ; strcat(new_path, new_extension);
    push r4
    push a1
    call f.strcat 2
    ; }
l.if_end_16:
    ; file->path = create_string(new_path, strlen(new_path));
    push r4
    push r0
    call f.strlen 1
    push rv
    call f.create_string 2
    add  r0 a0 3 * size  ; size = sizeof(size_t), offset of path in File struct
    save r0 rv
    mov  rv 0
    ret
; }

; void change_file_name(File* file, const string new_name) {
f.change_file_name:
    ; a0: file
    ; a1: new_name
    ; file->name = new_name;
    add  r0 a0 2 * size  ; size = sizeof(size_t), offset of name in File struct
    save r0 a1
    ; if (file->dirs != 0) {
    load r0 a0
    cmpu r0 0
    jmp  l.if_end_17 eq
    ; StrNode* current = file->dirs;
    load r0 a0  ; r0: current
    ; while (current != 0) {
l.while_start_2:
    cmpu r0 0
    jmp  l.while_end_2 eq
    ; if (current->next == 0) {
    add  r1 r0 size  ; size = sizeof(size_t), offset of next in StrNode struct
    load r1 r1
    cmpu r1 0
    jmp  l.if_end_18 ne
    ; string ext_cstr = file->extension != NULL ? file->extension : "";
    add  r1 a0 size  ; size = sizeof(size_t), offset of extension in File struct
    load r1 r1
    cmpu r1 0
    jmp  l.if_end_20 ne
    mov  r1 c.str_0  ; r1: ext_cstr
l.if_end_20:
    ; size_t full_name_len = strlen(new_name);
    push a1
    call f.strlen 1
    mov  r2 rv  ; r2: full_name_len
    ; if (file->extension != 0) {
    add  r3 a0 size  ; size = sizeof(size_t), offset of extension in File struct
    load r3 r3
    cmpu r3 0
    jmp  l.if_end_19 eq
    ; full_name_len += strlen(ext_cstr);
    push r1
    call f.strlen 1
    add  r2 r2 rv
    ; }
l.if_end_19:
    ; string full_name = create_string("", full_name_len + 1);
    ; sprintf(full_name, "%s%s", new_name, ext_cstr);
    ; current->dir = create_string(full_name, strlen(full_name));
    ; break;
    ; }
l.if_end_18:
    ; current = current->next;
    jmp  l.while_start_2 always
    ; }
l.while_end_2:
    ; }
l.if_end_17:
    ; string dir = get_file_dir(file);
    ; string dir_cstr = dir != NULL ? dir : "";
    ; string ext_cstr = file->extension != 0 ? (file->extension) : "";
    ; size_t path_len = strlen(dir_cstr) + 1 + strlen(new_name);
    ; if (file->extension != 0) {
    ; path_len += strlen(ext_cstr);
    ; }
    ; string new_path = create_string("", path_len + 1);
    ; if (dir != NULL && strlen(dir_cstr) > 0) {
    ; sprintf(new_path, "%s/%s%s", dir_cstr, new_name, ext_cstr);
    ; } else {
    ; sprintf(new_path, "%s%s", new_name, ext_cstr);
    ; }
    ; file->path = create_string(new_path, strlen(new_path));
; }
