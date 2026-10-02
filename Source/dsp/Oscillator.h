// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>
#include "common/Constants.h"

class Oscillator
{
public:
    float amplitude = 0.0f;
    float period = 0.0f;
    float modulation = 1.0f;

    void reset()
    {
        inc = 0.0f;
        phase = 0.0f;

        sin0 = 0.0f;
        sin1 = 0.0f;
        dsin = 0.0f;

        dc = 0.0f;
    }

    float nextSample()
    {
        float output = 0.0f;

        phase += inc;

        if (phase <= (PI / 4.0f))
        {
            float halfPeriod = (period / 2.0f) * modulation;
            phaseMax = std::floor (0.5f + halfPeriod) - 0.5f;
            dc = 0.5f * amplitude / phaseMax;
            phaseMax *= PI;

            inc = phaseMax / halfPeriod;
            phase = -phase;

            sin0 = amplitude * std::sin (phase);
            sin1 = amplitude * std::sin (phase - inc);
            dsin = 2.0f * std::cos (inc);

            if (phase * phase > 1e-9)
                output = sin0 / phase;
            else
                output = amplitude;
        }

        else
        {
            if (phase > phaseMax)
            {
                phase = phaseMax + phaseMax - phase;
                inc = -inc;
            }

            float sinp = dsin * sin0 - sin1;
            sin1 = sin0;
            sin0 = sinp;

            output = sinp / phase;
        }

        return output - dc;
    }

    void squareWave (Oscillator& other, float newPeriod)
    {
        reset();

        if (other.inc > 0.0f)
        {
            phase = other.phaseMax + other.phaseMax - other.phase;
            inc = -other.inc;
        }
        else if (other.inc < 0.0f)
        {
            phase = other.phase;
            inc = other.inc;
        }
        else
        {
            phase = -PI;
            inc = PI;
        }

        phase += PI * newPeriod / 2.0f;
        phaseMax = phase;
    }

private:
    float phase = 0.0f;
    float phaseMax = 0.0f;
    float inc = 0.0f;

    float sin0 = 0.0f;
    float sin1 = 0.0f;
    float dsin = 0.0f;

    float dc = 0.0f;

    // Band-limited impulse train (BLIT) sawtooth from the book. A BLEP
    // oscillator would alias less at high pitches; that redesign is
    // intentionally not part of JX11.5 (see MODERNIZATION.md, D-1).
};