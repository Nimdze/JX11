// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include "Synth.h"
#include <algorithm>

namespace
{
bool isSilent (const float* b, int n)
{
    for (int i = 0; i < n; ++i)
        if (b[i] != 0.0f)
            return false;
    return true;
}
} // namespace

class SynthTests : public juce::UnitTest
{
public:
    SynthTests()
        : juce::UnitTest ("Synth", "JX11")
    {
    }

    void runTest() override
    {
        constexpr int n = 512;
        float left[n]{}, right[n]{};
        float* outputs[2] = {left, right};

        Synth synth;
        synth.allocateResources (44100.0, n);

        synth.envAttack = 0.75f; // fast
        synth.envDecay = 0.75f;
        synth.envSustain = 1.0f;  // hold
        synth.envRelease = 0.75f; // fast (~32 samples to silence)

        // reset synth + zero the buffers before each test
        auto fresh = [&]
        {
            synth.reset();
            std::fill (left, left + n, 0.0f);
            std::fill (right, right + n, 0.0f);
        };

        auto renderUntilSilent = [&] (int maxBlocks)
        {
            for (int b = 0; b < maxBlocks; ++b)
            {
                std::fill (left, left + n, 0.0f);
                std::fill (right, right + n, 0.0f);
                synth.render (outputs, n);
                if (isSilent (left, n))
                    return true;
            }
            return false;
        };

        beginTest ("silent before any note");
        fresh();
        synth.render (outputs, n);
        expect (isSilent (left, n));

        beginTest ("note on produces audio");
        fresh();
        synth.midiMessage (0x90, 60, 100);
        synth.render (outputs, n);
        expect (!isSilent (left, n));

        beginTest ("note off fades to silence");
        fresh();
        synth.midiMessage (0x90, 60, 100);
        synth.render (outputs, n);
        synth.midiMessage (0x80, 60, 0);
        expect (renderUntilSilent (10));

        beginTest ("note on with velocity 0 acts as note off");
        fresh();
        synth.midiMessage (0x90, 60, 100);
        synth.render (outputs, n);
        synth.midiMessage (0x90, 60, 0); // off
        expect (renderUntilSilent (10));

        beginTest ("all channels are accepted");
        fresh();
        synth.midiMessage (0x92, 60, 100);
        synth.render (outputs, n);
        expect (!isSilent (left, n));

        beginTest ("note off for a different note does'nt stop the voice");
        fresh();
        synth.midiMessage (0x90, 60, 100);
        synth.midiMessage (0x80, 62, 0);
        synth.render (outputs, n);
        expect (!isSilent (left, n));
    }
};

static SynthTests synthTests;