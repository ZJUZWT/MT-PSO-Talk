@echo off
setlocal enabledelayedexpansion
REM ================================================================
REM  run_all.bat - One-click PSO DCE Benchmark Suite
REM
REM  Automatically:
REM    1. Builds & runs the Desktop (Windows) benchmark
REM    2. Builds & installs the Android APK (if device connected)
REM    3. Triggers the Android benchmark via adb
REM    4. Pulls logcat results from the device
REM    5. Generates a combined report in results/
REM
REM  Usage:
REM    run_all.bat              (default: 500 draw calls)
REM    run_all.bat 1000         (custom draw call count)
REM    run_all.bat 500 desktop  (desktop only)
REM    run_all.bat 500 android  (android only)
REM ================================================================

set ITERATIONS=500
if not "%1"=="" set ITERATIONS=%1

set MODE=all
if /i "%2"=="desktop" set MODE=desktop
if /i "%2"=="android" set MODE=android

set SCRIPT_DIR=%~dp0
set RESULTS_DIR=%SCRIPT_DIR%results
REM Use wmic for locale-independent timestamp
for /f "tokens=2 delims==" %%I in ('wmic os get localdatetime /value 2^>nul') do set DT=%%I
set TIMESTAMP=%DT:~0,8%_%DT:~8,6%
set REPORT=%RESULTS_DIR%\benchmark_%TIMESTAMP%.txt

if not exist "%RESULTS_DIR%" mkdir "%RESULTS_DIR%"

echo.
echo ================================================================
echo   PSO Shader Compile Benchmark - Automated Test Suite
echo   Iterations: %ITERATIONS%    Mode: %MODE%
echo   Report:     %REPORT%
echo ================================================================
echo.

REM ── Initialize report ─────────────────────────────────────────────
(
echo ================================================================
echo   PSO Shader Compile Benchmark Report
echo   Date: %DT:~0,4%-%DT:~4,2%-%DT:~6,2% %DT:~8,2%:%DT:~10,2%:%DT:~12,2%
echo   Iterations: %ITERATIONS%
echo ================================================================
echo.
) > "%REPORT%"

REM ================================================================
REM  PHASE 1: Desktop Benchmark
REM ================================================================
if /i "%MODE%"=="android" goto skip_desktop

echo [1/4] Building Desktop benchmark...
echo.

REM ── Build desktop ─────────────────────────────────────────────────
if not exist build (
    cmake -S . -B build -DCMAKE_BUILD_TYPE=Release >nul 2>&1
    if !ERRORLEVEL! NEQ 0 (
        echo [ERROR] CMake configure failed!
        echo [DESKTOP] CMake configure failed >> "%REPORT%"
        goto skip_desktop
    )
)

cmake --build build --config Release --parallel >nul 2>&1
if !ERRORLEVEL! NEQ 0 (
    echo [ERROR] Desktop build failed!
    echo [DESKTOP] Build failed >> "%REPORT%"
    goto skip_desktop
)

echo [1/4] Desktop build OK.
echo.

REM ── Find executable ───────────────────────────────────────────────
set DESKTOP_EXE=
if exist "build\Release\shader_compile_test.exe" (
    set DESKTOP_EXE=build\Release\shader_compile_test.exe
) else if exist "build\shader_compile_test.exe" (
    set DESKTOP_EXE=build\shader_compile_test.exe
)

if "%DESKTOP_EXE%"=="" (
    echo [ERROR] Desktop executable not found!
    echo [DESKTOP] Executable not found >> "%REPORT%"
    goto skip_desktop
)

REM ── Run desktop benchmark ─────────────────────────────────────────
echo [2/4] Running Desktop benchmark (%ITERATIONS% draw calls)...
echo.

echo ================================================================ >> "%REPORT%"
echo  DESKTOP (Windows) RESULTS >> "%REPORT%"
echo ================================================================ >> "%REPORT%"
echo. >> "%REPORT%"

"%DESKTOP_EXE%" %ITERATIONS% >> "%REPORT%" 2>&1

echo [2/4] Desktop benchmark complete.
echo.

REM ── Also run a quick sweep of different iteration counts ──────────
echo [INFO] Running iteration sweep: 100, 500, 1000, 5000 ...
echo. >> "%REPORT%"
echo ================================================================ >> "%REPORT%"
echo  DESKTOP ITERATION SWEEP >> "%REPORT%"
echo ================================================================ >> "%REPORT%"

for %%N in (100 500 1000 5000) do (
    echo   Running %%N draw calls...
    echo. >> "%REPORT%"
    echo --- %%N draw calls --- >> "%REPORT%"
    "%DESKTOP_EXE%" %%N >> "%REPORT%" 2>&1
)

echo [INFO] Desktop sweep complete.
echo.

:skip_desktop

REM ================================================================
REM  PHASE 2: Android Benchmark
REM ================================================================
if /i "%MODE%"=="desktop" goto skip_android

echo [3/4] Checking Android device...

REM ── Check adb ─────────────────────────────────────────────────────
where adb >nul 2>&1
if !ERRORLEVEL! NEQ 0 (
    echo [WARN] adb not found in PATH. Skipping Android tests.
    echo [ANDROID] adb not found - skipped >> "%REPORT%"
    goto skip_android
)

REM ── Check device connected ────────────────────────────────────────
set DEVICE_COUNT=0
for /f %%i in ('adb devices ^| find /c "device"') do set DEVICE_COUNT=%%i
REM Subtract 1 for the header line "List of devices attached"
set /a DEVICE_COUNT=%DEVICE_COUNT%-1

if %DEVICE_COUNT% LEQ 0 (
    echo [WARN] No Android device connected. Skipping Android tests.
    echo [ANDROID] No device connected - skipped >> "%REPORT%"
    goto skip_android
)

echo [3/4] Device found. Getting device info...

REM ── Collect device info ───────────────────────────────────────────
echo. >> "%REPORT%"
echo ================================================================ >> "%REPORT%"
echo  ANDROID DEVICE INFO >> "%REPORT%"
echo ================================================================ >> "%REPORT%"
echo. >> "%REPORT%"

for /f "tokens=*" %%a in ('adb shell getprop ro.product.model 2^>nul') do (
    echo   Model: %%a
    echo   Model: %%a >> "%REPORT%"
)
for /f "tokens=*" %%a in ('adb shell getprop ro.product.manufacturer 2^>nul') do (
    echo   Manufacturer: %%a >> "%REPORT%"
)
for /f "tokens=*" %%a in ('adb shell getprop ro.hardware.chipname 2^>nul') do (
    echo   Chipset: %%a >> "%REPORT%"
)
for /f "tokens=*" %%a in ('adb shell getprop ro.board.platform 2^>nul') do (
    echo   Platform: %%a >> "%REPORT%"
)
for /f "tokens=*" %%a in ('adb shell getprop ro.build.version.sdk 2^>nul') do (
    echo   API Level: %%a >> "%REPORT%"
)
echo. >> "%REPORT%"

REM ── Build and install APK ─────────────────────────────────────────
echo [3/4] Building Android APK...

pushd "%SCRIPT_DIR%android"

REM Copy SPIR-V shaders
if not exist "app\src\main\assets\shaders\spirv" mkdir "app\src\main\assets\shaders\spirv"
xcopy /Y "..\shaders\spirv\*.spv" "app\src\main\assets\shaders\spirv\" >nul 2>&1

REM Build
call gradlew.bat assembleDebug --no-daemon >nul 2>&1
if !ERRORLEVEL! NEQ 0 (
    echo [ERROR] Android build failed!
    echo [ANDROID] Build failed >> "%REPORT%"
    popd
    goto skip_android
)

popd

echo [3/4] Installing APK...
adb install -r "%SCRIPT_DIR%android\app\build\outputs\apk\debug\app-debug.apk" >nul 2>&1
if !ERRORLEVEL! NEQ 0 (
    echo [ERROR] APK install failed!
    echo [ANDROID] Install failed >> "%REPORT%"
    goto skip_android
)

echo [3/4] APK installed.
echo.

REM ── Run Android benchmark ─────────────────────────────────────────
echo [4/4] Running Android benchmark (%ITERATIONS% draw calls)...
echo       Waiting for results from device...

echo. >> "%REPORT%"
echo ================================================================ >> "%REPORT%"
echo  ANDROID BENCHMARK RESULTS >> "%REPORT%"
echo ================================================================ >> "%REPORT%"
echo. >> "%REPORT%"

REM Clear logcat
adb logcat -c 2>nul

REM Launch the benchmark in auto-run mode
adb shell am start -n com.psobench.shadercompiletest/.BenchmarkActivity --ez autorun true --ei iterations %ITERATIONS% >nul 2>&1

REM Wait for the benchmark to complete (poll logcat for the conclusion marker)
echo [INFO] Polling logcat for results (timeout: 120s)...

set ANDROID_RESULT_FILE=%RESULTS_DIR%\android_logcat_%TIMESTAMP%.txt
set WAIT_COUNT=0
set FOUND=0

:poll_loop
if %WAIT_COUNT% GEQ 120 (
    echo [WARN] Timeout waiting for Android benchmark results.
    echo [ANDROID] Timeout after 120s >> "%REPORT%"
    goto android_done
)

REM Check if the benchmark has finished by looking for the conclusion marker
adb logcat -d -s PSO-Bench:I 2>nul | findstr /C:"================" >nul 2>&1
if !ERRORLEVEL! EQU 0 (
    REM Also check for actual result data (Speedup line)
    adb logcat -d -s PSO-Bench:I 2>nul | findstr /C:"Speedup" >nul 2>&1
    if !ERRORLEVEL! EQU 0 (
        set FOUND=1
        goto android_done
    )
)

timeout /t 2 /nobreak >nul
set /a WAIT_COUNT=%WAIT_COUNT%+2
echo   ... waiting (%WAIT_COUNT%s)
goto poll_loop

:android_done

if %FOUND% EQU 1 (
    echo [4/4] Android benchmark complete!
    echo.

    REM Dump all PSO-Bench logs
    adb logcat -d -s PSO-Bench:I PSO-Bench-VK:I PSO-Bench-GLES:I > "%ANDROID_RESULT_FILE%" 2>&1

    echo --- Android Logcat Output --- >> "%REPORT%"
    type "%ANDROID_RESULT_FILE%" >> "%REPORT%"
    echo. >> "%REPORT%"
) else (
    echo [WARN] Could not capture Android results.
    echo [ANDROID] Results not captured >> "%REPORT%"
)

REM ── Stop the app ──────────────────────────────────────────────────
adb shell am force-stop com.psobench.shadercompiletest >nul 2>&1

:skip_android

REM ================================================================
REM  PHASE 3: Summary
REM ================================================================
echo.
echo ================================================================
echo   All tests complete!
echo   Full report saved to:
echo     %REPORT%
echo ================================================================
echo.

REM ── Print a quick summary to console ──────────────────────────────
echo --- Quick Summary ---
echo.

REM Extract key numbers from the report
for /f "tokens=*" %%a in ('findstr /C:"GPU Time:" "%REPORT%"') do (
    echo   %%a
)
echo.
for /f "tokens=*" %%a in ('findstr /C:"Speedup" "%REPORT%"') do (
    echo   %%a
)
echo.

echo ================================================================
echo   Done. Press any key to exit.
echo ================================================================
pause >nul

endlocal
