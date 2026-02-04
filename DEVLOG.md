# WjWorld 개발 로그

## 2026-02-04
### 작업 내용
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

### 이슈/해결
- UHT 오류: 파라미터명 `Slot`이 UWidget::Slot과 충돌 → `CosmeticSlot`으로 변경
- `SetBrushFromTexture`가 RenderTarget 미지원 → `SetBrushResourceObject` 사용
- 멀티플레이어에서 OnLoadoutChanged 브로드캐스트가 모든 Pawn에 영향 → `IsLocallyControlled()` 체크 추가
- WaitingRoom 3자 코스메틱 미동기화 → `CharacterBase.OnRep_PlayerState()`에서 `OnPawnSet()` 호출하도록 수정

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
