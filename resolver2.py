import re

with open('src/file.c', 'r') as f:
    content = f.read()

content = content.replace("<<<<<<< Updated upstream\n=======\nstring get_file_extension(File* path) {\n    return path->extension;\n}\n\n>>>>>>> Stashed changes\n", "")
content = content.replace("<<<<<<< Updated upstream\n\n=======\n            \n>>>>>>> Stashed changes\n", "\n")
content = content.replace("<<<<<<< Updated upstream\n}\n\nstring get_file_name(File* path) {\n    if (path == NULL || path->name == NULL) return create_string(\"\", 0);\n    return create_string(path->name, strlen(path->name));\n=======\n>>>>>>> Stashed changes\n", "}\n\nstring get_file_name(File* path) {\n    if (path == NULL || path->name == NULL) return create_string(\"\", 0);\n    return create_string(path->name, strlen(path->name));\n")

with open('src/file.c', 'w') as f:
    f.write(content)
