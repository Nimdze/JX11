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
        const float sampleRate = 44100.0f;
        synth.tune = sampleRate * std::exp (0.05776226505f * -36.3763f); // octave 0, tuning 0
        synth.detune = 1.0f;
        synth.oscMix = 0.0f;

        synth.allocateResources (sampleRate, n);

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

        beginTest ("period halves per octave and matches A440");
        {
            fresh();
            synth.detune = 1.0f;
            synth.tune = sampleRate * std::exp (0.05776226505f * -36.3763f);
            const float p69 = synth.calcPeriod (69);

            expectWithinAbsoluteError (p69, sampleRate / 440.0f, 0.5f);
            expectWithinAbsoluteError (synth.calcPeriod (81), p69 * 0.5f, 0.5f); // +12 semitones
            expectWithinAbsoluteError (synth.calcPeriod (57), p69 * 2.0f, 1.0f); // -12 semitones
        }

        beginTest ("pitch bend maps centre, max up, max down");
        {
            fresh();
            synth.midiMessage (0xE0, 0, 64); // centre
            expectWithinAbsoluteError (synth.pitchBend, 1.0f, 1.0e-3f);
            synth.midiMessage (0xE0, 127, 127); // max up
            expectWithinAbsoluteError (synth.pitchBend, std::pow (2.0f, -2.0f / 12.0f), 1.0e-3f);
            synth.midiMessage (0xE0, 0, 0); // max up
            expectWithinAbsoluteError (synth.pitchBend, std::pow (2.0f, 2.0f / 12.0f), 1.0e-3f);
        }

        beginTest ("osc2 silent when oscMix is 0");
        {
            fresh();
            synth.oscMix = 0.0f;
            synth.detune = 1.0f;
            synth.midiMessage (0x90, 60, 100);
            synth.render (outputs, n);
            expect (!isSilent (left, n));
        }

        beginTest ("equally mixed, undetuned oscillators cancel");
        {
            fresh();
            synth.oscMix = 1.0f;
            synth.detune = 1.0f;
            synth.midiMessage (0x90, 60, 100);
            synth.render (outputs, n);
            expect (isSilent (left, n)); // sample1 - sample2 == 0
        }

        beginTest ("detuning prevents cancellation");
        {
            fresh();
            synth.oscMix = 1.0f;
            synth.detune = 0.994f;
            synth.midiMessage (0x90, 60, 100);
            synth.render (outputs, n);
            expect (!isSilent (left, n));
        }
    }
};

static SynthTests synthTests;