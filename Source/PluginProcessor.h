// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>

#include "Synth.h"
#include "Preset.h"
#include "Parameters.h"

//==============================================================================
/**
 */
class JX11AudioProcessor : public juce::AudioProcessor, private juce::ValueTree::Listener, private juce::Timer
{
public:
    //==============================================================================
    JX11AudioProcessor();

    ~JX11AudioProcessor() override;

    //==============================================================================
    void prepareToPlay (double sampleRate, int samplesPerBlock) override;
    void releaseResources() override;
    void reset() override;

    //==============================================================================
#ifndef JucePlugin_PreferredChannelConfigurations
    bool isBusesLayoutSupported (const BusesLayout& layouts) const override;
#endif

    //==============================================================================
    void processBlock (juce::AudioBuffer<float>&, juce::MidiBuffer&) override;

    //==============================================================================
    juce::AudioProcessorEditor* createEditor() override;
    bool hasEditor() const override;

    //==============================================================================
    const juce::String getName() const override;

    bool acceptsMidi() const override;
    bool producesMidi() const override;
    bool isMidiEffect() const override;
    double getTailLengthSeconds() const override;

    //==============================================================================
    int getNumPrograms() override;
    int getCurrentProgram() override;
    void setCurrentProgram (int index) override;
    const juce::String getProgramName (int index) override;
    void changeProgramName (int index, const juce::String& newName) override;

    //==============================================================================
    void getStateInformation (juce::MemoryBlock& destData) override;
    void setStateInformation (const void* data, int sizeInBytes) override;

    juce::AudioProcessorValueTreeState apvts{*this, nullptr, "Parameters", Params::createParameterLayout()};

private:
    //==============================================================================
    // Parameter Plumbing - audio thread reads paramValues
    //==============================================================================
    std::array<std::atomic<float>*, Params::NumParams> paramValues{};

    float parameterValue (int index) const noexcept
    {
        return paramValues[static_cast<std::size_t> (index)]->load (std::memory_order_relaxed);
    }

    void update();

    //==============================================================================
    // Presets - message thread
    //==============================================================================
    std::vector<Preset> presets;
    int currentProgram = 0;

    //==============================================================================
    // Audio path - audio thread
    //==============================================================================
    Synth synth;

    void splitBufferByEvents (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midimessages);
    void handleMIDI (uint8_t data0, uint8_t data1, uint8_t data2);
    void render (juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset);

    //==============================================================================
    // Cross-thread handshakes
    //==============================================================================
    std::atomic<bool> parametersChanged{false};
    std::atomic<int> pendingProgram{-1};
    std::atomic<bool> resetRequested{false};

    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override
    {
        parametersChanged.store (true, std::memory_order_relaxed);
    }

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JX11AudioProcessor)
};
