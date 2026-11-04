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
    addAndMakeVisible(title); addAndMakeVisible(status); addAndMakeVisible(controlHeading); addAndMakeVisible(surface);

    audition.setTooltip("Audition the cymbal using the current Velocity and strike position.");
    save.setTooltip("Save the current sound controls to Documents/CymbalForge/preset.json.");
    load.setTooltip("Load Documents/CymbalForge/preset.json and restore its sound controls.");
    exportWav.setTooltip("Render a 24-bit WAV cymbal hit to Documents/CymbalForge/cymbal-hit.wav.");
    for (auto* button : { &audition, &save, &load, &exportWav }) { button->setName(button->getButtonText()); addAndMakeVisible(button); }

    audition.onClick = [this] { engine.strike(engine.parameters.velocity.load(), engine.parameters.position.load()); status.setText("Auditioning current cymbal strike.", juce::dontSendNotification); };
    auto dir = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("CymbalForge"); dir.createDirectory();
    save.onClick = [this, dir] { auto f=dir.getChildFile("preset.json"); status.setText(PresetManager::save(f,engine.parameters)?"Preset saved to Documents/CymbalForge/preset.json":"Preset save failed",juce::dontSendNotification); };
    load.onClick = [this, dir] { auto f=dir.getChildFile("preset.json"); status.setText(PresetManager::load(f,engine.parameters)?"Preset loaded from Documents/CymbalForge/preset.json":"No valid preset found in Documents/CymbalForge",juce::dontSendNotification); };
    exportWav.onClick = [this, dir] { auto f=dir.getChildFile("cymbal-hit.wav"); status.setText(OfflineRenderer::renderWav(f,engine.parameters)?"24-bit WAV exported to Documents/CymbalForge/cymbal-hit.wav":"WAV export failed",juce::dontSendNotification); };

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
    auto header = b.removeFromTop(30); title.setBounds(header.removeFromLeft(430)); status.setBounds(header);
    auto actions = b.removeFromTop(34); for (auto* x : { &audition, &save, &load, &exportWav }) x->setBounds(actions.removeFromLeft(150).reduced(3));
    auto controls = b.removeFromBottom(172); controlHeading.setBounds(controls.removeFromTop(22)); surface.setBounds(b);
    auto width = controls.getWidth() / sliders.size();
    for (int i = 0; i < sliders.size(); ++i) { auto slot = controls.removeFromLeft(width).reduced(4); controlLabels[i]->setBounds(slot.removeFromTop(22)); sliders[i]->setBounds(slot); }
}
}
