// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

#include <cmath>

// Period in samples for a MIDI note, including a small per-voice analog-style
// detune and octave folding so the period never falls below 6 samples.
// Pure function, called at note rate only.
inline float periodForNote (float tune, float detune, int voice, int note) noexcept
{
    constexpr float ANALOG = 0.002f;

    // freq = 440 * 2^((note - 69)/12) = 440 * 2^(-69/12) * 2^(note/12)
    // per = (sampleRate/(440 * 2^(-69/12))) * 2^(-note/12)
    // define tune = (sampleRate/(440 * 2^(-69/12)))
    // 2^(-note/12) = (2^(-1/12))^note = exp^(M * note)
    float period = tune * std::exp (-0.05776226505f * (float (note) + ANALOG * float (voice)));

    while (period < 6.0f || (period * detune) < 6.0f)
    {
        if (period <= 0.0f)
            period = 6.0f;

        period += period;
    }

    return period;
}
