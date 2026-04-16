@echo off
chcp 65001 >nul
REM ============================================================
REM 批量导出 utrace 文件的 Scopes CSV
REM 使用 UE 自带的 SummarizeTrace Commandlet
REM
REM 输出: 每个 utrace 同目录下生成 <basename>Scopes.csv
REM       例如: 20260406_213832_OpenGLScopes.csv
REM ============================================================

REM 配置路径
set UE_EDITOR_CMD=M:\WeigongGame\UnrealEngine\Engine\Binaries\Win64\UnrealEditor-Cmd.exe
set UPROJECT=M:\WeigongGame\WeigongGame\WeigongGame.uproject
set UTRACE_DIR=M:\MyWorkSpace\AIProjects\MT-PSO-Talk\Supplement\耗时Insight

REM 检查 UnrealEditor-Cmd.exe 是否存在
if not exist "%UE_EDITOR_CMD%" (
    echo [错误] 找不到 UnrealEditor-Cmd.exe: %UE_EDITOR_CMD%
    echo 请先编译 Editor 或修改路径
    pause
    exit /b 1
)

echo ============================================================
echo  批量导出 utrace -^> Scopes CSV
echo  UE Editor: %UE_EDITOR_CMD%
echo  Project:   %UPROJECT%
echo ============================================================

REM ---- 1/4: Nubia Z60 Ultra - OpenGL ----
echo.
echo [1/4] Nubia Z60 Ultra - OpenGL
echo      输入: %UTRACE_DIR%\Nubia Z60 Ultra\20260406_213832_OpenGL.utrace
"%UE_EDITOR_CMD%" "%UPROJECT%" -run=SummarizeTrace -inputfile="%UTRACE_DIR%\Nubia Z60 Ultra\20260406_213832_OpenGL.utrace" -unattended -nullrhi
if %ERRORLEVEL% EQU 0 (
    echo      [成功]
) else (
    echo      [警告] 返回码: %ERRORLEVEL%
)

REM ---- 2/4: Nubia Z60 Ultra - Vulkan ----
echo.
echo [2/4] Nubia Z60 Ultra - Vulkan
echo      输入: %UTRACE_DIR%\Nubia Z60 Ultra\20260406_214009_Vulkan.utrace
"%UE_EDITOR_CMD%" "%UPROJECT%" -run=SummarizeTrace -inputfile="%UTRACE_DIR%\Nubia Z60 Ultra\20260406_214009_Vulkan.utrace" -unattended -nullrhi
if %ERRORLEVEL% EQU 0 (
    echo      [成功]
) else (
    echo      [警告] 返回码: %ERRORLEVEL%
)

REM ---- 3/4: Pixel 7 - OpenGL ----
echo.
echo [3/4] Pixel 7 - OpenGL
echo      输入: %UTRACE_DIR%\Pixel 7\20260407_103207_OpenGL.utrace
"%UE_EDITOR_CMD%" "%UPROJECT%" -run=SummarizeTrace -inputfile="%UTRACE_DIR%\Pixel 7\20260407_103207_OpenGL.utrace" -unattended -nullrhi
if %ERRORLEVEL% EQU 0 (
    echo      [成功]
) else (
    echo      [警告] 返回码: %ERRORLEVEL%
)

REM ---- 4/4: Pixel 7 - Vulkan ----
echo.
echo [4/4] Pixel 7 - Vulkan
echo      输入: %UTRACE_DIR%\Pixel 7\20260407_103556_Vulkan.utrace
"%UE_EDITOR_CMD%" "%UPROJECT%" -run=SummarizeTrace -inputfile="%UTRACE_DIR%\Pixel 7\20260407_103556_Vulkan.utrace" -unattended -nullrhi
if %ERRORLEVEL% EQU 0 (
    echo      [成功]
) else (
    echo      [警告] 返回码: %ERRORLEVEL%
)

echo.
echo ============================================================
echo  导出完成！请检查以下 CSV 文件:
echo.
echo  %UTRACE_DIR%\Nubia Z60 Ultra\20260406_213832_OpenGLScopes.csv
echo  %UTRACE_DIR%\Nubia Z60 Ultra\20260406_214009_VulkanScopes.csv
echo  %UTRACE_DIR%\Pixel 7\20260407_103207_OpenGLScopes.csv
echo  %UTRACE_DIR%\Pixel 7\20260407_103556_VulkanScopes.csv
echo.
echo  接下来运行: python parse_pso_timing.py --csv-only
echo ============================================================
pause
 