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
if exist "C:\UEProjects\WjWorld\Binaries" rmdir /s /q "C:\UEProjects\WjWorld\Binaries"
if exist "C:\UEProjects\WjWorld\Intermediate" rmdir /s /q "C:\UEProjects\WjWorld\Intermediate"
if exist "C:\UEProjects\WjWorld\Saved\Crashes" rmdir /s /q "C:\UEProjects\WjWorld\Saved\Crashes"
if exist "C:\UEProjects\WjWorld\Saved\Logs" rmdir /s /q "C:\UEProjects\WjWorld\Saved\Logs"

echo.
echo Generating Visual Studio files...
"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" ^
-projectfiles ^
-project="C:\UEProjects\WjWorld\WjWorld.uproject" ^
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
