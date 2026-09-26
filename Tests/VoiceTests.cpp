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

        Voice v;
        v.note = 60;

        v.reset();

        expectEquals (v.note, -1);
    }
};

static VoiceTests voiceTests;