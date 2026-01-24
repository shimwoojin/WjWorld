@echo off
echo ================================
echo Package WjWorld (DebugGame)
echo ================================
echo.

"C:\Program Files\Epic Games\UE_5.7\Engine\Build\BatchFiles\RunUAT.bat" ^
BuildCookRun ^
-project="%WJWORLD_ROOT%\WjWorld.uproject" ^
-platform=Win64 ^
-clientconfig=DebugGame ^
-cook -stage -pak -package ^
-archive -archivedirectory="%WJWORLD_ROOT%\Windows"

echo.
echo ================================
echo Done!
echo ================================
echo.
echo Package output: %WJWORLD_ROOT%\Windows
echo.
pause
