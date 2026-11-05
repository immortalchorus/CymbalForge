#pragma once
#include "ModalBank.h"
#include "NoiseLayer.h"
#include <memory>

namespace cymbalforge {
// UI-free, allocation-free per-voice impact renderer. This is intentionally
// shaped for later adoption by SpaceAge's Voice/renderSelectedEngineVoice path.
struct ImpactControls {
    float brightness = .65f, decay = .35f, choke = 0.0f, complexity = .55f, shimmer = .45f, noise = .25f, noiseTone=.5f;
    float subLevel = 0.0f, subTune = .35f, subDecay = .35f, subDelay=0, filterCutoff = 1.0f, filterResonance = 0.0f;
    float modLfoRate=.25f, modLfoDepth=0, tempoBpm=120.0f; int modTarget=0, lfoSync=0;
    int filterMode = 0, modes = 384, model = 0;
    struct SampleLayer { float level=0, tune=0.5f, attack=0, decay=.3f, sustain=1.0f, release=.35f, delay=0, lfoRate=.25f, lfoDepth=0; };
    std::array<SampleLayer, 4> samples {};
};

struct ImpactSampleData { juce::AudioBuffer<float> audio; double sampleRate=48000.0; };

class ImpactVoiceCore {
public:
    void prepare (double sampleRate);
    void trigger (float velocity, float position, const ImpactControls& controls);
    std::array<float, 2> render();
    bool active() const noexcept;
    void setSample (int slot, std::shared_ptr<const ImpactSampleData> sample);
private:
    ModalBank modal;
    NoiseLayer noise;
    double rate = 48000.0;
    float subSin = 0, subCos = 1, subStepSin = 0, subStepCos = 1, subEnvelope = 0, subDecayCoefficient = .999f;
    float filterLowL = 0, filterLowR = 0, filterBandL = 0, filterBandR = 0, filterCoefficient = 1, filterResonance = 0;
    float subHz = 80.0f;
    struct SampleVoice { std::shared_ptr<const ImpactSampleData> data; double position=0, increment=1; int delaySamples=0, releaseSamples=1, stage=0; float envelope=0, attackStep=1, decayCoefficient=.999f, sustain=1, lfoPhase=0, lfoStep=0, lfoDepth=0, level=0; bool active=false; };
    std::array<std::shared_ptr<const ImpactSampleData>, 4> sampleData;
    std::array<SampleVoice, 4> sampleVoices;
    int filterMode = 0;
    int subDelaySamples=0; float modPhase=0, modStep=0, modDepth=0; int modTarget=0;
};
}
