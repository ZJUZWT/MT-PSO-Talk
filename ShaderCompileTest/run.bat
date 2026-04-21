@echo off
REM ================================================================
REM  Run the ShaderCompileTest benchmark
REM ================================================================

set ITERATIONS=100
if not "%1"=="" set ITERATIONS=%1

echo.
echo Running PSO DCE Benchmark with %ITERATIONS% draw calls ...
echo.

if exist build\Release\shader_compile_test.exe (
    build\Release\shader_compile_test.exe %ITERATIONS%
) else if exist build\shader_compile_test.exe (
    build\shader_compile_test.exe %ITERATIONS%
) else (
    echo [ERROR] Executable not found! Run build.bat first.
    pause
    exit /b 1
)

echo.
pause
