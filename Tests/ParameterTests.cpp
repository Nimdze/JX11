#include <juce_core/juce_core.h>
#include "model/Parameters.h"

class ParameterTests : public juce::UnitTest
{
public:
    ParameterTests()
        : juce::UnitTest ("Parameters", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("spec table matches the canonical parameter list");
        {
            // Each enum index must select the spec row with the same id.
            // catches enum reordering, table reordering and a missing spec row.
            for (int i = 0; i < Params::NumParams; ++i)
                expectEquals (juce::String (Params::kSpecs[i].id), juce::String (Params::kIds[i]),
                              "index " + juce::String (i));
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