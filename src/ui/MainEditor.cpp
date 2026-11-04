#include "MainEditor.h"

namespace cymbalforge {
MainEditor::MainEditor(CymbalEngine& e) : engine(e), surface(e)
{
    title.setText("CYMBALFORGE  /  HYBRID METAL SYNTHESIS", juce::dontSendNotification);
    title.setJustificationType(juce::Justification::centredLeft);
    title.setTooltip("CymbalForge: a modal synthesis instrument for acoustic cymbal sounds.");
    status.setJustificationType(juce::Justification::centredRight);
    status.setText("Click the cymbal to audition a strike.", juce::dontSendNotification);
    controlHeading.setText("SOUND CONTROLS", juce::dontSendNotification);
    controlHeading.setJustificationType(juce::Justification::centredLeft);
    modelLabel.setText("Cymbal Family", juce::dontSendNotification);
    modelLabel.setTooltip("Choose the underlying modal family: Thin Crash, Dark Ride, or Trash Stack.");
    modelLabel.setJustificationType(juce::Justification::centredRight);
    modelSelector.setName("Cymbal Family");
    modelSelector.setTooltip("Thin Crash is bright and quick; Dark Ride is heavier and more focused; Trash Stack is irregular and noisy.");
    modelSelector.addItem("Thin Crash — bright, spreading", 1);
    modelSelector.addItem("Dark Ride — focused, long ring", 2);
    modelSelector.addItem("Trash Stack — irregular, raw", 3);
    modelSelector.setSelectedId(e.parameters.model.load() + 1, juce::dontSendNotification);
    modelSelector.onChange = [this] { engine.parameters.model.store(modelSelector.getSelectedId() - 1); status.setText("Cymbal family changed. Strike to hear the new model.", juce::dontSendNotification); };
    addAndMakeVisible(title); addAndMakeVisible(status); addAndMakeVisible(controlHeading); addAndMakeVisible(modelLabel); addAndMakeVisible(modelSelector); addAndMakeVisible(surface);

    audition.setTooltip("Audition the cymbal using the current Velocity and strike position.");
    save.setTooltip("Choose a name and location for a JSON preset containing the current sound controls.");
    load.setTooltip("Choose a CymbalForge JSON preset to restore its sound controls.");
    exportWav.setTooltip("Render a 24-bit WAV cymbal hit to Documents/CymbalForge/cymbal-hit.wav.");
    for (auto* button : { &audition, &save, &load, &exportWav }) { button->setName(button->getButtonText()); addAndMakeVisible(button); }

    audition.onClick = [this] { engine.strike(engine.parameters.velocity.load(), engine.parameters.position.load()); status.setText("Auditioning current cymbal strike.", juce::dontSendNotification); };
    auto presetDirectory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("CymbalForge"); presetDirectory.createDirectory();
    save.onClick = [this, presetDirectory] {
        presetChooser = std::make_unique<juce::FileChooser>("Save CymbalForge preset", presetDirectory.getChildFile("untitled-cymbal.json"), "*.json");
        presetChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting, [this] (const juce::FileChooser& chooser) {
            auto file = chooser.getResult(); if (file == juce::File{}) return;
            if (! file.hasFileExtension("json")) file = file.withFileExtension("json");
            status.setText(PresetManager::save(file, engine.parameters) ? "Preset saved: " + file.getFileName() : "Preset save failed", juce::dontSendNotification);
        });
    };
    load.onClick = [this, presetDirectory] {
        presetChooser = std::make_unique<juce::FileChooser>("Load CymbalForge preset", presetDirectory, "*.json");
        presetChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& chooser) {
            auto file = chooser.getResult(); if (file == juce::File{}) return;
            auto loaded = PresetManager::load(file, engine.parameters); if (loaded) modelSelector.setSelectedId(engine.parameters.model.load()+1, juce::dontSendNotification);
            status.setText(loaded ? "Preset loaded: " + file.getFileName() : "Preset load failed: select a valid JSON preset", juce::dontSendNotification);
        });
    };
    exportWav.onClick = [this, presetDirectory] { auto f=presetDirectory.getChildFile("cymbal-hit.wav"); status.setText(OfflineRenderer::renderWav(f,engine.parameters)?"24-bit WAV exported to Documents/CymbalForge/cymbal-hit.wav":"WAV export failed",juce::dontSendNotification); };

    const char* names[] = { "Velocity", "Brightness", "Decay", "Complexity", "Shimmer", "Noise", "Reverb", "Output Gain" };
    const char* help[] = {
        "Strike strength. Higher values create a louder, harder hit.",
        "Spectral brightness. Higher values emphasize higher cymbal partials.",
        "Ring time. Higher values make the cymbal sustain longer.",
        "Modal density and detune. Higher values create a denser, less pitch-centred metal field.",
        "Delayed high-frequency bloom. Higher values create more crash-like evolving metallic energy.",
        "Transient wash. Higher values add more metallic noise at the hit.",
        "Room effect amount. Higher values add more simulated ambience.",
        "Final output level. Lower this control if the sound is too loud."
    };
    std::atomic<float>* values[] = { &e.parameters.velocity, &e.parameters.brightness, &e.parameters.decay, &e.parameters.complexity, &e.parameters.shimmer, &e.parameters.noise, &e.parameters.reverb, &e.parameters.gain };
    for (int i = 0; i < 8; ++i) {
        auto* label = controlLabels.add(new juce::Label());
        label->setText(names[i], juce::dontSendNotification); label->setTooltip(help[i]); label->setJustificationType(juce::Justification::centred);
        auto* slider = sliders.add(new juce::Slider(names[i]));
        slider->setName(names[i]); slider->setTooltip(help[i]); slider->setRange(0, 1, .001); slider->setValue(values[i]->load());
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
        slider->onValueChange = [slider, value = values[i]] { value->store((float) slider->getValue()); };
        addAndMakeVisible(label); addAndMakeVisible(slider);
    }
}

void MainEditor::resized()
{
    auto b = getLocalBounds().reduced(14);
    auto header = b.removeFromTop(30); title.setBounds(header.removeFromLeft(350)); modelLabel.setBounds(header.removeFromLeft(100)); modelSelector.setBounds(header.removeFromLeft(220).reduced(2)); status.setBounds(header);
    auto actions = b.removeFromTop(34); for (auto* x : { &audition, &save, &load, &exportWav }) x->setBounds(actions.removeFromLeft(150).reduced(3));
    auto controls = b.removeFromBottom(172); controlHeading.setBounds(controls.removeFromTop(22)); surface.setBounds(b);
    auto width = controls.getWidth() / sliders.size();
    for (int i = 0; i < sliders.size(); ++i) { auto slot = controls.removeFromLeft(width).reduced(4); controlLabels[i]->setBounds(slot.removeFromTop(22)); sliders[i]->setBounds(slot); }
}
}
