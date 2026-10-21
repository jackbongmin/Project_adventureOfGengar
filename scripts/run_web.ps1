# 웹 빌드를 로컬 서버로 띄워 브라우저에서 실행한다. (index.html을 더블클릭하면 wasm 로딩이 막히므로 서버가 필요하다.)
#   사용법:  powershell -ExecutionPolicy Bypass -File scripts\run_web.ps1 [-Port 8080] [-NoBrowser]
#   build-web\index.html 이 없으면 먼저 빌드한다. 종료는 터미널에서 Ctrl+C.
param(
  [int]$Port = 8080,
  [switch]$NoBrowser,
  [string]$EmsdkPath = $(if ($env:EMSDK) { $env:EMSDK } else { "C:\emsdk" })
)

$root = Split-Path -Parent $PSScriptRoot
$index = "$root\build-web\index.html"

if (-not (Test-Path $index)) {
  Write-Host "build-web\index.html 이 없어서 먼저 빌드합니다..."
  & "$PSScriptRoot\build_web.ps1"
  if ($LASTEXITCODE -ne 0) { exit $LASTEXITCODE }
}

$env:EMSDK_QUIET = "1"
$ErrorActionPreference = "Continue"
& "$EmsdkPath\emsdk_env.ps1" *> $null

$emrunArgs = @("--port", "$Port")
if ($NoBrowser) { $emrunArgs += "--no_browser" }

Write-Host "http://localhost:$Port/index.html 에서 실행합니다. (Ctrl+C로 종료)"
& emrun @emrunArgs $index
