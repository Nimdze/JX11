// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include <algorithm>
#include "dsp/Oscillator.h"

namespace
{
Oscillator makeOsc (float period, float amplitude)
{
    Oscillator o;
    o.amplitude = amplitude;
    o.period = period;
    o.modulation = 1.0f;
    o.reset();
    return o;
}
} // namespace

class OscillatorTests : public juce::UnitTest
{
public:
    OscillatorTests()
        : juce::UnitTest ("Oscillator", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("reset() makes output deterministic");
        {
            auto a = makeOsc (100.0f, 0.5f);
            auto b = makeOsc (100.0f, 0.5f);

            for (int i = 0; i < 2000; ++i)
                expectEquals (a.nextSample(), b.nextSample());
        }

        beginTest ("reset() restarts the phase");
        {
            auto o = makeOsc (100.0f, 0.5f);

            float first[256];
            for (auto& v : first)
                v = o.nextSample();

            o.reset();

            for (float expected : first)
                expectEquals (o.nextSample(), expected);
        }

        beginTest ("output is finite and bounded");
        {
            auto o = makeOsc (100.0f, 1.0f);

            float peak = 0.0f;
            for (int i = 0; i < 44100; ++i)
            {
                const float y = o.nextSample();
                expect (std::isfinite (y));
                peak = std::max (peak, std::abs (y));
            }

            expect (peak < 10.0f, "oscillator output grew without bound");
        }

        beginTest ("period changes the waveform");
        {
            auto a = makeOsc (100.0f, 0.5f);
            auto b = makeOsc (200.0f, 0.5f);

            bool differs = false;
            for (int i = 0; i < 512 && !differs; ++i)
                differs = std::abs (a.nextSample() - b.nextSample()) > 1.0e-6f;

            expect (differs);
        }

        beginTest ("squareWave() produces finite output");
        {
            auto reference = makeOsc (100.0f, 0.5f);

            Oscillator square;
            square.amplitude = 0.5f;
            square.period = 100.0f;
            square.squareWave (reference, reference.period);

            for (int i = 0; i < 2000; ++i)
                expect (std::isfinite (square.nextSample()));
        }
    }
};

static OscillatorTests oscillatorTests;
