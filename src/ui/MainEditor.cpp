#include "MainEditor.h"

namespace cymbalforge {
namespace {
std::array<std::atomic<float>*, 29> globalValues(CymbalParameters& p)
{
    return { &p.velocity,&p.variation,&p.brightness,&p.decay,&p.choke,&p.complexity,&p.shimmer,&p.swell,&p.noise,&p.noiseTone,&p.subLevel,&p.subTune,&p.subDecay,&p.subDelay,&p.sub2Level,&p.sub2Tune,&p.sub2Decay,&p.sub2Delay,&p.filterCutoff,&p.filterResonance,&p.modLfoRate,&p.modLfoDepth,&p.tempoBpm,&p.transient,&p.drive,&p.width,&p.limiter,&p.reverb,&p.gain };
}

std::array<std::atomic<float>*, 9> layerValues(CymbalParameters& p, int slot)
{
    const auto index = (size_t) slot;
    return { &p.sampleLevel[index],&p.sampleTune[index],&p.sampleAttack[index],&p.sampleDecay[index],&p.sampleSustain[index],&p.sampleRelease[index],&p.sampleDelay[index],&p.sampleLfoRate[index],&p.sampleLfoDepth[index] };
}
}
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
    modTargetLabel.setText("LFO Target", juce::dontSendNotification); modTargetLabel.setJustificationType(juce::Justification::centredRight); modTargetLabel.setTooltip("Choose whether the global LFO modulates loudness or the shared filter frequency.");
    modTargetSelector.addItem("Amplitude", 1); modTargetSelector.addItem("Filter Cutoff", 2); modTargetSelector.setSelectedId(e.parameters.modTarget.load() + 1, juce::dontSendNotification);
    modTargetSelector.setTooltip("Amplitude creates tremolo; Filter Cutoff creates moving resonant timbre."); modTargetSelector.onChange=[this]{engine.parameters.modTarget.store(modTargetSelector.getSelectedId()-1);};
    exciterLabel.setText("Exciter", juce::dontSendNotification); exciterLabel.setJustificationType(juce::Justification::centredRight); exciterLabel.setTooltip("Choose how the metal is excited: a stick transient, softer mallet, brush wash, or bowed swell.");
    exciterSelector.addItem("Stick", 1); exciterSelector.addItem("Mallet", 2); exciterSelector.addItem("Brush", 3); exciterSelector.addItem("Bow", 4); exciterSelector.setSelectedId(e.parameters.exciter.load() + 1, juce::dontSendNotification);
    exciterSelector.setTooltip("Exciter changes the contact-versus-wash balance at the synthesis source."); exciterSelector.onChange=[this]{engine.parameters.exciter.store(exciterSelector.getSelectedId()-1);};
    sampleLayerLabel.setText("Edit Layer", juce::dontSendNotification); sampleLayerLabel.setJustificationType(juce::Justification::centredRight);
    sampleLayerLabel.setTooltip("Choose which of the four sample layers is shown in the detailed ADSR, tuning, delay, and pitch-LFO editor.");
    for (int slot = 0; slot < 4; ++slot) sampleLayerSelector.addItem("Sample Layer " + juce::String(slot + 1), slot + 1);
    sampleLayerSelector.setSelectedId(1, juce::dontSendNotification); sampleLayerSelector.setTooltip("Select a sample layer to edit its labeled detailed controls.");
    sampleLayerSelector.onChange = [this] { showSampleLayer(sampleLayerSelector.getSelectedId() - 1); };
    sampleLoopLabel.setText("Playback", juce::dontSendNotification); sampleLoopLabel.setJustificationType(juce::Justification::centredRight); sampleLoopLabel.setTooltip("Choose one-shot playback or a loop that sustains until MIDI note-off.");
    sampleLoopSelector.addItem("One-shot", 1); sampleLoopSelector.addItem("Loop until Note-Off", 2); sampleLoopSelector.setSelectedId(1, juce::dontSendNotification); sampleLoopSelector.setTooltip("Loop is useful for sustained textures. MIDI Note-Off uses the sample layer Release control.");
    sampleLoopSelector.onChange = [this] { engine.parameters.sampleLoop[(size_t) selectedSampleLayer].store(sampleLoopSelector.getSelectedId() - 1); };
    starterLabel.setText("Starter Sound", juce::dontSendNotification); starterLabel.setJustificationType(juce::Justification::centredRight);
    starterSelector.addItem("Choose a starting point…", 1); starterSelector.addItem("Short Metal Bar", 2); starterSelector.addItem("Trailer Impact", 3); starterSelector.addItem("Dark Ride", 4); starterSelector.addItem("Textural Swell", 5);
    starterSelector.setSelectedId(1, juce::dontSendNotification); starterSelector.setTooltip("Loads a useful starting configuration. It does not load or erase your audio files.");
    starterSelector.onChange = [this] { if (starterSelector.getSelectedId() > 1) applyStarterSound(starterSelector.getSelectedId()); };
    sampleHeading.setText("SAMPLE IMPACT LAYERS — LOAD UP TO FOUR WAV/AIFF/MP3 FILES", juce::dontSendNotification); sampleHeading.setJustificationType(juce::Justification::centredLeft);
    addAndMakeVisible(title); addAndMakeVisible(status); addAndMakeVisible(controlHeading); addAndMakeVisible(modelLabel); addAndMakeVisible(modelSelector); addAndMakeVisible(filterLabel); addAndMakeVisible(filterSelector); addAndMakeVisible(lfoSyncLabel); addAndMakeVisible(lfoSyncSelector); addAndMakeVisible(modTargetLabel); addAndMakeVisible(modTargetSelector); addAndMakeVisible(exciterLabel); addAndMakeVisible(exciterSelector); addAndMakeVisible(sampleHeading); addAndMakeVisible(sampleLayerLabel); addAndMakeVisible(sampleLayerSelector); addAndMakeVisible(sampleLoopLabel); addAndMakeVisible(sampleLoopSelector); addAndMakeVisible(starterLabel); addAndMakeVisible(starterSelector); addAndMakeVisible(surface);

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
            auto loaded = PresetManager::load(file, engine); if (loaded) refreshFromEngine();
            status.setText(loaded ? "Preset loaded: " + file.getFileName() : "Preset load failed: select a valid JSON preset", juce::dontSendNotification);
        });
    };
    exportWav.onClick = [this, presetDirectory] { auto f=presetDirectory.getChildFile("cymbal-hit.wav"); status.setText(OfflineRenderer::renderWav(f,engine)?"24-bit WAV exported to Documents/CymbalForge/cymbal-hit.wav":"WAV export failed",juce::dontSendNotification); };

    const char* names[] = { "Velocity", "Strike Variation", "Brightness", "Metal Decay", "Choke", "Complexity", "Shimmer", "Metal Swell", "Noise", "Noise Colour", "Sub 1 Level", "Sub 1 Tune", "Sub 1 Decay", "Sub 1 Delay", "Sub 2 Level", "Sub 2 Tune", "Sub 2 Decay", "Sub 2 Delay", "Filter Cutoff", "Filter Resonance", "Mod LFO Rate", "Mod LFO Depth", "Tempo BPM", "Transient", "Drive", "Stereo Width", "Safety Limiter", "Reverb", "Output Gain",
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
        "Ramps the modal body in after the hit for bowed metal and cinematic rising impacts.",
        "Transient wash. Higher values add more metallic noise at the hit.",
        "Noise spectrum: low values are body, centre is metallic band, high values are airy transient.",
        "First cinematic sub-impact level.", "First sub oscillator pitch from low thump to tonal impact.", "First sub oscillator release time.", "Delay before Sub 1 arrives on the impact timeline.",
        "Second sub-impact level for layered cinematic impacts.", "Second sub oscillator pitch; set apart from Sub 1 for a richer impact body.", "Second sub oscillator release time.", "Delay before Sub 2 arrives on the impact timeline.",
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
    const auto values = globalValues(e.parameters);
    for (int i = 0; i < globalControlCount; ++i) {
        auto* label = controlLabels.add(new juce::Label());
        label->setText(names[i], juce::dontSendNotification); label->setTooltip(help[i]); label->setJustificationType(juce::Justification::centred);
        auto* slider = sliders.add(new juce::Slider(names[i]));
        slider->setName(names[i]); slider->setTooltip(help[i]); slider->setRange(i == 22 ? 40.0 : 0.0, i == 22 ? 240.0 : 1.0, i == 22 ? 1.0 : .001); slider->setValue(values[i]->load());
        slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
        slider->onValueChange = [slider, value = values[i]] { value->store((float) slider->getValue()); };
        addAndMakeVisible(label); addAndMakeVisible(slider);
    }
    for (int slot = 0; slot < 4; ++slot) {
        const auto layer = layerValues(e.parameters, slot);
        for (int control = 0; control < 9; ++control) {
            const auto index = globalControlCount + slot * sampleControlCount + control;
            auto* label = controlLabels.add(new juce::Label());
            label->setText(names[index], juce::dontSendNotification); label->setTooltip(help[index]); label->setJustificationType(juce::Justification::centred);
            auto* slider = sliders.add(new juce::Slider(names[index]));
            slider->setName(names[index]); slider->setTooltip(help[index]); slider->setRange(0, 1, .001); slider->setValue(layer[control]->load());
            slider->setTextBoxStyle(juce::Slider::TextBoxBelow, false, 64, 20);
            slider->onValueChange = [slider, value = layer[control]] { value->store((float) slider->getValue()); };
            addAndMakeVisible(label); addAndMakeVisible(slider);
        }
    }
    showSampleLayer(0);
}

void MainEditor::showSampleLayer(int slot)
{
    selectedSampleLayer = juce::jlimit(0, 3, slot);
    sampleLoopSelector.setSelectedId(engine.parameters.sampleLoop[(size_t) selectedSampleLayer].load() + 1, juce::dontSendNotification);
    for (int index = globalControlCount; index < sliders.size(); ++index) {
        const auto layer = (index - globalControlCount) / sampleControlCount;
        const auto visible = layer == selectedSampleLayer;
        sliders[index]->setVisible(visible); controlLabels[index]->setVisible(visible);
    }
    resized();
}

void MainEditor::refreshFromEngine()
{
    const auto globals = globalValues(engine.parameters);
    for (int index = 0; index < globalControlCount; ++index) sliders[index]->setValue(globals[index]->load(), juce::dontSendNotification);
    for (int slot = 0; slot < 4; ++slot) {
        const auto layer = layerValues(engine.parameters, slot);
        for (int control = 0; control < sampleControlCount; ++control) sliders[globalControlCount + slot * sampleControlCount + control]->setValue(layer[control]->load(), juce::dontSendNotification);
    }
    modelSelector.setSelectedId(engine.parameters.model.load() + 1, juce::dontSendNotification); filterSelector.setSelectedId(engine.parameters.filterMode.load() + 1, juce::dontSendNotification); lfoSyncSelector.setSelectedId(engine.parameters.lfoSync.load() + 1, juce::dontSendNotification); modTargetSelector.setSelectedId(engine.parameters.modTarget.load() + 1, juce::dontSendNotification); exciterSelector.setSelectedId(engine.parameters.exciter.load() + 1, juce::dontSendNotification);
    sampleLoopSelector.setSelectedId(engine.parameters.sampleLoop[(size_t) selectedSampleLayer].load() + 1, juce::dontSendNotification);
}

void MainEditor::applyStarterSound(int id)
{
    auto& p = engine.parameters;
    if (id == 2) { p.model = 1; p.decay = .01f; p.choke = .85f; p.complexity = .22f; p.brightness = .35f; p.noise = .08f; p.subLevel = .08f; p.subDecay = .04f; p.filterMode = 2; p.filterCutoff = .62f; p.reverb = .03f; }
    if (id == 3) { p.model = 2; p.decay = .14f; p.choke = .45f; p.complexity = .72f; p.brightness = .28f; p.noise = .56f; p.noiseTone = .25f; p.subLevel = .82f; p.subTune = .15f; p.subDecay = .46f; p.subDelay = .04f; p.transient = .68f; p.drive = .27f; p.filterMode = 1; p.filterCutoff = .55f; p.filterResonance = .20f; p.reverb = .10f; }
    if (id == 4) { p.model = 1; p.decay = .70f; p.choke = .0f; p.complexity = .40f; p.brightness = .42f; p.noise = .13f; p.subLevel = .0f; p.filterMode = 0; p.reverb = .25f; }
    if (id == 5) { p.model = 0; p.decay = .62f; p.choke = .0f; p.complexity = .90f; p.brightness = .75f; p.shimmer = .88f; p.noise = .33f; p.noiseTone = .78f; p.modLfoDepth = .32f; p.modLfoRate = .38f; p.modTarget = 1; p.lfoSync = 1; p.tempoBpm = 96.0f; p.filterMode = 3; p.filterCutoff = .76f; p.filterResonance = .32f; p.reverb = .42f; }
    refreshFromEngine();
    status.setText("Starter sound loaded. Strike the cymbal to audition it.", juce::dontSendNotification);
}

void MainEditor::resized()
{
    auto b = getLocalBounds().reduced(14);
    auto header = b.removeFromTop(30); title.setBounds(header.removeFromLeft(350)); modelLabel.setBounds(header.removeFromLeft(100)); modelSelector.setBounds(header.removeFromLeft(220).reduced(2)); status.setBounds(header);
    auto actions = b.removeFromTop(34); for (auto* x : { &audition, &save, &load, &exportWav }) x->setBounds(actions.removeFromLeft(140).reduced(3)); starterLabel.setBounds(actions.removeFromLeft(95)); starterSelector.setBounds(actions.removeFromLeft(230).reduced(2));
    auto controls = b.removeFromBottom(430); controlHeading.setBounds(controls.removeFromTop(22));
    auto selectors=controls.removeFromTop(28); filterLabel.setBounds(selectors.removeFromLeft(100)); filterSelector.setBounds(selectors.removeFromLeft(135).reduced(2)); lfoSyncLabel.setBounds(selectors.removeFromLeft(70)); lfoSyncSelector.setBounds(selectors.removeFromLeft(105).reduced(2)); modTargetLabel.setBounds(selectors.removeFromLeft(72)); modTargetSelector.setBounds(selectors.removeFromLeft(110).reduced(2)); exciterLabel.setBounds(selectors.removeFromLeft(55)); exciterSelector.setBounds(selectors.removeFromLeft(105).reduced(2)); sampleHeading.setBounds(selectors);
    auto loads=controls.removeFromTop(30); for(auto* button:sampleLoadButtons) button->setBounds(loads.removeFromLeft(145).reduced(2));
    auto layerBar = controls.removeFromTop(28); sampleLayerLabel.setBounds(layerBar.removeFromLeft(110)); sampleLayerSelector.setBounds(layerBar.removeFromLeft(170).reduced(2)); sampleLoopLabel.setBounds(layerBar.removeFromLeft(85)); sampleLoopSelector.setBounds(layerBar.removeFromLeft(170).reduced(2));
    auto layerControls = controls.removeFromBottom(122); surface.setBounds(b);
    constexpr int globalColumns = 7, globalRows = 4; const auto globalWidth = controls.getWidth() / globalColumns, globalHeight = controls.getHeight() / globalRows;
    for (int i = 0; i < globalControlCount; ++i) { auto slot = juce::Rectangle<int>(controls.getX() + (i % globalColumns) * globalWidth, controls.getY() + (i / globalColumns) * globalHeight, globalWidth, globalHeight).reduced(4); controlLabels[i]->setBounds(slot.removeFromTop(20)); sliders[i]->setBounds(slot); }
    constexpr int layerColumns = 9; const auto layerWidth = layerControls.getWidth() / layerColumns;
    for (int control = 0; control < sampleControlCount; ++control) { const auto index = globalControlCount + selectedSampleLayer * sampleControlCount + control; auto slot = juce::Rectangle<int>(layerControls.getX() + control * layerWidth, layerControls.getY(), layerWidth, layerControls.getHeight()).reduced(4); controlLabels[index]->setBounds(slot.removeFromTop(20)); sliders[index]->setBounds(slot); }
}
}
