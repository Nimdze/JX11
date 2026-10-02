#include <juce_core/juce_core.h>
#include <cmath>
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

        beginTest ("MIDI Learn captures the first CC and it survives a state round trip");
        {
            JX11AudioProcessor a;
            a.prepareToPlay (44100.0, 512);

            expectEquals (static_cast<int> (a.getMidiLearnCC()), 0x47, "default learned CC");
            expect (! a.isMidiLearnActive());

            a.setMidiLearn (true);
            expect (a.isMidiLearnActive());

            // A CC on any channel is captured, ignoring the channel.
            {
                juce::AudioBuffer<float> buffer (2, 512);
                buffer.clear();
                juce::MidiBuffer midi;
                midi.addEvent (juce::MidiMessage::controllerEvent (1, 0x2A, 64), 0);
                a.processBlock (buffer, midi);
            }

            expect (! a.isMidiLearnActive(), "learn mode ends after the first CC");
            expectEquals (static_cast<int> (a.getMidiLearnCC()), 0x2A);

            // The learned CC is part of the saved state.
            juce::MemoryBlock block;
            a.getStateInformation (block);

            JX11AudioProcessor b;
            expectEquals (static_cast<int> (b.getMidiLearnCC()), 0x47);
            b.setStateInformation (block.getData(), static_cast<int> (block.getSize()));
            expectEquals (static_cast<int> (b.getMidiLearnCC()), 0x2A, "learned CC restored from state");
        }

        beginTest ("processBlock renders finite audio and releases to silence");
        {
            JX11AudioProcessor proc;
            proc.setRateAndBufferSizeDetails (44100.0, 512);
            proc.prepareToPlay (44100.0, 512);

            const int n = 512;
            juce::AudioBuffer<float> buffer (2, n);

            // Note on: the output must be finite and actually sounding.
            juce::MidiBuffer noteOn;
            noteOn.addEvent (juce::MidiMessage::noteOn (1, 60, static_cast<juce::uint8> (100)), 0);
            buffer.clear();
            proc.processBlock (buffer, noteOn);

            float peak = 0.0f;
            for (int ch = 0; ch < buffer.getNumChannels(); ++ch)
            {
                for (int i = 0; i < n; ++i)
                {
                    const float x = buffer.getSample (ch, i);
                    expect (std::isfinite (x));
                    peak = juce::jmax (peak, std::abs (x));
                }
            }
            expect (peak > 1.0e-4f, "note on produced silence");

            // Note off: the voice must decay back to silence without blowing up.
            float lastPeak = peak;
            for (int block = 0; block < 200 && lastPeak > 1.0e-4f; ++block)
            {
                buffer.clear();
                juce::MidiBuffer events;
                if (block == 0)
                    events.addEvent (juce::MidiMessage::noteOff (1, 60), 0);

                proc.processBlock (buffer, events);

                lastPeak = buffer.getMagnitude (0, n);
                expect (std::isfinite (lastPeak));
                expect (lastPeak < 10.0f);
            }

            expect (lastPeak <= 1.0e-4f, "voice never released to silence");
        }
    }
};

static ProcessorTests processorTests;