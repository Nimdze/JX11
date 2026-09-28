// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Synth.h"
#include "Voice.h"
#include "Utils.h"

Synth::Synth()
{
    sampleRate = 44100.0f;
}

// =============================
// Basic operations
// =============================
void Synth::allocateResources (double sampleRate_, int /*samplesPerBlock*/)
{
    sampleRate = static_cast<float> (sampleRate_);
}

void Synth::deallocateResources() {}

void Synth::reset()
{
    voice.reset();
    noiseGen.reset();
    pitchBend = 1.0f;
}

// =============================
// Audio Rendering
// =============================
void Synth::render (float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];

    voice.osc1.period = voice.period * pitchBend;
    voice.osc2.period = voice.osc1.period * detune;

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        float noise = noiseGen.nextValue() * noiseMix;

        float outputLeft = 0.0f;
        float outputRight = 0.0f;

        if (voice.env.isActive())
        {
            float output = voice.render (noise);
            outputLeft += output * voice.panLeft;
            outputRight += output * voice.panRight;
        }

        if (outputBufferRight != nullptr)
        { // if for mono / stereo
            outputBufferLeft[sample] = outputLeft;
            outputBufferRight[sample] = outputRight;
        }
        else
            outputBufferLeft[sample] = (outputLeft + outputRight) * 0.5f;
    }

    if (!voice.env.isActive())
    {
        voice.env.reset();
    }

    guardFlags.fetch_or (protectYourEars (outputBufferLeft, sampleCount), std::memory_order_relaxed);
    if (outputBufferRight != nullptr)
        guardFlags.fetch_or (protectYourEars (outputBufferRight, sampleCount), std::memory_order_relaxed);
}

// =============================
// MIDI handeling
// =============================

void Synth::midiMessage (uint8_t data0, uint8_t data1, uint8_t data2)
{
    switch (data0 & 0xF0)
    {
        case 0x80: // Note off
            noteOff (data1 & 0x7F);
            break;

        case 0x90: // Note on
        {
            uint8_t note = data1 & 0x7F;
            uint8_t velo = data2 & 0x7F;
            if (velo > 0)
            {
                noteOn (note, velo);
            }
            else
            {
                noteOff (note);
            }
            break;
        }

        case 0xE0: // Pitch bend
            // float(data1 +128 * data2 - 8192) gives anumber between -8192 and 8191
            // mapping this range to 2^(2/12) to 2^(-2/12) (multiplying the period)
            // 2^((-2*(data/8192))/12) = (2^((-2/8192)/12))^data = exp^(M*data)
            pitchBend = std::exp (-0.000014102f * float (data1 + 128 * data2 - 8192));
            break;
    }
}

void Synth::noteOn (int note, int velocity)
{
    voice.note = note;
    voice.updatePanning();

    float period = calcPeriod (note);
    voice.period = period;

    voice.osc1.amplitude = (static_cast<float> (velocity) / 127.0f) * 0.5f;
    // voice.osc1.reset();

    voice.osc2.amplitude = voice.osc1.amplitude * oscMix;
    // voice.osc2.reset();

    Envelope& env = voice.env;
    env.attackMultiplier = envAttack;
    env.decayMultiplier = envDecay;
    env.sustainLevel = envSustain;
    env.releaseMultiplier = envRelease;
    env.attack();
}

void Synth::noteOff (int note)
{
    if (voice.note == note)
    {
        voice.release();
    }
}

float Synth::calcPeriod (int note) const
{
    // freq = 440 * 2^((note - 69)/12) = 440 * 2^(-69/12) * 2^(note/12)
    // per = (sampleRate/(440 * 2^(-69/12))) * 2^(-note/12)
    // define tune = (sampleRate/(440 * 2^(-69/12)))
    // 2^(-note/12) = (2^(-1/12))^note = exp^(M * note)
    float period = tune * std::exp (-0.05776226505f * float (note));

    while (period < 6.0f || (period * detune) < 6.0f)
    {
        period += period;
    }
    if (period <= 0.0f)
        period = 6.0f;

    return period;
}
