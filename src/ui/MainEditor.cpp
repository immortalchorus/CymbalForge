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
    filterLabel.setText("Impact Filter", juce::dontSendNotification); filterLabel.setJustificationType(juce::Justification::centredRight);
    filterLabel.setTooltip("Filter applied to all metal, sub, noise, and sample layers.");
    filterSelector.addItem("Bypass",1); filterSelector.addItem("Low-pass",2); filterSelector.addItem("High-pass",3); filterSelector.addItem("Band-pass",4);
    filterSelector.setSelectedId(e.parameters.filterMode.load()+1, juce::dontSendNotification);
    filterSelector.onChange=[this]{engine.parameters.filterMode.store(filterSelector.getSelectedId()-1);};
    sampleHeading.setText("SAMPLE IMPACT LAYERS — LOAD UP TO FOUR WAV/AIFF/MP3 FILES", juce::dontSendNotification); sampleHeading.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(title); addAndMakeVisible(status); addAndMakeVisible(controlHeading); addAndMakeVisible(modelLabel); addAndMakeVisible(modelSelector); addAndMakeVisible(filterLabel); addAndMakeVisible(filterSelector); addAndMakeVisible(sampleHeading); addAndMakeVisible(surface);

    audition.setTooltip("Audition the cymbal using the current Velocity and strike position.");
    save.setTooltip("Choose a name and location for a JSON preset containing the current sound controls.");
    load.setTooltip("Choose a CymbalForge JSON preset to restore its sound controls.");
    exportWav.setTooltip("Render a 24-bit WAV cymbal hit to Documents/CymbalForge/cymbal-hit.wav.");
    for (auto* button : { &audition, &save, &load, &exportWav }) { button->setName(button->getButtonText()); addAndMakeVisible(button); }

    audition.onClick = [this] { engine.strike(engine.parameters.velocity.load(), engine.parameters.position.load()); status.setText("Auditioning current cymbal strike.", juce::dontSendNotification); };
    auto presetDirectory = juce::File::getSpecialLocation(juce::File::userDocumentsDirectory).getChildFile("CymbalForge"); presetDirectory.createDirectory();
    for(int slot=0;slot<4;++slot) { auto* button=sampleLoadButtons.add(new juce::TextButton("Load Sample " + juce::String(slot+1))); button->setName("Sample layer " + juce::String(slot+1)); button->setTooltip("Load audio into Sample Layer " + juce::String(slot+1) + ". Its level, tune, envelope, delay, and pitch LFO are available in the control grid."); button->onClick=[this,slot]{ sampleChooser=std::make_unique<juce::FileChooser>("Load impact sample",juce::File(),"*.wav;*.aif;*.aiff;*.mp3"); sampleChooser->launchAsync(juce::FileBrowserComponent::openMode|juce::FileBrowserComponent::canSelectFiles,[this,slot](const juce::FileChooser& chooser){auto file=chooser.getResult(); if(file==juce::File{}) return; status.setText(engine.loadSampleSlot(slot,file)?"Loaded Sample "+juce::String(slot+1)+": "+file.getFileName():"Sample load failed",juce::dontSendNotification);}); }; addAndMakeVisible(button); }
    save.onClick = [this, presetDirectory] {
        presetChooser = std::make_unique<juce::FileChooser>("Save CymbalForge preset", presetDirectory.getChildFile("untitled-cymbal.json"), "*.json");
        presetChooser->launchAsync(juce::FileBrowserComponent::saveMode | juce::FileBrowserComponent::canSelectFiles | juce::FileBrowserComponent::warnAboutOverwriting, [this] (const juce::FileChooser& chooser) {
            auto file = chooser.getResult(); if (file == juce::File{}) return;
            if (! file.hasFileExtension("json")) file = file.withFileExtension("json");
            status.setText(PresetManager::save(file, engine) ? "Preset saved: " + file.getFileName() : "Preset save failed", juce::dontSendNotification);
        });
    };
    load.onClick = [this, presetDirectory] {
        presetChooser = std::make_unique<juce::FileChooser>("Load CymbalForge preset", presetDirectory, "*.json");
        presetChooser->launchAsync(juce::FileBrowserComponent::openMode | juce::FileBrowserComponent::canSelectFiles, [this] (const juce::FileChooser& chooser) {
            auto file = chooser.getResult(); if (file == juce::File{}) return;
            auto loaded = PresetManager::load(file, engine); if (loaded) modelSelector.setSelectedId(engine.parameters.model.load()+1, juce::dontSendNotification);
            status.setText(loaded ? "Preset loaded: " + file.getFileName() : "Preset load failed: select a valid JSON preset", juce::dontSendNotification);
        });
    };
    exportWav.onClick = [this, presetDirectory] { auto f=presetDirectory.getChildFile("cymbal-hit.wav"); status.setText(OfflineRenderer::renderWav(f,engine.parameters)?"24-bit WAV exported to Documents/CymbalForge/cymbal-hit.wav":"WAV export failed",juce::dontSendNotification); };

    const char* names[] = { "Velocity", "Brightness", "Metal Decay", "Choke", "Complexity", "Shimmer", "Noise", "Sub Level", "Sub Tune", "Sub Decay", "Sub Delay", "Filter Cutoff", "Filter Resonance", "Mod LFO Rate", "Mod LFO Depth", "Reverb", "Output Gain", "Sample 1 Level", "Sample 2 Level", "Sample 3 Level", "Sample 4 Level" };
    const char* help[] = {
        "Strike strength. Higher values create a louder, harder hit.",
        "Spectral brightness. Higher values emphasize higher cymbal partials.",
        "Metal ring time. The lowest value is a very short metallic impact.",
        "Damps metal modes immediately. Higher values create tight bars, mutes, and impacts.",
        "Modal density and detune. Higher values create a denser, less pitch-centred metal field.",
        "Delayed high-frequency bloom. Higher values create more crash-like evolving metallic energy.",
        "Transient wash. Higher values add more metallic noise at the hit.",
        "Cinematic sub-impact level.", "Sub oscillator pitch from low thump to tonal impact.", "Sub oscillator release time.", "Delay before the sub arrives on the impact timeline.",
        "Shared filter frequency.", "Shared filter feedback/resonance.", "Tempo-ready modulation LFO speed.", "Global LFO depth; routes to amplitude by default.",
        "Room effect amount. Higher values add more simulated ambience.",
        "Final output level. Lower this control if the sound is too loud.",
        "Sample Layer 1 level.", "Sample Layer 2 level.", "Sample Layer 3 level.", "Sample Layer 4 level."
    };
    std::atomic<float>* values[] = { &e.parameters.velocity,&e.parameters.brightness,&e.parameters.decay,&e.parameters.choke,&e.parameters.complexity,&e.parameters.shimmer,&e.parameters.noise,&e.parameters.subLevel,&e.parameters.subTune,&e.parameters.subDecay,&e.parameters.subDelay,&e.parameters.filterCutoff,&e.parameters.filterResonance,&e.parameters.modLfoRate,&e.parameters.modLfoDepth,&e.parameters.reverb,&e.parameters.gain,&e.parameters.sampleLevel[0],&e.parameters.sampleLevel[1],&e.parameters.sampleLevel[2],&e.parameters.sampleLevel[3] };
    for (int i = 0; i < 21; ++i) {
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
    auto controls = b.removeFromBottom(300); controlHeading.setBounds(controls.removeFromTop(22));
    auto selectors=controls.removeFromTop(28); filterLabel.setBounds(selectors.removeFromLeft(110)); filterSelector.setBounds(selectors.removeFromLeft(155).reduced(2)); sampleHeading.setBounds(selectors);
    auto loads=controls.removeFromTop(30); for(auto* button:sampleLoadButtons) button->setBounds(loads.removeFromLeft(145).reduced(2));
    surface.setBounds(b); auto rows=3, columns=7, slotWidth=controls.getWidth()/columns, slotHeight=controls.getHeight()/rows;
    for (int i = 0; i < sliders.size(); ++i) { auto row=i/columns, column=i%columns; auto slot=juce::Rectangle<int>(controls.getX()+column*slotWidth,controls.getY()+row*slotHeight,slotWidth,slotHeight).reduced(4); controlLabels[i]->setBounds(slot.removeFromTop(20)); sliders[i]->setBounds(slot); }
}
}
