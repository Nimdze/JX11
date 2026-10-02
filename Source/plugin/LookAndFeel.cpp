// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

#include "plugin/LookAndFeel.h"

#include <cmath>

//==============================================================================
JX11LookAndFeel::JX11LookAndFeel()
{
    setColour (juce::ResizableWindow::backgroundColourId, juce::Colour (30, 60, 90));

    setColour (juce::Slider::rotarySliderOutlineColourId, juce::Colours::black);
    setColour (juce::Slider::rotarySliderFillColourId, juce::Colour (90, 180, 240));
    setColour (juce::Slider::thumbColourId, juce::Colours::white);

    // Buttons are also styled here because the MIDI Learn button uses them.
    setColour (juce::TextButton::buttonColourId, juce::Colour (15, 30, 45));
    setColour (juce::TextButton::buttonOnColourId, juce::Colour (90, 180, 240));
    setColour (juce::TextButton::textColourOffId, juce::Colour (180, 180, 180));
    setColour (juce::TextButton::textColourOnId, juce::Colours::white);

    // ComboBoxes are used for the choice parameters; JUCE draws their outline
    // with this colour id.
    setColour (juce::ComboBox::outlineColourId, juce::Colour (180, 180, 180));
}

void JX11LookAndFeel::drawRotarySlider (juce::Graphics& g, int x, int y, int width, int height,
                                        float sliderPos, float rotaryStartAngle, float rotaryEndAngle,
                                        juce::Slider& slider)
{
    // Fit the largest circle that fits in the given bounds, centred.
    const float size = static_cast<float> (juce::jmin (width, height));
    const auto bounds = juce::Rectangle<float> (size, size)
                            .withCentre ({ static_cast<float> (x) + static_cast<float> (width) * 0.5f,
                                           static_cast<float> (y) + static_cast<float> (height) * 0.5f });

    const float lineW = 6.0f;
    const float arcRadius = bounds.getWidth() * 0.5f - lineW * 0.5f;
    const float toAngle = rotaryStartAngle + sliderPos * (rotaryEndAngle - rotaryStartAngle);
    const auto centre = bounds.getCentre();

    const juce::PathStrokeType stroke (lineW, juce::PathStrokeType::curved, juce::PathStrokeType::butt);

    juce::Path backgroundArc;
    backgroundArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                 rotaryStartAngle, rotaryEndAngle, true);
    g.setColour (slider.findColour (juce::Slider::rotarySliderOutlineColourId));
    g.strokePath (backgroundArc, stroke);

    if (slider.isEnabled())
    {
        juce::Path valueArc;
        valueArc.addCentredArc (centre.x, centre.y, arcRadius, arcRadius, 0.0f,
                                rotaryStartAngle, toAngle, true);
        g.setColour (slider.findColour (juce::Slider::rotarySliderFillColourId));
        g.strokePath (valueArc, stroke);
    }

    const float dialRadius = arcRadius - lineW;
    const float angle = toAngle - juce::MathConstants<float>::halfPi;
    const juce::Point<float> thumb (centre.x + dialRadius * std::cos (angle),
                                    centre.y + dialRadius * std::sin (angle));

    g.setColour (slider.findColour (juce::Slider::thumbColourId));
    g.drawLine (centre.x, centre.y, thumb.x, thumb.y, 3.0f);
    g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre (thumb));
    g.fillEllipse (juce::Rectangle<float> (6.0f, 6.0f).withCentre (centre));
}
