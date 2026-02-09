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
│   │   ├── GA_LiftBrick              # 벽돌 이동/재배치 (Preview + Confirm/Cancel)
│   │   ├── GA_Push                   # Sumo 넉백 (구형 오버랩 + LaunchCharacter)
│   │   └── GA_Jump                   # Sumo 점프 (UE CharacterJump 패턴, LocalPredicted)
│   ├── AttributeSets/                 # 어트리뷰트 셋
│   │   └── WjWorldCharacterAttributeSet  # HP, MaxSpawnBrickCharges, SpawnBrickCharges 등
│   ├── Effects/                       # GameplayEffect 파일들
│   │   ├── GE_AbilityCooldown         # 어빌리티 쿨다운
│   │   ├── GE_SpawnBrickChargeCost    # SpawnBrick 충전 비용
│   │   ├── GE_SumoSpeedBoost         # Sumo 이동속도 버프 (참조용 GE)
│   │   ├── GE_SumoSuperPush          # Sumo 강화 넉백 버프 (참조용 GE)
│   │   └── GE_SumoShield             # Sumo 보호막 버프 (참조용 GE)
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
│   │   ├── WjWorldGameRuleApproachingWall  # Approaching Wall 규칙
│   │   └── WjWorldGameRuleSumo       # Sumo Knockoff 규칙 (Z 낙하 감지)
│   ├── GameData/                      # 게임 데이터 컴포넌트
│   │   ├── WjWorldGameDataComponent   # 데이터 컴포넌트 베이스
│   │   ├── ApproachingWallGameDataComponent   # AW 게임 데이터
│   │   ├── ApproachingWallPlayerDataComponent # AW 플레이어 데이터
│   │   ├── SumoGameDataComponent      # Sumo 게임 데이터 (AlivePlayerCount)
│   │   └── SumoPlayerDataComponent    # Sumo 플레이어 데이터 (bIsAlive)
│   ├── Editor/                        # 에디터 모드 (배치 편집용 싱글플레이 맵)
│   │   ├── AWEditor/                  # Approaching Wall 에디터
│   │   │   ├── WjWorldGameModeAWEditor
│   │   │   ├── WjWorldGameStateAWEditor  # IWjWorldPlacementDataProvider 구현
│   │   │   ├── WjWorldPlayerControllerAWEditor
│   │   │   └── WjWorldHUDAWEditor
│   │   └── JumpMapEditor/             # JumpMap 에디터
│   │       ├── WjWorldGameModeJumpMapEditor
│   │       ├── WjWorldGameStateJumpMapEditor
│   │       ├── WjWorldPlayerControllerJumpMapEditor
│   │       └── WjWorldHUDJumpMapEditor
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
├── Animation/                         # 애니메이션
│   └── WjWorldAnimInstance            # 커스텀 AnimInstance (LiftBrickBlendWeight 등)
├── Setting/                           # 개발자 설정
│   └── WjWorldDeveloperSettings       # 중앙 설정 (맵, GameMode, 캐릭터, 게임플레이 에셋)
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
│   ├── Placement/                     # 다중 컨텍스트 배치 시스템
│   │   ├── WjWorldPlacementComponent  # 배치 핵심 로직 (컨텍스트 지원, 슬롯 저장/로드)
│   │   ├── WjWorldPlacementPreviewActor  # 배치 프리뷰 액터
│   │   ├── WjWorldPlacedObjectActor   # 배치된 오브젝트 액터
│   │   ├── WjWorldPlacementTypes      # EPlacementContext, SaveSlot 헬퍼
│   │   └── IWjWorldPlacementDataProvider  # 배치 데이터 프로바이더 인터페이스
│   ├── Quest/                         # 퀘스트 시스템
│   │   ├── Quest
│   │   ├── QuestInstance
│   │   ├── QuestState
│   │   ├── QuestFactory
│   │   └── QuestSubsystem
│   ├── Sumo/                          # Sumo Knockoff 게임플레이
│   │   ├── SumoFloorRingActor        # 축소 플랫폼 링 (Active/Warning/Destroyed)
│   │   └── SumoPowerUpActor          # 파워업 픽업 (Speed/SuperPush/Shield)
│   └── Wall/                          # Approaching Wall 게임플레이
│       ├── WjWorldBrickActor          # 벽돌 액터
│       ├── WjWorldBrickComponent      # 벽돌 컴포넌트
│       ├── WjWorldBrickMovement       # 벽돌 이동 로직
│       ├── WjWorldBrickSpawner        # 벽돌 스포너 (비동기)
│       ├── WjWorldBrickPreviewActor   # 어빌리티 배치 프리뷰 (유효/무효 색상)
│       ├── WjWorldTileActor           # 안전 구역 타일 (폭탄 신호, 색상 전환)
│       ├── WjWorldWallManager         # 벽 이동 관리
│       ├── WjWorldWallDescriptionDataAsset  # 벽 레이아웃 데이터
│       └── WjWorldWallLayoutConverter  # 배치 오브젝트 → WallLayout CSV 변환
├── Network/                           # 네트워크/패킷 관련
│   ├── PacketData
│   ├── PacketDataQuest
│   ├── SessionTypes
│   └── WjWorldLanNetDriver           # LAN 전용 NetDriver (PLATFORM_SOCKETSUBSYSTEM 명시)
└── UI/                                # UI 위젯들
    ├── WjWorldUserWidgetBase          # UI 베이스 클래스
    ├── Intro/IntroWindow
    ├── Login/LoginWindow
    ├── Lobby/
    │   ├── LobbyHUDWidget
    │   └── PlacementHUDWidget         # 로비 배치 모드 HUD (PlacementHUDWidgetBase 상속)
    ├── Placement/                      # 배치 UI 공통
    │   ├── PlacementHUDWidgetBase     # 배치 HUD 베이스 (Save/Load/Catalog)
    │   ├── PlacementSaveDialogWidget  # 저장 다이얼로그 (슬롯 이름, 유효성 표시)
    │   ├── PlacementLoadDialogWidget  # 불러오기 다이얼로그 (슬롯 목록)
    │   ├── PlacementContextSelectWidget  # 컨텍스트 선택 팝업 (Lobby/AW/JumpMap)
    │   ├── PlacementHUDWidgetAWEditor # AW 에디터 전용 HUD (WallLayout 변환)
    │   └── PlacementHUDWidgetJumpMapEditor  # JumpMap 에디터 전용 HUD
    ├── Session/
    │   ├── CreateRoomWindow
    │   ├── RoomListWindow
    │   └── RoomListEntryWidget
    ├── WaitingRoom/WaitingRoomHUDWidget  # 대기실 HUD (플레이어 목록, 호스트 설정 패널)
    ├── Interact/InteractionWidget
    ├── Ability/                        # 어빌리티 UI
    │   ├── AbilitySlotWidget           # 어빌리티 슬롯 (아이콘, 키바인딩, 쿨다운, 충전)
    │   └── AbilityPromptWidget         # Confirm/Cancel 프롬프트 (WidgetComponent)
    ├── Profile/                        # 플레이어 프로필
    │   ├── PlayerProfileWidget         # 프로필 UI (3D 프리뷰 + 스탯 표시)
    │   └── CharacterPreviewActor       # 3D 캐릭터 프리뷰 (Socket 부착, Static/SkeletalMesh)
    ├── Cosmetic/                        # 코스메틱 UI
    │   ├── CosmeticMainWindow          # 코스메틱 메인 윈도우 (상점/인벤토리 모드)
    │   ├── CosmeticItemEntryWidget     # 아이템 엔트리 위젯
    │   └── CosmeticPreviewPanel        # 3D 프리뷰 패널
    └── HUD/
        ├── GameplayGlobalHUDWidget     # 게임플레이 글로벌 HUD
        ├── ApproachingWallHUDWidget    # Approaching Wall 전용 HUD
        └── SumoHUDWidget              # Sumo Knockoff 전용 HUD (킬피드, 라운드)
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
GameRule: UWjWorldGameRuleBase → ApproachingWall, Sumo (미니게임 규칙, MinigameCatalog에서 조회)
GameData: UWjWorldGameDataComponent → ApproachingWall, Sumo 전용 데이터
Ability: UWjWorldGameplayAbilityBase → GA_NormalAttack, GA_SpawnBrick, GA_LiftBrick, GA_Push, GA_Jump
NetDriver: UIpNetDriver → UWjWorldLanNetDriver (LAN 전용, PLATFORM_SOCKETSUBSYSTEM)
Subsystem: UGameInstanceSubsystem → CosmeticSubsystem, PurchaseSubsystem, StatsSubsystem
AnimInstance: UWjWorldAnimInstance (LiftBrickBlendWeight, GameplayTag 기반 상태)
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
- **ApproachingWallGameDataComponent**: `CurrentWallName` 리플리케이트 (클라이언트 WallDesc 로드용)

### 미니게임 카탈로그 시스템
`UWjWorldMinigameDataAsset` 기반 미니게임 정의 및 동적 조회.
- **FWjWorldMinigameDefinition**: DisplayName, GameModeId, LevelPath, GameRuleClass, MapOptions, AllowedAbilityTags, StatNamespace
- **FWjWorldMinigameMapOption**: 맵 변형 옵션 (예: 기본, 랜덤)
- **AllowedAbilityTags**: 미니게임별 허용 어빌리티 태그 (빈 = 전부 허용, 하위 호환)
- **StatNamespace**: 미니게임별 스탯 키 접두사 (예: "AW", "Sumo")
- **동적 GameRule 조회**: `GameModePlay::InitGame()`에서 URL Options의 `GameModeId`로 카탈로그 조회
- **DeveloperSettings 참조**: `MinigameCatalog` 소프트 참조

### 다중 컨텍스트 배치 시스템
Lobby / ApproachingWall / JumpMap 3개 컨텍스트를 지원하는 확장된 배치 시스템.
- **EPlacementContext**: `None`, `Lobby`, `ApproachingWall`, `JumpMap` 열거형
- **IWjWorldPlacementDataProvider**: GameState 추상화 인터페이스 (AddPlacedObject, RemovePlacedObjectAt, GetPlacedObjects)
- **PlacementComponent**: 컨텍스트 지원, `SaveLayoutToSlot()`/`LoadLayoutFromSlot()`, `GetSavedLayoutSlots()`, `LoadedSlotName` 추적
- **PlacementPreviewActor**: 배치 프리뷰 (유효/무효 색상), `FStreamableManager` 비동기 메시 로드
- **PlacedObjectActor**: 실제 배치된 오브젝트, 삭제 모드 하이라이트
- **PlaceableObjectDataAsset**: 컨텍스트별 배치 가능 오브젝트 카탈로그 (`FPlaceableObjectDefinition`)
- **LayoutSaveGame**: `USaveGame` 기반 레이아웃 저장/로드 (컨텍스트별 SaveSlot: `LobbyLayout`, `ApproachingWallLayout`, `JumpMapLayout`)
- **GameStateLobby**: 배치 오브젝트 리플리케이션 (`TArray<FPlacedObjectSaveEntry>`)
- **입력**: LMB(배치), R(회전), DEL(삭제), ESC(종료)
- **에디터 모드**: AWEditor, JumpMapEditor 전용 GameMode/GameState/HUD
- **WallLayoutConverter**: AW 컨텍스트용 배치 오브젝트 → WallLayout CSV 변환, 외부/내부 영역 구분 유효성 검사
- **CSV 내보내기**: `ExportLayoutAsCSV()` - SaveGame 저장 시 CSV 파일도 자동 내보내기 (`Content/WallLayouts/User/`)
- **유저 레이아웃 자동 스캔**: `WallDescriptionDataAsset`에서 유저 CSV 디렉토리 런타임 스캔, 내장+유저 레이아웃 통합 지원

### Approaching Wall 미니게임
첫 번째 미니게임. 벽이 점진적으로 다가오며 플레이어들이 안전 구역으로 이동해야 하는 PvP 게임.
- **BrickSpawner**: 데이터 에셋 기반 비동기 벽돌 스폰 (8개/틱), 내장+유저 레이아웃 통합 지원
- **WallDescriptionDataAsset**: 내장 레이아웃 + 유저 레이아웃 자동 스캔 (`ScanUserWallLayouts()`, `GetWallDescriptionByNameIncludingUser()`)
- **BrickMovement**: 개별 벽돌 이동 로직 (경로 탐색)
- **WallManager**: 벽 이동 진행 관리 (레벨별 속도 조절)
- **레벨 시스템**: 12초마다 레벨업, 이동 시간 5초→1초 (10레벨)
- **안전 구역**: Flood Fill 알고리즘으로 축소
- **TileActor**: 안전 구역 타일, 폭탄 신호 시스템 (3초 차징), 노랑→빨강 색상 전환, 방향별 오버랩 체크
- **BrickPreviewActor**: 어빌리티 배치 프리뷰, 유효(초록)/무효(빨강) 색상 표시, 동적 머티리얼

### Sumo Knockoff 미니게임
두 번째 미니게임. 원형 플랫폼 위에서 상대를 밀어 떨어뜨리는 PvP 서바이벌.
- **WjWorldGameRuleSumo**: TickGameRule에서 매 프레임 Z 위치 체크, FallThresholdZ(-500) 미만 시 Eliminate
- **GA_Push**: 전방 구형 오버랩 → LaunchCharacter() 넉백, SetLastAttacker() 킬 추적, SuperPush 배율, CameraShake 피격 피드백
- **SumoGameDataComponent**: AlivePlayerCount, TotalPlayerCount, KillFeed (LastKillFeedText+Counter), Round (CurrentRound/MaxRounds), FSumoPlayerScore 배열 (모두 Replicated)
- **SumoPlayerDataComponent**: bIsAlive, TotalScore (Replicated + OnRep + Delegate)
- **SumoFloorRingActor**: 축소 플랫폼 링 (ESumoRingState: Active/Warning/Destroyed), RingOrder 기반 외곽부터 파괴
- **SumoPowerUpActor**: 파워업 픽업 (ESumoPowerUpType: SpeedBoost/SuperPush/Shield), SphereComponent 오버랩, AddLooseGameplayTag 버프
- **라운드 시스템**: 3라운드, 탈락 순서 기반 점수 배분, 라운드 간 링/파워업/플레이어 리셋
- **승리 조건**: 라운드 내 AlivePlayerCount <= 1, 최종 TotalScore 기준 우승자
- **맵 변형**: MapOption URL 파라미터 (Default/Bridge/Obstacle), 맵별 설정 분기
- **엣지 케이스**: 솔로 자동 승리, 동시 탈락, 전원 이탈
- **상태**: C++ 코드 완료, 에디터 세팅 필요 (BP 프로퍼티, 링 배치, HUD 위젯, 파워업 BP)

### Gameplay Ability System
GAS 기반 어빌리티 시스템. `UWjWorldGameplayAbilityBase`를 상속받아 각 어빌리티 구현.
- **AbilityBase 공통 기능**: AbilityName, AbilityIcon (UI 메타), GetPromptDescription(), 충전 시스템 인터페이스 (IsChargeBased, GetCurrentCharges, GetMaxCharges, GetChargeRefillTimeRemaining)
- **AbilityBase 어빌리티 제한**: `CanActivateAbility()` 오버라이드 - GameState의 `AllowedAbilityTags` 체크 (빈 = 전부 허용)
- **GA_NormalAttack**: 4방향 스냅(Yaw 기반) 벽돌 공격, BrickType별 처리 (Standard 파괴 불가, Explosive/Moving/Destructible)
- **GA_SpawnBrick**: 충전 기반 벽돌 배치, Preview → Confirm/Cancel 패턴, GE 기반 충전 리필, 어트리뷰트 변경 위임
- **GA_LiftBrick**: 벽돌 재배치 어빌리티, Moving/Destructible 벽돌 들어올리기, Cancel 시 원래 위치 복원, 들고 있는 벽돌 색상 리플리케이션
- **GA_Push**: Sumo 넉백 어빌리티, 전방 구형 오버랩 → LaunchCharacter(), PushForce=1200, CooldownDuration=1.5s, SetLastAttacker(), SuperPushMultiplier(2x), PushHitCameraShake
- **GA_Jump**: Sumo 점프 어빌리티, UE CharacterJump 패턴 기반, LocalPredicted, CommitAbility(), Character->Jump()/StopJumping(), 가변 높이 점프, InputReleased로 종료
- **AttributeSet**: HP, MaxSpawnBrickCharges, SpawnBrickCharges, OnRep 콜백
- **Effects**: GE_AbilityCooldown (쿨다운), GE_SpawnBrickChargeCost (충전 비용), GE_SumoSpeedBoost/SuperPush/Shield (참조용 GE, 실제 버프는 AddLooseGameplayTag)

### GameplayTag 정의
- `State_SpawnBrickPreview` - GA_SpawnBrick 활성 상태
- `State_LiftBrickCarry` - GA_LiftBrick 활성 상태
- `Cooldown_NormalAttack` - NormalAttack 쿨다운 태그
- `Cooldown_LiftBrick` - LiftBrick 쿨다운 태그
- `Ability_Push` - GA_Push 어빌리티 태그
- `Cooldown_Push` - GA_Push 쿨다운 태그
- `GameplayCue_Ability_Push` - Push 이펙트/사운드
- `Buff_SpeedBoost` - Sumo 이동속도 버프
- `Buff_SuperPush` - Sumo 강화 넉백 버프 (1회 소모)
- `Buff_Shield` - Sumo 보호막 (제거 1회 무시)
- `GameplayCue_Sumo_PowerUp_Pickup` - 파워업 획득 이펙트
- `Ability_Jump` - GA_Jump 어빌리티 태그
- `Cooldown_Jump` - GA_Jump 쿨다운 태그

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
- **미니게임 스탯**: 네임스페이스 기반 (`WjWorldStats::ApproachingWall`, `WjWorldStats::Sumo`)
- **FMinigameStatEntry**: 개별 스탯 항목
- **FMinigameStatDescriptor**: UI 표시용 스탯 설명자
- **자동 기록**: GameStatePlay에서 게임 종료 시 `StatNamespace` 기반 동적 스탯 키로 승/패/킬 자동 증가
- **WITH_STEAM 조건부 컴파일**: Steam API 사용, 비Steam 빌드는 TMap 폴백

### 플레이어 프로필 시스템
- **PlayerProfileWidget**: 3D 캐릭터 프리뷰 + 미니게임별 스탯 표시, 비동기 스탯 로드, CosmeticLoadout 연동
- **CharacterPreviewActor**: SceneCaptureComponent2D로 오프스크린 3D 렌더링 (256x512), FStreamableManager 비동기 코스메틱 메시 로드, Socket 기반 부착 (GetDefaultSocketName), StaticMesh/SkeletalMesh 동시 지원, SetupFromPawn()으로 Pawn에서 메시/ABP 복사

### 세션 관리 시스템
`USessionManager` (UObject, GameInstance 소유). Online Subsystem Session 관리.
- **OSS 초기화**: Steam OSS 우선 → 실패 시 NULL OSS 폴백
- **세션 CRUD**: `CreateSession()`, `FindSessions()`, `JoinSession()`, `StartSession()`, `EndSession()`, `DestroySession()`
- **네트워크 모드**: `ENetworkMode::LAN` / `ENetworkMode::Steam` 분기
  - LAN: `bIsLANMatch=true`, `bUsesPresence=false`
  - Steam: `bIsLANMatch=false`, `bUsesPresence=true`, `bUseLobbiesIfAvailable=true` (반드시 매칭)
- **검색 큐**: `bIsSearchInProgress` 플래그 + `PendingSearchRequest` (이전 검색 완료 후 자동 실행)
- **호스트 마이그레이션**: `CreateMigrationSession()`, `FindMigrationSession()` (MIGRATION_TAG 커스텀 세팅)
- **델리게이트**: `OnRoomCreatedEvent`, `OnRoomsFoundEvent`, `OnRoomJoinedEvent`, `OnRoomDestroyedEvent`, `OnRoomStartedEvent`, `OnRoomEndedEvent`

### Steam 빌드 설정
- **AppID**: 4399350, **DepotID**: 4399351
- **조건부 컴파일**: `WITH_STEAM` 매크로 (Win64에서만 활성화)
- **모듈**: Steamworks, OnlineSubsystemSteam (Win64 전용)
- **플러그인**: OnlineSubsystemSteam, SocketSubsystemSteamIP 활성화
- **네트워킹**: Steam=SteamNetDriver, LAN=WjWorldLanNetDriver (런타임 전환 via ApplyNetDriverForMode)
- **코스메틱/구매/스탯 코드**: `#if WITH_STEAM` 블록으로 Steam API 호출 분리
- **Inventory Service**: `Steam/itemdefs.json`에 아이템 정의
- **빌드 업로드**: `Steam/upload.bat` (SteamCMD 사용)
- **빌드 자동화**: `Batch/PackageAndUploadSteam.bat` (패키징→복사→업로드)

### Steam 네트워킹 Config (DefaultEngine.ini)
```ini
[/Script/Engine.Engine]
!NetDriverDefinitions=ClearArray
+NetDriverDefinitions=(DefName="GameNetDriver",DriverClassName="/Script/SocketSubsystemSteamIP.SteamNetDriver",DriverClassNameFallback="/Script/OnlineSubsystemUtils.IpNetDriver")

[OnlineSubsystemSteam]
bEnabled=true
SteamDevAppId=4399350
bUseSteamNetworking=true

[/Script/SocketSubsystemSteamIP.SteamNetDriver]
NetConnectionClassName="/Script/SocketSubsystemSteamIP.SteamNetConnection"
```
- **DriverClassName 형식**: `/Script/ModuleName.ClassName` (StaticLoadClass 정규 경로, 짧은 형식 불가)
- **bUseSteamNetworking**: SocketSubsystemSteamIP 모듈이 Steam 소켓 서브시스템 등록하는 조건
- **에디터 제한**: SocketSubsystemSteamIP은 패키징된 빌드에서만 동작 (에디터에서 자동 비활성화)
- **LAN 소켓 충돌 해결**: SocketSubsystemSteamIP가 기본 소켓을 Steam으로 오버라이드 → IpNetDriver 사용 불가 → `WjWorldLanNetDriver`(UIpNetDriver 서브클래스)에서 `GetSocketSubsystem()` → `PLATFORM_SOCKETSUBSYSTEM` 명시

### 패키징 주의사항
- **새 레벨/맵 추가 시**: Project Settings > Packaging > List of maps to include in a packaged build에 반드시 추가. 누락 시 `Failed to load package` 에러로 ServerTravel 실패
- **Non-asset 파일** (`.txt`, `.csv` 등): `DefaultGame.ini`의 `DirectoriesToAlwaysStageAsNonUFS`로 명시적 포함 필요
- **FFilePath 경로**: 에디터에서 절대 경로 저장 → 패키지 빌드에서 `FPaths::ProjectContentDir()` 기준으로 변환 필요
- **Debug vs Development 빌드**: Debug는 개발 PC 파일 시스템 직접 접근, Development/Shipping은 .pak 파일 사용

### WjWorldDeveloperSettings (중앙 설정)
에디터에서 설정 가능한 중앙 집중식 에셋/클래스 참조. Project Settings > Game > WjWorld Developer Settings에서 설정.
- **맵**: LobbyMapPath, AWEditorMapPath, JumpMapEditorMapPath
- **GameMode 클래스**: WaitingRoomGameModeClass, PlayGameModeClass, AWEditorGameModeClass, JumpMapEditorGameModeClass
- **캐릭터 기본값**: DefaultCharacterMesh, DefaultAnimBlueprintClass, DefaultInputMappingContext
- **Approaching Wall**: BrickMesh, TileMesh, WallDescriptionAsset
- **배치 카탈로그**: LobbyPlaceableCatalog, ApproachingWallPlaceableCatalog, JumpMapPlaceableCatalog
- **기타 카탈로그**: MinigameCatalog, CosmeticCatalog
- **헬퍼 함수**: GetLobbyMapPath(), GetWaitingRoomOpenLevelURL(), GetPlayServerTravelURL(), GetPlaceableCatalogForContext(), GetEditorMapOpenLevelURL(), HasEditorMapForContext()

**설정 우선순위 패턴**: BP 서브클래스 UPROPERTY 값 우선 → DeveloperSettings 폴백

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
- **코스메틱 미리보기/시착 시스템** (CharacterPreviewActor Socket 부착, StaticMesh/SkeletalMesh 지원, 다중 슬롯 시착)
- **하드코딩 경로 제거 및 DeveloperSettings 중앙화** (맵/GameMode/캐릭터/Approaching Wall 에셋)
- **Approaching Wall 완성** (승리 조건, 게임 결과 처리, 대기실 복귀)
  - Kills 스탯 추적 (LastAttacker 시스템)
  - 플레이어 이탈 시 캐릭터 제거 처리
  - 엣지 케이스 처리 (솔로 게임, 동시 제거, 전원 이탈)
- **Steam 2PC 테스트 버그 수정**
  - ServerTravel 타이머 람다 `this` 캡처 문제 → `TWeakObjectPtr<UWorld>` + URL 값 캡처
  - SaveLayout() 호스트만 저장하도록 `NetMode` 체크 추가
  - WaitingRoom 3자 캐릭터 코스메틱 → `TActorIterator`로 PlayerState 검색
  - 클라이언트 어빌리티 프리뷰 → `CurrentWallName` 리플리케이트 기반 WallDesc 로드
- **WjWorldAnimInstance** (GameplayTag 기반 LiftBrickBlendWeight)
- **LiftBrick 벽돌 색상 리플리케이션** (CarriedBrickColor + Dynamic Material)
- **LAN/Steam 네트워크 모드 토글** (ENetworkMode, UI 선택, 세션 생성/검색 분기)
- **Steam P2P 네트워킹** (SteamNetDriver via SocketSubsystemSteamIP, IpNetDriver 폴백)
- **세션 관리 고도화** (Steam→NULL OSS 폴백, 검색 큐 패턴, 호스트 마이그레이션)
- **빌드 자동화** (PackageAndUploadSteam.bat: 패키징→Steam content 복사→업로드)
- **Sumo Knockoff 기본 코드 구현** (GA_Push, GameRuleSumo, SumoGameData/PlayerDataComponent, 스탯)
- **Sumo Knockoff 기본 에디터 세팅 완료** (맵, 카탈로그 등록, BP, 입력 바인딩, 패키징 맵 목록, AllowedAbilityTags)
- **Sumo Knockoff 6대 기능 코드 구현** (Push 히트 피드백, 킬피드, 축소 플랫폼, 라운드 시스템, 파워업, 맵 변형)
- **미니게임별 어빌리티 제한 시스템** (AllowedAbilityTags, CanActivateAbility 오버라이드)
- **스탯 네임스페이스 범용화** (StatNamespace 기반 동적 스탯 키)
- **LAN SocketSubsystem 충돌 수정** (WjWorldLanNetDriver + ApplyNetDriverForMode 런타임 전환)
- **GA_Jump 어빌리티 구현** (UE CharacterJump 패턴, LocalPredicted, Ability7 InputID)
- **PackageAndUploadSteam.bat 버그 수정** (call 키워드, -build 플래그, pause 추가)
- **다중 컨텍스트 배치 시스템** (EPlacementContext, IWjWorldPlacementDataProvider, 컨텍스트별 카탈로그/SaveSlot)
- **배치 에디터 모드** (AWEditor, JumpMapEditor - GameMode/GameState/PlayerController/HUD)
- **배치 UI 위젯 리팩토링** (PlacementHUDWidgetBase 공통 기능, Save/Load 다이얼로그)
- **WallLayout 변환 시스템** (WjWorldWallLayoutConverter: 배치 오브젝트 → CSV, IsWallClosed 유효성 검사)
- **DeveloperSettings 확장** (에디터 맵 경로, 컨텍스트별 카탈로그)
- **AW Editor CSV 내보내기** (SaveGame 저장 시 CSV 자동 내보내기, `Content/WallLayouts/User/`)
- **유저 벽 레이아웃 지원** (WallDescriptionDataAsset 런타임 스캔, BrickSpawner 유저 레이아웃 검색)
- **WallLayoutConverter 외부/내부 영역 구분** (MarkExteriorCells + FindInteriorEmptyCell 로직)
- **대기실 호스트 설정 UI** (WaitingRoomHUDWidget: 게임모드/맵 변경, 유저 레이아웃 선택)
- **Steam 빌드 테스트 버그 수정 (#4, #5, #8, #12)**
  - TileActor collision 배열 초기화
  - Sumo 라운드 리셋 시 State_Eliminated 태그 제거
  - 클라이언트 벽돌 preview GridIndex Server RPC
  - 늦은 클라이언트 카운트다운 서버 시간 동기화
- **Steam 2PC 버그 수정 2차 (#2, #11, #16)**
  - Sumo 코스메틱 전이 버그 → 로드아웃 존재 여부 + 로컬 컨트롤러 체크
  - 호스트 설정 패널 클라이언트 표시 → GetNetMode() 이중 체크
  - 3자 프로필 스탯 조회 → Steam OSS IdentityInterface로 FUniqueNetIdRepl 생성
- **Steam 2PC 버그 수정 3차 - 전체 해결**
  - [Critical] 클라이언트 벽돌 스폰 → UObject에서 Server RPC 불가, Character RPC로 이동
  - [Critical] #14 호스트 설정 값 반영 → OnStartGameClicked에서 ApplyCurrentUISettings 자동 호출
  - #2 호스트 설정 패널 → 전체 표시 + 클라이언트 입력 비활성화 (읽기 전용)
  - #11 3자 프로필 조회 → TMap<UButton*, PlayerID> 매핑 + HasMouseCapture() 체크
  - #1 WaitingRoom UI 미갱신 → UpdateRoomInfo에 옵셔널 FRoomSettings 직접 전달
  - #4 유저 커스텀 맵 offset → CSV에 `#META:CenterOffset:x,y,z` 메타데이터 헤더 추가/파싱
  - #8 TileActor collision → SetBoxExtent(InSize * 0.5f), 방향별 HitBox 위치도 half extent
- **Steam 4차 버그 수정 및 코드 검증**
  - GamePhase 기반 어빌리티 제한 (Playing 상태에서만 허용)
  - 유저 맵 클라이언트 벽돌 스폰 수정 (GetWallDescriptionByNameIncludingUser)
  - BrickComponent collision 분리 (QueryOnly+Overlap / BlockAll 95%)
  - WaitingRoom 호스트 설정 패널 호스트 전용 표시 + Apply 후 Display 갱신
  - Steam CCallResult 패턴으로 타 유저 스탯 콜백 수정
  - ParseWallLayout #META:CenterOffset 메타데이터 파싱 추가
  - 제거 시 살아있는 플레이어로 관전 전환 (SetViewTargetWithBlend)
  - FindRoomButton 숨김 + 그래픽 품질 사이클 설정 (Low/Medium/High/Epic)
  - GA_Jump Super::ActivateAbility() 누락 수정
- **배치 에디터 BP 세팅 완료** (에디터 맵 생성, BP_PlacementSaveDialogWidget, BP_PlacementLoadDialogWidget, 컨텍스트별 카탈로그 DataAsset)
- **LobbyHUDWidget 정리** (DirectConnectButton 제거, FindRoomButton null 접근 버그 수정)

## 진행 중 / 미구현
- Sumo Knockoff 6대 기능 에디터 세팅 (BP 생성/프로퍼티 할당, 링 배치, HUD 위젯, 파워업 비주얼)
- 추가 미니게임 구현
- Steam 정식 출시 준비

## 잔존 버그 (Steam 2PC 테스트 2026-02-09)
- **#3 대각선 맵 movement** — 상하좌우 연결 맵은 정상이나, 대각선 연결 맵에서 movement가 wall closed하게 움직이지 않음
- **#4 클라이언트 벽돌 preview offset** — 유저 커스텀 맵에서 클라이언트 벽돌 설치 시 preview가 50,50 어긋남 (Default 모드 정상)
- **#11 3자 프로필 조회 안 됨** — 타 플레이어 프로필 조회 실패
- **#3(Sumo) Host 관전 Yaw 미적용** — Host가 클라이언트를 관찰할 때 Yaw가 적용되지 않음 (클라→Host 관측은 정상)
- **#4(Sumo) 유저 맵 클라이언트 벽돌 스폰 위치** — 유저 만든 AW 맵에서 클라이언트가 설치한 벽돌이 엉뚱한 위치에 스폰

## 확인 필요 사항
- Room 목록 스케일링 — Steam 배포 시 다수 방(1000+) 표시 및 부하 체크
- Sumo FloorRing 레벨 디자인 변경 검토 — 원형 축소 대신 개별 타일 랜덤 파괴 방식 전환 시 리플리케이션 비용 확인

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
  - `PackageAndUploadSteam.bat` - Development 패키징 + Steam 업로드 자동화
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

### 다중 컨텍스트 배치 시스템
```
[컨텍스트 선택] LobbyHUD "배치 모드" 버튼 → PlacementContextSelectWidget
    ↓
Lobby 선택 → EnterPlacementModeWithContext(Lobby)
AW/JumpMap 선택 → OpenLevel(EditorMap) → 전용 에디터 진입

[배치 편집] PlacementHUDWidgetBase
    ↓
카탈로그에서 오브젝트 선택 → PlacementPreviewActor 생성
    ↓
TickComponent: 마우스 트레이스 → 프리뷰 위치/유효성 갱신
    ↓
LMB: ConfirmPlacement() → PlacedObjectActor 스폰

[불러오기] Load 버튼 → PlacementLoadDialogWidget (슬롯 목록)
    ↓
슬롯 선택 → LoadLayoutFromSlot() → LoadedSlotName 저장
    ↓
기존 배치 클리어 → 로드된 오브젝트 스폰

[저장] Save 버튼 → PlacementSaveDialogWidget (기본값: LoadedSlotName)
    ↓
슬롯 이름 편집 → SaveLayoutToSlot()
    ↓
AW 컨텍스트: ExportLayoutAsCSV() → Content/WallLayouts/User/{SlotName}.csv
    ↓
WallLayoutConverter.ValidateWallLayout() → 외부/내부 영역 구분 유효성 검사

[AW Editor → 게임플레이 연동]
유저 CSV 저장 (Content/WallLayouts/User/)
    ↓
WallDescriptionDataAsset.ScanUserWallLayouts() (런타임 스캔)
    ↓
GetWallDescriptionByNameIncludingUser() (내장+유저 통합 검색)
    ↓
BrickSpawner.SpawnBricksFromWallNameAsync() (유저 레이아웃 로드)

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
