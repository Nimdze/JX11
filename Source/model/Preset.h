#pragma once

#include <juce_core/juce_core.h>
#include <initializer_list>
#include <vector>
#include "model/ParameterList.h"

// A single preset override, keyed by the canonical enum index. Any parameter
// not listed falls back to its default from Params::kSpecs.
struct PresetPatch
{
    Params::Index index;
    float value;
};

struct Preset
{
    Preset (const char* presetName, std::initializer_list<PresetPatch> overrides);

    juce::String name;
    float param[Params::NumParams];
};

// returns the built-in bank in Params::Index order, each Preset::param is  indexed by that enum.
std::vector<Preset> createFactoryPresets();