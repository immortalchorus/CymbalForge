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
    static constexpr int globalControlCount = 28;
    static constexpr int sampleControlCount = 9;
    void showSampleLayer(int slot);
    void refreshFromEngine();
    void applyStarterSound(int id);
    CymbalEngine& engine;
    CymbalSurface surface;
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> controlLabels;
    juce::OwnedArray<juce::TextButton> sampleLoadButtons;
    juce::Label title, status, controlHeading, modelLabel;
    juce::ComboBox modelSelector, filterSelector, lfoSyncSelector, modTargetSelector, sampleLayerSelector, starterSelector;
    juce::Label filterLabel, lfoSyncLabel, modTargetLabel, sampleHeading, sampleLayerLabel, starterLabel;
    int selectedSampleLayer = 0;
    std::unique_ptr<juce::FileChooser> presetChooser, sampleChooser;
    juce::TextButton audition{"Strike Cymbal"}, save{"Save Preset"}, load{"Load Preset"}, exportWav{"Export WAV Hit"};
};
}
