juce_add_console_app(JX11Tests PRODUCT_NAME "JX11 Tests")

target_sources(JX11Tests PRIVATE
    Main.cpp
    VoiceTests.cpp
    )

target_include_directories(JX11Tests PRIVATE ${CMAKE_SOURCE_DIR}/Source)

target_link_libraries(JX11Tests PRIVATE
    juce::juce_core
    juce::juce_recommended_warning_flags
    )

add_test (NAME JX11Tests COMMAND JX11Tests)