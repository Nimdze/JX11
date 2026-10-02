// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <cmath>
#include "common/Constants.h"

class LFO
{
public:
    static constexpr int MAX_STEPS = 32;

    void reset()
    {
        step = 0;
        phase = 0.0f;
    }

    // Call on every audio sample, advance once in MAX_STEPS calls
    bool advance (float inc)
    {
        if (--step > 0)
            return false;

        step = MAX_STEPS;
        phase += inc;
        if (phase > PI)
            phase -= 2.0f * PI;

        return true;
    }

    float current() const { return std::sin (phase); }

private:
    int step = 0;
    float phase = 0.0f;
};