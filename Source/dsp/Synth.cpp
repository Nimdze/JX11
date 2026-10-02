#include "dsp/Synth.h"
#include "dsp/Voice.h"
#include "dsp/Utils.h"
#include "common/Modulation.h"

Synth::Synth()
{
    sampleRate = 44100.0f;
    allocator.prepare (voices, &params, sampleRate);
}

// =============================
// Basic operations
// =============================
void Synth::allocateResources (double sampleRate_, int /*samplesPerBlock*/)
{
    sampleRate = static_cast<float> (sampleRate_);

    for (int v = 0; v < MAX_VOICES; ++v)
    {
        voices[v].filter.sampleRate = sampleRate;
    }

    allocator.prepare (voices, &params, sampleRate);

    detuneSmoother.reset (sampleRate, kParamSmoothingSeconds);
    filterQSmoother.reset (sampleRate, kParamSmoothingSeconds);
    detuneSmoother.setCurrentAndTargetValue (params.detune);
    filterQSmoother.setCurrentAndTargetValue (params.filterQ);
    smoothedDetune = params.detune;
    smoothedFilterQ = params.filterQ;
}

void Synth::deallocateResources() {}

void Synth::reset()
{
    for (int v = 0; v < MAX_VOICES; ++v)
        voices[v].reset();

    allocator.reset();
    noiseGen.reset();
    midi.reset();

    outputLevelSmoother.reset (sampleRate, 0.05);

    // Start the filter modulation at its steady-state value so the first note
    // does not sweep in from zero (the book's start-of-playback artifact).
    filterZip = params.filterKeyTracking + midi.filterCtl;

    detuneSmoother.reset (sampleRate, kParamSmoothingSeconds);
    filterQSmoother.reset (sampleRate, kParamSmoothingSeconds);
    detuneSmoother.setCurrentAndTargetValue (params.detune);
    filterQSmoother.setCurrentAndTargetValue (params.filterQ);
    smoothedDetune = params.detune;
    smoothedFilterQ = params.filterQ;

    lfo.reset();
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
        if (voice.env.isActive())
        {
            updatePeriod (voice);
            voice.glideRate = params.glideRate;
            voice.pitchBend = midi.pitchBend;
            voice.filterEnvDepth = params.filterEnvDepth;
        }
    }

    for (int sample = 0; sample < sampleCount; ++sample)
    {
        smoothedDetune = detuneSmoother.getNextValue();
        smoothedFilterQ = filterQSmoother.getNextValue();

        updateLFO();

        const float noise = noiseGen.nextValue() * params.noiseMix;

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
            voice.filter.reset();
        }
    }

#if JX11_ENABLE_SAMPLE_GUARD
    guardFlags.fetch_or (protectYourEars (outputBufferLeft, sampleCount), std::memory_order_relaxed);
    if (outputBufferRight != nullptr)
        guardFlags.fetch_or (protectYourEars (outputBufferRight, sampleCount), std::memory_order_relaxed);
#else
    juce::ignoreUnused (outputBufferLeft, outputBufferRight);
#endif
}

// =============================
// MIDI handeling
// =============================

void Synth::midiMessage (uint8_t data0, uint8_t data1, uint8_t data2)
{
    switch (data0 & 0xF0)
    {
        case 0x80: // Note off
            allocator.noteOff (data1 & 0x7F, midi.sustainPedalPressed);
            break;

        case 0x90: // Note on
        {
            uint8_t note = data1 & 0x7F;
            uint8_t velo = data2 & 0x7F;
            if (velo > 0)
            {
                allocator.noteOn (note, velo);
            }
            else
            {
                allocator.noteOff (note, midi.sustainPedalPressed);
            }
            break;
        }

        case 0xB0: // Control change
        {
            controlChange (data1, data2);
            break;
        }

        case 0xD0:
            midi.pressure = 0.0001f * float (data1 * data1);
            break;

        case 0xE0: // Pitch bend
        {
            // float(data1 +128 * data2 - 8192) gives anumber between -8192 and 8191
            // mapping this range to 2^(2/12) to 2^(-2/12) (multiplying the period)
            // 2^((-2*(data/8192))/12) = (2^((-2/8192)/12))^data = exp^(M*data)
            midi.pitchBend = std::exp (-kPitchBendRate * float (data1 + 128 * data2 - 8192));
            break;
        }
    }
}

void Synth::controlChange (uint8_t data1, uint8_t data2)
{
    switch (data1)
    {
        // Sustain pedal
        case 0x40:
            midi.sustainPedalPressed = (data2 >= 64);

            if (!midi.sustainPedalPressed)
                allocator.noteOff (VoiceAllocator::SUSTAIN, false);

            break;

        case 0x01:
            midi.modWheel = modWheelDepth (data2);
            break;

        case 0x4A: // Filter +
            midi.filterCtl = 0.02f * float (data2);
            break;

        case 0x4B: // Filter -
            midi.filterCtl = -0.03f * float (data2);
            break;

        default: // panic, all notes off - 120 or above
            if (data1 >= 0X78)
            {
                allocator.allNotesOff();
                midi.sustainPedalPressed = false;
            }
            break;
    }

    // Resonance is a learnable CC, so it cannot be a switch case (case labels
    // must be constant expressions).
    if (data1 == resoCC)
    {
        midi.resonanceCtl = 154.0f / float (154 - data2);
    }
}

float Synth::calcPeriod (int v, int note) const
{
    return periodForNote (params.tune, params.detune, v, note);
}

// =============================
// Modulation
// =============================

void Synth::updateLFO()
{
    if (!lfo.advance (params.lfoInc))
        return;

    const float sine = lfo.current();

    float vibratoMod = 1.0f + sine * (midi.modWheel + params.vibrato);
    float pwm = 1.0f + sine * (midi.modWheel + params.pwmDepth);

    float filterMod = params.filterKeyTracking + midi.filterCtl + (params.filterLFODepth + midi.pressure) * sine;

    filterZip += kFilterModSmoothing * (filterMod - filterZip);

    for (int v = 0; v < MAX_VOICES; ++v)
    {
        Voice& voice = voices[v];
        if (voice.env.isActive())
        {
            voice.osc1.modulation = vibratoMod;
            voice.osc2.modulation = pwm;
            // if vibrato is non-negative both values will be the same and both oscillators will have vibrato,
            // if vibrato is negative then vibratoMod is 0 and only the osc2 get modulated (resulting in pwm)

            voice.filterQ = smoothedFilterQ * midi.resonanceCtl;
            voice.filterMod = filterZip;
            voice.updateLFO();
            updatePeriod (voice);
        }
    }
}
