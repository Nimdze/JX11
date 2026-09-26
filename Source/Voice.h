// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "Oscillator.h"

struct Voice
{
    int note;
    Oscillator osc; // for now voice uses only Oscillator

    float saw;

    void reset()
    {
        note = -1;
        osc.reset();

        saw = 0.0f;
    }

    float render()
    {
        float sample = osc.nextSample();
        saw = saw * 0.997f + sample;
        return saw;
    }


};