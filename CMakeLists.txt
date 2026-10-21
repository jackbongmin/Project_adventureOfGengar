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
  if(TARGET SDL2::SDL2main)
    target_link_libraries(gengar PRIVATE SDL2::SDL2main SDL2::SDL2)
  else()
    target_link_libraries(gengar PRIVATE SDL2::SDL2)
  endif()
  if(MSVC)
    target_compile_options(gengar PRIVATE /W4 /permissive-)
  else()
    target_compile_options(gengar PRIVATE -Wall -Wextra)
  endif()
endif()
