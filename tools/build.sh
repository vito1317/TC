#!/bin/bash
# build.sh - collect all .c from src and .h from include, then run gcc

# --- Build Flags ---
# Set any of these to an empty string to disable the flag.
FLAG_DEBUG="-g"
FLAG_OPTIMIZE="-O2"
FLAG_WALL="-Wall"
FLAG_WEXTRA="-Wextra"
# --- End Build Flags ---

SRCDIR="src"
INCDIR="include"
OUTDIR="build"
OUTEXE="program"

# Check if source and include directories exist
if [ ! -d "$SRCDIR" ]; then
    echo "Source directory \"$SRCDIR\" not found."
    exit 1
fi
if [ ! -d "$INCDIR" ]; then
    echo "Include directory \"$INCDIR\" not found."
    exit 1
fi

# Find all .c files
SOURCES=$(find "$SRCDIR" -name "*.c")

# Check if any source files were found
if [ -z "$SOURCES" ]; then
    echo "No .c files found under \"$SRCDIR\"."
    exit 1
fi

# Create output directory if it doesn't exist
mkdir -p "$OUTDIR"

# Combine GCC flags, filtering out empty strings
GCC_FLAGS=""
for flag in "$FLAG_WALL" "$FLAG_WEXTRA" "$FLAG_OPTIMIZE" "$FLAG_DEBUG"; do
    if [ -n "$flag" ]; then
        GCC_FLAGS="$GCC_FLAGS $flag"
    fi
done

# Show the full command
echo "Build command: gcc$GCC_FLAGS -I\"$INCDIR\" $SOURCES -o \"$OUTDIR/$OUTEXE\""

# Actually compile
gcc $GCC_FLAGS -I"$INCDIR" $SOURCES -o "$OUTDIR/$OUTEXE"
if [ $? -ne 0 ]; then
    echo "Build failed."
    exit 1
fi

echo "Build succeeded: \"$OUTDIR/$OUTEXE\""
