# Architecture

This document describes how JX11.5 is put together: the layers, the signal path,
and — most importantly — which thread each piece of code runs on. For a
file-by-file breakdown see [`Source/README.md`](Source/README.md).

## Build targets

| Target | CMake | Contents | Links |
|---|---|---|---|
| `JX11_engine` | `Source/CMakeLists.txt` | `common/`, `model/`, `dsp/` | JUCE core/audio_basics/audio_processors |
| `JX11` | `Source/CMakeLists.txt` | `plugin/` (processor + editor + look-and-feel) | `JX11_engine` + JUCE audio/GUI |
| `JX11Tests` | `Tests/CMakeLists.txt` | `Tests/` + `PluginProcessor.cpp` (headless) | `JX11_engine` |

The audio engine is a static library compiled **once** and linked by both the
shipping plugin and the test binary, so the tested DSP is the shipped DSP.

## Layers

```
common/   pure helpers and tuning math        (no state, any thread)
model/    parameters, presets, engine config  (plain data + APVTS glue)
dsp/      the signal path                      (audio thread)
plugin/   JUCE host shell and UI               (message + audio thread)
```

Includes are subfolder-qualified (`#include "dsp/Synth.h"`) so the layer is
visible at every use site.

## Signal flow

```
 host MIDI ─► PluginProcessor::processBlock
                 │
                 ├─ update()               APVTS params ─► SynthParams (once per block,
                 │                         or whenever a parameter changes)
                 │
                 └─ splitBufferByEvents()  sample-accurate MIDI
                        │
                        ├─ handleMIDI() ─► Synth::midiMessage() ─► VoiceAllocator
                        │                                            (note on/off, legato,
                        │                                             sustain, stealing)
                        └─ render() ─────► Synth::render()
                                               │
                                               ├─ updateLFO()  every 32 samples
                                               │
                                               └─ per sample, per active voice:
                                                     Osc1 ──┐
                                                            ├─► saw ─► Filter ─► Amp Env ─┐
                                                     Osc2 ──┘         ▲                   ├─► output
                                                                      │                   │
                                                     Filter Env ──────┘                   │
                                                     (key track + LFO + pressure + CC)  ◄─┘
```

`Synth::render` mixes all active voices, applies the smoothed output gain, and
runs the sample guard before returning.

## Thread model

### Audio thread
`PluginProcessor::processBlock` and everything it calls:
`splitBufferByEvents`, `handleMIDI`, `render`, `update`, and the whole `dsp/`
tree. Rules: no allocation, no locks, no JUCE parameter objects, only plain data
and atomics.

### Message thread
Constructor, `setCurrentProgram`, `get/setStateInformation`, `timerCallback`,
the editor and look-and-feel, and factory-preset construction.

### The boundary
The two threads only communicate through the atomics declared in the
"Cross-thread handshakes" section of `PluginProcessor.h`:

| Atomic | Direction | Purpose |
|---|---|---|
| `parametersChanged` | message → audio | APVTS changed, re-read params |
| `pendingProgram` | audio → message | program-change CC, applied by the timer |
| `resetRequested` | message → audio | reset envelopes/voices on program load |
| `pendingOutputLevel` | audio → message | CC 7 volume, pushed back to the parameter |
| `midiLearn` / `midiLearnCC` | message ↔ audio | capture a CC for filter resonance |

The APVTS itself is the other shared channel: the audio thread reads raw atomic
parameter pointers, the message thread owns the `ValueTree`.

## Parameter data flow

```
ParameterList.h   enum + ids (canonical order, JUCE-free)
      │
Parameters.h      kSpecs[]: range, default, choices, display, apply()
      │
      ├─ createParameterLayout() ─► APVTS            (message thread, once)
      └─ apply* (p, value, ctx)  ─► SynthParams       (audio thread, per change)
                                        │
                                finalizeParams()      (cross-field: volumeTrim,
                                        │              tune, detune)
                                        ▼
                                   Synth::setParam()
```

Adding a parameter means adding one row to the `JX11_PARAM_LIST`, one row to
`kSpecs[]`, and (optionally) a preset override. The UI builds itself from
`kSpecs[]`.

## Tests

`JX11Tests` links `JX11_engine` and compiles `PluginProcessor.cpp` with
`JX11_HEADLESS`, which makes `createEditor()` return `nullptr` so the UI
translation units stay out of the console app. `GoldenRenderTests` pins the
offline output of the DSP to a per-window RMS fingerprint; regenerate it
deliberately when the DSP changes on purpose.
