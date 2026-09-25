# JX11

[![CI](https://github.com/Nimdze/JX11/actions/workflows/ci.yml/badge.svg)](https://github.com/Nimdze/JX11/actions/workflows/ci.yml)

A subtractive synthesizer built with JUCE. Formats: Standalone, AU, VST3.

## Prerequisites

- macOS
- CMake 3.22 or newer
- A C++17 compiler: Xcode Command Line Tools (`xcode-select --install`) or Xcode

JUCE is downloaded automatically at configure time (pinned to 8.0.12) via CMake
`FetchContent` — no manual setup required.

## Getting started

    git clone https://github.com/Nimdze/JX11.git
    cd JX11

    # Enable the pre-commit hook (builds + runs tests before each commit)
    git config core.hooksPath .githooks

## Configure

Use a dedicated build directory. The generator is locked in on first configure,
so don't reuse a directory configured with a different generator.

    cmake -B build-vscode -G "Unix Makefiles"

## Build

    cmake --build build-vscode --target JX11_All -j8

Targets:

- `JX11_All` — all formats + standalone
- `JX11_Standalone`, `JX11_AU`, `JX11_VST3`

## Test

    cmake --build build-vscode --target JX11Tests -j8
    ctest --test-dir build-vscode --output-on-failure

## Outputs

Built artefacts live under `build-vscode/Source/JX11_artefacts/Debug/`.
Because `COPY_PLUGIN_AFTER_BUILD` is enabled, AU/VST3 are also copied to:

- `~/Library/Audio/Plug-Ins/Components/JX11.component`
- `~/Library/Audio/Plug-Ins/VST3/JX11.vst3`

Run the standalone from:

    open build-vscode/Source/JX11_artefacts/Debug/Standalone/JX11.app

## Troubleshooting

- **"generator does not match the generator used previously"** — the build
  directory was configured with a different generator. Use a fresh directory
  (e.g. `build-xcode`) or delete `CMakeCache.txt` and `CMakeFiles/`.
- **C++ IntelliSense can't find includes** — re-run configure after adding or
  moving source files; `compile_commands.json` and `JuceHeader.h` are generated
  at configure/build time.

## License

This project is licensed under the GNU General Public License v3.0 or later
(SPDX: `GPL-3.0-or-later`), because it is built on JUCE (GPLv3/commercial).
See [LICENSE](LICENSE) for the full text.
