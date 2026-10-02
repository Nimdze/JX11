juce_add_plugin(JX11
COMPANY_NAME "yourcompany"
PLUGIN_MANUFACTURER_CODE Manu
PLUGIN_CODE Jx11
PRODUCT_NAME "JX11"
DESCRIPTION "JX11 Subtractive Synth"
FORMATS Standalone VST3 AU
IS_SYNTH TRUE
NEEDS_MIDI_INPUT TRUE
IS_MIDI_EFFECT FALSE
COPY_PLUGIN_AFTER_BUILD TRUE
VERSION ${PROJECT_VERSION}
)

target_sources(JX11 PRIVATE
    plugin/PluginProcessor.cpp
    plugin/PluginProcessor.h
    plugin/PluginEditor.cpp
    plugin/PluginEditor.h
    plugin/LookAndFeel.cpp
    plugin/LookAndFeel.h

    common/Constants.h
    common/Modulation.h

    model/ParameterList.h
    model/Parameters.h
    model/SynthParams.h
    model/MidiState.h
    model/Preset.h
    model/Preset.cpp

    dsp/Utils.h
    dsp/Envelope.h
    dsp/Filter.h
    dsp/LFO.h
    dsp/NoiseGenerator.h
    dsp/Oscillator.h
    dsp/SinOscillator.h
    dsp/Voice.h
    dsp/VoiceAllocator.h
    dsp/Synth.h
    dsp/Synth.cpp
)

# so subfolder-qualified includes like "dsp/Synth.h" resolve for the plugin target
target_include_directories(JX11 PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

target_compile_features(JX11 PRIVATE cxx_std_17)

target_compile_definitions(JX11 PRIVATE
    JUCE_VST3_CAN_REPLACE_VST2=0
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    DONT_SET_USING_JUCE_NAMESPACE=1
)

target_link_libraries(JX11 PRIVATE
    juce::juce_audio_basics
    juce::juce_audio_devices
    juce::juce_audio_processors
    juce::juce_audio_utils
    juce::juce_core
    juce::juce_data_structures
    juce::juce_events
    juce::juce_graphics
    juce::juce_gui_basics
    juce::juce_gui_extra
    juce::juce_recommended_config_flags
    juce::juce_recommended_warning_flags
    juce::juce_recommended_lto_flags
)

juce_generate_juce_header(JX11)

