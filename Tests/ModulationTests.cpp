// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include "dsp/Synth.h"
#include "common/Modulation.h"

class ModulationTests : public juce::UnitTest
{
public:
    ModulationTests()
        : juce::UnitTest ("Modulation", "JX11")
    {
    }

    void runTest() override
    {
        Synth synth;

        beginTest ("mod wheel CC sets a parabolic depth and reseat clears it");
        {
            synth.midiMessage (0xB0, 0x01, 0);
            expectWithinAbsoluteError (synth.midi.modWheel, 0.0f, 1.0e06f);

            synth.midiMessage (0xB0, 0x01, 127);
            expectWithinAbsoluteError (synth.midi.modWheel, 0.000005f * 127.0f * 127.0f, 1.0e-6f);

            synth.midiMessage (0xB0, 0x01, 64);
            expectWithinAbsoluteError (synth.midi.modWheel, 0.000005f * 64.0f * 64.0f, 1.0e-6f);

            synth.reset();
            expectWithinAbsoluteError (synth.midi.modWheel, 0.0f, 1.0e-6f);
        }

        beginTest ("velocity curve maps 1 - 127 to 8.9 - 137.9 and increases");
        {
            expectWithinAbsoluteError (velocityCurve (1), 8.9f, 0.1f);
            expectWithinAbsoluteError (velocityCurve (127), 137.9f, 0.1f);

            for (int v = 1; v < 127; ++v)
                expect (velocityCurve (v) < velocityCurve (v + 1));
        }

        beginTest ("vibrato depth is zero at 0%, maxed at 100%, and symmetric");
        {
            expectWithinAbsoluteError (vibratoDepth (0.0f), 0.0f, 1.0e-6f);
            expectWithinAbsoluteError (vibratoDepth (100.0f), 0.05f, 1.0e-4f);
            expectWithinAbsoluteError (vibratoDepth (-100.0f), 0.05f, 1.0e-4f); // negative = PWM, same depth

            expectWithinAbsoluteError (vibratoDepth (50.0f), vibratoDepth (-50.0f), 1.0e-6f);
        }

        beginTest ("LFO rate maps 0-1 to 0.018-20 Hz and increases");
        {
            expectWithinAbsoluteError (lfoRateHz (0.0f), 0.0183f, 1.0e-3f);
            expectWithinAbsoluteError (lfoRateHz (1.0f), 20.086f, 1.0e-2f);
            expect (lfoRateHz (0.5f) < lfoRateHz (0.6f));
        }

        beginTest ("glide coefficient is in (0, 1) and shrinks with the rate parameter");
        {
            const float inverseUpdateRate = (1.0f / 44100.0f) * 32.0f;       // inverseSampleRate * LFO_MAX
            const float fast = glideCoefficient (2.0f, inverseUpdateRate);   // low rate -> larger coeff -> faster
            const float slow = glideCoefficient (100.0f, inverseUpdateRate); // high rate -> small coeff -> slower

            expect (fast > 0.0f && fast < 1.0f);
            expect (slow > 0.0f && slow < 1.0f);
            expect (fast > slow);
        }

        beginTest ("mod wheel depth is 0 at rest and around 0.0806 fullt open");
        {
            expectWithinAbsoluteError (modWheelDepth (0), 0.0f, 1.0e-6f);
            expectWithinAbsoluteError (modWheelDepth (127), 0.0806f, 1.0e-4f);
        }
    }
};

static ModulationTests modulationTests;