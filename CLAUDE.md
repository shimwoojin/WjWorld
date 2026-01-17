# WjWorld 프로젝트 컨텍스트

## 프로젝트 개요
- **엔진**: Unreal Engine 5.7
- **언어**: C++
- **IDE**: Visual Studio 2022
- **목적**: 허브 공간, 미니게임, 멀티플레이어 기능을 갖춘 개인 학습 프로젝트

## 폴더 구조
```
Source/WjWorld/
├── Core/                           # 핵심 게임 로직
│   ├── Base/                       # 베이스 클래스들
│   │   ├── WjWorldGameModeBase
│   │   ├── WjWorldCharacterBase
│   │   ├── WjWorldPlayerControllerBase
│   │   ├── WjWorldGameStateBase
│   │   ├── WjWorldPlayerStateBase
│   │   └── WjWorldHUDBase
│   ├── Intro/                      # 인트로 화면
│   │   └── WjWorldGameModeIntro
│   ├── Login/                      # 로그인
│   │   └── WjWorldGameModeLogin
│   ├── Local/                      # 로컬 게임모드
│   │   ├── Lobby/                  # 로비/허브
│   │   │   ├── WjWorldGameModeLobby
│   │   │   ├── WjWorldCharacterLobby
│   │   │   ├── WjWorldPlayerControllerLobby
│   │   │   └── WjWorldHUDLobby
│   │   └── WaitingRoom/            # 대기실
│   │       ├── WjWorldGameModeWaitingRoom
│   │       ├── WjWorldCharacterWaitingRoom
│   │       ├── WjWorldPlayerControllerWaitingRoom
│   │       ├── WjWorldGameStateWaitingRoom
│   │       └── WjWorldHUDWaitingRoom
│   ├── Session/                    # 세션 관리
│   │   └── SessionManager
│   └── WjWorldGameInstance
├── GamePlay/                       # 게임플레이 시스템
│   ├── Interact/                   # 상호작용
│   │   └── InteractablePortal
│   └── Quest/                      # 퀘스트 시스템
│       ├── Quest
│       ├── QuestInstance
│       ├── QuestState
│       ├── QuestFactory
│       └── QuestSubsystem
├── Network/                        # 네트워크/패킷 관련
│   ├── PacketData
│   ├── PacketDataQuest
│   └── SessionTypes
└── UI/                             # UI 위젯들
    ├── WjWorldUserWidgetBase       # UI 베이스 클래스
    ├── Intro/IntroWindow
    ├── Login/LoginWindow
    ├── Lobby/LobbyHUDWidget
    ├── Session/
    │   ├── CreateRoomWindow
    │   ├── RoomListWindow
    │   └── RoomListEntryWidget
    ├── WaitingRoom/WaitingRoomHUDWidget
    └── Interact/InteractionWidget
```

## 주요 클래스 계층
```
GameMode: AWjWorldGameModeBase → Intro, Login, Lobby, WaitingRoom
Character: AWjWorldCharacterBase → Lobby, WaitingRoom
PlayerController: AWjWorldPlayerControllerBase → Lobby, WaitingRoom
HUD: AWjWorldHUDBase → Lobby, WaitingRoom
UI Widget: UWjWorldUserWidgetBase → IntroWindow, LoginWindow, LobbyHUDWidget, WaitingRoomHUDWidget, CreateRoomWindow, RoomListWindow, InteractionWidget
```

## 구현 완료 기능
- 인트로/로그인/로비/대기실 게임모드
- 세션 관리 (방 생성/참가)
- 포탈 상호작용 시스템
- 퀘스트 시스템 기본 구조
- 네트워크 패킷 구조

## 코딩 컨벤션
- 언리얼 엔진 코딩 표준 준수
- 클래스 접두사: `A` (Actor), `U` (UObject), `F` (구조체)
- 프로젝트 접두사: `WjWorld`
- 한글 주석 사용 가능

## 빌드 명령어
- Visual Studio에서 F5 (DebugGame Editor)
- `Batch/` 폴더의 배치 파일 활용

## 문서화
- Doxygen 사용 (`Doxyfile` 설정 완료)
- `docs/` 폴더에 생성된 문서 저장
