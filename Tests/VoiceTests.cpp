#include <juce_core/juce_core.h>
#include "dsp/Voice.h"

class VoiceTests : public juce::UnitTest
{
public:
    VoiceTests()
        : juce::UnitTest ("Voice", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("reset() clears note and velocity");
        {
            Voice v;
            v.note = 60;

            v.reset();

            expectEquals (v.note, -1);
        }

        beginTest ("panning centers at middle C and hard-pans two octaves out");
        {
            Voice v;
            v.reset();

            v.setPanPosition (60);
            v.updatePanning (true);
            expectWithinAbsoluteError (v.panLeft, 0.7071f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 0.7071f, 1.0e-3f);

            v.setPanPosition (84);
            v.updatePanning (true); // +24 semi -> hard right
            expectWithinAbsoluteError (v.panLeft, 0.0f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 1.0f, 1.0e-3f);

            v.setPanPosition (36);
            v.updatePanning (true); // -24 semi -> hard left
            expectWithinAbsoluteError (v.panLeft, 1.0f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 0.0f, 1.0e-3f);
        }

        beginTest ("panning is constant power and clamps out of range");
        {
            Voice v;
            v.reset();

            v.setPanPosition (72);
            v.updatePanning (true);
            expectWithinAbsoluteError (v.panLeft * v.panLeft + v.panRight * v.panRight, 1.0f, 1.0e-3f);

            v.setPanPosition (127);
            v.updatePanning (true); // clamps to hard right
            expectWithinAbsoluteError (v.panLeft, 0.0f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 1.0f, 1.0e-3f);

            v.setPanPosition (0);
            v.updatePanning (true); // clamps to hard left
            expectWithinAbsoluteError (v.panLeft, 1.0f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 0.0f, 1.0e-3f);
        }

        beginTest ("disabling panning centers every voice");
        {
            Voice v;
            v.reset();

            v.setPanPosition (84);
            v.updatePanning (false);
            expectWithinAbsoluteError (v.panLeft, 0.7071f, 1.0e-3f);
            expectWithinAbsoluteError (v.panRight, 0.7071f, 1.0e-3f);
        }
    }
};

static VoiceTests voiceTests;