@echo off
REM WjWorld Advanced Server Launcher

set UE_EDITOR="C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJECT="C:\UEProjects\WjWorld\WjWorld.uproject"

REM 서버 설정
set MAP=/Game/Map/02-1_Lobby
set PORT=7777
set MAX_PLAYERS=8

echo ================================
echo WjWorld Dedicated Server
echo ================================
echo Map: %MAP%
echo Port: %PORT%
echo Max Players: %MAX_PLAYERS%
echo ================================
echo.

REM Listen Server로 시작
%UE_EDITOR% %PROJECT% %MAP%?listen -game -log ^
-port=%PORT% ^
-windowed ^
-ResX=1280 -ResY=720 ^
-NOSTEAM

pause
