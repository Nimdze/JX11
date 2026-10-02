// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include <cmath>
#include "dsp/Filter.h"
#include "common/Constants.h"

namespace
{
Filter makeFilter (float sampleRate, float cutoff, float Q)
{
    Filter f;
    f.sampleRate = sampleRate;
    f.reset(); // clear state AND coefficients
    f.updateCoefficients (cutoff, Q);
    return f;
}

// Runs `freq` Hz for `n` samples and returns the RMS of the second half,
// skipping the settling transient.
float steadyStateRms (Filter& f, float freq, float amp, int n)
{
    const float sr = f.sampleRate;
    double sum = 0.0;
    int counted = 0;

    for (int i = 0; i < n; ++i)
    {
        const float x = amp * std::sin (2.0f * PI * freq * static_cast<float> (i) / sr);
        const float y = f.render (x);

        if (i >= n / 2)
        {
            sum += static_cast<double> (y) * y;
            ++counted;
        }
    }

    return static_cast<float> (std::sqrt (sum / counted));
}
} // namespace

class FilterTests : public juce::UnitTest
{
public:
    FilterTests()
        : juce::UnitTest ("Filter", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("cutoff is clamped below Nyquist at low sample rates");
        {
            // At 32 kHz the 20 kHz request is above Nyquist; without the clamp
            // tan() would go negative and produce invalid coefficients.
            expectWithinAbsoluteError (Filter::clampCutoff (20000.0f, 32000.0f), 0.45f * 32000.0f, 1.0e-3f);
            expectWithinAbsoluteError (Filter::clampCutoff (20000.0f, 44100.0f), 0.45f * 44100.0f, 1.0e-3f);
            expectWithinAbsoluteError (Filter::clampCutoff (1000.0f, 44100.0f), 1000.0f, 1.0e-3f);
            expectWithinAbsoluteError (Filter::clampCutoff (1.0f, 44100.0f), 20.0f, 1.0e-3f);

            // The clamped coefficient must still be finite and bounded at 32 kHz.
            auto f = makeFilter (32000.0f, 20000.0f, 0.707f);
            float peak = 0.0f;
            for (int i = 0; i < 4096; ++i)
            {
                const float y = f.render (i == 0 ? 1.0f : 0.0f);
                expect (std::isfinite (y));
                peak = std::max (peak, std::abs (y));
            }
            expect (peak < 4.0f);
        }

        beginTest ("reset() produces silence");
        {
            Filter f;
            f.sampleRate = 44100.0f;
            f.reset();

            expectEquals (f.render (1.0f), 0.0f);
            expectEquals (f.render (-0.5f), 0.0f);
        }

        beginTest ("reset() clears previous state");
        {
            auto a = makeFilter (44100.0f, 1000.0f, 0.707f);
            for (int i = 0; i < 500; ++i)
                a.render (1.0f);

            a.reset();
            a.updateCoefficients (1000.0f, 0.707f);

            auto b = makeFilter (44100.0f, 1000.0f, 0.707f);

            for (int i = 0; i < 500; ++i)
                expectWithinAbsoluteError (a.render (1.0f), b.render (1.0f), 1e-6f);
        }

        beginTest ("output scales linearly with input");
        {
            auto a = makeFilter (44100.0f, 2000.0f, 0.707f);
            auto b = makeFilter (44100.0f, 2000.0f, 0.707f);

            for (int i = 0; i < 1000; ++i)
            {
                const float x = std::sin (0.05f * static_cast<float> (i));
                expectWithinAbsoluteError (b.render (2.0f * x), 2.0f * a.render (x), 1e-4f);
            }
        }

        beginTest ("lowpass passes DC at unity gain");
        {
            auto f = makeFilter (44100.0f, 200.0f, 0.707f);

            float y = 0.0f;
            for (int i = 0; i < 44100; ++i)
                y = f.render (1.0f);

            expectWithinAbsoluteError (y, 1.0f, 1e-2f);
        }

        beginTest ("gain at cutoff equals Q");
        {
            const float sr = 44100.0f;
            const float fc = 1000.0f;
            const float amp = 0.1f;

            const float qs[] = {0.707f, 2.0f, 10.0f};

            for (float Q : qs)
            {
                auto f = makeFilter (sr, fc, Q);
                const float rms = steadyStateRms (f, fc, amp, static_cast<int> (sr));

                // Sine RMS = amp / sqrt(2); lowpass gain at cutoff = Q.
                const float expected = (amp / std::sqrt (2.0f)) * Q;
                expectWithinAbsoluteError (rms, expected, 0.1f * expected);
            }
        }

        beginTest ("attenuates frequencies above cutoff");
        {
            auto passFilter = makeFilter (44100.0f, 500.0f, 0.707f);
            auto stopFilter = makeFilter (44100.0f, 500.0f, 0.707f);

            const float pass = steadyStateRms (passFilter, 100.0f, 0.5f, 44100);
            const float stop = steadyStateRms (stopFilter, 8000.0f, 0.5f, 44100);

            expect (stop < pass * 0.1f);
        }

        beginTest ("stays finite and bounded for extreme settings");
        {
            // Q = 50 at 20 kHz is the most resonant setting the UI allows; the
            // response must stay bounded (the old bound of 1e4 would pass an
            // exponential blow-up).
            auto f = makeFilter (44100.0f, 20000.0f, 50.0f);

            float peak = 0.0f;
            for (int i = 0; i < 20000; ++i)
            {
                const float y = f.render (i == 0 ? 1.0f : 0.0f);

                expect (std::isfinite (y));
                peak = std::max (peak, std::abs (y));
            }

            expect (peak < 4.0f, "resonant filter grew past a safe bound");
        }

        beginTest ("rolls off at roughly 12 dB/octave");
        {
            const float fc = 500.0f;

            auto f4 = makeFilter (44100.0f, fc, 0.707f);
            auto f8 = makeFilter (44100.0f, fc, 0.707f);

            const float at4 = steadyStateRms (f4, 4.0f * fc, 0.5f, 44100);
            const float at8 = steadyStateRms (f8, 8.0f * fc, 0.5f, 44100);

            const float ratio = at4 / at8; // one octave apart -> ~4x (-12 dB)
            expect (ratio > 3.0f && ratio < 5.5f);
        }

        beginTest ("sample rate affects the response");
        {
            auto a = makeFilter (44100.0f, 1000.0f, 0.707f);
            auto b = makeFilter (96000.0f, 1000.0f, 0.707f);

            bool differs = false;
            for (int i = 0; i < 64; ++i)
            {
                const float x = i == 0 ? 1.0f : 0.0f;
                if (std::abs (a.render (x) - b.render (x)) > 1e-9f)
                    differs = true;
            }

            expect (differs);
        }

        beginTest ("impulse response decays to silence");
        {
            auto f = makeFilter (44100.0f, 1000.0f, 0.707f);

            const int n = 44100;
            double tailSum = 0.0;
            int counted = 0;

            for (int i = 0; i < n; ++i)
            {
                const float y = f.render (i == 0 ? 1.0f : 0.0f);

                if (i >= n * 9 / 10)
                {
                    tailSum += static_cast<double> (y) * y;
                    ++counted;
                }
            }

            const float tailRms = static_cast<float> (std::sqrt (tailSum / counted));
            expect (tailRms < 1e-4f);
        }
    }
};

static FilterTests filterTests;
