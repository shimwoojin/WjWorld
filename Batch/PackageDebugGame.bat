@echo off
setlocal

REM ========================================
REM WjWorld: Package (DebugGame)
REM ========================================

set PROJECT_ROOT=C:\UEProjects\WjWorld
set UE_ROOT=C:\Program Files\Epic Games\UE_5.7
set PACKAGE_OUTPUT=%PROJECT_ROOT%\DebugGamePackage\Windows

echo ========================================
echo  WjWorld DebugGame Package
echo ========================================
echo.
echo  Output: %PACKAGE_OUTPUT%
echo ========================================
echo.

echo [1/1] Packaging DebugGame build...
echo.

call "%UE_ROOT%\Engine\Build\BatchFiles\RunUAT.bat" ^
BuildCookRun ^
-project="%PROJECT_ROOT%\WjWorld.uproject" ^
-platform=Win64 ^
-clientconfig=DebugGame ^
-build -cook -stage -pak -package ^
-archive -archivedirectory="%PACKAGE_OUTPUT%"

if %ERRORLEVEL% NEQ 0 (
    echo.
    echo [FAILED] Packaging failed.
    pause
    exit /b 1
)

echo.
echo ========================================
echo  Package complete.
echo  Output: %PACKAGE_OUTPUT%
echo ========================================
endlocal
pause
