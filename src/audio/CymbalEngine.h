#pragma once
#include "ImpactVoiceCore.h"
#include <atomic>
namespace cymbalforge {
struct CymbalParameters {
    std::atomic<float> velocity{.8f}, position{.65f}, variation{.15f}, velocityTone{0}, brightness{.65f}, decay{.35f}, choke{0}, complexity{.55f}, shimmer{.45f}, swell{0}, noise{.25f}, noiseTone{.5f}, noiseDecay{.35f};
    std::atomic<float> subLevel{0}, subTune{.35f}, subDecay{.35f}, subDelay{0}, sub2Level{0}, sub2Tune{.65f}, sub2Decay{.2f}, sub2Delay{0}, filterCutoff{1}, filterResonance{0}, filterEnvAmount{0}, filterEnvDecay{.25f}, modLfoRate{.25f}, modLfoDepth{0}, tempoBpm{120.0f}, masterTune{0}, rattle{0}, drive{0}, transient{0}, width{.5f}, limiter{.8f}, reverb{.2f}, gain{.75f};
    std::array<std::atomic<float>,4> sampleLevel{}, sampleTune{}, sampleStart{}, samplePan{}, sampleAttack{}, sampleDecay{}, sampleSustain{}, sampleRelease{}, sampleDelay{}, sampleLfoRate{}, sampleLfoDepth{};
    std::array<std::atomic<int>,4> sampleLoop{}, sampleReverse{};
    std::atomic<int> filterMode{0}, modTarget{0}, lfoSync{0}, exciter{0}, midiPositionMap{0}, midiTuneMap{0}, modes{384}, voiceLimit{4}, model{0};
    CymbalParameters() { for(int i=0;i<4;++i){ sampleLevel[(size_t)i]=0; sampleTune[(size_t)i]=.5f; sampleStart[(size_t)i]=0; samplePan[(size_t)i]=.5f; sampleAttack[(size_t)i]=0; sampleDecay[(size_t)i]=.3f; sampleSustain[(size_t)i]=1; sampleRelease[(size_t)i]=.35f; sampleDelay[(size_t)i]=0; sampleLfoRate[(size_t)i]=.25f; sampleLfoDepth[(size_t)i]=0; sampleLoop[(size_t)i]=0; sampleReverse[(size_t)i]=0; } }
    void copyFrom(const CymbalParameters& source) noexcept;
    ImpactControls snapshot() const noexcept;
};
class CymbalEngine { public: static constexpr int maxImpactVoices=4; CymbalParameters parameters; void prepare(double rate, int block); void strike(float velocity, float position, float tuneOffset = 0); void strikeFromMidi(int note, float velocity); void setQuality(int index); int quality() const noexcept; void release(); void render(juce::AudioBuffer<float>& buffer, int start, int count); bool loadSampleSlot(int slot,const juce::File&); void clearSampleSlot(int slot); juce::File sampleFile(int slot) const; private: void setSampleSlot(int slot, std::shared_ptr<const ImpactSampleData> sample, const juce::File& file); std::array<ImpactVoiceCore,maxImpactVoices> impacts; int nextImpactVoice=0; double sampleRate=48000.0; juce::Random variationRandom{0x494d5041}; float previousL=0,previousR=0; juce::dsp::Reverb reverb; juce::AudioFormatManager formats; std::array<juce::File,4> sampleFiles; };
}
