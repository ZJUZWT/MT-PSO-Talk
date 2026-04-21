@echo off
REM ================================================================
REM  Build script for ShaderCompileTest (Windows)
REM  Prerequisites:
REM    - CMake 3.16+
REM    - Vulkan SDK installed (VULKAN_SDK env var set)
REM    - Visual Studio 2019/2022 or MinGW
REM    - Git (for FetchContent to download GLFW)
REM ================================================================

echo.
echo ================================================================
echo   Building ShaderCompileTest ...
echo ================================================================
echo.

REM Create build directory
if not exist build mkdir build

REM Configure
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] CMake configure failed!
    pause
    exit /b 1
)

REM Build
cmake --build build --config Release --parallel
if %ERRORLEVEL% NEQ 0 (
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo ================================================================
echo   Build successful!
echo   Run: build\Release\shader_compile_test.exe [iterations]
echo   Default iterations: 100
echo ================================================================
echo.
pause
