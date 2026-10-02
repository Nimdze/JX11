# Source layout

Folders group files by role. The most important question when reading this
codebase is "does this run on the audio thread?", so each file's thread
affinity is called out below.

## Folders

| Folder    | Role                                                        |
|-----------|-------------------------------------------------------------|
| `common/` | Pure helpers, constants, tuning math. No state.             |
| `model/`  | Parameter definitions, presets, and the engine's plain state structs. |
| `dsp/`    | The audio signal path.                                      |
| `plugin/` | The JUCE plugin shell that bridges host/MIDI to the engine. |

Includes are subfolder-qualified (`#include "dsp/Synth.h"`) so a file's layer is
visible at every use site. `Source/` is on the include path for both the plugin
and the test target.

## Thread affinity

### Audio thread only

Called from `PluginProcessor::processBlock`. Must never allocate, lock, or touch
JUCE parameter objects.

- `dsp/Synth.{h,cpp}` — block setup, per-sample mixing, `updateLFO()`, MIDI dispatch.
- `dsp/Voice.h` — per-voice `render()`, the hottest path.
- `dsp/VoiceAllocator.h` — note on/off, mono queue, voice stealing (note rate, still audio thread).
- `dsp/Oscillator.h`, `dsp/SinOscillator.h`, `dsp/Filter.h`, `dsp/Envelope.h`,
  `dsp/LFO.h`, `dsp/NoiseGenerator.h` — per-sample DSP primitives (header-inline
  so they inline into the loop).
- `dsp/Utils.h` — `protectYourEars()`, output guarding.

### Message thread only

- `plugin/PluginEditor.*` — the UI.
- `model/Preset.*` — the factory bank, built once in the processor constructor.

### Both / boundary

- `plugin/PluginProcessor.*` — `processBlock` is audio-thread; the constructor,
  `timerCallback`, `setCurrentProgram`, and get/setState are message-thread.
  Cross-thread communication is only through the atomics in the
  "Cross-thread handshakes" section of `PluginProcessor.h`, plus the APVTS.
- `model/Parameters.h` — `createParameterLayout()` runs at construction
  (message thread); the `apply*` functions run on the audio thread from
  `PluginProcessor::update()` (once per block).
- `model/ParameterList.h` — enum + ids only, no code.
- `model/SynthParams.h` — plain config struct, written on the audio thread by
  `update()` and read by the engine.
- `model/MidiState.h` — runtime MIDI state, mutated on the audio thread.

### Any thread

- `common/Constants.h`, `common/Modulation.h` — pure functions.
