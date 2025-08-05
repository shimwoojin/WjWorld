# WjWorld

언리얼 엔진 5.6으로 개발하는 개인 C++ 프로젝트

## 📋 프로젝트 개요

다양한 게임 장르를 실험하고 학습하기 위한 프로젝트입니다.

### 개발 목표
1. **허브 공간** - 플레이어가 컨텐츠로 진입할 수 있는 로컬 공간
2. **미니게임** - 다양한 장르의 미니게임 구현 
3. **멀티플레이어** - 기본적인 네트워킹 기능 구현
4. **융합 컨텐츠** - 로컬과 멀티플레이어 요소 결합

## 🛠️ 개발 환경

- **엔진**: Unreal Engine 5.6
- **언어**: C++
- **IDE**: Visual Studio 2022
- **버전 관리**: Git (TortoiseGit)

## 📁 프로젝트 구조

```
WjWorld/
├── Content/
│   ├── Maps/                   # 레벨 파일들
│   ├── GameModes/              # 게임모드 관련
│   │   ├── Base/              # 공통 베이스 클래스
│   │   ├── Local/             # 로컬 전용
│   │   └── Multiplayer/       # 멀티플레이어
│   ├── UI/                    # UMG 위젯들
│   ├── Characters/            # 캐릭터 관련
│   └── batch/                 # 개발용 배치 파일들
├── Source/
│   └── WjWorld/
│       ├── Public/            # 헤더 파일들
│       └── Private/           # 구현 파일들
└── Config/                    # 프로젝트 설정 파일들
```

## 🚀 빌드 방법

### 1. 필수 요구사항
- Visual Studio 2022 (C++ 개발 도구 포함)
- Unreal Engine 5.6
- Windows 10/11 SDK

### 2. 프로젝트 설정
```bash
# 저장소 클론
git clone [your-repository-url]
cd WjWorld

# Visual Studio 프로젝트 파일 생성
# .uproject 파일 우클릭 → "Generate Visual Studio project files"
```

### 3. 빌드
- Visual Studio에서 솔루션 열기
- **Solution Configuration**: `DebugGame Editor` 또는 `Development Editor`
- **F5** 또는 **Ctrl+Shift+B**로 빌드

### 4. DebugGame 모드 실행
배치 파일을 사용하여 DebugGame Editor 모드로 실행:
```bash
# batch/ 폴더의 OpenProjectDebugGame.bat 실행
```

## 📝 개발 일지

### Phase 1: 기본 구조 (진행 중)
- [x] 프로젝트 초기 설정
- [x] GameMode 클래스 구조 설계
- [x] 폴더 구조 정리
- [ ] 인트로 동영상 시스템
- [ ] 허브 레벨 구현

### Phase 2: 미니게임 (예정)
- [ ] 미니게임 프레임워크
- [ ] 장르별 미니게임 구현

### Phase 3: 네트워킹 (예정)
- [ ] 기본 멀티플레이어 구현
- [ ] 플레이어 동기화

## 🔧 주요 클래스

### GameMode 계층구조
```cpp
AGameModeBase
└── AWjWorldGameModeBase           // 프로젝트 공통 베이스
    ├── AWjWorldGameModeIntro      // 인트로 화면
    ├── AWjWorldGameModeLobby      // 허브/로비
    └── [미니게임별 GameMode들]
```

### UI 시스템
```cpp
UUserWidget
└── UWjWorldUserWidgetBase         // 프로젝트 UI 베이스
    └── UIntroWidget              // 인트로 동영상 UI
```

## 🎮 게임 플레이 플로우

1. **게임 시작** → 인트로 동영상 재생
2. **동영상 종료** → 유저 상태에 따른 레벨 이동
   - 첫 방문: 튜토리얼 레벨
   - 기존 유저: 허브 레벨
3. **허브에서** → 다양한 컨텐츠 선택 가능
4. **컨텐츠 완료** → 허브로 복귀

## 📚 참고 자료

- [Unreal Engine Documentation](https://docs.unrealengine.com/)
- [C++ Programming Guide](https://docs.unrealengine.com/5.6/en-US/programming-and-scripting/)
- [Multiplayer Programming Guide](https://docs.unrealengine.com/5.6/en-US/multiplayer-programming-quick-start/)

## 📄 라이선스

개인 학습 프로젝트

---

**개발자**: [Your Name]  
**시작일**: 2025.08.05  
**언리얼 엔진**: 5.6
