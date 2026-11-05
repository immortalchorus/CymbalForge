#include "ImpactVoiceCore.h"
namespace cymbalforge {
void ImpactVoiceCore::prepare (double sampleRate) { rate = sampleRate; modal.prepare(rate); noise.prepare(rate); }
void ImpactVoiceCore::trigger (float velocity, float position, const ImpactControls& c) {
    modal.strike(velocity, position, c.brightness, c.decay * (1.0f - c.choke), c.complexity, c.shimmer, c.model, c.modes);
    noise.strike(velocity, c.brightness, c.noise, c.shimmer, c.model);
    subEnvelope = velocity * c.subLevel;
    subDecayCoefficient = std::exp(-1.0f / (float(rate) * (0.018f + 2.5f * c.subDecay)));
    subHz = 28.0f + 172.0f * c.subTune;
    const auto phaseStep = juce::MathConstants<float>::twoPi * subHz / float(rate);
    subStepSin = std::sin(phaseStep); subStepCos = std::cos(phaseStep);
    subSin = 0.0f; subCos = 1.0f;
    filterCoefficient = .01f + .98f * c.filterCutoff;
    filterMix = c.filterMix;
}
std::array<float, 2> ImpactVoiceCore::render() {
    auto modalOutput = modal.process(); auto residual = noise.process();
    auto sub = subSin * subEnvelope;
    const auto nextSin = subSin * subStepCos + subCos * subStepSin;
    subCos = subCos * subStepCos - subSin * subStepSin; subSin = nextSin;
    subEnvelope *= subDecayCoefficient;
    auto left = modalOutput[0] + residual[0] + sub, right = modalOutput[1] + residual[1] + sub;
    filterStateL += filterCoefficient * (left - filterStateL); filterStateR += filterCoefficient * (right - filterStateR);
    return { left * (1.0f - filterMix) + filterStateL * filterMix, right * (1.0f - filterMix) + filterStateR * filterMix };
}
}
