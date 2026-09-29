# Deferred Work

Items intentionally postponed. None of these block current development —
they're recorded so future-me remembers the reasoning and the trigger for
picking each one up.

## Testing

- [ ] **processBlock output test** — `prepareToPlay` + `processBlock` with a                                                 
         MIDI note, asserting finite non-silent output. Remaining piece of the                                                  
         self-host story. 
- [ ] **pluginval in CI** — currently CI only runs `ctest`. Add a step that
      builds the VST3/AU and runs pluginval.
- [ ] **pluginval strictness 10** — raise from level 5 (used for local dev) to
      level 10 before releases, and consider `--repeat N`.
- [ ] **ThreadSanitizer (TSan)** build — once the UI thread and audio thread
      share state (parameters, editor), add a `-fsanitize=thread` build.
- [ ] **MemorySanitizer (MSan)** — catches uninitialised reads (the class of bug
      that hit `Voice::velocity`). Hard to set up: requires all dependencies
      instrumented. Revisit only if needed.
- [ ] **`currentProgram` not restored with state** — `setStateInformation`                                                   
         restores parameter values but the program index isn't stored in the state                                              
         tree, so `getCurrentProgram()` can be stale after a restore.  

## Plugin / DSP

- [ ] **Presets / program state** — `getNumPrograms`, `getCurrentProgram`,
      `getStateInformation` / `setStateInformation` with real data.
- [ ] **Parameter automation tests** — once the synth exposes parameters.

## Tooling / CI

- [ ] **clang-tidy** — static analysis with a curated `.clang-tidy`
      (`bugprone-*`, `cppcoreguidelines-*`, `modernize-*`, `performance-*`).
      Noisy to tune; the compiler warnings + tests cover common cases today.
- [ ] **Cache JUCE in CI** — FetchContent re-downloads JUCE each run; add
      `actions/cache` on the `_deps` directory to speed up CI.
- [ ] **`.editorconfig`** — cross-editor whitespace/indent consistency.
- [ ] **clang-format review** — revisit `SpaceBeforeParens: NonEmptyParentheses`
      (it produces `int (x)` casts) if the style becomes annoying.

## Packaging / Release

- [ ] **Licensing decision for public release** — repo is GPL-3.0-or-later
      because of JUCE. Distributing a closed-source build would require a JUCE
      commercial licence.
- [ ] **Code signing + notarization** — required for macOS plugin distribution.
- [ ] **Installers / release packaging** — `.pkg`/`.dmg` or equivalent.
- [ ] **`CHANGELOG.md` + git tags** — semantic versioning and release history.
- [ ] **Windows / Linux builds** — CI matrix if cross-platform is wanted.

## Docs

- [ ] **Architecture notes** — only when the codebase outgrows a flat `Source/`.
- [ ] **`CONTRIBUTING.md` / issue templates** — only if inviting collaborators.

---

## Done (for reference)

- Version control + private GitHub remote, focused history
- Unit tests (`ctest`) enforced by a pre-commit hook
- ASan/UBSan build tree (`build-asan/`)
- pluginval @5 + `auval` passing
- Universal macOS build (arm64 + x86_64)
- clang-format config + applied
- GPL-3.0-or-later license + SPDX headers
- GitHub Actions CI (build + ctest), green
- JUCE vendored via CMake `FetchContent` (pinned 8.0.12)
