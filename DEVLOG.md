# WjWorld 개발 로그

## 2026-02-03
### 작업 내용
- CLAUDE.md 문서 업데이트 - 배치 시스템, 카탈로그, 맵 전환 흐름 추가
- 로비 배치 시스템, GameRule 카탈로그 조회, Ready 버튼 피드백 수정
- 학습 노트 자동화 시스템 구축
  - `/devlog` 슬래시 명령어 생성 (일일 개발 로그 작성)
  - `/sync-learning` 슬래시 명령어 생성 (claude-learning 레포 동기화)
  - GitHub Actions 워크플로우 생성 (CLAUDE.md, DEVLOG.md 변경 시 자동 동기화)

### 학습/메모
- Claude Code Custom Slash Commands: `.claude/commands/` 폴더에 마크다운 파일로 정의
- GitHub Actions로 cross-repo 작업 시 Personal Access Token (Fine-grained) 필요
- 프로젝트별 DEVLOG.md + 전체 학습 레포 분리 구조가 관리에 효율적

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
