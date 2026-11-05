#pragma once
#include "ImpactVoiceCore.h"
#include <atomic>
namespace cymbalforge {
struct CymbalParameters { std::atomic<float> velocity{0.8f}, position{0.65f}, brightness{0.65f}, decay{0.35f}, choke{0.0f}, complexity{0.55f}, shimmer{0.45f}, noise{0.25f}, subLevel{0.0f}, subTune{0.35f}, subDecay{0.35f}, filterCutoff{1.0f}, filterResonance{0.0f}, reverb{0.2f}, gain{0.75f}; std::atomic<int> filterMode{0}, modes{384}, model{0}; };
class CymbalEngine { public: CymbalParameters parameters; void prepare(double rate, int block); void strike(float velocity, float position); void render(juce::AudioBuffer<float>& buffer, int start, int count); bool loadSampleSlot(int slot,const juce::File&); private: ImpactVoiceCore impact; juce::dsp::Reverb reverb; juce::AudioFormatManager formats; };
}
