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

    # The processor is the one host-facing unit the tests exercise directly.
    # It is compiled here with JX11_HEADLESS so the UI translation units are
    # not pulled into the console app. The DSP/model come from JX11_engine.
    ${CMAKE_SOURCE_DIR}/Source/plugin/PluginProcessor.cpp
    )

target_include_directories(JX11Tests PRIVATE ${CMAKE_SOURCE_DIR}/Source)

target_link_libraries(JX11Tests PRIVATE
    JX11_engine
    juce::juce_audio_processors
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
