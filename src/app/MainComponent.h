#pragma once
#include "audio/CymbalEngine.h"
#include "ui/MainEditor.h"
namespace cymbalforge { class MainComponent : public juce::AudioAppComponent, private juce::MidiInputCallback { public: MainComponent(); ~MainComponent() override; void prepareToPlay(int,double) override; void getNextAudioBlock(const AudioSourceChannelInfo&) override; void releaseResources() override; void resized() override; void handleIncomingMidiMessage(juce::MidiInput*,const juce::MidiMessage&) override; private: CymbalEngine engine; MainEditor editor; std::unique_ptr<juce::MidiInput> midi; }; }
