# WjWorld

언리얼 엔진 5.7으로 개발하는 개인 C++ 프로젝트

## 프로젝트 개요

허브 공간, 미니게임, 멀티플레이어 기능을 갖춘 개인 학습 프로젝트입니다.
Steam에 무료로 출시하며 코스메틱 아이템을 유료 판매합니다.

### 개발 목표
1. **허브 공간** - 플레이어가 컨텐츠로 진입할 수 있는 로컬 공간
2. **미니게임** - 다양한 장르의 미니게임 구현 (GameRule 시스템 기반)
3. **멀티플레이어** - 기본적인 네트워킹 기능 구현
4. **코스메틱 & 수익화** - Steam Inventory 연동 코스메틱 판매 시스템
5. **융합 컨텐츠** - 로컬과 멀티플레이어 요소 결합

## 개발 환경

- **엔진**: Unreal Engine 5.7
- **언어**: C++
- **IDE**: Visual Studio 2022
- **버전 관리**: Git
- **문서화**: Doxygen
- **배포 플랫폼**: Steam (Win64)

## 소스 구조

```
Source/WjWorld/
├── AbilitySystem/                     # Gameplay Ability System
│   ├── Abilities/                     # 어빌리티 (NormalAttack, SpawnBrick, LiftBrick)
│   ├── AttributeSets/                 # 어트리뷰트 셋 (HP, 충전 등)
│   ├── Effects/                       # GameplayEffect (쿨다운, 충전 비용)
│   └── WjWorldAbilitySystemComponent  # ASC 컴포넌트
├── Core/                              # 핵심 게임 로직
│   ├── Base/                          # 베이스 클래스들
│   ├── Intro/                         # 인트로 화면
│   ├── Login/                         # 로그인
│   ├── Local/                         # 로컬 게임모드
│   │   ├── Lobby/                     # 로비/허브
│   │   └── WaitingRoom/               # 대기실
│   ├── Play/                          # 게임플레이 모드 (미니게임 공통)
│   ├── GameRule/                      # 미니게임 규칙 시스템
│   ├── GameData/                      # 게임 데이터 컴포넌트
│   ├── Components/                    # 게임플레이 헬퍼 컴포넌트
│   ├── Session/                       # 세션 관리
│   └── WjWorldGameInstance
├── Cosmetic/                          # 코스메틱 및 구매 시스템
│   ├── WjWorldCosmeticTypes           # 타입 정의 (슬롯, 로드아웃, 아이템 인스턴스)
│   ├── WjWorldCosmeticComponent       # 캐릭터 비주얼 적용 (비동기 에셋 로드)
│   ├── WjWorldCosmeticSubsystem       # 인벤토리/로드아웃 관리
│   ├── WjWorldCosmeticDataAsset       # 아이템 카탈로그 (ItemId ↔ SteamItemDefId)
│   └── WjWorldPurchaseSubsystem       # Steam 마이크로트랜잭션 구매
├── Stats/                             # 플레이어 스탯 시스템
│   ├── WjWorldStatsSubsystem          # Steam User Stats 래핑
│   └── WjWorldStatTypes               # 스탯 타입 정의
├── Setting/                           # 개발자 설정
│   └── WjWorldDeveloperSettings       # BP 설정용 DeveloperSettings
├── DataAsset/                         # 데이터 에셋
├── GamePlay/                          # 게임플레이 시스템
│   ├── Camera/                        # 카메라 시스템
│   ├── Interact/                      # 상호작용
│   ├── Quest/                         # 퀘스트 시스템
│   └── Wall/                          # Approaching Wall 미니게임
├── Network/                           # 네트워크/패킷 관련
└── UI/                                # UI 위젯들
    ├── Ability/                       # 어빌리티 UI (슬롯, 프롬프트)
    ├── Cosmetic/                      # 코스메틱 UI (상점, 인벤토리, 프리뷰)
    ├── Profile/                       # 플레이어 프로필 (3D 프리뷰 + 스탯)
    ├── HUD/                           # 게임플레이 HUD
    └── ...                            # Intro, Login, Lobby, Session, WaitingRoom 등
```

## 주요 클래스 계층

### GameMode
```
AWjWorldGameModeBase
├── AWjWorldGameModeIntro          # 인트로 화면
├── AWjWorldGameModeLogin          # 로그인
├── AWjWorldGameModeLobby          # 로비/허브
├── AWjWorldGameModeWaitingRoom    # 대기실
└── AWjWorldGameModePlay           # 게임플레이 (미니게임)
```

### Character
```
AWjWorldCharacterBase
├── AWjWorldCharacterLobby
├── AWjWorldCharacterWaitingRoom
└── AWjWorldCharacterPlay          # 게임플레이 캐릭터 (ASC + CosmeticComponent)
```

### PlayerController
```
AWjWorldPlayerControllerBase
├── AWjWorldPlayerControllerLobby
├── AWjWorldPlayerControllerWaitingRoom
└── AWjWorldPlayerControllerPlay
```

### GameRule (미니게임 규칙)
```
UWjWorldGameRuleBase
└── UWjWorldGameRuleApproachingWall   # Approaching Wall 미니게임
```

### Gameplay Ability
```
UWjWorldGameplayAbilityBase
├── UGA_NormalAttack               # 4방향 벽돌 공격
├── UGA_SpawnBrick                 # 충전 기반 벽돌 배치
└── UGA_LiftBrick                  # 벽돌 이동/재배치
```

### Subsystem (GameInstanceSubsystem)
```
UGameInstanceSubsystem
├── UWjWorldCosmeticSubsystem         # 인벤토리/로드아웃 관리
├── UWjWorldPurchaseSubsystem         # Steam 구매 처리
└── UWjWorldStatsSubsystem            # 플레이어 스탯 관리
```

### UI Widget
```
UWjWorldUserWidgetBase
├── UIntroWindow
├── ULoginWindow
├── ULobbyHUDWidget
├── UWaitingRoomHUDWidget
├── UGameplayGlobalHUDWidget        # 게임플레이 HUD
├── UApproachingWallHUDWidget       # Approaching Wall 전용 HUD
├── UAbilitySlotWidget              # 어빌리티 슬롯 (쿨다운/충전)
├── UAbilityPromptWidget            # Confirm/Cancel 프롬프트
├── UPlayerProfileWidget            # 플레이어 프로필 (3D 프리뷰 + 스탯)
├── UCreateRoomWindow
├── URoomListWindow
└── UInteractionWidget
```

## 핵심 시스템

### GameRule 시스템
미니게임을 정의하기 위한 규칙 시스템. 각 미니게임은 `UWjWorldGameRuleBase`를 상속받아 구현합니다.

**라이프사이클:**
```
Initialize() → OnGameReady() → OnGameStart() → Tick() → OnGameEndPredict() → OnGameEnd()
```

**주요 기능:**
- 플레이어 이벤트 (`OnPlayerJoined`, `OnPlayerLeft`)
- 승리 조건 (`CheckWinCondition`, `GetWinner`)
- 프레임별 업데이트 (`FTickableGameObject`)

### GameData 컴포넌트 시스템
GameplayTag 기반 타입 세이프 데이터 저장 시스템.
- `GameStatePlay`에 게임 전체 데이터
- `PlayerStatePlay`에 플레이어별 데이터
- 리플리케이션 지원

### Approaching Wall 미니게임
첫 번째 미니게임. 벽이 점진적으로 다가오며 플레이어들이 안전 구역으로 이동해야 하는 PvP 서바이벌 게임.

**게임 규칙:**
- 데이터 에셋 기반 벽돌 맵 생성
- 12초마다 레벨업, 벽이 안쪽으로 이동
- 이동 시간: 5초 → 1초 (10레벨 동안 점진적 감소)
- Flood Fill 알고리즘으로 안전 구역 계산
- 타일 폭탄 신호 시스템 (3초 차징, 노랑→빨강 색상 전환)

**주요 클래스:**
- `WjWorldBrickSpawner` - 비동기 벽돌 스폰 (8개/틱)
- `WjWorldBrickMovement` - 개별 벽돌 이동 로직
- `WjWorldWallManager` - 벽 이동 진행 관리
- `WjWorldTileActor` - 안전 구역 타일 (폭탄 신호, 방향별 오버랩)
- `WjWorldBrickPreviewActor` - 어빌리티 배치 프리뷰 (유효/무효 색상)
- `WjWorldWallDescriptionDataAsset` - 벽 레이아웃 데이터

### Gameplay Ability System
GAS 기반 어빌리티 시스템. `UWjWorldGameplayAbilityBase`에서 공통 기능 제공.

**공통 기능:**
- AbilityName, AbilityIcon (UI 메타데이터)
- 충전 시스템 인터페이스 (IsChargeBased, GetCurrentCharges, GetMaxCharges)
- GetPromptDescription() (어빌리티별 프롬프트 텍스트)

**어빌리티:**
- `GA_NormalAttack` - 4방향 스냅(Yaw 기반) 벽돌 공격, BrickType별 처리
- `GA_SpawnBrick` - 충전 기반 벽돌 배치, Preview → Confirm/Cancel 패턴
- `GA_LiftBrick` - 벽돌 재배치, Cancel 시 원래 위치 복원

**어트리뷰트:** HP, MaxSpawnBrickCharges, SpawnBrickCharges
**이펙트:** GE_AbilityCooldown (쿨다운), GE_SpawnBrickChargeCost (충전 비용)

### 코스메틱 시스템
Steam 무료 출시 후 유료 코스메틱 판매를 위한 시스템.

**아키텍처:**
- `ItemId`(FName) 기반 플랫폼 독립 식별 → `SteamItemDefId`(int32) 매핑
- `CosmeticCatalogDataAsset`으로 아이템 정의 (메시, 아이콘, 가격, 슬롯, 희귀도)
- `CosmeticSubsystem`으로 인벤토리 캐시 및 로드아웃 관리, Steam Inventory 폴링 콜백
- `CosmeticComponent`로 캐릭터에 비동기 메시 적용
- `FCosmeticLoadout` (`TArray<FCosmeticSlotEntry>`) 기반 네트워크 리플리케이션

**슬롯 종류:** Head, Body, Back, Effect

**리플리케이션 흐름:**
```
[서버] Character.PossessedBy() → PlayerStateBase.OnPawnSet() → CosmeticComponent.ApplyLoadout()
[클라이언트] CharacterBase.OnRep_PlayerState() → PS->OnPawnSet() → 3자 캐릭터 포함 동기화
```

**테스트 콘솔 명령어:**
- `Cosmetic_GrantAll`, `Cosmetic_PrintInventory`, `Cosmetic_Equip/Unequip`

### 구매 시스템
- `PurchaseSubsystem`을 통한 Steam MicroTransaction API 연동
- 구매 상태 관리 (Idle → Pending → Completed/Failed)
- 성공 시 인벤토리 자동 갱신

### Stats 시스템
Steam User Stats 래핑 + GConfig 폴백 (비Steam 빌드용).

**주요 기능:**
- 로컬/원격 플레이어 스탯 읽기/쓰기
- 미니게임별 스탯 자동 기록 (승/패/킬/게임 수)
- 네임스페이스 기반 스탯 구조 (`WjWorldStats::ApproachingWall::Wins` 등)
- 비동기 원격 플레이어 스탯 조회

### 플레이어 프로필 시스템
- `PlayerProfileWidget` - 3D 캐릭터 프리뷰 + 미니게임별 스탯 표시
- `CharacterPreviewActor` - SceneCaptureComponent2D 기반 오프스크린 3D 렌더링 (256x512)
- 비동기 코스메틱 메시 로드 및 스탯 로드

### 어빌리티 UI
- `AbilitySlotWidget` - 어빌리티 아이콘, 키바인딩, 쿨다운/충전 오버레이 표시
- `AbilityPromptWidget` - Confirm/Cancel 키 이름 + 어빌리티 설명 표시 (WidgetComponent)

### Steam 빌드 설정
- `WITH_STEAM` 매크로로 조건부 컴파일 (Win64 전용)
- Steamworks, OnlineSubsystemSteam 모듈
- OnlineSubsystemSteam 플러그인 활성화
- Steam API 호출은 `#if WITH_STEAM` 블록으로 분리

## 빌드 방법

### 필수 요구사항
- Visual Studio 2022 (C++ 개발 도구 포함)
- Unreal Engine 5.7
- Windows 10/11 SDK
- Steamworks SDK (Steam 빌드 시)

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

### 배치 파일 (Batch/)
- `GenerateProjectFiles.bat` - 프로젝트 파일 생성
- `OpenSolution.bat` - VS 솔루션 열기
- `PackageDebugGame.bat` - 디버그 게임 패키징
- `RebuildProject.bat` - 전체 리빌드
- `RunDebugEditor.bat` - 디버그 에디터 실행
- `GenerateDocs.bat` - Doxygen 문서 생성
- `SetEnvironmentVariable.bat` - 환경 변수 설정

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
게임 시작 → GameModePlay 진입
    ↓
GameRule 초기화 → 카운트다운 → 게임 시작
    ↓
게임 진행 (레벨업, 벽 이동, 어빌리티 사용)
    ↓
승리 조건 체크 → 결과 표시 → 스탯 자동 기록
    ↓
대기실 복귀
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
- [x] **게임플레이 모드 프레임워크** (Play 클래스 세트)
- [x] **GameRule 시스템** (미니게임 규칙 정의)
- [x] **GameData 컴포넌트 시스템** (게임/플레이어 데이터)
- [x] **Ability System Component 통합**
- [x] **Approaching Wall 기본 구조**
  - [x] 데이터 에셋 기반 벽돌 맵 스폰
  - [x] 벽돌 이동 로직 (경로 탐색)
  - [x] 레벨 시스템 (12초 간격, 10레벨)
  - [x] 안전 구역 축소 알고리즘
  - [x] 타일 폭탄 신호 시스템 (3초 차징, 색상 전환)
  - [x] 게임플레이 HUD (카운트다운, 결과 표시, GameRule별 HUD 매핑)
  - [x] 플레이어 사망/제거 로직 (bIsEliminated 리플리케이션)
- [x] **게임플레이 어빌리티**
  - [x] GA_NormalAttack (4방향 벽돌 공격)
  - [x] GA_SpawnBrick (충전 기반 벽돌 배치, Preview + Confirm/Cancel)
  - [x] GA_LiftBrick (벽돌 재배치, Cancel 시 복원)
  - [x] 충전 시스템 (어트리뷰트 기반, GE 리필)
  - [x] 벽돌 프리뷰 시스템 (유효/무효 색상)
- [x] **어빌리티 UI**
  - [x] AbilitySlotWidget (아이콘, 키바인딩, 쿨다운, 충전 표시)
  - [x] AbilityPromptWidget (Confirm/Cancel 프롬프트)
- [x] **코스메틱 시스템**
  - [x] 코스메틱 타입 정의 (슬롯, 로드아웃, 아이템 인스턴스)
  - [x] 코스메틱 컴포넌트 (비동기 에셋 로드, 슬롯별 메시 관리)
  - [x] 코스메틱 서브시스템 (인벤토리 캐시, 로드아웃, 로컬 저장)
  - [x] 코스메틱 카탈로그 데이터 에셋 (ItemId ↔ SteamItemDefId)
  - [x] FCosmeticLoadout 리플리케이션 (PlayerStateBase, 모든 모드)
  - [x] Steam Inventory 폴링 콜백 시스템
  - [x] 코스메틱 상점 UI (CosmeticMainWindow, 장착/해제/구매)
  - [x] 멀티플레이어 3자 코스메틱 동기화 (OnRep_PlayerState)
  - [x] 테스트 콘솔 명령어 (Cosmetic_*)
- [x] **구매 시스템** (PurchaseSubsystem, Steam MicroTransaction 연동)
- [x] **스탯 시스템**
  - [x] WjWorldStatsSubsystem (Steam User Stats + GConfig 폴백)
  - [x] 미니게임별 스탯 자동 기록 (승/패/킬)
  - [x] 비동기 원격 플레이어 스탯 조회
- [x] **플레이어 프로필**
  - [x] PlayerProfileWidget (3D 프리뷰 + 스탯 표시)
  - [x] CharacterPreviewActor (SceneCaptureComponent2D)
- [x] **Steam 빌드 설정** (조건부 컴파일, 플러그인, 모듈)
- [x] **개발자 설정** (WjWorldDeveloperSettings)
- [x] **로그 카테고리** (LogWjWorld, LogWjWorldAbilities, LogWjWorldCosmetic, LogWjWorldStats)
- [x] **로비 배치 시스템** (PlacementComponent, 저장/로드, 멀티플레이어 리플리케이션)
- [x] **코스메틱 미리보기/시착 시스템**
  - [x] CharacterPreviewActor Socket 기반 메시 부착
  - [x] StaticMesh/SkeletalMesh 동시 지원
  - [x] 다중 슬롯 시착 유지 (슬롯 전환 시 리셋 안 함)
  - [x] Pawn에서 SkeletalMesh/AnimBP 복사
- [x] **하드코딩 경로 제거 및 DeveloperSettings 중앙화**
  - [x] 맵 경로 (LobbyMapPath)
  - [x] GameMode 클래스 (WaitingRoom, Play)
  - [x] 캐릭터 기본 메시/애니메이션
  - [x] Approaching Wall 에셋 (Brick, Tile, WallDescription)
  - [x] ConstructorHelpers 제거 → UPROPERTY + DeveloperSettings 폴백 패턴

### 진행 중
- [ ] **Approaching Wall 완성**
  - [ ] 승리 조건 (최후 생존자)
  - [ ] 게임 결과 처리 및 대기실 복귀

### 예정
- [ ] Steam 실제 환경 테스트 (AppID 발급 후)
- [ ] 추가 미니게임 구현

## 문서화

Doxygen으로 생성된 API 문서는 아래 링크에서 확인할 수 있습니다:
- **온라인 문서**: https://shimwoojin.github.io/WjWorld/
- **로컬 문서**: `docs/` 폴더

## 라이선스

개인 학습 프로젝트

---

**시작일**: 2025.08.05
