cmake_minimum_required(VERSION 3.24)
project(CymbalForge VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
option(CYMBALFORGE_BUILD_TESTS "Build DSP tests" ON)

include(FetchContent)
FetchContent_Declare(JUCE
  GIT_REPOSITORY https://github.com/juce-framework/JUCE.git
  GIT_TAG 8.0.6)
FetchContent_MakeAvailable(JUCE)

juce_add_gui_app(CymbalForge
  PRODUCT_NAME "CymbalForge"
  COMPANY_NAME "CymbalForge"
  BUNDLE_ID "com.cymbalforge.app")

target_sources(CymbalForge PRIVATE
  src/app/Main.cpp src/app/MainComponent.cpp
  src/audio/CymbalEngine.cpp src/audio/ImpactVoiceCore.cpp src/audio/ModalBank.cpp src/audio/NoiseLayer.cpp
  src/export/OfflineRenderer.cpp src/presets/PresetManager.cpp
  src/ui/CymbalSurface.cpp src/ui/MainEditor.cpp)
target_include_directories(CymbalForge PRIVATE src)
target_compile_definitions(CymbalForge PRIVATE
  JUCE_WEB_BROWSER=0 JUCE_USE_CURL=0 JUCE_APPLICATION_NAME_STRING="$<TARGET_PROPERTY:CymbalForge,JUCE_PRODUCT_NAME>")
target_link_libraries(CymbalForge PRIVATE
  juce::juce_gui_extra juce::juce_audio_utils juce::juce_dsp
  juce::juce_audio_formats juce::juce_audio_devices juce::juce_audio_basics
  juce::juce_events juce::juce_graphics)
juce_generate_juce_header(CymbalForge)

if(CYMBALFORGE_BUILD_TESTS)
  enable_testing()
  juce_add_console_app(CymbalForgeTests PRODUCT_NAME "CymbalForgeTests")
  target_sources(CymbalForgeTests PRIVATE tests/CymbalEngineTests.cpp
    src/audio/CymbalEngine.cpp src/audio/ImpactVoiceCore.cpp src/audio/ModalBank.cpp src/audio/NoiseLayer.cpp src/presets/PresetManager.cpp)
  target_include_directories(CymbalForgeTests PRIVATE src)
  target_link_libraries(CymbalForgeTests PRIVATE juce::juce_audio_basics juce::juce_audio_formats juce::juce_dsp)
  juce_generate_juce_header(CymbalForgeTests)
  add_test(NAME CymbalForgeTests COMMAND CymbalForgeTests)
endif()
