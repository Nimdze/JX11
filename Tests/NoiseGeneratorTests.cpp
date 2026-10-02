// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include "dsp/NoiseGenerator.h"
#include <cmath>

class NoiseGeneratorTests : public juce::UnitTest
{
public:
    NoiseGeneratorTests()
        : juce::UnitTest ("NoiseGenerator", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("output stays inside [-1, 1]");
        {
            NoiseGenerator noise;
            noise.reset();

            for (int i = 0; i < 100000; ++i)
            {
                const float v = noise.nextValue();
                expect (v >= -1.0f && v <= 1.0f);
            }
        }

        beginTest ("successive values vary");
        {
            NoiseGenerator noise;
            noise.reset();

            float previous = noise.nextValue();
            bool differs = false;

            for (int i = 0; i < 100 && !differs; ++i)
            {
                const float v = noise.nextValue();
                differs = std::abs (v - previous) > 1.0e-6f;
                previous = v;
            }

            expect (differs);
        }

        beginTest ("reset() reproduces the same sequence");
        {
            NoiseGenerator a;
            NoiseGenerator b;

            a.reset();
            b.reset();

            for (int i = 0; i < 4096; ++i)
                expectEquals (a.nextValue(), b.nextValue());
        }

        beginTest ("distribution is roughly zero-mean");
        {
            NoiseGenerator noise;
            noise.reset();

            double sum = 0.0;
            constexpr int n = 100000;

            for (int i = 0; i < n; ++i)
                sum += noise.nextValue();

            const double mean = sum / n;
            expect (std::abs (mean) < 0.01, "noise mean = " + juce::String (mean));
        }
    }
};

static NoiseGeneratorTests noiseGeneratorTests;
