cmake_minimum_required(VERSION 3.20)
project(Orbital LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(raylib REQUIRED)

add_executable(${PROJECT_NAME}
  src/main.cpp
  src/Engine.cpp
  src/Engine3D.cpp
)

target_link_libraries(${PROJECT_NAME} PRIVATE raylib)
