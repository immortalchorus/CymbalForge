#include "ImpactVoiceCore.h"
namespace cymbalforge {
namespace {
float lfoFrequency(float normalizedRate, float tempoBpm, bool synced) noexcept
{
    if (! synced) return .1f + 14.9f * normalizedRate;
    const auto beatsPerCycle = .25f + 3.75f * (1.0f - normalizedRate);
    return (tempoBpm / 60.0f) / beatsPerCycle;
}
}
void ImpactVoiceCore::prepare (double sampleRate) { rate = sampleRate; modal.prepare(rate); noise.prepare(rate); }
void ImpactVoiceCore::trigger (float velocity, float position, const ImpactControls& c, int onsetDelay) {
    onsetDelaySamples = juce::jmax(0, onsetDelay);
    modal.strike({ velocity, position, c.brightness, c.decay * (1.0f - c.choke), c.complexity, c.shimmer, c.swell, c.masterTune, c.model, c.modes });
    noise.strike(velocity, c.brightness, c.noise, c.noiseTone, c.noiseDecay, c.shimmer, c.model, c.exciter);
    const auto configureSub = [this, velocity, &c] (SubVoice& sub, float level, float tune, float decay, float delay) { const auto hz = (28.0f + 172.0f * tune) * std::pow(2.0f, c.masterTune / 12.0f); const auto step = juce::MathConstants<float>::twoPi * hz / float(rate); sub.sin=0; sub.cos=1; sub.stepSin=std::sin(step); sub.stepCos=std::cos(step); sub.envelope=velocity*level; sub.decayCoefficient=std::exp(-1.0f/(float(rate)*(.018f+2.5f*decay))); sub.delaySamples=int(delay*2.0f*rate); };
    configureSub(subVoices[0], c.subLevel, c.subTune, c.subDecay, c.subDelay);
    configureSub(subVoices[1], c.sub2Level, c.sub2Tune, c.sub2Decay, c.sub2Delay);
    const auto modStep=juce::MathConstants<float>::twoPi*lfoFrequency(c.modLfoRate,c.tempoBpm,c.lfoSync!=0)/float(rate); modSin=0; modCos=1; modStepSin=std::sin(modStep); modStepCos=std::cos(modStep); modDepth=c.modLfoDepth; modTarget=c.modTarget;
    filterCoefficient = .005f + .92f * c.filterCutoff; filterEnvelope.trigger(velocity*c.filterEnvAmount, std::exp(-1.0f/(float(rate)*(.004f+2.0f*c.filterEnvDecay))));
    filterResonance = c.filterResonance * .92f; filterMode = c.filterMode;
    for (int i=0;i<4;++i) { auto& voice=sampleVoices[(size_t)i]; voice.data=std::atomic_load_explicit(&sampleData[(size_t)i], std::memory_order_acquire); configureSampleVoice(voice,c.samples[(size_t)i],velocity,c.tempoBpm,c.masterTune,c.lfoSync!=0); }
}
std::array<float, 2> ImpactVoiceCore::render() {
    if (onsetDelaySamples > 0) { --onsetDelaySamples; return {}; }
    auto modalOutput = modal.process(); auto residual = noise.process();
    const auto modulation=modSin*modDepth; const auto nextModSin=modSin*modStepCos+modCos*modStepSin; modCos=modCos*modStepCos-modSin*modStepSin; modSin=nextModSin; auto sub=0.0f;
    for (auto& subVoice : subVoices) { if (subVoice.delaySamples-- <= 0) sub += subVoice.sin * subVoice.envelope; const auto nextSin = subVoice.sin * subVoice.stepCos + subVoice.cos * subVoice.stepSin; subVoice.cos = subVoice.cos * subVoice.stepCos - subVoice.sin * subVoice.stepSin; subVoice.sin = nextSin; subVoice.envelope *= subVoice.decayCoefficient; }
    auto left = modalOutput[0] + residual[0] + sub, right = modalOutput[1] + residual[1] + sub;
    for (auto& voice: sampleVoices) if (voice.active) { if(voice.delaySamples-- > 0) continue; const auto frames=voice.data->audio.getNumSamples(); if(!keepSampleInRange(voice,frames))continue; const auto index=int(voice.position); const auto frac=float(voice.position-index); const auto read=[&](int channel){auto* d=voice.data->audio.getReadPointer(juce::jmin(channel,voice.data->audio.getNumChannels()-1)); return d[index]+(d[index+1]-d[index])*frac;}; const auto nearEnd=voice.reverse?index<voice.releaseSamples:index>=frames-voice.releaseSamples; if(voice.released){voice.envelope*=voice.releaseCoefficient;if(voice.envelope<.00001f){voice.active=false;continue;}}else if(voice.stage==EnvelopeStage::attack){voice.envelope=juce::jmin(1.0f,voice.envelope+voice.attackStep);if(voice.envelope>=1.0f)voice.stage=EnvelopeStage::decay;}else if(!voice.loop&&nearEnd){voice.envelope*=voice.reverse?float(index)/float(voice.releaseSamples):float(frames-index)/float(voice.releaseSamples);}else if(voice.stage==EnvelopeStage::decay){voice.envelope=voice.sustain+(voice.envelope-voice.sustain)*voice.decayCoefficient;if(std::abs(voice.envelope-voice.sustain)<.001f)voice.stage=EnvelopeStage::sustain;} const auto amp=voice.level*voice.envelope; left+=read(0)*amp*voice.panL; right+=read(1)*amp*voice.panR; voice.position+=(voice.reverse?-1.0:1.0)*voice.increment*(1.0+voice.lfoSin*voice.lfoDepth*.12); const auto nextLfoSin=voice.lfoSin*voice.lfoStepCos+voice.lfoCos*voice.lfoStepSin; voice.lfoCos=voice.lfoCos*voice.lfoStepCos-voice.lfoSin*voice.lfoStepSin; voice.lfoSin=nextLfoSin; }
    const auto modulatedCoefficient=juce::jlimit(.002f,.995f,filterCoefficient+filterEnvelope.next()*.4f+(modTarget==1?modulation*.25f:0)); if(modTarget==0){left*=1.0f+modulation;right*=1.0f+modulation;} filterLowL += modulatedCoefficient * (left - filterLowL + filterResonance * filterBandL); filterBandL += modulatedCoefficient * (filterLowL - filterBandL); filterLowR += modulatedCoefficient * (right - filterLowR + filterResonance * filterBandR); filterBandR += modulatedCoefficient * (filterLowR - filterBandR);
    if(filterMode==1) return {filterLowL,filterLowR}; if(filterMode==2) return {left-filterLowL,right-filterLowR}; if(filterMode==3) return {filterBandL,filterBandR}; return {left,right};
}
bool ImpactVoiceCore::active() const noexcept { if (onsetDelaySamples > 0 || modal.active()) return true; for(const auto& sub:subVoices) if(sub.envelope>.00001f) return true; for(const auto& voice:sampleVoices) if(voice.active) return true; return false; }
bool ImpactVoiceCore::samplesActive() const noexcept { for (const auto& voice : sampleVoices) if (voice.active) return true; return false; }
void ImpactVoiceCore::release() noexcept { for (auto& voice : sampleVoices) voice.released = true; }
void ImpactVoiceCore::configureSampleVoice(SampleVoice& voice, const ImpactControls::SampleLayer& layer, float velocity, float tempoBpm, float masterTune, bool lfoSync) { voice.reverse=layer.reverse; const auto lastFrame=voice.data?juce::jmax(0,voice.data->audio.getNumSamples()-2):0; voice.position=voice.reverse?(1.0-layer.start)*lastFrame:layer.start*lastFrame; voice.increment=std::pow(2.0,(layer.tune-.5f)*4.0 + masterTune / 12.0f)*(voice.data?voice.data->sampleRate/rate:1.0); voice.delaySamples=int(layer.delay*2.0f*rate); voice.envelope=0; voice.attackStep=1.0f/juce::jmax(1.0f,layer.attack*.5f*float(rate)); voice.decayCoefficient=std::exp(-1.0f/(float(rate)*(.01f+2.0f*layer.decay))); voice.sustain=layer.sustain; voice.releaseSamples=juce::jmax(1,int((.005f+1.5f*layer.release)*rate)); voice.releaseCoefficient=std::exp(-1.0f/float(voice.releaseSamples)); voice.stage=EnvelopeStage::attack; voice.released=false; voice.loop=layer.loop; const auto pan=layer.pan*juce::MathConstants<float>::halfPi; voice.panL=std::cos(pan); voice.panR=std::sin(pan); const auto lfoStep=juce::MathConstants<float>::twoPi*lfoFrequency(layer.lfoRate,tempoBpm,lfoSync)/float(rate); voice.lfoSin=0; voice.lfoCos=1; voice.lfoStepSin=std::sin(lfoStep); voice.lfoStepCos=std::cos(lfoStep); voice.lfoDepth=layer.lfoDepth; voice.level=velocity*layer.level; voice.active=voice.data&&voice.level>.00001f; }
bool ImpactVoiceCore::keepSampleInRange(SampleVoice& voice, int frames) noexcept { if(frames<2){voice.active=false;return false;} const auto outOfRange=voice.reverse?voice.position<=0:voice.position>=frames-1; if(!outOfRange)return true; if(voice.loop&&!voice.released){voice.position=voice.reverse?frames-2:0;return true;} voice.active=false;return false; }
void ImpactVoiceCore::setSample (int slot, std::shared_ptr<const ImpactSampleData> sample) { if(juce::isPositiveAndBelow(slot,4)) std::atomic_store_explicit(&sampleData[(size_t)slot],std::move(sample),std::memory_order_release); }
}
