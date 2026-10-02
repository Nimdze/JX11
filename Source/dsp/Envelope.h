#pragma once

inline constexpr float SILENCE = 0.0001f;

class Envelope
{
public:
    float attackMultiplier = 0.0f;
    float decayMultiplier = 0.0f;
    float sustainLevel = 0.0f;
    float releaseMultiplier = 0.0f;

    float level = 0.0f;

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
    float multiplier = 0.0f;
    float target = 0.0f;
};