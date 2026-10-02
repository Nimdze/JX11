#include <juce_core/juce_core.h>
#include "plugin/PluginProcessor.h"

class ProcessorTests : public juce::UnitTest
{
public:
    ProcessorTests()
        : juce::UnitTest ("Processor", "JX11")
    {
    }

    void runTest() override
    {
        JX11AudioProcessor processor;

        beginTest ("constructs and has a program bank");
        expect (processor.getNumPrograms() > 0);

        beginTest ("APVTS exposes every spec with the right type, range and default");
        {
            for (int i = 0; i < Params::NumParams; ++i)
            {
                const auto& s = Params::kSpecs[i];
                const juce::String id (s.id);

                // returns a RangedAudioParameter*, if nullptr then the layout never created this param (table and
                // layout disagree)
                auto* p = processor.apvts.getParameter (s.id);

                expect (p != nullptr, id + " is missing from the APVTS");

                // if theres a disagreement error, continues to check the rest of the params
                if (p == nullptr)
                    continue;

                if (s.choices != nullptr)
                {
                    // Checking whether the layout and table agree on choice vs float param type in the case of choice
                    auto* choice = dynamic_cast<juce::AudioParameterChoice*> (p);
                    expect (choice != nullptr, id + " should be a choice parameter");

                    // Checking agreement for number of choices and choice strings
                    if (choice != nullptr)
                    {
                        expectEquals (choice->choices.size(), s.numChoices, id + " number of choices");

                        for (int k = 0; k < s.numChoices && k < choice->choices.size(); ++k)
                            expectEquals (choice->choices[k], juce::String (s.choices[k]),
                                          id + " choice " + juce::String (k));
                    }
                }

                else
                {
                    // Checking whether the layout and table agree on choice vs float param type in the case of float
                    auto* f = dynamic_cast<juce::AudioParameterFloat*> (p);
                    expect (f != nullptr, id + " should be a float parameter");

                    // Checking whether table and layout agree on the param range
                    if (f != nullptr)
                    {
                        const auto range = f->getNormalisableRange();
                        expectEquals (range.start, s.min, id + " min");
                        expectEquals (range.end, s.max, id + " max");
                    }
                }

                // Design default independent of current value (default value is the one in the table, getValue is the
                // current value, setCurrrentProgram(0) inits it) convertFrom0to1 to convert normalized juce values to
                // the format the table uses.
                expectWithinAbsoluteError (p->convertFrom0to1 (p->getDefaultValue()), s.defaultValue, 0.01f,
                                           id + " default");
            }
        }

        beginTest ("constructor leaves program 0 (Init) loaded");
        {
            // Program name check
            const auto presets = createFactoryPresets();
            expectEquals (presets[0].name, juce::String ("Init"));
            expectEquals (processor.getCurrentProgram(), 0);

            // Parameter values check
            for (int j = 0; j < Params::NumParams; ++j)
            {
                auto* p = processor.apvts.getParameter (Params::kSpecs[j].id);
                expect (p != nullptr);
                if (p == nullptr)
                    continue;

                expectWithinAbsoluteError (p->convertFrom0to1 (p->getValue()), presets[0].param[j], 1.0e-3f,
                                           juce::String (Params::kSpecs[j].id) + " for Init");
            }
        }

        beginTest ("setCurrentProgram maps preset values in parameter order");
        {
            const auto presets = createFactoryPresets();
            expect (presets.size() > 5);
            if (presets.size() <= 5)
                return;

            const int index = 5; // pick non 0 preset
            processor.setCurrentProgram (index);

            expectEquals (processor.getCurrentProgram(), index);

            const auto& preset = presets[static_cast<std::size_t> (index)];

            for (int j = 0; j < Params::NumParams; ++j)
            {
                auto* p = processor.apvts.getParameter (Params::kSpecs[j].id);
                expect (p != nullptr);
                if (p == nullptr)
                    continue;

                // getValue() is normalised; convertFrom0to1 gives plain value
                const float actual = p->convertFrom0to1 (p->getValue());

                expectWithinAbsoluteError (actual, preset.param[j], 1.0e-3f,
                                           juce::String (Params::kSpecs[j].id) + " from preset " + preset.name);
            }
        }

        beginTest ("state round trips through get/setStateInformation");
        {
            JX11AudioProcessor a;

            // Set parameters to values different from Init values
            auto* aFreq = a.apvts.getParameter (Params::kSpecs[Params::filterFreq].id);
            auto* aPoly = a.apvts.getParameter (Params::kSpecs[Params::polyMode].id);
            aFreq->setValueNotifyingHost (0.25f); // normalised
            aPoly->setValueNotifyingHost (0.0f);

            juce::MemoryBlock block;
            a.getStateInformation (block);
            expect (block.getSize() > 0);

            JX11AudioProcessor b;
            b.setStateInformation (block.getData(), static_cast<int> (block.getSize()));

            for (int i = 0; i < Params::NumParams; ++i)
            {
                auto* pa = a.apvts.getParameter (Params::kSpecs[i].id);
                auto* pb = b.apvts.getParameter (Params::kSpecs[i].id);
                expect (pa != nullptr && pb != nullptr);
                if (pa == nullptr || pb == nullptr)
                    continue;

                expectWithinAbsoluteError (pb->getValue(), pa->getValue(), 1.0e-3f,
                                           juce::String (Params::kSpecs[i].id) + " after restore");
            }
        }

        beginTest ("setStateInformation ignores garbage without changing state");
        {
            JX11AudioProcessor b;
            const float before = b.apvts.getParameter (Params::kSpecs[Params::filterFreq].id)->getValue();

            const char junk[] = "this is not xml";
            b.setStateInformation (junk, static_cast<int> (sizeof (junk)));

            expectWithinAbsoluteError (b.apvts.getParameter (Params::kSpecs[Params::filterFreq].id)->getValue(), before,
                                       1.0e-4f);
        }
    }
};

static ProcessorTests processorTests;