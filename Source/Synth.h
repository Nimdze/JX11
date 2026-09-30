// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include "Voice.h"
#include "NoiseGenerator.h"
#include "LFO.h"

class Synth
{
public:
    // Synth properties
    static constexpr int MAX_VOICES = 8;
    int numVoices = 1;

    float volumeTrim = 1.0f;
    juce::LinearSmoothedValue<float> outputLevelSmoother = 1.0f;

    float noiseMix = 0.0f;
    float oscMix = 0.0f;

    float detune = 1.0f;
    float tune = 0.0f;
    float pitchBend = 1.0f;

    float velocitySensitivity = 1.0f;
    bool ignoreVelocity = false;

    float envAttack = 0.0f;
    float envDecay = 0.0f;
    float envSustain = 1.0f;
    float envRelease = 0.0f;

    float lfoInc = 0.0f;
    float vibrato = 0.0f;

    float pwmDepth = 0.0f;
    float modWheel = 0.0f;

    int glideMode = 0;
    float glideRate = 1.0f;
    float glideBend = 1.0f;

    // Basic operations
    Synth();
    void allocateResources (double sampleRate, int samplesPerBlock);
    void deallocateResources();
    void reset();

    // Audio rendering
    void render (float** outputBuffers, int sampleCount);

    // MIDI handling
    void midiMessage (uint8_t data0, uint8_t data1, uint8_t data2);

    // Guarding against ivalid samples
    unsigned takeGuardFlags() noexcept { return guardFlags.exchange (0, std::memory_order_relaxed); }

    float calcPeriod (int v, int note) const;

private:
    // Synth Properties
    float sampleRate;
    Voice voices[MAX_VOICES];
    int lastNote;
    bool sustainPedalPressed;

    LFO lfo;

    void restartMonoVoice (int note, int velocity); // Make mono legato not retrigger the envelope
    void shiftQueuedNotes();                        // Mono legato note managment upon key release
    int nextQueuedNote();                           // Mono legato note managment upon key release

    NoiseGenerator noiseGen;

    // Voices and MIDI handeling
    void startVoice (int v, int note, int velocity);
    int findFreeVoice() const;
    void noteOn (int note, int velocity);
    void noteOff (int note);
    void controlChange (uint8_t data1, uint8_t data2);

    void updateLFO();
    inline void updatePeriod (Voice& voice)
    {
        voice.osc1.period = voice.period * pitchBend;
        voice.osc2.period = voice.osc1.period * detune;
    }

    bool isPlayingLegatoStyle() const;

    // Catching invalid sample type
    std::atomic<unsigned> guardFlags{0};
};

// defensive guard against audio thread locking
static_assert (std::atomic<unsigned>::is_always_lock_free, "guardFlags must be lock-free for the audio thread");