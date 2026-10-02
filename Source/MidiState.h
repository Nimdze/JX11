// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

// Runtime MIDI controller state, mutated on the audio thread by incoming MIDI.
// This is not configuration (that lives in SynthParams); it is the live state
// that note handling and modulation read.
struct MidiState
{
    float pitchBend           = 1.0f;
    float modWheel            = 0.0f;
    float resonanceCtl        = 1.0f;
    float pressure            = 0.0f;
    float filterCtl           = 0.0f;
    bool  sustainPedalPressed = false;

    void reset() noexcept { *this = MidiState{}; }
};
