# Changelog

All notable changes to JX11.5 are recorded here. The format follows
[Keep a Changelog](https://keepachangelog.com/en/1.1.0/), and the project aims
for [Semantic Versioning](https://semver.org/).

## [Unreleased] — JX11.5 modernization

Work in progress; see [MODERNIZATION.md](MODERNIZATION.md) for the roadmap.

### Added
- Architecture documentation, changelog, editor config.
- Shared `JX11_engine` library used by both the plugin and the tests.
- Tests for voice allocation, every parameter `apply*` mapping, `processBlock`
  output, and a golden offline render.
- Custom editor with an editor-scoped look-and-feel and a MIDI Learn button.

### Changed
- MIDI resonance CC is now learnable and persisted with plugin state.
- Parameter display (`lfoRateToText`) shares the DSP curve.

### Fixed
- Removed a never-failing test assertion (tolerance `1.0e06`).
- Tightened the resonant-filter stability bound and the envelope tests.

## [1.0.0] — book implementation

The JX11 synthesizer from *Creating Synthesizer Plug-Ins with C++ and JUCE*:
two BLIT sawtooth oscillators, resonant SVF low-pass filter, two ADSR
envelopes, LFO, glide, vibrato/PWM, noise, polyphony with voice stealing and
mono legato, factory presets, and full APVTS parameter handling.
