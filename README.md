# WjWorld

언리얼 엔진 5.6으로 개발하는 개인 C++ 프로젝트

## 프로젝트 개요

허브 공간, 미니게임, 멀티플레이어 기능을 갖춘 개인 학습 프로젝트입니다.

### 개발 목표
1. **허브 공간** - 플레이어가 컨텐츠로 진입할 수 있는 로컬 공간
2. **미니게임** - 다양한 장르의 미니게임 구현
3. **멀티플레이어** - 기본적인 네트워킹 기능 구현
4. **융합 컨텐츠** - 로컬과 멀티플레이어 요소 결합

## 개발 환경

- **엔진**: Unreal Engine 5.7
- **언어**: C++
- **IDE**: Visual Studio 2022
- **버전 관리**: Git
- **문서화**: Doxygen

## 소스 구조

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
    ├── Intro/
    │   └── IntroWindow
    ├── Login/
    │   └── LoginWindow
    ├── Lobby/
    │   └── LobbyHUDWidget
    ├── Session/
    │   ├── CreateRoomWindow
    │   ├── RoomListWindow
    │   └── RoomListEntryWidget
    ├── WaitingRoom/
    │   └── WaitingRoomHUDWidget
    └── Interact/
        └── InteractionWidget
```

## 주요 클래스 계층

### GameMode
```
AWjWorldGameModeBase
├── AWjWorldGameModeIntro          # 인트로 화면
├── AWjWorldGameModeLogin          # 로그인
├── AWjWorldGameModeLobby          # 로비/허브
└── AWjWorldGameModeWaitingRoom    # 대기실
```

### Character
```
AWjWorldCharacterBase
├── AWjWorldCharacterLobby
└── AWjWorldCharacterWaitingRoom
```

### PlayerController
```
AWjWorldPlayerControllerBase
├── AWjWorldPlayerControllerLobby
└── AWjWorldPlayerControllerWaitingRoom
```

### UI Widget
```
UWjWorldUserWidgetBase
├── UIntroWindow
├── ULoginWindow
├── ULobbyHUDWidget
├── UWaitingRoomHUDWidget
├── UCreateRoomWindow
├── URoomListWindow
└── UInteractionWidget
```

## 빌드 방법

### 필수 요구사항
- Visual Studio 2022 (C++ 개발 도구 포함)
- Unreal Engine 5.7
- Windows 10/11 SDK

### 프로젝트 설정
```bash
git clone https://github.com/your-repo/WjWorld.git
cd WjWorld
```
`.uproject` 파일 우클릭 → "Generate Visual Studio project files"

### 빌드
- Visual Studio에서 솔루션 열기
- Configuration: `DebugGame Editor` 또는 `Development Editor`
- F5로 빌드 및 실행

## 게임 플로우

```
게임 시작
    ↓
인트로 (동영상 재생)
    ↓
로그인
    ↓
로비 (허브)
    ↓
방 생성/참가 → 대기실
    ↓
게임 시작 → 컨텐츠 플레이
    ↓
결과 → 로비 복귀
```

## 개발 진행 상황

### 완료
- [x] 프로젝트 기본 구조 설계
- [x] GameMode/Character/PlayerController 클래스 계층 구현
- [x] 인트로 시스템
- [x] 로그인 시스템
- [x] 로비 시스템
- [x] 대기실 시스템
- [x] 세션 관리 (방 생성/참가)
- [x] 포탈 상호작용
- [x] 퀘스트 시스템 기본 구조
- [x] 네트워크 패킷 구조

### 진행 중
- [ ] 미니게임 구현
- [ ] 멀티플레이어 동기화

## 문서화

Doxygen으로 생성된 문서는 `docs/` 폴더에서 확인할 수 있습니다.

## 라이선스

개인 학습 프로젝트

---

**시작일**: 2025.08.05
