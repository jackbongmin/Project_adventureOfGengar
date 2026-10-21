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
| **X** | 필살기 (게이지 100일 때) |
| Esc / P | 일시정지 |
| Enter (또는 Z) | 선택 / 구매 |

키 배치는 [Source/Private/platform/Input.cpp](Source/Private/platform/Input.cpp)의 바인딩 표에서 바꾼다.

타이틀 화면에서 **G**를 누르면 모든 도트 스프라이트를 한눈에 보는 갤러리가 열린다. (개발용)

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

## 현재 상태

- 웹 빌드는 컴파일/링크 경고 없이 통과하고 브라우저에서 타이틀, 스테이지 1 진입, 이동/점프/공격, 적과 HUD 동작을 확인했다.
- 스테이지 2/3 전체 플레이, 보스 전 패턴, 상점 구매 흐름은 HTML 프로토타입에서 검증한 로직을 옮긴 것이며 C++ 쪽 실플레이 검증은 진행 중이다.
- 네이티브(Windows) 빌드는 아직 시도하지 않았다.
- 그래픽은 도트 스프라이트다. 현재 그림은 코드로 생성한 초안이며, 격자를 직접 고치거나 갤러리(G)로 확인할 수 있다.
- 화면 글자는 내장 픽셀 폰트(영문 대문자)라서 한글 표시는 아직 지원하지 않는다.

