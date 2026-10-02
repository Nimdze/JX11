juce_add_console_app(JX11Tests PRODUCT_NAME "JX11 Tests")

juce_generate_juce_header(JX11Tests)

target_sources(JX11Tests PRIVATE
    Main.cpp
    VoiceTests.cpp
    UtilsTests.cpp
    ParameterTests.cpp
    PresetTests.cpp

    ProcessorTests.cpp
    SynthTests.cpp
    SinOscillatorTests.cpp

    EnvelopeTests.cpp
    LFOTests.cpp
    FilterTests.cpp
    ModulationTests.cpp

    ${CMAKE_SOURCE_DIR}/Source/dsp/Synth.cpp
    ${CMAKE_SOURCE_DIR}/Source/model/Preset.cpp
    ${CMAKE_SOURCE_DIR}/Source/plugin/PluginProcessor.cpp
    )

target_include_directories(JX11Tests PRIVATE ${CMAKE_SOURCE_DIR}/Source)

target_link_libraries(JX11Tests PRIVATE
    juce::juce_core
    juce::juce_audio_processors
    juce::juce_recommended_warning_flags
    )

target_compile_definitions(JX11Tests PRIVATE                                                                               
         JucePlugin_Name="JX11"                                                                                                 
         JucePlugin_IsSynth=1                                                                                                   
         JucePlugin_WantsMidiInput=1                                                                                            
         JucePlugin_ProducesMidiOutput=0                                                                                        
         JucePlugin_IsMidiEffect=0) 

add_test (NAME JX11Tests COMMAND JX11Tests)