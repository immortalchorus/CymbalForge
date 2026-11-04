#pragma once
#include "audio/CymbalEngine.h"
#include "ui/CymbalSurface.h"
#include "export/OfflineRenderer.h"
#include "presets/PresetManager.h"
namespace cymbalforge {
class MainEditor : public juce::Component {
public:
    explicit MainEditor(CymbalEngine&);
    void resized() override;
private:
    CymbalEngine& engine;
    CymbalSurface surface;
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> controlLabels;
    juce::Label title, status, controlHeading;
    juce::TextButton audition{"Strike Cymbal"}, save{"Save Preset"}, load{"Load Preset"}, exportWav{"Export WAV Hit"};
};
}
