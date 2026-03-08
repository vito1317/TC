#include "file.h"

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
    sprintf(abs_path, "%s/%s", cwd, path);
    free(cwd);
    return create_string(abs_path, total_len);
}

string get_file_name(File* path) {
    return path->name;
}

string get_file_extension(File* path) {
    return path->extension;
}

string get_file_dir(File* path) {
    if (path->dirs == NULL) return 0;

    // Calculate total length needed
    size_t total_len = 0;
    size_t node_count = 0;
    StrNode* current = path->dirs;
    while (current != NULL) {
        if (current->next != NULL) {  // Not the last element (which is the filename)
            size_t dir_len = strlen(current->dir);
            total_len += dir_len;
            node_count++;
        }
        current = current->next;
    }

    if (node_count == 0) return 0;

    // Add space for separators (but not after root '/' or drive letter)
    if (node_count > 1)
        total_len += node_count - 1;

    // Build the directory path
    string dir_path = create_string("", total_len + 1);
    dir_path[0] = '\0';

    current = path->dirs;
    bool first = true;
    while (current != NULL) {
        if (current->next != NULL) {  // Not the last element
            if (!first && strcmp(current->dir, "/") != 0)
                // Add separator before non-root components
                if (strlen(dir_path) > 0 && dir_path[strlen(dir_path) - 1] != '/')
                    strcat(dir_path, "/");

            strcat(dir_path, current->dir);
            first = false;
        }
        current = current->next;
    }

    return create_string(dir_path, strlen(dir_path));
}

string get_full_path(File* path) {
    return path->path;
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
        sprintf(new_path, "%s/%s", dir_cstr, file->name);
    else
        sprintf(new_path, "%s", file->name);

    if (new_extension != NULL)
        strcat(new_path, new_extension);

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
    size_t full_path_len = 0;
    StrNode* current = dirs_head;
    size_t node_count = 0;

    while (current != NULL) {
        full_path_len += strlen(current->dir);
        node_count++;
        current = current->next;
    }

    // Add space for separators between components
    if (node_count > 1)
        full_path_len += (node_count - 1);

    string full_path = create_string("", full_path_len + 1);
    full_path[0] = '\0';

    current = dirs_head;
    bool is_first = true;
    while (current != NULL) {
        if (!is_first && strcmp(current->dir, "/") != 0) {
            // Add separator before non-root components
            if (strlen(full_path) > 0 && full_path[strlen(full_path) - 1] != '/') {
                strcat(full_path, "/");
            }
        }
        strcat(full_path, current->dir);
        is_first = false;
        current = current->next;
    }

    file->path = create_string(full_path, strlen(full_path));
}
