// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include "Synth.h"
#include "Voice.h"
#include "Utils.h"

static const float ANALOG = 0.002f;
static const int SUSTAIN = -2;

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
    for (int v = 0; v < MAX_VOICES; ++v)
        voices[v].reset();
        
    noiseGen.reset();
    pitchBend = 1.0f;
    sustainPedalPressed = false;

    outputLevelSmoother.reset(sampleRate, 0.05);
}

// =============================
// Audio Rendering
// =============================
void Synth::render (float** outputBuffers, int sampleCount)
{
    float* outputBufferLeft = outputBuffers[0];
    float* outputBufferRight = outputBuffers[1];

    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice& voice = voices[v];
        if (voice.env.isActive()) {
            voice.osc1.period = voice.period * pitchBend;
            voice.osc2.period = voice.osc1.period * detune;
        }
    }

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        const float noise = noiseGen.nextValue() * noiseMix;

        float outputLeft = 0.0f;
        float outputRight = 0.0f;

        for (int v = 0; v < MAX_VOICES; ++v)
        {
            Voice& voice = voices[v];
            if (voice.env.isActive())
            {
                float output = voice.render (noise);
                outputLeft += output * voice.panLeft;
                outputRight += output * voice.panRight;
            }
        }

        float outputLevel = outputLevelSmoother.getNextValue(); // linear interpolation to mitigate zipper noise
        outputLeft *= outputLevel;
        outputRight *= outputLevel;

        if (outputBufferRight != nullptr)
        { // if for mono / stereo
            outputBufferLeft[sample] = outputLeft;
            outputBufferRight[sample] = outputRight;
        }
        else
            outputBufferLeft[sample] = (outputLeft + outputRight) * 0.5f;
    }

    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice& voice = voices[v];
        if (!voice.env.isActive())
        {
            voice.env.reset();
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
        {
            // float(data1 +128 * data2 - 8192) gives anumber between -8192 and 8191
            // mapping this range to 2^(2/12) to 2^(-2/12) (multiplying the period)
            // 2^((-2*(data/8192))/12) = (2^((-2/8192)/12))^data = exp^(M*data)
            pitchBend = std::exp (-0.000014102f * float (data1 + 128 * data2 - 8192));
            break;
        }

        case 0xB0: // Control change
        {
            controlChange(data1, data2);
            break;
        }

    }
}

void Synth::startVoice (int v, int note, int velocity)
{
    float period = calcPeriod (v, note);

    Voice& voice = voices[v];
    voice.period = period;
    voice.note = note;
    voice.updatePanning();

    voice.osc1.amplitude = volumeTrim * static_cast<float> (velocity) ;
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

int Synth::findFreeVoice() const
{
    int v = 0;
    float l = 100.0f; // louder than any envelope

    for (int i = 0; i < MAX_VOICES; ++i)
    {
        if (voices[i].env.level < l && !voices[i].env.isInAttack())
        {
            l = voices[i].env.level;
            v = i;
        }
    }
    return v;
}

// Make mono legato not retrigger the envelope
void Synth::restartMonoVoice(int note, int velocity)
{
    float period = calcPeriod(0, note);

    Voice& voice = voices[0];
    voice.period = period;

    voice.env.level += SILENCE + SILENCE;
    voice.note = note;
    voice.updatePanning();
}

void Synth::shiftQueuedNotes()
{
    for (int temp = MAX_VOICES - 1; temp > 0; temp--)
    {
        voices[temp].note = voices[temp - 1].note;
        voices[temp].release();
    }
}

int Synth::nextQueuedNote()
{
    int held = 0;
    for (int v = MAX_VOICES -1; v > 0; v--)
        if (voices[v].note > 0)
            held = v;
    
    if (held > 0)
    {
        int note = voices[held].note;
        voices[held].note = 0;
        return note;
    }

    return 0;
}

void Synth::noteOn(int note, int velocity)
{
    int v = 0; // 0 for mono

    if (numVoices == 1) // mono
    {
        if (voices[0].note > 0) //legato
        {
            shiftQueuedNotes();
            restartMonoVoice(note, velocity);
            return;
        }
    }

    else // poly
    {
        v = findFreeVoice();
    }

    startVoice(v, note, velocity);
}

void Synth::noteOff (int note)
{
    if ((numVoices ==1) && (voices[0].note == note))
    {
        int queuedNote = nextQueuedNote();
        if (queuedNote > 0)
            restartMonoVoice(queuedNote, -1);
    }

    for (int v = 0; v < MAX_VOICES; v++)
    {
        if (voices[v].note == note)
        {
            if (sustainPedalPressed)
                voices[v].note = SUSTAIN;
            else {
                voices[v].release();
                voices[v].note = 0;
            }
        }
    }
}

void Synth::controlChange(uint8_t data1, uint8_t data2)
{
    switch (data1) {
        // Sustain pedal
        case 0x40:
            sustainPedalPressed = (data2 >= 64);

            if (!sustainPedalPressed)
                noteOff(SUSTAIN);

            break;
        
        default: // panic, all notes off - 120 or above
            if (data1 >= 0X78)
            {
                for (int v = 0; v < MAX_VOICES; ++v)
                    voices[v].reset();
                sustainPedalPressed = false;
            }
            break;
    }
}

float Synth::calcPeriod (int v, int note) const
{
    // freq = 440 * 2^((note - 69)/12) = 440 * 2^(-69/12) * 2^(note/12)
    // per = (sampleRate/(440 * 2^(-69/12))) * 2^(-note/12)
    // define tune = (sampleRate/(440 * 2^(-69/12)))
    // 2^(-note/12) = (2^(-1/12))^note = exp^(M * note)
    // adding slight detune per voice to simulate analog synth detune
    float period = tune * std::exp (-0.05776226505f * (float (note) + ANALOG * float(v)));

    while (period < 6.0f || (period * detune) < 6.0f)
    {
        if (period <= 0.0f)
            period = 6.0f;

        period += period;
    }

    return period;
}
