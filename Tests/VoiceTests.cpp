// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include "Voice.h"

class VoiceTests : public juce::UnitTest
{
public:
    VoiceTests()
        : juce::UnitTest ("Voice", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("reset() clears note and velocity");
        {
            Voice v;
            v.note = 60;

            v.reset();

            expectEquals (v.note, -1);
        }

        beginTest ("panning centers at middle C and hard-pans two octaves out");
        {
            Voice v;
            v.reset();

            v.note = 60;
            v.updatePanning();
            expectWithinAbsoluteError (v.panLeft, 0.7071f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 0.7071f, 1.0e-3f);

            v.note = 84;
            v.updatePanning(); // +24 semi -> hard right
            expectWithinAbsoluteError (v.panLeft, 0.0f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 1.0f, 1.0e-3f);

            v.note = 36;
            v.updatePanning(); // -24 semi ->hard left
            expectWithinAbsoluteError (v.panLeft, 1.0f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 0.0f, 1.0e-3f);
        }

        beginTest ("panning is constant power and clamps out of range");
        {
            Voice v;
            v.reset();

            v.note = 72;
            v.updatePanning();
            expectWithinAbsoluteError (v.panLeft * v.panLeft + v.panRight * v.panRight, 1.0f, 1.0e-3f);

            v.note = 127;
            v.updatePanning(); // clamps to hard right
            expectWithinAbsoluteError (v.panLeft, 0.0f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 1.0f, 1.0e-3f);

            v.note = 0;
            v.updatePanning(); // clamps to hard left
            expectWithinAbsoluteError (v.panLeft, 1.0f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 0.0f, 1.0e-3f);
        }
    }
};

static VoiceTests voiceTests;