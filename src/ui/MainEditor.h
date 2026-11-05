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
    static constexpr int globalControlCount = 33;
    static constexpr int sampleControlCount = 11;
    void showSampleLayer(int slot);
    void showGlobalPage(int page);
    void refreshFromEngine();
    void applyStarterSound(int id);
    CymbalEngine& engine;
    CymbalSurface surface;
    juce::OwnedArray<juce::Slider> sliders;
    juce::OwnedArray<juce::Label> controlLabels;
    juce::OwnedArray<juce::TextButton> sampleLoadButtons;
    juce::Label title, status, controlHeading, modelLabel;
    juce::ComboBox modelSelector, filterSelector, lfoSyncSelector, modTargetSelector, exciterSelector, midiPositionSelector, globalPageSelector, sampleLayerSelector, sampleLoopSelector, starterSelector;
    juce::Label filterLabel, lfoSyncLabel, modTargetLabel, exciterLabel, midiPositionLabel, globalPageLabel, sampleHeading, sampleLayerLabel, sampleLoopLabel, starterLabel;
    int selectedSampleLayer = 0, selectedGlobalPage = 0;
    std::unique_ptr<juce::FileChooser> presetChooser, sampleChooser;
    juce::TextButton audition{"Strike Cymbal"}, save{"Save Preset"}, load{"Load Preset"}, exportWav{"Export WAV Hit"}, clearLayer{"Clear Selected Layer"};
};
}
