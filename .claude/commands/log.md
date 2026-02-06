# /log - 언리얼 엔진 로그 빠른 검토

게임 실행 후 로그를 빠르게 확인하는 명령어입니다.

## 사용법

```
/log                    # 에러/경고 요약
/log placement          # LogWjWorldPlacement 로그만
/log error              # 에러만 검색
/log warning            # 경고만 검색
/log last 100           # 마지막 100줄
/log <pattern>          # 커스텀 패턴 검색
```

## 수행할 작업

### 인자가 없는 경우 (/log)
1. `Saved/Logs/WjWorld.log` 파일 존재 확인
2. 에러 검색: `Error:`, `error C`, `error LNK`
3. 경고 검색: `Warning:`, `warning C`
4. 프로젝트 로그 카테고리 검색:
   - `LogWjWorld`
   - `LogWjWorldPlacement`
   - `LogWjWorldAbilities`
   - `LogWjWorldCosmetic`
   - `LogWjWorldStats`
5. 결과 요약 출력

### placement 인자 (/log placement)
`LogWjWorldPlacement` 카테고리만 검색하여 출력

### error 인자 (/log error)
에러 패턴만 검색: `Error:|error C|error LNK|Fatal`

### warning 인자 (/log warning)
경고 패턴만 검색: `Warning:|warning C`

### last N 인자 (/log last 100)
로그 파일 마지막 N줄 출력

### 커스텀 패턴 (/log <pattern>)
사용자가 지정한 패턴으로 grep 검색

## 로그 파일 경로
```
C:\UEProjects\WjWorld\Saved\Logs\WjWorld.log          # 최근 세션
C:\UEProjects\WjWorld\Saved\Logs\WjWorld-backup-*.log # 이전 세션
```

## 출력 형식

```
## 로그 검토 결과

**파일**: WjWorld.log (최종 수정: YYYY-MM-DD HH:MM:SS)
**파일 크기**: XXX KB

### 에러 (N개)
```
[에러 로그 내용]
```

### 경고 (N개)
```
[경고 로그 내용]
```

### 프로젝트 로그 (카테고리별)
- LogWjWorldPlacement: N개
- LogWjWorldAbilities: N개
...

### 요약
- 치명적 오류: 있음/없음
- 주의가 필요한 경고: N개
- 권장 조치: [있다면 기술]
```

## 팁
- 백업 로그 확인 필요시: "이전 세션 로그도 확인해줘"
- 특정 시간대 검색: `/log 2026.02.06-11`
- 크래시 분석: `/log crash` 또는 `/log Fatal`
