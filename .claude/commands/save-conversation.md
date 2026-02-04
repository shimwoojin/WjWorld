# Save Conversation

현재 대화 내용을 요약해서 claude-learning 레포에 저장합니다.

## 사용법
```
/save-conversation
```

## 실행 순서

1. **대화 분석**: 현재 세션의 주요 내용 파악
   - 어떤 프로젝트 작업을 했는지
   - 배운 내용이나 새로운 패턴
   - 해결한 문제나 버그

2. **요약 문서 생성**: `C:\EtcProjects\claude-learning-docs\docs\conversations\` 경로에 마크다운 파일 생성
   - 파일명: `{YYYY-MM-DD}-{간단한-제목}.md`
   - 형식:
     ```markdown
     ---
     title: 대화 제목
     date: YYYY-MM-DD
     project: 프로젝트명
     tags: [태그1, 태그2]
     ---

     # 요약
     간략한 요약 (2-3문장)

     ## 작업 내용
     - 작업 항목 1
     - 작업 항목 2

     ## 배운 점
     - 학습 내용 1
     - 학습 내용 2

     ## 코드 스니펫 (있는 경우)
     ```언어
     코드
     ```

     ## 다음 할 일
     - TODO 1
     - TODO 2
     ```

3. **index.md 업데이트**: conversations/index.md에 새 항목 추가

4. **Git 커밋 및 푸시**:
   ```bash
   cd C:\EtcProjects\claude-learning-docs
   git add docs/conversations/
   git commit -m "Add conversation: {제목}"
   git push
   ```

## 참고
- 코드 전체를 복사하지 말고 핵심 패턴이나 스니펫만 포함
- 개인정보나 민감한 정보는 제외
- 가능한 한 간결하게 작성
