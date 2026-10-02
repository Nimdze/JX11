#pragma once
#include <algorithm>
#include <cmath>
#include "common/Constants.h"

class Filter
{
public:
    float sampleRate = 44100.0f;

    // Keep the bilinear transform inside its valid range: at or above Nyquist
    // tan() goes negative and the coefficients become invalid.
    [[nodiscard]] static float clampCutoff (float cutoff, float sampleRate) noexcept
    {
        return std::clamp (cutoff, kMinFilterCutoff, kMaxCutoffRatio * sampleRate);
    }

    void updateCoefficients (float cutoff, float Q)
    {
        cutoff = clampCutoff (cutoff, sampleRate);

        g = std::tan (PI * cutoff / sampleRate);
        k = 1.0f / Q;
        a1 = 1.0f / (1.0f + g * (g + k));
        a2 = g * a1;
        a3 = g * a2;
    }

    void reset()
    {
        g = 0.0f;
        k = 0.0f;
        a1 = 0.0f;
        a2 = 0.0f;
        a3 = 0.0f;

        ic1eq = 0.0f;
        ic2eq = 0.0f;
    }

    float render (float x)
    {
        float v3 = x - ic2eq;
        float v1 = a1 * ic1eq + a2 * v3;
        float v2 = ic2eq + a2 * ic1eq + a3 * v3;
        ic1eq = 2.0f * v1 - ic1eq;
        ic2eq = 2.0f * v2 - ic2eq;
        return v2;
    }

private:
    float g = 0.0f, k = 0.0f, a1 = 0.0f, a2 = 0.0f, a3 = 0.0f; // filter coefficients
    float ic1eq = 0.0f, ic2eq = 0.0f;                          // internal state
};