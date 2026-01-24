@echo off
echo ================================
echo WjWorld Project Rebuild
echo ================================
echo.
echo This will clean and rebuild the project.
echo Close all instances of Unreal Editor and Visual Studio.
echo.
pause

echo.
echo Cleaning binaries...
if exist "%WJWORLD_ROOT%\Binaries" rmdir /s /q "%WJWORLD_ROOT%\Binaries"
if exist "%WJWORLD_ROOT%\Intermediate" rmdir /s /q "%WJWORLD_ROOT%\Intermediate"
if exist "%WJWORLD_ROOT%\Saved\Crashes" rmdir /s /q "%WJWORLD_ROOT%\Saved\Crashes"
if exist "%WJWORLD_ROOT%\Saved\Logs" rmdir /s /q "%WJWORLD_ROOT%\Saved\Logs"

echo.
echo Generating Visual Studio files...
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" ^
-projectfiles ^
-project="%WJWORLD_ROOT%\WjWorld.uproject" ^
-game -engine

echo.
echo ================================
echo Done!
echo ================================
echo.
echo Next steps:
echo 1. Open WjWorld.sln in Visual Studio
echo 2. Build -> Rebuild Solution
echo 3. Run editor
echo.
pause
