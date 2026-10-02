#include <juce_core/juce_core.h>
#include <algorithm>
#include "dsp/Synth.h"
#include "dsp/Utils.h"

namespace
{
bool isSilent (const float* b, int n)
{
    for (int i = 0; i < n; ++i)
        if (b[i] != 0.0f)
            return false;
    return true;
}
} // namespace

class SynthTests : public juce::UnitTest
{
public:
    SynthTests()
        : juce::UnitTest ("Synth", "JX11")
    {
    }

    void runTest() override
    {
        constexpr int n = 512;
        float left[n]{}, right[n]{};
        float* outputs[2] = {left, right};

        Synth synth;
        SynthParams testParams;
        const float sampleRate = 44100.0f;

        synth.allocateResources (sampleRate, n);

        // set parameters for a bare synth that doesnt have update() called
        testParams.tune = sampleRate * std::exp (kSemitoneLog * -36.3763f); // octave 0, tuning 0
        testParams.detune = 1.0f;
        testParams.oscMix = 0.0f;
        testParams.volumeTrim = 0.00384f;

        testParams.envAttack = 0.75f; // fast
        testParams.envDecay = 0.75f;
        testParams.envSustain = 1.0f;  // hold
        testParams.envRelease = 0.75f; // fast (~32 samples to silence)

        synth.setParam (testParams);

        // reset synth + zero the buffers before each test
        auto fresh = [&]
        {
            synth.reset();
            testParams.numVoices = 1;
            testParams.oscMix = 0.0f;
            testParams.detune = 1.0f;
            synth.setParam (testParams);
            (void)synth.takeGuardFlags(); // clear diagnostics

            std::fill (left, left + n, 0.0f);
            std::fill (right, right + n, 0.0f);
        };

        auto renderUntilSilent = [&] (int maxBlocks)
        {
            for (int b = 0; b < maxBlocks; ++b)
            {
                std::fill (left, left + n, 0.0f);
                std::fill (right, right + n, 0.0f);
                synth.render (outputs, n);
                if (isSilent (left, n))
                    return true;
            }
            return false;
        };

        //=============================================================
        // Basic MIDI Testing
        //=============================================================

        beginTest ("silent before any note");
        fresh();
        synth.render (outputs, n);
        expect (isSilent (left, n));

        beginTest ("note on produces audio");
        fresh();
        synth.midiMessage (0x90, 60, 100);
        synth.render (outputs, n);
        expect (!isSilent (left, n));

        beginTest ("note off fades to silence");
        fresh();
        synth.midiMessage (0x90, 60, 100);
        synth.render (outputs, n);
        synth.midiMessage (0x80, 60, 0);
        expect (renderUntilSilent (10));

        beginTest ("note on with velocity 0 acts as note off");
        fresh();
        synth.midiMessage (0x90, 60, 100);
        synth.render (outputs, n);
        synth.midiMessage (0x90, 60, 0); // off
        expect (renderUntilSilent (10));

        beginTest ("all channels are accepted");
        fresh();
        synth.midiMessage (0x92, 60, 100);
        synth.render (outputs, n);
        expect (!isSilent (left, n));

        beginTest ("note off for a different note doesn't stop the voice");
        fresh();
        synth.midiMessage (0x90, 60, 100);
        synth.midiMessage (0x80, 62, 0);
        synth.render (outputs, n);
        expect (!isSilent (left, n));

        beginTest ("period halves per octave and matches A440");
        {
            fresh();
            testParams.detune = 1.0f;
            testParams.tune = sampleRate * std::exp (kSemitoneLog * -36.3763f);
            synth.setParam (testParams);
            const float p69 = synth.calcPeriod (0, 69);

            expectWithinAbsoluteError (p69, sampleRate / 440.0f, 0.5f);
            expectWithinAbsoluteError (synth.calcPeriod (0, 81), p69 * 0.5f, 0.5f); // +12 semitones
            expectWithinAbsoluteError (synth.calcPeriod (0, 57), p69 * 2.0f, 1.0f); // -12 semitones
        }

        //=============================================================
        // Tuning, detuning, pitch bend
        //=============================================================

        beginTest ("pitch bend maps centre, max up, max down");
        {
            fresh();
            synth.midiMessage (0xE0, 0, 64); // centre
            expectWithinAbsoluteError (synth.midi.pitchBend, 1.0f, 1.0e-3f);
            synth.midiMessage (0xE0, 127, 127); // max up
            expectWithinAbsoluteError (synth.midi.pitchBend, std::pow (2.0f, -2.0f / 12.0f), 1.0e-3f);
            synth.midiMessage (0xE0, 0, 0); // max up
            expectWithinAbsoluteError (synth.midi.pitchBend, std::pow (2.0f, 2.0f / 12.0f), 1.0e-3f);
        }

        beginTest ("osc2 silent when oscMix is 0");
        {
            fresh();
            testParams.oscMix = 0.0f;
            testParams.detune = 1.0f;
            synth.setParam (testParams);
            synth.midiMessage (0x90, 60, 100);
            synth.render (outputs, n);
            expect (!isSilent (left, n));
        }

        beginTest ("equally mixed, undetuned oscillators cancel");
        {
            fresh();
            testParams.oscMix = 1.0f;
            testParams.detune = 1.0f;
            synth.setParam (testParams);
            synth.midiMessage (0x90, 60, 100);
            synth.render (outputs, n);
            expect (isSilent (left, n)); // sample1 - sample2 == 0
        }

        beginTest ("detuning prevents cancellation");
        {
            fresh();
            testParams.oscMix = 1.0f;
            testParams.detune = 0.994f;
            synth.setParam (testParams);
            synth.midiMessage (0x90, 60, 100);
            synth.render (outputs, n);
            expect (!isSilent (left, n));
        }

        //=============================================================
        // Voice managment - polyphony, voice stealing, sustain pedal, mono legato
        //=============================================================

        beginTest ("polyphony - two notes sound and are independent");
        {
            fresh();
            testParams.numVoices = Synth::MAX_VOICES;
            synth.setParam (testParams);

            synth.midiMessage (0x90, 60, 80);
            synth.midiMessage (0x90, 64, 80);
            synth.render (outputs, n);
            expect (!isSilent (left, n));

            synth.midiMessage (0x80, 60, 0); // release first note only
            synth.render (outputs, n);
            expect (!isSilent (left, n)); // second note still plays
        }

        beginTest ("voice stealing - more notes than voices stays finite");
        {
            fresh();
            testParams.numVoices = Synth::MAX_VOICES;
            synth.setParam (testParams);
            for (int i = 0; i <= Synth::MAX_VOICES; ++i) // MAX_VOICES + 1 notes forces one voice to be stolen
                synth.midiMessage (0x90, (uint8_t)(60 + i), 80);

            synth.render (outputs, n);

            // Clamping acceptable for big chord, NaN/inf is not
            const unsigned flags = synth.takeGuardFlags();
            expect ((flags & SampleGuardNaN) == 0);
            expect ((flags & SampleGuardInf) == 0);
        }

        beginTest ("sustain pedal holds a released note until lifted");
        {
            fresh();
            synth.midiMessage (0x90, 60, 80);
            synth.render (outputs, n);

            synth.midiMessage (0xB0, 64, 127); // pedal pressed
            synth.midiMessage (0x80, 60, 0);   // key released
            synth.render (outputs, n);
            expect (!isSilent (left, n)); // key is supposed to still sound because of the pedal

            synth.midiMessage (0xB0, 64, 0); // pedal released
            expect (renderUntilSilent (10)); // now key fades out sine pedal is released
        }

        beginTest ("all notes off silence the synth");
        {
            fresh();
            testParams.numVoices = Synth::MAX_VOICES;
            synth.setParam (testParams);
            synth.midiMessage (0x90, 60, 80);
            synth.midiMessage (0x90, 64, 80);
            synth.render (outputs, n);

            synth.midiMessage (0xB0, 120, 0); // panic - all notes off
            synth.render (outputs, n);
            expect (isSilent (left, n));
        }

        beginTest ("mono legato falls back to the previously held note");
        {
            fresh();
            synth.midiMessage (0x90, 60, 80); // 1st note
            synth.render (outputs, n);

            synth.midiMessage (0x90, 64, 80); // 2nd note - legato
            synth.render (outputs, n);

            synth.midiMessage (0x80, 64, 0); // release second note
            synth.render (outputs, n);

            expect (!isSilent (left, n)); // first note rings
        }

        beginTest ("resoCC selects which controller drives resonance");
        {
            fresh();
            synth.resoCC = 0x2A;
            synth.midiMessage (0xB0, 0x2A, 64);
            const float learned = synth.midi.resonanceCtl;
            expect (learned > 1.0f);

            // The old hardcoded CC no longer affects resonance.
            synth.midi.resonanceCtl = 1.0f;
            synth.midiMessage (0xB0, 0x47, 64);
            expectWithinAbsoluteError (synth.midi.resonanceCtl, 1.0f, 1.0e-6f);
        }
    }
};

static SynthTests synthTests;