@echo off
echo ================================
echo Generate Visual Studio Project Files
echo ================================
echo.

"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\Build.bat" ^
-projectfiles ^
-project="%WJWORLD_ROOT%\WjWorld.uproject" ^
-game -engine

echo.
echo ================================
echo Done!
echo ================================
echo.
pause
