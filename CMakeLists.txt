project(hi)
cmake_minimum_required(VERSION 4.3)

add_executable(${PROJECT_NAME} main.c)

set(MINIFB_BUILD_EXAMPLES OFF)
add_subdirectory(dependencies/minifb)
target_link_libraries(${PROJECT_NAME} minifb)
