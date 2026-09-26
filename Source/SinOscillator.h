#pragma once

#include <cmath>
#include "Constants.h"

class SinOscillator
{
public:
    float amplitude;
    float inc;
    float phase;

    void reset()
    {
        phase = 0.0f;

        sin0 = amplitude * std::sin(phase * 2 * PI);
        sin1 = amplitude * std::sin((phase - inc) * 2 * PI);
        dsin = 2.0f * std::cos(inc * 2 * PI);
    }

    float nextSample()
    {
        float sinx = dsin * sin0 - sin1;
        sin1 = sin0;
        sin0 = sinx;
        return sinx;
    }

    private: 
        float sin0;
        float sin1;
        float dsin;
};

/*
sin(a + b) + sin(a − b) = 2·cos(b)·sin(a)  
sin(ω(n+1)) + sin(ω(n−1)) = 2·cos(ω)·sin(ωn)    
sin(ω(n+1)) = 2·cos(ω)·sin(ωn) − sin(ω(n−1))   
*/