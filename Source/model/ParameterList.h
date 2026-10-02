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

// The single ordered list of parameters. Add a new parameter here.
#define JX11_PARAM_LIST(X)                                                                                             \
    X (oscMix)                                                                                                         \
    X (oscTune) X (oscFine) X (glideMode) X (glideRate) X (glideBend) X (filterFreq) X (filterReso) X (filterEnv)      \
        X (filterLFO) X (filterVelocity) X (filterAttack) X (filterDecay) X (filterSustain) X (filterRelease)          \
            X (envAttack) X (envDecay) X (envSustain) X (envRelease) X (lfoRate) X (vibrato) X (noise) X (octave)      \
                X (tuning) X (outputLevel) X (polyMode)

namespace Params
{
enum Index
{
#define JX11_PARAM_ENUM(name) name,
    JX11_PARAM_LIST (JX11_PARAM_ENUM)
#undef JX11_PARAM_ENUM
    NumParams
};

// id strings, generated from the names above (they match by convention)
inline constexpr const char* kIds[] = {
#define JX11_PARAM_ID(name) #name,
    JX11_PARAM_LIST (JX11_PARAM_ID)
#undef JX11_PARAM_ID
};
} // namespace Params
