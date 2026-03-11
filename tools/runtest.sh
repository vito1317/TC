#!/bin/bash

# Build the project
bash ./tools/build.sh
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# Parse command line arguments
RUN_ALL=1
RUN_TEST1=0
RUN_TEST2=0
RUN_TEST3=0
CLEAR_SCREEN=0

if [ $# -eq 0 ]; then
    RUN_ALL=1
else
    RUN_ALL=0
    for arg in "$@"; do
        case $arg in
            -c)
                CLEAR_SCREEN=1
                ;;
            -1)
                RUN_TEST1=1
                ;;
            -2)
                RUN_TEST2=1
                ;;
            -3)
                RUN_TEST3=1
                ;;
            *)
                echo "Unknown option: $arg"
                echo "Usage: $0 [-c] [-1] [-2] [-3]"
                echo "  -c    Clear screen before running tests"
                echo "  -1    Run test1"
                echo "  -2    Run test2"
                echo "  -3    Run test3"
                echo "  (no options: run all tests)"
                exit 1
                ;;
        esac
    done
fi

if [ $CLEAR_SCREEN -eq 1 ]; then
    clear
fi

# Run tests
HAS_ERROR=0
echo ""
echo "========================================"
if [ $RUN_ALL -eq 1 ]; then
    RUN_TEST1=1
    RUN_TEST2=1
    RUN_TEST3=1
    echo "Running all tests..."
    echo "========================================"
fi

if [ $RUN_TEST1 -eq 1 ]; then
    echo ""
    echo "[Test 1]"
    ./build/program ./test/test1/test1.tc
    if [ $? -ne 0 ]; then
        echo "ERROR: Test 1 failed!"
        HAS_ERROR=1
    fi
fi

if [ $RUN_TEST2 -eq 1 ]; then
    echo ""
    echo "[Test 2]"
    ./build/program ./test/test2/test2.tc
    if [ $? -ne 0 ]; then
        echo "ERROR: Test 2 failed!"
        HAS_ERROR=1
    fi
fi

if [ $RUN_TEST3 -eq 1 ]; then
    echo ""
    echo "[Test 3]"
    ./build/program ./test/test3/test3.tc
    if [ $? -ne 0 ]; then
        echo "ERROR: Test 3 failed!"
        HAS_ERROR=1
    fi
fi

echo ""
echo "========================================"
if [ $HAS_ERROR -eq 1 ]; then
    echo "Tests completed with errors!"
    exit 1
else
    echo "Tests completed!"
fi
