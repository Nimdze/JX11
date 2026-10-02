// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <algorithm>
#include "Oscillator.h"
#include "Envelope.h"
#include "Filter.h"

// A single voice produced by the synth (one note)
struct Voice
{
    int note;
    Oscillator osc1; // for now voice doesnt use sinOsc
    Oscillator osc2;

    float period;
    float target;
    float glideRate;

    float panLeft, panRight;

    float saw;

    Envelope env;

    Filter filter;
    float cutoff;
    float filterMod;
    float filterQ;

    float pitchBend;

    Envelope filterEnv;
    float filterEnvDepth;

    void reset()
    {
        note = -1;
        panLeft = 0.707f;
        panRight = 0.707f;
        osc1.reset();
        osc2.reset();

        saw = 0.0f;

        env.reset();

        filter.reset();
        filterEnv.reset();
    }

    float render (float input)
    {
        float sample1 = osc1.nextSample();
        float sample2 = osc2.nextSample();
        saw = saw * 0.997f + sample1 - sample2;

        float output = saw + input;
        output = filter.render(output);

        float envelope = env.nextValue();
        return output * envelope;
    }

    void release() 
    { 
        env.release();
        filterEnv.release();
    }

    void updatePanning()
    {
        // setting panning according to midi note number, set for 49 keys
        float panning = std::clamp ((static_cast<float> (note) - 60.0f) / 24.0f, -1.0f, 1.0f);
        panLeft = std::sin ((PI / 4) * (1.0f - panning));
        panRight = std::sin ((PI / 4) * (1.0f + panning));
    }

    void updateLFO() 
    { 
        period += glideRate * (target - period);
        
        float fenv = filterEnv.nextValue();
        float modulatedCutoff = cutoff * std::exp(filterMod + filterEnvDepth * fenv) / pitchBend;
        modulatedCutoff = std::clamp(modulatedCutoff, 30.0f, 20000.0f);
        filter.updateCoefficients(modulatedCutoff, filterQ);
    }
};