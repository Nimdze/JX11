#include <juce_core/juce_core.h>
#include "Utils.h"
#include <limits>

class UtilsTests : public juce::UnitTest
{
    public:
        UtilsTests() : juce::UnitTest ("Utils", "JX11") {}

        void runTest() override
        {

            beginTest ("nullptr does not crash");
            protectYourEars (nullptr, 16);

            beginTest ("normal samples pass through unchanges");
            {
                float b[] = { 0.5f, -0.25f, 0.0f, 0.75f };
                protectYourEars (b, 4);
                expectEquals (b[0], 0.5f);
                expectEquals (b[1], -0.25f);
                expectEquals (b[2], 0.0f);
                expectEquals (b[3], 0.75f);
            }

            beginTest ("NaN silences the whole buffer");
            {
                float b[] = { 0.5f, std::numeric_limits<float>::quiet_NaN(), 0.5f};
                protectYourEars (b, 3);
                for (auto v : b) expectEquals (v, 0.0f);
            }

            beginTest ("inf silences the whole buffer");
            {
                float b[] = { 0.5f, std::numeric_limits<float>::infinity(), 0.5f};
                protectYourEars (b, 3);
                for (auto v : b) expectEquals (v, 0.0f);
            }

            beginTest ("out-of-range silences the whole buffer");
            {
                float b[] = { 0.5f, 3.0f, 0.5f};
                protectYourEars (b, 3);
                for (auto v : b) expectEquals (v, 0.0f);
            }

            beginTest ("moderate overload is clamped, not silenced");
            {
                float b[] = { 1.5f, -1.5f};
                protectYourEars (b, 2);
                expectEquals (b[0], 1.0f);
                expectEquals (b[1], -1.0f);
            }
        }
};

static UtilsTests utilsTests;
