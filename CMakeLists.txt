cmake_minimum_required(VERSION 3.16)
project(spat)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

set(SDL_TESTS        OFF)
set(SDL_EXAMPLES     OFF)
set(SDL_AUDIO        OFF)
set(SDL_VIDEO        ON )
set(SDL_GPU          OFF)
set(SDL_RENDER       ON )
set(SDL_CAMERA       OFF)
set(SDL_JOYSTICK     ON )
set(SDL_HAPTIC       OFF)
set(SDL_HIDAPI       OFF)
set(SDL_POWER        OFF)
set(SDL_SENSOR       OFF)
set(SDL_DIALOG       OFF)
set(SDL_TRAY         OFF)
set(SDL_NOTIFICATION ON )
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

add_executable(spat WIN32 main.c)
if(MSVC)
    target_compile_options(spat PRIVATE /W4 /WX)
else()
    target_compile_options(spat PRIVATE
        -Wall
        -Wextra
        -Werror
        -Wno-unused-parameter
    )
endif()

target_link_libraries(spat PRIVATE SDL3::SDL3)
