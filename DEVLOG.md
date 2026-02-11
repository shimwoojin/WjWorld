# WjWorld 개발 로그

## 2026-02-11
### 작업 내용 - 레이아웃 삭제 기능 + AW 버그 3건 수정

#### 레이아웃 삭제 기능 구현
- `WjWorldPlacementComponent::DeleteLayoutSlot()` 추가 — SaveGame + 컨텍스트별 CSV 파일 삭제
- `PlacementLoadDialogWidget` X 삭제 버튼 추가 — HorizontalBox 레이아웃 [슬롯이름(Fill) | X(Auto)]
- `PlacementHUDWidgetBase::OnSlotDeleteRequested()` 핸들러 — 삭제 후 다이얼로그 내 슬롯 목록 인플레이스 갱신
- `PlacementSaveDialogWidget` 슬롯 유효성 표시 개선

#### Bug 1: BrickMovement 대각선 이동 3-4칸 → 1칸 수정
- **원인**: `GetMovementVector()`에서 `GetNextDirections()`가 반환한 모든 방향을 for 루프로 전부 적용
- **수정**: 하나의 방향만 선택, 이전 이동 방향과 일치하는 방향 우선 (관성 유지)

#### Bug 2: 클라이언트 프리뷰 offset 50,50 수정
- **원인**: 클라이언트에서 유저 CSV 파일 부재 → 잘못된 WallDesc(CenterOffset)로 폴백
- **수정**: `ApproachingWallGameDataComponent`에 `WallBrickSize/WallCenterOffset/WallColumnNum/WallRowNum` 리플리케이트 추가
- `OnWallSpawnFinished()`에서 설정, GA_SpawnBrick/GA_LiftBrick에서 리플리케이트된 값으로 보정

#### Bug 3: 클라이언트 GA_LiftBrick 1프레임 취소 수정
- **원인**: 서버가 독립적으로 `CalculatePickupLocation()` 실행 시 네트워크 지연으로 다른 위치 계산 → 벽돌 미발견 → `EndAbility(replicate)` 취소
- **수정**: `ServerLiftBrickAtGridIndex` Server RPC 패턴 적용 (GA_SpawnBrick의 `ServerSpawnBrickAtGridIndex`와 동일)
  - 클라이언트: 그리드 인덱스 계산 → RPC 전송
  - 서버: 클라이언트 지정 인덱스로 벽돌 탐색/파괴 (`ServerHandleBrickPickup`)
  - `HasAuthority()` 블록 제거 → RPC 핸들러로 이전

### 학습/메모
- `LocalPredicted` 어빌리티에서 서버가 avatar 위치/회전을 기반으로 독립 판단하면 네트워크 지연으로 클라이언트와 불일치 발생 → 클라이언트가 계산한 결과를 Server RPC로 전달하는 패턴이 안정적
- `FIntPoint`에는 `IsZero()` 메서드가 없음 → `(X != 0 || Y != 0)` 으로 체크
- `BrickMovement::GetNextDirections()`는 FloodFill 경계점의 8방향 인접 셀을 모두 반환 — 이동 시 반드시 하나만 선택해야 함

### 이슈/해결
- 잔존 버그: #3(Sumo) Host 관전 Yaw, #4(Sumo) 유저 맵 클라이언트 벽돌 스폰 위치 — 미해결
- JumpMap 맵 레벨 생성 + 패키징 맵 목록 추가 필요

---

## 2026-02-10
### 작업 내용 - JumpMap 미니게임 전체 구현

#### JumpMap C++ 코드 구현 (Agent Teams 4병렬)
- GameRule: `WjWorldGameRuleJumpMap` — 시간 제한, 체크포인트 리스폰, 완주 추적, Z 낙하 감지
- GameData: `JumpMapGameDataComponent` (ElapsedTime, PlayerFinishOrder), `JumpMapPlayerDataComponent` (Checkpoint, DeathCount)
- 장애물 액터 7종: KillZone, MovingPlatform, RotatingObstacle, PushWind, Checkpoint, End, GrapplePoint
- 레이아웃: `JumpMapLayoutDataAsset` — 내장+유저 CSV 파싱 (`#META:MapName:` 지원)
- 어빌리티 3종: GA_Dash (전방 대시), GA_Grapple (라인트레이스→당김), GA_DoubleJump (공중 점프)
- HUD: `JumpMapHUDWidget` — 타이머, 체크포인트, 사망 횟수, 순위표
- 통합: WjTypes(Ability8/9/10), GameplayTags(5개), Stats(JumpMap 네임스페이스), DeveloperSettings, PlacementTypes

#### 코드 리뷰 중 버그 수정 (4건)
- KillZoneActor: `Character->OnEliminated()` → `GameRule->OnPlayerDied()` (영구 제거→체크포인트 리스폰)
- EndActor: GameRule 호출 누락 → `GameRule->OnPlayerFinished()` 추가
- RotatingObstacleActor: 킬 모드에서 동일 수정
- CheckpointActor: PlayerData 갱신 누락 → `SetCurrentCheckpointIndex()` + 역주행 방지 로직

#### 에디터 세팅 완료
- DA_MinigameCatalog에 JumpMap 등록
- DA_CharacterPlaySetup에 Dash/Grapple/DoubleJump 바인딩
- IMC_Default에 Shift(Dash)/E(Grapple) 입력 매핑
- BP_HUDPlay에 JumpMapHUDWidget 매핑

#### 태그/InputID 정리
- DefaultGameplayTags.ini에 5개 태그 등록 (Ability.Dash/Grapple/DoubleJump, Cooldown.Dash/Grapple)
- WjTypes.h에 Ability10(DoubleJump) 추가
- GA_Dash/Grapple/DoubleJump → `WjWorldGameplayTag::` 헬퍼 사용으로 통일

### 학습/메모
- Agent Teams 병렬 구현은 빠르지만, 기존 패턴(GameRule->OnPlayerDied vs Character->OnEliminated) 착오가 다수 발생 → 반드시 코드 리뷰 필요
- GA_DoubleJump의 CanActivateAbility: 부모(GA_Jump)의 CanJump() 우회를 위해 조부모(UWjWorldGameplayAbilityBase) 직접 호출 패턴 사용

---

## 2026-02-09
### 작업 내용 - 배치 에디터 BP 세팅 완료 + Steam 2PC 잔존 버그 확인

#### 배치 에디터 BP 세팅 완료
- 에디터 맵 생성
- BP_PlacementSaveDialogWidget, BP_PlacementLoadDialogWidget 생성
- 컨텍스트별 카탈로그 DataAsset 설정

#### LobbyHUDWidget 정리
- DirectConnectButton / OnDirectConnectClicked 제거
- FindRoomButton null 접근 버그 수정 (CreateRoomButton if 블록 안에서 null 체크 없이 접근)

#### Steam 2PC 테스트 — 잔존 버그 확인
- **#3 대각선 맵 movement** — 대각선 연결 맵에서 movement가 wall closed하게 움직이지 않음 (미해결)
- **#4 클라이언트 벽돌 preview offset** — 유저 커스텀 맵에서 50,50 어긋남 (미해결)
- **#11 3자 프로필 조회** — 타 플레이어 프로필 조회 실패 (미해결)
- **#3(Sumo) Host 관전 Yaw 미적용** — Host가 클라이언트 관찰 시 Yaw 미적용 (미해결)
- **#4(Sumo) 유저 맵 벽돌 스폰 위치** — 유저 AW 맵에서 클라이언트 벽돌 엉뚱한 위치 (미해결)

#### TODO
- Lobby HUD 설정 버튼에 그래픽 상/중/하 추가 (GPU 사용량 대비 간단한 설정 필요)

#### 확인 필요
- Room 목록 스케일링 (Steam 배포 시 다수 방 표시 및 부하)
- Sumo FloorRing 레벨 디자인 변경 (원형 축소 → 개별 타일 랜덤 파괴) 시 리플리케이션 비용

---

## 2026-02-07
### 작업 내용 - Steam 4차 버그 수정 + 코드 검증 + Agent Teams 테스트

#### 버그 수정 (커밋 263031b)

##### [해결] GamePhase 어빌리티 제한
- **증상**: 게임 시작 전/종료 후에도 어빌리티 사용 가능
- **수정**: `CanActivateAbility()`에서 `GamePhase != Playing` 체크 추가
- **파일**: `WjWorldGameplayAbilityBase.cpp`

##### [해결] 유저 맵 클라이언트 벽돌 스폰 위치 오류
- **증상**: 유저 커스텀 맵에서 클라이언트 벽돌이 엉뚱한 위치에 스폰
- **수정**: `GetWallDescriptionByName` → `GetWallDescriptionByNameIncludingUser`로 변경
- **파일**: `GA_SpawnBrick.cpp`, `GA_LiftBrick.cpp`

##### [해결] BrickComponent collision 분리
- **증상**: TileActor overlap 감지 실패 (나이아가라 근거없이 출력)
- **수정**: BrickMeshComponent를 QueryOnly+Overlap으로, BlockingCollisionComponent(95%)는 BlockAll로 분리
- **파일**: `WjWorldBrickComponent.cpp`

##### [해결] WaitingRoom 호스트 설정 UI
- **증상**: 호스트 설정 패널이 클라이언트에도 표시 + Apply 후 Display 미갱신
- **수정**: 패널 호스트 전용 표시 + Apply 후 `UpdateRoomInfo()` 명시적 호출
- **파일**: `WaitingRoomHUDWidget.cpp`

##### [해결] 3자 프로필 스탯 조회 실패
- **증상**: 타 플레이어 프로필 스탯이 "Loading..." 상태로 멈춤
- **원인**: `RequestUserStats()` 반환값 무시, CCallResult 미등록
- **수정**: `CCallResult<UWjWorldStatsSubsystem, UserStatsReceived_t>` 패턴 적용
- **파일**: `WjWorldStatsSubsystem.h/cpp`

##### [해결] ParseWallLayout 메타데이터 파싱 누락
- **증상**: 유저 커스텀 맵 preview offset 어긋남
- **수정**: `#META:CenterOffset:` 주석 라인 파싱 로직 추가
- **파일**: `WjWorldWallDescriptionDataAsset.cpp`

##### [해결] 제거 시 관전 전환 없음
- **증상**: 제거 후 화면 멈춤 (관전 시스템 미구현)
- **수정**: `HandleEliminationEffects()`에서 살아있는 플레이어로 `SetViewTargetWithBlend()` 전환
- **파일**: `WjWorldCharacterPlay.cpp`

##### [해결] Lobby HUD 정리
- **수정**: FindRoomButton 숨김 처리 (BindWidgetOptional), 그래픽 품질 사이클 설정 구현
- **파일**: `LobbyHUDWidget.h/cpp`

#### GAS 버그 수정 (커밋 0d323ff)

##### [해결] GA_Jump Super::ActivateAbility() 누락
- **증상**: LocalPredicted 정책에서 Prediction Key 생성 실패 가능
- **수정**: `Super::ActivateAbility()` 호출 추가
- **파일**: `GA_Jump.cpp`

#### 코드 검증 (4개 병렬 subagent)
- **네트워크 리플리케이션**: 문제 없음 (20+ 파일 검증)
- **GAS 어빌리티**: GA_Jump Super 누락 발견 → 즉시 수정
- **GameRule 라이프사이클**: 문제 없음 (프로덕션 레벨)
- **null 포인터/메모리 안전성**: BrickComponent GetGameModePlay() null 체크 권장 (실제 크래시 확률 낮음)

### 학습/메모
- **Claude Code Agent Teams**: Opus 4.6에서 실험적 기능으로 추가. `CLAUDE_CODE_EXPERIMENTAL_AGENT_TEAMS=1` + `teammateMode` 설정 필요
  - `"tmux"`: split-pane 모드 (Windows 미지원)
  - `"in-process"`: 메인 터미널에서 실행 (Windows 권장)
  - Subagent vs Teams: 독립 작업은 subagent가 효율적, 상호 소통 필요한 대규모 작업은 teams
  - 자동 팀 구성은 불가 - 명시적 요청 필요
- **Steam CCallResult 패턴**: `SteamAPICall_t` 반환값을 `CCallResult<>.Set()`에 등록해야 콜백이 호출됨. 단순 함수 호출만으로는 비동기 콜백 미동작

### 이슈/해결
- settings.local.json `teammatemode` → `teammateMode` (camelCase) 오타 수정
- Background agent 출력 파일이 빈 파일로 생성되는 현상 → resume로 결과 확인 가능

---

## 2026-02-06
### 작업 내용 - Steam 2PC 버그 수정 (3차 - 전체 해결)

#### 버그 수정 완료

##### [해결] [Critical] 클라이언트 벽돌 스폰 안 됨
- **증상**: 클라이언트에서 GA_SpawnBrick 사용 시 벽돌이 서버에 스폰되지 않음
- **원인**: UObject(UGameplayAbility)에서 Server RPC 호출 불가 - AActor에서만 가능
- **수정**: `AWjWorldCharacterPlay::ServerSpawnBrick_Implementation()` 추가, GA에서 Character RPC 호출
- **파일**: `WjWorldCharacterPlay.h/cpp`, `GA_SpawnBrick.cpp`

##### [해결] [Critical] #14 호스트 설정 패널 값 반영 안 됨
- **증상**: 게임모드/맵 ComboBox 선택해도 게임 시작 시 반영 안 됨
- **원인**: ComboBox 선택 후 "Apply Settings" 버튼을 눌러야만 저장됨 → 사용자가 누르지 않고 바로 게임 시작
- **수정**: `OnStartGameClicked()`에서 `ApplyCurrentUISettings()` 자동 호출
- **파일**: `WaitingRoomHUDWidget.cpp`

##### [해결] #2 호스트 설정 패널 클라이언트 표시
- **증상**: 패널이 호스트만 보이도록 숨겨짐 (클라이언트 설정 확인 불가)
- **수정**: 패널은 전체 표시, 클라이언트는 ComboBox disabled + Apply 버튼 숨김
- **파일**: `WaitingRoomHUDWidget.cpp` (UpdateHostSettingsPanelVisibility)

##### [해결] #11 3자 프로필 조회 안 됨
- **증상**: 플레이어 버튼 클릭 시 프로필이 안 열림
- **원인**: `IsHovered()` 버튼 클릭 후 unreliable
- **수정**: `PlayerButtonToIDMap` (TMap<UButton*, int32>) 추가, `IsHovered() || HasMouseCapture()` 체크
- **파일**: `WaitingRoomHUDWidget.h/cpp`

##### [해결] #1 WaitingRoom UI 미갱신
- **증상**: 호스트 설정 변경 시 UI 텍스트가 업데이트 안 됨
- **원인**: GameState 참조 타이밍 이슈
- **수정**: `UpdateRoomInfo(const FRoomSettings* InSettings = nullptr)` 옵셔널 직접 전달
- **파일**: `WaitingRoomHUDWidget.h/cpp`

##### [해결] #4 유저 커스텀 맵 preview offset
- **증상**: 유저 레이아웃에서 클라이언트 벽돌 preview 위치가 50,50만큼 어긋남
- **원인**: CSV 내보내기 시 GridOrigin 계산했으나 로드 시 CenterOffset=ZeroVector
- **수정**: CSV에 `#META:CenterOffset:x,y,z` 메타데이터 헤더 추가 및 파싱
- **파일**: `WjWorldPlacementComponent.cpp`, `WjWorldWallDescriptionDataAsset.cpp`

##### [해결] #8 TileActor collision 옆 칸 영향
- **증상**: 4방향 벽돌로 갇힌 타일의 Bomb()이 옆 타일 캐릭터에게도 영향
- **원인**: `SetBoxExtent(InSize)` → Half extent에 전체 크기 전달 (박스가 2배 커짐)
- **수정**: `SetBoxExtent(InSize * 0.5f)` + 방향별 HitBox 위치도 `InSize * 0.5f`로 수정
- **파일**: `WjWorldTileActor.cpp` (InitializeTile)
```cpp
const FVector HalfExtent = InSize * 0.5f;
CenterHitBoxComponent->SetBoxExtent(HalfExtent);
BoxLocation = FVector(HalfExtent.X, 0.0f, 0.0f);  // 방향별 HitBox 위치
```

### 학습/메모
- **Server RPC**: UObject에서 호출 불가, AActor에서만 가능 → Character로 이동
- **UBoxComponent::SetBoxExtent()**: Half extent를 받음 (전체 크기의 절반)
- **위젯 버튼 클릭 판별**: `IsHovered()` 대신 `TMap<UButton*, ID>` 매핑 + `HasMouseCapture()` 사용
- **CSV 메타데이터**: `#`으로 시작하는 주석 줄을 메타데이터 저장용으로 활용

### 남은 TODO (테스트 레코드 기반)
- #3 대각선 맵 movement가 wall closed하게 안 움직임
- Lobby HUD 로컬 방 찾기 버튼 제거
- 그래픽 설정 (상/중/하) 추가
- 게임 시작 전 어빌리티 사용 금지 (GameState::GamePhase)

### 다음 작업 - Steam 빌드 테스트
**스팀 빌드로 아래 버그 수정 사항 검증 필요:**
- [ ] [Critical] 클라이언트 벽돌 스폰 작동 확인
- [ ] [Critical] #14 호스트 설정 값 반영 확인 (게임모드/맵 변경)
- [ ] #2 호스트 설정 패널 - 클라이언트에서 읽기 전용으로 표시 확인
- [ ] #11 3자 프로필 조회 작동 확인
- [ ] #1 WaitingRoom UI 갱신 확인
- [ ] #4 유저 커스텀 맵 preview offset 정렬 확인
- [ ] #8 TileActor collision 옆 칸 영향 없음 확인

---

## 2026-02-06 (이전 기록)
### 작업 내용 - Steam 2PC 버그 수정 (2차)

#### 버그 수정 (High → Medium 해결)

##### [해결] #16 Sumo 코스메틱 전이 버그
- **증상**: 호스트가 Sumo에서 죽고 리스폰되면 호스트 코스메틱이 다른 플레이어에게 적용됨
- **원인**: `PossessedBy()`에서 `CosmeticSub->GetLoadout()`이 항상 서버의 로드아웃 반환
- **수정**: 로드아웃이 이미 있으면 덮어쓰지 않음 + 로컬 컨트롤러만 초기 로드아웃 설정
- **파일**: `WjWorldCharacterPlay.cpp` (lines 186-213)
```cpp
// 리스폰 시 PlayerState에 이미 로드아웃이 있으면 덮어쓰지 않음
if (PS->GetCosmeticLoadout().Entries.IsEmpty())
{
    APlayerController* PC = Cast<APlayerController>(NewController);
    if (PC && PC->IsLocalController())
    {
        PS->SetCosmeticLoadout(CosmeticSub->GetLoadout());
    }
}
```

##### [해결] #2 호스트 설정 패널 클라이언트 표시 버그
- **증상**: 호스트 설정 패널이 클라이언트 UI에도 표시됨
- **원인**: `SessionManager->IsHost()` 값이 클라이언트에서 잘못 반환되는 경우 있음
- **수정**: `GetNetMode()` 추가 체크 - NM_Client이면 무조건 bIsHost = false
- **파일**: `WaitingRoomHUDWidget.cpp` (UpdateHostSettingsPanelVisibility)

##### [해결] #11 3자 프로필 스탯 조회 안됨
- **증상**: 다른 플레이어 프로필 열어도 스탯이 로드되지 않음
- **원인**: `OnSteamUserStatsReceived()`에서 빈 `FUniqueNetIdRepl` 브로드캐스트
- **수정**: Steam OSS `IdentityInterface`로 유효한 FUniqueNetIdRepl 생성
- **파일**: `WjWorldStatsSubsystem.cpp` (OnSteamUserStatsReceived)
```cpp
IOnlineSubsystem* OSS = IOnlineSubsystem::Get(STEAM_SUBSYSTEM);
if (OSS)
{
    IOnlineIdentityPtr IdentityInterface = OSS->GetIdentityInterface();
    if (IdentityInterface.IsValid())
    {
        UserIdRepl = FUniqueNetIdRepl(IdentityInterface->CreateUniquePlayerId(UserIdStr));
    }
}
OnUserStatsReceived.Broadcast(UserIdRepl);
```

##### [디버깅] #1 WaitingRoom 설정 변경 시 UI 미갱신
- **상태**: 디버깅용 로그 추가, 테스트 필요
- **파일**: `WaitingRoomHUDWidget.cpp` (UpdateRoomInfo)

##### [디버깅] #10 AW 코스메틱 3자에게 잠시 보임
- **상태**: 디버깅용 로그 추가, 테스트 필요
- **파일**: `WjWorldCosmeticComponent.cpp` (ApplyLoadout)

### Steam 2PC 테스트 결과 (2차 수정 후)

#### 미해결 버그
- **#1** WaitingRoom 호스트 방 설정 변경 시 UI 미갱신
- **#2** 호스트 방 설정 패널이 클라이언트에도 표시됨 (수정했으나 여전히 발생)
- **#3** 대각선 맵에서 movement가 wall closed하게 안 움직임
- **#4** 유저 커스텀 맵에서 클라이언트 벽돌 preview offset 어긋남
- **#8** TileActor collision이 옆 칸에도 영향
- **#11** 3자 프로필 조회 안 됨 (수정했으나 여전히 발생)
- **#14** 호스트 설정 패널 값이 실제로 반영 안 됨 (게임모드 변경 적용 안 됨) **[Critical]**

#### 새 버그 [Critical]
- **클라이언트에서 벽돌 스폰이 안 됨** - 게임 진행 불가

#### TODO
- Lobby HUD 로컬 방 찾기 버튼 제거
- 그래픽 설정 (상/중/하) 추가
- 게임 시작 전 어빌리티 사용 금지 (GameState::GamePhase)

### 학습/메모
- **서버 측 로드아웃 관리 주의**: `CosmeticSubsystem->GetLoadout()`은 항상 로컬(서버) 로드아웃 반환 → 리스폰 시 다른 플레이어에게 적용 위험
- **IsHost() 신뢰성**: SessionManager의 IsHost() 외에도 GetNetMode() 이중 체크 권장
- **FUniqueNetIdRepl 생성**: Steam SteamId → FUniqueNetIdRepl 변환 시 OSS IdentityInterface 사용

### 이슈/해결
- [해결] #16 Sumo 코스메틱 전이 → 로드아웃 존재 여부 + 로컬 컨트롤러 체크
- [해결] #2 호스트 설정 패널 클라이언트 표시 → NetMode 이중 체크
- [해결] #11 3자 프로필 스탯 → FUniqueNetIdRepl 올바른 생성
- [테스트 필요] #1 UI 미갱신, #10 코스메틱 잠시 보임

---

### 작업 내용 - 배치 시스템 다중 컨텍스트 확장 & AW Editor CSV 연동 & 대기실 호스트 설정 UI

#### 배치 시스템 → AW 게임플레이 연동
- **PlacementComponent CSV 내보내기** (`WjWorldPlacementComponent.cpp`)
  - AW 컨텍스트에서 저장 시 CSV 파일도 자동 내보내기
  - `ExportLayoutAsCSV()` 메서드 추가
  - 저장 경로: `Content/WallLayouts/User/`
- **WallDescriptionDataAsset 유저 레이아웃 스캔** (`WjWorldWallDescriptionDataAsset.cpp`)
  - `ScanUserWallLayouts()`: 유저 CSV 디렉토리 자동 스캔
  - `GetAllWallNames()`: 내장 + 유저 레이아웃 통합 목록
  - `GetWallDescriptionByNameIncludingUser()`: 유저 레이아웃 포함 검색
  - `GenerateRandomWallNameIncludingUser()`: 유저 레이아웃 포함 랜덤 선택
- **BrickSpawner 유저 레이아웃 지원** (`WjWorldBrickSpawner.cpp`)
  - `SpawnBricksFromWallNameAsync()`: 유저 레이아웃 검색 연동
  - `GenerateRandomWallName()`: 유저 레이아웃 포함

#### WallLayoutConverter 버그 수정
- **[버그] ValidateWallLayout 시작점 오인 문제**
  - 원인: 첫 번째 -1 셀을 시작점으로 사용 → 외곽 빈 공간이 시작점이 됨
  - 문제: Padding 추가 시 모든 레이아웃이 "열려있음"으로 오판
  - 수정: 외부/내부 영역 분리 로직
    - `MarkExteriorCells()`: 경계에서 Flood Fill로 외부 영역 마킹
    - `FindInteriorEmptyCell()`: 외부가 아닌 빈 셀 = 내부 영역 찾기
  - 파일: `WjWorldWallLayoutConverter.cpp/.h`

#### 로그 검토 도구 추가
- **`/log` 스킬** (`.claude/commands/log.md`)
  - 빠른 로그 검토: `/log`, `/log error`, `/log placement`, `/log warning`
- **`ue-log-analyzer` 에이전트** (`.claude/agents/ue-log-analyzer/`)
  - 심층 로그 분석: 크래시, 패턴 감지, 네트워크 이슈

#### ue-build-runner 에이전트 제약 추가
- **문제**: 빌드 검증 요청 시 에이전트가 프로젝트 파일을 수정 시도 (UE 5.7 → 5.5 다운그레이드)
- **수정**: SKILL.md에 명확한 제약 추가
  - 파일 수정 금지 (분석/보고만)
  - 프로젝트 파일(.uproject, Target.cs) 수정 금지
  - 엔진 버전 변경 시도 금지

### 학습/메모
- **에이전트 제약의 중요성**: tools에 Bash가 있으면 sed/echo로 파일 수정 가능 → 명시적 금지 필요
- **벽 레이아웃 유효성 검사**: 경계에서 시작하는 Flood Fill로 외부 영역을 먼저 마킹해야 함

### 이슈/해결
- [해결] ue-build-runner가 수정한 파일 git checkout으로 복원
- [해결] WallLayoutConverter 외부/내부 영역 구분 로직 구현
- [해결] FWjWorldWallDescription::FindStartingEmptyCell 동일 버그 수정 (벽돌 스폰 안됨)
- [해결] CreateRoomWindow 유저 맵 표시 시 URL 콜론(:) 문제 → User_ 접두사로 변경
- [해결] AW 그리드 스냅 인접 배치 불가 → GridOverlapCheckRadius(5) 분리

#### WaitingRoomHUDWidget 호스트 설정 패널 버그 수정
- **[버그] 호스트 설정 패널 기능 동작 안 함**
  - 원인: 코드는 있지만 실제 바인딩/초기화 누락
  - 수정 내용:
    1. `ApplySettingsButton->OnClicked.AddDynamic()` 추가
    2. `GameModeComboBox->OnSelectionChanged.AddDynamic()` 추가
    3. `NativeConstruct()`에서 `InitializeHostSettingsPanel()`, `UpdateHostSettingsPanelVisibility()` 호출 추가
    4. `NativeDestruct()`에서 ComboBox 델리게이트 언바인딩 추가
    5. `UpdateRoomInfo()`에서 `MapText` 업데이트 로직 추가
    6. `UpdateMapComboBoxForGameMode()`에 유저 레이아웃 스캔 추가
    7. include 문을 파일 상단으로 이동 (중간에 있던 것 제거)
  - 파일: `WaitingRoomHUDWidget.cpp`

#### 플레이어 이탈 처리 버그 수정
- **GameModePlay::Logout() 추가** (`WjWorldGameModePlay.cpp`)
  - 플레이어 나갈 때 `GameRule->OnPlayerLeft()` 호출되지 않던 문제 수정
  - `Super::Logout()` 전에 호출하여 PlayerState 유효한 상태에서 처리

#### 비디오 플레이어 Steam 환경 이슈 해결
- **원인**: 비디오 파일(.mp4)이 프로젝트에 없었음
- **수정**: `Content/Movie/Intro.mp4` 추가, FileMediaSource 경로 설정

#### ue-build-runner 에이전트 제거
- 빌드 검증 시간이 오래 걸려 에이전트 삭제
- 직접 배치 파일로 빌드 검증하는 방식으로 변경

### Steam 빌드 테스트 버그 수정 (High)

#### [해결] #8 TileActor collision 문제
- **증상**: 벽돌이 옆 칸에도 collision 영향 → 의도하지 않은 즉사
- **원인**: `bIsOverlapBricks[EWjWorldDirection::Max]` 배열 초기화 안 됨 → 가비지 값
- **수정**: `= {0}` 초기화 추가
- **파일**: `WjWorldTileActor.h`

#### [해결] #12 Sumo 라운드 리셋 후 ability 발동 안 됨
- **증상**: 죽은 후 다음 라운드에서 ability 발동 불가
- **원인**: `State_Eliminated` 태그가 PlayerState의 ASC에 남아있음 (캐릭터 리스폰해도 유지)
- **수정**: `RemoveAllPlayerBuffs()`에서 `State_Eliminated` 태그도 제거, ASC를 PlayerState에서 가져오도록 변경
- **파일**: `WjWorldGameRuleSumo.cpp`

#### [해결] #4 클라이언트 벽돌 preview offset 어긋남
- **증상**: 클라이언트가 벽돌 설치 시 preview 위치와 실제 설치 위치 불일치 (50,50 차이)
- **원인**: 서버가 클라이언트의 캐릭터 위치로 독자 계산 → 네트워크 지연으로 위치 차이
- **수정**: 클라이언트가 GridIndex를 캐시하고 Server RPC로 전달
  - `CachedPreviewGridIndex` 추가
  - `ServerSpawnBrickAtGridIndex()` Server RPC 추가
- **파일**: `GA_SpawnBrick.h/.cpp`

#### [해결] #5 늦게 참여한 클라이언트 카운트다운 3초 고정
- **증상**: 게임 시작 후 늦게 접속한 클라이언트가 항상 3초 카운트다운
- **원인**: `StartCountDownTime`이 고정값(3초)이고 경과 시간 미고려
- **수정**: `CountdownStartServerTime` 서버 시간 기록 + 남은 시간 계산
  - `GetServerWorldTimeSeconds()` 기반 경과 시간 계산
  - 클라이언트에서 `RemainingTime = StartCountDownTime - Elapsed` 계산
- **파일**: `WjWorldGameStatePlay.h/.cpp`

### Steam 빌드 테스트 결과 (2026-02-06)

#### 버그 - Critical (게임 진행 불가) - 모두 해결
- ~~**#14** WaitingRoom 호스트 설정 패널 값 반영 안 됨~~ ✅
- ~~**#7, #9** 호스트 접속 종료 시 클라이언트가 intro부터 시작~~ ✅
- ~~**#15** Sumo 낙하 die 후 캐릭터 빙의/스폰 안 됨~~ ✅

#### 버그 - High (게임플레이 영향) - 대부분 해결
- ~~**#8** TileActor collision이 옆 칸에도 영향~~ ✅
- ~~**#12** Sumo die 후 다음 라운드에서 일부 ability 발동 안 됨~~ ✅
- **#3** 대각선 맵에서 movement가 wall closed하게 안 움직임 (미해결)
- ~~**#4** 클라이언트 벽돌 설치 시 preview offset 어긋남~~ ✅
- ~~**#5** 클라이언트 진입 늦을 경우 게임 시작해도 카운트 3초 셈~~ ✅

#### 버그 - Medium (UX 문제)
- **#1** WaitingRoom host 방 설정 변경 시 UI 변경 없음
- **#2** Host 방 설정 기능이 클라이언트 UI에도 표기됨
- **#13** WaitingRoom host 설정 패널 최초 값 비어있음
- **#10** Cosmetic 장비 시 AW 진입 초반 3자에게도 내 cosmetic 보임
- **#11** 3자 profile 조회 안 됨

#### 확인됨 (수정 작동)
- **#6** 클라이언트 접속 종료 시 host 쪽 카운트/win 정상 작동 ✅

#### TODO
- Lobby HUD 로컬 방 찾기 버튼 제거
- Lobby HUD 그래픽 설정 (상/중/하) 추가 - GPU 사용량 높음

#### 확인 필요 (향후)
- Room 목록 1000개+ 스케일링/부하 문제
- Sumo 개별 타일 랜덤 제거 방식 시 리플리케이션 비용

---

## 2026-02-05
### 작업 내용 - Steam 출시 Polishing & 네트워크 모드 토글

#### LAN/Steam 네트워크 모드 토글 기능
- **ENetworkMode enum 추가** (`SessionTypes.h`)
  - `LAN`, `Steam` 두 가지 모드 지원
- **SessionManager 네트워크 모드 분기**
  - LAN: `bIsLANMatch=true`, `bUsesPresence=false`
  - Steam: `bIsLANMatch=false`, `bUsesPresence=true`, `bUseLobbiesIfAvailable=true`
  - `CreateSession()`, `FindSessions()`, `CreateMigrationSession()`, `FindMigrationSession()` 모두 적용
- **UI 지원**
  - `CreateRoomWindow`: `NetworkModeComboBox` 추가 (WITH_STEAM 빌드에서만 Steam 옵션 표시)
  - `RoomListWindow`: `SetNetworkMode()`, `ShowPopupWithNetworkMode()` 추가

#### Steam 출시 Polishing (크래시 안전성 & 코드 품질)
- **Critical null 체크 추가** (6개 파일)
  - `OnRep_IsGameStartCountDownReady()`, `OnRep_GameResult()` 등
- **빈 Tick() 비활성화** - `bCanEverTick = false` 설정
- **로그 카테고리 일관성** - `LogWjWorld` → `LogWjWorldStats`
- **check() → ensureMsgf() 변경** - 릴리스 빌드 크래시 방지
- **AttributeSet OnRep 매크로 추가** - `GAMEPLAYATTRIBUTE_REPNOTIFY`

#### Steam 2PC 테스트 버그 수정
- **[버그] Approaching Wall 종료 후 WaitingRoom 복귀 실패**
  - 원인: `OnGameEnd()` 타이머 람다에서 `this` 캡처 후 `GetWorld()` 호출
  - 수정: `TravelURL` 값 캡처 + `TWeakObjectPtr<UWorld>` 사용
  - 파일: `WjWorldGameRuleBase.cpp`
- **[버그] LobbyLayout SaveGame 주체 문제**
  - 원인: 클라이언트도 `SaveLayout()` 호출하여 호스트 레이아웃 덮어씀
  - 수정: `NetMode` 체크 추가 (`NM_Standalone` 또는 `NM_ListenServer`만 저장)
  - 파일: `WjWorldPlacementComponent.cpp`
- **[버그] WaitingRoom 코스메틱 리플리케이션 실패**
  - 원인: `GetPawn()` 3자 캐릭터에서 null 반환, 로컬 로드아웃이 모든 캐릭터에 적용
  - 수정: `TActorIterator`로 PlayerState 기반 캐릭터 검색, 로컬 플레이어만 초기 로드아웃 적용
  - 파일: `WjWorldCosmeticComponent.cpp`, `WjWorldPlayerStateBase.cpp`
- **[버그] LiftBrick/SpawnBrick 클라이언트 프리뷰 색상 오류**
  - 원인: `GetAuthGameMode()` 클라이언트에서 null → `CachedWallDesc` 미설정
  - 수정: `CurrentWallName` 리플리케이트 추가, `GameState`에서 `WallDesc` 로드
  - 파일: `ApproachingWallGameDataComponent.h/.cpp`, `WjWorldGameRuleApproachingWall.cpp`, `GA_LiftBrick.cpp`, `GA_SpawnBrick.cpp`
- **WjWorldAnimInstance 생성**
  - `LiftBrickBlendWeight` (0~1 float) GameplayTag 기반 블렌딩
  - `State.LiftBrickCarry` 태그 체크하여 부드러운 전환
  - 파일: `Animation/WjWorldAnimInstance.h/.cpp`
- **LiftBrick 벽돌 색상 리플리케이션**
  - `CarriedBrickColor` 리플리케이트 프로퍼티 추가
  - `LiftedBrickDynamicMaterial`로 런타임 색상 적용
  - 파일: `WjWorldCharacterPlay.h/.cpp`

#### Steam P2P 네트워킹 (SteamNetDriver) 문제 해결
- **SessionManager::Initialize() 폴백 로직 추가**
  - `IOnlineSubsystem::Get(STEAM_SUBSYSTEM)` 우선 시도 → 실패 시 `NULL_SUBSYSTEM` 폴백
  - `#include "OnlineSubsystemNames.h"` 추가
- **steam_appid.txt 패키징 빌드 누락**
  - 증상: `SteamAPI failed to initialize`, `[AppId: 0]`
  - 수정: 패키징 빌드 폴더에 수동 복사 → 이후 자동화 배치에 포함
- **bUsesPresence/bUseLobbiesIfAvailable 매칭**
  - Steam OSS에서 두 값이 다르면 세션 생성 실패
  - 수정: Steam 모드에서 둘 다 `true`로 설정
- **검색 타이밍 이슈 해결**
  - 증상: LAN 검색 진행 중 Steam 전환 시 "Ignoring game search request while one is pending"
  - 수정: `bIsSearchInProgress` 플래그 + `PendingSearchRequest` 큐 패턴
  - `CancelFindSessions()` 사용 시 앱 행 → 제거하고 wait-and-queue 패턴 채택
- **SteamNetDriver 로딩 안됨 근본 원인 3가지 수정**
  1. Config 섹션: `[/Script/Engine.GameEngine]` → `[/Script/Engine.Engine]` (BaseEngine.ini와 동일)
  2. DriverClassName: `"SocketSubsystemSteamIP.SteamNetDriver"` → `"/Script/SocketSubsystemSteamIP.SteamNetDriver"` (StaticLoadClass 정규 경로)
  3. `[OnlineSubsystemSteam]`에 `bUseSteamNetworking=true` 추가 (Steam 소켓 서브시스템 등록 조건)
- **NetConnectionClassName도 `/Script/` 접두사 형식으로 통일**
- **BeaconNetDriver, DemoNetDriver 재정의** (ClearArray 후 누락 방지)

#### 빌드 자동화
- **PackageAndUploadSteam.bat 생성** (`Batch/`)
  - Development Win64 패키징 → `Steam/content/` 복사 → `upload.bat` 실행
  - 각 단계 실패 시 즉시 중단, `steam_appid.txt` 자동 생성

#### Sumo Knockoff 미니게임 코드 구현 (기본)
- **GA_Push 어빌리티** (`AbilitySystem/Abilities/GA_Push.h/.cpp`)
  - 전방 구형 오버랩 → 히트 캐릭터에 `LaunchCharacter()` 넉백
  - PushForce=1200, PushRange=300, PushUpForce=400, CooldownDuration=1.5s
  - `SetLastAttacker()` 호출 (킬 추적), GameplayCue 트리거
- **WjWorldGameRuleSumo** (`Core/GameRule/WjWorldGameRuleSumo.h/.cpp`)
  - TickGameRule에서 매 프레임 Z 위치 체크 → FallThresholdZ(-500) 미만 시 Eliminate
  - 엣지 케이스: 솔로 자동 승리, 동시 탈락, 전원 이탈
- **SumoGameDataComponent / SumoPlayerDataComponent** (`Core/GameData/`)
  - 게임: AlivePlayerCount, TotalPlayerCount (Replicated)
  - 플레이어: bIsAlive (Replicated + OnRep + Delegate)
- **GameplayTag 추가**: `Ability.Push`, `Cooldown.Push`, `GameplayCue.Ability.Push`
- **WjTypes**: `EWjWorldAbilityInputID::Ability6 = 6` 추가
- **WjWorldStatTypes**: `WjWorldStats::Sumo` 네임스페이스 + Sumo 디스크립터

#### Sumo Knockoff 6대 기능 추가 구현
- **1. Push 히트 피드백** (`GA_Push.h/.cpp`)
  - `PushHitCameraShake` (TSubclassOf<UCameraShakeBase>) 프로퍼티 추가
  - 피격자에게 `ClientStartCameraShake()` 호출
  - `SuperPushMultiplier` (기본 2.0) - Buff.SuperPush 태그 보유 시 Force 배율 적용 후 태그 소모
- **2. 킬피드 시스템** (`SumoGameDataComponent`, `SumoHUDWidget`)
  - `LastKillFeedText` + `KillFeedCounter` (ReplicatedUsing) → 클라이언트 자동 동기화
  - `FOnSumoKillFeed` 델리게이트 → HUD에서 3초 표시 후 자동 숨김
  - GameRuleSumo에서 Eliminate 시 "{Killer} knocked out {Victim}" 브로드캐스트
- **3. 축소 플랫폼** (신규 `SumoFloorRingActor.h/.cpp`)
  - `ESumoRingState` (Active/Warning/Destroyed), `RingOrder`+`RingRadius` 프로퍼티
  - WarningMaterial 적용 → 일정 시간 후 Collision/Visibility 비활성화
  - GameRuleSumo: `TickShrinkPlatform()` - ShrinkInterval마다 외곽 링 경고→파괴
- **4. 라운드 시스템 (3라운드)** (`SumoGameDataComponent`, `SumoPlayerDataComponent`, `GameRuleSumo`)
  - `CurrentRound`, `MaxRounds`, `FSumoPlayerScore` 배열 (Replicated)
  - `TotalScore` (PlayerData), `AwardRoundScores()` 탈락 순서 기반 점수 배분
  - `OnRoundEnd()` → `ResetRound()` → `RestartPlayer()` → 다음 라운드 시작
  - 최종 라운드 후 총점 기준 우승자 결정
- **5. 파워업 시스템** (신규 `SumoPowerUpActor.h/.cpp`, `GE_Sumo*.h/.cpp`)
  - `ESumoPowerUpType` (SpeedBoost/SuperPush/Shield), SphereComponent 오버랩 감지
  - AddLooseGameplayTag 기반 버프 적용 (SpeedBoost=MaxWalkSpeed 증가+5초 타이머)
  - `GE_SumoSpeedBoost/SuperPush/Shield` 참조 GE 클래스 3개 생성
  - Shield: `OnEliminated()`에서 Shield 태그 소모하여 제거 무시 (CharacterPlay)
  - GameRuleSumo: `TickPowerUpSpawn()` 일정 간격 랜덤 파워업 스폰
- **6. 맵 변형 지원** (`GameRuleSumo`)
  - `MapOption` URL 파라미터 파싱 (Default/Bridge/Obstacle)
  - 맵별 FallThresholdZ, ShrinkInterval, PowerUpSpawnInterval 설정 분기
- **코드 리팩토링**
  - `CharacterPlay::OnEliminated()`: AW 하드코딩 제거, Shield 태그 체크 추가
  - `GameRuleApproachingWall::OnPlayerEliminated()`: 분리된 AW 전용 PlayerData 업데이트
  - `FloorRings` TArray 타입: `TObjectPtr<>` → raw pointer (UE 5.7 Sort 호환)
- **GameplayTag 추가**: `Buff.SpeedBoost`, `Buff.SuperPush`, `Buff.Shield`, `GameplayCue.Sumo.PowerUp.Pickup`

#### 미니게임별 어빌리티 제한 시스템
- **WjWorldMinigameDataAsset**: `AllowedAbilityTags`, `StatNamespace` 필드 추가
- **WjWorldGameStatePlay**: `AllowedAbilityTags`, `StatNamespace` Replicated 프로퍼티
- **WjWorldGameRuleBase**: `OnGameReady()`에서 MinigameCatalog 조회 → GameState에 설정
- **WjWorldGameplayAbilityBase**: `CanActivateAbility()` 오버라이드
  - 빈 컨테이너 = 전부 허용 (하위 호환), 비어있지 않으면 AssetTag 매칭 필요

#### 스탯 네임스페이스 범용화
- **WjWorldGameStatePlay::OnRep_GameResult()**: 하드코딩된 `ApproachingWall` 네임스페이스 대신 `StatNamespace` 기반 동적 스탯 키 생성

#### LAN SocketSubsystem 충돌 수정
- **문제**: `SocketSubsystemSteamIP`가 기본 소켓을 Steam으로 오버라이드 → `IpNetDriver`가 SteamSocketsP2P 주소로 바인딩 시도 → 실패
- **수정**: `WjWorldLanNetDriver` 생성 (`Network/WjWorldLanNetDriver.h/.cpp`)
  - `UIpNetDriver` 서브클래스, `GetSocketSubsystem()` → `PLATFORM_SOCKETSUBSYSTEM` 명시
  - `ApplyNetDriverForMode()`: LAN 시 `/Script/WjWorld.WjWorldLanNetDriver` 사용
  - `DefaultEngine.ini`에 `WjWorldLanNetDriver` 설정 섹션 추가
  - `Build.cs`에 `Sockets`, `Networking` 모듈 의존성 추가
- **결과**: LAN 2PC 접속 성공 확인, WaitingRoom 정상 동작

#### GA_Jump 어빌리티 구현 (Sumo 전용 점프)
- **GA_Jump** (`AbilitySystem/Abilities/GA_Jump.h/.cpp`)
  - UE 기본 `UGameplayAbility_CharacterJump` 패턴 기반, `WjWorldGameplayAbilityBase` 상속
  - `Character->Jump()` / `StopJumping()` 사용 (가변 높이 점프 지원)
  - `NetExecutionPolicy::LocalPredicted` (서버 왕복 없이 즉시 점프)
  - `CanActivateAbility()`: Super(AllowedAbilityTags) + `Character->CanJump()` 체크
  - `CommitAbility()` 패턴 사용 (GA_Push의 직접 ApplyCooldown과 다름)
  - `InputReleased()` → `StopJumping()` + `EndAbility()`
  - `CancelAbility()` → `StopJumping()` + Super
- **GameplayTag 추가**: `Ability.Jump`, `Cooldown.Jump` (WjWorldGameplayTag + DefaultGameplayTags.ini)
- **WjTypes**: `EWjWorldAbilityInputID::Ability7 = 7` 추가
- **에디터 세팅 필요**: IA_Ability7 + Spacebar, BP_GA_Jump, SetupDA 등록, Sumo AllowedAbilityTags

#### PackageAndUploadSteam.bat 버그 수정
- **[버그] RunUAT.bat `call` 누락** → .bat를 call 없이 실행하면 호출 스크립트가 종료됨
  - 수정: `call "%UE_ROOT%\...\RunUAT.bat"` 로 변경
- **[버그] 스크립트 끝에 `pause` 없음** → 정상 완료 시에도 창 즉시 닫힘
  - 수정: `endlocal` 뒤에 `pause` 추가
- **[버그] Development 에디터 빌드 누락** → `WjWorldEditor.target` 파일 없음 (DebugGame만 빌드)
  - 수정: `-build` 플래그 추가 → 패키징 전 에디터+게임 자동 빌드

### 학습/메모
- `GetAuthGameMode()`는 클라이언트에서 null 반환 → GameState의 리플리케이트된 데이터로 폴백
- ServerTravel URL 포맷: `GetAssetPathString()` (`.MapName` 포함) vs `GetLongPackageName()` (순수 경로)
- Timer 람다에서 `this` 캡처 주의 → 객체 소멸 후 호출 시 크래시, `TWeakObjectPtr` 사용
- **Steam vs LAN 세션 설정 차이점**:
  - LAN: `bIsLANMatch=true`, `bUsesPresence=false`, `bUseLobbiesIfAvailable=false`
  - Steam: `bIsLANMatch=false`, `bUsesPresence=true`, `bUseLobbiesIfAvailable=true`
  - 검색 시 `bIsLanQuery` 플래그도 맞춰줘야 함
- `SEARCH_PRESENCE` 상수는 UE 5.7에서 변경됨 → 직접 사용 불가, 제거하거나 문자열로 대체
- **SocketSubsystemSteamIP 모듈 동작 조건**:
  - 에디터에서는 자동 비활성화 (`IsRunningDedicatedServer() || IsRunningGame()` 체크)
  - `bUseSteamNetworking=true` 설정 필요 (Steam 소켓 서브시스템 등록)
  - `SteamNetDriver::IsAvailable()`이 Steam 소켓 서브시스템 등록 여부로 판단
- **UE Config NetDriverDefinitions 형식**: `/Script/ModuleName.ClassName` (StaticLoadClass 정규 경로)
- **Config 섹션 상속**: `UGameEngine` → `UEngine`, NetDriverDefinitions는 `UEngine`에 선언 → `[/Script/Engine.Engine]` 섹션 사용
- **CancelFindSessions()** → `OnCancelFindSessionsComplete` 콜백 발생 (OnFindSessionsComplete 아님) → 대기열 패턴에서 사용 금지
- **SocketSubsystemSteamIP 기본 소켓 오버라이드**: 이 플러그인은 `RegisterSocketSubsystem()`으로 Steam 소켓을 기본으로 등록 → `ISocketSubsystem::Get()` 호출 시 Steam 반환 → IpNetDriver 사용 시 프로토콜 불일치. 해결: NetDriver 서브클래스에서 `Get(PLATFORM_SOCKETSUBSYSTEM)` 명시
- **UE 5.7 GetAssetTags() API**: `const FGameplayTagContainer&` 직접 반환 (출력 파라미터 아님)
- **새 레벨 추가 시 패키징 목록 필수**: Project Settings > Packaging > List of maps to include in a packaged build에 추가 안 하면 `Failed to load package` 에러
- **TArray<T*>::Sort() 람다**: UE의 `TDereferenceWrapper`가 포인터를 자동 역참조 → 람다 파라미터는 `const T&` (포인터 아님)
- **TObjectPtr Sort 호환성**: UE 5.7에서 `TArray<TObjectPtr<T>>::Sort()` 시 deprecation warning 발생 → `TArray<T*>`로 변경하면 해결
- **InheritableOwnedTagsContainer deprecated** (UE 5.7): GE 생성자에서 태그 직접 추가 불가 → AddLooseGameplayTag()로 런타임에 태그 적용
- **AddLooseGameplayTag vs GE 태그**: Loose 태그는 GE 없이 직접 ASC에 추가/제거, 일회성 버프에 적합
- **Batch 스크립트에서 .bat 호출 시 `call` 필수**: `call` 없이 실행하면 호출 스크립트가 종료되고 돌아오지 않음. RunUAT.bat 등 외부 .bat 호출 시 반드시 `call` 사용
- **RunUAT `-build` 플래그**: Cook 단계에서 `{Target}.target` 파일이 필요. DebugGame 빌드만 있으면 Development 에디터 .target 파일이 없어 실패. `-build` 추가하면 자동으로 빌드 후 Cook 진행
- **GA_Jump `LocalPredicted` vs GA_Push `ServerInitiated`**: 점프는 즉각적인 응답이 필요하므로 LocalPredicted, 밀치기는 서버 권한이 중요하므로 ServerInitiated
- **`CommitAbility()` vs 직접 `ApplyCooldown()`**: CommitAbility는 Cost+Cooldown 한꺼번에 처리. 베이스 클래스의 virtual ApplyCooldown이 정상 호출됨

### 이슈/해결
- COMDAT 중복 링크 오류 → Intermediate 폴더 정리 후 재빌드
- **Steam 세션 전체 흐름**: OSS 초기화 → 세션 생성(Lobby) → 검색 → 참가 → SteamNetDriver P2P 연결 → 정상 동작 확인

### 버그 수정
- **[해결] 클라이언트 마우스 Control Rotation 미적용**
  - 원인: `UGameplayCameraComponent`가 클라이언트에서 `InputComponent` 생성 전에 활성화되어 `InputAxisBinding2DCameraNode`가 입력을 찾지 못함
  - 수정: `SetAutoActivate(false)` + `SetupPlayerInputComponent()`에서 IMC 등록 후 `Activate()` 호출
  - 파일: `WjWorldCharacterBase.cpp`

### 발견된 이슈
1. ~~**[해결] LAN 모드 클라이언트 방 입장 시 강제종료**~~
   - 원인: SteamNetDriver가 LAN에서도 사용됨 + SocketSubsystemSteamIP가 기본 소켓 오버라이드
   - 수정: `WjWorldLanNetDriver` (PLATFORM_SOCKETSUBSYSTEM 명시) + `ApplyNetDriverForMode()` 런타임 전환

2. **[버그] 비디오 플레이어 재생 안 됨 (Steam 다운로드 환경)**
   - 증상: 로컬 환경에서는 정상, 다른 PC에서 Steam 다운로드 받은 환경에서 재생 안 됨
   - 추정 원인: 패키징 이슈 (비디오 파일 미포함 또는 코덱 문제)
   - 상태: 조사 필요

### 향후 미니게임 로드맵

#### 다음 구현: Sumo Knockoff (넉백 대전)
- 원형 플랫폼 위에서 상대를 밀어 떨어뜨리는 서바이벌
- 축소되는 플랫폼 + 넉백 어빌리티 (GA_Push)
- 재활용: GameRule 서바이벌, TileActor (축소), 캐릭터 제거 판정
- 신규: 넉백 어빌리티, 원형 맵, 낙하 판정

#### 이후 구현: Obstacle Race (장애물 레이스) + 유저 맵 제작 시스템
- 출발점→골인점 장애물 통과 레이스
- 기본 맵 1개 제공 + **유저가 Lobby에서 커스텀 맵 제작 가능**
- 재활용: BrickMovement (이동 장애물), WallManager, TileActor

#### 배치 시스템 확장 계획 (Obstacle Race와 함께 구현)
현재 Lobby 전용인 배치 시스템을 미니게임별 맵 에디터로 확장:
1. **Lobby용 자유 배치** (현재 구현 완료)
2. **Approaching Wall 전용 배치** (벽 레이아웃 커스텀)
3. **Obstacle Race 전용 배치** (장애물 코스 제작)
- PlacementComponent를 컨텍스트별로 분리 (PlaceableObjectCatalog를 모드별로 관리)
- LayoutSaveGame에 모드별 슬롯 추가
- 호스트의 커스텀 맵을 대기실에서 로드하여 게임에 적용

### 할 일

#### Sumo Knockoff 에디터 세팅 (완료)
- [x] Sumo 레벨 맵 생성 (`Content/Map/03-2_Sumo`) - 원형 플랫폼 + 배경
- [x] **패키징 맵 목록에 추가** (Project Settings > Packaging)
- [x] MinigameCatalog에 Sumo 엔트리 추가 (GameModeId="Sumo", GameRuleClass=BP_GameRuleSumo)
- [x] BP_GameRuleSumo 블루프린트 생성 (WjWorldGameRuleSumo 기반)
- [x] CharacterPlaySetupDataAsset: StartInputAbilities에 Ability6→GA_Push 추가
- [x] IMC_Default: IA_Ability6 InputAction 생성 + 키 바인딩
- [x] MinigameCatalog Sumo 엔트리에 AllowedAbilityTags 설정 (Ability.Push 등)

#### Sumo Knockoff 6대 기능 에디터 세팅 (완료)
- [x] BP_SumoPowerUpActor 생성 (ASumoPowerUpActor 기반, 메시/콜리전 설정)
- [x] BP_GameRuleSumo에 `PowerUpActorClass` → BP_SumoPowerUpActor 할당
- [x] BP_SumoFloorRingActor에 `WarningMaterial` 할당
- [x] 03-2_Sumo 맵에 SumoFloorRingActor 동심원 배치
- [x] WBP_SumoHUD에 `KillFeedText`, `RoundText` TextBlock 위젯 추가
- [x] BP_HUDPlay `GameRuleHUDWidgetClasses`에 BP_GameRuleSumo → WBP_SumoHUD 매핑 확인
- [x] DA_MinigameCatalog Sumo 항목에 MapOptions 추가 (Default/Bridge/Obstacle)
- [x] CameraShake BP 생성 → BPGA_Push에 `PushHitCameraShake` 할당
- [x] 링 파괴 VFX/사운드 추가
- [x] GameplayCue.Sumo.PowerUp.Pickup 에셋 생성

#### GA_Jump 에디터 세팅 (필요)
- [ ] IA_Ability7 Input Action + Spacebar 바인딩 (IMC)
- [ ] BP_GA_Jump 생성 (GA_Jump 기반)
- [ ] CharacterPlaySetupDataAsset: Ability7 → BP_GA_Jump
- [ ] MinigameCatalog Sumo AllowedAbilityTags에 `Ability.Jump` 추가

#### 에디터 세팅 (남은 권장)
- [ ] 파워업 타입별 비주얼 구분 (SpeedBoost=파랑, SuperPush=빨강, Shield=노랑)

#### 에셋/폴리싱
- [ ] Lobby 맵 풍성하게 꾸미기
- [ ] 배치 모드 Mesh 추가
- [ ] Approaching Wall 나이아가라 에셋 폴리싱
- [ ] Destructible 에셋 추가
- [ ] 배경 음악 추가

---

## 2026-02-04 (저녁)
### 작업 내용 - Steam 테스트 환경 구축
- **Steam 앱 설정 완료**
  - AppID: 4399350, DepotID: 4399351
  - VDF 스크립트 생성 (`Steam/scripts/`)
  - DefaultEngine.ini Steam 설정 추가
  - steam_appid.txt 생성 (로컬 테스트용)
- **Steam 빌드 업로드** (BuildID: 21779250)
  - SteamCMD 기반 업로드 스크립트 (`Steam/upload.bat`)
  - Dev Comp Package로 테스트 계정 접근 설정
- **Steam Inventory Service 설정**
  - itemdefs.json 생성 (100: Delivery Bag, 101: Military Hat)
  - AddPromoItem/AddAllPromoItems 함수 추가 (CosmeticSubsystem)
  - Cosmetic_AddPromo/Cosmetic_AddAllPromos 콘솔 명령어 추가
- **패키징 이슈 수정**
  - ToolWidgets 모듈 제거 (에디터 전용 모듈)
  - IntroWindow: 비디오 재생 실패 시 폴백 로직 추가
  - WjWorldGameModeIntro: IntroWidgetClass 미설정 시 스킵 로직 추가
- **멀티플레이어 테스트 환경**
  - 두 번째 Steam 계정으로 테스트 환경 구축
  - Steamworks 파트너 그룹에 테스트 계정 추가

### 이슈/해결 (진행 중)
- **[버그] Approaching Wall 벽돌 스폰 안됨** (Development/Shipping 빌드 전용)
  - 증상 정리:
    1. DebugGameEditor (에디터에서 실행, 리슨서버 2명) - **문제 없음**
    2. DebugGame 패키징 + VS 디버깅 - **문제 없음**
    3. Steam 빌드 (Development/Shipping) - **벽돌 스폰 안됨**
    4. Development 패키징 (로컬 실행) - **벽돌 스폰 안됨**
  - 시도한 수정:
    - `WjWorldBrickSpawner::CreateBrickSpawner()`: `LoadObject` → `LoadSynchronous()` 변경
    - 결과: 여전히 동일한 증상
  - **원인 확정**: WallLayout `.txt` 파일 경로 문제
    1. `FFilePath`에 저장된 절대 경로가 패키지 빌드에서 유효하지 않음
    2. `.txt` 파일이 자동으로 패키지에 포함되지 않음
  - **수정 내용**:
    1. `DefaultGame.ini`: `+DirectoriesToAlwaysStageAsNonUFS=(Path="GamePlay/Wall")` 추가
    2. `WjWorldWallDescriptionDataAsset.cpp`: 절대 경로 → Content 상대 경로 변환 로직 추가
  - **상태**: ✅ 해결 확인 (패키징 빌드에서 벽돌 스폰 정상 동작)

### 발견된 추가 이슈 (Steam 환경 2PC 테스트)
1. **[버그] Approaching Wall 종료 후 WaitingRoom 복귀 실패**
   - 증상: 게임 종료 후 WaitingRoom으로 ServerTravel 안됨
   - 추정 원인: 하드코딩 경로 수정 시 누락된 부분
   - 상태: 조사 필요

2. **[버그] LobbyLayout SaveGame 주체 문제**
   - 증상: 배치하지 않은 클라이언트 기준으로 SaveGame되는 경우 발생
   - 재현: 재접속 시 상대방의 일부 배치물이 보임
   - 추정 원인: SaveLayout() 호출 주체 검증 누락
   - 상태: 조사 필요

3. **[버그] WaitingRoom 코스메틱 리플리케이션 실패**
   - 증상: WaitingRoom에서 다른 플레이어 코스메틱이 보이지 않음
   - 참고: Lobby/Play에서는 정상 동작
   - 상태: 조사 필요

### 학습/메모
- Steam Dev Comp Package: 파트너 그룹 계정에게 무료로 앱 접근 권한 부여
- itemdefs.json: 모든 값은 문자열이어야 함 (`false` → `"false"`)
- **Non-asset 파일 패키징**: `.txt`, `.csv` 등은 `DirectoriesToAlwaysStageAsNonUFS`로 명시적 포함 필요
- **FFilePath 경로 문제**: 에디터에서 절대 경로 저장 → 패키지 빌드에서 `FPaths::ProjectContentDir()` 기준으로 변환 필요
- **Debug vs Development 빌드 차이**: Debug는 개발 PC 파일 시스템 직접 접근, Development/Shipping은 .pak 파일 사용

### 나중에 논의할 내용
- **에셋 팩 관리 방법**: 마켓플레이스 에셋 팩 (BigNiagaraBundle, Fantasy_Pack, GJM_Assets, sA_PickupSet_1 등)
  - .gitignore는 적절하지 않음 (팀원/다른 PC에서 필요)
  - Git LFS 도입? 별도 저장소? 빌드 파이프라인에서 관리?
  - 용량 문제와 버전 관리 전략 필요

---

## 2026-02-04
### 작업 내용
- **코스메틱 미리보기/시착 시스템 구현**
  - CharacterPreviewActor: Socket 기반 메시 부착, StaticMesh/SkeletalMesh 동시 지원
  - SetupFromPawn()으로 Pawn에서 메시/ABP 복사
  - 다중 슬롯 시착 유지 (슬롯 전환 시 리셋 안 함)
- **하드코딩 경로 제거 및 DeveloperSettings 중앙화**
  - 맵/GameMode/캐릭터/Approaching Wall 에셋 중앙 설정
  - ConstructorHelpers 제거 → UPROPERTY + DeveloperSettings 폴백 패턴
- **Approaching Wall 미니게임 완성**
  - Kills 스탯 추적: LastAttacker 시스템 (CharacterPlay)
  - 플레이어 이탈 시 캐릭터 Eliminate 처리
  - 엣지 케이스: 솔로 승리, 동시 제거(무승부), 전원 이탈
- **코스메틱 상점 UI 구현** (6개 파일 생성)
  - `CosmeticItemEntryWidget` - 아이템 그리드 엔트리 (아이콘, 이름, 희귀도, 가격)
  - `CosmeticPreviewPanel` - 3D 캐릭터 프리뷰 (CharacterPreviewActor 재사용)
  - `CosmeticMainWindow` - 상점/인벤토리 통합 윈도우 (탭 전환, 4열 그리드)
  - `LobbyHUDWidget`에 코스메틱 버튼 추가
- **CosmeticSubsystem 초기화 개선**
  - DeveloperSettings에 `CosmeticCatalog` 프로퍼티 추가
  - Initialize()에서 자동 로드하도록 수정
- **CosmeticComponent 개선**
  - `OnLoadoutChanged` 델리게이트 구독 추가 (실시간 메시 반영)
  - `CharacterPlay` → `CharacterBase`로 이동 (모든 캐릭터에서 사용 가능)
- **Socket 기반 코스메틱 부착 시스템 구현**
  - `FCosmeticItemDefinition`에 부착 설정 추가 (AttachSocketName, LocationOffset, RotationOffset, Scale)
  - 슬롯별 기본 소켓 매핑: Head→"head", Back→"spine_03", Effect→"root"
  - 모자 메시 임포트 및 테스트 완료
- **Steam Inventory 폴링 콜백 구현**
  - `CosmeticSubsystem`: 타이머 기반 폴링 (StartInventoryPolling, PollSteamInventoryResult, ParseInventoryResult)
  - `PurchaseSubsystem`: 구매 결과 폴링 콜백 추가
- **코스메틱 테스트 콘솔 명령어 추가** (PlayerControllerBase)
  - `Cosmetic_GrantItem`, `Cosmetic_GrantAll`, `Cosmetic_ClearInventory`
  - `Cosmetic_PrintInventory`, `Cosmetic_PrintLoadout`
  - `Cosmetic_Equip`, `Cosmetic_Unequip`, `Cosmetic_RefreshInventory`
- **코스메틱 상점 UI 마무리**
  - 상점 모드에서 소유 아이템 장착/해제 기능 추가
- **멀티플레이어 코스메틱 동기화 수정**
  - `CosmeticComponent.OnLoadoutChangedHandler()`: IsLocallyControlled() 체크 추가
  - `CharacterBase.OnRep_PlayerState()`: 3자 캐릭터 코스메틱 적용 로직 추가
  - `PlayerStateBase`: OnPawnSet(), OnCosmeticLoadoutUpdated() 구현 (Play에서 이동)
  - `CharacterWaitingRoom.PossessedBy()`: 서버 측 코스메틱 초기화 추가
- **CLAUDE.md 갱신** 및 `/update-claude-md` 스킬 생성

### 학습/메모
- Socket Attachment vs Leader Pose vs Skeletal Mesh Merge: 슬롯 유형별 적합한 부착 방식이 다름
- 모자 등 고정형 악세서리는 Socket Attachment, 옷/갑옷은 Leader Pose 권장
- Mesh Merge는 드로우콜 최적화에 효과적이나 아이템 교체 시 재머지 필요
- Steam Inventory API는 비동기 → 폴링 기반 콜백 패턴 필요
- 멀티플레이어 코스메틱 동기화: `PossessedBy()`(서버) + `OnRep_PlayerState()`(클라이언트) 양쪽 필요
- `OnRep_PlayerState()`는 자신/3자 모두에게 호출됨 → 3자 캐릭터 초기화에 활용
- **GameplayCue 자동 매칭**: 태그 `GameplayCue.Ability.NormalAttack` → BP명 `GCN_Ability_NormalAttack`
- **GameplayCueNotify_Static**: 단발성 효과용, `OnExecute` 오버라이드
- **GameplayCueNotify_Actor**: 지속 효과용, `OnActive`/`WhileActive`/`OnRemove` 오버라이드
- **Z-Fight 해결**: 이동 중인 오브젝트에 미세한 Z 오프셋 적용

### 이슈/해결
- UHT 오류: 파라미터명 `Slot`이 UWidget::Slot과 충돌 → `CosmeticSlot`으로 변경
- `SetBrushFromTexture`가 RenderTarget 미지원 → `SetBrushResourceObject` 사용
- 멀티플레이어에서 OnLoadoutChanged 브로드캐스트가 모든 Pawn에 영향 → `IsLocallyControlled()` 체크 추가
- WaitingRoom 3자 코스메틱 미동기화 → `CharacterBase.OnRep_PlayerState()`에서 `OnPawnSet()` 호출하도록 수정

### 완료된 작업 (Approaching Wall 개선)
- [x] Normal Attack 시 Montage 발동 (코드 완료, 에셋 필요)
- [x] Lift Brick 시 드는 포즈 및 실제 벽돌을 든 모습 3자 Replicate (코드 완료, 에셋 필요)
- [x] Brick 이동 시 다른 색 벽돌 간 Z-Fight 현상 수정
- [x] 벽돌과 플레이어 끼임 케이스 추가 처리
- [x] GameplayCue 사용으로 Ability 발동 시 사운드 효과 추가 (코드 완료, 에셋 필요)

### 완료된 에디터/에셋 작업
- [x] AnimBP에서 LiftBrickCarry 포즈 설정 (State.LiftBrickCarry 태그 체크)
- [x] GameplayCue 사운드 에셋 4개 (NormalAttack, SpawnBrick, LiftBrick, LiftBrick.Place)

### 다음 작업 예정 (에디터/에셋 작업 - 낮은 우선순위)
- [ ] 공격 AnimMontage 생성 및 BP_GA_NormalAttack에 할당

---

## 2026-02-03
### 작업 내용
- CLAUDE.md 문서 업데이트 - 배치 시스템, 카탈로그, 맵 전환 흐름 추가
- 로비 배치 시스템, GameRule 카탈로그 조회, Ready 버튼 피드백 수정
- 학습 노트 자동화 시스템 구축
  - `/devlog` 슬래시 명령어 생성 (일일 개발 로그 작성)
  - `/sync-learning` 슬래시 명령어 생성 (claude-learning 레포 동기화)
  - GitHub Actions 워크플로우 생성 (CLAUDE.md, DEVLOG.md 변경 시 자동 동기화)
- `/init-learning` 명령어 추가
- **프로젝트 전체 코드 리뷰** (5개 영역 병렬 검토)
  - 리플리케이션 검증: HP/MaxHP DOREPLIFETIME 누락 발견
  - GAS 검토: 쿨다운 ApplyCooldown() 미호출 발견
  - GameRule 검증: Player null 체크 누락 발견
  - 코스메틱 시스템: 클라이언트 카탈로그 미설정 발견
  - 빌드 설정 검토
- **즉시 수정 항목 5개 수정**
  - WjWorldCharacterAttributeSet: HP/MaxHP 리플리케이션 + 클램핑 추가
  - GA_NormalAttack, GA_LiftBrick: ApplyCooldown() 호출 추가
  - WjWorldGameRuleBase: Player null 체크 추가
  - WjWorldCharacterPlay: 클라이언트 카탈로그 설정 추가
  - WjWorldPlayerStatePlay: Pawn 없을 때 로드아웃 지연 적용
- **Claude Code 커스텀 에이전트 5개 생성** (`.claude/agents/`)
  - replication-validator: 네트워크 리플리케이션 검증
  - ability-system-expert: GAS 코드 검토
  - gamerule-tester: GameRule 라이프사이클 검증
  - ue-build-runner: 빌드 및 컴파일 오류 분석
  - cosmetic-reviewer: 코스메틱/구매 시스템 검토
- **Claude Code 활용 팁 문서 작성** (claude-learning 레포)

### 학습/메모
- Claude Code Custom Slash Commands: `.claude/commands/` 폴더에 마크다운 파일로 정의
- GitHub Actions로 cross-repo 작업 시 Personal Access Token (Fine-grained) 필요
- 프로젝트별 DEVLOG.md + 전체 학습 레포 분리 구조가 관리에 효율적
- **Claude Code Agent 시스템**: `.claude/agents/에이전트명/SKILL.md` 형식으로 커스텀 에이전트 생성 가능
- **Agent vs Skill**: Agent는 독립 컨텍스트에서 실행 (결과만 반환), Skill은 메인 대화 컨텍스트에서 실행
- **유용한 단축키**: `Shift+Tab` (권한 모드 전환), `Ctrl+O` (상세 출력), `Esc+Esc` (되돌리기)
- **비용 절감**: `/compact` 자주 사용, Plan Mode로 계획 후 실행, Haiku 모델 활용

### 이슈/해결
- `OnPawnSet` protected 접근 오류 → public으로 이동하여 해결

---

## 2026-02-02
### 작업 내용
- 로비 배치 시스템 구현 (PlacementComponent, PreviewActor, PlacedObjectActor)
- GameRule 카탈로그 조회 시스템 추가
- Ready 버튼 피드백 수정

### 학습/메모
-

---

## 이전 기록

### 주요 마일스톤
- ApproachingWall 버그 수정 및 HUD/GameData 시스템 구현
- 플레이어 프로필/스탯 시스템 구현
- 어빌리티 UI/HUD 추가
- GE 파일 구조 정리
