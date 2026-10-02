/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#include "plugin/PluginProcessor.h"
#include "plugin/PluginEditor.h"

namespace
{
constexpr int kColumns = 7;
constexpr int kUtilityBarHeight = 36;
} // namespace

//==============================================================================
JX11AudioProcessorEditor::JX11AudioProcessorEditor (JX11AudioProcessor& p)
    : AudioProcessorEditor (&p)
    , audioProcessor (p)
{
    setLookAndFeel (&lookAndFeel);

    for (int i = 0; i < Params::NumParams; ++i)
    {
        // Choice parameters get a combo box, everything else a rotary knob.
        // Ranges, choices and value display all come from the attachment, so
        // there is nothing per-parameter to configure here.
        JX11Ui::ParamControl* control = nullptr;

        if (Params::kSpecs[i].choices != nullptr)
            control = new JX11Ui::ParamChoice (i, audioProcessor.apvts);
        else
            control = new JX11Ui::ParamSlider (i, audioProcessor.apvts);

        controls.add (control);
        addAndMakeVisible (*control);
    }

    midiLearnButton.setButtonText ("MIDI Learn");
    midiLearnButton.addListener (this);
    addAndMakeVisible (midiLearnButton);

    setSize (660, 520);
}

JX11AudioProcessorEditor::~JX11AudioProcessorEditor()
{
    // Cancel any in-flight MIDI Learn and detach from the processor before the
    // UI members go away.
    audioProcessor.setMidiLearn (false);
    stopTimer();
    midiLearnButton.removeListener (this);
    setLookAndFeel (nullptr);
}

//==============================================================================
void JX11AudioProcessorEditor::paint (juce::Graphics& g)
{
    // (Our component is opaque, so we must completely fill the background with a solid colour)
    g.fillAll (getLookAndFeel().findColour (juce::ResizableWindow::backgroundColourId));
}

void JX11AudioProcessorEditor::resized()
{
    auto area = getLocalBounds().reduced (10);

    // Non-parameter controls live in a bar along the bottom so the generated
    // parameter grid keeps the rest of the space.
    auto utility = area.removeFromBottom (kUtilityBarHeight);
    midiLearnButton.setBounds (utility.removeFromLeft (120).reduced (2));

    const int columns = kColumns;
    const int rows = juce::jmax (1, (controls.size() + columns - 1) / columns);
    const int cellW = area.getWidth() / columns;
    const int cellH = area.getHeight() / rows;

    int col = 0;
    int row = 0;

    for (auto* control : controls)
    {
        control->setBounds (area.getX() + col * cellW, area.getY() + row * cellH, cellW, cellH);

        if (++col == columns)
        {
            col = 0;
            ++row;
        }
    }
}

//==============================================================================
void JX11AudioProcessorEditor::buttonClicked (juce::Button* button)
{
    if (button != &midiLearnButton)
        return;

    midiLearnButton.setButtonText ("Waiting...");
    midiLearnButton.setEnabled (false);

    audioProcessor.setMidiLearn (true);

    // Poll the processor until the audio thread captures a CC and clears the
    // flag. Polling an atomic is the safe way to signal back to the UI thread.
    startTimerHz (10);
}

void JX11AudioProcessorEditor::timerCallback()
{
    if (!audioProcessor.isMidiLearnActive())
    {
        stopTimer();
        midiLearnButton.setButtonText ("MIDI Learn");
        midiLearnButton.setEnabled (true);
    }
}
