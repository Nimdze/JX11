// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include "dsp/Synth.h"
#include <cmath>

namespace
{
constexpr int kBlockSize = 4800;   // ~109 ms at 44.1 kHz
constexpr int kWindows = 8;
constexpr int kWindowSize = kBlockSize / kWindows;

// A fixed, non-trivial patch. Changes to the DSP are expected to change this
// fingerprint; the test's job is to make such a change impossible to miss.
SynthParams goldenParams()
{
    SynthParams p;
    p.tune = 44100.0f * std::exp (0.05776226505f * -36.3763f);
    p.detune = 0.994f;
    p.oscMix = 0.5f;
    p.volumeTrim = 0.00384f;
    p.numVoices = 1;
    p.envAttack = 0.75f;
    p.envDecay = 0.75f;
    p.envSustain = 1.0f;
    p.envRelease = 0.75f;
    p.filterKeyTracking = 2.0f;
    p.filterQ = 1.5f;
    p.filterEnvDepth = 1.0f;
    p.lfoInc = 0.0005f;
    p.vibrato = 0.01f;
    p.pwmDepth = 0.01f;
    return p;
}

// RMS of each window of the rendered left channel.
std::array<float, kWindows> renderFingerprint()
{
    static Synth synth;
    static bool prepared = false;

    if (! prepared)
    {
        synth.allocateResources (44100.0, kBlockSize);
        prepared = true;
    }

    synth.reset();
    synth.setParam (goldenParams());

    float left[kBlockSize] {};
    float right[kBlockSize] {};
    float* outputs[2] = {left, right};

    synth.midiMessage (0x90, 60, 100);
    synth.render (outputs, kBlockSize);

    std::array<float, kWindows> rms {};

    for (int w = 0; w < kWindows; ++w)
    {
        double sum = 0.0;
        for (int i = 0; i < kWindowSize; ++i)
        {
            const double x = left[w * kWindowSize + i];
            sum += x * x;
        }
        rms[static_cast<std::size_t> (w)] = static_cast<float> (std::sqrt (sum / kWindowSize));
    }

    return rms;
}
} // namespace

class GoldenRenderTests : public juce::UnitTest
{
public:
    GoldenRenderTests()
        : juce::UnitTest ("GoldenRender", "JX11")
    {
    }

    void runTest() override
    {
        // Captured from the reference implementation. Regenerate deliberately
        // when the DSP is intentionally changed.
        const std::array<float, kWindows> expected {
            0.05838623f, 0.05454377f, 0.05561210f, 0.06790085f,
            0.06438537f, 0.07782743f, 0.07468926f, 0.08295093f
        };

        const auto actual = renderFingerprint();

        beginTest ("offline render is finite and non-silent");
        {
            for (float v : actual)
                expect (std::isfinite (v));

            expect (actual[1] > 1.0e-4f, "render produced silence");
        }

        beginTest ("offline render matches the stored fingerprint");
        {
            for (int w = 0; w < kWindows; ++w)
            {
                const float e = expected[static_cast<std::size_t> (w)];
                const float a = actual[static_cast<std::size_t> (w)];

                expectWithinAbsoluteError (a, e, juce::jmax (1.0e-4f, 0.01f * e),
                                           "window " + juce::String (w) + " (actual " +
                                           juce::String (a, 8) + ")");
            }
        }
    }
};

static GoldenRenderTests goldenRenderTests;
