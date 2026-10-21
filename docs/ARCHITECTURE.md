# 아키텍처

## 한눈에 보기

```
main -> Game ──(현재 하나)──> GameMode  (Title / Play / Shop / GameOver / Victory)
                                  │
                         PlayMode ├─ World ── Level(타일, 발판, 스폰 지점)
                                  │          ├─ Actor 목록 (Player, Enemy, Boss, 투사체, 픽업 ...)
                                  │          ├─ Camera, Effects
                                  └─ StateMachine<PlayMode>  (Playing / Paused / PlayerDead / StageClear)
```

로직은 고정 60Hz 스텝으로 갱신되고, 속도 단위는 "픽셀/프레임"이다. (core/Game.cpp의 Frame)

## 핵심 개념

### Mode (mode/) — 화면 단위 흐름
`Game`이 `GameMode` 하나만 들고 `Update/Render`를 위임한다. 모드 전환은 `Game::GoTitle / StartStage / GoShop / GoGameOver / GoVictory`로만 하며, 전환은 다음 스텝 시작에 적용되므로 모드 안에서 안전하게 호출할 수 있다.

### State (state/) — 상태 머신
`StateMachine<Owner>`는 현재 상태 객체 하나를 들고 있고, `Change()`는 현재 `Update`가 끝난 뒤 적용된다.
- `state/boss/`: 보스의 행동. Enter -> Float -> 공격 패턴(Volley, Ring, Summon, Slam, Beam, Rain, Pillar, Dash, Blink) -> Float ... -> Dying
- `state/play/`: PlayMode의 진행 단계. Playing, Paused, PlayerDead, StageClear

### Actor (actor/) — 월드의 객체
모든 객체의 기반은 `Actor`(위치/속도/크기, `Update(World&)`, `Render`). `ActorKind` 값의 순서가 그대로 렌더 레이어다.

- `Character`: 체력, 타일 충돌(`MoveAndCollide`), 컨트롤러를 가진 이동형 액터. `Player`, `Enemy`의 부모.
- `Enemy`: 접촉 피해/밟기/사망 보상을 처리. `Slime`, `Bat`, `Hound`, `Caster`가 상속.
- `Boss`: 상태 머신으로 움직이는 보스. 중심 좌표를 `pos`로 쓴다.
- `projectile/`, `hazard/`, `pickup/`: 섀도우볼, 적 마법구, 충격파, 그림자 기둥, 코인, 하트.

### Controller (controller/) — 누가 조종하는가
컨트롤러가 매 프레임 `Intent`(이동, 점프, 공격, 필살기, 조준)를 만들고, 액터는 Intent만 보고 움직인다.
입력 장치와 AI가 액터에서 분리돼서, 같은 `Enemy`에 다른 컨트롤러를 끼우면 행동이 바뀐다.

| 컨트롤러 | 사용처 | 동작 |
|---|---|---|
| PlayerController | Player | 키보드 -> Intent |
| PatrolController | Slime | 벽/낭떠러지/가시에서 방향 전환 |
| ChaseController | Hound | 범위 안이면 추적, 벗어나면 순찰 |
| TurretController | Caster | 사거리 안이면 조준 사격 |
| FlyController | Bat | 사인파 비행 |

### Data (data/) — 수치와 표
- `PlayerStats.h`: 업그레이드 레벨 -> 능력치 수식 (공격력, 사거리, 연사, 점프 등)
- `Upgrades.cpp`: 상점 항목 표
- `BossPatterns.cpp`: 보스 단계별 패턴 순서표
- `core/Config.h`: 화면/타일/중력/보스 체력 등 전역 상수
- `world/StageBuilder.cpp`: 스테이지 3개의 지형, 적, 아이템 배치

## 확장 레시피

**새 적 추가**
1. `actor/enemy/`에 `Enemy` 상속 클래스 작성 (`Render`, `DeathColor`, 필요하면 `OnShoot`/`StepPhysics`)
2. 기존 컨트롤러를 끼우거나 `controller/`에 새 컨트롤러 작성
3. `Level.h`의 `SpawnType`에 추가, `World.cpp` 생성 switch와 `StageBuilder.cpp` 배치에 등록

**새 보스 패턴 추가**
1. `data/BossPatterns.h`의 `BossAttack`에 항목 추가, `.cpp`의 시퀀스/이름 표 수정
2. `state/boss/BossStates.cpp`에 `BossStateBase` 상속 상태 작성, `MakeBossAttackState`의 switch에 등록

**새 스테이지 추가**
`StageBuilder.cpp`에 `BuildXxx` 함수를 만들고 `BuildStage`에 분기를 추가한다. `cfg::kStageCount`/`kBossStage`도 맞춘다.

**새 업그레이드 추가**
`core/Session.h`의 `UpgradeId`에 추가 -> `data/Upgrades.cpp` 표에 한 줄 -> `data/PlayerStats.h`에 수식 추가 -> 해당 수치를 쓰는 곳에서 사용.

**키 변경**
`platform/Input.cpp`의 바인딩 표.

## 폴더 규칙 (Public / Private)

- `Source/Public`: 다른 모듈이 include 하는 헤더
- `Source/Private`: 구현(.cpp)과 그 모듈 안에서만 쓰는 헤더 (예: `PixelFont.h`, `LevelRenderer.h`)
- 하위 폴더 구조는 Public과 Private에서 동일하게 유지한다. include 경로는 `"actor/Player.h"`처럼 하위 폴더부터 쓴다.


## 도트 스프라이트 (art/)

- 스프라이트는 **문자 격자**(한 글자 = 픽셀 1개, . = 투명)로 정의하고, 시작할 때 텍스처로 구워 둔다. 이미지 파일이 필요 없고 확대는 nearest-neighbor라 번지지 않는다.
- 격자 데이터: Source/Private/art/SpriteData.cpp, 색(팔레트): Source/Private/art/Art.cpp, ID 모음: Source/Public/art/Art.h (rt::A()).
- 같은 격자에 팔레트만 바꿔 변형을 만든다. (보스 3단계 색, 늑대 평소/추적 눈 색, 테마별 타일)
- 도트 1개 = 화면 2px(rt::kPixel). 선/원도 Graphics::PixelLine/PixelRing으로 계단식으로 그린다.
- **스프라이트 갤러리**: 타이틀에서 **G** 키. 모든 도트를 한눈에 확인한다.
- **스프라이트 생성기**: 	ools/spritegen/gen.js (Node). 도형(타원/다각형)으로 격자를 만들고 외곽선과 음영을 자동으로 입힌다.
  - 미리보기: 
ode tools/spritegen/gen.js preview phantom0 boss
  - 데이터 재생성: 
ode tools/spritegen/gen.js cpp Source/Private/art/SpriteData.cpp Source/Private/art/SpriteData.h
  - 생성된 SpriteData.cpp는 손으로 직접 고쳐도 된다. (다시 생성하면 덮어써진다)
- 나중에 PNG 스프라이트 시트로 교체하려면 Graphics::CreateSprite가 받는 형태(격자+팔레트) 대신 PNG 로더를 추가하면 된다.
## 알려진 제약 / 다음 단계

- 글자는 영문 대문자 내장 폰트만 지원한다. 한글은 TTF 폰트 도입이 필요하다.
- 보스의 소환 박쥐는 HTML 프로토타입과 달리 아레나 안쪽에서 순찰하도록 위치를 조정했다.

