# JX11.5 — Modernization Plan

> Working document for turning the book's JX11 into **JX11.5**, a portfolio-quality
> modernization of the original synthesizer. It records every issue found during a
> full read of the codebase, groups them by the five goals, and then gives a
> dependency-ordered plan so the work can be done safely.
>
> Status: planning / audit. Nothing in the source has been changed to satisfy this
> document yet.

## Decisions log

Calls made by the author while reviewing the audit. Where a decision contradicts
an earlier finding below, the decision wins.

| Topic | Decision |
|---|---|
| Commit the in-flight refactor | **Yes** — done in Phase 0 |
| `.kilo/` worktrees | **Delete**; ignore `.kilo/` |
| Plugin identity placeholders (`yourcompany`, `Manu`) | **Fine, leave as-is** |
| SPDX/copyright headers on every file | **No** — not adding; dropped from scope |
| Goal 1 (organization) | In scope |
| Goal 2 (modern audio standards) | In scope |
| Goal 3 (test quality) | In scope, but see below |
| Dedicated `Oscillator` / `NoiseGenerator` tests | **Skip for now** — those implementations may be replaced; `VoiceAllocator` and `processBlock` tests are the priority |
| Goal 4 (DSP modernization) | In scope; if the oscillator changes, use **BLEP** (not wavetable) |
| Goal 5 (new features) | **Out of scope** — focus on organization + modernization, avoid feature creep |

---

## 0. The five goals

| # | Goal | Type |
|---|------|------|
| 1 | Organization and clarity | required |
| 2 | Modern audio coding standards | required |
| 3 | Test quality (not tautological / never-failing) | required |
| 4 | Modernize the DSP | optional |
| 5 | Add optional features listed in the book | **out of scope** (focus on 1–4) |

Two constraints frame everything:

- **Do not change the sound (yet).** Goals 1–3 should be behaviour-preserving.
  Reach a green, tested baseline first, then change DSP (goal 4) with tests
  guarding the change, then add features (goal 5) as additive work.
- **The repo is the CV artifact.** Every phase should end in a clean commit, a
  green CI run, and a readable history. The git log is evidence of process.

---

## 1. Current state — what is already good

Worth keeping and highlighting, because a reviewer will notice these:

- Layered source tree (`common/` → `model/` → `dsp/` → `plugin/`) with subfolder-
  qualified includes.
- `Source/README.md` documents **thread affinity per file** — a strong signal of
  RT awareness.
- Single source of truth for parameters (`ParameterList.h` enum + `Parameters.h`
  spec table + `kSpecs[]`), preset/program bank, APVTS, and master state save/load.
- Explicit cross-thread handshakes (`parametersChanged`, `pendingProgram`,
  `resetRequested`, MIDI-Learn) documented in `PluginProcessor.h`.
- Custom DSP: BLIT saw oscillator, SVF filter, ADSR, LFO, noise, voice allocator
  with legato, sustain, stealing.
- 11 `juce::UnitTest` files, a pre-commit hook that builds + runs tests, ASan
  build tree, pluginval/auval done manually, GPL/SPDX headers (mostly), CI.
- `DEFERRED.md` shows deliberate prioritization.

The gaps below are mostly about *finishing* this trajectory, not restarting it.

---

## 2. Findings by goal

Legend: **[blocker]** must fix before submitting · **[high]** strongly recommended ·
**[med]** nice to have · **[low]** polish.

### 2.1 Organization and clarity

**O-1 [blocker] Large uncommitted refactor is sitting in the working tree.**
`git status` shows 12 modified files plus new `LookAndFeel.{h,cpp}`. Branch
`organization_cleanup` also exists. For a submission the tree must be committed
in logical, green commits. First action of Phase 0.

**O-2 [blocker] A full duplicate of the pre-refactor source tree is present.**
`.kilo/worktrees/elderly-engine/` contains the old flat `Source/` layout, an old
`README.md`, `DEFERRED.md`, etc. It is untracked but not ignored, so it will show
up in every `git status`, confuse `rg`, and look sloppy. Delete it and add `.kilo/`
to `.gitignore`.

**O-3 [wontfix] Placeholder plugin identity.** `Source/CMakeLists.txt` still has
`COMPANY_NAME "yourcompany"`, `PLUGIN_MANUFACTURER_CODE Manu`. **Decision: leave
as-is.** No change planned.

**O-4 [high] `Tests/CMakeLists.txt` recompiles production `.cpp` files.**
Tests build `Synth.cpp`, `Preset.cpp`, `PluginProcessor.cpp` directly and hand-
define `JucePlugin_*` macros. This duplicates compilation and can silently diverge
from the plugin build. Extract a `JX11_core` (DSP + model) static library and a
`JX11_plugin` shell; link the test target against `JX11_core`. Let `juce_add_plugin`
supply the macros to the plugin target only.

**O-5 [med] Docs are split and partially stale.** `README.md`, `Source/README.md`,
and `DEFERRED.md` overlap. Add a top-level `ARCHITECTURE.md` (signal flow, thread
model, data flow), keep `README.md` for build/run, and make `DEFERRED.md` a live
tracker (see §3). Add `CHANGELOG.md` and a `docs/` folder for a screenshot and an
audio demo link — CV reviewers will look for them.

**O-6 [wontfix] License-header inconsistency.** `Source/model/Preset.cpp`,
`Tests/ProcessorTests.cpp`, `Tests/SinOscillatorTests.cpp` lack the
`Copyright`/`SPDX` line the other files carry. **Decision: do not add headers**
and do not spend time on the inconsistency.

**O-7 [med] Machine-specific editor config is tracked.** `.vscode/c_cpp_properties.json`
hardcodes `macos-clang-arm64` and `.vscode/settings.json` hardcodes
`/usr/bin/clang` + `build-vscode`. Either generalise or document, and add an
`.editorconfig`.

**O-8 [low] Style drift.** `Source/model/SynthParams.h` has odd indentation and
trailing whitespace; `DEFERRED.md` notes the `SpaceBeforeParens:
NonEmptyParentheses` quirk. Run `clang-format` across the tree once and decide on
the policy.

**O-9 [low] Magic numbers.** `0.005f` (filter smoothing), `0.997f` (saw leak),
`0.75f`, `0.0008f`, etc. are unnamed. Extract named constants with a comment tying
each to its parameter mapping so the DSP is self-documenting.

### 2.2 Modern audio coding standards

**A-1 [blocker] Uninitialized DSP state = UB.**
`Voice` declares `period`, `target`, `glideRate`, `cutoff`, `filterMod`, `filterQ`,
`pitchBend`, `filterEnvDepth` with no initializer; `Oscillator` leaves `amplitude`
and `phaseMax` unset until `reset()`/`startVoice`; `NoiseGenerator::noiseSeed` is
unset until `reset()`; `SinOscillator` leaves `amplitude/inc/phase` unset. A
default-constructed `Synth`/`Voice` that renders before a full `reset()` reads
garbage. (The old `Voice::velocity` uninitialised-read bug recorded in
`DEFERRED.md` is the same class of bug.) Give every member a safe default and/or
`= {}` the arrays.

**A-2 [high] Filter cutoff is not clamped to Nyquist.**
`Voice.h:87` clamps to `[30, 20000]`. At sample rates ≤ ~40 kHz (32000, 22050,
offline renders) 20 kHz exceeds Nyquist; `Filter::updateCoefficients` then computes
`std::tan(PI * cutoff / sampleRate)` past π/2, which goes **negative**, producing
invalid/unstable coefficients. Clamp to `~0.45 * sampleRate`.

**A-3 [high] The known `filterZip` start-up sweep bug is still present.**
`Synth.cpp:43` resets `filterZip = 0.0f`, then `Synth.cpp:229` slews it toward the
real modulation value with `0.005f` per LFO step. Result: an audible ~100–150 ms
filter sweep on the first note. The book calls this out in Ch. 14. Initialize
`filterZip` to the current target (`params.filterKeyTracking + midi.filterCtl`)
when parameters change / on reset, or switch to a properly initialised smoother.

**A-4 [high] No parameter smoothing except output gain.**
`SynthParams` is copied once per block and read per sample for `filterQ`,
`oscMix`, `detune`, `volumeTrim`, `vibrato`, etc. Host automation therefore steps
at block boundaries → zipper noise. Add one-pole smoothing (in the audio thread,
allocation-free) for at least cutoff-related and gain-related values, or document
an explicit decision not to.

**A-5 [med] `protectYourEars` is always compiled in.**
The safety pass runs a per-sample branch loop on every buffer even in release
builds; the book explicitly suggests disabling it in release. Make it a
compile-time/debug-only switch (e.g. `JX11_ENABLE_SAMPLE_GUARD`) while keeping the
NaN/inf detection tested.

**A-6 [med] `getTailLengthSeconds()` returns 0 despite a long release.**
The env release can be several seconds (`applyEnvRelease`), so hosts may truncate
the tail on offline bounce. Return a realistic value derived from the release
parameters (or a safe constant).

**A-7 [med] `currentProgram` is not saved/restored.**
Already in `DEFERRED.md`. `setStateInformation` restores parameters but not the
program index, so `getCurrentProgram()` is stale after load. Store it in the
`<EXTRA>` XML element next to the learned CC.

**A-8 [med] Real-time-safety audit should be explicit and tested.**
The code looks allocation-free on the audio path, but there is no guard. Add a
documented rule + optionally an `allocation` assertion / TSan run (goal 3/CI),
and keep the cross-thread handshake table current.

**A-9 [low] Encapsulation / const-correctness.**
`Synth::outputLevelSmoother` and `Synth::midi` are public mutable; hot getters are
not `noexcept`/`[[nodiscard]]`. Tighten accessors and mark pure DSP `noexcept`.

**A-10 [low] Use JUCE facilities where they reduce risk.**
`common/Constants.h` defines a private `PI`; prefer `juce::MathConstants<float>::pi`.
Consider `juce::dsp` (SVF/filters, `FloatVectorOperations`, `AudioProcessLoadMeasurer`
for goal 4) and `juce::ScopedNoDenormals` (already used at `processBlock`).

### 2.3 Test quality

The suite is a good start but has one provably never-failing assertion, a whole
untested DSP class, and several weak tests.

**T-1 [blocker] Tautological assertion.**
`Tests/ModulationTests.cpp:23`:
```cpp
expectWithinAbsoluteError (synth.midi.modWheel, 0.0f, 1.0e06f);
```
The tolerance is `1,000,000`. This can never fail. It should be `1.0e-6f` (the
intent is visible two lines later). Sweep the suite for other huge tolerances.

**T-2 [deferred] No tests for the oscillator that the synth actually uses.**
`SinOscillator` is tested, but `Oscillator` (the BLIT sawtooth) has **zero** direct
tests. **Decision: skip for now** — the implementation may be replaced during goal 4,
so writing tests against the current BLIT would be churn. Revisit after the BLEP
work (see D-1), when the final oscillator must get full coverage.

**T-3 [deferred] No `NoiseGenerator` tests.** The book explicitly suggests them
(range, variation, reproducibility). **Decision: skip for now** for the same reason
— the generator may be replaced; test it once it is final.

**T-4 [high] No `VoiceAllocator` tests.** Voice count, stealing choice, mono
legato queue (`shiftQueuedNotes`/`nextQueuedNote`), sustain, and `SUSTAIN`
sentinel are only exercised indirectly through `Synth`. This is the most
error-prone code in the project.

**T-5 [high] No `processBlock` output test.** Already in `DEFERRED.md`. Add
`prepareToPlay` → feed a note-on → `processBlock` → assert finite, non-silent,
correct channel count, and silence after note-off.

**T-6 [high] Parameter mapping (`apply*`) is untested.**
`Parameters.h` contains all the conversion curves (`applyEnvAttack`,
`applyFilterReso`, `applyLfoRate`, `applyVibrato`, `applyPolyMode`, …). Add
monotonicity/endpoint tests and consistency checks that `toText` matches the same
curve the DSP uses (e.g. `lfoRateToText` vs `lfoRateHz`). This is where regressions
hide.

**T-7 [med] Weak / loose assertions.**
- `FilterTests` "stays finite" allows `|y| < 1.0e4` — it would pass a badly
  unstable filter. Tighten or rename to an explicit stability bound.
- Gain-at-cutoff tolerance is ±10%; acceptable but should be justified.
- Many `Synth` tests only assert `!isSilent`; add spectral/RMS expectations.

**T-8 [med] Envelope tests are slow and loosely specified.**
`advance(env, 100000..1000000)` with `0.99` multipliers is both slow and imprecise.
Test stages via time-constant math (e.g. level after N samples against
`exp`), plus the zero-attack and zero-release edge cases the book mentions.

**T-9 [med] Preset tests only check ranges.** Add a smoke test that every factory
preset, when loaded, produces finite non-silent output (or is intentionally
silent), and that preset count/names are stable.

**T-10 [med] No golden/regression test.** Add one offline render (MIDI → buffer)
with a stored hash/RMS-per-window, run in Debug and Release, so DSP changes are
caught. This doubles as the "null test" workflow the book describes.

**T-11 [low] Test polish.** Typos in test names ("sattles", "does'nt", "reseat",
"fullt"); `ProcessorTests.cpp` constructs a processor per test (fine) but the
suite never asserts bus-layout rejection, `getTailLengthSeconds`, or
`acceptsMidi`/`producesMidi` flags for a non-synth build.

### 2.4 Modernizing the DSP (optional)

Order these **after** goals 1–3 so tests guard the change.

**D-1 [high] Replace/enhance the sawtooth oscillator.** The current oscillator is
a BLIT saw. **Decision: if it changes, use BLEP** (band-limited step), not a
wavetable. Add an aliasing measurement test. This is the highest-value audible
modernization.

**D-2 [high] Robust, sample-rate-aware filter.** Fix the Nyquist clamp (A-2),
then consider a TPT/ZDF SVF (`juce::dsp::StateVariableTPTFilter` or the current
one with documented stability), and add cutoff/resonance smoothing (A-4).

**D-3 [med] Fix the filter modulation start-up** (A-3) and add a regression test
that the first block equals the steady-state filter response.

**D-4 [med] Block/SIMD processing.** The book discusses restructuring from
sample-by-sample to per-block staging; this enables cache-friendly loops and
auto-vectorization. Prototype behind the existing per-sample path and benchmark.

**D-5 [med] Performance instrumentation.** Add `juce::AudioProcessLoadMeasurer`
or a benchmark harness, and publish a CPU number (e.g. voices × sample rates) in
the README — great CV evidence.

**D-6 [low] Oversampling option** for the oscillator/filter path, and a noise
generator upgrade (better PRNG, documented distribution).

### 2.5 Optional features listed in the book (out of scope)

**Decision: not doing these now.** Kept for reference only — the priority is
organization and modernization, not feature creep. Recorded so the option is not
lost.

From Ch. 14 plus the book's feature discussion. Ranked by value/effort for a
portfolio:

1. **Oscillator combination mode parameter** — subtract (current), add, ring
   modulation, phase modulation, hard sync. High value, localized to `Voice::render`.
2. **Waveform selection** — sine, triangle, wavetable, distorted sine
   (`y = x + x²·x·(r·x² − 1 − r)`), sub-bass sine octave below. Builds directly on
   `SinOscillator`/`Oscillator`.
3. **Supersaw** — 7 detuned saws with distinct start phases per voice.
4. **More filter types** — high-pass, band-pass, notch, and a second filter with
   series/parallel/per-oscillator routing.
5. Third oscillator.

Each new feature must arrive with: a parameter spec row, a preset or two, a unit
test, and an updated `Source/README.md`/`ARCHITECTURE.md`. Features without tests
do not count as "done".

---

## 3. Reconciling `DEFERRED.md`

`DEFERRED.md` is useful but partly stale — two items are already implemented and
one is mis-scoped. Proposed edits (do this in Phase 0/2):

| Deferred item | Reality now | Action |
|---|---|---|
| processBlock output test | still open | Keep; promote to Phase 1 (T-5) |
| pluginval in CI | still open | Keep; Phase 4 |
| pluginval strictness 10 | still open | Keep; Phase 4 |
| TSan build | still open | Keep; Phase 4 |
| MSan | still open | Keep low priority |
| `currentProgram` not restored | still open | Keep; Phase 3 (A-7) |
| Presets / program state | **already implemented** (`getNumPrograms`, get/setState, bank) | Remove/strike through |
| Parameter automation tests "once the synth exposes parameters" | parameters **are** exposed | Rewrite as `apply*` mapping tests (T-6) |
| clang-tidy | still open | Keep; Phase 4 |
| Cache JUCE in CI | still open | Keep; Phase 4 |
| `.editorconfig` | still open | Keep; Phase 2 (O-7) |
| clang-format review | still open | Keep; Phase 2 (O-8) |
| Licensing decision | resolved (GPL-3.0-or-later) | Move to "Done" |
| Code signing / packaging | still open | Keep; packaging |
| CHANGELOG + tags | still open | Keep; Phase 2 |
| Windows/Linux | still open | Keep as stretch |
| Architecture notes "only when outgrows flat Source" | **it has outgrown it** (`common/model/dsp/plugin`) | Do it; Phase 2 (O-5) |
| CONTRIBUTING / issue templates | still open | Optional |

Also fold the **new** findings (A-2/A-3/T-1/T-2/T-3/T-4) into `DEFERRED.md` or
straight into the phase plan, so this document and `DEFERRED.md` don't drift apart.

---

## 4. Proposed order of work

Dependency-ordered. Each phase ends with green tests and one or more clean commits.
Do **not** start a later phase until the earlier one is committed and green.

### Phase 0 — Stabilize and clean the repository  *(goal 1)*
Why first: everything else is diffed against this tree; a reviewer sees it too.

- [x] Commit the in-flight refactor (O-1).
- [x] Delete `.kilo/worktrees/`, ignore `.kilo/` (O-2).
- [x] Plugin identity placeholders left as-is (O-3, decided).
- [x] License headers left as-is (O-6, decided).
- [x] Reconcile `DEFERRED.md` (§3), fix obvious typos/whitespace (O-8).
- [x] `git config core.hooksPath .githooks`; confirm tests pass.

**Done when:** `git status` is clean, CI green, the test suite passes locally,
and no duplicate source tree exists.

### Phase 1 — Restore test integrity  *(goal 3)*
Why before refactoring/DSP: tests are the safety net for the changes that follow.

- [x] Fix the never-failing `1.0e06f` assertion and sweep for similar (T-1).
- [x] Add `VoiceAllocator` tests (T-4) and the `processBlock` output test (T-5) —
      the two priorities.
- [x] Add `apply*` parameter-mapping tests (T-6).
- [ ] (Deferred) `Oscillator`/`NoiseGenerator` tests (T-2/T-3) — do after D-1.
- [x] Tighten the loose filter/envelope assertions (T-7/T-8); fix test typos.
- [x] Add the golden offline-render regression test (T-10).

**Done when:** every DSP class and every `apply*` function has direct coverage,
the suite has at least one test that fails if you intentionally break each core
module, and no assertion has a tolerance larger than the quantity it checks.

_State: 101 test cases, green in Debug; `JX11_All` builds. `Oscillator` and
`NoiseGenerator` remain untested by decision (T-2/T-3)._

### Phase 2 — Structure, docs, build  *(goal 1)*
- [x] Extract `JX11_engine` library; link plugin + tests against it (O-4).
- [x] Add `ARCHITECTURE.md`, `CHANGELOG.md`, `.editorconfig` (O-5/O-7).
      (`docs/` with a screenshot/audio demo still pending real assets.)
- [x] Generalise the machine-specific `.vscode` config.
- [x] Name the magic-number constants (O-9); `clang-format` pass (O-8).
- [x] ~~CI step grepping for missing SPDX headers~~ — dropped (SPDX out of scope).

**Done when:** adding a parameter touches only `ParameterList.h` + `Parameters.h`
+ presets; tests and plugin share one core target; docs describe the real
architecture.

_State: `JX11_engine` is the single compiled engine; plugin and tests link it.
`ARCHITECTURE.md` documents the signal flow, thread model, and data flow._

### Phase 3 — Modern audio standards  *(goal 2)*
- [x] Initialize all DSP state (A-1).
- [x] Clamp filter cutoff to Nyquist (A-2) + `Filter::clampCutoff` regression test.
- [x] Fix the `filterZip` start-up sweep (A-3) + golden regression (verified it
      fails when the fix is reverted).
- [x] Add parameter smoothing where it matters (A-4): detune and filter Q are
      one-pole smoothed per sample; filter cutoff/key-tracking was already
      smoothed through `filterZip`.
- [x] Compile-time sample guard, disabled in release (A-5).
- [x] Realistic `getTailLengthSeconds` (A-6); save/restore `currentProgram` (A-7).
- [x] `[[nodiscard]]`/`noexcept` tidy-up on hot getters (A-9).

**Done when:** no uninitialised reads (ASan/UBSan clean), the first note sounds
the same as later notes, automation is click-free, and state restore reports the
right program.

_State: defaults added across the DSP types; cutoff clamped in `Filter`; golden
fingerprint regenerated for the filter-sensitive patch and re-validated. ASan/UBSan
in CI still pending Phase 4._

### Phase 4 — CI / tooling elevation  *(goals 1–3)*
- [x] CI matrix: Debug + Release, arm64 + x86_64. (macOS only; Linux not added.)
- [x] ASan/UBSan and TSan CI jobs; both verified locally clean.
- [x] Cache the JUCE `_deps` download; run `pluginval` at strictness 10 + `auval`.
      (pluginval runs with `--skip-gui-tests`; `auval` verified locally.)
- [x] Add a curated `.clang-tidy` and a clang-tidy CI job. The job is advisory
      (`continue-on-error`) until the check set is tuned on a machine with LLVM.
- [x] `JX11_SANITIZERS` CMake option so CI and contributors build them the same way.

**Done when:** a clean clone gets a green badge with sanitizers, pluginval, and
auval all gating.

_State: three build legs, two sanitizer legs, and an advisory lint leg. Sanitizers
and `auval` were run locally and are clean; pluginval must be confirmed on the next
CI run (not installed locally). Remove `continue-on-error` from the clang-tidy job
once tuned._

### Phase 5 — DSP modernization  *(goal 4, optional)*
- [ ] BLEP oscillator + aliasing test (D-1).
- [ ] Robust filter + smoothing (D-2/D-3).
- [ ] Block/SIMD prototype + benchmark, keep whichever passes tests (D-4).
- [ ] Publish CPU measurements in README (D-5).
- [ ] Oversampling / noise upgrade (D-6).

### Phase 6 — Optional features  *(goal 5 — out of scope)*

Not planned. See §2.5 for the ranked list if this is ever revisited. If it is,
add in that order, each with spec row, preset, test, and doc update.

---

## 5. Definition of done (CV submission)

The repository is ready to attach to a CV when:

1. `git status` is clean; history is a sequence of focused, green commits.
2. `README.md` explains what JX11.5 is, what was modernized vs. the book, how to
   build/run/test, and links a screenshot + audio demo.
3. `ARCHITECTURE.md` shows signal flow, thread model, and data flow.
4. CI is green on Debug + Release with ASan/UBSan, TSan, pluginval, and auval.
5. Tests are meaningful: each core module has a test that demonstrably fails when
   the module is broken; no tautological tolerances remain.
6. No uninitialised state, no Nyquist bug, no first-note filter sweep.
7. `CHANGELOG.md` and a version tag describe the JX11 → JX11.5 delta.
8. At least one optional DSP improvement ships with tests. (New features are out of
   scope by decision.)
9. `DEFERRED.md` reflects reality (no stale "todo" items that are already done).

---

## 6. Quick file → issue map

| File | Issues |
|---|---|
| `.kilo/worktrees/elderly-engine/` | O-2 |
| `Source/CMakeLists.txt` | O-3 |
| `Tests/CMakeLists.txt` | O-4 |
| `README.md`, `Source/README.md`, `DEFERRED.md` | O-5, §3 |
| `Source/model/Preset.cpp`, `Tests/ProcessorTests.cpp`, `Tests/SinOscillatorTests.cpp` | O-6 |
| `.vscode/*`, `.editorconfig` | O-7 |
| `Source/dsp/Voice.h` | A-1, A-2, A-9 |
| `Source/dsp/Synth.cpp` / `Synth.h` | A-3, A-9 |
| `Source/dsp/Filter.h` | A-2, D-2 |
| `Source/dsp/Oscillator.h` | A-1, T-2, D-1 |
| `Source/dsp/NoiseGenerator.h` | A-1, T-3 |
| `Source/dsp/Envelope.h` | T-8 |
| `Source/model/Parameters.h` | T-6 |
| `Source/plugin/PluginProcessor.cpp` | A-6, A-7, T-5 |
| `Tests/ModulationTests.cpp` | T-1 |
| `Tests/FilterTests.cpp` | T-7 |
| `Tests/SynthTests.cpp` | T-7, T-9 |
