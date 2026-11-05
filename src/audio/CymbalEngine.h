#pragma once
#include "ImpactVoiceCore.h"
#include <atomic>
namespace cymbalforge {
struct CymbalParameters {
    std::atomic<float> velocity{.8f}, position{.65f}, brightness{.65f}, decay{.35f}, choke{0}, complexity{.55f}, shimmer{.45f}, noise{.25f};
    std::atomic<float> subLevel{0}, subTune{.35f}, subDecay{.35f}, subDelay{0}, filterCutoff{1}, filterResonance{0}, modLfoRate{.25f}, modLfoDepth{0}, reverb{.2f}, gain{.75f};
    std::array<std::atomic<float>,4> sampleLevel{}, sampleTune{}, sampleAttack{}, sampleRelease{}, sampleDelay{}, sampleLfoRate{}, sampleLfoDepth{};
    std::atomic<int> filterMode{0}, modTarget{0}, modes{384}, model{0};
    CymbalParameters() { for(int i=0;i<4;++i){ sampleLevel[(size_t)i]=0; sampleTune[(size_t)i]=.5f; sampleAttack[(size_t)i]=0; sampleRelease[(size_t)i]=.35f; sampleDelay[(size_t)i]=0; sampleLfoRate[(size_t)i]=.25f; sampleLfoDepth[(size_t)i]=0; } }
    ImpactControls snapshot() const noexcept;
};
class CymbalEngine { public: static constexpr int maxImpactVoices=4; CymbalParameters parameters; void prepare(double rate, int block); void strike(float velocity, float position); void render(juce::AudioBuffer<float>& buffer, int start, int count); bool loadSampleSlot(int slot,const juce::File&); juce::File sampleFile(int slot) const; private: std::array<ImpactVoiceCore,maxImpactVoices> impacts; int nextImpactVoice=0; juce::dsp::Reverb reverb; juce::AudioFormatManager formats; std::array<juce::File,4> sampleFiles; };
}
