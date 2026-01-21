@echo off
REM WjWorld Standalone Client Launcher

set UE_EDITOR="C:\Program Files\Epic Games\UE_5.7\Engine\Binaries\Win64\UnrealEditor-Win64-DebugGame.exe"
set PROJECT="C:\UEProjects\WjWorld\WjWorld.uproject"

echo ================================
echo WjWorld Standalone Client
echo ================================
echo.
echo Starting client...
echo.

%UE_EDITOR% %PROJECT% -game -log -windowed -ResX=1280 -ResY=720

pause
