#pragma once
#include "ModalBank.h"
#include "NoiseLayer.h"
#include <memory>

namespace cymbalforge {
// UI-free, allocation-free per-voice impact renderer. This is intentionally
// shaped for later adoption by SpaceAge's Voice/renderSelectedEngineVoice path.
struct ImpactControls {
    float brightness = .65f, decay = .35f, choke = 0.0f, complexity = .55f, shimmer = .45f, swell=0, noise = .25f, noiseTone=.5f, noiseDecay=.35f;
    float subLevel = 0.0f, subTune = .35f, subDecay = .35f, subDelay=0, sub2Level=0, sub2Tune=.65f, sub2Decay=.2f, sub2Delay=0, filterCutoff = 1.0f, filterResonance = 0.0f, filterEnvAmount=0, filterEnvDecay=.25f;
    float modLfoRate=.25f, modLfoDepth=0, tempoBpm=120.0f; int modTarget=0, lfoSync=0, exciter=0;
    int filterMode = 0, modes = 384, model = 0;
    struct SampleLayer { float level=0, tune=0.5f, attack=0, decay=.3f, sustain=1.0f, release=.35f, delay=0, lfoRate=.25f, lfoDepth=0; bool loop=false; };
    std::array<SampleLayer, 4> samples {};
};

struct ImpactSampleData { juce::AudioBuffer<float> audio; double sampleRate=48000.0; };

class ImpactVoiceCore {
public:
    void prepare (double sampleRate);
    void trigger (float velocity, float position, const ImpactControls& controls);
    void release() noexcept;
    std::array<float, 2> render();
    bool active() const noexcept;
    bool samplesActive() const noexcept;
    void setSample (int slot, std::shared_ptr<const ImpactSampleData> sample);
private:
    ModalBank modal;
    NoiseLayer noise;
    double rate = 48000.0;
    struct SubVoice { float sin=0, cos=1, stepSin=0, stepCos=1, envelope=0, decayCoefficient=.999f; int delaySamples=0; };
    std::array<SubVoice, 2> subVoices;
    struct DecayEnvelope { float value=0, coefficient=.999f; void trigger(float initial, float decay){value=initial;coefficient=decay;} float next(){const auto result=value;value*=coefficient;return result;} };
    float filterLowL = 0, filterLowR = 0, filterBandL = 0, filterBandR = 0, filterCoefficient = 1, filterResonance = 0;
    DecayEnvelope filterEnvelope;
    enum class EnvelopeStage { attack, decay, sustain };
    struct SampleVoice { std::shared_ptr<const ImpactSampleData> data; double position=0, increment=1; int delaySamples=0, releaseSamples=1; EnvelopeStage stage=EnvelopeStage::attack; float envelope=0, attackStep=1, decayCoefficient=.999f, releaseCoefficient=.999f, sustain=1, lfoSin=0, lfoCos=1, lfoStepSin=0, lfoStepCos=1, lfoDepth=0, level=0; bool active=false, released=false, loop=false; };
    std::array<std::shared_ptr<const ImpactSampleData>, 4> sampleData;
    std::array<SampleVoice, 4> sampleVoices;
    int filterMode = 0;
    float modSin=0, modCos=1, modStepSin=0, modStepCos=1, modDepth=0; int modTarget=0;
};
}
