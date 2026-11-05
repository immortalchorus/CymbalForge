#include "ImpactVoiceCore.h"
namespace cymbalforge {
void ImpactVoiceCore::prepare (double sampleRate) { rate = sampleRate; modal.prepare(rate); noise.prepare(rate); }
void ImpactVoiceCore::trigger (float velocity, float position, const ImpactControls& c) {
    modal.strike(velocity, position, c.brightness, c.decay * (1.0f - c.choke), c.complexity, c.shimmer, c.model, c.modes);
    noise.strike(velocity, c.brightness, c.noise, c.shimmer, c.model);
    subEnvelope = velocity * c.subLevel;
    subDecayCoefficient = std::exp(-1.0f / (float(rate) * (0.018f + 2.5f * c.subDecay)));
    subHz = 28.0f + 172.0f * c.subTune;
    subDelaySamples=int(c.subDelay*2.0f*rate); modPhase=0; modStep=juce::MathConstants<float>::twoPi*(.1f+14.9f*c.modLfoRate)/float(rate); modDepth=c.modLfoDepth; modTarget=c.modTarget;
    const auto phaseStep = juce::MathConstants<float>::twoPi * subHz / float(rate);
    subStepSin = std::sin(phaseStep); subStepCos = std::cos(phaseStep);
    subSin = 0.0f; subCos = 1.0f;
    filterCoefficient = .005f + .92f * c.filterCutoff;
    filterResonance = c.filterResonance * .92f; filterMode = c.filterMode;
    for (int i=0;i<4;++i) { auto& voice=sampleVoices[(size_t)i]; const auto& layer=c.samples[(size_t)i]; voice.data=std::atomic_load_explicit(&sampleData[(size_t)i], std::memory_order_acquire); voice.position=0; voice.increment=std::pow(2.0, (layer.tune-.5f)*4.0) * (voice.data ? voice.data->sampleRate/rate : 1.0); voice.delaySamples=int(layer.delay*2.0f*rate); voice.envelope=0; voice.attackStep=1.0f/juce::jmax(1.0f, layer.attack*.5f*float(rate)); voice.releaseCoefficient=std::exp(-1.0f/(float(rate)*(.015f+4.0f*layer.release))); voice.lfoPhase=0; voice.lfoStep=juce::MathConstants<float>::twoPi*(.1f+14.9f*layer.lfoRate)/float(rate); voice.lfoDepth=layer.lfoDepth; voice.level=velocity*layer.level; voice.active=voice.data && voice.level>.00001f; }
}
std::array<float, 2> ImpactVoiceCore::render() {
    auto modalOutput = modal.process(); auto residual = noise.process();
    const auto modulation=std::sin(modPhase)*modDepth; modPhase+=modStep; auto sub = subDelaySamples-- > 0 ? 0.0f : subSin * subEnvelope;
    const auto nextSin = subSin * subStepCos + subCos * subStepSin;
    subCos = subCos * subStepCos - subSin * subStepSin; subSin = nextSin;
    subEnvelope *= subDecayCoefficient;
    auto left = modalOutput[0] + residual[0] + sub, right = modalOutput[1] + residual[1] + sub;
    for (auto& voice: sampleVoices) if (voice.active) { if(voice.delaySamples-- > 0) continue; const auto frames=voice.data->audio.getNumSamples(); const auto index=int(voice.position); if(index>=frames-1) { voice.active=false; continue; } const auto frac=float(voice.position-index); const auto read=[&](int channel){auto* d=voice.data->audio.getReadPointer(juce::jmin(channel,voice.data->audio.getNumChannels()-1)); return d[index]+(d[index+1]-d[index])*frac;}; voice.envelope=juce::jmin(1.0f,voice.envelope+voice.attackStep); const auto amp=voice.level*voice.envelope; left+=read(0)*amp; right+=read(1)*amp; voice.position+=voice.increment*(1.0+std::sin(voice.lfoPhase)*voice.lfoDepth*.12); voice.lfoPhase+=voice.lfoStep; voice.level*=voice.releaseCoefficient; if(voice.level<.00001f) voice.active=false; }
    const auto modulatedCoefficient=juce::jlimit(.002f,.995f,filterCoefficient+(modTarget==1?modulation*.25f:0)); if(modTarget==0){left*=1.0f+modulation;right*=1.0f+modulation;} filterLowL += modulatedCoefficient * (left - filterLowL + filterResonance * filterBandL); filterBandL += modulatedCoefficient * (filterLowL - filterBandL); filterLowR += modulatedCoefficient * (right - filterLowR + filterResonance * filterBandR); filterBandR += modulatedCoefficient * (filterLowR - filterBandR);
    if(filterMode==1) return {filterLowL,filterLowR}; if(filterMode==2) return {left-filterLowL,right-filterLowR}; if(filterMode==3) return {filterBandL,filterBandR}; return {left,right};
}
void ImpactVoiceCore::setSample (int slot, std::shared_ptr<const ImpactSampleData> sample) { if(juce::isPositiveAndBelow(slot,4)) std::atomic_store_explicit(&sampleData[(size_t)slot],std::move(sample),std::memory_order_release); }
}
