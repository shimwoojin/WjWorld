# WjWorld 프로젝트 컨텍스트

## 프로젝트 개요
- **엔진**: Unreal Engine 5.7
- **언어**: C++
- **IDE**: Visual Studio 2022
- **목적**: 허브 공간, 미니게임, 멀티플레이어 기능을 갖춘 개인 학습 프로젝트
- **배포**: Steam (무료 출시, 코스메틱 유료 판매)

## 폴더 구조
```
Source/WjWorld/
├── AbilitySystem/                     # Gameplay Ability System
│   ├── Abilities/                     # 어빌리티 클래스들
│   │   ├── WjWorldGameplayAbilityBase # 어빌리티 베이스 (UI 메타, 충전 시스템 인터페이스)
│   │   ├── GA_NormalAttack            # 4방향 벽돌 공격
│   │   ├── GA_SpawnBrick              # 충전 기반 벽돌 배치 (Preview + Confirm/Cancel)
│   │   └── GA_LiftBrick              # 벽돌 이동/재배치 (Preview + Confirm/Cancel)
│   ├── AttributeSets/                 # 어트리뷰트 셋
│   │   └── WjWorldCharacterAttributeSet  # HP, MaxSpawnBrickCharges, SpawnBrickCharges 등
│   ├── Effects/                       # GameplayEffect 파일들
│   │   ├── GE_AbilityCooldown         # 어빌리티 쿨다운
│   │   └── GE_SpawnBrickChargeCost    # SpawnBrick 충전 비용
│   └── WjWorldAbilitySystemComponent  # ASC 컴포넌트
├── Core/                              # 핵심 게임 로직
│   ├── Base/                          # 베이스 클래스들
│   │   ├── WjWorldGameModeBase
│   │   ├── WjWorldCharacterBase
│   │   ├── WjWorldPlayerControllerBase
│   │   ├── WjWorldGameStateBase
│   │   ├── WjWorldPlayerStateBase     # FCosmeticLoadout 리플리케이션 (모든 모드에서 사용)
│   │   └── WjWorldHUDBase
│   ├── Intro/                         # 인트로 화면
│   │   └── WjWorldGameModeIntro
│   ├── Login/                         # 로그인
│   │   └── WjWorldGameModeLogin       # 타이머 기반 로그인 대기 시스템
│   ├── Local/                         # 로컬 게임모드
│   │   ├── Lobby/                     # 로비/허브
│   │   │   ├── WjWorldGameModeLobby
│   │   │   ├── WjWorldGameStateLobby  # 배치 오브젝트 리플리케이션
│   │   │   ├── WjWorldCharacterLobby
│   │   │   ├── WjWorldPlayerControllerLobby  # PlacementComponent 보유
│   │   │   └── WjWorldHUDLobby
│   │   └── WaitingRoom/               # 대기실 (Lobby 맵 + GameMode 오버라이드)
│   │       ├── WjWorldGameModeWaitingRoom
│   │       ├── WjWorldCharacterWaitingRoom
│   │       ├── WjWorldPlayerControllerWaitingRoom
│   │       ├── WjWorldGameStateWaitingRoom  # GameStateLobby 상속
│   │       └── WjWorldHUDWaitingRoom
│   ├── Play/                          # 게임플레이 모드 (미니게임 공통)
│   │   ├── WjWorldGameModePlay        # 게임플레이 게임모드
│   │   ├── WjWorldGameStatePlay       # 게임플레이 게임스테이트 (미니게임 스탯 자동 기록)
│   │   ├── WjWorldPlayerStatePlay     # 게임플레이 플레이어스테이트 (ASC)
│   │   ├── WjWorldCharacterPlay       # 게임플레이 캐릭터 (CosmeticComponent + 제거 상태)
│   │   ├── WjWorldPlayerControllerPlay
│   │   └── WjWorldHUDPlay             # 게임플레이 HUD (GameRule별 HUD 위젯 매핑)
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
├── Cosmetic/                          # 코스메틱 및 구매 시스템
│   ├── WjWorldCosmeticTypes           # 코스메틱 타입 (ECosmeticSlot, FCosmeticSlotEntry, FCosmeticLoadout 등)
│   ├── WjWorldCosmeticComponent       # 캐릭터 코스메틱 비주얼 컴포넌트 (비동기 에셋 로드)
│   ├── WjWorldCosmeticSubsystem       # 인벤토리/로드아웃 관리 (GameInstanceSubsystem)
│   ├── WjWorldCosmeticDataAsset       # 코스메틱 아이템 카탈로그 (ItemId ↔ SteamItemDefId)
│   └── WjWorldPurchaseSubsystem       # Steam 마이크로트랜잭션 구매 (GameInstanceSubsystem)
├── Stats/                             # 플레이어 스탯 시스템
│   ├── WjWorldStatsSubsystem          # Steam User Stats 래핑 (GameInstanceSubsystem)
│   └── WjWorldStatTypes               # 스탯 타입 정의 (FMinigameStatEntry, FMinigameStatDescriptor)
├── Setting/                           # 개발자 설정
│   └── WjWorldDeveloperSettings       # BP 설정용 (카탈로그 참조, TileActorClass 등)
├── DataAsset/                         # 데이터 에셋
│   ├── CharacterPlaySetupDataAsset    # 캐릭터 셋업 데이터
│   ├── WjWorldMinigameDataAsset       # 미니게임 카탈로그 (GameModeId → GameRuleClass)
│   └── WjWorldPlaceableObjectDataAsset  # 배치 오브젝트 카탈로그
├── Save/                              # 저장 시스템
│   └── WjWorldLayoutSaveGame          # 로비 배치 레이아웃 저장
├── GamePlay/                          # 게임플레이 시스템
│   ├── Camera/                        # 카메라 시스템
│   ├── Interact/                      # 상호작용
│   │   └── InteractablePortal
│   ├── Placement/                     # 로비 배치 시스템
│   │   ├── WjWorldPlacementComponent  # 배치 핵심 로직 (PC에 부착)
│   │   ├── WjWorldPlacementPreviewActor  # 배치 프리뷰 액터
│   │   └── WjWorldPlacedObjectActor   # 배치된 오브젝트 액터
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
│       ├── WjWorldBrickPreviewActor   # 어빌리티 배치 프리뷰 (유효/무효 색상)
│       ├── WjWorldTileActor           # 안전 구역 타일 (폭탄 신호, 색상 전환)
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
    ├── Lobby/
    │   ├── LobbyHUDWidget
    │   └── PlacementHUDWidget         # 배치 모드 HUD
    ├── Session/
    │   ├── CreateRoomWindow
    │   ├── RoomListWindow
    │   └── RoomListEntryWidget
    ├── WaitingRoom/WaitingRoomHUDWidget
    ├── Interact/InteractionWidget
    ├── Ability/                        # 어빌리티 UI
    │   ├── AbilitySlotWidget           # 어빌리티 슬롯 (아이콘, 키바인딩, 쿨다운, 충전)
    │   └── AbilityPromptWidget         # Confirm/Cancel 프롬프트 (WidgetComponent)
    ├── Profile/                        # 플레이어 프로필
    │   ├── PlayerProfileWidget         # 프로필 UI (3D 프리뷰 + 스탯 표시)
    │   └── CharacterPreviewActor       # 3D 캐릭터 프리뷰 (SceneCaptureComponent2D)
    ├── Cosmetic/                        # 코스메틱 UI
    │   ├── CosmeticMainWindow          # 코스메틱 메인 윈도우 (상점/인벤토리 모드)
    │   ├── CosmeticItemEntryWidget     # 아이템 엔트리 위젯
    │   └── CosmeticPreviewPanel        # 3D 프리뷰 패널
    └── HUD/
        ├── GameplayGlobalHUDWidget     # 게임플레이 글로벌 HUD
        └── ApproachingWallHUDWidget    # Approaching Wall 전용 HUD
```

## 주요 클래스 계층
```
GameMode: AWjWorldGameModeBase → Intro, Login, Lobby, WaitingRoom, Play
Character: AWjWorldCharacterBase → Lobby, WaitingRoom, Play (+ CosmeticComponent)
PlayerController: AWjWorldPlayerControllerBase → Lobby (+ PlacementComponent), WaitingRoom, Play
GameState: AWjWorldGameStateBase → GameStateLobby → GameStateWaitingRoom, GameStatePlay
PlayerState: AWjWorldPlayerStateBase (+ FCosmeticLoadout) → Play (+ IAbilitySystemInterface)
HUD: AWjWorldHUDBase → Lobby, WaitingRoom, Play
UI Widget: UWjWorldUserWidgetBase → 각종 HUD 및 윈도우 위젯
GameRule: UWjWorldGameRuleBase → ApproachingWall (미니게임 규칙, MinigameCatalog에서 조회)
GameData: UWjWorldGameDataComponent → ApproachingWall 전용 데이터
Ability: UWjWorldGameplayAbilityBase → GA_NormalAttack, GA_SpawnBrick, GA_LiftBrick
Subsystem: UGameInstanceSubsystem → CosmeticSubsystem, PurchaseSubsystem, StatsSubsystem
```

## 핵심 시스템

### GameRule 시스템
미니게임을 정의하기 위한 규칙 시스템. `UWjWorldGameRuleBase`를 상속받아 각 미니게임의 규칙을 구현.
- **라이프사이클**: `Initialize()` → `OnGameReady()` → `OnGameStart()` → `OnGameEndPredict()` → `OnGameEnd()`
- **플레이어 이벤트**: `OnPlayerJoined()`, `OnPlayerLeft()`
- **승리 조건**: `CheckWinCondition()`, `GetWinner()`
- **틱 처리**: `GameModePlay::Tick()`에서 `TickGameRule()` 직접 호출
- **동적 조회**: `MinigameCatalog`에서 `GameModeId`로 `GameRuleClass` 조회 (BP_GameModePlay 단일 사용)

### GameData 컴포넌트 시스템
게임/플레이어별 데이터를 관리하는 컴포넌트 시스템. GameplayTag 기반 타입 세이프 데이터 저장.
- `GameStatePlay`에 게임 전체 데이터 (예: 웨이브 타이밍)
- `PlayerStatePlay`에 플레이어별 데이터 (예: 점수, 상태)
- 리플리케이션 지원

### 미니게임 카탈로그 시스템
`UWjWorldMinigameDataAsset` 기반 미니게임 정의 및 동적 조회.
- **FWjWorldMinigameDefinition**: DisplayName, GameModeId, LevelPath, GameRuleClass, MapOptions
- **FWjWorldMinigameMapOption**: 맵 변형 옵션 (예: 기본, 랜덤)
- **동적 GameRule 조회**: `GameModePlay::InitGame()`에서 URL Options의 `GameModeId`로 카탈로그 조회
- **DeveloperSettings 참조**: `MinigameCatalog` 소프트 참조

### 로비 배치 시스템
로비에서 오브젝트를 배치/삭제하고 저장하는 시스템. 멀티플레이어 지원.
- **PlacementComponent**: `PlayerControllerLobby`에 부착, 배치 핵심 로직, EnhancedInput 바인딩
- **PlacementPreviewActor**: 배치 프리뷰 (유효/무효 색상), `FStreamableManager` 비동기 메시 로드
- **PlacedObjectActor**: 실제 배치된 오브젝트, 삭제 모드 하이라이트
- **PlaceableObjectDataAsset**: 배치 가능 오브젝트 카탈로그 (`FPlaceableObjectDefinition`)
- **LayoutSaveGame**: `USaveGame` 기반 레이아웃 저장/로드 (`LobbyLayout` 슬롯)
- **GameStateLobby**: 배치 오브젝트 리플리케이션 (`TArray<FPlacedObjectSaveEntry>`)
- **입력**: LMB(배치), R(회전), DEL(삭제), ESC(종료)

### Approaching Wall 미니게임
첫 번째 미니게임. 벽이 점진적으로 다가오며 플레이어들이 안전 구역으로 이동해야 하는 PvP 게임.
- **BrickSpawner**: 데이터 에셋 기반 비동기 벽돌 스폰 (8개/틱)
- **BrickMovement**: 개별 벽돌 이동 로직 (경로 탐색)
- **WallManager**: 벽 이동 진행 관리 (레벨별 속도 조절)
- **레벨 시스템**: 12초마다 레벨업, 이동 시간 5초→1초 (10레벨)
- **안전 구역**: Flood Fill 알고리즘으로 축소
- **TileActor**: 안전 구역 타일, 폭탄 신호 시스템 (3초 차징), 노랑→빨강 색상 전환, 방향별 오버랩 체크
- **BrickPreviewActor**: 어빌리티 배치 프리뷰, 유효(초록)/무효(빨강) 색상 표시, 동적 머티리얼

### Gameplay Ability System
GAS 기반 어빌리티 시스템. `UWjWorldGameplayAbilityBase`를 상속받아 각 어빌리티 구현.
- **AbilityBase 공통 기능**: AbilityName, AbilityIcon (UI 메타), GetPromptDescription(), 충전 시스템 인터페이스 (IsChargeBased, GetCurrentCharges, GetMaxCharges, GetChargeRefillTimeRemaining)
- **GA_NormalAttack**: 4방향 스냅(Yaw 기반) 벽돌 공격, BrickType별 처리 (Standard 파괴 불가, Explosive/Moving/Destructible)
- **GA_SpawnBrick**: 충전 기반 벽돌 배치, Preview → Confirm/Cancel 패턴, GE 기반 충전 리필, 어트리뷰트 변경 위임
- **GA_LiftBrick**: 벽돌 재배치 어빌리티, Moving/Destructible 벽돌 들어올리기, Cancel 시 원래 위치 복원
- **AttributeSet**: HP, MaxSpawnBrickCharges, SpawnBrickCharges, OnRep 콜백
- **Effects**: GE_AbilityCooldown (쿨다운), GE_SpawnBrickChargeCost (충전 비용)

### GameplayTag 정의
- `State_SpawnBrickPreview` - GA_SpawnBrick 활성 상태
- `State_LiftBrickCarry` - GA_LiftBrick 활성 상태
- `Cooldown_NormalAttack` - NormalAttack 쿨다운 태그
- `Cooldown_LiftBrick` - LiftBrick 쿨다운 태그

### 코스메틱 시스템
Steam 무료 출시 후 유료 코스메틱 판매를 위한 시스템. ItemId(FName) 기반 플랫폼 독립 식별.
- **CosmeticTypes**: `ECosmeticSlot`(Head/Body/Back/Effect), `FCosmeticSlotEntry`, `FCosmeticLoadout`(TArray 기반 리플리케이션 지원)
- **CosmeticComponent**: 캐릭터에 부착, 비동기 에셋 로드(FStreamableManager), 슬롯별 메시 관리, 로컬 플레이어만 브로드캐스트 수신
- **CosmeticSubsystem**: GameInstanceSubsystem. 인벤토리 캐시, 로드아웃 관리, 로컬 저장(GConfig), Steam Inventory 폴링 콜백
- **CosmeticDataAsset**: 카탈로그. `FCosmeticItemDefinition`(ItemId, SteamItemDefId, 메시, 아이콘, 가격). 양방향 룩업
- **PurchaseSubsystem**: GameInstanceSubsystem. Steam MicroTransaction API 연동, 구매 상태 관리, 폴링 기반 결과 콜백
- **테스트 함수**: `GenerateTestItem()`, `GrantAllItemsLocally()`, `ClearLocalInventory()`, `DebugPrintInventory/Loadout()`
- **콘솔 명령어**: `Cosmetic_GrantItem`, `Cosmetic_GrantAll`, `Cosmetic_ClearInventory`, `Cosmetic_PrintInventory/Loadout`, `Cosmetic_Equip/Unequip`, `Cosmetic_RefreshInventory`

### 코스메틱 리플리케이션 흐름
```
[서버 측 - PossessedBy]
Character.PossessedBy() → PlayerStateBase.OnPawnSet()
    ↓ (bPendingCosmeticApply 체크)
CosmeticComponent.ApplyLoadout() (서버에서 즉시 적용)

[클라이언트 - 자신의 캐릭터]
PlayerStateBase.BeginPlay() → ServerSetCosmeticLoadout() RPC
    ↓
OnRep_CosmeticLoadout() → OnCosmeticLoadoutUpdated()
    ↓ (Pawn 없으면 bPendingCosmeticApply = true)
CharacterBase.OnRep_PlayerState() → PS->OnPawnSet() → 적용

[클라이언트 - 3자 캐릭터]
CharacterBase.OnRep_PlayerState() (PlayerState 복제 시 호출)
    ↓
CosmeticComponent.SetCatalog() + PS->OnPawnSet()
    ↓
CosmeticComponent.ApplyLoadout() (비동기 메시 로드)
    ↓
캐릭터 비주얼 적용
```

### Stats 시스템
Steam User Stats 래핑 + GConfig 폴백 (비Steam 빌드용). `UWjWorldStatsSubsystem` (GameInstanceSubsystem).
- **로컬 스탯**: ReadLocalStat, IncrementLocalStat, StoreStats (GConfig 또는 Steam API)
- **원격 스탯**: RequestUserStats() + OnUserStatsReceived 비동기 델리게이트
- **미니게임 스탯**: 네임스페이스 기반 (`WjWorldStats::ApproachingWall::Wins/Losses/Kills/GamesPlayed`)
- **FMinigameStatEntry**: 개별 스탯 항목
- **FMinigameStatDescriptor**: UI 표시용 스탯 설명자
- **자동 기록**: GameStatePlay에서 게임 종료 시 승/패/킬 자동 증가
- **WITH_STEAM 조건부 컴파일**: Steam API 사용, 비Steam 빌드는 TMap 폴백

### 플레이어 프로필 시스템
- **PlayerProfileWidget**: 3D 캐릭터 프리뷰 + 미니게임별 스탯 표시, 비동기 스탯 로드, CosmeticLoadout 연동
- **CharacterPreviewActor**: SceneCaptureComponent2D로 오프스크린 3D 렌더링 (256x512), FStreamableManager 비동기 코스메틱 메시 로드, SpotLightComponent 조명

### Steam 빌드 설정
- **조건부 컴파일**: `WITH_STEAM` 매크로 (Win64에서만 활성화)
- **모듈**: Steamworks, OnlineSubsystemSteam (Win64 전용)
- **플러그인**: OnlineSubsystemSteam 활성화
- **코스메틱/구매/스탯 코드**: `#if WITH_STEAM` 블록으로 Steam API 호출 분리

## 구현 완료 기능
- 인트로/로그인/로비/대기실 게임모드
- 세션 관리 (방 생성/참가)
- 포탈 상호작용 시스템
- 퀘스트 시스템 기본 구조
- 네트워크 패킷 구조
- **게임플레이 모드 프레임워크** (Play 클래스 세트)
- **GameRule 시스템** (미니게임 규칙 정의, MinigameCatalog 동적 조회)
- **GameData 컴포넌트 시스템** (게임/플레이어 데이터)
- **Approaching Wall 기본 구조** (벽돌 스폰, 이동, 레벨 시스템, 타일 폭탄 신호)
- **게임플레이 HUD** (카운트다운, 결과 표시, GameRule별 HUD 위젯 매핑)
- **Ability System Component 통합**
- **게임플레이 어빌리티** (GA_NormalAttack, GA_SpawnBrick, GA_LiftBrick)
- **충전 시스템** (어트리뷰트 기반 충전, GE 기반 리필)
- **어빌리티 UI** (AbilitySlotWidget 쿨다운/충전 표시, AbilityPromptWidget Confirm/Cancel)
- **벽돌 프리뷰 시스템** (BrickPreviewActor 유효/무효 색상)
- **코스메틱 시스템** (컴포넌트, 서브시스템, 카탈로그, 타입)
- **코스메틱 리플리케이션** (PlayerStateBase로 이동, 모든 모드에서 사용)
- **코스메틱 멀티플레이어 동기화** (CharacterBase.OnRep_PlayerState, 3자 캐릭터 동기화)
- **코스메틱 상점 UI** (CosmeticMainWindow, 장착/해제/구매 통합)
- **Steam Inventory 폴링 콜백** (CosmeticSubsystem, PurchaseSubsystem)
- **코스메틱 테스트 콘솔 명령어** (PlayerControllerBase Exec 함수)
- **구매 시스템** (PurchaseSubsystem, Steam MicroTransaction 연동)
- **Steam 빌드 설정** (조건부 컴파일, 플러그인, 모듈)
- **플레이어 제거 상태** (bIsEliminated 리플리케이션)
- **스탯 시스템** (WjWorldStatsSubsystem, 미니게임별 스탯, 자동 기록)
- **플레이어 프로필** (PlayerProfileWidget, CharacterPreviewActor 3D 프리뷰)
- **개발자 설정** (WjWorldDeveloperSettings)
- **로그 카테고리** (LogWjWorld, LogWjWorldAbilities, LogWjWorldCosmetic, LogWjWorldStats, LogWjWorldPlacement)
- **로비 배치 시스템** (PlacementComponent, PreviewActor, PlacedObjectActor, 저장/로드)
- **GameStateLobby** (배치 오브젝트 멀티플레이어 리플리케이션)
- **미니게임 카탈로그** (MinigameDataAsset, GameRule 동적 조회)
- **WaitingRoom GameMode 오버라이드** (Lobby 맵 + `?game=` URL 옵션)
- **대기실 Ready 상태 즉시 동기화** (OnReadyStateChanged 구독)

## 진행 중 / 미구현
- 코스메틱 미리보기/시착 시스템
- 추가 미니게임 구현
- Steam 실제 환경 테스트 (AppID 발급 후)

## 코딩 컨벤션
- 언리얼 엔진 코딩 표준 준수
- 클래스 접두사: `A` (Actor), `U` (UObject), `F` (구조체)
- 프로젝트 접두사: `WjWorld`
- 한글 주석 사용 가능

## 빌드 명령어
- Visual Studio에서 F5 (DebugGame Editor)
- `Batch/` 폴더의 배치 파일 활용
  - `GenerateProjectFiles.bat` - 프로젝트 파일 생성
  - `OpenSolution.bat` - VS 솔루션 열기
  - `PackageDebugGame.bat` - 디버그 게임 패키징
  - `RebuildProject.bat` - 전체 리빌드
  - `RunDebugEditor.bat` - 디버그 에디터 실행
  - `GenerateDocs.bat` - Doxygen 문서 생성

## 문서화
- Doxygen 사용 (`Doxyfile` 설정 완료)
- `docs/` 폴더에 생성된 문서 저장

## 게임 플로우
```
게임 시작 → 인트로 → 로그인 → 로비 (싱글)
    ↓
방 생성 → OpenLevel(Lobby?game=WaitingRoom?Listen)
    ↓
대기실 (Lobby 맵 + WaitingRoom GameMode, 호스트 배치 오브젝트 표시)
    ↓
게임 시작 → ServerTravel(PlayMap?game=GameModePlay?GameModeId=xxx?MapOption=yyy)
    ↓
GameModePlay: MinigameCatalog에서 GameRuleClass 조회 → GameRule 생성
    ↓
GameRule 초기화 → OnGameReady → 카운트다운 → OnGameStart
    ↓
게임 진행 (TickGameRule) → 승리 조건 체크 → OnGameEndPredict → OnGameEnd
    ↓
결과 표시 → 스탯 자동 기록 → ServerTravel(Lobby?game=WaitingRoom) → 대기실 복귀
```

## 주요 데이터 흐름

### Approaching Wall
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
    ↓
TileActor (안전 구역 타일, 폭탄 신호)
```

### 어빌리티 사용 (Preview + Confirm/Cancel 패턴)
```
플레이어 입력 (R키 등)
    ↓
GA_SpawnBrick/GA_LiftBrick.ActivateAbility()
    ↓
BrickPreviewActor 생성 (유효/무효 색상)
    ↓ (Confirm)
GE_SpawnBrickChargeCost 적용 → 충전 감소
    ↓
벽돌 배치/이동 실행
    ↓ (또는 Cancel)
원래 상태 복원
```

### 코스메틱
```
CosmeticCatalogDataAsset (아이템 정의)
    ↓
CosmeticSubsystem (인벤토리 캐시 + 로드아웃 관리)
    ↓
PlayerStateBase (FCosmeticLoadout 리플리케이션, 모든 모드)
    ↓
CosmeticComponent (비동기 메시 로드 → 캐릭터 비주얼 적용)
```

### 구매
```
PurchaseSubsystem.RequestPurchase(ItemId)
    ↓ (카탈로그에서 SteamItemDefId 변환)
Steam MicroTransaction API
    ↓ (비동기 콜백)
PurchaseSubsystem.HandlePurchaseResult()
    ↓ (성공 시)
CosmeticSubsystem.RequestInventoryRefresh()
    ↓
OnPurchaseComplete 델리게이트 브로드캐스트
```

### 스탯
```
GameStatePlay.OnRep_GameResult() (게임 종료)
    ↓
WjWorldStatsSubsystem.IncrementLocalStat() (승/패/킬)
    ↓
StoreStats() (Steam API 또는 GConfig)
    ↓
PlayerProfileWidget (스탯 표시)
    ↓
CharacterPreviewActor (3D 프리뷰)
```

### 로비 배치 시스템
```
[싱글 로비] LobbyHUD "배치 모드" 버튼
    ↓
PlacementComponent.EnterPlacementMode() → IMC_Placement 활성화
    ↓
카탈로그에서 오브젝트 선택 → PlacementPreviewActor 생성
    ↓
TickComponent: 마우스 트레이스 → 프리뷰 위치/유효성 갱신
    ↓
LMB: ConfirmPlacement() → PlacedObjectActor 스폰 + SaveLayout()
    ↓
LayoutSaveGame (LobbyLayout 슬롯) → 로컬 저장

[멀티 대기실] GameModeWaitingRoom.BeginPlay()
    ↓
LoadHostLayoutToGameState() → 호스트 SaveGame 로드
    ↓
GameStateLobby.SetPlacedObjects() → 리플리케이션
    ↓
OnRep_PlacedObjects() → 모든 클라이언트에서 오브젝트 스폰
```

### 맵 전환 흐름
```
[방 생성]
CreateRoomWindow.OnRoomCreated()
    ↓
OpenLevel("/Game/Map/02-1_Lobby?game=/Game/.../BP_GameModeWaitingRoom_C?Listen")
    ↓
Lobby 맵 + WaitingRoom GameMode로 Listen Server 시작

[게임 시작]
GameModeWaitingRoom.StartGame()
    ↓
MinigameCatalog.FindByGameModeId(Settings.GameMode) → LevelPath 조회
    ↓
ServerTravel("{LevelPath}?game=BP_GameModePlay_C?GameModeId={id}?MapOption={opt}")
    ↓
GameModePlay.InitGame(): URL Options 파싱 → GameRule 생성

[게임 종료]
GameRuleBase.OnGameEnd()
    ↓
ServerTravel("/Game/Map/02-1_Lobby?game=BP_GameModeWaitingRoom_C")
    ↓
대기실 복귀 (Lobby 맵 + WaitingRoom GameMode)
```
