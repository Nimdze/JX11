# Deferred Work

Items intentionally postponed or explicitly out of scope. Scheduled work lives in
[MODERNIZATION.md](MODERNIZATION.md) — this file only holds things that are *not*
part of the current plan, so the two don't drift apart.

## Testing

- [ ] **MemorySanitizer (MSan)** — catches uninitialised reads. Hard to set up:
      requires all dependencies instrumented. Revisit only if needed.

## DSP

- [ ] **BLEP oscillator** — the current oscillator is the book's band-limited
      impulse train (BLIT) sawtooth. A BLEP replacement would alias less at high
      pitches. Deliberately not pursued for JX11.5; the existing oscillator is
      tested (`OscillatorTests`, `SynthTests`, golden render) and there is no
      known defect. See MODERNIZATION.md (D-1).

## Out of scope (features)

- [ ] **Optional new features from the book** (oscillator mix modes, waveform
      selection, distortion/sub oscillator, supersaw, extra filter types, third
      oscillator). Deliberately not doing these — the focus is organization and
      modernization, not feature creep. Full ranked list in
      MODERNIZATION.md §2.5.

## Packaging / Release

- [ ] **Code signing + notarization** — required for macOS plugin distribution.
- [ ] **Installers / release packaging** — `.pkg`/`.dmg` or equivalent.
- [ ] **Windows / Linux builds** — CI matrix if cross-platform is wanted.

## Docs

- [ ] **Screenshots / audio demo** — a `docs/` folder with a UI screenshot and an
      audio demo would help the README; pending real assets.
- [ ] **`CONTRIBUTING.md` / issue templates** — only if inviting collaborators.

---

## Done (for reference)

- Version control + focused history
- Unit tests (`ctest`) enforced by a pre-commit hook
- ASan/UBSan build tree, plus CI sanitizer jobs
- pluginval @5 + `auval` passing
- Universal macOS build (arm64 + x86_64)
- clang-format config + applied
- GPL-3.0-or-later license
- GitHub Actions CI (build + ctest), green
- JUCE vendored via CMake `FetchContent` (pinned 8.0.12)
- Presets / program state (`getNumPrograms`, `getCurrentProgram`,
  `get/setStateInformation`, factory bank)
- Parameters exposed through the APVTS with a spec table and apply functions
- `Oscillator` and `NoiseGenerator` tests

## Moved to the active plan (MODERNIZATION.md)

These were listed here before they had a phase. They are now scheduled, so they
live in the roadmap rather than this file:

- `processBlock` output test (Phase 1)
- pluginval in CI, strictness 10 (Phase 4)
- ThreadSanitizer build (Phase 4)
- `currentProgram` not restored with state (Phase 3)
- clang-tidy (Phase 4)
- Cache JUCE in CI (Phase 4)
- `.editorconfig` (Phase 2)
- clang-format review (Phase 2)
- Architecture notes (Phase 2)
- `CHANGELOG.md` + git tags (Phase 2)
