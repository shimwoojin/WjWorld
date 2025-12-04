@echo off
REM WjWorld Advanced Client Launcher

set UE_EDITOR="C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor.exe"
set PROJECT="C:\UEProjects\WjWorld\WjWorld.uproject"

REM 서버 주소 (변경 가능)
set SERVER_IP=127.0.0.1

echo ================================
echo WjWorld Client
echo ================================
echo Server: %SERVER_IP%
echo ================================
echo.
echo 1. Start at Lobby (자동 로그인)
echo 2. Direct Connect to Server
echo.

choice /c 12 /n /m "Select option: "

if errorlevel 2 goto DIRECT_CONNECT
if errorlevel 1 goto LOBBY

:LOBBY
echo Starting at Lobby...
%UE_EDITOR% %PROJECT% -game -log ^
-windowed ^
-ResX=1280 -ResY=720 ^
-NOSTEAM
goto END

:DIRECT_CONNECT
echo Connecting to %SERVER_IP%...
%UE_EDITOR% %PROJECT% %SERVER_IP% -game -log ^
-windowed ^
-ResX=1280 -ResY=720 ^
-NOSTEAM
goto END

:END
pause
