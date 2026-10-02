// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include "model/Parameters.h"
#include "dsp/LFO.h"
#include "common/Modulation.h"
#include <cmath>

namespace
{
const UpdateContext kCtx { 44100.0f, 1.0f / 44100.0f, (1.0f / 44100.0f) * float (LFO::MAX_STEPS) };

// Calls a spec's apply function on a default SynthParams and returns the result.
SynthParams apply (Params::Index index, float value)
{
    SynthParams p;
    const auto& s = Params::kSpecs[index];
    jassert (s.apply != nullptr);
    s.apply (p, value, kCtx);
    return p;
}

bool hasNoApply (int index)
{
    switch (index)
    {
        case Params::oscTune:
        case Params::oscFine:
        case Params::octave:
        case Params::tuning:
        case Params::outputLevel:
            return true;
        default:
            return false;
    }
}
} // namespace

class ParameterApplyTests : public juce::UnitTest
{
public:
    ParameterApplyTests()
        : juce::UnitTest ("ParameterApply", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("apply pointers are wired exactly where expected");
        {
            for (int i = 0; i < Params::NumParams; ++i)
            {
                const bool null = Params::kSpecs[i].apply == nullptr;
                expect (null == hasNoApply (i),
                        juce::String (Params::kIds[i]) + " apply wiring changed");
            }
        }

        beginTest ("every apply function stays finite across its range");
        {
            for (int i = 0; i < Params::NumParams; ++i)
            {
                const auto& s = Params::kSpecs[i];
                if (s.apply == nullptr)
                    continue;

                for (int step = 0; step <= 20; ++step)
                {
                    const float v = s.min + (s.max - s.min) * float (step) / 20.0f;
                    SynthParams p;
                    s.apply (p, v, kCtx);

                    expect (std::isfinite (p.volumeTrim));
                    expect (std::isfinite (p.oscMix));
                    expect (std::isfinite (p.noiseMix));
                    expect (std::isfinite (p.velocitySensitivity));
                    expect (std::isfinite (p.filterQ));
                    expect (std::isfinite (p.envAttack));
                    expect (std::isfinite (p.envDecay));
                    expect (std::isfinite (p.envSustain));
                    expect (std::isfinite (p.envRelease));
                    expect (std::isfinite (p.lfoInc));
                    expect (std::isfinite (p.vibrato));
                    expect (std::isfinite (p.pwmDepth));
                    expect (std::isfinite (p.glideRate));
                    expect (std::isfinite (p.glideBend));
                    expect (std::isfinite (p.filterKeyTracking));
                    expect (std::isfinite (p.filterLFODepth));
                    expect (std::isfinite (p.filterAttack));
                    expect (std::isfinite (p.filterDecay));
                    expect (std::isfinite (p.filterSustain));
                    expect (std::isfinite (p.filterRelease));
                    expect (std::isfinite (p.filterEnvDepth));
                }
            }
        }

        beginTest ("applyPolyMode maps below/above 0.5 to mono/poly");
        {
            expectEquals (apply (Params::polyMode, 0.0f).numVoices, 1);
            expectEquals (apply (Params::polyMode, 0.49f).numVoices, 1);
            expectEquals (apply (Params::polyMode, 0.5f).numVoices, SynthLimits::MAX_VOICES);
            expectEquals (apply (Params::polyMode, 1.0f).numVoices, SynthLimits::MAX_VOICES);
        }

        beginTest ("applyOscMix is linear 0..1");
        {
            expectWithinAbsoluteError (apply (Params::oscMix, 0.0f).oscMix, 0.0f, 1.0e-6f);
            expectWithinAbsoluteError (apply (Params::oscMix, 25.0f).oscMix, 0.25f, 1.0e-6f);
            expectWithinAbsoluteError (apply (Params::oscMix, 100.0f).oscMix, 1.0f, 1.0e-6f);
        }

        beginTest ("applyNoise is a monotonic quadratic ending at 0.06");
        {
            expectWithinAbsoluteError (apply (Params::noise, 0.0f).noiseMix, 0.0f, 1.0e-6f);
            expectWithinAbsoluteError (apply (Params::noise, 100.0f).noiseMix, 0.06f, 1.0e-6f);

            float previous = -1.0f;
            for (int v = 0; v <= 100; v += 10)
            {
                const float n = apply (Params::noise, float (v)).noiseMix;
                expect (n >= previous);
                previous = n;
            }
        }

        beginTest ("applyFilterReso is exp(3 * normalised)");
        {
            expectWithinAbsoluteError (apply (Params::filterReso, 0.0f).filterQ, 1.0f, 1.0e-5f);
            expectWithinAbsoluteError (apply (Params::filterReso, 100.0f).filterQ, std::exp (3.0f), 1.0e-4f);
            expectWithinAbsoluteError (apply (Params::filterReso, 50.0f).filterQ, std::exp (1.5f), 1.0e-4f);
        }

        beginTest ("envelope times get shorter as the parameter rises");
        {
            const Params::Index indices[] { Params::envAttack, Params::envDecay, Params::envRelease,
                                            Params::filterAttack, Params::filterDecay, Params::filterRelease };

            // Attack/decay/release store a one-pole multiplier in [0, 1]:
            // a larger parameter means a slower stage, i.e. a multiplier closer to 1.
            auto multiplier = [&] (Params::Index index, float v)
            {
                SynthParams p;
                Params::kSpecs[index].apply (p, v, kCtx);

                if (index == Params::envAttack)
                    return p.envAttack;
                if (index == Params::envDecay)
                    return p.envDecay;
                if (index == Params::envRelease)
                    return p.envRelease;
                if (index == Params::filterAttack)
                    return p.filterAttack;
                if (index == Params::filterDecay)
                    return p.filterDecay;
                return p.filterRelease;
            };

            for (auto index : indices)
            {
                const float m0 = multiplier (index, 0.0f);
                const float m100 = multiplier (index, 100.0f);
                expect (m0 > 0.0f && m0 < 1.0f);
                expect (m100 > 0.0f && m100 < 1.0f);
                expect (m100 > m0, juce::String (Params::kIds[index]) + " should be slower at 100%");

                float previous = m0;
                for (int v = 10; v <= 100; v += 10)
                {
                    const float m = multiplier (index, float (v));
                    expect (m >= previous - 1.0e-9f);
                    previous = m;
                }
            }
        }

        beginTest ("applyEnvSustain is linear and filter sustain is squared");
        {
            expectWithinAbsoluteError (apply (Params::envSustain, 50.0f).envSustain, 0.5f, 1.0e-6f);
            expectWithinAbsoluteError (apply (Params::filterSustain, 50.0f).filterSustain, 0.25f, 1.0e-6f);
            expectWithinAbsoluteError (apply (Params::filterEnv, 50.0f).filterEnvDepth, 0.06f * 50.0f, 1.0e-6f);
        }

        beginTest ("applyFilterVelocity handles the OFF sentinel");
        {
            const auto off = apply (Params::filterVelocity, -100.0f);
            expect (off.ignoreVelocity);
            expectWithinAbsoluteError (off.velocitySensitivity, 0.0f, 1.0e-6f);

            const auto on = apply (Params::filterVelocity, 100.0f);
            expect (! on.ignoreVelocity);
            expectWithinAbsoluteError (on.velocitySensitivity, 0.05f, 1.0e-6f);

            // -90 is not below the cutoff, so it stays enabled.
            expect (! apply (Params::filterVelocity, -90.0f).ignoreVelocity);
        }

        beginTest ("applyLfoRate uses the shared lfoRateHz curve");
        {
            const float expected = lfoRateHz (0.5f) * kCtx.inverseUpdateRate * 2.0f * PI;
            expectWithinAbsoluteError (apply (Params::lfoRate, 0.5f).lfoInc, expected, 1.0e-9f);
            expectWithinAbsoluteError (apply (Params::lfoRate, 0.0f).lfoInc,
                                       lfoRateHz (0.0f) * kCtx.inverseUpdateRate * 2.0f * PI, 1.0e-9f);
        }

        beginTest ("applyVibrato is PWM-only when negative");
        {
            const auto positive = apply (Params::vibrato, 50.0f);
            expectWithinAbsoluteError (positive.pwmDepth, vibratoDepth (50.0f), 1.0e-6f);
            expectWithinAbsoluteError (positive.vibrato, positive.pwmDepth, 1.0e-6f);

            const auto negative = apply (Params::vibrato, -50.0f);
            expectWithinAbsoluteError (negative.pwmDepth, vibratoDepth (-50.0f), 1.0e-6f);
            expectWithinAbsoluteError (negative.vibrato, 0.0f, 1.0e-6f);
        }

        beginTest ("applyGlideMode and applyGlideBend pass through");
        {
            expectEquals (apply (Params::glideMode, 2.0f).glideMode, 2);
            expectWithinAbsoluteError (apply (Params::glideBend, -12.0f).glideBend, -12.0f, 1.0e-6f);
        }

        beginTest ("applyGlideRate is off below 2% and then a valid coefficient");
        {
            expectWithinAbsoluteError (apply (Params::glideRate, 0.0f).glideRate, 1.0f, 1.0e-6f);
            expectWithinAbsoluteError (apply (Params::glideRate, 1.0f).glideRate, 1.0f, 1.0e-6f);

            const float expected = glideCoefficient (2.0f, kCtx.inverseUpdateRate);
            expectWithinAbsoluteError (apply (Params::glideRate, 2.0f).glideRate, expected, 1.0e-6f);
            expect (apply (Params::glideRate, 100.0f).glideRate < expected);
        }

        beginTest ("applyFilterFreq is a linear key-tracking offset");
        {
            expectWithinAbsoluteError (apply (Params::filterFreq, 0.0f).filterKeyTracking, -1.5f, 1.0e-6f);
            expectWithinAbsoluteError (apply (Params::filterFreq, 100.0f).filterKeyTracking, 6.5f, 1.0e-6f);
        }

        beginTest ("applyFilterLFO is a monotonic quadratic ending at 2.5");
        {
            expectWithinAbsoluteError (apply (Params::filterLFO, 0.0f).filterLFODepth, 0.0f, 1.0e-6f);
            expectWithinAbsoluteError (apply (Params::filterLFO, 100.0f).filterLFODepth, 2.5f, 1.0e-6f);
            expect (apply (Params::filterLFO, 50.0f).filterLFODepth < apply (Params::filterLFO, 100.0f).filterLFODepth);
        }
    }
};

static ParameterApplyTests parameterApplyTests;
