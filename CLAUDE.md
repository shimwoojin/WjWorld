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
├── AbilitySystem/                     # GAS (Abilities, AttributeSets, Effects, ASC)
│   ├── Abilities/                     # GA_NormalAttack, GA_SpawnBrick, GA_LiftBrick, GA_Push, GA_Jump, GA_Dash, GA_Grapple, GA_DoubleJump
│   ├── AttributeSets/                 # WjWorldCharacterAttributeSet (HP, SpawnBrickCharges)
│   ├── Effects/                       # GE_AbilityCooldown, GE_SpawnBrickChargeCost, GE_NormalAttackStagger, GE_Sumo*
│   └── WjWorldAbilitySystemComponent
├── Core/
│   ├── Base/                          # GameModeBase, CharacterBase, PlayerControllerBase, GameStateBase, PlayerStateBase(+Cosmetic), HUDBase
│   ├── Intro/Login/                   # 인트로/로그인
│   ├── Local/
│   │   ├── Lobby/                     # GameModeLobby, GameStateLobby(배치 리플리케이션), CharacterLobby, PCLobby(+PlacementComp), HUDLobby
│   │   └── WaitingRoom/               # Lobby 맵 + GameMode 오버라이드, GameStateWaitingRoom(←GameStateLobby)
│   ├── Play/                          # GameModePlay, GameStatePlay(스탯 자동 기록), PlayerStatePlay(ASC), CharacterPlay, HUDPlay
│   ├── GameRule/                      # GameRuleBase → ApproachingWall, Sumo, JumpMap
│   ├── GameData/                      # GameDataComponent → AW/Sumo/JumpMap 전용 Game/PlayerData
│   ├── Editor/                        # AWEditor, JumpMapEditor (배치 편집용 싱글플레이 맵)
│   ├── Components/                    # GameplaySceneComponent, GameplayActorComponent
│   ├── Session/SessionManager         # OSS 세션 관리 (Steam/LAN)
│   └── WjWorldGameInstance
├── Cosmetic/                          # CosmeticTypes, CosmeticComponent, CosmeticSubsystem, CosmeticDataAsset, PurchaseSubsystem
├── Currency/                          # CurrencySubsystem, CurrencyTypes (Coin/Gem 재화 관리)
├── Stats/                             # StatsSubsystem(Steam User Stats), StatTypes
├── Animation/                         # WjWorldAnimInstance
├── Setting/WjWorldDeveloperSettings   # 중앙 설정 (맵, GameMode, 에셋 참조)
├── DataAsset/                         # CharacterPlaySetup, MinigameDataAsset, PlaceableObjectDataAsset
├── Save/WjWorldLayoutSaveGame
├── GamePlay/
│   ├── Camera/
│   ├── Interact/InteractablePortal
│   ├── TreasureChest/                 # TreasureChestActor (보물상자 배치 오브젝트, 쿨타임+Coin 보상)
│   ├── Placement/                     # PlacementComponent, PreviewActor, PlacedObjectActor, CameraPawn, Types, IDataProvider
│   ├── Quest/                         # Quest, QuestInstance, QuestState, QuestFactory, QuestSubsystem
│   ├── Sumo/                          # SumoFloorRingActor, SumoPowerUpActor
│   ├── JumpMap/                       # ActorBase, KillZone, MovingPlatform, RotatingObstacle, PushWind, Checkpoint, End, GrapplePoint, LayoutDataAsset
│   └── Wall/                          # BrickActor/Component/Movement/Spawner, BrickPreviewActor, TileActor, WallManager, WallDescriptionDA, WallLayoutConverter
├── Network/                           # PacketData, SessionTypes, WjWorldLanNetDriver
└── UI/                                # UserWidgetBase, Intro, Login, Lobby, Placement, Session, WaitingRoom, Interact, Ability, Profile, Cosmetic, HUD
    ├── Chat/                          # ChatWidget (멀티플레이어 채팅)
    ├── Common/                        # ConfirmDialogWidget (공용 확인/취소 팝업)
    ├── HUD/                           # CoinGainNotificationWidget (재화 획득 토스트)
    ├── Session/                       # CreateRoomWindow, RoomListWindow, RoomListEntryWidget, PasswordInputWidget
    └── Setting/                       # SettingsWidget (그래픽 품질 + 마스터 볼륨 팝업)

Source/WjWorldEditor/                  # 에디터 전용 모듈
└── JumpMap/                           # JumpMapLevelEditorSubsystem, SJumpMapLayoutPanel
```

## 주요 클래스 계층
```
GameMode: AWjWorldGameModeBase → Intro, Login, Lobby, WaitingRoom, Play
Character: AWjWorldCharacterBase → Lobby, WaitingRoom, Play (+ CosmeticComponent)
PlayerController: AWjWorldPlayerControllerBase → Lobby (+ PlacementComponent), WaitingRoom, Play
GameState: AWjWorldGameStateBase → GameStateLobby → GameStateWaitingRoom, GameStatePlay
PlayerState: AWjWorldPlayerStateBase (+ FCosmeticLoadout) → Play (+ IAbilitySystemInterface)
HUD: AWjWorldHUDBase → Lobby, WaitingRoom, Play
GameRule: UWjWorldGameRuleBase → ApproachingWall, Sumo, JumpMap
GameData: UWjWorldGameDataComponent → AW/Sumo/JumpMap Game/PlayerData
Ability: UWjWorldGameplayAbilityBase → GA_NormalAttack, GA_SpawnBrick, GA_LiftBrick, GA_Push, GA_Jump(→GA_DoubleJump), GA_Dash, GA_Grapple
PlacedObject: AWjWorldPlacedObjectActor → TreasureChestActor
Widget: UWjWorldUserWidgetBase → PlacementHUDWidgetBase, PlacementSaveDialogWidget, PlacementLoadDialogWidget, ConfirmDialogWidget, SettingsWidget
Subsystem: UGameInstanceSubsystem → CosmeticSubsystem, CurrencySubsystem, PurchaseSubsystem, StatsSubsystem
```

## 핵심 시스템

### GameRule 시스템
- **라이프사이클**: `Initialize()` → `OnGameReady()` → `OnGameStart()` → `OnGameEndPredict()` → `OnGameEnd()`
- **플레이어 이벤트**: `OnPlayerJoined()`, `OnPlayerLeft()` / **승리**: `CheckWinCondition()`, `GetWinner()`
- **동적 조회**: `MinigameCatalog`에서 `GameModeId`로 `GameRuleClass` 조회 (BP_GameModePlay 단일 사용)

### GameData 컴포넌트 시스템
GameplayTag 기반 타입 세이프 데이터. `GameStatePlay`에 게임 데이터, `PlayerStatePlay`에 플레이어 데이터. 리플리케이션 지원.

### 미니게임 카탈로그
`UWjWorldMinigameDataAsset` — `FWjWorldMinigameDefinition`(DisplayName, GameModeId, LevelPath, GameRuleClass, MapOptions, AllowedAbilityTags, StatNamespace, DefaultCameraMode). `GameModePlay::InitGame()`에서 URL Options 기반 동적 조회. `DefaultCameraMode`는 `GameRuleBase::OnGameReady()`에서 `GameStatePlay`에 리플리케이션.

### 다중 컨텍스트 배치 시스템
Lobby / ApproachingWall / JumpMap 3개 컨텍스트 지원.
- **EPlacementContext** 열거형, **IWjWorldPlacementDataProvider** GameState 인터페이스
- **PlacementComponent**: 컨텍스트별 저장/로드/삭제, `ValidateJumpMapLayout()` 검증 (JumpMap 에디터 저장 시 검증 실패 → 차단)
- **PreviewActor**: 유효/무효 색상, 비동기 메시 로드, 회전 축(Yaw/Pitch/Roll) 전환
- **PlacedObjectActor**: ObjectId 저장, 삭제 하이라이트, `ActorClassOverride`로 서브클래스 스폰 분기
- **LayoutSaveGame**: `FPlacedObjectSaveEntry.CustomProperties` (JumpMap CheckpointOrder 등)
- **입력**: LMB(배치), R(회전), T(축 전환), G(각도 전환), DEL(삭제), F(공중모드), ESC(종료)
- **CSV 내보내기**: AW(`Content/WallLayouts/User/`), JumpMap(`Content/JumpMapLayouts/User/`, 11번째 Properties 컬럼)
- **유저 레이아웃**: `WallDescriptionDataAsset`/`JumpMapLayoutDataAsset`에서 유저 CSV 런타임 스캔
- **구매 시스템 (Lobby 전용)**: `FPlaceableObjectDefinition.CoinPrice/SteamItemDefId/MaxPlacementCount`, `DeveloperSettings.MaxTotalLobbyPlacedObjects`
  - **소유권**: `CosmeticSubsystem.GetItemQuantityByDefId()` (AllItemQuantities 캐시)
  - **구매**: `CurrencySubsystem.PurchasePlacementObject()` → Steam ExchangeItems / 비Steam GConfig
  - **구매 수량 = 설치 상한**: 1회 구매 = 1개 설치 권한, `MaxPlacementCount`는 구매 상한 (무료 아이템은 `MaxPlacementCount`가 설치 상한)
  - **제한**: SelectObject() 소유권 게이트, ConfirmPlacement() 유료→OwnedQty/무료→MaxPlacementCount 검증, GameStateLobby 서버 측 수량 검증
  - **UI**: PopulateCatalog()에서 유료 `[배치수/OwnedQty]`/무료 `[배치수/MaxPlacementCount]` 표시, 구매 버튼은 `OwnedQty < MaxPlacementCount`일 때 표시
  - **전체 삭제**: ClearButton → ConfirmDialogWidget 확인 → `ClearAllPlacedObjects()` (DataProvider.ClearPlacedObjects + SaveLayout)
  - **비Steam 폴백**: GConfig `[PlacementInventory]` 섹션
  - **테스트**: `Placement_Buy`, `Placement_PrintInventory`, `Placement_GrantItem` 콘솔 명령어
- **공용 ConfirmDialogWidget**: `UI/Common/` — ShowPopup/ClosePopup + OnConfirmed/OnCancelled 델리게이트, NativeConstruct 전 호출 캐시 패턴

### Approaching Wall 미니게임
벽이 다가오며 안전 구역으로 이동하는 PvP. BrickSpawner(비동기 8개/틱) → WallManager(레벨별 속도). 12초마다 레벨업, Flood Fill 안전 구역 축소(4방향 인접), TileActor 폭탄 신호.
- **벽 이동 알고리즘**: 중앙 할당 방식 — `AssignBrickTargets()`가 `ShrinkSafeZones()` 후 각 FloodFillPoint에 맨해튼 거리 기준 Greedy로 가장 가까운 Standard 벽돌 배정 → `BrickMovement.SetAssignedTarget()` 주입 → 4방향 제한 이동 (2칸 이상 거리 시 2칸 이동)
- **벽돌 타입별 머티리얼**: DeveloperSettings `BrickMaterialStandard/Explosive/Moving/Destructible` — `GetBrickMaterial()` 로드, 미설정 시 기존 색상 폴백
- **Destructible 단계별 파괴 연출**: `DestructibleBrickDamageStageMeshes` 배열 (타격마다 메시 교체) + `BrickDamageHitEffect` (타격 시 파편 Niagara)
  - `ApplyDamage()` → HP > 0: `MulticastSpawnDamageHitEffect()` + `UpdateDamageVisuals()` (메시 교체 + MID 재생성 + CrackIntensity)
  - HP = 0: 기존 GeometryCollection 파쇄 + DestroyBrick (변경 없음)
  - 배열 비어있으면 메시 교체 스킵, 이펙트 미설정이면 스폰 스킵 (하위 호환)

### Sumo Knockoff 미니게임
원형 플랫폼 PvP 서바이벌. Z 낙하 감지 Eliminate, GA_Push(넉백+킬 추적), 3라운드 시스템, FloorRing(외곽→파괴), PowerUp(Speed/SuperPush/Shield), MapOption(Default/Bridge/Obstacle).

### JumpMap 미니게임
장애물 코스 타임어택. 시간 제한 120초, 체크포인트 리스폰, 완주 순서 추적.
- **장애물**: KillZone, MovingPlatform(서버 시간 동기화 + OriginalLocation 리플리케이션), RotatingObstacle(서버 시간 동기화), PushWind, Checkpoint, End, GrapplePoint. Checkpoint/KillZone/PushWind는 커스텀 프로퍼티에서 BoxExtent 직렬화
- **장애물 동기화**: `ServerElapsedTime` Replicated + `CalculatePositionFromTime()`/`CalculateRotationFromTime()` 순수 함수 → 클라/서버 동일 위치
- **어빌리티**: GA_Dash(Shift), GA_Grapple(E), GA_DoubleJump
- **CSV 레이아웃**: `JumpMapLayoutDataAsset` 내장+유저 로드, `#META:MapName:` 헤더, CustomProperties 11번째 컬럼
- **액터 직렬화**: JumpMapActorBase의 JumpMapObjectId + Get/ApplySerializableProperties. KillZone, MovingPlatform, RotatingObstacle, PushWind, Checkpoint, End, GrapplePoint 7개 서브클래스 모두 구현 완료
- **에디터**: WjWorldEditor 모듈 — JumpMapLevelEditorSubsystem + SJumpMapLayoutPanel

### Gameplay Ability System
`UWjWorldGameplayAbilityBase` — AbilityName/Icon UI 메타, 충전 인터페이스, AllowedAbilityTags 제한, GamePhase 체크. `State_Eliminated`/`State_Staggered` ActivationBlockedTags 공통.
- **AW 어빌리티**: GA_NormalAttack(4방향 벽돌 + 플레이어 경직), GA_SpawnBrick(충전+Preview), GA_LiftBrick(ServerRPC 패턴)
- **NormalAttack 경직**: 피격 플레이어에 `GE_NormalAttackStagger` 적용 (1초 Duration, `State.Staggered` 태그) → 이동+어빌리티 차단, 서버에서 DisableMovement + 타이머 복원
- **Sumo 어빌리티**: GA_Push(넉백+SuperPush), GA_Jump(CharacterJump 패턴)
- **JumpMap 어빌리티**: GA_Dash(LaunchCharacter), GA_Grapple(라인트레이스→당김), GA_DoubleJump(공중 1회)
- **GameplayTag**: `State_*`(Eliminated, Staggered, SpawnBrickPreview, LiftBrickCarry), `Cooldown_*`, `Ability_*`, `Buff_*`, `GameplayCue_*` 접두사 패턴
- **주요 패턴**: Preview+Confirm/Cancel, 클라이언트 그리드좌표→Server RPC (LocalPredicted 위치 불일치 해결)

### 코스메틱 시스템
ItemId(FName) 기반. `ECosmeticSlot`(Head/Body/Back/Effect), 비동기 메시 로드, Steam Inventory 폴링.
- **리플리케이션**: PlayerStateBase → OnRep_CosmeticLoadout → CosmeticComponent.ApplyLoadout()
- **3자 동기화**: CharacterBase.OnRep_PlayerState() → PS->OnPawnSet() → 적용
- **구매**: PurchaseSubsystem → Steam MicroTransaction API → 콜백 → InventoryRefresh
- **테스트**: `Cosmetic_Grant*/Clear*/Print*/Equip*/Unequip` 콘솔 명령어

### Stats 시스템
`WjWorldStatsSubsystem` — Steam User Stats + GConfig 폴백. 네임스페이스 기반(`WjWorldStats::AW/Sumo/JumpMap`). GameStatePlay에서 게임 종료 시 자동 기록.

### 세션 관리
`USessionManager` — Steam OSS 우선 → NULL 폴백. LAN/Steam 모드 분기, 검색 큐, 호스트 마이그레이션.
- **비밀번호 방**: `CreateSession()`에서 PASSWORD 커스텀 설정 저장, `GetSessionPassword()`로 검색 결과에서 추출
- **비밀번호 검증 흐름**: RoomListEntryWidget → `bIsPrivate` 확인 → PasswordInputWidget 팝업 → RoomListWindow.JoinRoomWithPassword() → 클라이언트 사전 검증 → JoinSession
- **PasswordInputWidget**: `UI/Session/` — ShowPopup/ClosePopup 패턴, Enter키 제출, 에러 메시지 표시
- **[Private] 표시**: RoomListEntryWidget에서 비공개 방 이름 앞에 `[Private]` 접두사
- **게임 중 방 노출**: `IN_PROGRESS` 세션 커스텀 설정으로 실제 진행 상태 추적, `UpdateSessionInProgress()` — StartGame/EndGame에서 호출
- **[Playing] 표시**: RoomListEntryWidget에서 진행 중 방 이름 앞에 `[Playing]` 접두사, `bInProgress && !bAllowJoinInProgress` 시 Join 버튼 비활성화
- **중간 입장 관전자**: `GameModePlay::HandleStartingNewPlayer_Implementation()` — Playing/Finished 단계 입장 시 `StartSpectatingOnly()`, GameRule 3종(AW/Sumo/JumpMap)에서 `IsGameInProgress()` 가드로 관전자 참여 추적 방지

### Steam 설정
- **AppID**: 4399350, `WITH_STEAM` 매크로 (Win64), `Steam/itemdefs.json`
- **네트워킹**: Steam=SteamNetDriver, LAN=WjWorldLanNetDriver(`PLATFORM_SOCKETSUBSYSTEM` 명시)
- **Config**: DriverClassName `/Script/ModuleName.ClassName` 정규 경로 필수
- **LAN 소켓 충돌**: SocketSubsystemSteamIP가 기본 소켓 오버라이드 → WjWorldLanNetDriver로 해결
- **코스메틱 DefId 넘버링**: Head 2000~2199, Body 2200~2399, Back 2400~2599, Effect 2600~2799 (200 간격)

### 재화 시스템
`UWjWorldCurrencySubsystem` — Coin/Gem 재화 관리. Steam Inventory 기반 + 비Steam GConfig 폴백.
- **잔액 조회**: `GetBalance()`, `GetAllBalances()` — Steam 환경에서 `GetAllItems` 단일 패스로 잔액 + 인스턴스 ID 동시 캐싱
- **미니게임 보상**: `TriggerMatchReward()` → Steam `TriggerItemDrop` (실패 시 로컬 폴백) + 2.5s/5s 인벤토리 갱신 재시도
- **일일 보상 제한**: `TodayMatchRewardCount` + `LastRewardDate` GConfig 영속, `GetRemainingDailyRewards()`, `MaxDailyMatchRewards` (DeveloperSettings, 기본 10)
- **코스메틱 구매**: `PurchaseItemWithCurrency()` → Steam `ExchangeItems` (캐시된 인스턴스 ID 사용) / 비Steam 로컬 차감
- **유료 재화 팩**: `PurchaseGemPack()` → Steam `StartPurchase` → 오버레이 결제 UI
- **폴링**: Exchange 결과 0.5초, Gem 구매 1초 주기 폴링 + 300초 타임아웃
- **로컬 저장**: `GGameUserSettingsIni` (cross-session 안정)
- **테스트**: `Currency_GrantCoin/Gem`, `Currency_SetCoin/Gem`, `Currency_Print/Refresh`, `Steam_ConsumeCurrency`, `Steam_ConsumeAllItems` 콘솔 명령어

### 보물상자 시스템
`AWjWorldTreasureChestActor` — `AWjWorldPlacedObjectActor` 서브클래스. 로비 배치 상호작용 오브젝트.
- **상호작용**: BoxComponent 오버랩 → EnableInput + EnhancedInput BindAction(F키) → OnInteract
- **보상**: Steam `TriggerItemDrop`(ChestIndex별 독립 generator DefId 300~309, 실패 시 로컬 폴백 + 2.5s/5s 재시도) / 비Steam `GrantCurrencyLocally`
- **쿨타임**: `FDateTime CachedLastOpenedTime` 인메모리 캐시 + `GGameUserSettingsIni` 영속 저장, 위치 해시 키 (`Chest_X_Y_Z`)
- **비주얼**: DMI 어두운 회색 틴트 (쿨타임 중), UI 프롬프트 (InteractionWidget), 뚜껑 Roll 애니메이션
- **ActorClassOverride**: `FPlaceableObjectDefinition`에 스폰 클래스 분기 필드 → GameStateLobby에서 사용
- **DeveloperSettings**: `TreasureChest` 카테고리 (CoinReward, CooldownSeconds, InteractAction, WidgetClass, GeneratorStartDefId)
- **테스트**: `TreasureChest_ClearCooldowns` 콘솔 명령어

### 캐릭터 프리뷰 시스템
`ACharacterPreviewActor` — 프로필/상점 UI용 3D 캐릭터 프리뷰. `UPlayerProfileWidget`, `UCosmeticPreviewPanel`에서 사용.
- **메시 복사**: `SetupFromPawn()` — SkeletalMesh + AnimBlueprint + RelativeRotation 복사 (Yaw=-90 보정 포함)
- **코스메틱 프리뷰**: `SetupPreview()` — 비동기 메시 로드 + Socket 부착 + ShowOnlyList
- **SceneCapture**: `PRM_UseShowOnlyList` + `SCS_FinalColorHDR` + `ClearColor::Transparent` + `bAlwaysPersistRenderingState = true`
- **실시간 캡처**: `bCaptureEveryFrame = true` (SetupFromPawn 완료 후 활성화, Idle 모션 반영)
- **투명 배경**: UI Material(`M_CharacterPreview`)에서 OneMinus로 alpha 반전 → `MaterialInstanceDynamic` → `Image::SetBrushResourceObject(MID)`
- **DeveloperSettings**: `CharacterPreviewMaterial` (UI 카테고리) — Material 경로 관리
- **RenderTarget**: `RTF_RGBA16f` + `InitCustomFormat(500, 1000, PF_FloatRGBA, false)`
- **스폰 위치**: `(0, 0, 15000)` — 월드와 겹치지 않는 상공

### 설정 시스템
`USettingsWidget` — 로비/대기실 설정 팝업. ShowPopup/ClosePopup 패턴.
- **그래픽 품질**: `UGameUserSettings::SetOverallScalabilityLevel()` (Low/Medium/High/Epic), `SaveSettings()` 영속
- **마스터 볼륨**: `GConfig` (`GGameUserSettingsIni`, `[WjWorldSettings]` 섹션, `MasterVolume` 키)
- **볼륨 적용**: `FAudioDeviceHandle::SetTransientPrimaryVolume()` — static `ApplySavedMasterVolume()`
- **즉시 적용**: Apply 버튼 없이 변경 시 바로 반영
- **시작 시 복원**: `GameInstance::Init()` → `ApplySavedMasterVolume()`
- **HUD 연동**: LobbyHUDWidget, WaitingRoomHUDWidget에서 `SettingsWidgetClass`/`SettingsWidgetInstance` 관리

### 채팅 시스템
`UChatWidget` — 멀티플레이어 채팅. HUDBase에서 `bCreateChatWidget=true`인 경우만 생성 (Lobby/WaitingRoom/Play). Intro/Login은 미생성.
- **RPC 흐름**: PlayerControllerBase.SendChatMessage() → Server RPC → GameStateBase.MulticastReceiveChatMessage() → OnChatMessageReceived 델리게이트
- **위젯**: ScrollBox(메시지 목록) + EditableTextBox(입력), Enter 키 전송, `IsChatInputFocused()` API
- **글로벌 Enter 키**: PlayerControllerBase에서 Enter → `ChatWidget.FocusChatInput()` (이미 포커스 중이면 스킵)
- **DeveloperSettings**: `ChatWidgetClass` (UI 카테고리)
- **UMG Blueprint**: `WBP_ChatWidget` (ChatScrollBox, ChatInputBox BindWidget)

### 글로벌 입력 시스템
`PlayerControllerBase::SetupInputComponent()` — Enter/ESC 키 바인딩. 모든 컨텍스트 공용.
- **Enter 키**: `OnEnterPressed()` → HUDBase → ChatWidget → `FocusChatInput()` (이미 포커스 중이면 스킵)
- **ESC 키**: `OnEscapePressed()` → HUDBase → `TryCloseTopPopup()` (가상 함수)
  - Lobby: Settings → Profile → Cosmetic → PlacementContextSelect 순서
  - WaitingRoom: Settings → Profile 순서
  - Play: `OnEscapePressed()` override → LeaveDialog 토글 (기존 동작 유지)
- **HUD 위임 패턴**: `AWjWorldHUDBase::TryCloseTopPopup()` → HUD 서브클래스 override → 위젯 인스턴스 `TryCloseTopPopup()` 위임

### Coin 획득 알림 시스템
`UCoinGainNotificationWidget` — "+X Coin" 토스트 표시. HUDBase에서 생성.
- **구독**: OnCurrencyBalanceChanged 델리게이트 (CurrencySubsystem)
- **표시**: 이전 잔액 캐시 → 델타 계산 → 양수면 금색 텍스트 3초 표시
- **DeveloperSettings**: `CoinGainNotificationWidgetClass` (UI 카테고리)
- **UMG Blueprint**: `WBP_CoinGainNotification` (NotificationText BindWidget)

### WjWorldDeveloperSettings
Project Settings > Game > WjWorld. 맵 경로, GameMode 클래스, 캐릭터 기본값, 미니게임 에셋, 배치 카탈로그, 카메라 InputAction, 보물상자 설정, MaxDailyMatchRewards, CharacterPreviewMaterial.
**설정 우선순위**: BP 서브클래스 UPROPERTY 값 우선 → DeveloperSettings 폴백

### 패키징 주의사항
- 새 레벨/맵 → Packaging > maps list에 추가 필수
- Non-asset 파일(.csv 등) → `DefaultGame.ini` `DirectoriesToAlwaysStageAsNonUFS`
- FFilePath → 패키지 빌드에서 `FPaths::ProjectContentDir()` 기준 변환

## 진행 중 / 미구현
- Steam 정식 출시 준비
- 솔로 컨텐츠 기획 필요: 봇 시스템, 솔로 모드, 또는 싱글 미니게임 추가
- Skeletal mesh 코스메틱 확장: 코드 인프라 준비 완료, 에셋 제작 + itemdefs.json 등록 필요

## 출시 전 체크리스트
- (현재 없음)

## 잔존 버그
- (현재 없음)

## 확인 필요 사항
- Room 목록 스케일링 — 1000+ 방 표시 시 부하 체크
- Sumo FloorRing 디자인 변경 검토 — 개별 타일 랜덤 파괴 전환 시 리플리케이션 비용
- 에셋 커밋 전략 수립 — LFS 정책, 브랜치 전략, 에셋 전용 커밋 분리
- BP_WaitingRoomHUDWidget에서 ProfileWidgetClass 설정 확인 — 3자 프로필이 안 보이는 문제 (코드 로직 정상)

## 코딩 컨벤션
- 언리얼 엔진 코딩 표준 준수
- 클래스 접두사: `A` (Actor), `U` (UObject), `F` (구조체)
- 프로젝트 접두사: `WjWorld`
- 한글 주석 사용 가능

## 빌드 명령어
- Visual Studio에서 F5 (DebugGame Editor)
- `Batch/`: GenerateProjectFiles, OpenSolution, PackageDebugGame, PackageAndUploadSteam, RebuildProject, RunDebugEditor, GenerateDocs

## 게임 플로우
```
인트로 → 로그인 → 로비 → 방 생성(OpenLevel Lobby?game=WaitingRoom?Listen) → 대기실
    ↓
ServerTravel(PlayMap?game=GameModePlay?GameModeId=xxx?MapOption=yyy)
    ↓
GameModePlay: MinigameCatalog → GameRule 생성 → OnGameReady → 카운트다운 → OnGameStart
    ↓
TickGameRule → CheckWinCondition → OnGameEnd → 스탯 기록 → ServerTravel → 대기실 복귀
```

### 맵 전환 URL 패턴
- **방 생성**: `OpenLevel("/Game/Map/02-1_Lobby?game=BP_GameModeWaitingRoom_C?Listen")`
- **게임 시작**: `ServerTravel("{LevelPath}?game=BP_GameModePlay_C?GameModeId={id}?MapOption={opt}")`
- **게임 종료**: `ServerTravel("/Game/Map/02-1_Lobby?game=BP_GameModeWaitingRoom_C")`
