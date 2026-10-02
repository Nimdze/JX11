// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include "Voice.h"
#include "NoiseGenerator.h"
#include "LFO.h"
#include "SynthParams.h"
#include "MidiState.h"
#include "VoiceAllocator.h"

class Synth
{
public:
    // Synth properties
    static constexpr int MAX_VOICES = SynthLimits::MAX_VOICES;

    juce::LinearSmoothedValue<float> outputLevelSmoother = 1.0f;

    MidiState midi;

    // Basic operations
    Synth();
    void allocateResources (double sampleRate, int samplesPerBlock);
    void deallocateResources();
    void reset();
    void setParam (const SynthParams& value) noexcept { params = value; }

    // Audio rendering
    void render (float** outputBuffers, int sampleCount);

    // MIDI handling
    void midiMessage (uint8_t data0, uint8_t data1, uint8_t data2);

    // Guarding against ivalid samples
    unsigned takeGuardFlags() noexcept { return guardFlags.exchange (0, std::memory_order_relaxed); }

    float calcPeriod (int v, int note) const;

private:
    // Synth Properties
    SynthParams params;

    float sampleRate;
    Voice voices[MAX_VOICES];
    VoiceAllocator allocator;

    LFO lfo;

    NoiseGenerator noiseGen;

    float filterZip = 0.0f;

    void controlChange (uint8_t data1, uint8_t data2);

    void updateLFO();
    inline void updatePeriod (Voice& voice)
    {
        voice.osc1.period = voice.period * midi.pitchBend;
        voice.osc2.period = voice.osc1.period * params.detune;
    }

    // Catching invalid sample type
    std::atomic<unsigned> guardFlags{0};
};

// defensive guard against audio thread locking
static_assert (std::atomic<unsigned>::is_always_lock_free, "guardFlags must be lock-free for the audio thread");
