# 웹(WebAssembly) 빌드: CMake/Ninja 없이 emcc 만으로 빌드한다.
#   사용법:  powershell -ExecutionPolicy Bypass -File scripts\build_web.ps1 [-Debug]
#   결과물:  build-web\index.html (+ .js/.wasm)
#   실행:    emrun build-web\index.html   (또는 아무 정적 웹 서버)
param(
  [switch]$Debug,
  [string]$EmsdkPath = $(if ($env:EMSDK) { $env:EMSDK } else { "C:\emsdk" })
)

$root = Split-Path -Parent $PSScriptRoot

# emsdk 환경 변수(PATH 등)를 현재 세션에 적용 (안내 메시지는 stderr로 나오므로 오류로 취급하지 않는다)
$env:EMSDK_QUIET = "1"
$ErrorActionPreference = "Continue"
& "$EmsdkPath\emsdk_env.ps1" *> $null
# em++ 경고(stderr)가 오류로 처리되지 않도록 Continue 유지, 종료 코드로 판단한다

$sources = Get-ChildItem "$root\Source\Private" -Recurse -Filter *.cpp | ForEach-Object { $_.FullName }
$out = "$root\build-web"
New-Item -ItemType Directory -Force $out | Out-Null

$opt = @("-O2")
if ($Debug) { $opt = @("-O0", "-g") }

& em++ @sources `
  -I "$root\Source\Public" -I "$root\Source\Private" `
  -std=c++17 -Wall -Wextra @opt `
  -sUSE_SDL=2 -sALLOW_MEMORY_GROWTH=1 `
  --shell-file "$root\web\shell.html" `
  -o "$out\index.html"

if ($LASTEXITCODE -ne 0) { Write-Error "em++ failed ($LASTEXITCODE)"; exit $LASTEXITCODE }
Write-Host "OK -> $out\index.html"
