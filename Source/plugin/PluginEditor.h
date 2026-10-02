/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin editor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <memory>
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

        // Read-only value box, and don't let tab/click move keyboard focus onto
        // the knob, so it cannot start editing a value unexpectedly.
        slider.setTextBoxStyle (juce::Slider::TextBoxBelow, true, 100, 20);
        slider.setWantsKeyboardFocus (false);
        slider.setMouseClickGrabsKeyboardFocus (false);

        slider.setRotaryParameters (juce::degreesToRadians (225.0f), juce::degreesToRadians (495.0f), true);
        slider.setTitle (getDisplayName());
        addAndMakeVisible (slider);
    }

    bool isValueEditable() const noexcept { return slider.isTextBoxEditable(); }
    bool wantsKeyboardFocus() const noexcept { return slider.getWantsKeyboardFocus(); }

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
    {
        name.setText (getDisplayName(), juce::dontSendNotification);
        name.setJustificationType (juce::Justification::centred);
        name.setInterceptsMouseClicks (false, false);
        addAndMakeVisible (name);

        const auto& spec = Params::kSpecs[index];

        for (int i = 0; i < spec.numChoices; ++i)
        {
            auto* button = buttons.add (new juce::TextButton (spec.choices[i]));
            button->setClickingTogglesState (false);
            button->setWantsKeyboardFocus (false);
            button->setMouseClickGrabsKeyboardFocus (false);
            button->onClick = [this, i] { selectChoice (i); };
            addAndMakeVisible (button);
        }

        if (auto* parameter = state.getParameter (Params::kIds[index]))
        {
            attachment = std::make_unique<juce::ParameterAttachment> (*parameter,
                                                                      [this] (float value) { updateButtons (value); });
            attachment->sendInitialUpdate();
        }
    }

    void resized() override
    {
        auto area = getLocalBounds();
        name.setBounds (area.removeFromTop (18));

        area = area.reduced (2);
        if (buttons.isEmpty())
            return;

        // Stack the options vertically and centre the column, so labels like
        // "Legato"/"Always" get the full cell width instead of clipping.
        constexpr int buttonHeight = 22;
        const int totalHeight = buttons.size() * buttonHeight;
        area = area.withSizeKeepingCentre (area.getWidth(), juce::jmin (area.getHeight(), totalHeight));

        for (auto* button : buttons)
            button->setBounds (area.removeFromTop (buttonHeight).reduced (1));
    }

    // Exposed for the regression test that every option becomes a button.
    int getNumChoices() const noexcept { return buttons.size(); }

private:
    void selectChoice (int index)
    {
        updateButtons (static_cast<float> (index)); // immediate visual feedback

        if (attachment != nullptr)
            attachment->setValueAsCompleteGesture (static_cast<float> (index));
    }

    void updateButtons (float value)
    {
        const int index = juce::roundToInt (value);

        for (int i = 0; i < buttons.size(); ++i)
            buttons[i]->setToggleState (i == index, juce::dontSendNotification);
    }

    juce::Label name;
    juce::OwnedArray<juce::TextButton> buttons;
    // Declared after `buttons` so it is destroyed first (JUCE attachment rule).
    std::unique_ptr<juce::ParameterAttachment> attachment;
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
