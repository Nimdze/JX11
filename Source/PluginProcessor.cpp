// Copyright (C) 2026 Nimdze
// SPDX-License-Identifier: GPL-3.0-or-later

/*
  ==============================================================================

    JX11AudioProcessor — the host-facing object for the plugin.

    Threading model

    message thread: constructor, setCurrentProgram, get/setStateInformation, timerCallback
    audio thread:   processBlock -> splitBufferByEvents -> handleMIDI / render -> update
    The two threads communicate only through the atomics in the "Cross-thread handshakes"
    section of the header (parametersChanged, pendingProgram, resetRequested).

  ==============================================================================
*/

#include "PluginProcessor.h"
#include "Parameters.h"
#include "Utils.h"
#include "Modulation.h"

//==============================================================================

JX11AudioProcessor::JX11AudioProcessor()
#ifndef JucePlugin_PreferredChannelConfigurations
    : AudioProcessor (BusesProperties()
#if !JucePlugin_IsMidiEffect
#if !JucePlugin_IsSynth
                          .withInput ("Input", juce::AudioChannelSet::stereo(), true)
#endif
                          .withOutput ("Output", juce::AudioChannelSet::stereo(), true)
#endif
      )
#endif
{
    for (int i = 0; i < Params::NumParams; ++i)
    {
        paramValues[static_cast<std::size_t> (i)] = apvts.getRawParameterValue (Params::kSpecs[i].id);
        jassert (paramValues[static_cast<std::size_t> (i)] != nullptr);
    }

    apvts.state.addListener (this);

    presets = createFactoryPresets();
    setCurrentProgram (0);

    startTimerHz (30);
}

JX11AudioProcessor::~JX11AudioProcessor()
{
    stopTimer();
    apvts.state.removeListener (this);
}

//==============================================================================

void JX11AudioProcessor::prepareToPlay (double sampleRate, int samplesPerBlock)
{
    synth.allocateResources (sampleRate, samplesPerBlock);
    parametersChanged.store (true);
    reset();
}

void JX11AudioProcessor::releaseResources()
{
    synth.deallocateResources();
}

void JX11AudioProcessor::reset()
{
    synth.reset();
    synth.outputLevelSmoother.setCurrentAndTargetValue (
        juce::Decibels::decibelsToGain (parameterValue (Params::outputLevel)));
}

//==============================================================================

#ifndef JucePlugin_PreferredChannelConfigurations
bool JX11AudioProcessor::isBusesLayoutSupported (const BusesLayout& layouts) const
{
#if JucePlugin_IsMidiEffect
    juce::ignoreUnused (layouts);
    return true;
#else
    // This is the place where you check if the layout is supported.
    // In this template code we only support mono or stereo.
    // Some plugin hosts, such as certain GarageBand versions, will only
    // load plugins that support stereo bus layouts.
    if (layouts.getMainOutputChannelSet() != juce::AudioChannelSet::mono() &&
        layouts.getMainOutputChannelSet() != juce::AudioChannelSet::stereo())
        return false;

    // This checks if the input layout matches the output layout
#if !JucePlugin_IsSynth
    if (layouts.getMainOutputChannelSet() != layouts.getMainInputChannelSet())
        return false;
#endif

    return true;
#endif
}
#endif

//==============================================================================

void JX11AudioProcessor::processBlock (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    juce::ScopedNoDenormals noDenormals;

    if (resetRequested.exchange (false, std::memory_order_relaxed))
        synth.reset();

    auto totalNumInputChannels = getTotalNumInputChannels();
    auto totalNumOutputChannels = getTotalNumOutputChannels();

    // Clear output channels with no input data
    for (auto i = totalNumInputChannels; i < totalNumOutputChannels; ++i)
    {
        buffer.clear (i, 0, buffer.getNumSamples());
    }

    bool expected = true;
    if (isNonRealtime() || parametersChanged.compare_exchange_strong (expected, false))
    {
        update();
    }

    splitBufferByEvents (buffer, midiMessages);
}

void JX11AudioProcessor::splitBufferByEvents (juce::AudioBuffer<float>& buffer, juce::MidiBuffer& midiMessages)
{
    int bufferOffset = 0;

    for (const auto metadata : midiMessages)
    {
        // Render audio from before this event
        int samplesThisSegment = metadata.samplePosition - bufferOffset;
        if (samplesThisSegment > 0)
        {
            render (buffer, samplesThisSegment, bufferOffset);
            bufferOffset += samplesThisSegment;
        }

        // Handle the incoming event, ignore irrelevant MIDI messages (sysex, etc.)
        if (metadata.numBytes <= 3)
        {
            uint8_t data1 = (metadata.numBytes >= 2) ? metadata.data[1] : 0;
            uint8_t data2 = (metadata.numBytes == 3) ? metadata.data[2] : 0;
            handleMIDI (metadata.data[0], data1, data2);
        }
    }

    // Render the audio after last MIDI event, if there are no events, this renders everything.
    int samplesLastSegment = buffer.getNumSamples() - bufferOffset;
    if (samplesLastSegment > 0)
    {
        render (buffer, samplesLastSegment, bufferOffset);
    }

    midiMessages.clear();
}

void JX11AudioProcessor::handleMIDI (uint8_t data0, uint8_t data1, uint8_t data2)
{
    if ((data0 & 0xF0) == 0xB0)
    {
        if (data1 == 0x07) // volume
        {
            pendingOutputLevel.store (static_cast<float> (data2) / 127.0f, std::memory_order_relaxed);
        }
    }

    if ((data0 & 0xF0) == 0xC0)
    {
        pendingProgram.store (static_cast<int> (data1), std::memory_order_release);
    }
    synth.midiMessage (data0, data1, data2);
}

void JX11AudioProcessor::render (juce::AudioBuffer<float>& buffer, int sampleCount, int bufferOffset)
{
    float* outputBuffers[2] = {nullptr, nullptr};
    outputBuffers[0] = buffer.getWritePointer (0) + bufferOffset;
    if (getTotalNumOutputChannels() > 1)
    {
        outputBuffers[1] = buffer.getWritePointer (1) + bufferOffset;
    }

    synth.render (outputBuffers, sampleCount);
}

//==============================================================================

void JX11AudioProcessor::update()
{

    float sampleRate = static_cast<float> (getSampleRate());
    float inverseSampleRate = 1.0f / sampleRate;

    synth.params.numVoices = (parameterValue (Params::polyMode) < 0.5f) ? 1 : Synth::MAX_VOICES;

    synth.outputLevelSmoother.setTargetValue (juce::Decibels::decibelsToGain (parameterValue (Params::outputLevel)));

    float noiseMix = parameterValue (Params::noise) / 100.0f;
    noiseMix *= noiseMix;
    synth.params.noiseMix = noiseMix * 0.06f;

    synth.params.oscMix = parameterValue (Params::oscMix) / 100.0f;

    float filterReso = parameterValue (Params::filterReso) / 100.f;

    synth.params.filterQ = std::exp (3.0f * filterReso);

    synth.params.volumeTrim = 0.0008f * (3.2f - synth.params.oscMix - 25.0f * synth.params.noiseMix) * (1.5f - 0.5f * filterReso);

    float octave = parameterValue (Params::octave);
    float tuning = parameterValue (Params::tuning);

    // define tuneInSemi = (sampleRate/(440 * 2^(-69/12))) = sampleRate/8.1758
    // 8.1758Hz is the reference freq for MIDI note number 0 so we want exp(0.05776226505f * tuneInSemi) = 1/8.175 -->
    // tuneInSemi = -36.3763
    float tuneInSemi = -36.3763f - 12.0f * octave - tuning / 100.0f; // subtracting bcause higher -> period smaller
    synth.params.tune = sampleRate * std::exp (0.05776226505f * tuneInSemi);

    float semi = parameterValue (Params::oscTune);
    float cent = parameterValue (Params::oscFine);
    synth.params.detune = std::pow (
        1.059463094359f,
        -semi -
            0.01f *
                cent); // multiplying period by 2^(-1/12) increases pitch in 1 semitone, 2^(-N/12) = 1.059463094359^(-N)

    synth.params.envAttack = std::exp (-inverseSampleRate * std::exp (5.5f - 0.075f * parameterValue (Params::envAttack)));
    synth.params.envDecay = std::exp (-inverseSampleRate * std::exp (5.5f - 0.075f * parameterValue (Params::envDecay)));

    synth.params.envSustain = parameterValue (Params::envSustain) / 100.0f;

    float envRelease = parameterValue (Params::envRelease);
    if (envRelease < 1.0f)
        synth.params.envRelease = 0.75f; // fast release, still avoids clicks and pops
    else
        synth.params.envRelease = std::exp (-inverseSampleRate * std::exp (5.5f - 0.075f * envRelease));

    float filterVelocity = parameterValue (Params::filterVelocity);
    if (filterVelocity < -90.0f)
    {
        synth.params.velocitySensitivity = 0.0f;
        synth.params.ignoreVelocity = true;
    }
    else
    {
        synth.params.velocitySensitivity = 0.0005f * filterVelocity;
        synth.params.ignoreVelocity = false;
    }

    const float inverseUpdateRate = inverseSampleRate * LFO::MAX_STEPS;
    float lfoRate = lfoRateHz (parameterValue (Params::lfoRate)); // skew
    synth.params.lfoInc = lfoRate * inverseUpdateRate * float (2 * PI);

    float vibrato = parameterValue (Params::vibrato) / 200.0f;
    synth.params.vibrato = vibratoDepth (parameterValue (Params::vibrato));

    synth.params.pwmDepth = synth.params.vibrato;
    if (vibrato < 0.0f)
    {
        synth.params.vibrato = 0.0f;
    }

    synth.params.glideMode = parameterValue (Params::glideMode);
    float glideRate = parameterValue (Params::glideRate);
    if (glideRate < 2.0f)
    {
        synth.params.glideRate = 1.0f; // no glide
    }
    else
    {
        synth.params.glideRate = glideCoefficient (glideRate, inverseUpdateRate);
    }

    synth.params.glideBend = parameterValue (Params::glideBend);

    synth.params.filterKeyTracking = 0.08f * parameterValue (Params::filterFreq) - 1.5f; // converts 0-100 to -1.5-6.5
    float filterLFO = parameterValue(Params::filterLFO) / 100.0f;
    synth.params.filterLFODepth = 2.5f * filterLFO * filterLFO;

    synth.params.filterAttack = std::exp(-inverseUpdateRate * std::exp(5.5f - 0.075 * parameterValue(Params::filterAttack)));
    synth.params.filterDecay = std::exp(-inverseUpdateRate * std::exp(5.5f - 0.075 * parameterValue(Params::filterDecay)));
    float filterSustain = parameterValue(Params::filterSustain) / 100.0f;
    synth.params.filterSustain = filterSustain * filterSustain;
    synth.params.filterRelease = std::exp(-inverseUpdateRate * std::exp(5.5f - 0.075 * parameterValue(Params::filterRelease)));
    synth.params.filterEnvDepth = 0.06f * parameterValue(Params::filterEnv);
}

//==============================================================================

void JX11AudioProcessor::timerCallback()
{
    const int requested = pendingProgram.exchange (-1, std::memory_order_acquire);
    if (requested >= 0)
        setCurrentProgram (requested);

    if (const float level = pendingOutputLevel.exchange (-1.0f, std::memory_order_relaxed); level >= 0.0f)
    {
        if (auto* p = apvts.getParameter (Params::kSpecs[Params::outputLevel].id))
        {
            p->beginChangeGesture();
            p->setValueNotifyingHost (level);
            p->endChangeGesture();
        }
    }

    const unsigned flags = synth.takeGuardFlags();

    if (flags & SampleGuardNaN)
        DBG ("JX11: NaN in output - buffer(s) silenced");
    if (flags & SampleGuardInf)
        DBG ("JX11: inf in output - buffer(s) silenced");
    if (flags & SampleGuardOutOfRange)
        DBG ("JX11: out-of-range samples - buffer(s) silenced");
    if (flags & SampleGuardClamped)
        DBG ("JX11: samples clamped");
}

//==============================================================================

const juce::String JX11AudioProcessor::getName() const
{
    return JucePlugin_Name;
}

bool JX11AudioProcessor::acceptsMidi() const
{
#if JucePlugin_WantsMidiInput
    return true;
#else
    return false;
#endif
}

bool JX11AudioProcessor::producesMidi() const
{
#if JucePlugin_ProducesMidiOutput
    return true;
#else
    return false;
#endif
}

bool JX11AudioProcessor::isMidiEffect() const
{
#if JucePlugin_IsMidiEffect
    return true;
#else
    return false;
#endif
}

double JX11AudioProcessor::getTailLengthSeconds() const
{
    return 0.0;
}

//==============================================================================

bool JX11AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* JX11AudioProcessor::createEditor()
{
    // Generic UI while the synth/parameters are in development.
    // JX11AudioProcessorEditor becomes the real UI in the book's "User interface" chapter.
    auto editor = new juce::GenericAudioProcessorEditor (*this);
    editor->setSize (500, 750);
    return editor;
}

//==============================================================================

int JX11AudioProcessor::getNumPrograms()
{
    return static_cast<int> (presets.size());
}

int JX11AudioProcessor::getCurrentProgram()
{
    return currentProgram;
}

void JX11AudioProcessor::setCurrentProgram (int index)
{
    if (index < 0 || index >= static_cast<int> (presets.size()))
        return;

    currentProgram = index;
    const Preset& preset = presets[static_cast<std::size_t> (index)];

    for (int i = 0; i < Params::NumParams; ++i)
    {
        if (auto* param = apvts.getParameter (Params::kSpecs[i].id))
            param->setValueNotifyingHost (param->convertTo0to1 (preset.param[i]));
    }

    resetRequested.store (true, std::memory_order_relaxed);
}

const juce::String JX11AudioProcessor::getProgramName (int index)
{
    if (index < 0 || index >= static_cast<int> (presets.size()))
        return {};

    return {presets[static_cast<std::size_t> (index)].name};
}

void JX11AudioProcessor::changeProgramName (int index, const juce::String& newName)
{
    juce::ignoreUnused (index, newName);
}

//==============================================================================

void JX11AudioProcessor::getStateInformation (juce::MemoryBlock& destData)
{
    copyXmlToBinary (*apvts.copyState().createXml(), destData);
}

void JX11AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));
    if (xml.get() != nullptr && xml->hasTagName (apvts.state.getType()))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*xml));
        parametersChanged.store (true);
        resetRequested.store (true, std::memory_order_relaxed);
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JX11AudioProcessor();
}
