cmake_minimum_required(VERSION 3.22)
project(Plasmatik VERSION 0.4.0)

if(APPLE)
    set(CMAKE_OSX_ARCHITECTURES "arm64;x86_64" CACHE STRING "Build universal macOS binary" FORCE)
    set(CMAKE_OSX_DEPLOYMENT_TARGET "11.0" CACHE STRING "Minimum macOS version" FORCE)
endif()

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(
  JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.12
  GIT_SHALLOW TRUE
)
FetchContent_MakeAvailable(JUCE)

juce_add_binary_data(PlasmatikAssets SOURCES
    Assets/fairy_logo.jpg
)

juce_add_plugin(Plasmatik
    COMPANY_NAME "Volodymyr Gnatenko"
    BUNDLE_ID "com.volodymyrgnatenko.plasmatik"
    PLUGIN_MANUFACTURER_CODE Vhnt
    PLUGIN_CODE Plmk
    FORMATS VST3
    PRODUCT_NAME "Plasmatik"
    IS_SYNTH TRUE
    NEEDS_MIDI_INPUT TRUE
    NEEDS_MIDI_OUTPUT FALSE
    IS_MIDI_EFFECT FALSE
    COPY_PLUGIN_AFTER_BUILD FALSE
)

juce_generate_juce_header(Plasmatik)

target_sources(Plasmatik PRIVATE
    Source/PluginProcessor.cpp
    Source/PluginProcessor.h
    Source/PluginEditor.cpp
    Source/PluginEditor.h
    Source/PlasmaVoice.h
    Source/PlasmaVoice.cpp
    Source/Euclid.h
)

target_compile_definitions(Plasmatik PUBLIC
    JUCE_WEB_BROWSER=0
    JUCE_USE_CURL=0
    JUCE_VST3_CAN_REPLACE_VST2=0
)

target_link_libraries(Plasmatik PRIVATE
    PlasmatikAssets
    juce::juce_audio_utils
    juce::juce_dsp
    PUBLIC
    juce::juce_recommended_config_flags
    juce::juce_recommended_lto_flags
    juce::juce_recommended_warning_flags
)
