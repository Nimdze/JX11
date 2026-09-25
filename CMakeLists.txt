cmake_minimum_required(VERSION 3.22)

# Build universal binaries on macOS (Apple Silicon + Intel). Without this, CMake
# defaults to the architecture of the cmake binary itself, which is often
# Intel-only (e.g. Homebrew's /usr/local) even on Apple Silicon machines.
if(CMAKE_HOST_APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures" FORCE)
endif()

project(JX11 VERSION 1.0.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

add_subdirectory(juce)
add_subdirectory(Source)
add_subdirectory(Tests)

