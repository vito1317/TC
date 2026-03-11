import re

with open('src/file.c', 'r') as f:
    content = f.read()

# Pattern 1
# <<<<<<< Updated upstream
# =======
# string get_file_extension(File* path) {
#     return path->extension;
# }
#
# >>>>>>> Stashed changes
content = re.sub(r'<<<<<<< Updated upstream\n=======\nstring get_file_extension\(File\* path\) {\n    return path->extension;\n}\n\n>>>>>>> Stashed changes\n', '', content)

# Pattern 2
# <<<<<<< Updated upstream
#
# =======
#
# >>>>>>> Stashed changes
content = re.sub(r'<<<<<<< Updated upstream\n\n=======\n            \n>>>>>>> Stashed changes\n', '\n', content)

# Pattern 3
# <<<<<<< Updated upstream
# }
#
# string get_file_name(File* path) {
#     if (path == NULL || path->name == NULL) return create_string("", 0);
#     return create_string(path->name, strlen(path->name));
# =======
# >>>>>>> Stashed changes
content = re.sub(r'<<<<<<< Updated upstream\n}\n\nstring get_file_name\(File\* path\) {\n    if \(path == NULL \|\| path->name == NULL\) return create_string\("", 0\);\n    return create_string\(path->name, strlen\(path->name\)\);\n=======\n>>>>>>> Stashed changes\n', '}\n\nstring get_file_name(File* path) {\n    if (path == NULL || path->name == NULL) return create_string("", 0);\n    return create_string(path->name, strlen(path->name));\n', content)

with open('src/file.c', 'w') as f:
    f.write(content)
