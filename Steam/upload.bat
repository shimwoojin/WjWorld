@echo off
setlocal

REM ========================================
REM WjWorld Steam Build Upload Script
REM ========================================

REM Steam 계정 설정 (보안상 직접 입력 권장)
set STEAM_USER=swj627

REM SteamCMD 경로 (Steamworks SDK 설치 위치에 맞게 수정)
set STEAMCMD_PATH=C:\SteamworksSDK\tools\ContentBuilder\builder\steamcmd.exe

REM 스크립트 경로
set SCRIPT_PATH=%~dp0scripts\app_build_4399350.vdf

echo ========================================
echo WjWorld Steam Build Uploader
echo ========================================
echo.
echo AppID: 4399350
echo DepotID: 4399351
echo.

REM content 폴더 확인
if not exist "%~dp0content\WjWorld.exe" (
    echo [ERROR] content 폴더에 패키징된 게임이 없습니다.
    echo         UE에서 패키징 후 content 폴더에 복사하세요.
    echo         경로: %~dp0content\
    pause
    exit /b 1
)

echo Steam 로그인 중...
echo (2단계 인증 코드가 필요할 수 있습니다)
echo.

"%STEAMCMD_PATH%" +login %STEAM_USER% +run_app_build "%SCRIPT_PATH%" +quit

if %ERRORLEVEL% EQU 0 (
    echo.
    echo ========================================
    echo [SUCCESS] 빌드 업로드 완료!
    echo Steamworks에서 빌드를 확인하세요.
    echo ========================================
) else (
    echo.
    echo [ERROR] 업로드 실패. 로그를 확인하세요.
    echo 로그 위치: %~dp0output\
)

pause
