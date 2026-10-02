// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

/*
  ==============================================================================

    JX11's visual style: dark panel, light-blue rotary arcs and a slim dial.

    Applied to the editor with Component::setLookAndFeel rather than
    LookAndFeel::setDefaultLookAndFeel, so the style is scoped to this editor
    instance and does not leak into the host or other plug-in instances.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>

//==============================================================================
class JX11LookAndFeel : public juce::LookAndFeel_V4
{
public:
    JX11LookAndFeel();

    void drawRotarySlider (juce::Graphics&, int x, int y, int width, int height, float sliderPos,
                           float rotaryStartAngle, float rotaryEndAngle, juce::Slider&) override;

private:
    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JX11LookAndFeel)
};
