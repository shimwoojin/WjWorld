========================================
WjWorld Steam Build 업로드 가이드
========================================

AppID: 4399350
DepotID: 4399351

폴더 구조
---------
Steam/
├── content/          <- 패키징된 게임 파일 복사 위치
│   ├── WjWorld.exe
│   ├── WjWorld/
│   └── Engine/
├── output/           <- 빌드 로그 (자동 생성)
├── scripts/
│   ├── app_build_4399350.vdf
│   └── depot_build_4399351.vdf
├── upload.bat        <- 업로드 실행 스크립트
└── README.txt

사용 방법
---------
1. UE에서 Windows 패키징 실행
   - File > Package Project > Windows (64-bit)
   - 출력 폴더 선택

2. 패키징된 파일을 content/ 폴더에 복사
   - WjWorld.exe와 모든 폴더를 복사

3. upload.bat 수정
   - STEAM_USER: Steam 계정명
   - STEAMCMD_PATH: SteamCMD 경로

4. upload.bat 실행
   - 2단계 인증 코드 입력 필요

5. Steamworks에서 빌드 확인
   - SteamPipe > Builds
   - Default 브랜치에 설정

SteamCMD 설치
-------------
1. Steamworks SDK 다운로드
   https://partner.steamgames.com/downloads/steamworks_sdk.zip

2. 압축 해제 후 tools/ContentBuilder/builder/steamcmd.exe 경로 확인

3. upload.bat의 STEAMCMD_PATH 수정

주의사항
--------
- Steam Guard 활성화 계정은 2단계 인증 필요
- 빌드 전용 계정 생성 권장 (보안)
- content 폴더는 .gitignore에 추가됨
