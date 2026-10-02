juce_add_console_app(JX11Tests PRODUCT_NAME "JX11 Tests")

juce_generate_juce_header(JX11Tests)

target_sources(JX11Tests PRIVATE
    Main.cpp
    VoiceTests.cpp
    VoiceAllocatorTests.cpp
    UtilsTests.cpp
    ParameterTests.cpp
    ParameterApplyTests.cpp
    PresetTests.cpp

    ProcessorTests.cpp
    SynthTests.cpp
    OscillatorTests.cpp
    NoiseGeneratorTests.cpp

    EnvelopeTests.cpp
    LFOTests.cpp
    FilterTests.cpp
    ModulationTests.cpp
    GoldenRenderTests.cpp
    ChoiceControlTests.cpp
    SliderControlTests.cpp
    EditorFocusTests.cpp

    # Host-facing and UI units the tests exercise directly. PluginProcessor.cpp
    # is compiled with JX11_HEADLESS (createEditor() returns nullptr); the
    # editor UI translation units are included so their controls can be tested.
    # The DSP/model come from JX11_engine.
    ${CMAKE_SOURCE_DIR}/Source/plugin/PluginProcessor.cpp
    ${CMAKE_SOURCE_DIR}/Source/plugin/PluginEditor.cpp
    ${CMAKE_SOURCE_DIR}/Source/plugin/LookAndFeel.cpp
    )

target_include_directories(JX11Tests PRIVATE ${CMAKE_SOURCE_DIR}/Source)

target_link_libraries(JX11Tests PRIVATE
    JX11_engine
    juce::juce_audio_processors
    juce::juce_gui_basics
    juce::juce_recommended_warning_flags
    )

# A console app has no plugin metadata, so provide the macros PluginProcessor.cpp
# expects. Keep these in sync with the juce_add_plugin() call in Source/.
target_compile_definitions(JX11Tests PRIVATE
         JX11_HEADLESS=1
         JucePlugin_Name="JX11"
         JucePlugin_IsSynth=1
         JucePlugin_WantsMidiInput=1
         JucePlugin_ProducesMidiOutput=0
         JucePlugin_IsMidiEffect=0)

add_test (NAME JX11Tests COMMAND JX11Tests)
