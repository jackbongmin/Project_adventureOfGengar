# Adventure of Gengar

C++17 + SDL2로 만든 2D 횡스크롤 액션 게임. 같은 소스로 **웹(WebAssembly)** 과 **Windows 네이티브** 두 가지로 빌드된다.

- 보라색 유령으로 스테이지 3개를 돌파하고 최종 보스 VOIDMAW를 쓰러뜨린다.
- 코인으로 상점에서 능력치를 강화하고, 적을 처치해 필살기 게이지를 채운다.

## 조작

| 키 | 동작 |
|---|---|
| ← → | 이동 |
| **C** (또는 Space, ↑) | 점프 |
| **Z** | 섀도우볼 (누르고 있으면 연사) |
| **X** (또는 왼쪽 Shift) | 대시 (짧게 돌진, 그동안 피해 무시. 공중에서는 착지 전까지 1번) |
| **A** | 필살기 (게이지 100일 때) |
| Esc / P | 일시정지 |
| Enter (또는 Z) | 선택 / 구매, 시네마틱 다음 장면 |
| **C 꾹 누르기** | 오프닝/엔딩 시네마틱 건너뛰기 |

키 배치는 [Source/Private/platform/Input.cpp](Source/Private/platform/Input.cpp)의 바인딩 표에서 바꾼다.

타이틀 화면에서 **G**를 누르면 모든 도트 스프라이트를 한눈에 보는 갤러리가 열린다. (개발용)

## 실행하기

가장 쉬운 방법 (웹 빌드를 로컬 서버로 실행). **프로젝트 폴더로 이동한 뒤** 실행한다:

```powershell
cd C:\Users\user\Documents\GitHub\project_adventureOfGengar
powershell -ExecutionPolicy Bypass -File scripts\run_web.ps1
```

다른 폴더에서는 스크립트의 전체 경로를 `-File`에 주면 된다. (상대 경로는 현재 폴더 기준이라 못 찾는다.)

- `build-web\index.html`이 없으면 먼저 빌드하고, 브라우저가 열리면 게임 화면을 한 번 클릭한 뒤 키보드로 조작한다.
- 포트 변경은 `-Port 9000`, 브라우저를 자동으로 열지 않으려면 `-NoBrowser`.
- 빌드에 필요한 emsdk 설치는 아래 "빌드 (웹)" 참고.
- `index.html`을 더블클릭해서 여는 방식은 동작하지 않는다. (브라우저가 파일에서 `.wasm`을 불러오는 것을 막는다.)
- 결과물(`build-web/`)은 정적 웹 호스팅(GitHub Pages 등)에 그대로 올려서 공개할 수 있다.

## 폴더 구조

```
Source/
  Public/    외부에 공개하는 헤더 (.h)
  Private/   소스 (.cpp)와 내부 전용 헤더
    core/       Game(루트), Session(진행 데이터), 설정/타입
    platform/   SDL 래퍼: Graphics, Input, Audio, 픽셀 폰트
    world/      World, Level, StageBuilder, Camera, Effects
    actor/      Actor/Character, Player, enemy/, boss/, projectile/, hazard/, pickup/
    controller/ 입력/AI -> Intent 변환 (Player, Patrol, Chase, Turret, Fly)
    state/      상태 머신 + 보스 패턴 상태(boss/) + 플레이 진행 단계(play/)
    mode/       화면 단위 흐름: Title, Play, Shop, GameOver, Victory
    data/       업그레이드 표, 능력치 수식, 보스 패턴 순서표
    art/        도트 스프라이트 데이터, 팔레트, 에셋 ID
    ui/         HUD
web/shell.html  웹 빌드용 페이지 틀
scripts/        빌드 스크립트
tools/spritegen 도트 스프라이트 생성기 (Node)
prototype-html/ 처음 만든 HTML 프로토타입 (참고용)
docs/           설계 문서
```

설계 설명과 "새 적/보스 패턴/스테이지/업그레이드 추가 방법"은 [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) 참고.
개발 중 막혔던 문제와 해결 과정은 [docs/troubleshooting/troubleshooting.md](docs/troubleshooting/troubleshooting.md)에 정리했다.

## 빌드 (웹)

필요한 것: **Emscripten SDK (emsdk)**. 설치하면 컴파일러, Node, Python이 함께 설치된다.

```powershell
git clone https://github.com/emscripten-core/emsdk.git C:\emsdk
cd C:\emsdk
.\emsdk install latest
.\emsdk activate latest
```

> Windows에서 `emsdk.bat`가 "Python was not found"로 실패하면 시스템에 Python이 없는 경우다.
> [python.org](https://www.python.org/downloads/)에서 Python 3를 설치하거나, emsdk가 쓰는 Python 압축 파일을 `C:\emsdk\python\3.13.3_64bit`에 풀어 둔다.

빌드와 실행:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build_web.ps1
C:\emsdk\emsdk_env.ps1        # (새 터미널에서 한 번) emrun 사용을 위해
emrun build-web\index.html
```

결과물은 `build-web/` (index.html, index.js, index.wasm). 정적 웹 서버라면 어디서든 올려서 실행할 수 있다.
`.wasm`은 반드시 `application/wasm` MIME으로 제공돼야 한다. (GitHub Pages는 자동 처리)

## 빌드 (Windows 네이티브)

SDL2 개발 라이브러리와 CMake가 필요하다. vcpkg를 쓰는 예:

```powershell
vcpkg install sdl2:x64-windows
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE=<vcpkg 경로>\scripts\buildsystems\vcpkg.cmake
cmake --build build --config Release
```

## 검증용 디버그 옵션

주소 뒤에 붙이거나(웹) 명령줄 인자로 준다(네이티브: `--bot --god`).

| 옵션 | 설명 |
|---|---|
| `bot=1` | 자동 플레이 봇이 조종 (낭떠러지/벽 점프, 상점 자동 구매) |
| `god=1` | 피격 무적 (낙사는 여전히 사망) |
| `log=1` | 스테이지/상점/보스 패턴/피격 출처 등 이벤트를 콘솔에 출력 |
| `speed=8` | 배속 (1~16) |
| `stage=2` `coins=100` `hp=1` | 시작 스테이지(0~2) / 시작 코인 / 시작 체력 |
| `shop=1` | 시작하자마자 상점 (타이틀에서 Enter) |
| `bossphase=0~2` `bosshp=N` `immortal=1` | 보스전 검증: 시작 단계 / 체력 / 불사 |
| `heartdemo=1` | 시작 지점에 하트 추가 |
| `heightdemo=1` | 플레이어를 위 발판에 올리고 아래에 추적 중인 늑대를 배치 (추적 포기 검증, 타이틀에서 Enter) |

예: `index.html?bot=1&god=1&log=1&speed=8` 은 처음부터 엔딩까지 자동으로 진행한다. 웹 빌드는 로그를 `window.__logs`에도 모아 둔다.

## 현재 상태

- 웹 빌드는 컴파일/링크 경고 없이 통과하고 브라우저에서 타이틀, 스테이지 1 진입, 이동/점프/공격, 적과 HUD 동작을 확인했다.
- 봇 자동 플레이로 스테이지 1 → 상점 → 스테이지 2(무너지는 발판 포함) → 상점 → 보스 3단계 → 격파 → 엔딩까지 끝까지 통과하는 것을 확인했다. 사망/재도전/게임오버 흐름, 풀피 하트 규칙, 보스 패턴 14종과 피격 판정도 같은 방식으로 확인했다.
- 사람이 키보드로 직접 해 본 플레이 감각(난이도, 조작감)은 별도로 확인이 필요하다.
- 네이티브(Windows) 빌드는 아직 시도하지 않았다.
- 그래픽은 도트 스프라이트다. 현재 그림은 코드로 생성한 초안이며, 격자를 직접 고치거나 갤러리(G)로 확인할 수 있다.
- 화면 글자는 내장 픽셀 폰트(영문 대문자)라서 한글 표시는 아직 지원하지 않는다.

