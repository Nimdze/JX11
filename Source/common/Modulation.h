// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once
#include <cmath>
#include "common/Constants.h"

// Velocity curve - midi mapped from 1-127 to around 8.9-137.9
inline float velocityCurve (int velocity)
{
    return 0.004f * float ((velocity + 64) * (velocity + 64)) - 8.0f;
}

// Vibrato/PWM depth from vibrato parameter
inline float vibratoDepth (float vibratoParam)
{
    const float v = vibratoParam / 200.0f;
    return 0.2f * v * v;
}

// LFO rate in Hz from the LFO rate parameter
inline float lfoRateHz (float param)
{
    return std::exp (7.0f * param - 4.0f);
}

// Glide one-pole coefficient. zero for "no glide" is handled by the caller
inline float glideCoefficient (float glideRate, float inverseUpdateRate)
{
    return 1.0f - std::exp (-inverseUpdateRate * std::exp (6.0f - 0.07f * glideRate));
}

// Mod-wheel depth from the CC value
inline float modWheelDepth (int data2)
{
    return 0.000005f * float (data2 * data2);
}

// Period in samples for a MIDI note, including a small per-voice analog-style
// detune and octave folding so the period never falls below 6 samples.
inline float periodForNote (float tune, float detune, int voice, int note) noexcept
{
    constexpr float ANALOG = 0.002f;

    // freq = 440 * 2^((note - 69)/12) = 440 * 2^(-69/12) * 2^(note/12)
    // per = (sampleRate/(440 * 2^(-69/12))) * 2^(-note/12)
    // define tune = (sampleRate/(440 * 2^(-69/12)))
    // 2^(-note/12) = (2^(-1/12))^note = exp^(M * note)
    float period = tune * std::exp (-kSemitoneLog * (float (note) + ANALOG * float (voice)));

    while (period < kMinPeriodSamples || (period * detune) < kMinPeriodSamples)
    {
        if (period <= 0.0f)
            period = kMinPeriodSamples;

        period += period;
    }

    return period;
}