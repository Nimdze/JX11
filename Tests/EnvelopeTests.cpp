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

// Fast but still well-conditioned one-pole coefficient; converges in a few
// hundred samples instead of hundreds of thousands.
constexpr float kFast = 0.8f;
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
            auto env = makeEnvelope (kFast, kFast, 0.5f, kFast);
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

            expect (!env.isInAttack(), "attack never handed over to decay");
        }

        beginTest ("attack hands over to decay and settles at sustain");
        {
            auto env = makeEnvelope (kFast, kFast, 0.5f, kFast);
            env.attack();

            int guard = 0;
            while (env.isInAttack() && guard++ < 1000)
                env.nextValue();

            advance (env, 200);
            expectWithinAbsoluteError (env.level, 0.5f, 1.0e-3f);
            expect (env.isActive());
        }

        beginTest ("sustain at 100% holds near full level");
        {
            auto env = makeEnvelope (kFast, kFast, 1.0f, kFast);
            env.attack();

            advance (env, 400);
            expectWithinAbsoluteError (env.level, 1.0f, 1.0e-3f);
        }

        beginTest ("release() fades out and deactivates");
        {
            auto env = makeEnvelope (kFast, kFast, 0.5f, kFast);
            env.attack();
            advance (env, 200);
            expect (env.isActive());

            env.release();
            expect (!env.isInAttack());

            int guard = 0;
            while (env.isActive() && guard++ < 10000)
                env.nextValue();

            expect (!env.isActive());
        }

        beginTest ("attack() continues from the current level (legato)");
        {
            auto env = makeEnvelope (kFast, kFast, 0.5f, kFast);
            env.level = 0.5f;
            env.attack();

            expect (env.level > 0.5f);
        }

        beginTest ("zero attack time snaps to full level in one sample");
        {
            auto env = makeEnvelope (0.0f, kFast, 1.0f, kFast);
            env.attack();

            expect (env.isInAttack());
            env.nextValue();
            expect (env.level >= 2.0f - 1.0e-6f);
        }

        beginTest ("zero release time snaps to silence in one sample");
        {
            auto env = makeEnvelope (kFast, kFast, 0.5f, 0.0f);
            env.attack();
            advance (env, 200);
            expect (env.isActive());

            env.release();
            env.nextValue();

            expect (!env.isActive());
            expectEquals (env.level, 0.0f);
        }
    }
};

static EnvelopeTests envelopeTests;
