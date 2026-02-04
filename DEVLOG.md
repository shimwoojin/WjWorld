# WjWorld 개발 로그

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

### 다음 작업 예정 (에디터/에셋 작업 - 낮은 우선순위)
- [ ] 공격 AnimMontage 생성 및 BP_GA_NormalAttack에 할당
- [ ] AnimBP에서 LiftBrickCarry 포즈 설정 (State.LiftBrickCarry 태그 체크)
- [ ] GameplayCue 사운드 에셋 4개 (NormalAttack, SpawnBrick, LiftBrick, LiftBrick.Place)

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
