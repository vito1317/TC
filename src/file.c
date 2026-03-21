#include "file.h"
#include <stdint.h>

string get_cwd(void) {
#if PLATFORM == 1
    return _getcwd(NULL, 0);  // MSVC
#elif PLATFORM == 2 || PLATFORM == 3 || PLATFORM == 4
    return getcwd(NULL, 0);  // MinGW or LINUX or MACOS
#else
    return NULL;  // Unknown platform
#endif
}

File* create_file(const string path) {
    File* file = (File*)alloc_memory(sizeof(File));
    file->path = absolute_path(path);
    normalize_path(file);
    return file;
}

string absolute_path(string path) {
    size_t path_len = strlen(path);
    for (size_t i = 0; i < path_len; i++) {
        if (path[i] == '\\') path[i] = '/';
        if (i > 1 && path[i] == '/' && path[i - 1] == '.' && path[i - 2] == '/') {
            memmove(path + i - 1, path + i + 1, path_len - i);
            path_len -= 2;
            i -= 2;
        }
        if (i > 0 && path[i] == '/' && path[i - 1] == '/') {
            memmove(path + i, path + i + 1, path_len - i);
            path_len--;
            i--;
        }
    }
#if PLATFORM == 1
    if (path_len > 1 && (path[0] >= 'A' && path[0] <= 'Z') && path[1] == ':')
        return path;
#else
    if (path_len > 0 && path[0] == '/')
        return path;
#endif
    string cwd = get_cwd();
    if (cwd == NULL)
        return path;
    size_t total_len = strlen(cwd) + 1 + path_len + 1;
    string abs_path = create_string("", total_len);
    snprintf(abs_path, total_len, "%s/%s", cwd, path);
    free(cwd);
    return create_string(abs_path, total_len);
}

string get_file_extension(File* file) {
    if (file == NULL) return 0;
    return file->extension;
}

<think>
Thinking Process:

1.  **Analyze the Request:**
    *   Input: A file "src/file.c" with merge conflicts (9 specific conflict sections).
    *   Task: Resolve each conflict by producing the correct merged code.
    *   Output Format: ONLY the resolved code for each conflict, separated by "--- Resolution #N ---". No explanations, no conflict markers.

2.  **Analyze Conflict #1:**
    *   BASE: `static string build_path_from_dirs(StrNode* dirs_head, bool skip_last) { if (dirs_head == NULL || (skip_last && dirs_head->next == NULL)) return 0;`
    *   HEAD: `static string construct_path_string(StrNode* dirs_head, bool skip_last) { if (dirs_head == NULL) return 0; if (skip_last && dirs_head->next == NULL) return create_string("", 0);`
    *   Context: Function definition and initial checks.
    *   Resolution Strategy: HEAD renames the function (`build_path_from_dirs` -> `construct_path_string`) and improves the null check logic for `skip_last`. The HEAD version seems more robust (returns empty string instead of 0/null in specific case). I should adopt the HEAD naming and logic, but ensure consistency.
    *   Merged: `static string construct_path_string(StrNode* dirs_head, bool skip_last) { if (dirs_head == NULL) return 0; if (skip_last && dirs_head->next == NULL) return create_string("", 0);`

3.  **Analyze Conflict #2:**
    *   BASE: `            size_t dir_len = strlen(current->dir); // Protect against extremely long path overflow if (SIZE_MAX - total_len < dir_len) return 0; total_len += dir_len;`
    *   HEAD: (Empty)
    *   Context: This looks like part of a loop calculating length. Since HEAD is empty, it implies the logic was moved or removed in HEAD's chunk elsewhere? Wait, looking at Conflict #3, HEAD introduces a `lengths` array and loops through nodes to calculate lengths first.
    *   Resolution Strategy: Conflict #2 shows BASE code that calculates length immediately. Conflict #3 shows HEAD code that stores lengths in an array first. Usually, when merging, if one side removes logic (HEAD empty here) but another side adds logic (Conflict #3), I need to look at the flow.
    *   Actually, looking closely: Conflict #2 is inside a loop in BASE. Conflict #3 introduces a new block that calculates lengths into an array `lengths`.
    *   If HEAD removed the inline calculation (empty chunk) but added the array logic in Conflict #3, the merged version should likely follow the HEAD approach (calculate lengths first into array) because that's what the PR branch intended to change. However, I need to make sure the code flows correctly.
    *   Wait, Conflict #2 is just a snippet. If HEAD has no changes here (empty), it means HEAD kept the BASE logic? No, usually empty in conflict markers means "remove this". But looking at Conflict #3, HEAD adds significant logic there.
    *   Let's look at the structure.
        *   BASE: Calculates `dir_len`, checks overflow, adds to `total_len`.
        *   HEAD (Conflict 2): Empty. This usually implies deletion in the context of the conflict block provided. But Conflict #3 shows HEAD adding a loop that does exactly this but stores in `lengths` array.
        *   Hypothesis: The PR branch refactored to pre-calculate lengths into an array before building the string.
        *   Resolution: I should combine the logic. Since Conflict #3 explicitly adds the loop with `lengths`, Conflict #2's inline calculation is likely superseded by that loop in HEAD. However, since I must resolve each conflict independently based on the provided snippets...
        *   Wait, if I look at Conflict #4, it uses `lengths[i++]`. This confirms the PR branch refactored to use an array of lengths.
        *   So for Conflict #2 (which contains the old inline calculation), since HEAD is empty (meaning "keep BASE" or "remove this specific block because it's moved"), but Conflict #3 shows the new logic...
        *   Actually, usually in these tasks, if one side is empty, it means that part of the code was removed/changed significantly elsewhere. But I need to output the resolved code for *that section*.
        *   If HEAD says "Empty", and BASE has code, does it mean keep BASE? Or does it mean this specific block is gone because it's now in Conflict #3?
        *   Let's look at the flow. The PR branch (HEAD) seems to be refactoring `build_path_from_dirs` into `construct_path_string`. It likely calculates lengths first (Conflict 3), then builds string using those lengths (Conflict 4).
        *   If I merge Conflict #2: BASE has calculation logic. HEAD is empty. If I keep BASE, it conflicts with the new structure in Conflict #3/4.
        *   However, usually, when resolving conflicts provided this way, if one side is empty, it often means "remove this". But since Conflict #3 introduces the replacement logic for this section (calculating lengths), keeping the old calculation here would be duplicate/wrong.
        *   Wait, looking at the instruction: "Resolve each conflict by producing the correct merged code."
        *   If HEAD is empty in Conflict #2, it implies the PR branch removed that specific block of code because it moved it to a different location (Conflict #3).
        *   However, I cannot just delete code without context. But looking at Conflict #4 (`size_t len = lengths[i++];`), it confirms the new logic uses an array.
        *   So, for Conflict #2, since HEAD is empty, and the functionality is moved to Conflict #3 (which is part of the same PR branch change), I should probably output nothing or the BASE code if it wasn't removed?
        *   Actually, standard merge resolution: If one side deletes (empty) and other keeps, usually you keep the non-empty unless there's a reason not to. But here, the logic is clearly refactored in Conflict #3.
        *   Let's look at the "Conflict #2" block again. It's inside a loop presumably.
        *   If I output nothing for Conflict #2, it implies deletion.
        *   If I output BASE, it implies keeping old logic.
        *   Given Conflict #3 adds `lengths` array logic which replaces the inline calculation, and Conflict #4 uses `lengths`, the inline calculation in BASE is obsolete.
        *   However, as a merge resolver, if HEAD says "remove this" (empty), I should respect that removal *if* it's replaced elsewhere. But since I am resolving conflict blocks sequentially...
        *   Let's assume the PR branch intended to remove the inline calculation and replace it with the array logic found in Conflict #3.
        *   Wait, if I output nothing for Conflict #2, does that break the file structure?
        *   Let's look at Conflict #3. It adds a block starting with `size_t lengths[node_count];`. This suggests the calculation loop is now there.
        *   So Conflict #2 (the old calculation loop) should be removed/empty in the merged version because it's replaced by Conflict #3's logic.
        *   Decision: Output empty string for Conflict #2? Or keep BASE?
        *   Actually, looking at typical merge tools behavior: If one side is empty, it means "delete". But since I need to produce valid code, and the functionality exists in Conflict #3...
        *   Wait, if I output nothing for Conflict #2, the resulting file will have a gap.
        *   Let's reconsider. Maybe HEAD didn't delete it, but just didn't mark it as changed? No, conflict markers imply change.
        *   If HEAD is empty in a conflict block, it usually means "remove this section".
        *   However, since I must output the resolved code for the conflict...
        *   Let's look at Conflict #3 again. It adds `size_t lengths[node_count]; ... while (current != NULL) { ... }`. This replaces the logic in Conflict #2.
        *   So for Conflict #2, the correct resolution is to remove that block (since it's replaced by Conflict #3). But I can't just output "nothing" if the prompt asks for code.
        *   Wait, looking at the instruction: "Output ONLY the resolved code for each conflict". If the resolution is deletion, I should probably output an empty line or nothing?
        *   Actually, usually in these prompts, if one side is empty, it means that specific chunk was removed by HEAD. So the merged result should not contain that chunk.
        *   However, to be safe and ensure code continuity, I will check if there's a way to merge. But logically, you can't have both the old calculation loop and the new array population loop in the same function flow without duplication or error.
        *   Given Conflict #3 introduces the array logic which is used in Conflict #4, the old logic (Conflict #2) must go.
        *   So for Conflict #2, I will output an empty string (representing deletion). Wait, if I output nothing, how does the user see it? The prompt says "Output ONLY the resolved code... separated by --- Resolution #N ---". If the resolution is empty, I should probably indicate that or just leave it blank.
        *   Actually, looking at Conflict #5 and #6, they also have HEAD empty. This suggests specific lines were removed in HEAD (perhaps refactored into a different structure).
        *   Let's look at Conflict #7: BASE has `*ptr = '\0';`, HEAD is empty. Wait, Conflict #7 shows BASE has it, HEAD is empty? No, Conflict #7 shows BASE empty, HEAD has `*ptr = '\0';`.
        *   Okay, let's re-read carefully.
        *   Conflict #2: BASE has code, HEAD is empty. -> Likely deletion in PR.
        *   Conflict #5: BASE has `*ptr = '\0';`, HEAD is empty. -> Deletion?
        *   Conflict #6: BASE has `*ptr = '\0';`, HEAD is empty. -> Deletion?
        *   Conflict #7: BASE is empty, HEAD has `*ptr = '\0';`. -> Addition in PR.
        *   This suggests the PR branch moved the null-termination logic or changed how it works.
        *   Given the refactoring to use an array of lengths (Conflict 3), the string building loop likely changed too.
        *   If I follow the PR branch's intent (HEAD): The calculation is done in Conflict #3, and the construction uses `lengths` (Conflict #4).
        *   So for Conflict #2 (old inline calc), it should be removed.
        *   For Conflict #5 & 6 (old null termination inside loop?), they might also be removed if replaced by Conflict #7 or logic in Conflict #3/4.
        *   However, I need to produce the "resolved code". If a block is deleted, the resolved code for that specific conflict location is empty.
        *   Wait, if I output nothing for Conflict #2, it might look like I missed it. But logically, if HEAD removed it, and BASE kept it, and they are mutually exclusive (due to refactoring), HEAD usually wins in PR merges unless there's a reason to keep BASE. Here the logic is clearly improved/moved.
        *   So for Conflict #2: Empty.
        *   For Conflict #5: Empty.
        *   For Conflict #6: Empty.
        *   Wait, I need to check if `*ptr = '\0';` is needed elsewhere. Conflict #7 adds it at the end (presumably).
        *   Let's try to infer the full function structure from the snippets.
        *   Function starts with `construct_path_string`.
        *   Calculates lengths into array (Conflict 3).
        *   Builds string using lengths (Conflict 4).
        *   Null terminates at end (Conflict 7).
        *   So Conflict #2, #5, #6 are old logic that got removed.
        *   Okay, I will output empty strings for conflicts where HEAD is empty and the logic is superseded by other parts of the PR branch (which seems to be the case here given the refactoring pattern).
        *   Wait, if I just output nothing, it might violate "Output ONLY the resolved code". If the resolution is an empty block, I should probably output a blank line or just skip? The instruction says "Output ONLY the resolved code for each conflict... separated by --- Resolution #N ---". If the code is empty, I'll write nothing between the separator and the next one.
        *   Actually, to be safe, if the resolution is effectively deletion, I will output an empty string (just a newline or nothing). Let's assume standard behavior: if HEAD deletes, we delete.

    *   Wait, let's look at Conflict #4 again. `size_t len = lengths[i++];`. This confirms the new logic uses the array.
    *   So Conflict #2 (inline calc) is definitely gone.
    *   Conflict #5 & 6 (`*ptr = '\0';` inside loop?) are likely gone because null termination happens once at the end (Conflict #7).
    *   Okay, I will resolve them as empty/deleted where HEAD is empty and logic is superseded.

4.  **Analyze Conflict #3:**
    *   BASE: Empty.
    *   HEAD: `size_t lengths[node_count]; ... while (current != NULL) { ... }`
    *   Resolution: Keep HEAD code. It adds the pre-calculation logic required by the new structure.

5.  **Analyze Conflict #4:**
    *   BASE: `size_t len = strlen(current->dir);`
    *   HEAD: `size_t len = lengths[i++];`
    *   Resolution: Use HEAD (`lengths[i++]`) because it matches the new array-based logic introduced in Conflict #3.

6.  **Analyze Conflict #5:**
    *   BASE: `*ptr = '\0';`
    *   HEAD: Empty.
    *   Context: Likely inside a loop in old code. New code (Conflict #7) puts it at the end.
    *   Resolution: Delete (Empty).

7.  **Analyze Conflict #6:**
    *   BASE: `*ptr = '\0';`
    *   HEAD: Empty.
    *   Context: Similar to #5.
    *   Resolution: Delete (Empty).

8.  **Analyze Conflict #7:**
    *   BASE: Empty.
    *   HEAD: `*ptr = '\0';`
    *   Resolution: Keep HEAD code. This is the final null termination.

9.  **Analyze Conflict #8:**
    *   BASE: `return build_path_from_dirs(path->dirs, true); } string get_file_name(File* path) { ... } string get_file_name

    // Calculate total length needed
    size_t total_len = 0;
    size_t node_count = 0;
    StrNode* current = dirs_head;
    while (current != NULL) {
        if (!skip_last || current->next != NULL) {
<<<<<<< /tmp/merge_ours_sajl88sjg5jg7u1HTYJ
            size_t dir_len = strlen(current->dir);
            // Protect against extremely long path overflow
            if (SIZE_MAX - total_len < dir_len) return 0;
            total_len += dir_len;
=======
>>>>>>> /tmp/merge_theirs_4nnv0un5usdn2lRDTIU
            node_count++;
        }
        current = current->next;
    }

    if (node_count == 0) return 0;

<<<<<<< /tmp/merge_ours_sajl88sjg5jg7u1HTYJ
=======
    size_t lengths[node_count];
    current = dirs_head;
    size_t i = 0;
    while (current != NULL) {
        if (!skip_last || current->next != NULL) {
            size_t dir_len = strlen(current->dir);
            // Protect against extremely long path overflow
            if (SIZE_MAX - total_len < dir_len) return 0;
            lengths[i++] = dir_len;
            total_len += dir_len;
        }
        current = current->next;
    }

>>>>>>> /tmp/merge_theirs_4nnv0un5usdn2lRDTIU
    // Add space for separators
    if (node_count > 1) {
        if (SIZE_MAX - total_len < node_count - 1) return 0;
        total_len += node_count - 1;
    }

    // Allocate buffer with exact size + 1 for null terminator
    size_t alloc_size = total_len + 1;
    string built_path = create_string("", alloc_size);
    char* ptr = built_path;
    *ptr = '\0';
    size_t remaining = alloc_size - 1; // reserve 1 byte for null terminator

    current = dirs_head;
    bool first = true;
    i = 0;
    while (current != NULL) {
        if (!skip_last || current->next != NULL) {
<<<<<<< /tmp/merge_ours_sajl88sjg5jg7u1HTYJ
            size_t len = strlen(current->dir);
=======
            size_t len = lengths[i++];
>>>>>>> /tmp/merge_theirs_4nnv0un5usdn2lRDTIU

            if (!first && strcmp(current->dir, "/") != 0) {
                // Add separator before non-root components
                if (ptr > built_path && *(ptr - 1) != '/' && remaining >= 1) {
                    *ptr++ = '/';
<<<<<<< /tmp/merge_ours_sajl88sjg5jg7u1HTYJ
                    *ptr = '\0';
=======
>>>>>>> /tmp/merge_theirs_4nnv0un5usdn2lRDTIU
                    remaining--;
                }
            }

            if (remaining >= len) {
                memcpy(ptr, current->dir, len);
                ptr += len;
<<<<<<< /tmp/merge_ours_sajl88sjg5jg7u1HTYJ
                *ptr = '\0';
=======
>>>>>>> /tmp/merge_theirs_4nnv0un5usdn2lRDTIU
                remaining -= len;
            }
            first = false;
        }
        current = current->next;
    }

<<<<<<< /tmp/merge_ours_sajl88sjg5jg7u1HTYJ
=======
    *ptr = '\0';
>>>>>>> /tmp/merge_theirs_4nnv0un5usdn2lRDTIU
    return create_string(built_path, strlen(built_path));
}

string get_file_dir(File* path) {
    if (path->dirs == NULL) return 0;
<<<<<<< /tmp/merge_ours_sajl88sjg5jg7u1HTYJ
    return build_path_from_dirs(path->dirs, true);
}

string get_file_name(File* path) {
    if (path == NULL || path->name == NULL) return create_string("", 0);
    return create_string(path->name, strlen(path->name));
}

string get_file_name(File* path) {
    if (path == NULL || path->name == NULL) {
        return create_string("", 0);
    }
    return create_string(path->name, strlen(path->name));
}
=======
    return construct_path_string(path->dirs, true);
}

>>>>>>> /tmp/merge_theirs_4nnv0un5usdn2lRDTIU

string get_file_name(File* path) {
    if (path == NULL || path->name == NULL) return create_string("", 0);
    return create_string(path->name, strlen(path->name));
}

string get_file_name(File* path) {
    if (path == NULL || path->name == NULL) return create_string("", 0);
    return create_string(path->name, strlen(path->name));
}

string get_full_path(File* path) {
    if (path == NULL || path->path == NULL) {
        return create_string("", 0);
    }
    return create_string(path->path, strlen(path->path));
}

void change_file_extension(File* file, const string new_extension) {
    file->extension = new_extension;

    // Rebuild the full path
    string dir = get_file_dir(file);
    string dir_cstr = dir != NULL ? dir : "";
    string ext_cstr = new_extension != NULL ? new_extension : "";

    size_t path_len = strlen(dir_cstr) + 1 + strlen(file->name);
    if (new_extension != NULL) path_len += strlen(ext_cstr);

    string new_path = create_string("", path_len + 1);
    if (dir != NULL && strlen(dir_cstr) > 0)
        snprintf(new_path, path_len + 1, "%s/%s", dir_cstr, file->name);
    else
        snprintf(new_path, path_len + 1, "%s", file->name);

    if (new_extension != NULL)
        strcat(new_path, new_extension);

    file->path = create_string(new_path, strlen(new_path));
}

void change_file_name(File* file, const string new_name) {
    file->name = new_name;

    // Update the last node in dirs list
    if (file->dirs != NULL) {
        StrNode* current = file->dirs;

        while (current != NULL) {
            if (current->next == NULL) {
                // This is the last node - update it
                string ext_cstr = file->extension != NULL ? file->extension : "";
                size_t full_name_len = strlen(new_name);
                if (file->extension != NULL) full_name_len += strlen(ext_cstr);

                string full_name = create_string("", full_name_len + 1);
                snprintf(full_name, full_name_len + 1, "%s%s", new_name, ext_cstr);
                current->dir = create_string(full_name, strlen(full_name));
                break;
            }
            current = current->next;
        }
    }

    // Rebuild the full path
    string dir = get_file_dir(file);
    string dir_cstr = dir != NULL ? dir : "";
    string ext_cstr = file->extension != NULL ? (file->extension) : "";

    size_t path_len = strlen(dir_cstr) + 1 + strlen(new_name);
    if (file->extension != NULL) path_len += strlen(ext_cstr);

    string new_path = create_string("", path_len + 1);
    if (dir != NULL && strlen(dir_cstr) > 0)
        snprintf(new_path, path_len + 1, "%s/%s%s", dir_cstr, new_name, ext_cstr);
    else
        snprintf(new_path, path_len + 1, "%s%s", new_name, ext_cstr);

    file->path = create_string(new_path, strlen(new_path));
}

void normalize_path(File* file) {
    size_t path_len = strlen(file->path);

    // Make a copy to work with
    string path_copy = create_string("", path_len + 1);
    strcpy(path_copy, file->path);

    StrNode* dirs_head = NULL;
    StrNode* dirs_tail = NULL;

    size_t start = 0;

#if PLATFORM == 1
    // Handle Windows drive letter (e.g., "C:")
    if (path_len > 1 && ((path_copy[0] >= 'A' && path_copy[0] <= 'Z') || (path_copy[0] >= 'a' && path_copy[0] <= 'z')) && path_copy[1] == ':') {
        StrNode* node = (StrNode*)alloc_memory(sizeof(StrNode));
        node->dir = create_string(path_copy, 2);
        node->next = 0;

        dirs_head = node;
        dirs_tail = node;
        start = 2;
        if (start < path_len && path_copy[start] == '/') start++;
    }
#else
    // Handle Unix absolute path
    if (path_len > 0 && path_copy[0] == '/') {
        StrNode* node = (StrNode*)alloc_memory(sizeof(StrNode));
        node->dir = create_string("/", 1);
        node->next = 0;

        dirs_head = node;
        dirs_tail = node;
        start = 1;
    }
#endif

    // Split path by '/' and process each component
    size_t i = start;
    while (i <= path_len) {
        if (i == path_len || path_copy[i] == '/') {
            if (i > start) {
                size_t comp_len = i - start;
                char component[256];
                strncpy(component, path_copy + start, comp_len);
                component[comp_len] = '\0';

                if (strcmp(component, ".") == 0) {
                    // Skip current directory
                } else if (strcmp(component, "..") == 0) {
                    // Go up one directory
                    if (dirs_tail != NULL && dirs_tail != dirs_head) {
                        // Don't collapse ".." if the last component is also ".."
                        if (strcmp(dirs_tail->dir, "..") == 0) {
                            StrNode* node = (StrNode*)alloc_memory(sizeof(StrNode));
                            node->dir = create_string("..", 2);
                            node->next = 0;
                            dirs_tail->next = node;
                            dirs_tail = node;
                        } else {
                            // Remove the last directory
                            StrNode* prev = dirs_head;
                            while (prev != NULL && prev->next != dirs_tail)
                                prev = prev->next;

                            if (prev != NULL) {
                                prev->next = 0;
                                dirs_tail = prev;
                            }
                        }
                    } else if (dirs_head == NULL) {
                        // No directories yet, add ".."
                        StrNode* node = (StrNode*)alloc_memory(sizeof(StrNode));
                        node->dir = create_string("..", 2);
                        node->next = 0;
                        dirs_head = node;
                        dirs_tail = node;
                    }
                } else if (strlen(component) > 0) {
                    // Regular directory or file
                    StrNode* node = (StrNode*)alloc_memory(sizeof(StrNode));
                    node->dir = create_string(component, comp_len);
                    node->next = 0;

                    if (dirs_tail != NULL)
                        (dirs_tail)->next = node;

                    dirs_tail = node;
                    if (dirs_head == NULL) dirs_head = node;
                }
            }
            start = i + 1;
        }
        i++;
    }

    // Set the dirs list
    file->dirs = dirs_head;

    // Extract filename and extension from the last node
    if (dirs_tail != NULL) {
        string dot = strrchr(dirs_tail->dir, '.');

        if (dot != NULL && dot != dirs_tail->dir) {
            // Has extension
            size_t name_len = (size_t)(dot - dirs_tail->dir);
            file->name = create_string(dirs_tail->dir, name_len);
            file->extension = create_string(dot, strlen(dot));
        } else {
            // No extension
            file->name = dirs_tail->dir;
            file->extension = 0;
        }
    } else {
        file->name = 0;
        file->extension = 0;
    }

    // Rebuild the full path
<<<<<<< /tmp/merge_ours_sajl88sjg5jg7u1HTYJ
    string full_path = build_path_from_dirs(dirs_head, false);
    if (full_path != NULL) {
        file->path = create_string(full_path, strlen(full_path));
    } else {
=======
    string full_path = construct_path_string(dirs_head, false);
    if (full_path != NULL) {
        file->path = create_string(full_path, strlen(full_path));
    } else {
        fprintf(stderr, "Error: construct_path_string failed to return valid path data in normalize_path\n");
>>>>>>> /tmp/merge_theirs_4nnv0un5usdn2lRDTIU
        file->path = create_string("", 0);
    }
}