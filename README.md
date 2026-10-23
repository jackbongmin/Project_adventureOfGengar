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

타이틀 화면은 메인 메뉴다: NEW GAME / **BOSS BATTLE**(코인 150개를 들고 상점에서 시작해, 나가면 바로 보스전) / STAGE SELECT / CONTROLS / SETTINGS / SPRITE GALLERY / QUIT(웹 제외).
SETTINGS에서 효과음 크기, 화면 흔들림, 점프 방식(FIXED 고정 높이 / VARIABLE 짧게=낮게·꾹=높게 / SHORT TAP 짧게 누르면 더 낮게)을 바꾼다. 설정은 저장되지 않고 실행할 때마다 기본값으로 돌아간다.
메인 메뉴에서 **G**를 눌러도 스프라이트 갤러리가 열린다. (개발용)

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
tools/icongen   exe 아이콘 생성기: 보물 모자 도트로 res/windows/gengar.ico 를 만든다 (Python + Pillow)
res/windows     exe 아이콘과 리소스 파일
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

## 빌드 (Windows 네이티브, 배포용 exe)

SDL2와 C++ 런타임을 exe 안에 넣어서, **받는 사람은 exe 파일 하나만 있으면 된다.** (Visual Studio나 DLL 설치 불필요)

필요한 것 (만드는 쪽에만):
- Visual Studio 2022 (C++ 데스크톱 개발). CMake와 Ninja는 VS에 들어 있는 것을 쓴다.
- vcpkg와 정적 SDL2

```powershell
git clone --depth 1 https://github.com/microsoft/vcpkg.git D:\jongmin\vcpkg
D:\jongmin\vcpkg\bootstrap-vcpkg.bat -disableMetrics
$env:VCPKG_DEFAULT_BINARY_CACHE = "D:\jongmin\vcpkg\_cache"   # 캐시도 C 드라이브 대신 vcpkg 폴더 안에 둔다 (폴더를 먼저 만들어 둘 것)
D:\jongmin\vcpkg\vcpkg.exe install sdl2:x64-windows-static --clean-after-build
```

빌드:

```powershell
powershell -ExecutionPolicy Bypass -File scripts\build_win.ps1            # vcpkg 위치가 다르면 -VcpkgRoot <경로>
```

결과물은 `build-win\dist\AdventureOfGengar.exe`(약 2MB)와 배포용 `build-win\AdventureOfGengar-<버전>-beta-win64.zip`이다.
코드 서명이 없어서 받는 사람이 처음 실행하면 "Windows의 PC 보호" 창이 뜰 수 있다. "추가 정보 → 실행"을 누르면 된다.

## 버전

모든 화면 오른쪽 아래와 창 제목에 버전이 나온다. 값은 [Version.h](Source/Public/core/Version.h) 한 곳에서 바꾼다.

- 형식은 `MAJOR.MINOR.PATCH`. 정식 출시 전에는 MAJOR가 0이다.
- 기능(스테이지, 적, 메뉴 등)이 늘면 MINOR를 올리고 PATCH는 0으로, 버그 수정·밸런스 조정만 있으면 PATCH를 올린다.
- 현재 `v0.1.0 BETA`. 첫 정식 출시 때 `1.0.0`.

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
| `stompdemo=N` | 가만히 선 늑대 머리 위에서 떨어뜨림. N(1~32)만큼 시작 높이를 1px씩 바꿈 (밟기 판정 검증, 자동 시작) |

예: `index.html?bot=1&god=1&log=1&speed=8` 은 처음부터 엔딩까지 자동으로 진행한다. 웹 빌드는 로그를 `window.__logs`에도 모아 둔다.

## 현재 상태

- 웹 빌드는 컴파일/링크 경고 없이 통과하고 브라우저에서 타이틀, 스테이지 1 진입, 이동/점프/공격, 적과 HUD 동작을 확인했다.
- 봇 자동 플레이로 스테이지 1 → 상점 → 스테이지 2(무너지는 발판 포함) → 상점 → 보스 3단계 → 격파 → 엔딩까지 끝까지 통과하는 것을 확인했다. 사망/재도전/게임오버 흐름, 풀피 하트 규칙, 보스 패턴 14종과 피격 판정도 같은 방식으로 확인했다.
- 사람이 키보드로 직접 해 본 플레이 감각(난이도, 조작감)은 별도로 확인이 필요하다.
- 네이티브(Windows) 빌드는 경고 없이 통과하고, 외부 DLL 없이(Windows 기본 DLL만 사용) 실행되는 것을 확인했다.
- 그래픽은 도트 스프라이트다. 현재 그림은 코드로 생성한 초안이며, 격자를 직접 고치거나 갤러리(G)로 확인할 수 있다.
- 화면 글자는 내장 픽셀 폰트(영문 대문자)라서 한글 표시는 아직 지원하지 않는다.

