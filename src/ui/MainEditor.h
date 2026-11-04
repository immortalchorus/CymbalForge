#pragma once
#include "audio/CymbalEngine.h"
#include "ui/CymbalSurface.h"
#include "export/OfflineRenderer.h"
#include "presets/PresetManager.h"
namespace cymbalforge { class MainEditor : public juce::Component { public: explicit MainEditor(CymbalEngine&); void resized() override; private: CymbalEngine& engine; CymbalSurface surface; juce::OwnedArray<juce::Slider> sliders; juce::Label title, status; juce::TextButton audition{"Strike"}, save{"Save Preset"}, load{"Load Preset"}, exportWav{"Export WAV"}; }; }
