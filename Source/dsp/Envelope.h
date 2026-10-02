// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#pragma once

const float SILENCE = 0.0001f;

class Envelope
{
public:
    float attackMultiplier;
    float decayMultiplier;
    float sustainLevel;
    float releaseMultiplier;

    float level;

    float nextValue()
    {
        level = multiplier * (level - target) + target; // one pole filter difference equation

        if (level + target > 3.0f)
        {
            multiplier = decayMultiplier;
            target = sustainLevel;
        }

        return level;
    }

    inline bool isInAttack() const { return target >= 2.0f; }

    void attack()
    {
        level += SILENCE + SILENCE;
        target = 2.0f;
        multiplier = attackMultiplier;
    }

    void release()
    {
        target = 0.0f;
        multiplier = releaseMultiplier;
    }

    inline bool isActive() const { return level > SILENCE; }

    void reset()
    {
        level = 0.0f;
        target = 0.0f;
        multiplier = 0.0f;
    }

private:
    float multiplier;
    float target;
};