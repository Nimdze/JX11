#pragma once

#include <cmath>
#include "Constants.h"

class Oscillator
{
    public:
        float amplitude;
        float period = 0.0f;

        void reset()
        {
            inc = 0.0f;
            phase = 0.0f;

            sin0 = 0.0f;
            sin1 = 0.0f;
            dsin = 0.0f;

            dc = 0.0f;
        }

        float nextSample()
        {
            float output = 0.0f;

            phase += inc;

            if (phase <= (PI/4.0f) ) 
            {
                float halfPeriod = period / 2.0f;
                phaseMax = std::floor(0.5f + halfPeriod) -0.5f;
                dc = 0.5f * amplitude / phaseMax;
                phaseMax *= PI;

                inc = phaseMax / halfPeriod;
                phase = -phase;

                sin0 = amplitude * std::sin(phase);
                sin1 = amplitude * std::sin(phase - inc);
                dsin = 2.0f * std::cos(inc);

                if (phase*phase > 1e-9) 
                    output = sin0 / phase;
                else
                    output = amplitude;
            }

            else 
            {
                if (phase > phaseMax)
                {
                    phase = phaseMax + phaseMax - phase;
                    inc = -inc;
                }

                float sinp = dsin * sin0 - sin1;
                sin1 = sin0; 
                sin0 = sinp;

                output = sinp / phase;
            }

            return output - dc;
        }

    private: 
        float phase;
        float phaseMax;
        float inc;

        float sin0;
        float sin1;
        float dsin;

        float dc;

    /*later replace with (and only then write tests) -  The most popular methods are based on BLEP, which stands for BandLimited stEP
function. This is an enhancement of the BLIT method you’ve used in this chapter.
BLEP already does the integration ahead of time. There are a few different variations
of BLEP. If you’re up for some homework, google “MinBLEP” or “PolyBLEP” and see if
you can implement these algorithms in the Oscillator class*/
};