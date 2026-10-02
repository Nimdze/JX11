// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>
#include "dsp/Voice.h"
#include "model/SynthParams.h"
#include "common/Modulation.h"
#include "common/Constants.h"

// Note lifecycle: which voice a note-on lands on, the mono legato note queue,
// voice stealing, and priming a voice's envelopes/oscillators. Everything here
// runs at MIDI / note rate, never per sample.
//
// Holds a pointer to the owner's voice array and SynthParams so the render loop
// keeps iterating the same storage directly.
class VoiceAllocator
{
public:
    static constexpr int SUSTAIN = -2;

    void prepare (Voice* voiceArray, const SynthParams* p, float sr) noexcept
    {
        voices = voiceArray;
        params = p;
        sampleRate = sr;
    }

    void reset() noexcept { lastNote = 0; }

    void noteOn (int note, int velocity) noexcept
    {
        if (params->ignoreVelocity)
            velocity = 80;

        int v = 0; // 0 for mono

        if (params->numVoices == 1) // mono
        {
            if (voices[0].note > 0) // legato
            {
                shiftQueuedNotes();
                restartMonoVoice (note, velocity);
                return;
            }
        }
        else // poly
        {
            v = findFreeVoice();
        }

        startVoice (v, note, velocity);
    }

    void noteOff (int note, bool sustainPedalDown) noexcept
    {
        if ((params->numVoices == 1) && (voices[0].note == note))
        {
            int queuedNote = nextQueuedNote();
            if (queuedNote > 0)
                restartMonoVoice (queuedNote, -1);
        }

        for (int v = 0; v < SynthLimits::MAX_VOICES; v++)
        {
            if (voices[v].note == note)
            {
                if (sustainPedalDown)
                    voices[v].note = SUSTAIN;
                else
                {
                    voices[v].release();
                    voices[v].note = 0;
                }
            }
        }
    }

    void allNotesOff() noexcept
    {
        for (int v = 0; v < SynthLimits::MAX_VOICES; ++v)
            voices[v].reset();
    }

private:
    void startVoice (int v, int note, int rawVelocity) noexcept
    {
        float period = periodForNote (params->tune, params->detune, v, note);

        Voice& voice = voices[v];
        voice.target = period;

        int noteDistance = 0;
        if (lastNote > 0)
        {
            if ((params->glideMode == 2) || ((params->glideMode == 1) && isPlayingLegatoStyle()))
                noteDistance = note - lastNote;
        }

        voice.period = period * std::pow (1.059463094359f, float (noteDistance) - params->glideBend);

        if (voice.period < 6.0f)
            voice.period = 6.0f;

        lastNote = note;

        voice.note = note;
        voice.updatePanning();

        float velocity = velocityCurve (rawVelocity);
        voice.osc1.amplitude = params->volumeTrim * static_cast<float> (velocity);
        voice.osc2.amplitude = voice.osc1.amplitude * params->oscMix;

        if (params->vibrato == 0.0f && params->pwmDepth > 0.0f)
            voice.osc2.squareWave (voice.osc1, voice.period);

        voice.cutoff = sampleRate / (period * PI);
        voice.cutoff *= std::exp (params->velocitySensitivity * float (velocity - 64));

        Envelope& env = voice.env;
        env.attackMultiplier = params->envAttack;
        env.decayMultiplier = params->envDecay;
        env.sustainLevel = params->envSustain;
        env.releaseMultiplier = params->envRelease;
        env.attack();

        Envelope& filterEnv = voice.filterEnv;
        filterEnv.attackMultiplier = params->filterAttack;
        filterEnv.decayMultiplier = params->filterDecay;
        filterEnv.sustainLevel = params->filterSustain;
        filterEnv.releaseMultiplier = params->filterRelease;
        filterEnv.attack();
    }

    int findFreeVoice() const noexcept
    {
        int v = 0;
        float l = 100.0f; // louder than any envelope

        for (int i = 0; i < SynthLimits::MAX_VOICES; ++i)
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
    void restartMonoVoice (int note, int velocity) noexcept
    {
        float period = periodForNote (params->tune, params->detune, 0, note);

        Voice& voice = voices[0];
        voice.target = period;

        if (params->glideMode == 0)
            voice.period = period;

        voice.env.level += SILENCE + SILENCE;
        voice.note = note;
        voice.updatePanning();

        voice.cutoff = sampleRate / (period * PI);
        if (velocity > 0)
            voice.cutoff *= std::exp (params->velocitySensitivity * float (velocity - 64));
    }

    void shiftQueuedNotes() noexcept
    {
        for (int temp = SynthLimits::MAX_VOICES - 1; temp > 0; temp--)
        {
            voices[temp].note = voices[temp - 1].note;
            voices[temp].release();
        }
    }

    int nextQueuedNote() noexcept
    {
        int held = 0;
        for (int v = SynthLimits::MAX_VOICES - 1; v > 0; v--)
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

    bool isPlayingLegatoStyle() const noexcept
    {
        int held = 0;
        for (int i = 0; i < SynthLimits::MAX_VOICES; ++i)
            if (voices[i].note > 0)
                held += 1;

        return held > 0;
    }

    Voice* voices = nullptr;
    const SynthParams* params = nullptr;
    float sampleRate = 44100.0f;
    int lastNote = 0;
};
