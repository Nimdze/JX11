// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <juce_core/juce_core.h>
#include <atomic>
#include "Voice.h"
#include "NoiseGenerator.h"

class Synth
{
public:
    // Synth properties
    float noiseMix = 0.0f;
    float envAttack = 0.0f;
    float envDecay = 0.0f;
    float envSustain = 1.0f;
    float envRelease = 0.0f;

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

private:
    // Synth Properties
    float sampleRate;
    Voice voice;
    NoiseGenerator noiseGen;

    // MIDI handeling
    void noteOn (int note, int velocity);
    void noteOff (int note);

    // Catching invalid sample type
    std::atomic<unsigned> guardFlags{0};
};

// defensive guard against audio thread locking
static_assert (std::atomic<unsigned>::is_always_lock_free, "guardFlags must be lock-free for the audio thread");