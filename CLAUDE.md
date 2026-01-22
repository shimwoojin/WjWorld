# WjWorld 프로젝트 컨텍스트

## 프로젝트 개요
- **엔진**: Unreal Engine 5.7
- **언어**: C++
- **IDE**: Visual Studio 2022
- **목적**: 허브 공간, 미니게임, 멀티플레이어 기능을 갖춘 개인 학습 프로젝트

## 폴더 구조
```
Source/WjWorld/
├── AbilitySystem/                     # Gameplay Ability System
│   ├── Abilities/                     # 어빌리티 클래스들
│   ├── AttributeSets/                 # 어트리뷰트 셋
│   └── WjWorldAbilitySystemComponent  # ASC 컴포넌트
├── Core/                              # 핵심 게임 로직
│   ├── Base/                          # 베이스 클래스들
│   │   ├── WjWorldGameModeBase
│   │   ├── WjWorldCharacterBase
│   │   ├── WjWorldPlayerControllerBase
│   │   ├── WjWorldGameStateBase
│   │   ├── WjWorldPlayerStateBase
│   │   └── WjWorldHUDBase
│   ├── Intro/                         # 인트로 화면
│   │   └── WjWorldGameModeIntro
│   ├── Login/                         # 로그인
│   │   └── WjWorldGameModeLogin
│   ├── Local/                         # 로컬 게임모드
│   │   ├── Lobby/                     # 로비/허브
│   │   │   ├── WjWorldGameModeLobby
│   │   │   ├── WjWorldCharacterLobby
│   │   │   ├── WjWorldPlayerControllerLobby
│   │   │   └── WjWorldHUDLobby
│   │   └── WaitingRoom/               # 대기실
│   │       ├── WjWorldGameModeWaitingRoom
│   │       ├── WjWorldCharacterWaitingRoom
│   │       ├── WjWorldPlayerControllerWaitingRoom
│   │       ├── WjWorldGameStateWaitingRoom
│   │       └── WjWorldHUDWaitingRoom
│   ├── Play/                          # 게임플레이 모드 (미니게임 공통)
│   │   ├── WjWorldGameModePlay        # 게임플레이 게임모드
│   │   ├── WjWorldGameStatePlay       # 게임플레이 게임스테이트
│   │   ├── WjWorldPlayerStatePlay     # 게임플레이 플레이어스테이트 (ASC 포함)
│   │   ├── WjWorldCharacterPlay       # 게임플레이 캐릭터
│   │   ├── WjWorldPlayerControllerPlay
│   │   └── WjWorldHUDPlay             # 게임플레이 HUD
│   ├── GameRule/                      # 미니게임 규칙 시스템
│   │   ├── WjWorldGameRuleBase        # 게임 규칙 베이스 클래스
│   │   └── WjWorldGameRuleApproachingWall  # Approaching Wall 규칙
│   ├── GameData/                      # 게임 데이터 컴포넌트
│   │   ├── WjWorldGameDataComponent   # 데이터 컴포넌트 베이스
│   │   ├── ApproachingWallGameDataComponent   # AW 게임 데이터
│   │   └── ApproachingWallPlayerDataComponent # AW 플레이어 데이터
│   ├── Components/                    # 게임플레이 헬퍼 컴포넌트
│   │   ├── WjWorldGameplaySceneComponent
│   │   └── WjWorldGameplayActorComponent
│   ├── Session/                       # 세션 관리
│   │   └── SessionManager
│   ├── WjWorldCoreTypes.h             # 코어 타입 정의 (EGamePhase 등)
│   └── WjWorldGameInstance
├── DataAsset/                         # 데이터 에셋
│   └── CharacterPlaySetupDataAsset    # 캐릭터 셋업 데이터
├── GamePlay/                          # 게임플레이 시스템
│   ├── Camera/                        # 카메라 시스템
│   ├── Interact/                      # 상호작용
│   │   └── InteractablePortal
│   ├── Quest/                         # 퀘스트 시스템
│   │   ├── Quest
│   │   ├── QuestInstance
│   │   ├── QuestState
│   │   ├── QuestFactory
│   │   └── QuestSubsystem
│   └── Wall/                          # Approaching Wall 게임플레이
│       ├── WjWorldBrickActor          # 벽돌 액터
│       ├── WjWorldBrickComponent      # 벽돌 컴포넌트
│       ├── WjWorldBrickMovement       # 벽돌 이동 로직
│       ├── WjWorldBrickSpawner        # 벽돌 스포너 (비동기)
│       ├── WjWorldWallManager         # 벽 이동 관리
│       └── WjWorldWallDescriptionDataAsset  # 벽 레이아웃 데이터
├── Network/                           # 네트워크/패킷 관련
│   ├── PacketData
│   ├── PacketDataQuest
│   └── SessionTypes
└── UI/                                # UI 위젯들
    ├── WjWorldUserWidgetBase          # UI 베이스 클래스
    ├── Intro/IntroWindow
    ├── Login/LoginWindow
    ├── Lobby/LobbyHUDWidget
    ├── Session/
    │   ├── CreateRoomWindow
    │   ├── RoomListWindow
    │   └── RoomListEntryWidget
    ├── WaitingRoom/WaitingRoomHUDWidget
    ├── Interact/InteractionWidget
    └── HUD/
        └── GameplayGlobalHUDWidget    # 게임플레이 글로벌 HUD
```

## 주요 클래스 계층
```
GameMode: AWjWorldGameModeBase → Intro, Login, Lobby, WaitingRoom, Play
Character: AWjWorldCharacterBase → Lobby, WaitingRoom, Play
PlayerController: AWjWorldPlayerControllerBase → Lobby, WaitingRoom, Play
GameState: AWjWorldGameStateBase → WaitingRoom, Play
PlayerState: AWjWorldPlayerStateBase → Play (+ IAbilitySystemInterface)
HUD: AWjWorldHUDBase → Lobby, WaitingRoom, Play
UI Widget: UWjWorldUserWidgetBase → 각종 HUD 및 윈도우 위젯
GameRule: UWjWorldGameRuleBase → ApproachingWall (미니게임 규칙)
GameData: UWjWorldGameDataComponent → ApproachingWall 전용 데이터
```

## 핵심 시스템

### GameRule 시스템
미니게임을 정의하기 위한 규칙 시스템. `UWjWorldGameRuleBase`를 상속받아 각 미니게임의 규칙을 구현.
- **라이프사이클**: `Initialize()` → `OnGameReady()` → `OnGameStart()` → `OnGameEndPredict()` → `OnGameEnd()`
- **플레이어 이벤트**: `OnPlayerJoined()`, `OnPlayerLeft()`
- **승리 조건**: `CheckWinCondition()`, `GetWinner()`
- **Tickable**: `FTickableGameObject` 상속으로 프레임별 업데이트

### GameData 컴포넌트 시스템
게임/플레이어별 데이터를 관리하는 컴포넌트 시스템. GameplayTag 기반 타입 세이프 데이터 저장.
- `GameStatePlay`에 게임 전체 데이터 (예: 웨이브 타이밍)
- `PlayerStatePlay`에 플레이어별 데이터 (예: 점수, 상태)
- 리플리케이션 지원

### Approaching Wall 미니게임
첫 번째 미니게임. 벽이 점진적으로 다가오며 플레이어들이 안전 구역으로 이동해야 하는 PvP 게임.
- **BrickSpawner**: 데이터 에셋 기반 비동기 벽돌 스폰 (8개/틱)
- **BrickMovement**: 개별 벽돌 이동 로직 (경로 탐색)
- **WallManager**: 벽 이동 진행 관리 (레벨별 속도 조절)
- **레벨 시스템**: 12초마다 레벨업, 이동 시간 5초→1초 (10레벨)
- **안전 구역**: Flood Fill 알고리즘으로 축소

## 구현 완료 기능
- 인트로/로그인/로비/대기실 게임모드
- 세션 관리 (방 생성/참가)
- 포탈 상호작용 시스템
- 퀘스트 시스템 기본 구조
- 네트워크 패킷 구조
- **게임플레이 모드 프레임워크** (Play 클래스 세트)
- **GameRule 시스템** (미니게임 규칙 정의)
- **GameData 컴포넌트 시스템** (게임/플레이어 데이터)
- **Approaching Wall 기본 구조** (벽돌 스폰, 이동, 레벨 시스템)
- **게임플레이 HUD** (카운트다운, 결과 표시)
- **Ability System Component 통합**

## 진행 중 / 미구현
- Approaching Wall 승리 조건 (생존 판정)
- 플레이어 어빌리티 (이동, 공격 등)
- 플레이어 사망/제거 로직
- 게임 결과 처리 및 대기실 복귀

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

## 게임 플로우
```
게임 시작 → 인트로 → 로그인 → 로비 → 방 생성/참가 → 대기실
    ↓
GameModePlay 진입 (미니게임)
    ↓
GameRule 초기화 → OnGameReady → 카운트다운 → OnGameStart
    ↓
게임 진행 (Tick) → 승리 조건 체크 → OnGameEndPredict → OnGameEnd
    ↓
결과 표시 → 대기실 복귀
```

## 주요 데이터 흐름
```
WallDescriptionDataAsset (벽 레이아웃 정의)
    ↓
BrickSpawner (비동기 스폰)
    ↓
BrickComponent + BrickMovement (개별 벽돌)
    ↓
WallManager (전체 벽 이동 제어)
    ↓
GameRuleApproachingWall (게임 로직, 레벨업, 안전 구역)
```
