/*
  ==============================================================================

    This file contains the basic framework code for a JUCE plugin processor.

  ==============================================================================
*/

#pragma once

#include <JuceHeader.h>
#include <array>
#include <atomic>

#include "dsp/Synth.h"
#include "model/Preset.h"
#include "model/Parameters.h"

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

    // MIDI Learn. The editor calls setMidiLearn(true) from the message thread;
    // handleMIDI clears it on the audio thread once a CC is captured.
    void setMidiLearn (bool shouldLearn) noexcept { midiLearn.store (shouldLearn, std::memory_order_relaxed); }
    bool isMidiLearnActive() const noexcept { return midiLearn.load (std::memory_order_relaxed); }
    uint8_t getMidiLearnCC() const noexcept { return midiLearnCC.load (std::memory_order_relaxed); }

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

    void finalizeParams (SynthParams&, const UpdateContext&) const;

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
    std::atomic<float> pendingOutputLevel{-1.0f};

    // MIDI Learn handshake. midiLearnCC is written by handleMIDI (audio thread)
    // and read by get/setStateInformation (any thread); processBlock publishes
    // it into the audio-thread-only Synth::resoCC.
    std::atomic<bool> midiLearn{false};
    std::atomic<uint8_t> midiLearnCC{0x47};

    void valueTreePropertyChanged (juce::ValueTree&, const juce::Identifier&) override
    {
        parametersChanged.store (true, std::memory_order_relaxed);
    }

    void timerCallback() override;

    JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (JX11AudioProcessor)
};
