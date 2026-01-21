@echo off
REM WjWorld Standalone Server Launcher (Development Build)

set UE_EDITOR="C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJECT="C:\UEProjects\WjWorld\WjWorld.uproject"

echo ================================
echo WjWorld Standalone Server
echo Build: Development
echo ================================
echo.
echo Starting server...
echo.
echo Note: Make sure to build in Development configuration!
echo Visual Studio: Configuration -> Development Editor
echo.

%UE_EDITOR% %PROJECT% -game -log -windowed -ResX=1280 -ResY=720

pause
