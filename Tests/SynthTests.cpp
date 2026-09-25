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
}

class SynthTests : public juce::UnitTest
{
    public:
        SynthTests() : juce::UnitTest ("Synth", "JX11") {}

        void runTest() override
        {
            constexpr int n = 512;
            float left[n] {}, right[n] {};
            float* outputs[2] = { left, right };

            Synth synth;
            synth.allocateResources (44100.0, n);

            // reset synth + zero the buffers before each test
            auto fresh = [&]
            {
                synth.reset();
                std::fill (left, left + n, 0.0f);
                std::fill (right, right + n, 0.0f);
            };

            beginTest ("silent before any note");
            fresh();
            synth.render (outputs, n);
            expect (isSilent (left, n));

            beginTest ("note on produces audio");
            fresh();
            synth.midiMessage (0x90, 60, 100);
            synth.render (outputs, n);
            expect (! isSilent (left, n));

            beginTest ("note off produces silence");
            fresh();
            synth.midiMessage (0x90, 60, 100);
            synth.midiMessage (0x80, 60, 0);
            synth.render (outputs, n);
            expect (isSilent (left, n));

            beginTest ("note on with velocity 0 acts as note off");
            fresh();
            synth.midiMessage(0x90, 60, 100); 
            synth.midiMessage(0x90, 60, 0); // off
            synth.render (outputs, n);
            expect (isSilent (left, n));

            beginTest ("all channels are excepted");
            fresh();
            synth.midiMessage(0x92, 60, 100);
            synth.render(outputs, n);
            expect (! isSilent (left, n));

            beginTest ("note off for a different note doesnt stop the voice");
            fresh();
            synth.midiMessage (0x90, 60, 100);
            synth.midiMessage (0x80, 62, 0);
            synth.render (outputs, n);
            expect (! isSilent (left, n));
        }
};

static SynthTests synthTests;