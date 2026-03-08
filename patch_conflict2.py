with open("src/file.c", "r") as f:
    lines = f.readlines()

new_lines = []
skip = False
for line in lines:
    if line.startswith("<<<<<<< HEAD"):
        skip = False
        continue
    if line.startswith("======="):
        skip = True
        continue
    if line.startswith(">>>>>>> origin/main"):
        skip = False
        continue
    if not skip:
        new_lines.append(line)

with open("src/file.c", "w") as f:
    f.writelines(new_lines)
