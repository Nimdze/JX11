// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include "model/Preset.h"
#include "model/Parameters.h"

class PresetTests : public juce::UnitTest
{
public:
    PresetTests()
        : juce::UnitTest ("Presets", "JX11")
    {
    }

    void runTest() override
    {
        const std::vector<Preset> presets = createFactoryPresets();

        beginTest ("factory bank is populated");
        expect (!presets.empty());

        if (presets.empty())
            return;

        beginTest ("preset names are non-empty and unique");
        {
            for (size_t i = 0; i < presets.size(); ++i)
                expect (presets[i].name.isNotEmpty());

            for (size_t i = 0; i < presets.size(); ++i)
                for (size_t j = i + 1; j < presets.size(); ++j)
                    expect (presets[i].name != presets[j].name, presets[i].name + " duplicates " + presets[j].name);
        }

        beginTest ("every preset value is inside its parameter range");
        {
            for (const auto& preset : presets)
            {
                for (int i = 0; i < Params::NumParams; ++i)
                {
                    const auto& s = Params::kSpecs[i];
                    const float v = preset.param[i];

                    expect (v >= s.min && v <= s.max, preset.name + " / " + juce::String (s.id) + " = " +
                                                          juce::String (v, 2) + " outside [" + juce::String (s.min, 2) +
                                                          ", " + juce::String (s.max, 2) + "]");
                }
            }
        }

        beginTest ("choice parameters hold valid integer indices");
        {
            for (const auto& preset : presets)
            {
                for (int i = 0; i < Params::NumParams; ++i)
                {
                    const auto& s = Params::kSpecs[i];
                    if (s.choices == nullptr)
                        continue;

                    const float v = preset.param[i];
                    const int idx = static_cast<int> (v);

                    expectWithinAbsoluteError (v, static_cast<float> (idx), 1.0e-6f);
                    expect (idx >= 0 && idx < s.numChoices, preset.name + " / " + juce::String (s.id) + ": index " +
                                                                juce::String (idx) + " out of range");
                }
            }
        }
    }
};

static PresetTests presetTests;
