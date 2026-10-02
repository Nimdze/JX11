# JX11.5

[![CI](https://github.com/Nimdze/JX11/actions/workflows/ci.yml/badge.svg)](https://github.com/Nimdze/JX11/actions/workflows/ci.yml)

A subtractive synthesizer plug-in (Standalone / AU / VST3) written in C++17 with
JUCE.

This is a personal learning project. It started from the JX11 synth built in
*Creating Synthesizer Plug-Ins with C++ and JUCE* by Matthijs Hollemans, and I
reorganized and extended it: a layered engine, a real test suite, sanitizer and
plug-in validation in CI, and a custom UI.

## What's different from the book's JX11

- **Layered source tree** — `common/` → `model/` → `dsp/` → `plugin/`, with the
  engine built as a `JX11_engine` static library shared by the plug-in and the
  tests, so the tested DSP is the shipped DSP.
- **Single source of truth for parameters** — one canonical list drives the APVTS
  layout, presets, the UI, and the parameter→engine mapping.
- **Audio-thread discipline** — no allocation or locks on the audio thread,
  explicit atomics for cross-thread handshakes, and one-pole smoothing for
  continuous parameters.
- **A few fixes over the book's code** — the filter cutoff is clamped below
  Nyquist, the first-note filter-modulation sweep is gone, the reported tail
  length is realistic, and the selected program survives save/load.
- **Tests** — unit tests for voice allocation (legato, sustain, stealing), every
  parameter mapping, oscillators, the filter, envelopes, LFO, noise and processor
  state, plus an offline-render regression test.
- **CI** — Debug/Release × arm64/x86_64, AddressSanitizer + UndefinedBehaviorSanitizer,
  ThreadSanitizer, `pluginval` at strictness 10, and `auval`.
- **Custom UI** — a parameter grid generated from the parameter table, an
  editor-scoped look-and-feel, and a learnable MIDI resonance CC.

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

- [`ARCHITECTURE.md`](ARCHITECTURE.md) — layers, signal flow, and thread model.
- [`Source/README.md`](Source/README.md) — file-by-file thread affinity.

## Troubleshooting

- **"generator does not match the generator used previously"** — the build
  directory was configured with a different generator. Use a fresh directory
  (e.g. `build-xcode`) or delete `CMakeCache.txt` and `CMakeFiles/`.
- **C++ IntelliSense can't find includes** — re-run configure after adding or
  moving source files; `compile_commands.json` and `JuceHeader.h` are generated
  at configure/build time.

## Credits

Based on the JX11 synthesizer from *Creating Synthesizer Plug-Ins with C++ and
JUCE* by Matthijs Hollemans, which in turn is based on Paul Kellett's MDA JX10.

## License

GPL-3.0-or-later, because it is built on JUCE (GPLv3/commercial). See
[LICENSE](LICENSE) for the full text.
