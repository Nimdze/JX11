cmake_minimum_required(VERSION 3.22)

# Build universal binaries on macOS (Apple Silicon + Intel). Without this, CMake
# defaults to the architecture of the cmake binary itself, which is often
# Intel-only (e.g. Homebrew's /usr/local) even on Apple Silicon machines.
if(CMAKE_HOST_APPLE AND NOT CMAKE_OSX_ARCHITECTURES)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "macOS architectures" FORCE)
endif()

project(JX11 VERSION 1.0.0 LANGUAGES C CXX)

# Optional sanitizers for hardening builds, e.g.
#   cmake -B build-asan -DJX11_SANITIZERS=address,undefined
#   cmake -B build-tsan -DJX11_SANITIZERS=thread
# Kept as a cache variable so CI and contributors configure them the same way.
set(JX11_SANITIZERS "" CACHE STRING "Comma-separated sanitizers (address,undefined or thread)")
if(JX11_SANITIZERS)
    add_compile_options(-fsanitize=${JX11_SANITIZERS} -fno-omit-frame-pointer -g -fno-sanitize-recover=all)
    add_link_options(-fsanitize=${JX11_SANITIZERS})
endif()

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

enable_testing()

include(FetchContent)

FetchContent_Declare(
    JUCE
    GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
    GIT_TAG        8.0.12
    GIT_SHALLOW    TRUE
)

FetchContent_MakeAvailable(JUCE)
add_subdirectory(Source)
add_subdirectory(Tests)

