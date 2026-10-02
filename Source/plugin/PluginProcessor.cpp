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

#include "plugin/PluginProcessor.h"
#ifndef JX11_HEADLESS
#include "plugin/PluginEditor.h"
#endif
#include "model/Parameters.h"
#include "dsp/Utils.h"
#include "common/Modulation.h"

namespace
{
// Top-level state tags written by getStateInformation / read by
// setStateInformation.
const juce::Identifier pluginTag = "PLUGIN";
const juce::Identifier extraTag = "EXTRA";
const juce::Identifier midiCCAttribute = "midiCC";
const juce::Identifier programAttribute = "program";
} // namespace

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
    midiLearn.store (false, std::memory_order_relaxed);
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

    // Publish the learned resonance CC to the audio-thread-only Synth copy.
    synth.resoCC = midiLearnCC.load (std::memory_order_relaxed);

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
    // MIDI Learn: while active, the next CC message is captured (ignoring the
    // channel) and from then on drives filter resonance via Synth::resoCC.
    if (midiLearn.load (std::memory_order_relaxed) && (data0 & 0xF0) == 0xB0)
    {
        midiLearnCC.store (data1, std::memory_order_relaxed);
        midiLearn.store (false, std::memory_order_relaxed);
        return;
    }

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
    UpdateContext ctx{static_cast<float> (getSampleRate()), 0.0f, 0.0f};
    ctx.inverseSampleRate = 1.0f / ctx.sampleRate;
    ctx.inverseUpdateRate = ctx.inverseSampleRate * LFO::MAX_STEPS;

    SynthParams p;

    for (int i = 0; i < Params::NumParams; ++i)
        if (auto fn = Params::kSpecs[i].apply)
            fn (p, parameterValue (i), ctx);

    finalizeParams (p, ctx);
    synth.setParam (p);

    synth.outputLevelSmoother.setTargetValue (juce::Decibels::decibelsToGain (parameterValue (Params::outputLevel)));
}

void JX11AudioProcessor::finalizeParams (SynthParams& p, const UpdateContext& ctx) const
{
    // oscMix + noise + filterReso -> volumeTrim
    const float filterReso = parameterValue (Params::filterReso) / 100.0f;
    p.volumeTrim = 0.0008f * (3.2f - p.oscMix - 25.0f * p.noiseMix) * (1.5f - 0.5f * filterReso);

    // octave + tuning -> tune
    const float tuneInSemi =
        -36.3763f - 12.0f * parameterValue (Params::octave) - parameterValue (Params::tuning) / 100.0f;
    p.tune = ctx.sampleRate * std::exp (kSemitoneLog * tuneInSemi);

    // oscTune + oscFine -> detune
    const float semi = parameterValue (Params::oscTune);
    const float cent = parameterValue (Params::oscFine);
    p.detune = std::pow (kSemitoneRatio, -semi - 0.01f * cent);
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
    // The amp envelope is a one-pole; a long release can take several seconds
    // to fall below the silence threshold. Estimate it from the release
    // parameter and clamp to a host-friendly maximum.
    const double sr = getSampleRate() > 0.0 ? getSampleRate() : 44100.0;
    const float releaseParam = parameterValue (Params::envRelease);
    const double multiplier = std::exp (-1.0 / sr * std::exp (5.5 - 0.075 * static_cast<double> (releaseParam)));
    const double tauSeconds = 1.0 / (1.0 - multiplier) / sr;

    return juce::jlimit (0.0, 30.0, 10.0 * tauSeconds);
}

//==============================================================================

bool JX11AudioProcessor::hasEditor() const
{
    return true; // (change this to false if you choose to not supply an editor)
}

juce::AudioProcessorEditor* JX11AudioProcessor::createEditor()
{
#ifdef JX11_HEADLESS
    // The test binary links the processor but never opens an editor, so it
    // builds without the UI translation units.
    return nullptr;
#else
    return new JX11AudioProcessorEditor (*this);
#endif
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
    auto xml = std::make_unique<juce::XmlElement> (pluginTag);

    // All APVTS parameters live one level down, under <Parameters>.
    xml->addChildElement (apvts.copyState().createXml().release());

    // Non-parameter state that should survive a save/load: the learned CC and
    // the selected program index.
    auto extraXML = std::make_unique<juce::XmlElement> (extraTag);
    extraXML->setAttribute (midiCCAttribute, static_cast<int> (midiLearnCC.load (std::memory_order_relaxed)));
    extraXML->setAttribute (programAttribute, currentProgram);
    xml->addChildElement (extraXML.release());

    copyXmlToBinary (*xml, destData);
}

void JX11AudioProcessor::setStateInformation (const void* data, int sizeInBytes)
{
    std::unique_ptr<juce::XmlElement> xml (getXmlFromBinary (data, sizeInBytes));

    if (xml == nullptr || !xml->hasTagName (pluginTag))
        return;

    if (auto* parametersXML = xml->getChildByName (apvts.state.getType()))
    {
        apvts.replaceState (juce::ValueTree::fromXml (*parametersXML));
        parametersChanged.store (true);
        resetRequested.store (true, std::memory_order_relaxed);
    }

    if (auto* extraXML = xml->getChildByName (extraTag))
    {
        const int cc = extraXML->getIntAttribute (midiCCAttribute);
        if (cc > 0)
            midiLearnCC.store (static_cast<uint8_t> (cc), std::memory_order_relaxed);

        // Restore only the program label; the parameter values are restored from
        // the APVTS subtree above so user edits survive the round trip.
        const int program = extraXML->getIntAttribute (programAttribute, -1);
        if (program >= 0 && program < static_cast<int> (presets.size()))
            currentProgram = program;
    }
}

//==============================================================================
// This creates new instances of the plugin..
juce::AudioProcessor* JUCE_CALLTYPE createPluginFilter()
{
    return new JX11AudioProcessor();
}
