#pragma once

#include <juce_audio_processors/juce_audio_processors.h>
#include <memory>
#include <cmath>

#include "model/SynthParams.h"
#include "model/ParameterList.h"
#include "common/Modulation.h"
#include "common/Constants.h"

namespace Params
{

using ToTextFn = juce::String (*) (float value, int maxLength);

struct Spec
{
    const char* id;
    const char* name;
    const char* label; // "" if none
    float min, max, interval, skew;
    bool symmetricSkew;
    float defaultValue;
    const char* const* choices; // nullptr => float param
    int numChoices;
    ToTextFn toText; // nullptr => JUCE default formatting
    ApplyFn apply;
};

// Choice lists outside the table - spec trivially copyable
inline const char* const kGlideModeChoices[] = {"Off", "Legato", "Always"};
inline const char* const kPolyModeChoices[] = {"Mono", "Poly"};
inline const char* const kPanningChoices[] = {"Off", "On"};

//==============================================================================
// value to text (UI thread only)
//==============================================================================
inline juce::String oscMixToText (float value, int)
{
    char s[16] = {};
    snprintf (s, sizeof (s), "%4.0f:%2.0f", 100.0f - 0.5f * value, 0.5f * value);
    return juce::String (s);
}
inline juce::String filterVelocityToText (float value, int)
{
    return value < -90.0f ? juce::String ("OFF") : juce::String (value);
}
inline juce::String lfoRateToText (float value, int)
{
    // Same curve the DSP uses (see Modulation.h) - format it, don't re-derive it.
    return juce::String (lfoRateHz (value), 3);
}
inline juce::String vibratoToText (float value, int)
{
    return value < 0.0f ? "PWM " + juce::String (-value, 1) : juce::String (value, 1);
}

//==============================================================================
//
//==============================================================================

inline void applyPolyMode (SynthParams& p, float v, const UpdateContext&) noexcept
{
    p.numVoices = (v < 0.5f) ? 1 : SynthLimits::MAX_VOICES;
}

inline void applyPanning (SynthParams& p, float v, const UpdateContext&) noexcept
{
    p.panEnabled = (v >= 0.5f);
}

inline void applyNoise (SynthParams& p, float v, const UpdateContext&) noexcept
{
    const float n = v / 100.0f;
    p.noiseMix = n * n * 0.06f;
}

inline void applyOscMix (SynthParams& p, float v, const UpdateContext&) noexcept
{
    p.oscMix = v / 100.0f;
}

inline void applyFilterReso (SynthParams& p, float v, const UpdateContext&) noexcept
{
    p.filterQ = std::exp (3.0f * (v / 100.0f));
}

inline void applyEnvAttack (SynthParams& p, float v, const UpdateContext& c) noexcept
{
    p.envAttack = std::exp (-c.inverseSampleRate * std::exp (5.5f - 0.075f * v));
}

inline void applyEnvDecay (SynthParams& p, float v, const UpdateContext& c) noexcept
{
    p.envDecay = std::exp (-c.inverseSampleRate * std::exp (5.5f - 0.075f * v));
}

inline void applyEnvSustain (SynthParams& p, float v, const UpdateContext&) noexcept
{
    p.envSustain = v / 100.0f;
}

inline void applyEnvRelease (SynthParams& p, float v, const UpdateContext& c) noexcept
{
    p.envRelease = (v < 1.0f) ? 0.75f : std::exp (-c.inverseSampleRate * std::exp (5.5f - 0.075f * v));
}

inline void applyFilterVelocity (SynthParams& p, float v, const UpdateContext&) noexcept
{
    if (v < -90.0f)
    {
        p.velocitySensitivity = 0.0f;
        p.ignoreVelocity = true;
    }
    else
    {
        p.velocitySensitivity = 0.0005f * v;
        p.ignoreVelocity = false;
    }
}

inline void applyLfoRate (SynthParams& p, float v, const UpdateContext& c) noexcept
{
    p.lfoInc = lfoRateHz (v) * c.inverseUpdateRate * float (2 * PI);
}

inline void applyVibrato (SynthParams& p, float v, const UpdateContext&) noexcept
{
    p.pwmDepth = vibratoDepth (v);
    p.vibrato = (v < 0.0f) ? 0.0f : p.pwmDepth;
}

inline void applyGlideMode (SynthParams& p, float v, const UpdateContext&) noexcept
{
    p.glideMode = static_cast<int> (v);
}

inline void applyGlideRate (SynthParams& p, float v, const UpdateContext& c) noexcept
{
    p.glideRate = (v < 2.0f) ? 1.0f : glideCoefficient (v, c.inverseUpdateRate);
}

inline void applyGlideBend (SynthParams& p, float v, const UpdateContext&) noexcept
{
    p.glideBend = v;
}

inline void applyFilterFreq (SynthParams& p, float v, const UpdateContext&) noexcept
{
    p.filterKeyTracking = 0.08f * v - 1.5f;
}

inline void applyFilterLFO (SynthParams& p, float v, const UpdateContext&) noexcept
{
    const float f = v / 100.0f;
    p.filterLFODepth = 2.5f * f * f;
}

inline void applyFilterAttack (SynthParams& p, float v, const UpdateContext& c) noexcept
{
    p.filterAttack = std::exp (-c.inverseUpdateRate * std::exp (5.5f - 0.075f * v));
}

inline void applyFilterDecay (SynthParams& p, float v, const UpdateContext& c) noexcept
{
    p.filterDecay = std::exp (-c.inverseUpdateRate * std::exp (5.5f - 0.075f * v));
}

inline void applyFilterSustain (SynthParams& p, float v, const UpdateContext&) noexcept
{
    const float s = v / 100.0f;
    p.filterSustain = s * s;
}

inline void applyFilterRelease (SynthParams& p, float v, const UpdateContext& c) noexcept
{
    p.filterRelease = std::exp (-c.inverseUpdateRate * std::exp (5.5f - 0.075f * v));
}

inline void applyFilterEnv (SynthParams& p, float v, const UpdateContext&) noexcept
{
    p.filterEnvDepth = 0.06f * v;
}

//==============================================================================
//
//==============================================================================

// must follow the exact order in ParameterList.h
inline const Spec kSpecs[NumParams] = {
    // id,             name,            label,  min,     max,     interval, skew,  sym,   default, choices, n, toText,
    // apply
    {"oscMix", "Osc Mix", "%", 0.0f, 100.0f, 0.0f, 1.0f, false, 0.0f, nullptr, 0, oscMixToText, applyOscMix},
    {"oscTune", "Osc Tune", "semi", -24.0f, 24.0f, 1.0f, 1.0f, false, -12.0f, nullptr, 0, nullptr, nullptr},
    {"oscFine", "Osc Fine", "cent", -50.0f, 50.0f, 0.1f, 0.3f, true, 0.0f, nullptr, 0, nullptr, nullptr},
    {"glideMode", "Glide Mode", "", 0.0f, 2.0f, 1.0f, 1.0f, false, 0.0f, kGlideModeChoices, 3, nullptr, applyGlideMode},
    {"glideRate", "Glide Rate", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 35.0f, nullptr, 0, nullptr, applyGlideRate},
    {"glideBend", "Glide Bend", "semi", -36.0f, 36.0f, 0.01f, 0.4f, true, 0.0f, nullptr, 0, nullptr, applyGlideBend},
    {"filterFreq", "Filter Freq", "%", 0.0f, 100.0f, 0.1f, 1.0f, false, 100.0f, nullptr, 0, nullptr, applyFilterFreq},
    {"filterReso", "Filter Reso", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 15.0f, nullptr, 0, nullptr, applyFilterReso},
    {"filterEnv", "Filter Env", "%", -100.0f, 100.0f, 0.1f, 1.0f, false, 50.0f, nullptr, 0, nullptr, applyFilterEnv},
    {"filterLFO", "Filter LFO", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 0.0f, nullptr, 0, nullptr, applyFilterLFO},
    {"filterVelocity", "Velocity", "%", -100.0f, 100.0f, 1.0f, 1.0f, false, 0.0f, nullptr, 0, filterVelocityToText,
     applyFilterVelocity},
    {"filterAttack", "Filter Attack", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 0.0f, nullptr, 0, nullptr,
     applyFilterAttack},
    {"filterDecay", "Filter Decay", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 30.0f, nullptr, 0, nullptr, applyFilterDecay},
    {"filterSustain", "Filter Sustain", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 0.0f, nullptr, 0, nullptr,
     applyFilterSustain},
    {"filterRelease", "Filter Release", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 25.0f, nullptr, 0, nullptr,
     applyFilterRelease},
    {"envAttack", "Env Attack", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 0.0f, nullptr, 0, nullptr, applyEnvAttack},
    {"envDecay", "Env Decay", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 50.0f, nullptr, 0, nullptr, applyEnvDecay},
    {"envSustain", "Env Sustain", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 100.0f, nullptr, 0, nullptr, applyEnvSustain},
    {"envRelease", "Env Release", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 30.0f, nullptr, 0, nullptr, applyEnvRelease},
    {"lfoRate", "LFO Rate", "Hz", 0.0f, 1.0f, 0.0f, 1.0f, false, 0.81f, nullptr, 0, lfoRateToText, applyLfoRate},
    {"vibrato", "Vibrato", "%", -100.0f, 100.0f, 1.0f, 1.0f, false, 0.0f, nullptr, 0, vibratoToText, applyVibrato},
    {"noise", "Noise", "%", 0.0f, 100.0f, 1.0f, 1.0f, false, 0.0f, nullptr, 0, nullptr, applyNoise},
    {"octave", "Octave", "", -2.0f, 2.0f, 1.0f, 1.0f, false, 0.0f, nullptr, 0, nullptr, nullptr},
    {"tuning", "Tuning", "cent", -100.0f, 100.0f, 0.1f, 1.0f, false, 0.0f, nullptr, 0, nullptr, nullptr},
    {"outputLevel", "Output Level", "dB", -24.0f, 6.0f, 0.1f, 1.0f, false, 0.0f, nullptr, 0, nullptr, nullptr},
    {"polyMode", "Polyphony", "", 0.0f, 1.0f, 1.0f, 1.0f, false, 1.0f, kPolyModeChoices, 2, nullptr, applyPolyMode},
    {"panning", "Panning", "", 0.0f, 1.0f, 1.0f, 1.0f, false, 1.0f, kPanningChoices, 2, nullptr, applyPanning},
};

//==============================================================================
//
//==============================================================================

inline juce::AudioProcessorValueTreeState::ParameterLayout createParameterLayout()
{
    juce::AudioProcessorValueTreeState::ParameterLayout layout;

    for (int i = 0; i < NumParams; ++i)
    {
        const auto& s = kSpecs[i];

        if (s.choices != nullptr)
        {
            layout.add (std::make_unique<juce::AudioParameterChoice> (juce::ParameterID{s.id, 1}, s.name,
                                                                      juce::StringArray (s.choices, s.numChoices),
                                                                      static_cast<int> (s.defaultValue)));
        }

        else
        {
            auto attributes = juce::AudioParameterFloatAttributes();

            if (s.label != nullptr && *s.label != 0)
                attributes = attributes.withLabel (s.label);
            if (s.toText != nullptr)
                attributes = attributes.withStringFromValueFunction (s.toText);

            layout.add (std::make_unique<juce::AudioParameterFloat> (
                juce::ParameterID{s.id, 1}, s.name,
                juce::NormalisableRange<float> (s.min, s.max, s.interval, s.skew, s.symmetricSkew), s.defaultValue,
                attributes));
        }
    }
    return layout;
}

} // namespace Params