// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include "Parameters.h"

class ParameterTests : public juce::UnitTest
{
public:
    ParameterTests()
        : juce::UnitTest ("Parameters", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("expected parameter count");
        {
            expectEquals (static_cast<int> (Params::NumParams), 26);
        }

        beginTest ("spec table order matches the enum");
        {
            // Each enum constant must select the row whose id matches its name.
            // catches both enum reordering and table reordering
            auto expectId = [this] (int index, const char* id)
            { expectEquals (juce::String (Params::kSpecs[index].id), juce::String (id)); };

            expectId (Params::oscMix, "oscMix");
            expectId (Params::oscTune, "oscTune");
            expectId (Params::oscFine, "oscFine");
            expectId (Params::glideMode, "glideMode");
            expectId (Params::glideRate, "glideRate");
            expectId (Params::glideBend, "glideBend");
            expectId (Params::filterFreq, "filterFreq");
            expectId (Params::filterReso, "filterReso");
            expectId (Params::filterEnv, "filterEnv");
            expectId (Params::filterLFO, "filterLFO");
            expectId (Params::filterVelocity, "filterVelocity");
            expectId (Params::filterAttack, "filterAttack");
            expectId (Params::filterDecay, "filterDecay");
            expectId (Params::filterSustain, "filterSustain");
            expectId (Params::filterRelease, "filterRelease");
            expectId (Params::envAttack, "envAttack");
            expectId (Params::envDecay, "envDecay");
            expectId (Params::envSustain, "envSustain");
            expectId (Params::envRelease, "envRelease");
            expectId (Params::lfoRate, "lfoRate");
            expectId (Params::vibrato, "vibrato");
            expectId (Params::noise, "noise");
            expectId (Params::octave, "octave");
            expectId (Params::tuning, "tuning");
            expectId (Params::outputLevel, "outputLevel");
            expectId (Params::polyMode, "polyMode");
        }

        beginTest ("parameter ids are unique");
        {
            for (int i = 0; i < Params::NumParams; ++i)
                for (int j = i + 1; j < Params::NumParams; ++j)
                    expect (juce::String (Params::kSpecs[i].id) != juce::String (Params::kSpecs[j].id));
        }

        beginTest ("every spec is self-consistent");
        {
            for (int i = 0; i < Params::NumParams; ++i)
            {
                const auto& s = Params::kSpecs[i];

                expect (s.id != nullptr && s.id[0] != 0);
                expect (s.name != nullptr && s.name[0] != 0);
                expect (s.min < s.max);
                expect (s.defaultValue >= s.min && s.defaultValue <= s.max);
                expect (s.interval >= 0.0f);
                expect (s.skew > 0.0f);

                if (s.choices == nullptr)
                    expectEquals (s.numChoices, 0);
                else
                {
                    expect (s.numChoices > 0);

                    const int def = static_cast<int> (s.defaultValue);
                    expectWithinAbsoluteError (s.defaultValue, static_cast<float> (def), 1.0e-6f);
                    expect (def >= 0 && def < s.numChoices);

                    for (int k = 0; k < s.numChoices; ++k)
                        expect (s.choices[k] != nullptr && s.choices[k][0] != 0);
                }
            }
        }

        beginTest ("spot-check important specs");
        {
            const auto& tune = Params::kSpecs[Params::oscTune];
            expectEquals (tune.min, -24.0f);
            expectEquals (tune.max, 24.0f);
            expectEquals (tune.defaultValue, -12.0f);
            expectEquals (tune.interval, 1.0f);

            const auto& fine = Params::kSpecs[Params::oscFine];
            expect (fine.symmetricSkew);
            expectEquals (fine.skew, 0.3f);

            const auto& glide = Params::kSpecs[Params::glideMode];
            expect (glide.choices != nullptr);
            expectEquals (glide.numChoices, 3);
            expectEquals (glide.defaultValue, 0.0f);

            const auto& poly = Params::kSpecs[Params::polyMode];
            expect (poly.choices != nullptr);
            expectEquals (poly.numChoices, 2);
            expectEquals (poly.defaultValue, 1.0f);

            const auto& lfo = Params::kSpecs[Params::lfoRate];
            expectEquals (lfo.defaultValue, 0.81f);
            expect (lfo.toText != nullptr);

            expect (Params::kSpecs[Params::oscMix].toText != nullptr);
        }
    }
};

static ParameterTests parameterTests;