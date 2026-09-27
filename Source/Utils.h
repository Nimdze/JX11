// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_core/juce_core.h>
#include <cmath>
#include <cstring>

// clamping / silencing reason
enum SampleGuard : unsigned
{
    SampleGuardClean = 0,
    SampleGuardClamped = 1u << 0,
    SampleGuardNaN = 1u << 1,
    SampleGuardInf = 1u << 2,
    SampleGuardOutOfRange = 1u << 3,
};

// Clamps moderate overload; bails out and silences the whole buffer on the
// first NaN/inf/out-of-range sample. Never logs — runs on the audio thread.
inline unsigned protectYourEars (float* buffer, int sampleCount)
{
    if (buffer == nullptr)
        return SampleGuardClean;

    auto bail = [buffer, sampleCount] (unsigned reason)
    {
        std::memset (buffer, 0, static_cast<size_t> (sampleCount) * sizeof (float));
        return reason;
    };

    bool clamped = false;

    for (int i = 0; i < sampleCount; ++i)
    {
        const float x = buffer[i];

        if (std::isnan (x))
            return bail (SampleGuardNaN);
        if (std::isinf (x))
            return bail (SampleGuardInf);
        if (x < -2.0f || x > 2.0f)
            return bail (SampleGuardOutOfRange);

        if (x < -1.0f)
        {
            buffer[i] = -1.0f;
            clamped = true;
        }
        else if (x > 1.0f)
        {
            buffer[i] = 1.0f;
            clamped = true;
        }
    }

    return clamped ? SampleGuardClamped : SampleGuardClean;
}
