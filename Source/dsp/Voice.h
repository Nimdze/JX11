#pragma once

#include <algorithm>
#include "dsp/Oscillator.h"
#include "dsp/Envelope.h"
#include "dsp/Filter.h"

// A single voice produced by the synth (one note)
struct Voice
{
    int note = -1;
    Oscillator osc1;
    Oscillator osc2;

    float period = 0.0f;
    float target = 0.0f;
    float glideRate = 1.0f;

    float panLeft = 0.707f, panRight = 0.707f;
    float panPosition = 0.0f; // -1 (left) .. +1 (right), set at note-on

    float saw = 0.0f;

    Envelope env;

    Filter filter;
    float cutoff = 0.0f;
    float filterMod = 0.0f;
    float filterQ = 1.0f;

    float pitchBend = 1.0f;

    Envelope filterEnv;
    float filterEnvDepth = 0.0f;

    void reset()
    {
        note = -1;
        panLeft = 0.707f;
        panRight = 0.707f;
        panPosition = 0.0f;
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
        saw = saw * kSawLeak + sample1 - sample2;

        float output = saw + input;
        output = filter.render (output);

        float envelope = env.nextValue();
        return output * envelope;
    }

    void release()
    {
        env.release();
        filterEnv.release();
    }

    // Position within the keyboard. Stored separately from `note` so it stays
    // valid when a released note is parked on the SUSTAIN sentinel.
    void setPanPosition (int midiNote)
    {
        panPosition = std::clamp ((static_cast<float> (midiNote) - 60.0f) / 24.0f, -1.0f, 1.0f);
    }

    // Constant-power pan. When disabled, every voice is centred.
    void updatePanning (bool enabled)
    {
        if (!enabled)
        {
            panLeft = panRight = 0.707f;
            return;
        }

        panLeft = std::sin ((PI / 4) * (1.0f - panPosition));
        panRight = std::sin ((PI / 4) * (1.0f + panPosition));
    }

    void updateLFO()
    {
        period += glideRate * (target - period);

        float fenv = filterEnv.nextValue();
        float modulatedCutoff = cutoff * std::exp (filterMod + filterEnvDepth * fenv) / pitchBend;
        modulatedCutoff = std::clamp (modulatedCutoff, 30.0f, 20000.0f);
        filter.updateCoefficients (modulatedCutoff, filterQ);
    }
};