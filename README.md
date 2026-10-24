# Adventure of Gengar

C++17 + SDL2로 만든 2D 횡스크롤 액션 게임. 같은 소스로 **웹(WebAssembly)** 과 **Windows 네이티브** 두 가지로 빌드된다.

- 보라색 유령으로 스테이지 3개를 돌파하고 최종 보스 VOIDMAW를 쓰러뜨린다.
- 로그라이크 성장: 적을 처치해 경험치를 모으고, 레벨업할 때마다 카드 3장 중 1장을 골라 능력치를 올리거나 새 능력을 얻는다. 카드는 일반/희귀/영웅/전설 4등급(테두리 색이 다르다), 60종. 혈사포·레이저·부메랑 같은 공 모양 카드로 빌드가 갈리고, 일부 카드는 겉모습(뿔, 날개, 도는 구슬, 비행 날개)도 바꾼다. 피해는 10 단위(기본 10).
- 스테이지 사이 상점에서는 코인으로 카드를 더 뽑거나(첫 50코인, 그 판에서 뽑을 때마다 +25로 계속 오름, 희귀 이상 1장 보장), 얻은 카드를 강화할 수 있다(등급별 60~200코인 + 단계마다 20). 보스전에서는 보스 체력 단계가 바뀔 때 카드를 준다.
- 스테이지 1·2: 들어가면 벽이 닫히고 적이 웨이브로 나오는 매복 결투장(웨이브마다 제한 시간, 넘기면 증원. 끝나면 보물상자 = 코인 + 보너스 카드), 밟으면 높이 튀어 오르는 점프 버섯, 특별 스킬을 쓰는 금빛 정예 적, 뛰어드는 그림자 두꺼비, 높은 길의 숨은 상자.
- 카드 기획서(등급 확률, 카드 목록, 중첩 규칙, 경험치/상점 수치): [docs/design/roguelike_cards.xlsx](docs/design/roguelike_cards.xlsx)

## 조작

| 키 | 동작 |
|---|---|
| ← → | 이동 |
| **C** (또는 Space, ↑) | 점프 |
| **Z** | 섀도우볼 (누르고 있으면 연사) |
| **X** (또는 왼쪽 Shift) | 대시 (짧게 돌진. 대시 칸을 하나 쓰고 칸은 다시 찬다. 공중에서는 착지 전까지 1번. 무적은 '무적 대시' 카드로 얻는다) |
| **A** | 필살기 (게이지 100일 때) |
| Esc / P | 일시정지 |
| Enter (또는 Z) | 선택 / 구매, 시네마틱 다음 장면 |
| 카드 화면: ← → / Z / **R** | 카드 고르기 / 확정 / 다시 뽑기(리롤 카드의 무료 횟수, 없으면 15코인부터) |
| **Tab** | 능력치·카드 창 (카드에 마우스를 올리면 자세한 설명, Tab/Esc 로 닫기) |
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
    data/       카드 표와 뽑기 규칙, 능력치 수식, 보스 패턴 순서표
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

설계 설명과 "새 적/보스 패턴/스테이지/카드 추가 방법"은 [docs/ARCHITECTURE.md](docs/ARCHITECTURE.md) 참고.
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
- 현재 `v0.4.8 BETA` (0.2.0: 로그라이크 카드 시스템, 0.2.1: 카드 연출·설명과 보스 밸런스 조정, 0.2.2: 저주탄 조정, 0.2.3: 분열·튕김·차지샷 재설계와 보스전 발판, 0.3.0: 코인 다시 뽑기·스테이지 클리어 화면·바닥 기둥 높이, 0.3.1: 상점 화면 정리와 카드 버그 수정, 0.4.0: Tab 능력치 창, 스테이지 1·2 개편, 0.4.1: 정예 스킬·두꺼비·결투장 제한 시간, 보스 체력 900, 0.4.2: 피해 10단위·새 카드 10종·카드 외형, 0.4.3: 상점 카드 강화·뽑기 가격 상승·확률/체력 조정, 0.4.4: 전설 확률·뽑기 가격 상한, 0.4.5: 웨이브 공 수정·확률 재조정·강화 화면 정리, 0.4.6: 분신 0.8초 주기·뽑기 가격 누적, 0.4.7: 그림자 날개·사거리 표시 정리, 0.4.8: 부메랑 반복 피해 수정·폭발탄 범위 확대). 첫 정식 출시 때 `1.0.0`.

## 검증용 디버그 옵션

주소 뒤에 붙이거나(웹) 명령줄 인자로 준다(네이티브: `--bot --god`).

| 옵션 | 설명 |
|---|---|
| `bot=1` | 자동 플레이 봇이 조종 (낭떠러지/벽 점프, 카드 자동 선택, 상점 자동 뽑기) |
| `god=1` | 피격 무적 (낙사는 여전히 사망) |
| `log=1` | 스테이지/상점/보스 패턴/피격 출처 등 이벤트를 콘솔에 출력 |
| `speed=8` | 배속 (1~16) |
| `perf=1` | 2초마다 fps, 프레임 간격, 렌더 시간, 프레임당 로직 스텝 수, 그리기 호출 수를 출력 (끊김 진단용. exe는 `Start-Process -RedirectStandardOutput`로 받는다) |
| `stage=2` `coins=100` `hp=1` | 시작 스테이지(0~2) / 시작 코인 / 시작 체력 |
| `shop=1` | 시작하자마자 상점 (타이틀에서 Enter) |
| `cards=homing,split,attackup,attackup` | 시작할 때 카드 지급. 이름은 카드 게임 표기에서 공백을 뺀 것, 같은 이름을 여러 번 쓰면 여러 단계 |
| `pick=N` | 스테이지를 시작하자마자 카드 고르기 N번 (카드 화면 검증용, 자동 시작) |
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

