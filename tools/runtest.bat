@echo off
setlocal enabledelayedexpansion

REM Build the project
call .\tools\build.bat
if errorlevel 1 (
    echo Build failed!
    exit /b 1
)

REM Parse command line arguments
set "RUN_ALL=1"
set "RUN_TEST1=0"
set "RUN_TEST2=0"
set "RUN_TEST3=0"
set "CLEAR_SCREEN=0"

if "%~1"=="" goto run_tests

set "RUN_ALL=0"
:parse_args
if "%~1"=="" goto run_tests
if "%~1"=="-c" set "CLEAR_SCREEN=1"
if "%~1"=="-1" set "RUN_TEST1=1"
if "%~1"=="-2" set "RUN_TEST2=1"
if "%~1"=="-3" set "RUN_TEST3=1"
shift
goto parse_args

:run_tests

if "%CLEAR_SCREEN%"=="1" cls

REM Run tests
set "HAS_ERROR=0"
echo.
echo ========================================

if "%RUN_ALL%"=="1" (
    set "RUN_TEST1=1"
    set "RUN_TEST2=1"
    set "RUN_TEST3=1"
    echo Running all tests...
    echo ========================================
)

if "%RUN_TEST1%"=="1" (
    echo.
    echo [Test 1]
    .\build\program.exe .\test\test1\test1.tc
    if errorlevel 1 (
        echo ERROR: Test 1 failed!
        set "HAS_ERROR=1"
    )
)

if "%RUN_TEST2%"=="1" (
    echo.
    echo [Test 2]
    .\build\program.exe .\test\test2\test2.tc
    if errorlevel 1 (
        echo ERROR: Test 2 failed!
        set "HAS_ERROR=1"
    )
)

if "%RUN_TEST3%"=="1" (
    echo.
    echo [Test 3]
    .\build\program.exe .\test\test3\test3.tc
    if errorlevel 1 (
        echo ERROR: Test 3 failed!
        set "HAS_ERROR=1"
    )
)

echo.
echo ========================================
if "%HAS_ERROR%"=="1" (
    echo Tests completed with errors!
    endlocal
    exit /b 1
) else (
    echo Tests completed!
    endlocal
)
