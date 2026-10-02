# JX11.5

[![CI](https://github.com/Nimdze/JX11/actions/workflows/ci.yml/badge.svg)](https://github.com/Nimdze/JX11/actions/workflows/ci.yml)

A subtractive synthesizer plug-in (Standalone / AU / VST3) written in C++17 with
JUCE. JX11.5 is a **modernization** of the JX11 synth from
*Creating Synthesizer Plug-Ins with C++ and JUCE* by Matthijs Hollemans: the same
synthesizer engine, reorganized and hardened into a production-shaped codebase
with a real test suite, CI, and documentation.

The original JX11 was written as a teaching project — a single flat source tree
with no tests. JX11.5 keeps the audio behaviour recognizably the same and rebuilds
everything around it: architecture, real-time discipline, tests, and docs.

## What JX11.5 adds over the book's JX11

- **Layered engine.** `common/` → `model/` → `dsp/` → `plugin/`, with the engine
  extracted into a `JX11_engine` static library that the shipping plug-in and the
  test binary both link. The tested DSP is the shipped DSP.
- **Single source of truth for parameters.** One canonical parameter list drives
  the APVTS layout, presets, the UI, and the parameter→engine mapping.
- **Real-time discipline.** No allocation, locks, or JUCE parameter objects on the
  audio thread; explicit atomics for cross-thread handshakes; one-pole smoothing
  for continuous parameters; every DSP type initialized.
- **Fixes over the book's code.** Filter cutoff is clamped below Nyquist (safe at
  32 kHz/22 kHz/offline rates), the first-note filter-modulation sweep is gone,
  `getTailLengthSeconds()` reports a realistic tail, and the selected program
  index survives save/load.
- **Test suite.** 100+ `juce::UnitTest` cases covering voice allocation (legato,
  sustain, stealing), every parameter-mapping function, the filter, envelopes,
  LFO, noise, processor state, and a **golden offline-render regression** that
  pins the DSP output.
- **CI.** Debug/Release × arm64/x86_64, AddressSanitizer + UndefinedBehaviorSanitizer,
  ThreadSanitizer, `pluginval` at strictness 10, and `auval`.
- **Custom UI.** A parameter grid generated from the parameter table, an
  editor-scoped look-and-feel, and a learnable MIDI resonance CC.

## What this project demonstrates

For a reviewer, the interesting parts are less the synth features than the
engineering around them:

- Reading and refactoring an existing real-time DSP codebase without changing its
  sound, then proving it with a golden test.
- Understanding audio-thread constraints and designing thread handshakes around
  them (`ARCHITECTURE.md` documents the thread model).
- Writing tests that can actually fail — including a deliberately regenerated
  regression fingerprint and a filter-modulation fix validated by reverting it.
- Setting up modern C++/CMake tooling: FetchContent, a shared engine library,
  sanitizers, and a matrix CI pipeline.

## The synthesizer

- Two band-limited sawtooth oscillators with tune, fine-tune, and mix
- Resonant low-pass state-variable filter with its own ADSR and velocity/key tracking
- Loudness ADSR envelope
- LFO for vibrato / PWM / filter modulation
- Glide (portamento), poly/mono with legato, sustain pedal, voice stealing
- Noise generator, tuning, output level
- Learnable MIDI CC for filter resonance
- 53 factory presets

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

Hardening builds use the `JX11_SANITIZERS` cache variable:

    cmake -B build-asan -DJX11_SANITIZERS=address,undefined
    cmake -B build-tsan -DJX11_SANITIZERS=thread

## Validation

Build the AU/VST3 and validate them:

    auval -v aumu Jx11 Nimd
    /Applications/pluginval.app/Contents/MacOS/pluginval --strictness-level 10 \
        --validate build-vscode/Source/JX11_artefacts/Debug/VST3/JX11.vst3

## Outputs

Built artefacts live under `build-vscode/Source/JX11_artefacts/Debug/`.
Because `COPY_PLUGIN_AFTER_BUILD` is enabled, AU/VST3 are also copied to:

- `~/Library/Audio/Plug-Ins/Components/JX11.component`
- `~/Library/Audio/Plug-Ins/VST3/JX11.vst3`

Run the standalone from:

    open build-vscode/Source/JX11_artefacts/Debug/Standalone/JX11.app

## Documentation

- [`ARCHITECTURE.md`](ARCHITECTURE.md) — layers, signal flow, thread model, build targets.
- [`Source/README.md`](Source/README.md) — file-by-file thread affinity.
- [`MODERNIZATION.md`](MODERNIZATION.md) — what JX11.5 changes and why.
- [`CHANGELOG.md`](CHANGELOG.md) — release history.
- [`DEFERRED.md`](DEFERRED.md) — intentionally postponed work.

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
