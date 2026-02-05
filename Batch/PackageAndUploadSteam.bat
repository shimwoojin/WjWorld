@echo off
setlocal

REM ========================================
REM WjWorld: Package (Development) + Steam Upload
REM ========================================

set PROJECT_ROOT=C:\UEProjects\WjWorld
set UE_ROOT=C:\Program Files\Epic Games\UE_5.7
set PACKAGE_OUTPUT=%PROJECT_ROOT%\DevelopmentPackage\Windows
set STEAM_CONTENT=%PROJECT_ROOT%\Steam\content

echo ========================================
echo  WjWorld Steam Build Pipeline
echo ========================================
echo.
echo  1. Package (Development, Win64)
echo  2. Copy to Steam\content
echo  3. Upload via SteamCMD
echo ========================================
echo.

REM ----------------------------------------
REM Step 1: Package Development Build
REM ----------------------------------------
echo [1/3] Packaging Development build...
echo.

call "%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" ^
BuildCookRun ^
-project="%PROJECT_ROOT%\WjWorld.uproject" ^
-platform=Win64 ^
-clientconfig=Development ^
-build -cook -stage -pak -package ^
-archive -archivedirectory="%PACKAGE_OUTPUT%" ^
-nodebuginfo

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [FAILED] Packaging failed. Aborting.
    pause
    exit /b 1
)

echo.
echo [1/3] Packaging complete.
echo.

REM ----------------------------------------
REM Step 2: Copy to Steam/content
REM ----------------------------------------
echo [2/3] Copying to Steam\content...
echo.

REM Clear old content
if exist "%STEAM_CONTENT%" (
    rmdir /S /Q "%STEAM_CONTENT%"
)
mkdir "%STEAM_CONTENT%"

REM Copy packaged build
xcopy "%PACKAGE_OUTPUT%\*" "%STEAM_CONTENT%\" /E /I /Q /Y

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [FAILED] Copy failed. Aborting.
    pause
    exit /b 1
)

REM Ensure steam_appid.txt exists
if not exist "%STEAM_CONTENT%\steam_appid.txt" (
    echo 4399350> "%STEAM_CONTENT%\steam_appid.txt"
    echo  - steam_appid.txt created.
)

echo [2/3] Copy complete.
echo.

REM ----------------------------------------
REM Step 3: Upload to Steam
REM ----------------------------------------
echo [3/3] Uploading to Steam...
echo.

call "%PROJECT_ROOT%\Steam\upload.bat"

echo.
echo ========================================
echo  Pipeline complete.
echo ========================================
endlocal
pause
