// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include <algorithm>
#include "dsp/LFO.h"

class LFOTests : public juce::UnitTest
{
public:
    LFOTests()
        : juce::UnitTest ("LFO", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("reset() then advance() steps on the first call");
        {
            LFO lfo;
            lfo.reset();
            expect (lfo.advance (0.1f)); // step started at 0, this advances
            expectWithinAbsoluteError (lfo.current(), std::sin (0.1f), 1.0e-5f);
        }

        beginTest ("advances once every MAX_STEPS calls");
        {
            LFO lfo;
            lfo.reset();

            lfo.advance (0.1f);
            const float held = lfo.current();

            for (int i = 0; i < LFO::MAX_STEPS - 1; ++i)
            {
                expect (!lfo.advance (0.1f)); // no step
                expectWithinAbsoluteError (lfo.current(), held, 1.0e-6f);
            }

            expect (lfo.advance (0.1f)); // another step
        }

        beginTest ("zero increment never moves");
        {
            LFO lfo;
            lfo.reset();
            lfo.advance (0.0f);

            for (int i = 0; i < LFO::MAX_STEPS * 4; ++i)
            {
                lfo.advance (0.0f);
                expectWithinAbsoluteError (lfo.current(), 0.0f, 1.0e-6f);
            }
        }

        beginTest ("phase wraps after a full cycle");
        {
            LFO lfo;
            lfo.reset();

            const int N = 100;
            const float inc = 2.0f * PI / float (N);

            int steps = 0;
            for (int i = 0; i < N * LFO::MAX_STEPS + 1 && steps < N; ++i)
                if (lfo.advance (inc))
                    ++steps;

            expectWithinAbsoluteError (lfo.current(), 0.0f, 1.0e-3f);
        }
    }
};

static LFOTests lfoTests;