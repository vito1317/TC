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

static string build_path_from_dirs(StrNode* dirs_head, bool skip_last) {
    if (dirs_head == NULL || (skip_last && dirs_head->next == NULL)) return 0;

    // Calculate total length needed
    size_t total_len = 0;
    size_t node_count = 0;
    StrNode* current = dirs_head;
    while (current != NULL) {
        if (!skip_last || current->next != NULL) {
            size_t dir_len = strlen(current->dir);
            // Protect against extremely long path overflow
            if (SIZE_MAX - total_len < dir_len) return 0;
            total_len += dir_len;
            node_count++;
        }
        current = current->next;
    }

    if (node_count == 0) return 0;

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
    while (current != NULL) {
        if (!skip_last || current->next != NULL) {
            size_t len = strlen(current->dir);

            if (!first && strcmp(current->dir, "/") != 0) {
                // Add separator before non-root components
                if (ptr > built_path && *(ptr - 1) != '/' && remaining >= 1) {
                    *ptr++ = '/';
                    *ptr = '\0';
                    remaining--;
                }
            }

            if (remaining >= len) {
                memcpy(ptr, current->dir, len);
                ptr += len;
                *ptr = '\0';
                remaining -= len;
            }
            first = false;
        }
        current = current->next;
    }

    return create_string(built_path, strlen(built_path));
}

string get_file_dir(File* path) {
    if (path->dirs == NULL) return 0;
    return build_path_from_dirs(path->dirs, true);
}

string get_file_name(File* path) {
    if (path == NULL || path->name == NULL) return create_string("", 0);
    return create_string(path->name, strlen(path->name));
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
    string full_path = build_path_from_dirs(dirs_head, false);
    if (full_path != NULL) {
        file->path = create_string(full_path, strlen(full_path));
    } else {
        file->path = create_string("", 0);
    }
}