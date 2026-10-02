// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include "dsp/Envelope.h"

namespace
{
Envelope makeEnvelope (float attack, float decay, float sustain, float release)
{
    Envelope env;
    env.attackMultiplier = attack;
    env.decayMultiplier = decay;
    env.sustainLevel = sustain;
    env.releaseMultiplier = release;
    env.reset();
    return env;
}

void advance (Envelope& env, int samples)
{
    for (int i = 0; i < samples; ++i)
        env.nextValue();
}
} // namespace

class EnvelopeTests : public juce::UnitTest
{
public:
    EnvelopeTests()
        : juce::UnitTest ("Envelope", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("reset() leaves the envelope idle");
        {
            Envelope env;
            env.level = 0.7f;
            env.reset();

            expectEquals (env.level, 0.0f);
            expect (!env.isActive());
            expect (!env.isInAttack());
        }

        beginTest ("attack() activates and level rises monotonically");
        {
            auto env = makeEnvelope (0.99f, 0.99f, 0.5f, 0.99f);
            env.attack();

            expect (env.isActive());
            expect (env.isInAttack());

            float previous = env.level;
            for (int i = 0; i < 100 && env.isInAttack(); ++i)
            {
                const float v = env.nextValue();
                expect (v >= previous - 1.0e-6f);
                previous = v;
            }
        }

        beginTest ("attack hands over to decay and sattles at sustain");
        {
            auto env = makeEnvelope (0.99f, 0.99f, 0.5f, 0.99f);
            env.attack();

            int guard = 0;
            while (env.isInAttack() && guard++ < 100000)
                env.nextValue();

            expect (!env.isInAttack());

            advance (env, 100000);
            expectWithinAbsoluteError (env.level, 0.5f, 1.0e-2f);
            expect (env.isActive());
        }

        beginTest ("sustain at 100% holds near full level");
        {
            auto env = makeEnvelope (0.99f, 0.99f, 1.0f, 0.99f);
            env.attack();

            advance (env, 200000);
            expectWithinAbsoluteError (env.level, 1.0f, 1.0e-2f);
        }

        beginTest ("release() fades out and deactivates");
        {
            auto env = makeEnvelope (0.99f, 0.99f, 0.5f, 0.99f);
            env.attack();
            advance (env, 100000);
            expect (env.isActive());

            env.release();
            expect (!env.isInAttack());

            int guard = 0;
            while (env.isActive() && guard++ < 1000000)
                env.nextValue();

            expect (!env.isActive());
        }

        beginTest ("attack() continues from the current level (legato)");
        {
            auto env = makeEnvelope (0.99f, 0.99f, 0.5f, 0.99f);
            env.level = 0.5f;
            env.attack();

            expect (env.level > 0.5f);
        }
    }
};

static EnvelopeTests envelopeTests;