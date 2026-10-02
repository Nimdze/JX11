#include <juce_core/juce_core.h>
#include "plugin/PluginProcessor.h"
#include "plugin/PluginEditor.h"

// Regression test: the knobs must not take keyboard focus or offer an editable
// value box, otherwise pressing Tab (or clicking) puts them into text-entry
// mode and starts typing values.
class SliderControlTests : public juce::UnitTest
{
public:
    SliderControlTests()
        : juce::UnitTest ("SliderControls", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("knob values are read-only and do not take keyboard focus");
        {
            JX11AudioProcessor processor;

            for (int i = 0; i < Params::NumParams; ++i)
            {
                if (Params::kSpecs[i].choices != nullptr)
                    continue;

                JX11Ui::ParamSlider control (i, processor.apvts);

                expect (!control.isValueEditable(), juce::String (Params::kIds[i]));
                expect (!control.wantsKeyboardFocus(), juce::String (Params::kIds[i]));
            }
        }
    }
};

static SliderControlTests sliderControlTests;
