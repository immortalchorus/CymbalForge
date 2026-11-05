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
    lfoSyncLabel.setText("LFO Clock", juce::dontSendNotification); lfoSyncLabel.setJustificationType(juce::Justification::centredRight); lfoSyncLabel.setTooltip("Choose free-running Hertz or tempo-synced beat divisions for global and sample pitch LFOs.");
    lfoSyncSelector.addItem("Free Hz", 1); lfoSyncSelector.addItem("Tempo Sync", 2); lfoSyncSelector.setSelectedId(e.parameters.lfoSync.load() + 1, juce::dontSendNotification);
    lfoSyncSelector.setTooltip("Tempo Sync maps each LFO Rate control from fast beat divisions to slow bars using Tempo BPM.");
    lfoSyncSelector.onChange=[this]{engine.parameters.lfoSync.store(lfoSyncSelector.getSelectedId()-1);};
    sampleLayerLabel.setText("Edit Layer", juce::dontSendNotification); sampleLayerLabel.setJustificationType(juce::Justification::centredRight);
    sampleLayerLabel.setTooltip("Choose which of the four sample layers is shown in the detailed ADSR, tuning, delay, and pitch-LFO editor.");
    for (int slot = 0; slot < 4; ++slot) sampleLayerSelector.addItem("Sample Layer " + juce::String(slot + 1), slot + 1);
    sampleLayerSelector.setSelectedId(1, juce::dontSendNotification); sampleLayerSelector.setTooltip("Select a sample layer to edit its labeled detailed controls.");
    sampleLayerSelector.onChange = [this] { showSampleLayer(sampleLayerSelector.getSelectedId() - 1); };
    sampleHeading.setText("SAMPLE IMPACT LAYERS — LOAD UP TO FOUR WAV/AIFF/MP3 FILES", juce::dontSendNotification); sampleHeading.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(title); addAndMakeVisible(status); addAndMakeVisible(controlHeading); addAndMakeVisible(modelLabel); addAndMakeVisible(modelSelector); addAndMakeVisible(filterLabel); addAndMakeVisible(filterSelector); addAndMakeVisible(lfoSyncLabel); addAndMakeVisible(lfoSyncSelector); addAndMakeVisible(sampleHeading); addAndMakeVisible(sampleLayerLabel); addAndMakeVisible(sampleLayerSelector); addAndMakeVisible(surface);

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
    exportWav.onClick = [this, presetDirectory] { auto f=presetDirectory.getChildFile("cymbal-hit.wav"); status.setText(OfflineRenderer::renderWav(f,engine)?"24-bit WAV exported to Documents/CymbalForge/cymbal-hit.wav":"WAV export failed",juce::dontSendNotification); };

    const char* names[] = { "Velocity", "Strike Variation", "Brightness", "Metal Decay", "Choke", "Complexity", "Shimmer", "Noise", "Sub Level", "Sub Tune", "Sub Decay", "Sub Delay", "Filter Cutoff", "Filter Resonance", "Mod LFO Rate", "Mod LFO Depth", "Tempo BPM", "Transient", "Drive", "Stereo Width", "Safety Limiter", "Reverb", "Output Gain",
        "Sample 1 Level", "Sample 1 Tune", "Sample 1 Attack", "Sample 1 Decay", "Sample 1 Sustain", "Sample 1 Release", "Sample 1 Delay", "Sample 1 LFO Rate", "Sample 1 LFO Depth",
        "Sample 2 Level", "Sample 2 Tune", "Sample 2 Attack", "Sample 2 Decay", "Sample 2 Sustain", "Sample 2 Release", "Sample 2 Delay", "Sample 2 LFO Rate", "Sample 2 LFO Depth",
        "Sample 3 Level", "Sample 3 Tune", "Sample 3 Attack", "Sample 3 Decay", "Sample 3 Sustain", "Sample 3 Release", "Sample 3 Delay", "Sample 3 LFO Rate", "Sample 3 LFO Depth",
        "Sample 4 Level", "Sample 4 Tune", "Sample 4 Attack", "Sample 4 Decay", "Sample 4 Sustain", "Sample 4 Release", "Sample 4 Delay", "Sample 4 LFO Rate", "Sample 4 LFO Depth" };
    const char* help[] = {
        "Strike strength. Higher values create a louder, harder hit.",
        "Changes velocity and strike position slightly on each hit for more natural repeated impacts.",
        "Spectral brightness. Higher values emphasize higher cymbal partials.",
        "Metal ring time. The lowest value is a very short metallic impact.",
        "Damps metal modes immediately. Higher values create tight bars, mutes, and impacts.",
        "Modal density and detune. Higher values create a denser, less pitch-centred metal field.",
        "Delayed high-frequency bloom. Higher values create more crash-like evolving metallic energy.",
        "Transient wash. Higher values add more metallic noise at the hit.",
        "Cinematic sub-impact level.", "Sub oscillator pitch from low thump to tonal impact.", "Sub oscillator release time.", "Delay before the sub arrives on the impact timeline.",
        "Shared filter frequency.", "Shared filter feedback/resonance.", "Tempo-ready modulation LFO speed.", "Global LFO depth; routes to amplitude by default.",
        "Tempo used only while LFO Clock is set to Tempo Sync.",
        "Emphasizes the first edge of the hit for sharper film-impact attacks.",
        "Soft saturation after the impact mix. Use for denser, more aggressive hits.",
        "Stereo spread after the impact mix. Low is focused; high is wide.",
        "Output ceiling. Lower values protect against unexpectedly hot layers.",
        "Room effect amount. Higher values add more simulated ambience.",
        "Final output level. Lower this control if the sound is too loud.",
        "Sample Layer 1 level.", "Sample Layer 1 pitch: low to high.", "Sample Layer 1 ADSR attack.", "Sample Layer 1 ADSR decay.", "Sample Layer 1 ADSR sustain level.", "Sample Layer 1 final release/fade time.", "Sample Layer 1 delay after the metal hit.", "Sample Layer 1 pitch-LFO speed.", "Sample Layer 1 pitch-LFO amount.",
        "Sample Layer 2 level.", "Sample Layer 2 pitch: low to high.", "Sample Layer 2 ADSR attack.", "Sample Layer 2 ADSR decay.", "Sample Layer 2 ADSR sustain level.", "Sample Layer 2 final release/fade time.", "Sample Layer 2 delay after the metal hit.", "Sample Layer 2 pitch-LFO speed.", "Sample Layer 2 pitch-LFO amount.",
        "Sample Layer 3 level.", "Sample Layer 3 pitch: low to high.", "Sample Layer 3 ADSR attack.", "Sample Layer 3 ADSR decay.", "Sample Layer 3 ADSR sustain level.", "Sample Layer 3 final release/fade time.", "Sample Layer 3 delay after the metal hit.", "Sample Layer 3 pitch-LFO speed.", "Sample Layer 3 pitch-LFO amount.",
        "Sample Layer 4 level.", "Sample Layer 4 pitch: low to high.", "Sample Layer 4 ADSR attack.", "Sample Layer 4 ADSR decay.", "Sample Layer 4 ADSR sustain level.", "Sample Layer 4 final release/fade time.", "Sample Layer 4 delay after the metal hit.", "Sample Layer 4 pitch-LFO speed.", "Sample Layer 4 pitch-LFO amount."
    };
    std::atomic<float>* values[] = { &e.parameters.velocity,&e.parameters.variation,&e.parameters.brightness,&e.parameters.decay,&e.parameters.choke,&e.parameters.complexity,&e.parameters.shimmer,&e.parameters.noise,&e.parameters.subLevel,&e.parameters.subTune,&e.parameters.subDecay,&e.parameters.subDelay,&e.parameters.filterCutoff,&e.parameters.filterResonance,&e.parameters.modLfoRate,&e.parameters.modLfoDepth,&e.parameters.tempoBpm,&e.parameters.transient,&e.parameters.drive,&e.parameters.width,&e.parameters.limiter,&e.parameters.reverb,&e.parameters.gain };
    for (int i = 0; i < globalControlCount; ++i) {
        auto* label = controlLabels.add(new juce::Label());
        label->setText(names[i], juce::dontSendNotification); label->setTooltip(help[i]); label->setJustificationType(juce::Justification::centred);
        auto* slider = sliders.add(new juce::Slider(names[i]));
        slider->setName(names[i]); slider->setTooltip(help[i]); slider->setRange(i == 16 ? 40.0 : 0.0, i == 16 ? 240.0 : 1.0, i == 16 ? 1.0 : .001); slider->setValue(values[i]->load());
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
        slider->onValueChange = [slider, value = values[i]] { value->store((float) slider->getValue()); };
        addAndMakeVisible(label); addAndMakeVisible(slider);
    }
    for (int slot = 0; slot < 4; ++slot) {
        std::atomic<float>* layerValues[] = { &e.parameters.sampleLevel[(size_t) slot], &e.parameters.sampleTune[(size_t) slot], &e.parameters.sampleAttack[(size_t) slot], &e.parameters.sampleDecay[(size_t) slot], &e.parameters.sampleSustain[(size_t) slot], &e.parameters.sampleRelease[(size_t) slot], &e.parameters.sampleDelay[(size_t) slot], &e.parameters.sampleLfoRate[(size_t) slot], &e.parameters.sampleLfoDepth[(size_t) slot] };
        for (int control = 0; control < 9; ++control) {
            const auto index = globalControlCount + slot * sampleControlCount + control;
            auto* label = controlLabels.add(new juce::Label());
            label->setText(names[index], juce::dontSendNotification); label->setTooltip(help[index]); label->setJustificationType(juce::Justification::centred);
            auto* slider = sliders.add(new juce::Slider(names[index]));
            slider->setName(names[index]); slider->setTooltip(help[index]); slider->setRange(0, 1, .001); slider->setValue(layerValues[control]->load());
            slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
            slider->onValueChange = [slider, value = layerValues[control]] { value->store((float) slider->getValue()); };
            addAndMakeVisible(label); addAndMakeVisible(slider);
        }
    }
    showSampleLayer(0);
}

void MainEditor::showSampleLayer(int slot)
{
    selectedSampleLayer = juce::jlimit(0, 3, slot);
    for (int index = globalControlCount; index < sliders.size(); ++index) {
        const auto layer = (index - globalControlCount) / sampleControlCount;
        const auto visible = layer == selectedSampleLayer;
        sliders[index]->setVisible(visible); controlLabels[index]->setVisible(visible);
    }
    resized();
}

void MainEditor::resized()
{
    auto b = getLocalBounds().reduced(14);
    auto header = b.removeFromTop(30); title.setBounds(header.removeFromLeft(350)); modelLabel.setBounds(header.removeFromLeft(100)); modelSelector.setBounds(header.removeFromLeft(220).reduced(2)); status.setBounds(header);
    auto actions = b.removeFromTop(34); for (auto* x : { &audition, &save, &load, &exportWav }) x->setBounds(actions.removeFromLeft(150).reduced(3));
    auto controls = b.removeFromBottom(430); controlHeading.setBounds(controls.removeFromTop(22));
    auto selectors=controls.removeFromTop(28); filterLabel.setBounds(selectors.removeFromLeft(110)); filterSelector.setBounds(selectors.removeFromLeft(155).reduced(2)); lfoSyncLabel.setBounds(selectors.removeFromLeft(80)); lfoSyncSelector.setBounds(selectors.removeFromLeft(120).reduced(2)); sampleHeading.setBounds(selectors);
    auto loads=controls.removeFromTop(30); for(auto* button:sampleLoadButtons) button->setBounds(loads.removeFromLeft(145).reduced(2));
    auto layerBar = controls.removeFromTop(28); sampleLayerLabel.setBounds(layerBar.removeFromLeft(110)); sampleLayerSelector.setBounds(layerBar.removeFromLeft(170).reduced(2));
    auto layerControls = controls.removeFromBottom(122); surface.setBounds(b);
    constexpr int globalColumns = 6, globalRows = 4; const auto globalWidth = controls.getWidth() / globalColumns, globalHeight = controls.getHeight() / globalRows;
    for (int i = 0; i < globalControlCount; ++i) { auto slot = juce::Rectangle<int>(controls.getX() + (i % globalColumns) * globalWidth, controls.getY() + (i / globalColumns) * globalHeight, globalWidth, globalHeight).reduced(4); controlLabels[i]->setBounds(slot.removeFromTop(20)); sliders[i]->setBounds(slot); }
    constexpr int layerColumns = 9; const auto layerWidth = layerControls.getWidth() / layerColumns;
    for (int control = 0; control < sampleControlCount; ++control) { const auto index = globalControlCount + selectedSampleLayer * sampleControlCount + control; auto slot = juce::Rectangle<int>(layerControls.getX() + control * layerWidth, layerControls.getY(), layerWidth, layerControls.getHeight()).reduced(4); controlLabels[index]->setBounds(slot.removeFromTop(20)); sliders[index]->setBounds(slot); }
}
}
