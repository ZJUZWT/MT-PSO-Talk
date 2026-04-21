@echo off
REM ================================================================
REM  Build & Install the Android PSO Benchmark APK
REM ================================================================
REM
REM  Prerequisites:
REM    1. Android SDK installed (ANDROID_HOME or ANDROID_SDK_ROOT set)
REM    2. Android NDK installed (via SDK Manager)
REM    3. Java JDK 17+ installed (JAVA_HOME set)
REM    4. A connected Android device or running emulator
REM
REM  Usage:
REM    build_android.bat          - Build debug APK
REM    build_android.bat release  - Build release APK
REM    build_android.bat install  - Build and install on device
REM ================================================================

echo.
echo ================================================================
echo   PSO Shader Benchmark - Android Build
echo ================================================================
echo.

cd /d "%~dp0android"

REM ── Check for gradlew ─────────────────────────────────────────────
if not exist "gradlew.bat" (
    echo [INFO] Generating Gradle wrapper...
    if defined GRADLE_HOME (
        call "%GRADLE_HOME%\bin\gradle.bat" wrapper --gradle-version 8.2
    ) else (
        echo [ERROR] gradlew.bat not found and GRADLE_HOME not set.
        echo         Please install Gradle or create the wrapper manually:
        echo         Run: gradle wrapper --gradle-version 8.2
        echo         in the android/ directory.
        pause
        exit /b 1
    )
)

REM ── Copy SPIR-V shaders to assets ────────────────────────────────
echo [INFO] Copying SPIR-V shaders to assets...
if not exist "app\src\main\assets\shaders\spirv" mkdir "app\src\main\assets\shaders\spirv"
xcopy /Y "..\shaders\spirv\*.spv" "app\src\main\assets\shaders\spirv\" >nul 2>&1

REM ── Build ─────────────────────────────────────────────────────────
if /i "%1"=="release" (
    echo [INFO] Building Release APK...
    call gradlew.bat assembleRelease
) else if /i "%1"=="install" (
    echo [INFO] Building and installing Debug APK...
    call gradlew.bat installDebug
) else (
    echo [INFO] Building Debug APK...
    call gradlew.bat assembleDebug
)

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [ERROR] Build failed!
    pause
    exit /b 1
)

echo.
echo ================================================================
echo   Build successful!
echo.
if /i "%1"=="install" (
    echo   APK installed on device. Launch the app:
    echo   "PSO Shader Benchmark"
) else if /i "%1"=="release" (
    echo   APK location:
    echo   android\app\build\outputs\apk\release\app-release-unsigned.apk
) else (
    echo   APK location:
    echo   android\app\build\outputs\apk\debug\app-debug.apk
    echo.
    echo   To install on device, run:
    echo     build_android.bat install
    echo   Or:
    echo     adb install android\app\build\outputs\apk\debug\app-debug.apk
)
echo ================================================================
echo.
pause
