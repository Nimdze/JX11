// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include "Oscillator.h"
#include "Envelope.h"

// A single voice produced by the synth (one note)
struct Voice
{
    int note;
    Oscillator osc; // for now voice uses only Oscillator

    float saw;

    Envelope env;

    void reset()
    {
        note = -1;
        osc.reset();

        saw = 0.0f;

        env.reset();
    }

    float render (float input)
    {
        float sample = osc.nextSample();
        saw = saw * 0.997f + sample;

        float output = saw + input;

        float envelope = env.nextValue();
        return output * envelope;
    }

    void release() { env.release(); }
};