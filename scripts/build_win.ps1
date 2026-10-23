# Windows 배포용 exe 빌드: SDL2 와 C++ 런타임을 exe 안에 넣어서, 받는 사람은 exe 하나만 있으면 된다.
#   사용법:  powershell -ExecutionPolicy Bypass -File scripts\build_win.ps1 [-VcpkgRoot D:\jongmin\vcpkg]
#   결과물:  build-win\dist\AdventureOfGengar.exe, build-win\AdventureOfGengar-<버전>-win64.zip
#   필요한 것: Visual Studio 2022 (C++ 데스크톱 개발), vcpkg 에 설치한 sdl2:x64-windows-static
param(
  [string]$VcpkgRoot = $(if ($env:VCPKG_ROOT) { $env:VCPKG_ROOT } else { "D:\jongmin\vcpkg" })
)

$ErrorActionPreference = "Stop"
$root = Split-Path -Parent $PSScriptRoot
$out = "$root\build-win"

if (-not (Test-Path "$VcpkgRoot\scripts\buildsystems\vcpkg.cmake")) { Write-Error "vcpkg not found: $VcpkgRoot" }

# Visual Studio 를 찾아 64비트 컴파일러 환경 변수를 현재 세션에 적용한다
$vswhere = "${env:ProgramFiles(x86)}\Microsoft Visual Studio\Installer\vswhere.exe"
$vs = & $vswhere -latest -products * -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath
if (-not $vs) { Write-Error "Visual Studio C++ tools not found" }
cmd /c "`"$vs\VC\Auxiliary\Build\vcvars64.bat`" >nul && set" | ForEach-Object {
  if ($_ -match "^([^=]+)=(.*)$") { Set-Item -Path "env:$($Matches[1])" -Value $Matches[2] }
}
# VS 에 딸린 CMake / Ninja
$env:PATH = "$vs\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin;$vs\Common7\IDE\CommonExtensions\Microsoft\CMake\Ninja;$env:PATH"

# 컴파일러 경고(stderr)가 오류로 처리되지 않도록 Continue 로 두고, 종료 코드로 판단한다
$ErrorActionPreference = "Continue"
cmake -S $root -B $out -G Ninja `
  -DCMAKE_BUILD_TYPE=Release `
  "-DCMAKE_TOOLCHAIN_FILE=$VcpkgRoot\scripts\buildsystems\vcpkg.cmake" `
  -DVCPKG_TARGET_TRIPLET=x64-windows-static `
  -DGENGAR_DIST=ON
if ($LASTEXITCODE -ne 0) { Write-Error "cmake configure failed ($LASTEXITCODE)"; exit $LASTEXITCODE }
cmake --build $out
if ($LASTEXITCODE -ne 0) { Write-Error "build failed ($LASTEXITCODE)"; exit $LASTEXITCODE }
$ErrorActionPreference = "Stop"

# 버전은 Source/Public/core/Version.h 에서 읽는다
$version = (Select-String -Path "$root\Source\Public\core\Version.h" -Pattern 'kNumber = "([^"]+)"').Matches[0].Groups[1].Value
$dist = "$out\dist"
New-Item -ItemType Directory -Force $dist | Out-Null
Copy-Item "$out\AdventureOfGengar.exe" $dist -Force
$zip = "$out\AdventureOfGengar-$version-beta-win64.zip"
# 이전 zip 을 다른 프로그램이 열고 있으면 덮어쓸 수 없으므로, 그때는 시각을 붙인 이름으로 저장한다
$tmp = "$out\dist.zip.tmp.zip"
Compress-Archive -Path "$dist\*" -DestinationPath $tmp -Force
try {
  Move-Item $tmp $zip -Force -ErrorAction Stop
} catch {
  $zip = $zip -replace "\.zip$", "-$(Get-Date -Format 'yyyyMMdd-HHmmss').zip"
  Move-Item $tmp $zip -Force
  Write-Warning "previous zip is in use, saved as a new file"
}
Write-Host "OK -> $zip"
