#ifndef FILE_H
#define FILE_H

#include "lib.h"

typedef struct StrNode StrNode;
struct StrNode {
    string dir;
    StrNode* next;
};

typedef struct File {
    StrNode* dirs;
    string extension;
    string name;
    string path;
} File;

string get_cwd(void);
File* create_file(const string path);
string absolute_path(string path);
string get_file_name(File* path);
string get_file_extension(File* path);
string get_file_dir(File* path);
string get_full_path(File* path);
void change_file_extension(File* file, const string new_extension);
void change_file_name(File* file, const string new_name);
void normalize_path(File* file);

#endif  // FILE_H
