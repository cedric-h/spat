cmake_minimum_required(VERSION 3.16)
project(spat)

# set the output directory for built objects.
# This makes sure that the dynamic library goes into the build directory automatically.
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/$<CONFIGURATION>")

# This assumes the SDL source is available in vendored/SDL
set(SDL_TESTS OFF)
set(SDL_EXAMPLES OFF)
set(SDL_AUDIO OFF)
# set(SDL_JOYSTICK OFF)
set(SDL_HAPTIC OFF)
set(SDL_SENSOR OFF)
set(SDL_CAMERA OFF)
set(SDL_POWER OFF)
set(SDL_HIDAPI OFF)
set(SDL_GPU OFF)
set(SDL_RENDER OFF)
add_subdirectory(vendored/SDL EXCLUDE_FROM_ALL)

# Create your game executable target as usual
add_executable(spat WIN32 main.c)

# Link to the actual SDL3 library.
target_link_libraries(spat PRIVATE SDL3::SDL3)
