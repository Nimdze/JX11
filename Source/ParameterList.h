// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Canonical parameter order for JX11.
//
// This header is deliberately JUCE-free. It exists on its own so that data-only
// code (Preset.h) and lightweight tests can know the parameter order without
// pulling in juce_audio_processors and its GUI dependencies. The JUCE-facing
// descriptions (ranges, defaults, choices, the ParameterLayout) live in
// Parameters.h.
//
// Params::kSpecs[] and Preset::param[] must follow this exact order. Never
// reorder after a release: preset values and saved plugin state are indexed by it.

namespace Params
{
enum Index
{
    oscMix,
    oscTune,
    oscFine,
    glideMode,
    glideRate,
    glideBend,
    filterFreq,
    filterReso,
    filterEnv,
    filterLFO,
    filterVelocity,
    filterAttack,
    filterDecay,
    filterSustain,
    filterRelease,
    envAttack,
    envDecay,
    envSustain,
    envRelease,
    lfoRate,
    vibrato,
    noise,
    octave,
    tuning,
    outputLevel,
    polyMode,
    NumParams
};
} // namespace Params
