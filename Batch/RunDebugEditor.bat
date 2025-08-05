@echo off
REM DebugGame Editor로 언리얼 프로젝트 열기
REM 프로젝트 경로와 엔진 경로를 수정하세요

REM === 설정 부분 (본인 환경에 맞게 수정) ===
set UNREAL_ENGINE_PATH=C:\Program Files\Epic Games\UE_5.6\Engine\Binaries\Win64\UnrealEditor-Win64-DebugGame.exe
set PROJECT_PATH=%~dp0..\WjWorld.uproject

REM === 실행 ===
echo Starting Unreal Engine in DebugGame Editor mode...
echo Project: %PROJECT_PATH%
echo Engine: %UNREAL_ENGINE_PATH%
echo.

REM 파일 존재 여부 먼저 확인
if not exist "%UNREAL_ENGINE_PATH%" (
    echo Error: Engine not found at %UNREAL_ENGINE_PATH%
    pause
    exit /b 1
)

if not exist "%PROJECT_PATH%" (
    echo Error: Project not found at %PROJECT_PATH%
    pause
    exit /b 1
)

REM 언리얼 엔진 실행 (백그라운드로 시작)
echo Launching Unreal Engine...
start "" "%UNREAL_ENGINE_PATH%" "%PROJECT_PATH%" -DebugGameEditor

REM 실행 후 잠시 대기하여 프로세스 시작 확인
timeout /t 3 /nobreak > nul

REM 언리얼 에디터 프로세스가 실행되었는지 확인
tasklist /fi "imagename eq UnrealEditor.exe" 2>nul | find /i "UnrealEditor.exe" >nul
if %ERRORLEVEL% equ 0 (
    echo Unreal Engine started successfully in DebugGame mode
    echo Window will close automatically...
    timeout /t 2 /nobreak > nul
) else (
    echo.
    echo Error: Failed to start Unreal Engine
    echo Please check:
    echo 1. Engine path is correct: %UNREAL_ENGINE_PATH%
    echo 2. Project path is correct: %PROJECT_PATH%
    echo 3. DebugGame Editor binaries are built
    echo.
    pause
)