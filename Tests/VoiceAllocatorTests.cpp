// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include <juce_core/juce_core.h>
#include <algorithm>
#include <iterator>
#include "dsp/VoiceAllocator.h"
#include "common/Modulation.h"

namespace
{
const float kSampleRate = 44100.0f;
const float kTune = kSampleRate * std::exp (kSemitoneLog * -36.3763f);

// Allocator under test plus its voice storage, reset to a clean state.
struct Fixture
{
    explicit Fixture (int numVoices)
    {
        params.numVoices = numVoices;
        params.tune = kTune;
        params.detune = 1.0f;

        for (auto& v : voices)
            v.reset();

        allocator.prepare (voices, &params, kSampleRate);
        allocator.reset();
    }

    int activeVoices() const
    {
        return static_cast<int> (
            std::count_if (std::begin (voices), std::end (voices), [] (const Voice& v) { return v.note > 0; }));
    }

    SynthParams params;
    Voice voices[SynthLimits::MAX_VOICES];
    VoiceAllocator allocator;
};
} // namespace

class VoiceAllocatorTests : public juce::UnitTest
{
public:
    VoiceAllocatorTests()
        : juce::UnitTest ("VoiceAllocator", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("poly note-on assigns successive free voices");
        {
            Fixture f (SynthLimits::MAX_VOICES);

            f.allocator.noteOn (60, 80);
            expectEquals (f.voices[0].note, 60);
            expect (f.voices[0].env.isActive());
            expectEquals (f.activeVoices(), 1);

            f.allocator.noteOn (62, 80);
            expectEquals (f.voices[1].note, 62);
            expectEquals (f.activeVoices(), 2);
        }

        beginTest ("poly note-off releases only the matching voice");
        {
            Fixture f (SynthLimits::MAX_VOICES);
            f.allocator.noteOn (60, 80);
            f.allocator.noteOn (64, 80);

            f.allocator.noteOff (60, false);

            expectEquals (f.voices[0].note, 0);  // released
            expectEquals (f.voices[1].note, 64); // still held
            expectEquals (f.activeVoices(), 1);
        }

        beginTest ("mono legato reuses one voice and queues the old note");
        {
            Fixture f (1);

            f.allocator.noteOn (60, 80);
            expectEquals (f.voices[0].note, 60);

            f.allocator.noteOn (64, 80);
            expectEquals (f.voices[0].note, 64); // playing voice moves
            expectEquals (f.voices[1].note, 60); // previous note queued

            f.allocator.noteOff (64, false);
            expectEquals (f.voices[0].note, 60); // falls back to queued note
        }

        beginTest ("mono legato queue handles three overlapping notes");
        {
            Fixture f (1);

            f.allocator.noteOn (60, 80);
            f.allocator.noteOn (64, 80);
            f.allocator.noteOn (67, 80);

            expectEquals (f.voices[0].note, 67);
            expectEquals (f.voices[1].note, 64); // queue is most-recent-first
            expectEquals (f.voices[2].note, 60);

            f.allocator.noteOff (67, false);
            expectEquals (f.voices[0].note, 64); // most recent queued note first
        }

        beginTest ("sustain pedal parks the note on the SUSTAIN sentinel");
        {
            Fixture f (SynthLimits::MAX_VOICES);

            f.allocator.noteOn (60, 80);
            f.allocator.noteOff (60, true); // key up, pedal down
            expectEquals (f.voices[0].note, VoiceAllocator::SUSTAIN);
            expectEquals (f.activeVoices(), 0); // sentinel is not a held note

            // Lifting the pedal releases everything on the sentinel.
            f.allocator.noteOff (VoiceAllocator::SUSTAIN, false);
            expectEquals (f.voices[0].note, 0);
        }

        beginTest ("allNotesOff clears every voice");
        {
            Fixture f (SynthLimits::MAX_VOICES);
            f.allocator.noteOn (60, 80);
            f.allocator.noteOn (64, 80);

            f.allocator.allNotesOff();

            expectEquals (f.activeVoices(), 0);
            for (const auto& v : f.voices)
                expectEquals (v.note, -1);
        }

        beginTest ("voice stealing keeps the polyphony bounded");
        {
            Fixture f (SynthLimits::MAX_VOICES);

            for (int i = 0; i < SynthLimits::MAX_VOICES; ++i)
                f.allocator.noteOn (60 + i, 80);

            expectEquals (f.activeVoices(), SynthLimits::MAX_VOICES);

            f.allocator.noteOn (72, 80);

            // The new note must be present, and no extra voice may be active.
            bool found = false;
            for (const auto& v : f.voices)
                found = found || (v.note == 72);

            expect (found, "stolen note is not sounding");
            expectEquals (f.activeVoices(), SynthLimits::MAX_VOICES);
        }

        beginTest ("ignoreVelocity forces a fixed velocity");
        {
            Fixture f (SynthLimits::MAX_VOICES);
            f.params.ignoreVelocity = true;
            f.params.volumeTrim = 0.5f;

            f.allocator.noteOn (60, 1);

            const float expected = 0.5f * velocityCurve (80);
            expectWithinAbsoluteError (f.voices[0].osc1.amplitude, expected, 1.0e-6f);
        }
    }
};

static VoiceAllocatorTests voiceAllocatorTests;
