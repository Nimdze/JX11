#include <juce_core/juce_core.h>
#include "plugin/PluginProcessor.h"
#include "plugin/PluginEditor.h"

// Regression test: JUCE's ComboBoxAttachment syncs the selection but does not
// populate the list, so ParamChoice must add the parameter's choices itself.
class ChoiceControlTests : public juce::UnitTest
{
public:
    ChoiceControlTests()
        : juce::UnitTest ("ChoiceControls", "JX11")
    {
    }

    void runTest() override
    {
        beginTest ("choice parameters populate their ComboBox");
        {
            JX11AudioProcessor processor;

            for (int i = 0; i < Params::NumParams; ++i)
            {
                const auto& spec = Params::kSpecs[i];

                if (spec.choices == nullptr)
                    continue;

                JX11Ui::ParamChoice control (i, processor.apvts);

                expectEquals (control.getNumChoices(), spec.numChoices, juce::String (Params::kIds[i]));
            }
        }
    }
};

static ChoiceControlTests choiceControlTests;
