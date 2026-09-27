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
}

// =============================
// Audio Rendering
// =============================
void Synth::render (float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];

    for (int sample = 0; sample < sampleCount; ++sample)
    {

        float noise = noiseGen.nextValue() * noiseMix;

        float output = 0.0f;
        if (voice.note > -1)
        {
            output = voice.render() + noise;
        }

        outputBufferLeft[sample] = output;

        if (outputBufferRight != nullptr)
        { // if for mono / stereo
            outputBufferRight[sample] = output;
        }
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

        case 0x90:
        { // Note on
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
    }
}

void Synth::noteOn (int note, int velocity)
{
    voice.note = note;

    float freq = 440.0f * std::exp2 (float (note - 69) / 12.0f);

    voice.osc.amplitude = (static_cast<float> (velocity) / 127.0f) * 0.5f;

    voice.osc.period = sampleRate / freq;
    voice.osc.reset();
}

void Synth::noteOff (int note)
{
    if (voice.note == note)
    {
        voice.note = -1;
    }
}
