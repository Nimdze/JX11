/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include "plugin/PluginProcessor.h"
#include "plugin/LookAndFeel.h"
#include "model/ParameterList.h"
#include "model/Parameters.h"

//==============================================================================
namespace JX11Ui
{

// One parameter's widget plus its APVTS attachment. The widget is declared
// before the attachment so the attachment is destroyed first, which is the
// lifetime rule juce attachments require. Keeping that rule inside the control
// means it is stated once instead of once per parameter.
class ParamControl : public juce::Component
{
public:
    explicit ParamControl (int index)
        : paramIndex (index)
    {
        setTitle (Params::kSpecs[index].name);
        setExplicitFocusOrder (index + 1);
    }

    int getParamIndex() const noexcept { return paramIndex; }
    juce::String getDisplayName() const { return Params::kSpecs[paramIndex].name; }

private:
    int paramIndex;
};

class ParamSlider : public ParamControl
{
public:
    ParamSlider (int index, juce::AudioProcessorValueTreeState& state)
        : ParamControl (index)
        , attachment (state, Params::kIds[index], slider)
    {
        name.setText (getDisplayName(), juce::dontSendNotification);
        name.setJustificationType (juce::Justification::centred);
        name.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (name);

        slider.setSliderStyle (juce::Slider::RotaryHorizontalVerticalDrag);
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, false, 100, 20);
        slider.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
        slider.setTitle (getDisplayName());
        addAndMakeVisible (slider);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        name.setBounds (area.removeFromTop (18));
        slider.setBounds (area.reduced (2));
    }

private:
    juce::Label name;
    juce::Slider slider;
    juce::AudioProcessorValueTreeState::SliderAttachment attachment;
};

class ParamChoice : public ParamControl
{
public:
    ParamChoice (int index, juce::AudioProcessorValueTreeState& state)
        : ParamControl (index)
        , attachment (state, Params::kIds[index], combo)
    {
        name.setText (getDisplayName(), juce::dontSendNotification);
        name.setJustificationType (juce::Justification::centred);
        name.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (name);

        combo.setTitle (getDisplayName());
        addAndMakeVisible (combo);
    }

    void resized() override
    {
        auto area = getLocalBounds();
        name.setBounds (area.removeFromTop (18));
        combo.setBounds (area.withSizeKeepingCentre (area.getWidth() - 8, 24));
    }

private:
    juce::Label name;
    juce::ComboBox combo;
    juce::AudioProcessorValueTreeState::ComboBoxAttachment attachment;
};

} // namespace JX11Ui

//==============================================================================
class JX11AudioProcessorEditor : public juce::AudioProcessorEditor, private juce::Button::Listener, private juce::Timer
{
public:
    JX11AudioProcessorEditor (JX11AudioProcessor&);
    ~JX11AudioProcessorEditor() override;

    //==============================================================================
    void paint (juce::Graphics&) override;
    void resized() override;

private:
    void buttonClicked (juce::Button*) override;
    void timerCallback() override;

    // This reference is provided as a quick way for your editor to
    // access the processor object that created it.
    JX11AudioProcessor& audioProcessor;

    // Declared before the components so it outlives them (members are destroyed
    // in reverse order). Scoped to this editor, not installed globally.
    JX11LookAndFeel lookAndFeel;

    // Built from Params::kSpecs in the constructor: one control per parameter,
    // so adding a parameter never touches this class. Owns every control, so
    // teardown (and therefore attachment teardown) is automatic.
    juce::OwnedArray<JX11Ui::ParamControl> controls;

    // Not a parameter: triggers MIDI Learn on the processor and polls for it
    // finishing. See the timerCallback / buttonClicked implementations.
    juce::TextButton midiLearnButton;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JX11AudioProcessorEditor)
};
