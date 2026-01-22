# WjWorld

언리얼 엔진 5.7으로 개발하는 개인 C++ 프로젝트

## 프로젝트 개요

허브 공간, 미니게임, 멀티플레이어 기능을 갖춘 개인 학습 프로젝트입니다.

### 개발 목표
1. **허브 공간** - 플레이어가 컨텐츠로 진입할 수 있는 로컬 공간
2. **미니게임** - 다양한 장르의 미니게임 구현 (GameRule 시스템 기반)
3. **멀티플레이어** - 기본적인 네트워킹 기능 구현
4. **융합 컨텐츠** - 로컬과 멀티플레이어 요소 결합

## 개발 환경

- **엔진**: Unreal Engine 5.7
- **언어**: C++
- **IDE**: Visual Studio 2022
- **버전 관리**: Git
- **문서화**: Doxygen

## 소스 구조

```
Source/WjWorld/
├── AbilitySystem/                     # Gameplay Ability System
│   ├── Abilities/                     # 어빌리티 클래스들
│   ├── AttributeSets/                 # 어트리뷰트 셋
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
├── DataAsset/                         # 데이터 에셋
├── GamePlay/                          # 게임플레이 시스템
│   ├── Camera/                        # 카메라 시스템
│   ├── Interact/                      # 상호작용
│   ├── Quest/                         # 퀘스트 시스템
│   └── Wall/                          # Approaching Wall 미니게임
├── Network/                           # 네트워크/패킷 관련
└── UI/                                # UI 위젯들
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
└── AWjWorldCharacterPlay          # 게임플레이 캐릭터 (ASC 지원)
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

### UI Widget
```
UWjWorldUserWidgetBase
├── UIntroWindow
├── ULoginWindow
├── ULobbyHUDWidget
├── UWaitingRoomHUDWidget
├── UGameplayGlobalHUDWidget        # 게임플레이 HUD
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

**주요 클래스:**
- `WjWorldBrickSpawner` - 비동기 벽돌 스폰 (8개/틱)
- `WjWorldBrickMovement` - 개별 벽돌 이동 로직
- `WjWorldWallManager` - 벽 이동 진행 관리
- `WjWorldWallDescriptionDataAsset` - 벽 레이아웃 데이터

## 빌드 방법

### 필수 요구사항
- Visual Studio 2022 (C++ 개발 도구 포함)
- Unreal Engine 5.7
- Windows 10/11 SDK

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
게임 진행 (레벨업, 벽 이동)
    ↓
승리 조건 체크 → 결과 표시
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
  - [x] 게임플레이 HUD (카운트다운)

### 진행 중
- [ ] **Approaching Wall 완성**
  - [ ] 승리 조건 (최후 생존자)
  - [ ] 플레이어 사망/제거 로직
  - [ ] 플레이어 어빌리티 (이동, 공격)
  - [ ] 게임 결과 처리 및 대기실 복귀

### 예정
- [ ] 추가 미니게임 구현
- [ ] 멀티플레이어 동기화 개선

## 문서화

Doxygen으로 생성된 문서는 `docs/` 폴더에서 확인할 수 있습니다.

## 라이선스

개인 학습 프로젝트

---

**시작일**: 2025.08.05
