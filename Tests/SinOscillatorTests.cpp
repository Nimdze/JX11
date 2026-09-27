#include <juce_core/juce_core.h>
#include <cmath>
#include <algorithm>
#include "SinOscillator.h"
#include "Constants.h"

namespace
{

SinOscillator makeOsc (float inc, float amp)
{
    SinOscillator o;
    o.amplitude = amp;
    o.inc = inc;
    o.reset();
    return o;
}
} // namespace

class SinOscillatorTests : public juce::UnitTest
{
public:
    SinOscillatorTests()
        : juce::UnitTest ("SinOscillator", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("reset() makes output deterministic");
        {
            auto a = makeOsc (0.01f, 1.0f);
            auto b = makeOsc (0.01f, 1.0f);

            for (int i = 0; i < 1000; ++i)
                expectEquals (a.nextSample(), b.nextSample());
        }

        beginTest ("reset() restarts the phase");
        {
            auto o = makeOsc (0.01f, 1.0);

            float firstRun[100];
            for (int i = 0; i < 100; ++i)
                firstRun[i] = o.nextSample();

            o.reset();

            for (int i = 0; i < 100; ++i)
                expectEquals (o.nextSample(), firstRun[i]);
        }

        beginTest ("output stays within +/- amplitude");
        {
            const float amp = 0.5f;
            auto o = makeOsc (0.01f, amp);

            float peak = 0.0f;
            for (int i = 0; i < 44100; ++i)
                peak = std::max (peak, std::abs (o.nextSample()));

            expect (peak <= amp + 1e-3f);
        }

        beginTest ("matches a reference sine");
        {
            const float inc = 0.01f;
            const float amp = 1.0f;
            auto o = makeOsc (inc, amp);

            const float omega = inc * 2 * PI;

            for (int k = 0; k < 500; ++k)
            {
                const float expected = amp * std::sin (omega * static_cast<float> (k + 1));
                expectWithinAbsoluteError (o.nextSample(), expected, 1e-3f);
            }
        }

        beginTest ("amplitude scales output linearly");
        {
            auto half = makeOsc (0.01f, 0.5f);
            auto full = makeOsc (0.01f, 1.0f);

            for (int k = 0; k < 500; ++k)
                expectWithinAbsoluteError (full.nextSample(), 2.0f * half.nextSample(), 1e-3f);
        }
    }
};

static SinOscillatorTests sinOscillatorTests;
