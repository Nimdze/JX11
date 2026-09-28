// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include "Utils.h"
#include <limits>

class UtilsTests : public juce::UnitTest
{
public:
    UtilsTests()
        : juce::UnitTest ("Utils", "JX11")
    {
    }

    void runTest() override
    {

        beginTest ("nullptr does not crash and returns clean");
        expect (protectYourEars (nullptr, 16) == SampleGuardClean);

        beginTest ("normal samples pass through unchanged");
        {
            float b[] = {0.5f, -0.25f, 0.0f, 0.75f};
            const unsigned result = protectYourEars (b, 4);

            expect (result == SampleGuardClean);
            expectEquals (b[0], 0.5f);
            expectEquals (b[1], -0.25f);
            expectEquals (b[2], 0.0f);
            expectEquals (b[3], 0.75f);
        }

        beginTest ("NaN silences the whole buffer and reports NaN");
        {
            float b[] = {0.5f, std::numeric_limits<float>::quiet_NaN(), 0.5f};
            const unsigned result = protectYourEars (b, 3);

            expect (result == SampleGuardNaN);
            for (auto v : b)
                expectEquals (v, 0.0f);
        }

        beginTest ("inf silences the whole buffer and reports inf");
        {
            float b[] = {0.5f, std::numeric_limits<float>::infinity(), 0.5f};
            const unsigned result = protectYourEars (b, 3);

            expect (result == SampleGuardInf);
            for (auto v : b)
                expectEquals (v, 0.0f);
        }

        beginTest ("out-of-range silences the whole buffer and reports out of range");
        {
            float b[] = {0.5f, 3.0f, 0.5f};
            const unsigned result = protectYourEars (b, 3);

            expect (result == SampleGuardOutOfRange);
            for (auto v : b)
                expectEquals (v, 0.0f);
        }

        beginTest ("moderate overload is clamped, not silenced");
        {
            float b[] = {1.5f, -1.5f};
            const unsigned result = protectYourEars (b, 2);

            expect (result == SampleGuardClamped);
            expectEquals (b[0], 1.0f);
            expectEquals (b[1], -1.0f);
        }

        beginTest ("bail reason wins over an earlier clamp");
        {
            float b[] = {1.5f, std::numeric_limits<float>::quiet_NaN(), 0.5f};
            const unsigned result = protectYourEars (b, 3);

            expect (result == SampleGuardNaN); // not SampleGuardClamped
            for (auto v : b)
                expectEquals (v, 0.0f); // whole buffer zeroed
        }
    }
};

static UtilsTests utilsTests;
