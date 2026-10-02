#pragma once

#include <juce_core/juce_core.h>
#include <juce_audio_basics/juce_audio_basics.h>
#include <atomic>
#include "dsp/Voice.h"
#include "dsp/NoiseGenerator.h"
#include "dsp/LFO.h"
#include "model/SynthParams.h"
#include "model/MidiState.h"
#include "dsp/VoiceAllocator.h"

class Synth
{
public:
    // Synth properties
    static constexpr int MAX_VOICES = SynthLimits::MAX_VOICES;

    juce::LinearSmoothedValue<float> outputLevelSmoother = 1.0f;

    MidiState midi;

    // MIDI CC number that extra filter resonance is mapped to. Changed by MIDI
    // Learn; written on the audio thread from a copy published by the processor.
    uint8_t resoCC = 0x47;

    // Basic operations
    Synth();
    void allocateResources (double sampleRate, int samplesPerBlock);
    void deallocateResources();
    void reset();

    // Stores the per-block parameter snapshot. When no voice is sounding the
    // modulation smoothers snap to the new values, so the first note does not
    // sweep in from the previous (or a default) state.
    void setParam (const SynthParams& value) noexcept
    {
        params = value;

        if (!anyVoiceActive())
        {
            filterZip = params.filterKeyTracking + midi.filterCtl;
            detuneSmoother.setCurrentAndTargetValue (params.detune);
            filterQSmoother.setCurrentAndTargetValue (params.filterQ);
        }
        else
        {
            detuneSmoother.setTargetValue (params.detune);
            filterQSmoother.setTargetValue (params.filterQ);
        }
    }

    // Audio rendering
    void render (float** outputBuffers, int sampleCount);

    // MIDI handling
    void midiMessage (uint8_t data0, uint8_t data1, uint8_t data2);

    // Guarding against ivalid samples
    [[nodiscard]] unsigned takeGuardFlags() noexcept { return guardFlags.exchange (0, std::memory_order_relaxed); }

    [[nodiscard]] float calcPeriod (int v, int note) const;

private:
    // Synth Properties
    SynthParams params;

    float sampleRate = 44100.0f;
    Voice voices[MAX_VOICES];
    VoiceAllocator allocator;

    LFO lfo;

    NoiseGenerator noiseGen;

    // Filter-modulation and continuous-parameter smoothing. Advanced once per
    // sample so host automation cannot step at block boundaries.
    float filterZip = 0.0f;
    juce::LinearSmoothedValue<float> detuneSmoother{1.0f};
    juce::LinearSmoothedValue<float> filterQSmoother{1.0f};
    float smoothedDetune = 1.0f;
    float smoothedFilterQ = 1.0f;

    [[nodiscard]] bool anyVoiceActive() const noexcept
    {
        for (int v = 0; v < MAX_VOICES; ++v)
            if (voices[v].env.isActive())
                return true;

        return false;
    }

    void controlChange (uint8_t data1, uint8_t data2);

    void updateLFO();
    inline void updatePeriod (Voice& voice)
    {
        voice.osc1.period = voice.period * midi.pitchBend;
        voice.osc2.period = voice.osc1.period * smoothedDetune;
    }

    // Catching invalid sample type
    std::atomic<unsigned> guardFlags{0};
};

// defensive guard against audio thread locking
static_assert (std::atomic<unsigned>::is_always_lock_free, "guardFlags must be lock-free for the audio thread");
