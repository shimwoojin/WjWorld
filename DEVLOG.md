# WjWorld 개발 로그

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
