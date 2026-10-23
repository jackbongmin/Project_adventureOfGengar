cmake_minimum_required(VERSION 3.16)
project(AdventureOfGengar LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Source/Private 아래의 모든 .cpp 를 빌드한다. (헤더는 Source/Public, 내부 전용 헤더는 Source/Private)
file(GLOB_RECURSE GENGAR_SOURCES CONFIGURE_DEPENDS "${CMAKE_SOURCE_DIR}/Source/Private/*.cpp")

add_executable(gengar ${GENGAR_SOURCES})
target_include_directories(gengar PRIVATE
  "${CMAKE_SOURCE_DIR}/Source/Public"
  "${CMAKE_SOURCE_DIR}/Source/Private")

if(EMSCRIPTEN)
  # 웹 빌드: SDL2 는 Emscripten 포트를 사용한다.
  target_compile_options(gengar PRIVATE -sUSE_SDL=2)
  target_link_options(gengar PRIVATE
    -sUSE_SDL=2 -sALLOW_MEMORY_GROWTH=1
    --shell-file "${CMAKE_SOURCE_DIR}/web/shell.html")
  set_target_properties(gengar PROPERTIES SUFFIX ".html")
else()
  # 네이티브 빌드: SDL2 개발 라이브러리가 필요하다. (예: vcpkg install sdl2)
  find_package(SDL2 CONFIG REQUIRED)
  # 공유 SDL2(SDL2::SDL2)가 없으면 정적 SDL2(SDL2::SDL2-static)를 쓴다.
  target_link_libraries(gengar PRIVATE
    $<TARGET_NAME_IF_EXISTS:SDL2::SDL2main>
    $<IF:$<TARGET_EXISTS:SDL2::SDL2>,SDL2::SDL2,SDL2::SDL2-static>)
  set_target_properties(gengar PROPERTIES OUTPUT_NAME AdventureOfGengar)
  if(WIN32)
    target_sources(gengar PRIVATE "${CMAKE_SOURCE_DIR}/res/windows/gengar.rc")  # 보물 모자 아이콘
  endif()
  # 배포용 빌드(scripts/build_win.ps1): vcpkg x64-windows-static 과 함께 쓰면 SDL2 와 C++ 런타임이
  # exe 안에 들어가서, 받는 사람은 Visual Studio 나 DLL 없이 exe 하나로 실행할 수 있다.
  option(GENGAR_DIST "Static C++ runtime and no console window" OFF)
  if(GENGAR_DIST)
    set_target_properties(gengar PROPERTIES
      MSVC_RUNTIME_LIBRARY "MultiThreaded$<$<CONFIG:Debug>:Debug>"
      WIN32_EXECUTABLE ON)
  endif()
  if(MSVC)
    target_compile_options(gengar PRIVATE /W4 /permissive- /utf-8)  # 소스는 UTF-8 (한글 주석)
  else()
    target_compile_options(gengar PRIVATE -Wall -Wextra)
  endif()
endif()
