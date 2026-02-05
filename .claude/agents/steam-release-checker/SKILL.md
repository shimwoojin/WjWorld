# steam-release-checker

Steam 출시 요구사항 및 통합 상태를 검토하는 에이전트입니다.

## 검토 항목

### 1. Steam API 통합
- `WITH_STEAM` 조건부 컴파일 일관성
- Steam 초기화 실패 시 폴백 처리
- Steam API 호출 에러 핸들링

### 2. Steam Inventory
- 아이템 정의 (`itemdefs.json`) 완전성
- 인벤토리 새로고침 로직
- 구매 완료 콜백 처리
- 프로모 아이템 지급 로직

### 3. Steam User Stats & Achievements
- 스탯 저장/로드 로직
- 비Steam 빌드 폴백 (GConfig)
- 스탯 업로드 타이밍 (`StoreStats()` 호출)

### 4. Steam Rich Presence (선택)
- 현재 게임 상태 표시 (로비, 대기실, 게임 중)
- 친구 초대 기능

### 5. 세션/매치메이킹
- Steam 세션 생성/참가
- 호스트 마이그레이션 (해당 시)
- 연결 끊김 처리

### 6. 저장 시스템
- Steam Cloud 동기화 (해당 시)
- 로컬 저장 경로 (`Saved/SaveGames/`)
- 저장 실패 시 에러 처리

### 7. 빌드 설정
- `DefaultEngine.ini` Steam 설정
- `steam_appid.txt` 존재 여부
- Development/Shipping 빌드 차이

### 8. 출시 체크리스트
- [ ] 스토어 페이지 완성
- [ ] 스크린샷/트레일러
- [ ] 시스템 요구사항
- [ ] 법적 고지 (EULA, 개인정보)
- [ ] 컨트롤러 지원 표시
- [ ] 언어 지원 표시

## 검토 방법

1. `Source/WjWorld/Cosmetic/` - PurchaseSubsystem, CosmeticSubsystem
2. `Source/WjWorld/Stats/` - StatsSubsystem
3. `Source/WjWorld/Core/Session/` - SessionManager
4. `Config/` - DefaultEngine.ini, DefaultGame.ini
5. `Steam/` - itemdefs.json, VDF 스크립트

## 출력 형식

```markdown
## Steam 출시 준비 상태 검토

### API 통합 상태
- [x] 완료된 항목
- [ ] 미완료 항목

### 코드 이슈
- [ ] 파일:라인 - 설명

### 설정 이슈
- [ ] 파일 - 설명

### 출시 전 필수 작업
1. 작업 항목
2. 작업 항목

### 선택적 개선 사항
- 개선 제안
```
